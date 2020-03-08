#ifndef _RSBSPEXPORTER_H
#define _RSBSPEXPORTER_H

#pragma warning( disable : 4002 )

#include <windows.h>
#include <stdio.h>
#include <math.h>

#include "max.h"
#include "export.h"

#include "cmlist.h"

#include "RTypes.h"
#include "RNameSpace.h"
#include "RLightList.h"
#include "RMaterialList.h"
#include "ROcclusionList.h"
#include "RNavigationMesh.h"

#include "dmath.h"

#include <list>
#include <vector>
#include <string>
#include <map>

//using namespace std;
_USING_NAMESPACE_REALSPACE2

struct MaxSubMaterial : public RMATERIAL {
	double	sina,cosa,tileu,tilev;
	int	RMLIndex;
};

class MaxMaterial {
public:
	int nSubMaterial;
	int *SubMaterials;
	MaxMaterial() {SubMaterials=NULL;}
	virtual ~MaxMaterial() {if(SubMaterials) delete []SubMaterials;};
};

class RSMSubMaterialList : public vector<MaxSubMaterial*>
{
public:
	int GetByPointer(MaxSubMaterial* pt)
	{
		for(size_t i=0;i<size();i++)
		{	if(at(i)==pt)return i;	}
		return -1;
	}
	int GetByName(const char*tname)
	{
		for(size_t i=0;i<size();i++)
		{	if(strcmp(at(i)->Name.c_str(),tname)==0) return i;	}
		return -1;
	}
	MaxSubMaterial* Get(int i) {
		return at(i);
	}
	void Add(MaxSubMaterial* pitem) {
		push_back(pitem);
	}
	size_t GetCount() { return size(); }
} ;

inline void operator +=(rvector &v,Point3 &p) {v.x+=-p.x;v.y+=p.y;v.z+=p.z;}

struct rvertex
{
	dpoint coord,normal;
	float u,v;
	float u2,v2;

	void Save(FILE *file) {
		coord.SaveFloat(file);
		normal.SaveFloat(file);
		fwrite(&u,sizeof(float),4,file);
	}
};

struct rpolygon
{
	rpolygon() { v=NULL;bPlaneUsed=false;dwFlags=0;nMaterial=0; }
	~rpolygon() { SAFE_DELETE(v); }

	rpolygon(int n) { nCount=n; v=new rvertex[n];bPlaneUsed=false;dwFlags=0;nMaterial=0; }
	rpolygon(rpolygon *pp) { memcpy(this,pp,sizeof(rpolygon));v=new rvertex[nCount];memcpy(v,pp->v,sizeof(rvertex)*nCount); }

	rvertex *GetVertex(int nIndex) { return &v[(nIndex+nCount)%nCount]; }
	dpoint *coord(int nIndex) { return &v[(nIndex+nCount)%nCount].coord; }

	dplane GetPlane() { return dplane(normal.x,normal.y,normal.z,d); }

	double GetArea() {
		double fArea=0;
		for(int i=0;i<nCount-2;i++)
			fArea+=::GetArea(v[0].coord,v[i+1].coord,v[i+2].coord);
		return fArea;
	}

	void dump();

	int		nCount;
	rvertex *v;
	int nMaterial,nSourceIndex;
	dpoint	normal;
	double	d;
	DWORD	dwFlags;
	bool	bPlaneUsed;
	int		nID;

	int		*vi;	// vertex index ( solid bsp 만들때 임시로 쓴다 )
//	dpoint *en;	// edge normal ( solid bsp 만들때 임시로 쓴다 )
};

typedef CMLinkedList<rvertex> RSVertexList;
typedef CMLinkedList<rpolygon> RSPolygonList;
typedef list<INode*> NODELIST;

struct RSPAWNPOSITION {
	string name;
	rvector position;
};

struct UserProp {
	enum VALUE_TYPE { STRING, INT, FLOAT	};
	string	key;
	void* pValue;
	VALUE_TYPE type;
	UserProp() { pValue = NULL; }
	~UserProp() { if (pValue!=NULL) delete pValue; }
};

#define MAX_USERPROP	10

struct RDUMMY {
	string name;
	rvector position;
	rvector direction;
	map<string, UserProp*>	UserPropMap;

	void SetUserPropValue(const char* szKey, float value)
	{
		UserProp* up = new UserProp;
		up->type = UserProp::FLOAT;
		up->pValue = new float;
		*((float*)up->pValue) = value;

		UserPropMap.insert(map<string, UserProp*>::value_type(string(szKey), up));
	}
	void SetUserPropValue(const char* szKey, int value)
	{
		UserProp* up = new UserProp;
		up->type = UserProp::INT;
		up->pValue = new int;
		*((int*)up->pValue) = value;

		UserPropMap.insert(map<string, UserProp*>::value_type(string(szKey), up));
	}
	void SetUserPropValue(const char* szKey, char* value)
	{
		UserProp* up = new UserProp;
		up->type = UserProp::STRING;
		up->pValue = new char[strlen(value)+2];
		strcpy((char*)up->pValue, value);

		UserPropMap.insert(map<string, UserProp*>::value_type(string(szKey), up));
	}
	bool GetUserPropValue(const char* szKey, float* out)
	{
		map<string, UserProp*>::iterator itor = UserPropMap.find(string(szKey));
		if (itor != UserPropMap.end())
		{
			UserProp* p = (*itor).second;
			if (p->type != UserProp::FLOAT)	return false;
			if (p->pValue == NULL) return false;
			*out = *((float*)(p->pValue));
			return true;
		}
		return false;
	}
	bool GetUserPropValue(const char* szKey, int* out)
	{
		map<string, UserProp*>::iterator itor = UserPropMap.find(string(szKey));
		if (itor != UserPropMap.end())
		{
			UserProp* p = (*itor).second;
			if (p->type != UserProp::INT)	return false;
			if (p->pValue == NULL) return false;
			*out = *((int*)(p->pValue));
			return true;
		}
		return false;
	}
	bool GetUserPropValue(const char* szKey, char* out, int nMaxLen)
	{
		map<string, UserProp*>::iterator itor = UserPropMap.find(string(szKey));
		if (itor != UserPropMap.end())
		{
			UserProp* p = (*itor).second;
			if (p->type != UserProp::STRING) return false;
			if (p->pValue == NULL) return false;
			if (nMaxLen < (int)strlen((char*)(p->pValue))+1) return false;
			strcpy(out, (char*)(p->pValue));
			return true;
		}
		return false;
	}
	~RDUMMY()
	{
		while (!UserPropMap.empty())
		{
			map<string, UserProp*>::iterator i = UserPropMap.begin();
			delete (*i).second;
			UserPropMap.erase(i);
		}
	}
};

rmatrix MatrixInverse(const rmatrix & m);

class RSBspNode
{
public:
	int			nFace;
	rpolygon	*Face;
	RSBspNode *Positive,*Negative;

	bool		bSolidNode;

	dplane plane;
	dboundingbox bbTree;

	bool Save(FILE *file);
	bool SaveCol(FILE *file);
	bool SaveNavigation(FILE* file);

	int	GetNodeCount();
	int GetPolygonCount();
	int GetVerticesCount();
	int GetIndicesCount();

	RSBspNode();
	virtual ~RSBspNode();
};

class RVertexList : public vector<dpoint*> {
public:
	virtual ~RVertexList() {
		while(size()) {
			delete *begin();
			erase(begin());
		}
	}
	dpoint *Get(int i) { return (*this)[(i+size())%size()]; }
};

/*
struct RConvexPolygon {
	dplane			plane;
	RVertexList		vertices;
	RVertexList		normals;
	int				nMaterial;
	list<int>		RefIDs;
	double			fArea;
//	vector<rpolygon*>	faces;		// 원본 폴리곤.. normal & uv 를 위해 보관한다..
	DWORD			dwFlags;
};
*/

// 평면과 폴리곤의 관계..
enum RSIDE {
	RSIDE_POSITIVE	=0,		// 앞쪽
	RSIDE_NEGATIVE	=1,		// 뒷쪽
	RSIDE_BOTH		=2,		// 걸친경우
	RSIDE_COPLANAR_POS	=3,		// 같은평면에 있는 경우 앞쪽
	RSIDE_COPLANAR_NEG	=4		// 같은평면에 있는 경우 뒷쪽
};

class RCSGObject {
public:
	RCSGObject();
	~RCSGObject();

	string			m_Name;
	RSPolygonList	*m_pPolygons;
	RSBspNode		*m_pTree;
};

// 네비게이션 메쉬
struct RSNavigationMesh
{
	int			nVertCount;
	int			nFaceCount;
	rvector*	vertices;
	RNavFace*	faces;

	RSNavigationMesh() {
		nVertCount = nFaceCount = 0;
		vertices = NULL;
		faces = NULL;
	}
	void Init(int vert_count, int face_count)
	{
		if (vertices != NULL) delete [] vertices;
		if (faces != NULL) delete [] faces;

		nVertCount = vert_count;
		nFaceCount = face_count;
		vertices = new rvector[vert_count];
		faces = new RNavFace[face_count];
	}
};


class RSBspExporter  
{
public:
	RSBspExporter();
	virtual ~RSBspExporter();

	Interface*	ip;

	MaxSubMaterial *thismat;

	RSMSubMaterialList			MaxSubMaterialList;
	CMLinkedList<MaxMaterial>	MaxMaterialList;
	
	rvector		m_AmbientLightColor;	
	NODELIST	m_ObjectsList;
	RLightList	m_StaticLightList;
	ROcclusionList m_OcclusionList;
	map<int,dplane>	m_PartitionPlanes;
	vector<dplane>	m_PartitionPlaneVector;
	list<RCSGObject*>	m_CSGObjects;
//	list<RCSGObject*>	m_NavObjects;

	//list<RSPAWNPOSITION*>	m_SpawnPositionList;
	list<RDUMMY*>			m_DummyList;

	int			nMesh;

	void Preprocessing();				// 세이브하기 전에 해야 할것들..

	bool SaveRS(const char* name);
	bool SaveBSP(const char* name);
	bool SaveDesc(const char* name);
	bool SaveCol(const char* name);
	bool SaveSpawn(const char* name);
	bool SaveFlag( const char* name );
	bool SaveSmoke(const char* name);
	bool SaveNavigation(const char* name);


	bool SaveSoundProp(MXmlElement& Root);
	bool SaveFog(MXmlElement& Root);
	bool SaveCustomProperties(MXmlElement& Root);


	char	MapName[256];
	char	MaxFileName[256];

	RSPolygonList face,faceOc,faceCol;
	RSPolygonList faceNavigation;

	// navigation 용
	RSNavigationMesh		m_Navigation;


	dboundingbox m_bb;					// 전체의 바운딩박스

private:
	// csgunion 을 위해 필요한 펑션들
	void CutPolygon(rpolygon *ppolygon);
	void CheckPolygonInSolid(rpolygon *ppolygon,RSBspNode *pNode);
	bool CSGUnion(RSPolygonList *pOutputPolygons, RSPolygonList* pOutputPolygonList, list<RCSGObject*>& Objects);

	bool SaveSource(FILE *file);		// bsp 생성 전의 mesh 를 저장해둔다.
	bool ConstructBspTree();
	void ConstructBoundingBox();	// 말단 노드서부터 계산하여 root까지 올라감
	bool Save(FILE *file);

	RSBspNode *ConstructOctree(RSPolygonList *FaceList,int depth,dboundingbox *bb);
	RSBspNode *ConstructBspTree(RSPolygonList *FaceList);
	RSBspNode *ConstructSolidBspTree(RSPolygonList *FaceList);
	RSBspNode *ConstructBevelSolidBspTree(RSPolygonList *FaceList,RSPolygonList *SpaceFaceList);
	
	bool ChoosePlane(RSPolygonList *,dplane *);
	bool ChoosePlaneSolid(RSPolygonList *,dplane *);
	void Partition(dplane *,RSPolygonList *FaceList,RSPolygonList *NegList,RSPolygonList *PosList);
	RSIDE whichSideIsFaceWRTplane(rpolygon *,dplane *);

	void ConstructBoundingBox(RSBspNode *bspNode);

	RSBspNode *GetLeafNode(dpoint *pos,RSBspNode *node);
	inline RSBspNode *GetLeafNode(dpoint *pos) { return GetLeafNode(pos,BspHead); }

	/*
	class RConvexPolygons : public vector<RConvexPolygon*> {
	public:
		void MergePolygons(int nStartIndex);
	} m_ConvexPolygons;

	void ConstructConvexPolygon();
	*/
	void MergePolygons(RSPolygonList *pSourcePolygons,int nStartIndex);
	void MergePolygons(RSPolygonList *pSourcePolygons,RSPolygonList *pOutputPolygons);

	int	m_nDepth;

	RSBspNode *BspHead,*OcHead,*ColBspRoot;
//	RSBspNode *m_pNavBspRoot;

	rvector ViewPoint;

};

#define FLOAT2RGB24(r, g, b) ( ( ((long)((r) * 255)) << 16) | \
								(((long)((g) * 255)) << 8) | (long)((b) * 255))

// TOLER 에 비해 지나치게 큰 맵을 처리하다보면 무한루프에 빠질수있씀 ㅜ.ㅡ
#define BSPTOLER	0.01
#define BSPEQ(a,b)	((fabs(double(a)-(b)) >= BSPTOLER) ? 0 : 1)
#define BSPEQ3(a,b)	(BSPEQ((a).x,(b).x) && BSPEQ((a).y,(b).y) && BSPEQ((a).z,(b).z) )

#define signof(f) (((f) < -BSPTOLER) ? NEGATIVE : ((f) > BSPTOLER ? POSITIVE : ZERO))
#define RANDOMFLOAT ((float)rand()/(float)RAND_MAX)

#endif // !defined(AFX_RSBspExporter_H__A7395ED0_81F3_11D2_9B5F_00AA006E4A4E__INCLUDED_)
