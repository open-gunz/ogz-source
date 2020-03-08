#include "stdafx.h"
#include <stdio.h>
#include <math.h>
#include <tchar.h>

#include "MXml.h"

#include "RealSpace2.h"
#include "RMesh.h"
#include "RMeshMgr.h"

#include "MDebug.h"

#include "RAnimationMgr.h"
#include "RVisualmeshMgr.h"

#include "MZFileSystem.h"
#include "fileinfo.h"

#include "RShaderMgr.h"
#include "vector"

#include "ROcclusionList.h"

#ifndef _PUBLISH

#define __BP(i,n)	MBeginProfile(i,n);
#define __EP(i)		MEndProfile(i);

#else

#define __BP(i,n) ;
#define __EP(i) ;

#endif

bool g_bVertex_Soft = false;

_NAMESPACE_REALSPACE2_BEGIN

int g_poly_render_cnt;

#define RENDER_NODE_MAX 1000

static int			g_render_cnt=0;
static RRenderNode	g_render_node[ RENDER_NODE_MAX ];

static RRenderNodeList	g_RenderNodeList[eRRenderNode_End];
static RRenderNodeList	g_RenderLNodeList[eRRenderNode_End];

static bool g_rmesh_render_start_begin = false;

static int	 g_vert_index_pos = 0;
static int	 g_lvert_index_pos = 0;

////////////////////////////////////////////////////////////

void RMesh::Render(const rmatrix* world_mat, bool NoPartsChange)
{
	RenderSub(*world_mat, NoPartsChange);
}

RMeshNode* RMesh::FindNode(RMeshPartsPosInfoType type)
{
	RMeshNode* pNode = NULL;

	RMeshNodeHashList_Iter it_obj =  m_list.begin();

	while (it_obj !=  m_list.end()) {

		pNode = (*it_obj);

		if(pNode->m_PartsPosInfoType==type)
			return pNode;

		it_obj++;
	}

	return NULL;
}

bool RMesh::CalcParts(RMeshNode* pPartsMeshNode,RMeshNode* pMeshNode,bool NoPartsChange)
{
	return false;
}

#define _SORT_ALPHA_NODE_TABLE	100
#define _SORT_LAST_NODE_TABLE	100

void RMesh::RenderSub(const rmatrix& world_mat, bool NoPartsChange)
{
	__BP(198, "RMesh::RenderSub");

	LPDIRECT3DDEVICE9 dev = RGetDevice();

	m_vBBMax = { -9999.f, -9999.f, -9999.f };
	m_vBBMin = { 9999.f, 9999.f, 9999.f };

 	m_vBBMaxNodeMatrix = m_vBBMax;
	m_vBBMinNodeMatrix = m_vBBMin;

	__BP(202, "RMesh::RenderSub::RenderFrame");
	RenderFrame();
	__EP(202);

	__BP(199, "RMesh::RenderSub::State b");

	RMeshNode* pMeshNode = NULL;
	RMeshNode* pPartsMeshNode = NULL;

	if(m_pVisualMesh && !m_LitVertexModel) { 
		m_pVisualMesh->UpdateLight();
	}

	if(m_LitVertexModel)
		dev->SetRenderState( D3DRS_LIGHTING, FALSE );
	else 
		dev->SetRenderState( D3DRS_LIGHTING, TRUE );

	dev->SetRenderState( D3DRS_CULLMODE, D3DCULL_CCW );
	dev->SetRenderState( D3DRS_ALPHABLENDENABLE, FALSE );
	dev->SetRenderState( D3DRS_ZWRITEENABLE, TRUE );

	__EP(199);

	static RMeshNode* pAlphaNode[_SORT_ALPHA_NODE_TABLE];
	static RMeshNode* pLastNode[_SORT_LAST_NODE_TABLE];

	int nAlphaNodeCnt = 0;
	int nLastNodeCnt = 0;

	__BP(500, "RMesh::RenderSub::Loop");

	if(m_pVisualMesh && m_pVisualMesh->m_bIsCharacter)
		m_pVisualMesh->ClearPartInfo();

	for (auto* pMeshNode : m_list)
	{
		pMeshNode->CheckAlign(world_mat);

		CalcNodeMatrixBBox( pMeshNode );

		pPartsMeshNode = pMeshNode;

		if( GetToolMesh() && m_pToolSelectNode ) {

			if(pMeshNode==m_pToolSelectNode)
				draw_box(&pMeshNode->m_mat_result,pMeshNode->m_max,pMeshNode->m_min,0xffff0000);
		}

		if(m_pVisualMesh) {
			if(NoPartsChange)	pPartsMeshNode = NULL;
			else				pPartsMeshNode = m_pVisualMesh->GetParts(pMeshNode->m_PartsType);

			m_pVisualMesh->UpdateWeaponDummyMatrix(pMeshNode);

			if( pMeshNode->m_isWeaponMesh ) {
				m_pVisualMesh->UpdateWeaponPartInfo(pMeshNode);
				continue;
			} 
			else {
				if( pPartsMeshNode==NULL ) {
					pPartsMeshNode = pMeshNode;
				} 
				else if( pPartsMeshNode->m_PartsType != pMeshNode->m_PartsType)
				{
					pPartsMeshNode = pMeshNode;
				}
			}
		}

		pPartsMeshNode->m_pBaseMesh = this;

		if(pMeshNode->m_isAddMeshNode)
		{
			if(pMeshNode->m_pParent) {
				pPartsMeshNode->m_mat_result = pMeshNode->m_mat_add * pMeshNode->m_pParent->m_mat_result;
			}
		}

		if(pPartsMeshNode->m_WeaponDummyType != weapon_dummy_etc) {
			continue;
		}

		if (pPartsMeshNode->m_isDummyMesh) { //Bip,Bone,Dummy Skip
			if (m_pVisualMesh && m_pVisualMesh->m_pBipMatrix) {
				if (pMeshNode->m_PartsPosInfoType != eq_parts_pos_info_etc) {
					m_pVisualMesh->m_pBipMatrix[pMeshNode->m_PartsPosInfoType] = pMeshNode->m_mat_result;
				}
			}
			continue;
		}		

		pPartsMeshNode->GetNodeVisValue();

		// Cloth MeshNode Skip

		if( pPartsMeshNode->m_PartsType == eq_parts_chest && !NoPartsChange && !m_is_map_object) //low poly model
		{
			if( m_pVisualMesh && m_pVisualMesh->isChestClothMesh() && pPartsMeshNode->m_isClothMeshNode ) {

				__BP(405, "RMesh::Draw::Cloth::update&render");

				m_pVisualMesh->UpdateCloth();
				m_pVisualMesh->RenderCloth();
				
				__EP(405);

				continue;
			}
		}

		if( pPartsMeshNode->m_isLastModel ) {

			pLastNode[nLastNodeCnt] = pPartsMeshNode;
			nLastNodeCnt++;

			if(nLastNodeCnt > _SORT_LAST_NODE_TABLE) {
				nLastNodeCnt = _SORT_LAST_NODE_TABLE;
				mlog("nLastNodeCnt > %d, filename = %s\n", _SORT_LAST_NODE_TABLE, GetFileName());
				nLastNodeCnt--;
			}

			continue;
		}

		if( pPartsMeshNode->isAlphaMtrlNode() ) {

			if( RRenderNodeMgr::m_bRenderBuffer ) {
			}
			else {
			
				pAlphaNode[nAlphaNodeCnt] = pPartsMeshNode;
				nAlphaNodeCnt++;

				if(nAlphaNodeCnt > _SORT_ALPHA_NODE_TABLE-1) {
					nAlphaNodeCnt = _SORT_ALPHA_NODE_TABLE-1;
					mlog("nAlphaNodeCnt > %d, filename = %s\n", _SORT_ALPHA_NODE_TABLE - 1, GetFileName());
					nAlphaNodeCnt--;
				}

			}

			continue;
		}

		RenderNode(pPartsMeshNode, world_mat);
	}

	RMeshNode* pATMNode{};
	RMeshNode* pTNodeHead{};
	RMeshNode* pTLastModel{};

	for(int n=0;n<nAlphaNodeCnt;n++) {
		pATMNode = pAlphaNode[n];
		if(!pATMNode) continue;

		if(pATMNode->m_PartsType == eq_parts_head ) {
			pTNodeHead = pATMNode;
			continue;
		}

		RenderNode(pATMNode, world_mat);
	}

	if (pTNodeHead) {
		RenderNode(pTNodeHead, world_mat);
	}

	std::sort(pLastNode, pLastNode + nLastNodeCnt,
		[&](auto* a, auto* b) { return a->m_Name < b->m_Name; });

	for (int n = 0; n < nLastNodeCnt; n++)
		if (auto* pATMNode = pLastNode[n])
			RenderNode(pATMNode, world_mat);

	static rmatrix _init_mat = GetIdentityMatrix();
	dev->SetTransform(D3DTS_WORLD, static_cast<const D3DMATRIX*>(_init_mat));

	__EP(500);

	__EP(198);
}

class RIVec
{
public:
	void Add(int v)
	{
		m_Value[m_size] = v;
		m_size++;

		if(m_size>2)
			m_size=2;
	}

	int m_size{};
	int m_Value[3]{};
};

class CIndexBufferMake
{
public:
	CIndexBufferMake() {

		m_pFaceIndex = NULL;
		m_tAddPos = 0;
	}

	~CIndexBufferMake() {
		Clear();
	}

	void Create(int face_num,int point_num) {
		
		if(face_num) {

			m_pFaceIndex = new WORD[face_num*3];
			memset(m_pFaceIndex,0,sizeof(WORD)*face_num*3);
		}

		m_tAddPos = point_num;
		
	}

	void Clear() {

		if(m_pFaceIndex)
			delete[] m_pFaceIndex;
	}

	void MakeLVertex(RMeshNode* pMNode,RFaceInfo* pFace,int i,DWORD color)
	{
		int in = 0;

		RFaceNormalInfo* pFNL = &pMNode->m_face_normal_list[i];

		for(int k=0;k<3;k++) {

			int in = GetLIndex( pFace->m_point_index[k] , &pFace->m_point_tex[k] );

			m_pFaceIndex[i*3+k] = in;
		}
	}

	void MakeVertex(RMeshNode* pMNode,RFaceInfo* pFace,int i)
	{
		int in = 0;

		RFaceNormalInfo* pFNL = &pMNode->m_face_normal_list[i];

		for(int k=0;k<3;k++) {

			int in = GetIndex( pFace->m_point_index[k] , &pFace->m_point_tex[k] );

			m_pFaceIndex[i*3+k] = in;
		}
	}

	int GetLIndex(int in,rvector* puv)
	{
		static float _uv[4];

		int _size = m_pPointTable[in].m_size;

		if(_size) {
			return m_tAddPos-1;
		}

		m_pPointTable[in].Add( in );

		return in;
	}

	int GetIndex(int in,rvector* puv)
	{
		static float _uv[4];

		int _size = m_pPointTable[in].m_size;

		if(_size) {
			return m_tAddPos-1;
		}

		m_pPointTable[in].Add( in );

		return in;
	}

	void GetLUV(float* uv,RLVertex* pVert,rvector* v) {
		uv[0] = pVert->tu;
		uv[1] = pVert->tv;
		uv[2] = v->x;
		uv[3] = v->y;
	}


	void GetUV(float* uv,RVertex* pVert,rvector* v) {
		uv[0] = pVert->tu;
		uv[1] = pVert->tv;
		uv[2] = v->x;
		uv[3] = v->y;
	}

	bool isEqualUV(float u1,float v1,float u2,float v2) {

		if(u1 == u2 )
			if(v1 == v2 )
				return true;
		return false;
	}

	bool isEqualUV(float* uv) {
		return isEqualUV(uv[0],uv[1],uv[2],uv[3]);
	}

	RIVec	m_pPointTable[2000];
	WORD*	m_pFaceIndex;
	
	int		m_tAddPos;
};

void BBoxSubCalc(v3_pod* max,v3_pod* min);

bool RMesh::CheckOcclusion(RMeshNode *pMeshNode)
{
	if( m_is_map_object && !mbSkyBox)
	{
		if(m_pVisualMesh)
		{
			if(m_pVisualMesh->m_pTOCCL)
			{
				rboundingbox bb;

				rmatrix _mm;

				static rmatrix _mrot = RGetRotY(180) * RGetRotX(90);
	
				_mm = pMeshNode->m_mat_base * _mrot;

				bb.vmax = pMeshNode->m_max * _mm;
				bb.vmin = pMeshNode->m_min * _mm;
				
				BBoxSubCalc(&bb.vmax, &bb.vmin);

				if(m_pVisualMesh->m_bCheckViewFrustum) {
					if(isInViewFrustumWithZ(bb, RGetViewFrustum()) == false) {
						return false;
					}
				}

				return m_pVisualMesh->m_pTOCCL->IsVisible(bb);
			}
		}
	}
	return true;
}

void RMesh::RenderNode(RMeshNode *pMeshNode, const rmatrix& world_mat)
{
	if (pMeshNode->m_face_num == 0) return;

	if (m_pVisualMesh)
		if (m_pVisualMesh->m_bRenderMatrix)
			return;

	if (CheckOcclusion(pMeshNode) == false)
		return;

	__BP(501, "RMesh::RenderNode");

	SetMtrlUvAni_ON();

	bool bDrawCharPhysique = false;

	if( RIsSupportVS() && RShaderMgr::mbUsingShader )
	{
		if((m_pAniSet[0]) && (RAniType_Bone == m_pAniSet[0]->GetAnimationType()) 
			&& mHardwareAccellated && pMeshNode->m_MatrixCount > 0 )
		{
			bDrawCharPhysique = true;
		}
	}

	if(bDrawCharPhysique) {
		pMeshNode->RenderNodeVS(this, world_mat);
	}
	else {
		pMeshNode->CalcVertexBuffer(world_mat);
		pMeshNode->Render();
	}

	SetMtrlUvAni_OFF();

	__EP(501);

}

static bool find_intersects_triangle_sub(const rvector& orig, const rvector& dir,
	const rvector& v0, const rvector& v1, const rvector& v2, float* t, float* u, float* v )
{
	auto edge1 = v1 - v0;
	auto edge2 = v2 - v0;

	auto pvec = CrossProduct(dir, edge2 );

	auto det = DotProduct(edge1, pvec);
	if( det < 0.0001f )
		return false;

	auto tvec = orig - v0;

	*u = DotProduct(tvec, pvec);

	if( *u < 0.0f || *u > det )
		return false;

	auto qvec = CrossProduct(tvec, edge1);

	*v = DotProduct(dir, qvec);

	if( *v < 0.0f || *u + *v > det )
		return false;

	*t = DotProduct(edge2, qvec);

	auto fInvDet = 1.0f / det;

	*t *= fInvDet;
	*u *= fInvDet;
	*v *= fInvDet;

	return true;
}

bool RMesh::CalcIntersectsTriangle(const v3& origin, const v3& dir, RPickInfo* pInfo, rmatrix* world_mat, bool fastmode)
{
	float best_t = 9999.f;

	RMeshNode* pFindMeshNode = NULL;
	RMeshNode* pPartsMeshNode = NULL;

	rvector		vFindVec[3];
	
	bool  bFind = false;

	for (auto* pMeshNode : m_list)
	{
		pPartsMeshNode = UpdateNodeAniMatrix(pMeshNode);

		if (m_PickingType == pick_collision_mesh) {
			if (!pPartsMeshNode->m_isCollisionMesh) {
				continue;
			}
		}
		else if (m_PickingType == pick_real_mesh) {
			if (pPartsMeshNode->m_isDummyMesh) {//Bip,Bone,Dummy Skip
				continue;
			}
		}

		if (pPartsMeshNode->m_face_num == 0)
			continue;

		std::vector<rvector> VecPick;
		pPartsMeshNode->CalcPickVertexBuffer(*world_mat, VecPick);

		float t, u, v;
		rvector vec[3];

		for (int i = 0; i < pPartsMeshNode->m_face_num; i++) {

			vec[0] = VecPick[pPartsMeshNode->m_face_list[i].m_point_index[0]];
			vec[1] = VecPick[pPartsMeshNode->m_face_list[i].m_point_index[1]];
			vec[2] = VecPick[pPartsMeshNode->m_face_list[i].m_point_index[2]];

			if (find_intersects_triangle_sub(origin, dir, vec[0], vec[1], vec[2], &t, &u, &v)) {

				if (t < best_t) {
					best_t = t;
					pFindMeshNode = pPartsMeshNode;
					memcpy(vFindVec, vec, sizeof(rvector) * 3);
				}

				if (fastmode) {
					if (pInfo) {
						pInfo->vOut = vec[0] + u * (vec[1] - vec[0]) + v * (vec[2] - vec[0]);
						pInfo->t = best_t;
					}
					return true;
				}

				bFind = true;
			}
		}
	}

	if( bFind ) {
		if(pInfo) {
			auto pl = PlaneFromPoints(vFindVec[0], vFindVec[1], vFindVec[2]);

			rvector p;
			IntersectLineSegmentPlane(&p, pl, origin, origin + dir * 10000.f);

			pInfo->vOut	 = p;
			pInfo->t	 = best_t;
			pInfo->parts = pFindMeshNode->m_PartsType;
		}
		return true;
	}
	return false;
}

static RRenderNodeMgr g_render_node_mgr;

void RRenderNode::Render()
{
	if(m_pNode && m_pNode->m_pParentMesh) {
		m_pNode->Render(&m_matWorld);
	}
}

void RRenderNodeMgr::Clear() 
{
	for(int i=0;i<eRRenderNode_End;i++) {
		m_RenderNodeList[i].Clear();
	}

	m_nTotalCount = 0;
}

int RRenderNodeMgr::Add(rmatrix& m,int mode,RMeshNode* pMNode,int nMtrl)
{
	bool lit = false;

	if(pMNode==NULL)
		return m_nTotalCount;

	RRenderNode* pNode = new RRenderNode;

	pNode->Set(mode,m,pMNode,nMtrl,0,0,1.f);

	m_RenderNodeList[mode].push_back( pNode );

#ifdef _DEBUG
	int _size = m_RenderNodeList[mode].size();
	m_RenderNodeList[mode].m_data[_size-1] = pNode;
#endif

	m_nTotalCount++;

	return m_nTotalCount;
}

void RRenderNodeMgr::Render()
{
	LPDIRECT3DDEVICE9 dev = RGetDevice();

	for(int i=0;i<eRRenderNode_End;i++) {
		m_RenderNodeList[i].Render();
	}

	Clear();
}

void RenderNodeMgr_Add(rmatrix& m,int mode,RMeshNode* pMNode,int nMtrl)
{
	if(RRenderNodeMgr::m_bRenderBuffer==false) 
		return;

	g_render_node_mgr.Add(m,mode,pMNode,nMtrl);
}

void RenderNodeMgr_Render()
{
	if(RRenderNodeMgr::m_bRenderBuffer==false) 
		return;

	g_render_node_mgr.Render();
}

void RMeshRenderS(bool lit,int Rmode,rmatrix m,RMeshNode* pMNode,RMtrl* pMtrl,int begin,int size ,float vis_alpha);
bool RMeshRenderSBegin();
bool RMeshRenderSEnd();

RRenderNode* GetNewRenderNode() {
	if(g_render_cnt > RENDER_NODE_MAX-1) {
		return NULL;
	}
	g_render_node[g_render_cnt].Clear();
	g_render_cnt++;
	return &g_render_node[g_render_cnt-1];
}

void ClearRenderNodeBuffer() {
	g_render_cnt = 0;
}

void RMeshRenderS(bool lit,int Rmode,rmatrix m,RMeshNode* pMNode,RMtrl* pMtrl,int begin,int size ,float vis_alpha)
{
	RRenderNode* pRNode = GetNewRenderNode();

	if(pRNode==NULL) return;

	pRNode->Set(Rmode,m,pMNode,pMtrl,begin,size,vis_alpha);

	if(lit) {
		g_RenderLNodeList[Rmode].push_back(pRNode);
	}
	else {
		g_RenderNodeList[Rmode].push_back(pRNode);
	}
}

bool RMeshRenderSBegin()
{
	if(g_rmesh_render_start_begin) {
		mlog("RMeshRenderSEnd() 을 먼저 한 후 사용\n");
		return false;
	}

	g_rmesh_render_start_begin = true;

	ClearRenderNodeBuffer();

	g_vert_index_pos = 0;
	g_lvert_index_pos = 0;

	return true;
};

bool RMeshRenderSEnd()
{
	return true;
}

_NAMESPACE_REALSPACE2_END

#undef __BP
#undef __EP