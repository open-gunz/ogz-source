#include "exporter.h"
#include "BELog.h"
#include "RSMaterialList.h"

/****************************************************************************

  Global output [Scene info]
  
****************************************************************************/

// Dump some global animation information.
void Exporter::ExportGlobalInfo()
{
	Interval range = ip->GetAnimRange();

	struct tm *newtime;
	time_t aclock;

	time( &aclock );
	newtime = localtime(&aclock);

	TSTR today = _tasctime(newtime);	// The date string has a \n appended.
	today.remove(today.length()-1);		// Remove the \n

	strcpy(rsm->MaxFileName,FixupName(ip->GetCurFilePath()));
	
//	Texmap* env = ip->GetEnvironmentMap();

}

/****************************************************************************

  GeomObject output
  
****************************************************************************/

void Exporter::ExportGeomObject(INode* node, int indentLevel)
{
	ObjectState os = node->EvalWorldState(GetStaticFrame());
	if (!os.obj)
		return;
	
	// Targets are actually geomobjects, but we will export them
	// from the camera and light objects, so we skip them here.
	if (os.obj->ClassID() == Class_ID(TARGET_CLASS_ID, 0))
		return;
	
	
	ExportNodeHeader(node);
	ExportNodeTM(node, indentLevel,&rsm->mesh->mat);
	ExportMesh(node, GetStaticFrame(), indentLevel);
	ExportMaterial(node, indentLevel);
}

void Exporter::ExportNodeHeader(INode* node)
{
	{	// add mesh;
		rsm->mesh=new RSMMesh;
		rsm->MeshList.Add(rsm->mesh);
		sprintf(rsm->mesh->name,"%s",FixupName(node->GetName()));
	}
}

void Exporter::ExportNodeTM(INode* node, int indentLevel,rmatrix *mat)
{
	Matrix3 pivot = node->GetNodeTM(GetStaticFrame());
	// Export TM inheritance flags
	DWORD iFlags = node->GetTMController()->GetInheritanceFlags();
	Point3 row;
	row = pivot.GetRow(0);
	mat->_11=-row.x;mat->_12=row.y;mat->_13=row.z;mat->_14=0;
	row = pivot.GetRow(1);
	mat->_21=-row.x;mat->_22=row.y;mat->_23=row.z;mat->_24=0;
	row = pivot.GetRow(2);
	mat->_31=-row.x;mat->_32=row.y;mat->_33=row.z;mat->_34=0;
	row = pivot.GetRow(3);
	mat->_41=-row.x;mat->_42=row.y;mat->_43=row.z;mat->_44=1;
	*mat=MatrixInverse(*mat);
}


/****************************************************************************

  Mesh output
  
****************************************************************************/

void Exporter::ExportMesh(INode* node, TimeValue t, int indentLevel)
{
	int i;
	Mtl* nodeMtl = node->GetMtl();
	Matrix3 tm = node->GetObjTMAfterWSM(t);
	BOOL negScale = TMNegParity(tm);
	int vx1, vx2, vx3;
	TSTR indent;
	
	ObjectState os = node->EvalWorldState(t);
	if (!os.obj || os.obj->SuperClassID()!=GEOMOBJECT_CLASS_ID) {
		return; // Safety net. This shouldn't happen.
	}
	
	// Order of the vertices. Get 'em counter clockwise if the objects is
	// negatively scaled.
	if (negScale) {
		vx1 = 2;
		vx2 = 1;
		vx3 = 0;
	}
	else {
		vx1 = 0;
		vx2 = 1;
		vx3 = 2;
	}
	
	BOOL needDel;
	TriObject* tri = GetTriObjectFromNode(node, t, needDel);
	if (!tri) {
		return;
	}
	
	Mesh* mesh = &tri->mesh;
	
	mesh->buildNormals();
	
	{	// make vertices and faces
		rsm->mesh->nV=mesh->getNumVerts();
		rsm->mesh->nF=mesh->getNumFaces();
		rsm->mesh->ver=new rvertex[mesh->getNumVerts()];
		rsm->mesh->face=new rface[mesh->getNumFaces()];
	}
	
	// Export the vertices
	for (i=0; i<mesh->getNumVerts(); i++) {
		Point3 v = tm * mesh->verts[i];
		rsm->mesh->ver[i].coord=rvector(v);
		rsm->mesh->ver[i].normal=rvector(0,0,0);
	}
	
	// To determine visibility of a face, get the vertices in clockwise order.
	// If the objects has a negative scaling, we must compensate for that by
	// taking the vertices counter clockwise
	for (i=0; i<mesh->getNumFaces(); i++) {
		
		rsm->mesh->face[i].a=(WORD)mesh->faces[i].v[vx1];
		rsm->mesh->face[i].c=(WORD)mesh->faces[i].v[vx2];
		rsm->mesh->face[i].b=(WORD)mesh->faces[i].v[vx3];
		rsm->mesh->face[i].nMaterial=mesh->faces[i].getMatID();
	}
	
	// Export face map texcoords if we have them...
	if (!CheckForAndExportFaceMap(nodeMtl, mesh, indentLevel+1)) {
		// If not, export standard tverts
		int numTVx = mesh->getNumTVerts();

		if (numTVx) {
			rface *f=rsm->mesh->face;
			for (i=0; i<mesh->getNumFaces(); i++) {
// dubble added
				TVFace *tvf=&mesh->tvFace[i];
				f->u[0]=mesh->tVerts[tvf->t[vx1]].x;
				f->v[0]=1.0f-mesh->tVerts[tvf->t[vx1]].y;
				f->u[2]=mesh->tVerts[tvf->t[vx2]].x;
				f->v[2]=1.0f-mesh->tVerts[tvf->t[vx2]].y;
				f->u[1]=mesh->tVerts[tvf->t[vx3]].x;
				f->v[1]=1.0f-mesh->tVerts[tvf->t[vx3]].y;  // good
				f++;
// end of dubble added
			}
		}
	}

	{
		// Export mesh (face + vertex) normals
		
		Point3 fn;  // Face normal
		Point3 vn;  // Vertex normal
		int  vert;
		Face* f;
		
		// Face and vertex normals.
		// In MAX a vertex can have more than one normal (but doesn't always have it).
		// This is depending on the face you are accessing the vertex through.
		// To get all information we need to export all three vertex normals
		// for every face.
		Matrix3 pivot = node->GetNodeTM(GetStaticFrame());
		pivot.NoTrans();

		for (i=0; i<mesh->getNumFaces(); i++) {
			f = &mesh->faces[i];
			fn = mesh->getFaceNormal(i);
			rsm->mesh->face[i].normal=fn;
			
			vert = f->getVert(vx1);
			vn = GetVertexNormal(mesh, i, mesh->getRVertPtr(vert));
			rsm->mesh->face[i].vnormals[0]=pivot*vn;
			rsm->mesh->ver[rsm->mesh->face[i].a].normal+=vn;
			
			vert = f->getVert(vx2);
			vn = GetVertexNormal(mesh, i, mesh->getRVertPtr(vert));
			rsm->mesh->face[i].vnormals[2]=pivot*vn;
			rsm->mesh->ver[rsm->mesh->face[i].b].normal+=vn;
			
			vert = f->getVert(vx3);
			vn = GetVertexNormal(mesh, i, mesh->getRVertPtr(vert));
			rsm->mesh->face[i].vnormals[1]=pivot*vn;
			rsm->mesh->ver[rsm->mesh->face[i].c].normal+=vn;
		}
	}
	
	if (needDel) {
		delete tri;
	}
}

Point3 Exporter::GetVertexNormal(Mesh* mesh, int faceNo, RVertex* rv)
{
	Face* f = &mesh->faces[faceNo];
	DWORD smGroup = f->smGroup;
	int numNormals;
	Point3 vertexNormal;
	
	// Is normal specified
	// SPCIFIED is not currently used, but may be used in future versions.
	if (rv->rFlags & SPECIFIED_NORMAL) {
		vertexNormal = rv->rn.getNormal();
	}
	// If normal is not specified it's only available if the face belongs
	// to a smoothing group
	else if ((numNormals = rv->rFlags & NORCT_MASK) && smGroup) {
		// If there is only one vertex is found in the rn member.
		if (numNormals == 1) {
			vertexNormal = rv->rn.getNormal();
		}
		else {
			// If two or more vertices are there you need to step through them
			// and find the vertex with the same smoothing group as the current face.
			// You will find multiple normals in the ern member.
			for (int i = 0; i < numNormals; i++) {
				if (rv->ern[i].getSmGroup() & smGroup) {
					vertexNormal = rv->ern[i].getNormal();
				}
			}
		}
	}
	else {
		// Get the normal from the Face if no smoothing groups are there
		vertexNormal = mesh->getFaceNormal(faceNo);
	}
	
	return vertexNormal;
}

/****************************************************************************

  Material and Texture Export
  
****************************************************************************/

void Exporter::ExportMaterialList()
{

	int numMtls = mtlList.Count();

	for (int i=0; i<numMtls; i++) {
		MaxMaterial *maxm=new MaxMaterial;
		rsm->MaxMaterialList.Add(maxm);
		DumpMaterial(maxm,mtlList.GetMtl(i), i, -1, 0);
	}
}

void Exporter::ExportMaterial(INode* node, int indentLevel)
{
	Mtl* mtl = node->GetMtl();
	
	// If the node does not have a material, export the wireframe color
	if (mtl) {
		int mtlID = mtlList.GetMtlID(mtl);
		if (mtlID >= 0) {
			rsm->mesh->refMaterial=mtlID;
		}
	}
	else {
		DWORD c = node->GetWireColor();
		rsm->mesh->refMaterial=-1;
	}
}

char *GetStdMaterialName(Mtl* mtl)
{
	if(mtl->ClassID() == Class_ID(DMTL_CLASS_ID, 0))	// if standard material
		return mtl->GetName();
	_ASSERT(mtl->NumSubMtls() > 0);
	return GetStdMaterialName(mtl->GetSubMtl(0));
}

void Exporter::DumpMaterial(MaxMaterial *maxm,Mtl* mtl, int mtlID, int subNo, int indentLevel)
{
	int i;
	TimeValue t = GetStaticFrame();
	
	if (!mtl) return;
	
//	for(i=0;i<indentLevel;i++) { log("   "); }
//	log("material %s adding. type : ",mtl->GetName());
	
	// We know the Standard material, so we can get some extra info
	if (mtl->ClassID() == Class_ID(DMTL_CLASS_ID, 0)) {			// top level & standard material

//		log("standard \n");
		StdMat* std = (StdMat*)mtl;

		MaxStdMaterial *stdm=new MaxStdMaterial;
		strcpy(stdm->name,mtl->GetName());
		stdm->Ambient=rvector(std->GetAmbient(t));
		stdm->Ambient.x=-stdm->Ambient.x;
		stdm->Diffuse=rvector(std->GetDiffuse(t));
		stdm->Diffuse.x=-stdm->Diffuse.x;
		stdm->Specular=rvector(std->GetSpecular(t));
		stdm->Specular.x=-stdm->Specular.x;				// 축의 바뀜때문에 만들어 놓은.. 으흑..
		stdm->TwoSide=std->GetTwoSided();
		if(std->GetTransparencyType()==TRANSP_ADDITIVE)
			stdm->ShadeMode=RSSHADEMODE_ADD;
		else stdm->ShadeMode=RSSHADEMODE_NORMAL;
		
		if(rsm->MaxStdMaterialList.GetByName(stdm->name)==-1)	// 이미 있는 standard material 이면 더하지 않음.
		{
			rsm->MaxStdMaterialList.Add(stdm);
			stdm->RMLIndex=rsm->MaxStdMaterialList.GetCount()-1;
			for (i=0; i<mtl->NumSubTexmaps(); i++) {
				Texmap* subTex = mtl->GetSubTexmap(i);
				float amt = 1.0f;
				if (subTex) {
					// If it is a standard material we can see if the map is enabled.
					if (mtl->ClassID() == Class_ID(DMTL_CLASS_ID, 0)) {
						if (!((StdMat*)mtl)->MapEnabled(i))
							continue;
						amt = ((StdMat*)mtl)->GetTexmapAmt(i, 0);
						
					}
					DumpTexture(stdm, subTex, mtl->ClassID(), i, amt, indentLevel+1);
				}
			}
		}
		else
		{
			delete stdm;
		}

		maxm->nSubMaterial=1;
		maxm->SubMaterials=new int[1];
		maxm->SubMaterials[0]=rsm->MaxStdMaterialList.GetByName(mtl->GetName());
	}

	if (mtl->NumSubMtls() > 0)  {
//		log("multi/sub ( count : %d )\n",mtl->NumSubMtls());
		maxm->nSubMaterial=mtl->NumSubMtls();
		maxm->SubMaterials=new int[maxm->nSubMaterial];
		maxm->pSubMaterials=new MaxMaterial*[maxm->nSubMaterial];

		for (i=0; i<mtl->NumSubMtls(); i++) {
			Mtl* subMtl = mtl->GetSubMtl(i);
			if (subMtl) {
				maxm->pSubMaterials[i]=new MaxMaterial;
				DumpMaterial(maxm->pSubMaterials[i],subMtl, 0, i, indentLevel+1);
				if(subMtl->ClassID() == Class_ID(DMTL_CLASS_ID, 0))
				{
					maxm->SubMaterials[i]= rsm->MaxStdMaterialList.GetByName(subMtl->GetName());
				}
				else
					maxm->SubMaterials[i]= maxm->pSubMaterials[i]->SubMaterials[0];
			}
			else
			{
				maxm->pSubMaterials[i]=NULL;
				maxm->SubMaterials[i]=-1;
			}
		}
	}
}


void Exporter::DumpTexture(MaxStdMaterial *stdm,Texmap* tex, Class_ID cid, int subNo, float amt, int indentLevel)
{
	if (!tex) return;
	
	TSTR className;
	tex->GetClassName(className);

	// Is this a bitmap texture?
	// We know some extra bits 'n pieces about the bitmap texture
	if (tex->ClassID() == Class_ID(BMTEX_CLASS_ID, 0x00)) {
		TSTR mapName = ((BitmapTex *)tex)->GetMapName();

		switch(subNo)  // dubble added.
		{
		case ID_DI:	strcpy(stdm->DiMapName,mapName);break;
		case ID_OP:	stdm->ShadeMode=RSSHADEMODE_ALPHAMAP;strcpy(stdm->OpMapName,mapName);break;
		}
		
		StdUVGen* uvGen = ((BitmapTex *)tex)->GetUVGen();
		if (uvGen) {
			DumpUVGen(stdm,uvGen, indentLevel+1);
			
		}
		
	}
	
	for (int i=0; i<tex->NumSubTexmaps(); i++) {
		DumpTexture(stdm,tex->GetSubTexmap(i), tex->ClassID(), i, 1.0f, indentLevel+1);
	}
}

void Exporter::DumpUVGen(MaxStdMaterial *stdm,StdUVGen* uvGen, int indentLevel)
{
	int mapType = uvGen->GetCoordMapping(0);
	TimeValue t = GetStaticFrame();
	stdm->sina=sin(uvGen->GetAng(t));
	stdm->cosa=cos(uvGen->GetAng(t));
	stdm->uOffset=uvGen->GetUOffs(t);
	stdm->uTiling=uvGen->GetUScl(t);
	stdm->vOffset=uvGen->GetVOffs(t);
	stdm->vTiling=uvGen->GetVScl(t);
}

/****************************************************************************

  Face Mapped Material functions
  
****************************************************************************/

BOOL Exporter::CheckForAndExportFaceMap(Mtl* mtl, Mesh* mesh, int indentLevel)
{
	if (!mtl || !mesh) {
		return FALSE;
	}
	
	ULONG matreq = mtl->Requirements(-1);
	
	// Are we using face mapping?
	if (!(matreq & MTLREQ_FACEMAP)) {
		return FALSE;
	}
	
	// OK, we have a FaceMap situation here...
	
	for (int i=0; i<mesh->getNumFaces(); i++) {
		Point3 tv[3];
		Face* f = &mesh->faces[i];
		make_face_uv(f, tv);
	}
	
	return TRUE;
}


// Determine is the node has negative scaling.
// This is used for mirrored objects for example. They have a negative scale factor
// so when calculating the normal we should take the vertices counter clockwise.
// If we don't compensate for this the objects will be 'inverted'.
BOOL Exporter::TMNegParity(Matrix3 &m)
{
	return (DotProd(CrossProd(m.GetRow(0),m.GetRow(1)),m.GetRow(2))<0.0)?1:0;
}

// Return a pointer to a TriObject given an INode or return NULL
// if the node cannot be converted to a TriObject
TriObject* Exporter::GetTriObjectFromNode(INode *node, TimeValue t, int &deleteIt)
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


// From the SDK
// How to calculate UV's for face mapped materials.
static Point3 basic_tva[3] = { 
	Point3(0.0,0.0,0.0),Point3(1.0,0.0,0.0),Point3(1.0,1.0,0.0)
};
static Point3 basic_tvb[3] = { 
	Point3(1.0,1.0,0.0),Point3(0.0,1.0,0.0),Point3(0.0,0.0,0.0)
};
static int nextpt[3] = {1,2,0};
static int prevpt[3] = {2,0,1};

void Exporter::make_face_uv(Face *f, Point3 *tv)
{
	int na,nhid,i;
	Point3 *basetv;
	/* make the invisible edge be 2->0 */
	nhid = 2;
	if (!(f->flags&EDGE_A))  nhid=0;
	else if (!(f->flags&EDGE_B)) nhid = 1;
	else if (!(f->flags&EDGE_C)) nhid = 2;
	na = 2-nhid;
	basetv = (f->v[prevpt[nhid]]<f->v[nhid]) ? basic_tva : basic_tvb; 
	for (i=0; i<3; i++) {  
		tv[i] = basetv[na];
		na = nextpt[na];
	}
}


/****************************************************************************

  String manipulation functions
  
****************************************************************************/

#define CTL_CHARS  31
#define SINGLE_QUOTE 39
// Replace some characters we don't care for.
TCHAR* Exporter::FixupName(TCHAR* name)
{
	static char buffer[256];
	TCHAR* cPtr;
	
    _tcscpy(buffer, name);
    cPtr = buffer;
	
    while(*cPtr) {
		if (*cPtr == '"')
			*cPtr = SINGLE_QUOTE;
        else if (*cPtr <= CTL_CHARS)
			*cPtr = _T('_');
        cPtr++;
    }
	
	return buffer;
}

/****************************************************************************

  Camera output
  
****************************************************************************/

void Exporter::ExportCameraObject(INode* node, int indentLevel)
{
	RSCameraObject *pCO=new RSCameraObject;
	pCO->name=new char[strlen(node->GetName())+1];
	strcpy(pCO->name,node->GetName());

	INode* target = node->GetTarget();
	ExportNodeTM(node, indentLevel,&pCO->tm);

	CameraState cs;
	TimeValue t = GetStaticFrame();
	Interval valid = FOREVER;
	// Get animation range
	Interval animRange = ip->GetAnimRange();
	
	ObjectState os = node->EvalWorldState(t);
	CameraObject *cam = (CameraObject *)os.obj;
	
	cam->EvalCameraState(t,valid,&cs);
	pCO->fov=cs.fov;

#define TARGETFPS	60

	TimeValue start = ip->GetAnimRange().Start();
	TimeValue end = ip->GetAnimRange().End();
	int delta = GetTicksPerFrame() * GetFrameRate() / TARGETFPS;
	Matrix3 tm;

	rsm->m_nCameraFrame=(end-start)/delta+1;
	Matrix3 *m=pCO->am=new Matrix3[rsm->m_nCameraFrame];
	for (t=start; t<=end; t+=delta) {
		tm = node->GetNodeTM(t) * Inverse(node->GetParentTM(t));
		*m=tm;
		m++;
	}
	
	rsm->m_CameraList.Add(pCO);

}