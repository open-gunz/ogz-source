// RSMObject.cpp: implementation of the RSMObject class.
//
//////////////////////////////////////////////////////////////////////

#include "RSMObject.h"
#include "stdio.h"
#include "RSMaterialList.h"
#include "Dib.h"
#include "Image24.h"
#include "FileInfo.h"
#include "BELog.h"
#include "rtexture.h"
#include "rutils_max.h"

MaxStdMaterial::MaxStdMaterial() 
{
	name[0]=0;
	DiMapName[0]=0;
	OpMapName[0]=0;
	RMLName[0]=0;
}

MaxStdMaterial::~MaxStdMaterial(){}

MaxMaterial::MaxMaterial()
{
	pSubMaterials=NULL;
	SubMaterials=NULL;
	nSubMaterial=0;
}

MaxMaterial::~MaxMaterial()
{
	if(SubMaterials) delete []SubMaterials;
	if(pSubMaterials) 
	{
		for(int i=0;i<nSubMaterial;i++)
			delete pSubMaterials[i];
		delete pSubMaterials;
	}
}

RSCameraObject::RSCameraObject()
{
	name=NULL;
	am=NULL;
}

RSCameraObject::~RSCameraObject()
{
	if(name) delete name;
	if(am) delete am;
}


RSMObject::RSMObject()
{
	mesh=NULL;
}

RSMObject::~RSMObject()
{
}

RSMFaces::RSMFaces()
{
	nFace=0;
	face=NULL;
	indicies=NULL;
	indicies_original=NULL;
}

RSMFaces::~RSMFaces()
{ 
	if(face) delete []face;
	if(indicies) delete []indicies;
	if(indicies_original) delete []indicies_original;
}

RSMMesh::RSMMesh()
{
	ver=NULL;
	face=NULL;
}

RSMMesh::~RSMMesh()
{
	if(ver) delete []ver;
	if(face) delete []face;
}

RSMObject::RSMMeshList::Compare(RSMMesh *pRecord1,RSMMesh *pRecord2)
{
	int nReturn=strcmp(pRecord1->name,pRecord2->name);
	if(nReturn>0)
		return 1;
	else if (nReturn==0)
		return 0;
	else return -1;
}

#define WriteInt(x) fwrite(&(x),sizeof(int),1,stream)
#define WriteBool(x) fwrite(&(x),sizeof(BOOL),1,stream)
#define WriteWORD(x) fwrite(&(x),sizeof(WORD),1,stream)
#define WriteFloat(x) fwrite(&(x),sizeof(float),1,stream)
#define WriteVector(x) fwrite(&(x),sizeof(rvector),1,stream)
#define WriteMatrix(m) 	{WriteFloat(m._11);WriteFloat(m._12);WriteFloat(m._13);WriteFloat(m._14);\
	WriteFloat(m._21);WriteFloat(m._22);WriteFloat(m._23);WriteFloat(m._24);\
	WriteFloat(m._31);WriteFloat(m._32);WriteFloat(m._33);WriteFloat(m._34);\
	WriteFloat(m._41);WriteFloat(m._42);WriteFloat(m._43);WriteFloat(m._44);}

void RSMObject::WriteString(char* x,FILE* stream)
{
	BYTE n=strlen(x);
	fwrite(&n,1,1,stream);
	fwrite(x,n,1,stream);
}

void RSMObject::TransformUV(float &u,float &v,MaxStdMaterial *m)
{
	float ou=u,ov=v;
	u=float(ou*m->cosa+ov*m->sina+m->uOffset)*m->uTiling;
	v=float(-ou*m->sina+ov*m->cosa+m->vOffset)*m->vTiling;
}

void RSMObject::EliminateInvalid()
{
	int i,j,m;
	int nEV=0,nEF=0;

	bool *pRef=NULL;
	for(m=0;m<MeshList.GetCount();m++)
	{
		mesh=MeshList.Get(m);

		if(pRef) delete []pRef;
		pRef=new bool[mesh->nV];

		for(i=0;i<mesh->nV;i++)
		{
			pRef[i]=false;
		}

		rface *face=mesh->face;
		for(i=0;i<mesh->nF;i++)
		{
			face=&mesh->face[i];
			if(IS_EQ3(mesh->ver[face->a].coord,mesh->ver[face->b].coord) ||
				IS_EQ3(mesh->ver[face->c].coord,mesh->ver[face->b].coord) ||
				IS_EQ3(mesh->ver[face->a].coord,mesh->ver[face->c].coord))
			{
				memcpy(&mesh->face[i],&mesh->face[mesh->nF-1],sizeof(rface));
				mesh->nF--;
				i--;
				nEF++;
			}
			else
			{
				pRef[face->a]=true;
				pRef[face->b]=true;
				pRef[face->c]=true;
			}
		}

		for(i=0;i<mesh->nV;i++)
		{
			if(!pRef[i])
			{
				for(j=0;j<mesh->nF;j++)		// rearrange vertex index;
				{
					face=&mesh->face[j];
					_ASSERT(face->a!=i);
					_ASSERT(face->b!=i);
					_ASSERT(face->c!=i);
				}
				memcpy(&mesh->ver[i],&mesh->ver[mesh->nV-1],sizeof(rvertex));
				pRef[i]=pRef[mesh->nV-1];
				for(j=0;j<mesh->nF;j++)		// rearrange vertex index;
				{
					face=&mesh->face[j];
					if(face->a==mesh->nV-1) face->a=i;
					if(face->b==mesh->nV-1) face->b=i;
					if(face->c==mesh->nV-1) face->c=i;
				}
				i--;
				nEV++;
				mesh->nV--;
			}
		}
	}
	if(pRef) delete []pRef;
	log("   %d vertices %d faces eliminated.\n",nEV,nEF);
}

void RSMObject::Optimize(bool bLoose)
{
	m_bLoose=bLoose;
	if(!bLoose) 
		EliminateInvalid();

// prework for save... calc vertex normal and find minmax

	m_BoundingBox.Reset();

	int i,j,m;
	for(m=0;m<MeshList.GetCount();m++)
	{
//		log("Processing mesh : %s .\n",mesh->name);
		mesh=MeshList.Get(m);
		char header[4];					// for filtering Bipxxxx named object from character studio.
		header[3]=0;
		memcpy(header,mesh->name,3);
		if( (mesh->nV==0) || (mesh->nF==0) ||			// zero vertex or zero face mesh
			(strcmp(header,"Bip")==0)) // it will be deleted. cause it is biped
		{
			log("     Object : %s Deleted. \n",mesh->name);
			MeshList.Delete(m);
			m--;
		}
		else
		{
			mesh->m_bbox.Reset();

			for(i=0;i<mesh->nV;i++)
			{
				for(j=0;j<3;j++)
				{
					m_BoundingBox.m[j][0]=min(m_BoundingBox.m[j][0],mesh->ver[i].coord.v[j]);
					m_BoundingBox.m[j][1]=max(m_BoundingBox.m[j][1],mesh->ver[i].coord.v[j]);

					mesh->m_bbox.m[j][0]=min(mesh->m_bbox.m[j][0],mesh->ver[i].coord.v[j]);
					mesh->m_bbox.m[j][1]=max(mesh->m_bbox.m[j][1],mesh->ver[i].coord.v[j]);
				}
			}
			OptimizeMesh(mesh);
		}
	}
	MeshList.Sort();
}

bool RSMObject::SaveRSM(FILE *stream)
{
	int i,j,m;

	m_Header.RSMID=HEADER_RSMID;
	m_Header.Build=HEADER_BUILD;
	fwrite(&m_Header,sizeof(m_Header),1,stream);
	fwrite(&m_BoundingBox,sizeof(rboundingbox),1,stream);

	int nMaterial=MaxStdMaterialList.GetCount();

	WriteInt(nMaterial);
	for(i=0;i<nMaterial;i++)
	{
		MaxStdMaterial *material=MaxStdMaterialList.Get(i);
		WriteString(material->name,stream);
	}
	
	int nMesh=MeshList.GetCount();
	WriteInt(nMesh);

	for(m=0;m<MeshList.GetCount();m++)
	{
		mesh=MeshList.Get(m);

		WriteString(mesh->name,stream);
		WriteMatrix(mesh->mat);
		fwrite(&mesh->m_bbox,sizeof(rboundingbox),1,stream);

		int nFaces=mesh->faceslist.GetCount();
		WriteInt(nFaces);
		for(i=0;i<nFaces;i++)
		{
			RSMFaces *faces=mesh->faceslist.Get(i);
			WriteInt(faces->nMaterial);
			
			int nVer=faces->verlist.GetCount();
			WriteInt(nVer);
			for(j=0;j<nVer;j++)
			{
				rvertex *v=faces->verlist.Get(j);
				fwrite(v,sizeof(rvertex),1,stream);				
			}
			if(m_bLoose)
			{
				fputc(1,stream);
				fwrite(faces->indicies_original,sizeof(WORD),nVer,stream);
			}
			else
			{
				fputc(0,stream);
				int nInd= faces->nFace*3;
				WriteInt(nInd);
				fwrite(faces->indicies,sizeof(WORD),nInd,stream);
			}
		}
//		log("%s\n",mesh->name);
	}
	SaveShadowTexture(stream);
	AnimationList.Save(stream);
	return true;
}

bool RSMObject::SaveRSM(const char *name)
{
	FILE *stream=fopen(name,"wb+");
	if(!stream) return 0;
	SaveRSM(stream);
	fclose(stream);
	return true;	
}

bool RSMObject::AddMaterials(RSMaterialList *pTargetML)
{
	char MaxFilePath[MAX_PATH];
	{
		char drive[_MAX_DRIVE],dir[_MAX_DIR],fname[_MAX_FNAME],ext[_MAX_EXT];
		_splitpath(MaxFileName,drive,dir,fname,ext);
		sprintf(MaxFilePath,"%s%s",drive,dir);
	}

	RSMaterialList *ml=new RSMaterialList;
	int nMaterial=MaxStdMaterialList.GetCount();
	for(int i=0;i<nMaterial;i++)
	{
		MaxStdMaterial *material=MaxStdMaterialList.Get(i);
		RSMaterial *m=new RSMaterial;

		m->Name=new char[strlen(material->name)+1];
		strcpy(m->Name,material->name);

#define ADJUSTRANGE(x) x=max(min(x,1.f),0.f)
		ADJUSTRANGE(material->Ambient.x);
		ADJUSTRANGE(material->Ambient.y);
		ADJUSTRANGE(material->Ambient.z);
		ADJUSTRANGE(material->Diffuse.x);
		ADJUSTRANGE(material->Diffuse.y);
		ADJUSTRANGE(material->Diffuse.z);
		m->Ambient=(DWORD(material->Ambient.x*255))<<16|
					(DWORD(material->Ambient.y*255))<<8|
					(DWORD(material->Ambient.z*255));
		m->Diffuse=(DWORD(material->Diffuse.x*255))<<16|
					(DWORD(material->Diffuse.y*255))<<8|
					(DWORD(material->Diffuse.z*255));
		m->b2Sided=material->TwoSide;
		m->ShadeMode=material->ShadeMode;

// adding texture to texture list
		if(material->DiMapName[0])
		{
			int index=-1;
			char drive[_MAX_DRIVE],dir[_MAX_DIR],fname[_MAX_FNAME],ext[_MAX_EXT];
			_splitpath(material->DiMapName,drive,dir,fname,ext);

			index=ml->GetTextureIndex(fname);
			if(index==-1)		// if not exist
			{
				char bmpname[256];
				ReplaceExtension(bmpname,material->DiMapName,"bmp");
				if(!IsExist(bmpname))
					sprintf(bmpname,"%s%s.bmp",MaxFilePath,fname);

				RSTexture *tex=new RSTexture;
				if(tex->CreateFromBMP(bmpname))
				{
						index=ml->AddTexture(tex);
						m->nTexture=1;
						m->TextureIndices=new int[1];
						m->TextureIndices[0]=index;
				}
				else
				{
					delete tex;
					log("     Error : Cannot Load BMP file : %s\n",bmpname);
				}
			}
			else
			{
				m->nTexture=1;
				m->TextureIndices=new int[1];
				m->TextureIndices[0]=index;
			}
		}

// adding opacity map texture to texture list
		if(material->OpMapName[0])
		{
			int index=-1;
			char drive[_MAX_DRIVE],dir[_MAX_DIR],fname[_MAX_FNAME],ext[_MAX_EXT];
			_splitpath(material->OpMapName,drive,dir,fname,ext);

			index=ml->GetTextureIndex(fname);
			if(index==-1)		// if not exist
			{
				char bmpname[256];
				ReplaceExtension(bmpname,material->OpMapName,"bmp");
				if(!IsExist(bmpname))
				{
					char fname2[_MAX_FNAME];
					_splitpath(material->DiMapName,drive,dir,fname2,ext);
					sprintf(bmpname,"%s%s%s.bmp",drive,dir,fname);
				}
				if(!IsExist(bmpname))
					sprintf(bmpname,"%s%s.bmp",MaxFilePath,fname);

				RSTexture *tex=new RSTexture;
				if(tex->CreateFromBMP(bmpname))
						index=ml->AddTexture(tex);
				else
				{
					delete tex;
					log("     Error : Cannot Load BMP file : %s\n",material->OpMapName);
				}
			}
		}

		strcpy(material->RMLName,material->name);
		ml->Add(m);
	}
	pTargetML->AddLibrary(ml);
	delete ml;
	return true;
}

bool RSMObject::SaveRML(const char *name)
{
	RSMaterialList *ml=new RSMaterialList;
	bool bReturn=AddMaterials(ml);
	if(!ml->Save(name))
	{
		log("     Error while saving %s\n",name);
		return false;
	}
	delete ml;
	return true;
}

bool RSMObject::SaveRML(FILE *stream)
{
	RSMaterialList *ml=new RSMaterialList;
	bool bReturn=AddMaterials(ml);
	if(!ml->Save(stream))
		return false;
	delete ml;
	return true;
}

bool RSMObject::AddAnimation(AnimConstructor *pAnim)
{
	int imesh;
	ACAnimNodeList newlist;
	ACAnimNodeList::iterator i;
	for(imesh=0;imesh<MeshList.GetCount();imesh++)
	{
		char *name1=MeshList.Get(imesh)->name;
		i=pAnim->objlist.Get(name1);
		if(i==NULL)
		{
			newlist.erase(newlist.begin(),newlist.end());
			return false;
		}
		newlist.push_back(*i);
	}
	pAnim->objlist.erase(pAnim->objlist.begin(),pAnim->objlist.end());
	for(i=newlist.begin();i!=newlist.end();i++)
		pAnim->objlist.push_back(*i);
	newlist.erase(newlist.begin(),newlist.end());

	AnimationList.Add(pAnim);
	return true;
}

bool RSMObject::BEAnimationList::Save(FILE *stream)
{
	int i;
	int nAnimCount=GetCount();
	WriteInt(nAnimCount);
	for(i=0;i<nAnimCount;i++)
		Get(i)->Save(stream);
	return true;
}

void RSMObject::OptimizeMesh(RSMMesh *pMesh)
{
	int i,j;

	rface **temp=new rface*[mesh->nF];

	for(i=0;i<mesh->nF;i++)
		temp[i]=mesh->face+i;

	for(i=0;i<pMesh->nF-1;i++)			// sort faces with #submaterial
	{
		for(j=i+1;j<pMesh->nF;j++)
		{
			if(temp[i]->nMaterial>temp[j]->nMaterial)
			{
				rface *t;
				t=temp[i];
				temp[i]=temp[j];
				temp[j]=t;
			}
		}
	}

	MaxMaterial *mat=(mesh->refMaterial==-1)?NULL:MaxMaterialList.Get(mesh->refMaterial);
	rface *face=mesh->face;
	for(i=0;i<mesh->nF;i++)
	{
		face->nMaterial= !mat ? -1 :
							(mat->nSubMaterial==0) ? -1 :					// 없으면 NULL
							(mat->nSubMaterial==1) ? mat->SubMaterials[0] :					// standard material 이면 
							(face->nMaterial<mat->nSubMaterial) ? mat->SubMaterials[face->nMaterial] : -1;	// multi-sub
		if(face->nMaterial!=-1)
		{
			MaxStdMaterial *pmat=MaxStdMaterialList.Get(face->nMaterial);
			TransformUV(face->u[0],face->v[0],pmat);
			TransformUV(face->u[1],face->v[1],pmat);
			TransformUV(face->u[2],face->v[2],pmat);
		}
		face++;
	}

	int laststart=0;
	for(i=1;i<=mesh->nF;i++)
	{
		if((i==mesh->nF)||(temp[i]->nMaterial!=temp[i-1]->nMaterial))
		{
			// generate RSMFaces
			RSMFaces *pFaces=new RSMFaces;
			int nCount=i-laststart;
			pFaces->face=new rface[nCount];
			for(j=0;j<nCount;j++)
			{
				memcpy(&pFaces->face[j],temp[j+laststart],sizeof(rface));
			}
			pFaces->nFace=nCount;
			pFaces->nMaterial=temp[laststart]->nMaterial;
			pMesh->faceslist.Add(pFaces);
			OptimizeFaces(pMesh,pFaces);
			laststart=i;
		}
	}

	delete temp;
}

WORD RSMFaces::RSVerList::Insert(rvertex *pVer)
{
	WORD i;
	for(i=0;i<GetCount();i++)
	{
		if(pVer->isEqual(m_Vertices[i]))
			return i;
	}
	memcpy(&m_Vertices[nV],pVer,sizeof(rvertex));nV++;
	return nV-1;
}

void RSMObject::OptimizeFaces(RSMMesh *pMesh,RSMFaces *pFaces)
{
	int i,j;
	rface *f=pFaces->face;
	rvertex v;
	if(m_bLoose)		// for vertex animation
	{
		WORD *index=pFaces->indicies_original=new WORD[pFaces->nFace*3];
		pFaces->verlist.Reset(pFaces->nFace*3);
		pFaces->verlist.nV=pFaces->nFace*3;
		for(i=0;i<pFaces->nFace;i++)
		{
			for(j=0;j<3;j++)
			{
				v.coord=pMesh->ver[f->ver[j]].coord;
				v.normal=f->vnormals[j];
				v.u=f->u[j];
				v.v=f->v[j];
				memcpy(pFaces->verlist.Get(i*3+j),&v,sizeof(rvertex));
				pFaces->indicies_original[i*3+j]=f->ver[j];
			}
			f++;
		}
	}
	else
	{
		WORD *index=pFaces->indicies=new WORD[pFaces->nFace*3];
		pFaces->verlist.Reset(pFaces->nFace*3);
		for(i=0;i<pFaces->nFace;i++)
		{
			for(j=0;j<3;j++)
			{
				v.coord=pMesh->ver[f->ver[j]].coord;
				v.normal=f->vnormals[j];
				v.u=f->u[j];
				v.v=f->v[j];
				*index=pFaces->verlist.Insert(&v);
				index++;
			}
			f++;
		}
	}
}

void RSMObject::CalcNormal(rvertex *verbase,rface *face)
{
	rvector *c1,*c2,*c3;
	c1=(rvector*)(&verbase[face->a].coord);
	c2=(rvector*)(&verbase[face->b].coord);
	c3=(rvector*)(&verbase[face->c].coord);
	face->normal=CrossProduct(*c1-*c2,*c1-*c3);
	face->normal.Normalize();
}

bool RSMObject::SaveShadowTexture(FILE *stream)
{
/*
	if(m_bLoose)
	{
		rtexture rt;
		rt.New(16,16,RTEXTUREFORMAT_24);
		rt.Save(stream);
		return true;
	}
*/
#define EFFECTIVE_SIZE	0.99f
#define SHADOW_COLOR	0xffffff

	rtexture rt;
	rt.New(256,256,RTEXTUREFORMAT_24);

	rmatrix tm;
	tm=ViewMatrix44(rvector(0,0,0),rvector(0,0,-1),rvector(1,0,0));

	rt.Fill(0);

	int i,j,k;
	float maxx=0,maxy=0;

	// 적당한 스케일 값을 얻기위해 최대 최소를 찾는다.
	for(i=0;i<MeshList.GetCount();i++)
	{
		RSMMesh *mesh=MeshList.Get(i);
		mesh->trv=new rvector[mesh->nV];

		for(j=0;j<mesh->nV;j++)
		{
			mesh->trv[j]=TransformVector(mesh->ver[j].coord,tm);
			maxx=max((float)fabs(mesh->trv[j].x),maxx);
			maxy=max((float)fabs(mesh->trv[j].y),maxy);
		}
	}


	// 변환행렬을 세팅하고.

	rmatrix invx=IdentityMatrix44();invx._11=-1;

	tm=invx*ScaleMatrix44(128.0f*EFFECTIVE_SIZE/max(maxx,maxy))
		*ViewMatrix44(rvector(0,0,0),rvector(0,0,-1),rvector(0,1,0))
		*TranslateMatrix44(128,128,0);

	for(i=0;i<MeshList.GetCount();i++)
	{
		RSMMesh *mesh=MeshList.Get(i);
		for(j=0;j<mesh->nV;j++)
		{
			mesh->trv[j]=TransformVector(mesh->ver[j].coord,tm);
		}
		for(j=0;j<mesh->faceslist.GetCount();j++)
		{
			RSMFaces *faces=mesh->faceslist.Get(j);
			if(faces->nMaterial!=-1 && 
				MaxStdMaterialList.Get(faces->nMaterial)->ShadeMode==RSSHADEMODE_NORMAL)
			{
				if(m_bLoose)
				{
					for(k=0;k<faces->nFace;k++)
					{
						rt.FillTriangle(
							mesh->trv[faces->indicies_original[k*3]].x,mesh->trv[faces->indicies_original[k*3]].y,
							mesh->trv[faces->indicies_original[k*3+1]].x,mesh->trv[faces->indicies_original[k*3+1]].y,
							mesh->trv[faces->indicies_original[k*3+2]].x,mesh->trv[faces->indicies_original[k*3+2]].y,
							SHADOW_COLOR);
					}
				}
				else
				{
					for(k=0;k<faces->nFace;k++)
					{
						rface *f=&faces->face[k];
						rt.FillTriangle(
							mesh->trv[f->ver[0]].x,mesh->trv[f->ver[0]].y,
							mesh->trv[f->ver[1]].x,mesh->trv[f->ver[1]].y,
							mesh->trv[f->ver[2]].x,mesh->trv[f->ver[2]].y,
							SHADOW_COLOR);
					}
				}
			}
		}
		delete mesh->trv;
	}

//	rt.SaveAsBMP("testtest.bmp");
	rtexture newrt1;newrt1.CreateAsHalf(&rt);		// 128
	rtexture newrt2;newrt2.CreateAsHalf(&newrt1);	// 64

	newrt2.FillBoundary(0);
	return newrt2.Save(stream);
}

bool RSMObject::SaveCameraInfo(FILE *file)
{
	int i,j;
	int nCamera=m_CameraList.GetCount();
	fwrite(&nCamera,sizeof(int),1,file);
	fwrite(&m_nCameraFrame,sizeof(int),1,file);
	for(i=0;i<nCamera;i++)
	{
		RSCameraObject *pCO=m_CameraList.Get(i);
		WriteString(pCO->name,file);
		fwrite(&pCO->fov,sizeof(float),1,file);
		for(j=0;j<m_nCameraFrame;j++)
		{
			rvector v;
			v=-pCO->am[j].GetRow(2);
			fwrite(&v,sizeof(rvector),1,file);	// dir
			v=pCO->am[j].GetRow(1);
			fwrite(&v,sizeof(rvector),1,file);	// up
			v=pCO->am[j].GetRow(3);
			fwrite(&v,sizeof(rvector),1,file);	// position
		}
	}
	return true;
}
