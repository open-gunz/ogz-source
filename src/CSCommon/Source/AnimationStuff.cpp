#include "stdafx.h"
#include "AnimationStuff.h"
#include "MDebug.h"
#include "MMatchItem.h"
#include <array>
#include "RAnimationMgr.h"
#include <cassert>
#include "RMath.h"

using namespace RealSpace2;

static RAnimationMgr* AniMgrs[2];

ZANIMATIONINFO g_AnimationInfoTableLower[ZC_STATE_LOWER_END] = {
	{ ""				,true	,true	,false 	,false },

	{ "idle"			,true	,true	,false 	,false },
	{ "idle2"			,true	,true	,false 	,false },
	{ "idle3"			,true	,true	,false 	,false },
	{ "idle4"			,true	,true	,false 	,false },

	{ "run"				,true	,true	,false 	,false },
	{ "runB"			,true	,true	,false 	,false },
	{ "runL"			,true	,true	,false 	,false },
	{ "runR"			,true	,true	,false 	,false },

	{ "jumpU"			,true	,false	,false 	,false },
	{ "jumpD"			,true	,false	,false 	,false },

	{ "die" 			,true	,false	,false 	,false },
	{ "die2" 			,true	,false	,false 	,false },
	{ "die3" 			,true	,false	,false 	,false },
	{ "die4"			,true	,false	,false 	,false },

	{ "runLW"			,false	,true 	,false	,false },
	{ "runLW_down"		,false	,false	,false	,false },
	{ "runW" 			,false	,false	,true	,false },
	{ "runW_downF"		,false	,false	,false	,false },
	{ "runW_downB"		,false	,false	,false	,false },
	{ "runRW" 			,false	,true 	,false	,false },
	{ "runRW_down"		,false	,false 	,false	,false },

	{ "tumbleF"			,false	,false	,false	,false },
	{ "tumbleB"			,false	,false	,false	,false },
	{ "tumbleR"			,false	,false	,false	,false },
	{ "tumbleL"			,false	,false	,false	,false },

	{ "bind"			,false	,false	,false	,false },

	{ "jumpwallF"		,false	,false 	,false	,false },
	{ "jumpwallB"		,false	,false 	,false	,false },
	{ "jumpwallL"		,false	,false 	,false	,false },
	{ "jumpwallR"		,false	,false 	,false	,false },

	{ "attack1"			,false	,false 	,true  	,false },
	{ "attack1_ret"		,false	,false 	,true  	,true },
	{ "attack2"			,false	,false 	,true  	,false },
	{ "attack2_ret"		,false	,false 	,true  	,true },
	{ "attack3"			,false	,false 	,true  	,false },
	{ "attack3_ret"		,false	,false 	,true  	,true },
	{ "attack4"			,false	,false 	,true  	,false },
	{ "attack4_ret"		,false	,false 	,true  	,true },
	{ "attack5"			,false	,false 	,true  	,false },

	{ "attack_Jump"		,false	,false 	,false	,false },
	{ "uppercut"		,false	,false 	,true	,false },

	{ "guard_start"		,false	,false 	,true 	,false },
	{ "guard_idle"		,false	,false 	,false	,false },
	{ "guard_block1"	,false	,false 	,false	,false },
	{ "guard_block1_ret",false	,false 	,false	,false },
	{ "guard_block2"	,false	,false 	,false	,false },
	{ "guard_cancel"	,false	,false 	,false	,false },

	{ "blast"			,false	,false 	,false 	,false },
	{ "blast_fall"		,false	,false 	,false 	,false },
	{ "blast_drop"		,false	,false 	,false 	,false },
	{ "blast_stand"		,false	,false 	,false 	,false },
	{ "blast_airmove"	,false	,false 	,false 	,false },

	{ "blast_dagger"	 ,false ,false 	,false 	,false },
	{ "blast_drop_dagger",false ,false 	,false 	,false },

	{ "damage"			,false	,false 	,false 	,false },
	{ "damage2"			,false	,false 	,false 	,false },
	{ "damage_down"		,false	,false 	,false 	,false },

	{ "taunt"			,true	,false	,false	,false },
	{ "bow"				,true	,false	,false	,false },
	{ "wave"			,true	,false	,false	,false },
	{ "laugh"			,true	,false	,false	,false },
	{ "cry"				,true	,false	,false	,false },
	{ "dance"			,true	,false	,false	,false },

	{ "cancel"			,false	,false 	,false 	,false },
	{ "charge"			,false	,false 	,true  	,true },
	{ "slash"			,false	,false 	,true  	,false },
	{ "jump_slash1"		,false	,false 	,false	,false },
	{ "jump_slash2"		,false	,false 	,false	,false },

	{ "lightning"		,false	,false 	,false	,false },
	{ "stun"			,false	,false 	,false	,false },

	{ "pit"				,false	,false 	,false	,false },
};

ZANIMATIONINFO g_AnimationInfoTableUpper[ZC_STATE_UPPER_END] = {
	{ ""				,true	,true	,false	,false },

	{ "attackS"			,false	,false	,false	,false },
	{ "reload"			,false	,false	,false	,false },
	{ "load"			,false	,false	,false	,false },

	{ "guard_start"		,false	,false 	,false	,false },
	{ "guard_idle"		,false	,false 	,false	,false },
	{ "guard_block1"	,false	,false 	,false	,false },
	{ "guard_block1_ret",false	,false 	,false	,false },
	{ "guard_block2"	,false	,false 	,false	,false },
	{ "guard_cancel"	,false	,false 	,false	,false },
};

RAnimationMgr * GetAnimationMgr(MMatchSex Sex)
{
	return AniMgrs[Sex];
}

void SetAnimationMgr(MMatchSex Sex, RAnimationMgr* AniMgr)
{
	AniMgrs[Sex] = AniMgr;
}

struct NodeInfo
{
	const char* Name = "";
	RMeshPartsPosInfoType PosParts;
	CutParts cut_parts;
	const NodeInfo* Parent;
	const NodeInfo* Child;
};

static const std::array<NodeInfo, eq_parts_pos_info_end> Nodes = []()
{
	std::array<NodeInfo, eq_parts_pos_info_end> infos;

	auto i = eq_parts_pos_info_Root;
	auto last_i = i;
	infos[i].Name = "Bip01";
	infos[i].PosParts = i;
	infos[i].cut_parts = cut_parts_lower_body;
	infos[i].Parent = nullptr;

	last_i = i;
	i = eq_parts_pos_info_Pelvis;
	infos[i].Name = "Bip01 Pelvis";
	infos[i].PosParts = i;
	infos[i].cut_parts = cut_parts_lower_body;
	infos[i].Parent = &Nodes[last_i];

	last_i = i;
	i = eq_parts_pos_info_Spine;
	infos[i].Name = "Bip01 Spine";
	infos[i].PosParts = i;
	infos[i].cut_parts = cut_parts_lower_body;
	infos[i].Parent = &Nodes[last_i];

	last_i = i;
	i = eq_parts_pos_info_Spine1;
	infos[i].Name = "Bip01 Spine1";
	infos[i].PosParts = i;
	infos[i].cut_parts = cut_parts_upper_body;
	infos[i].Parent = &Nodes[last_i];

	last_i = i;
	i = eq_parts_pos_info_Spine2;
	infos[i].Name = "Bip01 Spine2";
	infos[i].PosParts = i;
	infos[i].cut_parts = cut_parts_upper_body;
	infos[i].Parent = &Nodes[last_i];

	last_i = i;
	i = eq_parts_pos_info_Neck;
	infos[i].Name = "Bip01 Neck";
	infos[i].PosParts = i;
	infos[i].cut_parts = cut_parts_upper_body;
	infos[i].Parent = &Nodes[last_i];

	last_i = i;
	i = eq_parts_pos_info_Head;
	infos[i].Name = "Bip01 Head";
	infos[i].PosParts = i;
	infos[i].cut_parts = cut_parts_upper_body;
	infos[i].Parent = &Nodes[last_i];

	last_i = i;
	i = eq_parts_pos_info_HeadNub;
	infos[i].Name = "Bip01 HeadNub";
	infos[i].PosParts = i;
	infos[i].cut_parts = cut_parts_upper_body;
	infos[i].Parent = &Nodes[last_i];

	last_i = eq_parts_pos_info_Spine;
	i = eq_parts_pos_info_RThigh;
	infos[i].Name = "Bip01 R Thigh";
	infos[i].PosParts = i;
	infos[i].cut_parts = cut_parts_lower_body;
	infos[i].Parent = &Nodes[last_i];

	last_i = i;
	i = eq_parts_pos_info_RCalf;
	infos[i].Name = "Bip01 R Calf";
	infos[i].PosParts = i;
	infos[i].cut_parts = cut_parts_lower_body;
	infos[i].Parent = &Nodes[last_i];

	last_i = i;
	i = eq_parts_pos_info_RFoot;
	infos[i].Name = "Bip01 R Foot";
	infos[i].PosParts = i;
	infos[i].cut_parts = cut_parts_lower_body;
	infos[i].Parent = &Nodes[last_i];

	last_i = i;
	i = eq_parts_pos_info_RToe0;
	infos[i].Name = "Bip01 R Toe0";
	infos[i].PosParts = i;
	infos[i].cut_parts = cut_parts_lower_body;
	infos[i].Parent = &Nodes[last_i];

	last_i = i;
	i = eq_parts_pos_info_RToe0Nub;
	infos[i].Name = "Bip01 R Toe0Nub";
	infos[i].PosParts = i;
	infos[i].cut_parts = cut_parts_lower_body;
	infos[i].Parent = &Nodes[last_i];

	last_i = eq_parts_pos_info_Spine;
	i = eq_parts_pos_info_LThigh;
	infos[i].Name = "Bip01 L Thigh";
	infos[i].PosParts = i;
	infos[i].cut_parts = cut_parts_lower_body;
	infos[i].Parent = &Nodes[last_i];

	last_i = i;
	i = eq_parts_pos_info_LCalf;
	infos[i].Name = "Bip01 L Calf";
	infos[i].PosParts = i;
	infos[i].cut_parts = cut_parts_lower_body;
	infos[i].Parent = &Nodes[last_i];

	last_i = i;
	i = eq_parts_pos_info_LFoot;
	infos[i].Name = "Bip01 L Foot";
	infos[i].PosParts = i;
	infos[i].cut_parts = cut_parts_lower_body;
	infos[i].Parent = &Nodes[last_i];

	last_i = i;
	i = eq_parts_pos_info_LToe0;
	infos[i].Name = "Bip01 L Toe0";
	infos[i].PosParts = i;
	infos[i].cut_parts = cut_parts_lower_body;
	infos[i].Parent = &Nodes[last_i];

	last_i = i;
	i = eq_parts_pos_info_LToe0Nub;
	infos[i].Name = "Bip01 L Toe0Nub";
	infos[i].PosParts = i;
	infos[i].cut_parts = cut_parts_lower_body;
	infos[i].Parent = &Nodes[last_i];

	last_i = eq_parts_pos_info_Neck;
	i = eq_parts_pos_info_RClavicle;
	infos[i].Name = "Bip01 R Clavicle";
	infos[i].PosParts = i;
	infos[i].cut_parts = cut_parts_upper_body;
	infos[i].Parent = &Nodes[last_i];

	last_i = i;
	i = eq_parts_pos_info_RUpperArm;
	infos[i].Name = "Bip01 R UpperArm";
	infos[i].PosParts = i;
	infos[i].cut_parts = cut_parts_upper_body;
	infos[i].Parent = &Nodes[last_i];

	last_i = i;
	i = eq_parts_pos_info_RForeArm;
	infos[i].Name = "Bip01 R ForeArm";
	infos[i].PosParts = i;
	infos[i].cut_parts = cut_parts_upper_body;
	infos[i].Parent = &Nodes[last_i];

	last_i = i;
	i = eq_parts_pos_info_RHand;
	infos[i].Name = "Bip01 R Hand";
	infos[i].PosParts = i;
	infos[i].cut_parts = cut_parts_upper_body;
	infos[i].Parent = &Nodes[last_i];

	last_i = i;
	i = eq_parts_pos_info_RFinger0;
	infos[i].Name = "Bip01 R Finger0";
	infos[i].PosParts = i;
	infos[i].cut_parts = cut_parts_upper_body;
	infos[i].Parent = &Nodes[last_i];

	last_i = i;
	i = eq_parts_pos_info_RFingerNub;
	infos[i].Name = "Bip01 R Finger0Nub";
	infos[i].PosParts = i;
	infos[i].cut_parts = cut_parts_upper_body;
	infos[i].Parent = &Nodes[last_i];

	last_i = eq_parts_pos_info_Neck;
	i = eq_parts_pos_info_LClavicle;
	infos[i].Name = "Bip01 L Clavicle";
	infos[i].PosParts = i;
	infos[i].cut_parts = cut_parts_upper_body;
	infos[i].Parent = &Nodes[last_i];

	last_i = i;
	i = eq_parts_pos_info_LUpperArm;
	infos[i].Name = "Bip01 L UpperArm";
	infos[i].PosParts = i;
	infos[i].cut_parts = cut_parts_upper_body;
	infos[i].Parent = &Nodes[last_i];

	last_i = i;
	i = eq_parts_pos_info_LForeArm;
	infos[i].Name = "Bip01 L ForeArm";
	infos[i].PosParts = i;
	infos[i].cut_parts = cut_parts_upper_body;
	infos[i].Parent = &Nodes[last_i];

	last_i = i;
	i = eq_parts_pos_info_LHand;
	infos[i].Name = "Bip01 L Hand";
	infos[i].PosParts = i;
	infos[i].cut_parts = cut_parts_upper_body;
	infos[i].Parent = &Nodes[last_i];

	last_i = i;
	i = eq_parts_pos_info_LFinger0;
	infos[i].Name = "Bip01 L Finger0";
	infos[i].PosParts = i;
	infos[i].cut_parts = cut_parts_upper_body;
	infos[i].Parent = &Nodes[last_i];

	last_i = i;
	i = eq_parts_pos_info_LFingerNub;
	infos[i].Name = "Bip01 L Finger0Nub";
	infos[i].PosParts = i;
	infos[i].cut_parts = cut_parts_upper_body;
	infos[i].Parent = &Nodes[last_i];

	return infos;
}();

static void GetAniMat(rmatrix& mat, RAnimationNode& node, const rmatrix* parent_base_inv, int frame);
static void GetRotAniMat(rmatrix& mat, RAnimationNode& node, const rmatrix* parent_base_inv, int frame);
static void GetPosAniMat(rmatrix& mat, RAnimationNode& node, const rmatrix* parent_base_inv, int frame);
static void RotateSpine(rmatrix& mat, RMeshPartsPosInfoType parts, float y, float tremble);

template <size_t size>
static rmatrix GetNodeHierarchyMatrix(RAnimation* LowerAni, RAnimation* UpperAni,
	int LowerFrame, int UpperFrame,
	const RMeshPartsPosInfoType(&Hierarchy)[size],
	float y, float tremble)
{
	assert(LowerFrame >= 0);
	assert(UpperFrame >= 0);
	rmatrix last_mat;
	rmatrix last_mat_inv;
	rmatrix mat;
	rmatrix* last_mat_inv_ptr = nullptr;
	for (auto Parts : Hierarchy)
	{
		auto& nodeinfo = Nodes[Parts];

		RAnimation* Ani = nullptr;
		int Frame = 0;
		if (UpperAni)
		{
			if (nodeinfo.cut_parts == cut_parts_lower_body)
			{
				Ani = LowerAni;
				Frame = LowerFrame;
			}
			else
			{
				Ani = UpperAni;
				Frame = UpperFrame;
			}
		}
		else
		{
			Ani = LowerAni;
			Frame = LowerFrame;
		}

		auto& cur = *Ani->m_pAniData->GetNode(nodeinfo.Name);

		GetIdentityMatrix(mat);

		if (Parts == eq_parts_pos_info_Spine1 && UpperAni)
		{
			GetUpperSpine1(mat, UpperAni, UpperFrame, y, tremble);
		}
		else
		{
			GetAniMat(mat, cur, last_mat_inv_ptr, Frame);
			RotateSpine(mat, Parts, y, tremble);
		}

		if (last_mat_inv_ptr)
			mat *= last_mat;

		last_mat = mat;
		RMatInv(last_mat_inv, cur.m_mat_base);
		last_mat_inv_ptr = &last_mat_inv;
	}

	return mat;
}

template <size_t size>
static rmatrix LogNodeHierarchyMatrix(RAnimation* LowerAni, RAnimation* UpperAni,
	int LowerFrame, int UpperFrame,
	const RMeshPartsPosInfoType(&Hierarchy)[size],
	float y, float tremble)
{
	ASSERT(LowerFrame >= 0);
	ASSERT(UpperFrame >= 0);
	rmatrix last_mat;
	rmatrix last_mat_inv;
	rmatrix mat;
	rmatrix* last_mat_inv_ptr = nullptr;
	for (auto Parts : Hierarchy)
	{
		auto& nodeinfo = Nodes[Parts];

		RAnimation* Ani = nullptr;
		int Frame = 0;
		if (UpperAni)
		{
			if (nodeinfo.cut_parts == cut_parts_lower_body)
			{
				Ani = LowerAni;
				Frame = LowerFrame;
			}
			else
			{
				Ani = UpperAni;
				Frame = UpperFrame;
			}
		}
		else
		{
			Ani = LowerAni;
			Frame = LowerFrame;
		}

		auto& cur = *Ani->m_pAniData->GetNode(nodeinfo.Name);

		GetIdentityMatrix(mat);

		if (Parts == eq_parts_pos_info_Spine1 && UpperAni)
		{
			GetUpperSpine1(mat, UpperAni, UpperFrame, y, tremble);
		}
		else
		{
			GetAniMat(mat, cur, last_mat_inv_ptr, Frame);
			RotateSpine(mat, Parts, y, tremble);
		}

		DMLog("%s: trans = %f, %f, %f\n",
			cur.GetName(),
			GetTransPos(mat).x, GetTransPos(mat).y, GetTransPos(mat).z);

		if (last_mat_inv_ptr)
			mat *= last_mat;

		last_mat = mat;
		RMatInv(last_mat_inv, cur.m_mat_base);
		last_mat_inv_ptr = &last_mat_inv;
	}

	return mat;
}

bool GetNodeMatrix(rmatrix& mat, const char* Name, const rmatrix* parent_base_inv,
	RAnimation* Ani, int Frame, float y, float tremble)
{
	const NodeInfo* nodeinfo = nullptr;
	for (auto& node : Nodes)
	{
		if (!strcmp(node.Name, Name))
		{
			nodeinfo = &node;
			break;
		}
	}
	
	//DMLog("%s: %p, %p\n", Name, nodeinfo, Ani);
	
	if (!nodeinfo)
		return false;

	if (!Ani)
		return false;

	auto& cur = *Ani->m_pAniData->GetNode(nodeinfo->Name);
	GetIdentityMatrix(mat);
	GetAniMat(mat, cur, parent_base_inv, Frame);
	RotateSpine(mat, nodeinfo->PosParts, y, tremble);
	
	return true;
}

bool GetUpperSpine1(rmatrix& mat, RAnimation* Ani, int Frame, float y, float tremble)
{
	auto& spine1 = *Ani->m_pAniData->GetNode("Bip01 Spine1");
	auto& spine = *Ani->m_pAniData->GetNode("Bip01 Spine");
	auto spine_base_inv = spine.m_mat_base;
	RMatInv(spine_base_inv, spine_base_inv);
	GetAniMat(mat, spine1, &spine_base_inv, Frame);
	RotateSpine(mat, eq_parts_pos_info_Spine1, y, tremble);
	return true;
}

v3 GetHeadPosition(RAnimation* LowerAni, RAnimation* UpperAni,
	int LowerFrame, int UpperFrame, float y, float tremble)
{
	static const RMeshPartsPosInfoType Hierarchy[] = { eq_parts_pos_info_Root, eq_parts_pos_info_Pelvis,
		eq_parts_pos_info_Spine, eq_parts_pos_info_Spine1, eq_parts_pos_info_Spine2,
		eq_parts_pos_info_Neck, eq_parts_pos_info_Head };

	/*DMLog("GetHeadPosition -- %p, %p, %d, %d, %f, %f\n", LowerAni, UpperAni, LowerFrame, UpperFrame,
		y, tremble);*/

	auto mat = GetNodeHierarchyMatrix(LowerAni, UpperAni, LowerFrame, UpperFrame,
		Hierarchy, y, tremble);

	/*v3 trans = GetTransPos(mat);

	if (trans.y < 100)
	{
		DMLog("GetHeadPosition -- %s, %s, %p, %p, %d/%d, %d/%d, %f, %f; trans: %f, %f, %f\n",
			LowerAni->GetName(), UpperAni ? UpperAni->GetName() : "(null)",
			LowerAni, UpperAni,
			LowerFrame, LowerAni->GetMaxFrame(),
			UpperFrame, UpperAni ? UpperAni->GetMaxFrame() : 0,
			y, tremble,
			trans.x, trans.y, trans.z);

		LogNodeHierarchyMatrix(LowerAni, UpperAni, LowerFrame, UpperFrame,
			Hierarchy, y, tremble);
	}*/

	return GetTransPos(mat);
}

v3 GetFootPosition(RAnimation* LowerAni, int Frame)
{
	static const RMeshPartsPosInfoType HierarchyR[] = { eq_parts_pos_info_Root, eq_parts_pos_info_Pelvis,
		eq_parts_pos_info_Spine, eq_parts_pos_info_RThigh, eq_parts_pos_info_RCalf, eq_parts_pos_info_RFoot };
	static const RMeshPartsPosInfoType HierarchyL[] = { eq_parts_pos_info_Root, eq_parts_pos_info_Pelvis,
		eq_parts_pos_info_Spine, eq_parts_pos_info_LThigh, eq_parts_pos_info_LCalf, eq_parts_pos_info_LFoot };

	auto matR = GetNodeHierarchyMatrix(LowerAni, nullptr, Frame, 0 ,HierarchyR, 0, 0);
	auto matL = GetNodeHierarchyMatrix(LowerAni, nullptr, Frame, 0, HierarchyL, 0, 0);

	v3 ret = GetTransPos(matR) + GetTransPos(matL);
	ret /= 2;
	ret.y -= 12;
	return ret;
}

static void GetAniMat(rmatrix& mat, RAnimationNode& node, const rmatrix* parent_base_inv, int frame)
{
	if (node.m_mat_cnt)
	{
		mat = node.GetTMValue(frame);
		return;
	}

	bool HasPosValue = node.m_pos_cnt != 0;
	bool HasRotValue = node.m_rot_cnt != 0;

	if (!HasPosValue && !HasRotValue)
	{
		mat = node.m_mat_base;

		if (parent_base_inv)
			mat *= *parent_base_inv;

		return;
	}

	GetRotAniMat(mat, node, parent_base_inv, frame);
	GetPosAniMat(mat, node, parent_base_inv, frame);
}

static void GetRotAniMat(rmatrix& mat, RAnimationNode& node, const rmatrix* parent_base_inv, int frame)
{
	rmatrix buffer;

	if (node.m_rot_cnt) {
		mat = QuaternionToMatrix(node.GetRotValue(frame));
	}
	else {

		GetIdentityMatrix(buffer);

		buffer = node.m_mat_base;

		if (parent_base_inv)
			buffer *= *parent_base_inv;

		buffer._41 = buffer._42 = buffer._43 = 0;
		mat *= buffer;
	}
}

static void GetPosAniMat(rmatrix& mat, RAnimationNode& node, const rmatrix* parent_base_inv, int frame)
{
	rmatrix buffer;

	if (node.m_pos_cnt) {
		auto pos = node.GetPosValue(frame);

		for (int i = 0; i < 3; i++)
			mat(3, i) = pos[i];
	}
	else {
		GetIdentityMatrix(buffer);

		buffer = node.m_mat_base;

		if (parent_base_inv)
			buffer *= *parent_base_inv;

		for (int i = 0; i < 3; i++)
			mat(3, i) = buffer(3, i);
	}
}

static void RotateSpine(rmatrix& mat, RMeshPartsPosInfoType parts, float y, float tremble)
{
	float ratio = 0;

	switch (parts)
	{
	case eq_parts_pos_info_Head:
		ratio = 0.3f;
		break;
	case eq_parts_pos_info_Spine1:
		ratio = 0.6f;
		break;
	case eq_parts_pos_info_Spine2:
		ratio = 0.5f;
		break;
	default:
		return;
	};

	float y_clamped = y;

	constexpr auto MAX_YA_FRONT = 50.f;
	constexpr auto MAX_YA_BACK = -70.f;

	if (y_clamped > MAX_YA_FRONT)
		y_clamped = MAX_YA_FRONT;

	if (y_clamped < MAX_YA_BACK)
		y_clamped = MAX_YA_BACK;

	auto my = RGetRotY((y_clamped + tremble) * ratio);

	mat *= my;
}

static float GetSpeed(ZC_STATE_LOWER LowerState, int MaxFrame, MMatchItemDesc* ItemDesc)
{
	float ret = 4.8f;

	[&]()
	{
		if (!ItemDesc)
			return;

		if (ItemDesc->m_nType != MMIT_MELEE)
			return;

		switch (LowerState)
		{
		case ZC_STATE_LOWER_ATTACK1:
		case ZC_STATE_LOWER_ATTACK1_RET:
		case ZC_STATE_LOWER_ATTACK2:
		case ZC_STATE_LOWER_ATTACK2_RET:
		case ZC_STATE_LOWER_ATTACK3:
		case ZC_STATE_LOWER_ATTACK3_RET:
		case ZC_STATE_LOWER_ATTACK4:
		case ZC_STATE_LOWER_ATTACK4_RET:
		case ZC_STATE_LOWER_ATTACK5:
		case ZC_STATE_LOWER_JUMPATTACK:
			break;
		case ZC_STATE_SLASH:
			ret *= 0.9f;
			return;
		default:
			return;
		};

		int Delay = GetSelectWeaponDelay(ItemDesc);

		int Time = static_cast<int>(MaxFrame / 4.8);

		int AttackSpeed = Time + Delay;
		if (AttackSpeed < 1)
			AttackSpeed = 1;

		ret = float(Time) / AttackSpeed * 4.8f;
	}();

	return ret;
}

int GetSelectWeaponDelay(MMatchItemDesc* pSelectItemDesc)
{
	if (!pSelectItemDesc) return 0;

	int nReturnDelay = pSelectItemDesc->m_nDelay;

	if (pSelectItemDesc->m_nType != MMIT_MELEE)
		return 0;

	switch (pSelectItemDesc->m_nWeaponType)
	{
	case MWT_DAGGER:
		nReturnDelay -= 266;
		break;

	case MWT_DUAL_DAGGER:
		nReturnDelay -= 299;
		break;

	case MWT_KATANA:
		nReturnDelay -= 299;
		break;

	case MWT_DOUBLE_KATANA:
		nReturnDelay -= 299;
		break;

	case MWT_GREAT_SWORD:
		nReturnDelay -= 399;
		break;

	default:
		_ASSERT(0);
		break;
	}

	return nReturnDelay;
}

int GetFrame(RAnimation& Ani, ZC_STATE_LOWER LowerState, MMatchItemDesc* ItemDesc, float Time)
{
	if (Time < 0)
		Time = 0;

	int Frame = static_cast<int>(Time * 1000 * GetSpeed(LowerState, Ani.GetMaxFrame(), ItemDesc));

	if (Ani.GetAnimationLoopType() == RAniLoopType_Loop)
		Frame %= Ani.GetMaxFrame();
	else if (Frame >= Ani.GetMaxFrame())
		Frame = Ani.GetMaxFrame() - 1;

	return Frame;
}

v3 GetAbsHead(const v3 & Origin, const v3 & Dir, MMatchSex Sex,
	ZC_STATE_LOWER LowerState, ZC_STATE_UPPER UpperState,
	int LowerFrame, int UpperFrame,
	RWeaponMotionType MotionType, bool IsDead)
{
	auto LowerAni = GetAnimationMgr(Sex)->GetAnimation(g_AnimationInfoTableLower[LowerState].Name, MotionType);
	RAnimation* UpperAni = nullptr;
	bool HasUpperAni = UpperState != ZC_STATE_UPPER_NONE;
	if (HasUpperAni)
		UpperAni = GetAnimationMgr(Sex)->GetAnimation(g_AnimationInfoTableUpper[UpperState].Name, MotionType);
	if (!LowerAni)
		return Origin + v3(0, 0, 180);

	float y = IsDead ? 0 : (Dir.z + 0.05f) * 50;

	v3 Head = GetHeadPosition(LowerAni, UpperAni, LowerFrame, UpperFrame, y, 0);

	v3 xydir = Dir;
	xydir.z = 0;
	Normalize(xydir);

	v3 AdjPos = Origin;

	if (g_AnimationInfoTableLower[LowerState].bMove)
	{
		v3 Foot = GetFootPosition(LowerAni, LowerFrame);

		rmatrix WorldRot;
		MakeWorldMatrix(&WorldRot, { 0, 0, 0 }, xydir, { 0, 0, 1 });

		Foot *= WorldRot;

		AdjPos = Origin - Foot;
	}

	rmatrix World;
	MakeWorldMatrix(&World, AdjPos, xydir, v3(0, 0, 1));

	return Head * World;
}
