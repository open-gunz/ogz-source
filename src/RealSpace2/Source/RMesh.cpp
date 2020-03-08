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

#ifndef _PUBLISH

#define __BP(i,n)	MBeginProfile(i,n);
#define __EP(i)		MEndProfile(i);

#else

#define __BP(i,n) ;
#define __EP(i) ;

#endif

_NAMESPACE_REALSPACE2_BEGIN

bool			RMesh::mHardwareAccellated = false;
unsigned int	RMesh::mNumMatrixConstant = 0;

bool			RMesh::m_bTextureRenderOnOff = true;
bool			RMesh::m_bVertexNormalOnOff = true;
bool			RMesh::m_bToolMesh	= false;
bool			RMesh::m_bSilhouette = false;
float			RMesh::m_fSilhouetteLength = 300.f;
int				RMesh::m_parts_mesh_loading_skip = 0;

bool			RRenderNodeMgr::m_bRenderBuffer = false;

RMesh::RMesh() 
{
	Init();
}

RMesh::~RMesh() 
{
	Destroy();
}

void RMesh::Init()
{
	m_id			= -1;

	m_data_num		= 0;

	m_max_frame[0]	= 0;
	m_max_frame[1]	= 0;

	m_frame[0]		= 0;
	m_frame[1]		= 0;

	m_pVisualMesh	= NULL;

	m_is_use_ani_set = false;
	m_mtrl_auto_load = true;
	m_is_map_object = false;

	m_bUnUsededCheck = false;

	m_pAniSet[0] = NULL;
	m_pAniSet[1] = NULL;

	m_parts_mgr = NULL;

	m_base_mtrl_mesh = NULL;

	m_data.reserve(MAX_MESH_NODE_TABLE);

	m_isScale = false;
	m_vScale = rvector(1.f,1.f,1.f);

	m_PickingType = pick_real_mesh;
	
	m_MeshWeaponMotionType	= eq_weapon_etc;

	m_LitVertexModel	= false;
	m_bEffectSort		= false;
	m_isPhysiqueMesh	= false;

	m_fVis = 1.0f;

	mbSkyBox = false;

	m_vBBMax = rvector(-9999.f,-9999.f,-9999.f);
	m_vBBMin = rvector( 9999.f, 9999.f, 9999.f);

	m_vBBMaxNodeMatrix = m_vBBMax;
	m_vBBMinNodeMatrix = m_vBBMin;

	m_vAddBipCenter = rvector(0,0,0);

	m_isMultiAniSet = false;
	m_isCharacterMesh = false;
	m_isNPCMesh = false;

	m_nSpRenderMode = 0;

	m_isMeshLoaded = false;

	m_pToolSelectNode = NULL;

}

void RMesh::Destroy()
{
	DelMeshList();

	if(m_parts_mgr) {
		delete m_parts_mgr;
		m_parts_mgr = NULL;
	}

	m_isMeshLoaded = false;
}

void RMesh::ReloadAnimation() 
{
	m_ani_mgr.ReloadAll();
}

float RMesh::GetMeshVis() 
{ 
	return m_fVis;	
}

void  RMesh::SetMeshVis(float vis) 
{ 
	m_fVis = vis; 
}

float RMesh::GetMeshNodeVis(RMeshNode* pNode) 
{
	if(pNode==NULL) return 1.f;

	return max(min(pNode->m_vis_alpha,m_fVis),0.f);
}

void RMesh::SetVisualMesh(RVisualMesh* vm) 
{ 
	m_pVisualMesh = vm; 
}

RVisualMesh* RMesh::GetVisualMesh() 
{ 
	return m_pVisualMesh; 
}

void RMesh::SetMtrlAutoLoad(bool b) 
{ 
	m_mtrl_auto_load = b;
}

bool RMesh::GetMtrlAutoLoad() 
{ 
	return m_mtrl_auto_load; 
}

void RMesh::SetMapObject(bool b) 
{ 
	m_is_map_object = b; 
}

bool RMesh::GetMapObject() 
{ 
	return m_is_map_object; 
}

const char* RMesh::GetFileName()
{
	return m_FileName.c_str();
}

void RMesh::SetFileName(const char* name)
{
	if(!name[0]) return;

	m_FileName = name;
}

void RMesh::SetBaseMtrlMesh(RMesh* pMesh) 
{
	m_base_mtrl_mesh = pMesh; 
}

void RMesh::SetScale(rvector& v) 
{
	m_vScale = v;
	m_isScale = true;
}

void RMesh::ClearScale() 
{
	m_vScale = rvector(1.f,1.f,1.f);
	m_isScale = false;
}

void RMesh::SetPickingType(RPickType type) 
{
	m_PickingType = type;
}

RPickType RMesh::GetPickingType() 
{
	return m_PickingType;
}

void RMesh::SetMeshWeaponMotionType(RWeaponMotionType t) 
{
	m_MeshWeaponMotionType = t;
}

RWeaponMotionType RMesh::GetMeshWeaponMotionType() 
{
	return m_MeshWeaponMotionType;
}

void RMesh::SetPhysiqueMeshMesh(bool b) 
{
	m_isPhysiqueMesh = b;
}

bool RMesh::GetPhysiqueMesh() 
{
	return m_isPhysiqueMesh;
}

bool RMesh::isVertexAnimation(RMeshNode* pNode) 
{
	RAnimation* pAniSet = GetNodeAniSet(pNode);

	if(pAniSet) 
		if( pAniSet->GetAnimationType() == RAniType_Vertex ) 
			return true;

	return false;
}

void RMesh::SetSpRenderMode(int mode) 
{
	m_nSpRenderMode = mode;
}

bool RMesh::CmpFileName(const char* name)
{
	if(!name[0]) return false;
	
	if(m_FileName == name )
		return true;
	return false;
}

const char* RMesh::GetName()
{
	return m_ModelName.c_str();
}

void RMesh::SetName(const char* name)
{
	if(!name[0]) return;

	m_ModelName = name;
}

bool RMesh::CmpName(const char* name)
{
	if(!name[0]) return false;

	if(m_ModelName == name )
		return true;

	return false;
}

void RMesh::GetMeshData(RMeshPartsType type, std::vector<RMeshNode*>& nodetable)
{
	RMeshNode*	pMesh = NULL;

	RMeshNodeHashList_Iter it_obj =  m_list.begin();

	while (it_obj !=  m_list.end()) {
		pMesh = (*it_obj);
		if(pMesh->m_PartsType==type) {
			nodetable.push_back(pMesh);
		}
		it_obj++;
	}
}

RMeshNode* RMesh::GetMeshData(RMeshPartsType type)
{
	RMeshNode*	pMesh = NULL;

	RMeshNodeHashList_Iter it_obj =  m_list.begin();

	while (it_obj !=  m_list.end()) {
		pMesh = (*it_obj);
		if(pMesh->m_PartsType==type)
			return pMesh;
		it_obj++;
	}
	return NULL;
}

RMeshNode* RMesh::GetMeshData(const char* name)
{
	RMeshNode*	pMesh = NULL;

	RMeshNodeHashList_Iter it_obj =  m_list.begin();

	while (it_obj !=  m_list.end()) {
		pMesh = (*it_obj);
		if(strcmp(pMesh->GetName(),name)==0)
			return pMesh;
		it_obj++;
	}
	return NULL;
}

RMeshNode* RMesh::GetPartsNode(const char* name)
{
	if(!m_parts_mgr)
		return NULL;

	return m_parts_mgr->GetPartsNode(name);
}

void RMesh::GetPartsNode(RMeshPartsType type, std::vector<RMeshNode*>& nodetable)
{
	if(!m_parts_mgr)
		return;

	m_parts_mgr->GetPartsNode(type,nodetable);
}

void RMesh::DelMeshList()
{
	if (m_list.empty())
		return;

	RMeshNodeHashList_Iter node =  m_list.begin();

	while (node !=  m_list.end()) {
		delete (*node);
		node =  m_list.Erase(node);
	}

	m_data_num = 0;
}

int RMesh::FindMeshId(RAnimationNode* pANode)
{
	if( pANode == NULL )
		return -1;

	int ret_id = -1;

	bool bReConnect = false;

	if(pANode->m_pConnectMesh != this)
		bReConnect = true;
	else if( pANode->m_node_id == -1 )
		bReConnect = true;

	if( bReConnect ) {

		ret_id = FindMeshIdSub( pANode );

		pANode->m_node_id = ret_id;
		pANode->m_pConnectMesh = this;

		return ret_id;
	}

	return pANode->m_node_id;
}

int RMesh::FindMeshParentId(RMeshNode* pMeshNode)
{
	if(pMeshNode==NULL)
		return -1;

	int ret_id = -1;

	if( pMeshNode->m_nParentNodeID == -1 ) {
		ret_id = _FindMeshId(pMeshNode->m_Parent);
		pMeshNode->m_nParentNodeID = ret_id;
		return ret_id;
	}
		
	return pMeshNode->m_nParentNodeID;
}

int RMesh::_FindMeshId(int e_name)
{
	if (m_list.empty())
		return -1;

	RMeshNode* pNode = m_list.Find(e_name);
	if (pNode != NULL)
		return pNode->m_id;

	return -1;
}

int RMesh::_FindMeshId(const char* name)
{
	if (m_list.empty())
		return -1;

	RMeshNode* pNode = m_list.Find(name);
	if (pNode != NULL)
		return pNode->m_id;

	return -1;
}

int RMesh::FindMeshId(RMeshNode* pNode)
{
	if(!pNode) return -1;

	if(pNode->m_NameID != -1)
		return _FindMeshId(pNode->m_NameID);

	return _FindMeshId(pNode->GetName());
}

int RMesh::FindMeshIdSub(RAnimationNode* pANode)
{
	if(!pANode) return -1;

	if(pANode->m_NameID != -1)
		return _FindMeshId(pANode->m_NameID);

	return _FindMeshId(pANode->GetName());
}

void __SetPosMat(rmatrix* m1,rmatrix* m2) {

	m1->_41 = m2->_41;
	m1->_42 = m2->_42;
	m1->_43 = m2->_43;
}

void __SetRotMat(rmatrix* m1,rmatrix* m2) {

	rmatrix m;

	m = *m1;
	*m1 = *m2;

	m1->_41 = m._41;
	m1->_42 = m._42;
	m1->_43 = m._43;
}

bool RMesh::ConnectMtrl()
{
	RMeshNode*	pMeshNode = NULL;

	RMeshNodeHashList_Iter it_obj =  m_list.begin();

	while (it_obj !=  m_list.end()) {
		pMeshNode = (*it_obj);

		if(pMeshNode) {
			if(pMeshNode->m_face_num)
				pMeshNode->ConnectMtrl();
		}

		it_obj++;
	}

	return NULL;
}

// Not sure if this is necessary?
bool RMesh::ConnectAnimation(RAnimation* pAniSet)
{
	if(!pAniSet) 
		return false;

	RAnimationNode* pANode = NULL;

	int pid = -1;

	int node_cnt = pAniSet->GetAniNodeCount();

	for(int i=0;i< node_cnt;i++) {

		pANode = pAniSet->GetAniNode(i);

		pid = FindMeshId(pANode);
	}

	pAniSet->m_isConnected = true;

	return true;
}

static bool NeedsReconnection(RAnimation* CurrentAniSet, RAnimation* NewAniSet)
{
	return !(CurrentAniSet == NewAniSet &&
		NewAniSet->m_isConnected &&
		CurrentAniSet->CheckName(NewAniSet->GetName()));
}

void RMesh::SetAnimationNodes(RAnimation* AniSet, size_t Index, CutParts parts)
{
	assert(AniSet);

	m_is_use_ani_set = true;

	for (size_t i{}, end = AniSet->GetAniNodeCount(); i < end; ++i)
	{
		auto* AnimationNode = AniSet->GetAniNode(i);
		if (!AnimationNode)
			continue;

		auto pid = FindMeshId(AnimationNode);
		if (pid == -1)
			continue;

		auto* MeshNode = m_data[pid];
		if (!MeshNode)
			continue;

		if (parts != -1 && MeshNode->m_CutPartsType != parts)
			continue;

		MeshNode->m_pAnimationNode = AnimationNode;
		MeshNode->m_mat_base = MeshNode->m_mat_local = AnimationNode->m_mat_base;
		MeshNode->m_mat_inv = Inverse(MeshNode->m_mat_local);

		if (parts == cut_parts_upper_body && MeshNode->m_LookAtParts == lookat_parts_spine1) {
			auto* ParentAnimationNode = AniSet->GetNode(MeshNode->m_Parent);
			auto LocalMatrix = AnimationNode->m_mat_base * Inverse(ParentAnimationNode->m_mat_base);
			MeshNode->m_spine_local_pos = GetTransPos(LocalMatrix);
		}
	}
}

void RMesh::ReconnectAnimation(RAnimation* AniSet, size_t Index, bool HoldFrame)
{
	if (!AniSet->m_isConnected) {
		ConnectAnimation(AniSet);
	}

	m_pAniSet[Index] = AniSet;
	if (!HoldFrame)
		m_frame[Index] = 0;

	m_max_frame[Index] = AniSet->GetMaxFrame();
}

void RMesh::ClearAnimationNodesBeforeChange()
{
	m_is_use_ani_set = true;
	for (int i = 0; i < m_data_num; i++)
		m_data[i]->m_pAnimationNode = nullptr;
}

bool RMesh::SetAnimation1Parts(RAnimation* AniSetLower) {

	auto RMeshSetAnimation1Parts = MBeginProfile("RMesh::SetAnimation1Parts");

	bool HoldFrame{};

	if (!NeedsReconnection(m_pAniSet[0], AniSetLower))
	{
		if (!m_pAniSet[1])
			return true;
		
		HoldFrame = true;
	}

	ClearAnimationNodesBeforeChange();

	ReconnectAnimation(AniSetLower, 0, HoldFrame);
	SetAnimationNodes(AniSetLower, 0, static_cast<CutParts>(-1));
	m_pAniSet[1] = nullptr;
	
	return true;
}

bool RMesh::SetAnimation2Parts(RAnimation* AniSetLower, RAnimation* AniSetUpper) {

	auto prof = MBeginProfile("RMesh::SetAnimation2Parts");

	bool r[2] = {
		NeedsReconnection(m_pAniSet[0], AniSetLower),
		NeedsReconnection(m_pAniSet[1], AniSetUpper)
	};

	if (!r[0] && !r[1])
		return true;

	ClearAnimationNodesBeforeChange();

	if (r[0])
		ReconnectAnimation(AniSetLower, 0);
	if (r[1])
		ReconnectAnimation(AniSetUpper, 1);

	SetAnimationNodes(AniSetLower, 0, cut_parts_lower_body);
	SetAnimationNodes(AniSetUpper, 1, cut_parts_upper_body);

	RAnimationNode* RootNodes[] = { AniSetLower->GetNode("Bip01"), AniSetUpper->GetNode("Bip01") };

	if (RootNodes[0] && RootNodes[1])
		m_vAddBipCenter = GetTransPos(RootNodes[1]->m_mat_base) - GetTransPos(RootNodes[0]->m_mat_base);
	else
		m_vAddBipCenter = { 0, 0, 0 };

	return true;
}

bool RMesh::SetAnimation(RAnimation* pAniSet, RAnimation* pAniSetUpper)
{
	if (!pAniSet) return false;

	if (pAniSetUpper)
		return SetAnimation2Parts(pAniSet, pAniSetUpper);

	return SetAnimation1Parts(pAniSet);
}

bool RMesh::SetAnimation(char* name, char* ani_name_upper) {

	if (m_ani_mgr.m_list.empty())
		return false;

	RAnimation* pAniSet = NULL;
	RAnimation* pAniSetUpper = NULL;

	pAniSet = m_ani_mgr.GetAnimation(name, -1);

	if (ani_name_upper) {
		pAniSetUpper = m_ani_mgr.GetAnimation(ani_name_upper, -1);
	}

	return SetAnimation(pAniSet, pAniSetUpper);
}

void RMesh::ClearAnimation()
{
	m_is_use_ani_set = false;
	m_pAniSet[0] = NULL;
	m_pAniSet[1] = NULL;
}

bool RMesh::Pick(int mx, int my, RPickInfo* pInfo, rmatrix* world_mat)
{
	int sw = RGetScreenWidth();
	int sh = RGetScreenHeight();

	const auto& matProj = RProjection;

	rvector v{
		(2.0f * mx / sw - 1) / matProj._11,
		-(2.0f * my / sh - 1) / matProj._22,
		1.0f };

	auto InverseView = Inverse(RView);

	auto pos = GetTransPos(InverseView);
	auto dir = TransformNormal(v, InverseView);

	Normalize(dir);

	return CalcIntersectsTriangle(pos, dir, pInfo, world_mat);

}

bool RMesh::Pick(const rvector& pos, const rvector& dir, RPickInfo* pInfo, rmatrix* world_mat)
{
	return CalcIntersectsTriangle(pos, dir, pInfo, world_mat);
}

bool RMesh::Pick(const rvector* vInVec, RPickInfo* pInfo, rmatrix* world_mat)
{
	return Pick(vInVec[0], vInVec[1], pInfo, world_mat);
}

void RMesh::ClearMtrl() {
	m_mtrl_list_ex.DelAll();
}

void RMesh::CalcBoxNode(rmatrix* world_mat)
{
	m_vBBMax = { -9999.f, -9999.f, -9999.f };
	m_vBBMin = { 9999.f, 9999.f, 9999.f };

	for (auto* Node : m_list)
	{
		CalcNodeMatrixBBox(Node);
		UpdateNodeAniMatrix(Node);
	}
}

void RMesh::CalcBox(rmatrix* world_mat)
{
	if( m_list.empty())	return;

	RMeshNode* pTMeshNode  = NULL;

	m_vBBMax = { -9999.f, -9999.f, -9999.f };
	m_vBBMin = { 9999.f, 9999.f, 9999.f };

	RMeshNodeHashList_Iter it_obj =  m_list.begin();

	while (it_obj !=  m_list.end()) {

		RMeshNode*	pMeshNode = (*it_obj);

		CalcNodeMatrixBBox(pMeshNode);//bbox

		pTMeshNode = UpdateNodeAniMatrix(pMeshNode);

		if(pTMeshNode->m_isDummyMesh) {//Bip,Bone,Dummy Skip
			it_obj++;
			continue;
		}

		if(pTMeshNode->m_face_num != 0) {
			if(world_mat)
				pTMeshNode->CalcVertexBuffer(*world_mat,true);
		}

		it_obj++;
	}
}

void RMesh::CalcNodeMatrixBBox(RMeshNode* pNode)
{
	auto Trans = GetTransPos(pNode->m_mat_result);
	SubCalcBBox(
		&m_vBBMaxNodeMatrix,
		&m_vBBMinNodeMatrix,
		&Trans);
}

void RMesh::CalcBBox(rvector* v) 
{
	SubCalcBBox(&m_vBBMax, &m_vBBMin, v);
}

void RMesh::SubCalcBBox(rvector* vmax, rvector* vmin, rvector* v)
{
	if ( (vmax==NULL) || (vmin==NULL) || (v==NULL) ) return;

	vmin->x = (std::min)(vmin->x, v->x);
	vmin->y = (std::min)(vmin->y, v->y);
	vmin->z = (std::min)(vmin->z, v->z);

	vmax->x = (std::max)(vmax->x, v->x);
	vmax->y = (std::max)(vmax->y, v->y);
	vmax->z = (std::max)(vmax->z, v->z);
}

void RMesh::RenderBox(rmatrix* world_mat)
{
	draw_box(world_mat,m_vBBMax,m_vBBMin,0xffffffff);
}

rvector RMesh::GetOrgPosition()
{
	if (auto* pMNode = m_data[0])
		return GetTransPos(pMNode->m_mat_base);

	return{ 0, 0, 0 };
}

_NAMESPACE_REALSPACE2_END

#undef __BP
#undef __EP