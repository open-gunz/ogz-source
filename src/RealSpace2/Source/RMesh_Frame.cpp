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
#include "LogMatrix.h"

#ifdef _DEBUG
#include "../../CSCommon/Include/AnimationStuff.h"
#endif

_NAMESPACE_REALSPACE2_BEGIN

RMeshNode* RMesh::UpdateNodeAniMatrix(RMeshNode* pNode)
{
	RMeshNode* pTMeshNode = pNode;

	auto ani_mat = GetIdentityMatrix();

	GetNodeAniMatrix(pNode, ani_mat);

	if(m_pVisualMesh) {
		pTMeshNode = m_pVisualMesh->GetParts(pNode->m_PartsType);

		if( pTMeshNode==NULL ) {
			pTMeshNode = pNode;
		}
		else if( pTMeshNode->m_PartsType != pNode->m_PartsType) {
			pTMeshNode = pNode;
		}
	}
	else {
		pTMeshNode = pNode;
	}

	pTMeshNode->m_mat_result = ani_mat;

	return pTMeshNode;
}

void RMesh::GetNodeAniMatrix(RMeshNode* pMeshNode,rmatrix& ani_mat)
{
	RAnimation* pAniSet = GetNodeAniSet(pMeshNode);
	int frame = pMeshNode->GetNodeAniSetFrame();

	AnimationType ani_type = RAniType_Bone;

	if(pAniSet) {
		ani_type = pAniSet->GetAnimationType();
	}

#if 0
	bool CustomCalced = false;
	
	if (m_pVisualMesh)
	{
		matrix* parent_base_inv = nullptr;
		if (pMeshNode->m_pParent)
		{
			parent_base_inv = &pMeshNode->m_pParent->m_mat_inv;
		}

		if (pMeshNode->m_PartsPosInfoType == eq_parts_pos_info_Spine1 && m_pAniSet[1])
		{
			CustomCalced = GetUpperSpine1(ani_mat, pAniSet, frame,
				m_pVisualMesh->m_vRotXYZ.y, m_pVisualMesh->m_FrameTime.GetValue());
		}
		else
		{
			CustomCalced = GetNodeMatrix(ani_mat, pMeshNode->GetName(), parent_base_inv,
				pAniSet, frame, m_pVisualMesh->m_vRotXYZ.y, m_pVisualMesh->m_FrameTime.GetValue());
		}


		if (CustomCalced)
		{
#ifdef SCALE_RMESHNODE
			if (pMeshNode->ScaleEnabled)
				ani_mat *= pMeshNode->matScale;
#endif
			if (pMeshNode->m_pParent)
				ani_mat *= pMeshNode->m_pParent->m_mat_result;

			return;
		}
	}
#endif

	if(ani_type == RAniType_Tm)
	{
		_RGetAniMat(pMeshNode,frame,ani_mat);
	}
	else if(ani_type == RAniType_Vertex)
	{
		// skip	
	} 
	else
	{
		_RGetRotAniMat(pMeshNode,frame,ani_mat);
		_RGetPosAniMat(pMeshNode,frame,ani_mat);

		CalcLookAtParts(ani_mat, pMeshNode, m_pVisualMesh);

#ifdef SCALE_RMESHNODE
		if (pMeshNode->ScaleEnabled)
			ani_mat *= pMeshNode->matScale;
#endif

		if (pMeshNode->m_pParent) {
			ani_mat *= pMeshNode->m_pParent->m_mat_result;
		}
	}
}

void RMesh::_RGetAniMat(RMeshNode* pMeshNode,int frame,rmatrix& t_ani_mat)
{
	if(pMeshNode==NULL) {
		mlog("_RGetAniMat() pMeshNode==NULL\n");
		return;
	}

	RAnimationNode* pANode = pMeshNode->m_pAnimationNode;

	bool bAni = false;

	if ( pANode )
		if( pANode->m_mat_cnt )
			bAni = true;

	if ( bAni )	{
		t_ani_mat = pANode->GetTMValue(frame);
	}
}

void RMesh::_RGetRotAniMat(RMeshNode* pMeshNode,int frame,rmatrix& t_ani_mat)
{
	rmatrix buffer,Inv;

	if(pMeshNode==NULL)
		return;

	RAnimationNode* pANode = pMeshNode->m_pAnimationNode;

	bool bAni = false;

	if ( pANode ) 
		if( pANode->m_rot_cnt )
			bAni = true;

	if ( bAni )	{
		t_ani_mat = QuaternionToMatrix(pANode->GetRotValue(frame));
	} else {

		GetIdentityMatrix(buffer);

		if(m_isNPCMesh && (pMeshNode->m_WeaponDummyType != weapon_dummy_etc) )
		{
			memcpy(&buffer,&pMeshNode->m_mat_local,sizeof(rmatrix));

		} else  {

			if(pMeshNode->m_pParent) {
				RMatInv(Inv, pMeshNode->m_pParent->m_mat_base);
				buffer = pMeshNode->m_mat_base * Inv;
			}
			else {
				memcpy(&buffer,&pMeshNode->m_mat_local,sizeof(rmatrix));
			}
		}

		SetTransPos(buffer, { 0, 0, 0 });
		t_ani_mat *= buffer;
	}
}

void RMesh::_RGetPosAniMat(RMeshNode* pMeshNode,int frame,rmatrix& t_ani_mat)
{
	if(pMeshNode==NULL)
		return;

	rmatrix buffer;

	RAnimationNode* pANode = pMeshNode->m_pAnimationNode;

	if(pMeshNode->m_LookAtParts == lookat_parts_spine1 && m_pAniSet[1]) {
		SetTransPos(t_ani_mat, pMeshNode->m_spine_local_pos);
		return;
	}

	bool bAni = false;

	if ( pANode ) 
		if( pANode->m_pos_cnt )
			bAni = true;

	if ( bAni ) {
		SetTransPos(t_ani_mat, pANode->GetPosValue(frame));
	}
	else {
		GetIdentityMatrix(buffer);

		if( m_isNPCMesh && pMeshNode->m_WeaponDummyType != weapon_dummy_etc ) {
			buffer = pMeshNode->m_mat_local;
		}
		else if(pMeshNode->m_pParent) {
			buffer = pMeshNode->m_mat_base * pMeshNode->m_pParent->m_mat_inv;
		}
		else {
			buffer = pMeshNode->m_mat_local;
		}

		auto Trans = GetTransPos(buffer);
		SetTransPos(t_ani_mat, Trans);
	}
}

#define MAX_XA_LEFT		90.f
#define MAX_XA_RIGHT	-90.f

#define MAX_YA_FRONT	50.f
#define MAX_YA_BACK		-70.f

void RMesh::CalcLookAtParts(rmatrix& pAniMat,RMeshNode* pMeshNode,RVisualMesh* pVisualMesh)
{
	if (!pMeshNode || !pVisualMesh)
		return;

	float add_value = pVisualMesh->m_FrameTime.GetValue();
	float add_value_npc = 0.f;

	float rot_x = pVisualMesh->m_vRotXYZ.x;
	float rot_y = pVisualMesh->m_vRotXYZ.y;

	if (m_isNPCMesh) {
		add_value_npc = add_value;
	}

	if (rot_x > MAX_XA_LEFT)		rot_x = MAX_XA_LEFT;
	if (rot_x < MAX_XA_RIGHT)	rot_x = MAX_XA_RIGHT;

	if (rot_y > MAX_YA_FRONT)	rot_y = MAX_YA_FRONT;
	if (rot_y < MAX_YA_BACK)		rot_y = MAX_YA_BACK;

	auto Rotate = [&](auto&& add, auto&& coefficient) {
		float rot_y2 = rot_y + add;

		auto mx = RGetRotX(rot_x * coefficient);
		auto my = RGetRotY(rot_y2 * coefficient);

		pAniMat *= mx;
		pAniMat *= my;
	};

	if (pMeshNode->m_LookAtParts == lookat_parts_spine1) Rotate(add_value_npc, 0.6f);
	else if (pMeshNode->m_LookAtParts == lookat_parts_spine2) Rotate(add_value, 0.5f);
	else if (pMeshNode->m_LookAtParts == lookat_parts_head) Rotate(add_value * 0.5f, 0.3f);
}

RAnimation* RMesh::GetNodeAniSet(RMeshNode* pNode)
{
	if(!pNode) return NULL;

	RAnimation* pAniSet = m_pAniSet[0];

	if( m_pAniSet[1] ) {
		if(pNode->m_CutPartsType == cut_parts_upper_body)
			pAniSet = m_pAniSet[1];
	}
	return pAniSet;
}

static void GetQuat(rquaternion& q,rvector& dir1,rvector& dir2)
{
	rvector axis;
	CrossProduct(&axis,dir1,dir2);
	Normalize(axis);
	float theta = DotProduct(dir1,dir2);

	float rad = acos( theta );

	q = AngleAxisToQuaternion(axis, rad);
}

static void GetMat(rmatrix& m, rvector& dir1, rvector& dir2)
{
	rquaternion q;
	GetQuat(q, dir1, dir2);
	m = QuaternionToMatrix(q);
}

static void CalcNodeMatrix(RVisualMesh* pVMesh , RMeshNode* pNode ,bool upLimit)
{
	if( pNode==NULL ) return;

	rvector vTargetPos = pVMesh->m_vTargetPos;
	rmatrix world = pVMesh->m_WorldMat;

	rmatrix rot;

	rmatrix m = pNode->m_mat_result*world;

	draw_box(&m,rvector(5,5,5),rvector(-5,-5,-5),0xff88ffff);

	draw_box(&m,rvector(5,5,5),rvector(-5,-5,-5),0xff22ff22);

	rvector lpos = vTargetPos;

	rvector hpos = rvector(m._41,m._42,m._43);
	rvector dir1 = rvector(m._31,m._32,m._33);
	rvector dir2 = lpos - hpos;

	if(upLimit) {
		dir1.y = 0.f;
		dir2.y = 0.f;
	}

	Normalize(dir1);
	Normalize(dir2);

	GetMat(rot,dir1,dir2);

	m._41 = 0.f;
	m._42 = 0.f;
	m._43 = 0.f;

	m = m * rot;

	m._41 = pNode->m_mat_result._41;
	m._42 = pNode->m_mat_result._42;
	m._43 = pNode->m_mat_result._43;

	pNode->m_mat_result = m;
}

void RMesh::RenderFrame()
{
	if (m_list.empty())
		return;

	RMeshNodeHashList_Iter it_obj =  m_list.begin();

	if(!m_pAniSet[0])
		return;

	RMeshNode* pHeadMeshNode = NULL;
	RMeshNode* pSpine1MeshNode = NULL;
	RMeshNode* pSpine2MeshNode = NULL;

	while (it_obj !=  m_list.end()) {

		RMeshNode* pMeshNode = (*it_obj);

		UpdateNodeAniMatrix(pMeshNode);

		it_obj++;
	}

}

void RMesh::SetFrame(int nFrame,int nFrame2)
{
	if(nFrame < 0 || nFrame >= m_max_frame[0] ) 
		return;

	m_frame[0] = nFrame;

	if(nFrame2 != -1) {
		if(nFrame2 < 0 || nFrame2 >= m_max_frame[1] ) 
			return;
		m_frame[1] = nFrame2;
	}
	else {
		m_frame[1] = 0;
	}
}

_NAMESPACE_REALSPACE2_END