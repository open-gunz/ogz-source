#pragma once

#include "RMeshNodeData.h"
#include "RShaderMgr.h"

_NAMESPACE_REALSPACE2_BEGIN

//#define USE_TOON_RENDER
//#define SCALE_RMESHNODE

class CIndexBufferMake;

enum CalcVertexBufferBboxMode
{
	CalcVertexBufferBboxMode_None = 0,

	CalcVertexBufferBboxMode_VertexAni,
	CalcVertexBufferBboxMode_Physique,
	CalcVertexBufferBboxMode_TM_MapObject,
	CalcVertexBufferBboxMode_TM_Object,

	CalcVertexBufferBboxMode_End,
};

class RBoneBaseMatrix
{
public:
	rmatrix	mat = GetIdentityMatrix();
	int	id = -1;
};

#define MAX_MATRIX		60
#define MAX_PRIMITIVE	10

class RMeshNodeInfo
{
public:
	RMeshNodeInfo();

public:
	bool	m_isAddMeshNode;
	bool	m_isCollisionMesh;
	bool	m_isDummyMesh;
	bool	m_isWeaponMesh;
	bool	m_isDummy;
	bool	m_isAlphaMtrl;
	bool	m_isLastModel;

	bool	m_isClothMeshNode;
	bool	m_bClothMeshNodeSkip;
	float	m_vis_alpha;

	int						m_nAlign;

	CutParts				m_CutPartsType;
	LookAtParts				m_LookAtParts;
	WeaponDummyType			m_WeaponDummyType;

	RMeshPartsType			m_PartsType;
	RMeshPartsPosInfoType	m_PartsPosInfoType;

	bool			m_bNpcWeaponMeshNode;
	float			m_AlphaSortValue;

};

class RBatch
{
public:
	RBatch();
	~RBatch();

	bool CreateVertexBuffer(char* pVert, DWORD fvf, int vertexsize, int vert_num, DWORD flag);
	bool UpdateVertexBuffer(char* pVert);
	bool UpdateVertexBufferSoft(char* pVert);

	bool CreateIndexBuffer(int index, WORD* pIndex, int _size);

public:

	RIndexBuffer*		m_ib[MAX_PRIMITIVE];

	RVertexBuffer*		m_vb;
	RVertexBuffer*		m_vsb;

	bool				m_bIsMakeVertexBuffer;

};

class RMeshNode : public RMeshNodeData, public RMeshNodeMtrl, public RMeshNodeInfo, public RBatch
{
public:
	RMeshNode();
	~RMeshNode();

	void ConnectToNameID();
	bool ConnectMtrl();

	void UpdateNodeBuffer();
	void MakeNodeBuffer(DWORD flag);

	void MakeVertexBuffer(int index, bool lvert, char* pBuf, int _vsize, DWORD flag);
	bool MakeVSVertexBuffer();

	void RenderNodeVS(RMesh* pMesh, const rmatrix& world_mat, ESHADER shader_ = SHADER_SKIN);

	void Render(rmatrix* pWorldMatrix = NULL);

	RBoneBaseMatrix* GetBaseMatrix(int pid);

	bool isSoftRender();

	void CalcVertexBuffer(const rmatrix& world_mat, bool box = false);
	bool CalcPickVertexBuffer(const rmatrix& world_mat, std::vector<rvector>& OutVecs);
	int	 CalcVertexBuffer_VertexAni(int frame);
	void CalcVertexBuffer_Physique(const rmatrix& world_mat, int frame);
	void CalcVertexBuffer_Tm(const rmatrix& world_mat, int frame);
	void CalcVertexBuffer_Bbox(CalcVertexBufferBboxMode nBboxMode, rmatrix& mat);

	void CalcVertexNormal(rmatrix* world_mat);

	void CheckAlign(const rmatrix& worldmat);
	void CheckAlignMapObject(rmatrix& hr_mat);

	float GetNodeVisValue();
	int   GetNodeAniSetFrame();

	bool isAlphaMtrlNode();

	void ToonRenderSettingOnOld(RMtrl* pMtrl);
	void ToonRenderSettingOn(RMtrl* pMtrl);
	void ToonRenderSettingOff();

	void ToonRenderSilhouetteSettingOn();
	void ToonRenderSilhouetteSettingOff();

	RMesh*			m_pParentMesh;
	RMesh*			m_pBaseMesh;
	RMeshNode*		m_pParent;
	RMeshNode*		m_Next;
	RMeshNode*		m_ChildRoot;

	float				m_fDist;

	RBoneBaseMatrix*	m_BoneBaseMatrix;
	int					m_nBoneBaseMatrixCnt;

	int m_MatrixMap[MAX_MATRIX]; // Matrix Register Bone index
	int m_MatrixCount;

	rmatrix		m_ModelWorldMatrix;

	bool			m_bConnectPhysique;

#ifndef _BLEND_ANIMATION
	RAnimationNode*	m_pAnimationNode;
#else
	RAnimationNode* m_pAnimationNode[2];
#endif

#ifdef SCALE_RMESHNODE
	void SetScale(v3 vec) { rmatrixScaling(&matScale, vec.x, vec.y, vec.z); ScaleEnabled = true; }

	bool ScaleEnabled = false;
	matrix matScale;
#endif

private:
	bool SetBVertData(RBlendVertex* pBVert, int i, int j, int pv_index, int* DifferenceMap, int& matrixIndex);
};

_NAMESPACE_REALSPACE2_END