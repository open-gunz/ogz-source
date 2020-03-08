// RSMObject.h: interface for the RSMObject class.

#ifndef __RSMOBJECT_H
#define __RSMOBJECT_H

#include "windows.h"
#include "stdio.h"
#include "export.h"
#include "math.h"
#include "max.h"
#include "cmlist.h"
#include "cmptrlist.h"
#include "RSMVersion.h"
#include "AnimConstructor.h"
#include "rutils_max.h"
#include "RSMaterialList.h"

typedef struct _RSMHEADER {
	int			RSMID;
	int			Build;
	FILETIME	MaterialTimeStamp;
} RSMHEADER;

class MaxStdMaterial {
public:
	char name[256],DiMapName[256],OpMapName[256],RMLName[256];
	rvector Ambient,Diffuse,Specular;
	BOOL	ColorKeyEnable,TwoSide;
	RSSHADEMODE ShadeMode;
	DWORD	 TextureHandle;
	double	sina,cosa;
	double	uOffset,vOffset,uTiling,vTiling;

	int	RMLIndex;

	MaxStdMaterial();
	virtual ~MaxStdMaterial();
};

class MaxMaterial {
public:
	int nSubMaterial;
	int *SubMaterials;
	MaxMaterial **pSubMaterials;

	MaxMaterial();
	virtual ~MaxMaterial();
};

struct rface {
	union{
		struct { WORD a,b,c; };
		WORD ver[3];
	};
	float u[3],v[3];
	int nMaterial;
	rvector normal,vnormals[3];
};

class RSMFaces {
public:
	int nMaterial;
	int nFace;
	rface *face;
	WORD *indicies;
	WORD *indicies_original;
	
	class RSVerList
	{
	public:
		RSVerList() { m_Vertices=NULL;nV=0; }
		virtual ~RSVerList() { if(m_Vertices) delete m_Vertices; }

		void Reset(int n) { if(m_Vertices) delete m_Vertices;m_Vertices=new rvertex[n];nV=0; }
		WORD Insert(rvertex* pVer);
		int GetCount() { return nV; }
		rvertex *Get(int i) { return m_Vertices+i; }

		rvertex *m_Vertices;
		int nV;
	}verlist;

	RSMFaces();
	virtual ~RSMFaces();
};

class RSMMesh {
public:
	RSMMesh();
	virtual ~RSMMesh();

	char		name[256];
	rvertex		*ver;
	rvector		*trv;
	rface		*face;
	int			nV,nF,refMaterial;
	rmatrix		mat;
	CMLinkedList <RSMFaces> faceslist;
	rboundingbox	m_bbox;
};

class RSCameraObject {
public:
	RSCameraObject();
	virtual ~RSCameraObject();

	char *name;
	rmatrix tm;
	Matrix3 *am;
	rvector *pos;
	float	fov;
};

class RSMObject {
public:
	RSMObject();
	virtual ~RSMObject();

	void Optimize(bool bLoose=false);
	bool SaveRSM(const char* name);
	bool SaveRSM(FILE *file);
	bool SaveRML(const char* name);
	bool SaveRML(FILE *file);
	bool SaveCameraInfo(FILE *file);

	bool AddMaterials(RSMaterialList *pTargetML);
	bool AddAnimation(AnimConstructor *pAnim);

	class RSMStdMaterialList : public CMLinkedList<MaxStdMaterial>
	{
	public:
		int GetByPointer(MaxStdMaterial* pt)
		{
			for(int i=0;i<GetCount();i++)
			{	if(Get(i)==pt)return i;	}
			return -1;
		}
		int GetByName(char*tname)
		{
			for(int i=0;i<GetCount();i++)
			{	if(strcmp(Get(i)->name,tname)==0) return i;	}
			return -1;
		}
	} MaxStdMaterialList;
	
	class RSMMeshList : public CMLinkedList<RSMMesh>
	{
	public:
	int Compare(RSMMesh *pRecord1,RSMMesh *pRecord2);
	} MeshList;

	CMLinkedList<MaxMaterial>	MaxMaterialList;
	CMLinkedList<RSCameraObject>	m_CameraList;
	int	m_nCameraFrame;

	RSMMesh			*mesh;

	char	MaxFileName[256];

private:
	RSMHEADER		m_Header;
	rboundingbox	m_BoundingBox;
	bool		m_bLoose;

	class BEAnimationList : public CMLinkedList<AnimConstructor>
	{
	public:
		bool Save(FILE *stream);
	} AnimationList;

	void CalcNormal(rvertex *verbase,rface *face);
	void OptimizeMesh(RSMMesh *pMesh);
	void OptimizeFaces(RSMMesh *pMesh,RSMFaces *faces);
	void EliminateInvalid();
	void WriteString(char*,FILE*);
	void TransformUV(float &u,float &v,MaxStdMaterial *m);
	bool SaveShadowTexture(FILE *stream);
};

#endif