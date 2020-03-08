#include "stdafx.h"
#include "BasicInfoHistory.h"
#include "RAnimation.h"
#include "RAnimationMgr.h"
using namespace RealSpace2;

void BasicInfoHistoryManager::AddBasicInfo(BasicInfoItem bii)
{
	if (BasicInfoList.empty())
	{
		bii.LowerFrameTime = 0;
		bii.UpperFrameTime = bii.upperstate == ZC_STATE_UPPER_NONE ? -1.f : 0.f;
		if (bii.lowerstate == -1)
		{
			bii.lowerstate = ZC_STATE_LOWER_IDLE1;
			bii.upperstate = ZC_STATE_UPPER_NONE;
		}

		if (bii.SelectedSlot == -1)
			bii.SelectedSlot = MMCIP_PRIMARY;
	}
	else
	{
		auto prev_it = BasicInfoList.begin();

		if (bii.lowerstate == -1)
		{
			bii.lowerstate = prev_it->lowerstate;
			bii.upperstate = prev_it->upperstate;
		}

		if (bii.SelectedSlot == -1)
			bii.SelectedSlot = prev_it->SelectedSlot;

		if (prev_it->lowerstate != bii.lowerstate)
		{
			bii.LowerFrameTime = 0;
		}
		else
		{
			bii.LowerFrameTime = prev_it->LowerFrameTime + float(bii.RecvTime - prev_it->RecvTime);
		}

		if (prev_it->upperstate != bii.upperstate)
		{
			bii.UpperFrameTime = bii.upperstate == ZC_STATE_UPPER_NONE ? -1.f : 0.f;
		}
		else
		{
			bii.UpperFrameTime = bii.upperstate == ZC_STATE_UPPER_NONE ? -1
				: prev_it->UpperFrameTime + float(bii.RecvTime - prev_it->RecvTime);
		}
	}

	BasicInfoList.push_front(bii);

	while (BasicInfoList.size() > 1000)
	{
		BasicInfoList.pop_back();
	}
}

template <typename Iterator>
static v3 GetHead(const v3& Pos, const v3& Dir, const Iterator& pre_it,
	float LowerFrameTime, float UpperFrameTime, bool IsDead, MMatchSex Sex,
	function_view<MMatchItemDesc*(MMatchCharItemParts)> GetItemDesc)
{
	auto ItemDesc = GetItemDesc(pre_it->SelectedSlot);
	auto MotionType = eq_weapon_etc;
	if (ItemDesc)
		MotionType = WeaponTypeToMotionType(ItemDesc->m_nWeaponType);

	auto LowerAni = GetAnimationMgr(Sex)->GetAnimation(g_AnimationInfoTableLower[pre_it->lowerstate].Name, MotionType);
	RAnimation* UpperAni = nullptr;
	bool HasUpperAni = pre_it->upperstate != ZC_STATE_UPPER_NONE;
	if (HasUpperAni)
		UpperAni = GetAnimationMgr(Sex)->GetAnimation(g_AnimationInfoTableUpper[pre_it->upperstate].Name, MotionType);
	if (!LowerAni)
		return Pos + v3(0, 0, 180);

	int LowerFrame = GetFrame(*LowerAni, pre_it->lowerstate, ItemDesc, LowerFrameTime);
	int UpperFrame = 0;
	if (HasUpperAni)
		UpperFrame = GetFrame(*UpperAni, ZC_STATE_LOWER(0), nullptr, UpperFrameTime);

	return GetAbsHead(Pos, Dir, Sex,
		pre_it->lowerstate, pre_it->upperstate,
		LowerFrame, UpperFrame,
		MotionType, IsDead);
}

bool BasicInfoHistoryManager::GetInfo(const Info& Out, double Time,
	function_view<MMatchItemDesc*(MMatchCharItemParts)> GetItemDesc,
	MMatchSex Sex, bool IsDead) const
{
	// Using a macro instead of a lambda to avoid evaluation of arguments if unused.
	// I.e., we don't want to call the expensive GetHead function if Out.Head is null.
#define SET_RETURN_VALUES(srv_head, srv_pos, srv_dir, srv_cameradir) \
	do { \
		if (Out.Head) \
		{ \
			*Out.Head = srv_head; \
		} \
		if (Out.Pos) \
		{ \
			*Out.Pos = srv_pos; \
		} \
		if (Out.Dir) \
		{ \
			*Out.Dir = srv_dir; \
		} \
		if (Out.CameraDir) \
		{ \
			*Out.CameraDir = srv_cameradir; \
		} \
	}  while (false)

	if (BasicInfoList.empty())
	{
		v3 Head{0, 0, 180};
		v3 Pos{0, 0, 0};
		v3 Dir{1, 0, 0};
		v3 CameraDir{Dir};
		SET_RETURN_VALUES(Head, Pos, Dir, CameraDir);
		return true;
	}

	if (BasicInfoList.size() == 1)
	{
		auto it = BasicInfoList.begin();
		SET_RETURN_VALUES(
			GetHead(it->position, it->direction, it,
				it->LowerFrameTime, it->UpperFrameTime,
				IsDead, Sex, GetItemDesc),
			it->position,
			it->direction,
			it->cameradir
		);
		return true;
	}

	auto pre_it = BasicInfoList.begin();
	auto post_it = BasicInfoList.begin();

	while (pre_it != BasicInfoList.end() && Time < pre_it->RecvTime)
	{
		post_it = pre_it;
		pre_it++;
	}

	if (pre_it == BasicInfoList.end())
		pre_it = post_it;

	v3 AbsPos;
	v3 Dir;
	v3 CameraDir;
	float LowerFrameTime;
	float UpperFrameTime;

	if (pre_it != post_it)
	{
		auto t = float(Time - pre_it->RecvTime) / float(post_it->RecvTime - pre_it->RecvTime);
		AbsPos = Lerp(pre_it->position, post_it->position, t);
		Dir = Slerp(pre_it->direction, post_it->direction, t);
		CameraDir = Slerp(pre_it->cameradir, post_it->cameradir, t);
		LowerFrameTime = Lerp(pre_it->LowerFrameTime, post_it->LowerFrameTime, t);
		UpperFrameTime = Lerp(pre_it->UpperFrameTime, post_it->UpperFrameTime, t);
	}
	else
	{
		AbsPos = pre_it->position;
		Dir = pre_it->direction;
		CameraDir = pre_it->cameradir;
		LowerFrameTime = pre_it->LowerFrameTime + float(Time - pre_it->RecvTime);
		UpperFrameTime = pre_it->UpperFrameTime + float(Time - pre_it->RecvTime);
	}

	SET_RETURN_VALUES(
		GetHead(AbsPos, Dir, pre_it,
			LowerFrameTime, UpperFrameTime,
			IsDead, Sex, GetItemDesc),
		AbsPos,
		Dir,
		CameraDir
	);

	return true;
}