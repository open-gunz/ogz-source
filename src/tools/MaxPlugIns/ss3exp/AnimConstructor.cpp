
#include "Max.h"
#include "decomp.h"
#include "mesh.h"
#include "AnimConstructor.h"
#include "BELog.h"


#define ALMOST_ZERO 1.0e-3f

// Not truly the correct way to compare floats of arbitary magnitude...
BOOL EqualPoint3(Point3 p1, Point3 p2)
{
	if (fabs(p1.x - p2.x) > ALMOST_ZERO)
		return FALSE;
	if (fabs(p1.y - p2.y) > ALMOST_ZERO)
		return FALSE;
	if (fabs(p1.z - p2.z) > ALMOST_ZERO)
		return FALSE;

	return TRUE;
}

ACAnimNodeList::~ACAnimNodeList()
{
	for(iterator i=begin();i!=end();i++)
		delete *i;
}

ACAnimNodeList::iterator ACAnimNodeList::Get(char *Name)
{
	for(iterator i=begin();i!=end();i++)
		if(strcmp((*i)->name,Name)==0)
			return i;
	return NULL;
}

AnimConstructor::AnimConstructor()
{
	m_bDetailLog=false;
}

TriObject* AnimConstructor::GetTriObjectFromNode(INode *node, TimeValue t, int &deleteIt)
{
	deleteIt = FALSE;
	Object *obj = node->EvalWorldState(t).obj;
	if (obj->CanConvertToType(Class_ID(TRIOBJ_CLASS_ID, 0))) { 
		TriObject *tri = (TriObject *) obj->ConvertToType(t, 
			Class_ID(TRIOBJ_CLASS_ID, 0));
		// Note that the TriObject should only be deleted
		// if the pointer to it is not equal to the object
		// pointer that called ConvertToType()
		if (obj != tri) deleteIt = TRUE;
		return tri;
	}
	else {
		return NULL;
	}
}

bool AnimConstructor::CheckForAnimation(INode* node, BOOL& bPos, BOOL& bRot, BOOL& bScale)
{
	TimeValue start = ip->GetAnimRange().Start();
	TimeValue end = ip->GetAnimRange().End();
	TimeValue t;
	int delta = GetTicksPerFrame();
	Matrix3 tm;
	AffineParts ap;
	Point3 firstPos;
	float rotAngle, firstRotAngle;
	Point3 rotAxis;
	Point3 firstScaleFactor;

	bPos = bRot = bScale = FALSE;

	for (t=start; t<=end; t+=delta) {
		tm = node->GetNodeTM(t) * Inverse(node->GetParentTM(t));

		decomp_affine(tm, &ap);

		AngAxisFromQ(ap.q, &rotAngle, rotAxis);

		if (t != start) {
			if (!EqualPoint3(ap.t, firstPos)) {
				bPos = TRUE;
			}
			// We examine the rotation angle to see if the rotation component
			// has changed.
			// Although not entierly true, it should work.
			// It is rare that the rotation axis is animated without
			// the rotation angle being somewhat affected.
			if (fabs(rotAngle - firstRotAngle) > ALMOST_ZERO) {
				bRot = TRUE;
			}
			if (!EqualPoint3(ap.k, firstScaleFactor)) {
				bScale = TRUE;
			}			
		}
		else {
			firstPos = ap.t;
			firstRotAngle = rotAngle;
			firstScaleFactor = ap.k;
		}

		// No need to continue looping if all components are animated
		if (bPos && bRot && bScale)
			break;
	}

	return bPos || bRot || bScale;
}

bool AnimConstructor::isNotBipName(const char *name)
{
	return strncmp(name,"Bip",3)!=0;
}

#define Indent(x) (&IndentBuffer[255-x*2])

static char path[20][256]={0,};

bool AnimConstructor::nodeEnum(INode* node, int indentLevel) 
{
	/*
	for(int l=0;l<indentLevel;l++) log("  ");
	log("traverse : %s\n",node->GetName());
	*/
	strcpy(path[indentLevel],node->GetName());
	indentLevel++;

	ObjectState os = node->EvalWorldState(0); 

	if (os.obj) {
		if(os.obj->SuperClassID()==GEOMOBJECT_CLASS_ID)
		{
			if(isNotBipName(node->GetName()))
			{
				TriObject *tri=NULL;
				int deleteIt = FALSE;
				Object *obj = node->EvalWorldState(0).obj;
				if (obj->CanConvertToType(Class_ID(TRIOBJ_CLASS_ID, 0))) { 
					tri = (TriObject *) obj->ConvertToType(0, 
						Class_ID(TRIOBJ_CLASS_ID, 0));
					// Note that the TriObject should only be deleted
					// if the pointer to it is not equal to the object
					// pointer that called ConvertToType()
					if (obj != tri) deleteIt = TRUE;
					Mesh* mesh = &tri->mesh;
					if(isNotBipName(node->GetName())
						&&(mesh->getNumVerts()!=0)
						&&(mesh->getNumFaces()!=0))
					{
						AnimNode *an=new AnimNode;
						an->nV=0;
						an->name=new char[strlen(node->GetName())+1];
						strcpy(an->name,node->GetName());

						if(m_bDetailLog)
							log("Processing Animation Object : %s\n",an->name );

						TimeValue start = ip->GetAnimRange().Start();
						TimeValue end = ip->GetAnimRange().End();
						int i,j,t;

						if(an->name[0]!='_')
						{
							switch(m_AnimationMethod)
							{
								case AM_TRANSFORM: {
									an->tm=new Matrix3[nFrame];
									for(t=0;t<nFrame;t++)
										an->tm[t] = node->GetNodeTM(t*delta+start);
								} break;
								case AM_VERTEX: {
									an->nV=mesh->getNumVerts();
									Point3 *tp=an->vertanim=new Point3[an->nV*nFrame];
									for(i=0;i<nFrame;i++)
									{
										t=i*delta+start;
										node->EvalWorldState(t);
										BOOL needDel;
										Matrix3 tm = node->GetObjTMAfterWSM(t);
										TriObject* tri = GetTriObjectFromNode(node, t, needDel);
										Mesh* mesh = &tri->GetMesh();

										for (j=0; j<mesh->getNumVerts(); j++) {
											*tp = tm * mesh->verts[j];
											tp->x=-tp->x;
											tp++;
										}

										if(needDel)
											delete tri;
									}
								} break;
								case AM_KEYFRAME: BuildKeys(an,node);break;
							}
						}
						
						BuildVisibility(&an->Visibility,node);

						objlist.push_back(an);
					}
					if(deleteIt)
						delete tri;
				} // of if (tri)
			}
		}
	}
	
	// For each child of this node, we recurse into ourselves 
	// until no more children are found.
	for (int c = 0; c < node->NumberOfChildren(); c++) {
		if (!nodeEnum(node->GetChildNode(c), indentLevel))
			return FALSE;
	}
	
	indentLevel--;
	return TRUE;
}

bool AnimConstructor::BuildAnimation(Interface *ip,char *name,float fSpeed,ANIMATIONMETHOD method)
{
	strcpy(m_szName,name);
	m_fSpeed=fSpeed;
	m_fFrameSpeed=GetFrameRate();
	m_AnimationMethod=method;
	AnimConstructor::ip=ip;

	TimeValue start = ip->GetAnimRange().Start();
	TimeValue end = ip->GetAnimRange().End();
	delta = GetTicksPerFrame();
//	if(bVertexAnimation) delta*=4;
	nFrame = (end-start)/delta+1;

	nodeEnum(ip->GetRootNode(), 0);
//	objlist.Sort();
	return true;
}

void AnimConstructor::Save(FILE *pStream)
{
	ACAnimNodeList::iterator i;
	int j,k;
	int nObject=objlist.size();
	
	fputc(strlen(m_szName),pStream);
	fwrite(m_szName,strlen(m_szName),1,pStream);
	fwrite(&m_fFrameSpeed,sizeof(float),1,pStream);
	fwrite(&m_fSpeed,sizeof(float),1,pStream);
	fwrite(&nFrame,sizeof(int),1,pStream);
	fwrite(&nObject,sizeof(int),1,pStream);
	fwrite(&m_AnimationMethod,sizeof(int),1,pStream);
	for(i=objlist.begin();i!=objlist.end();i++)
	{
		AnimNode *an=*i;
		if(m_bDetailLog) 
			log("Ani Write : %s \n",an->name);
		switch(m_AnimationMethod)
		{
			case AM_TRANSFORM:
			{
				for(j=0;j<nFrame;j++)
				{
					for(k=0;k<4;k++)
					{
						Point3 col;
						col=an->tm[j].GetRow(k);
						col.x=-col.x;
						fwrite(&col,sizeof(col),1,pStream);
					}
				}
			}break;
			case AM_VERTEX:
			{
				fwrite(&an->nV,sizeof(int),1,pStream);
				if(an->nV)
					fwrite(an->vertanim,sizeof(Point3),an->nV*nFrame,pStream);
			}break;
			case AM_KEYFRAME: 
			{
				an->RotationKeyList.Save(pStream);
				an->ScaleKeyList.Save(pStream);
				an->PositionKeyList.Save(pStream);
			}break;
		}
	}


	for(i=objlist.begin();i!=objlist.end();i++)
	{
		AnimNode *an=*i;

		/*
		log("%s : \n",an->name);
		for(RSVisList::iterator i=an->Visibility.begin();i!=an->Visibility.end();i++)
			log("%f %f \n",(*i)->time,(*i)->value);
*/
		an->Visibility.Save(pStream);
	}
}

void RSVisList::Get(float fTime,RSVisibilityKey *ret)
{
	if(!size()) {ret->value=1.f;return;}

	m_Current=end();
	m_Current--;
	while((*m_Current)->time>fTime && m_Current!=begin())
		m_Current--;

	RSVisList::iterator Next=NULL;
	if(m_Current!=end()) {
		Next=m_Current;
		Next++;
	}
		
	if((*m_Current)->time>fTime) {ret->value=(*m_Current)->value;return;}
	if(Next!=NULL) {ret->value=(*m_Current)->value;return;}

	float t=(fTime-(*m_Current)->time)/((*Next)->time-(*m_Current)->time);
	ret->value=(*m_Current)->value*(1-t)+(*Next)->value*t;
}

// 부모가 visibility 키를 갖고 있으면 recursive 하게 얻어내서 중복의 소지가 있음.
// 비효율적이긴 하나 코드가 간단해서 걍 이렇게 해놨슴.

// Output float keys if this is a known float controller that
// supports key operations. Otherwise we will sample the controller 
// once for each frame to get the value.
bool AnimConstructor::BuildVisibility(RSVisList *vl, INode *node) 
{
	if(!node) return false;
//	log("object : %s \n",node->GetName());
	RSVisList vlparent,vlthis;
	bool bParent=BuildVisibility(&vlparent,node->GetParentNode()),bThis=false;

	Control* cont= node->GetVisController();
	if(cont)
	{
		int i;
		IKeyControl *ikc = NULL;
		ikc = GetKeyControlInterface(cont);
		if(ikc)
		{
			for (i=0; i<ikc->GetNumKeys(); i++) {

				RSVisibilityKey *rkey=new RSVisibilityKey;
				if(cont->ClassID() == Class_ID(TCBINTERP_FLOAT_CLASS_ID, 0))
				{
					ITCBFloatKey key;
					ikc->GetKey(i, &key);
					rkey->time=key.time/GetTicksPerFrame();
					rkey->value=key.val;
				}else
				if(cont->ClassID() == Class_ID(HYBRIDINTERP_FLOAT_CLASS_ID, 0))
				{
					IBezFloatKey key;
					ikc->GetKey(i, &key);
					rkey->time=key.time/GetTicksPerFrame();
					rkey->value=key.val;
				}else
				if(cont->ClassID() == Class_ID(LININTERP_FLOAT_CLASS_ID, 0)) {
					ILinFloatKey key;
					ikc->GetKey(i, &key);
					rkey->time=key.time/GetTicksPerFrame();
					rkey->value=key.val;
				}
					
				vlthis.push_back(rkey);
			}
			bThis=true;
		}
	}

	if(!bParent && !bThis) return false;

	if(bParent && bThis)
	{
		RSVisList::iterator nP=vlparent.begin(),nT=vlthis.begin();
		while(nP!=vlparent.end() || nT!=vlthis.end())
		{
			float time;
			if(nP!=vlparent.end() && nT!=vlthis.end() )
			{
				if((*nP)->time < (*nT)->time )
				{
					time=(*nP)->time;
					nP++;
				} else
				{
					time=(*nT)->time;
					nT++;
				}
			} else
			if(nP!=vlparent.end())
			{
				time=(*nP)->time;
				nP++;
			} else
			{
				time=(*nT)->time;
				nT++;
			}
			
			RSVisibilityKey kP,kT;
			vlparent.Get(time,&kP);
			vlthis.Get(time,&kT);

			RSVisibilityKey *rkey=new RSVisibilityKey;
			rkey->time=time;
			rkey->value=kP.value*kT.value;
			vl->push_back(rkey);
		}
	}
	else
	{
		RSVisList *pvl=bParent ? &vlparent : &vlthis;
		vl->insert(vl->begin(),pvl->begin(),pvl->end());
		pvl->erase(pvl->begin(),pvl->end());
	}

	return true;
}

// Determine if a TM controller is known by the system.
bool AnimConstructor::IsKnownController(Control* cont)
{
	ulong partA, partB;

	if (!cont)
		return FALSE;

	partA = cont->ClassID().PartA();
	partB = cont->ClassID().PartB();

	if (partB != 0x00)
		return FALSE;

	switch (partA) {
		case TCBINTERP_POSITION_CLASS_ID:
		case TCBINTERP_ROTATION_CLASS_ID:
		case TCBINTERP_SCALE_CLASS_ID:
		case HYBRIDINTERP_POSITION_CLASS_ID:
		case HYBRIDINTERP_ROTATION_CLASS_ID:
		case HYBRIDINTERP_SCALE_CLASS_ID:
		case LININTERP_POSITION_CLASS_ID:
		case LININTERP_ROTATION_CLASS_ID:
		case LININTERP_SCALE_CLASS_ID:
			return TRUE;
	}

	return FALSE;
}

bool AnimConstructor::BuildKeys(AnimNode *an, INode *node)
{
	int i;

	Control* pC = node->GetTMController()->GetPositionController();
	Control* rC = node->GetTMController()->GetRotationController();
	Control* sC = node->GetTMController()->GetScaleController();

	_ASSERT(IsKnownController(pC));
	_ASSERT(IsKnownController(rC));
	_ASSERT(IsKnownController(sC));

	// rotation Key
	{
		IKeyControl *ikc = GetKeyControlInterface(rC);
		if (ikc && rC->ClassID() == Class_ID(TCBINTERP_ROTATION_CLASS_ID, 0)) {
			int numKeys;
			if (numKeys = ikc->GetNumKeys()) {
				for (i=0; i<numKeys; i++) {
					ITCBRotKey key;
					ikc->GetKey(i, &key);
					RSRotationKey *pKey=new RSRotationKey;
					pKey->time=key.time/GetTicksPerFrame();
					pKey->value=key.val;
					an->RotationKeyList.push_back(pKey);
				}
			}
		}
	}

	// scale key
	{
		IKeyControl *ikc = GetKeyControlInterface(sC);
		if (ikc && sC->ClassID() == Class_ID(TCBINTERP_SCALE_CLASS_ID, 0))
		{
			int numKeys;
			if (numKeys = ikc->GetNumKeys()) {
				for (i=0; i<numKeys; i++) {
					ITCBScaleKey key;
					ikc->GetKey(i, &key);
					RSScaleKey *pKey=new RSScaleKey;
					pKey->time=key.time/GetTicksPerFrame();
					pKey->value=key.val;
					an->ScaleKeyList.push_back(pKey);
				}
			}
		}
	}
	
	// position Key
	{
		IKeyControl *ikc = GetKeyControlInterface(pC);
		// TCB position
		if (ikc && pC->ClassID() == Class_ID(TCBINTERP_POSITION_CLASS_ID, 0)) {
			int numKeys;
			if (numKeys = ikc->GetNumKeys()) {
				for (i=0; i<numKeys; i++) {
					ITCBPoint3Key key;
					ikc->GetKey(i, &key);
					RSPositionKey *pKey=new RSPositionKey;
					pKey->time=key.time/GetTicksPerFrame();
					pKey->value=key.val;
					an->PositionKeyList.push_back(pKey);
				}
			}
		}

	}
	return true;
}