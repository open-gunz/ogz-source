#pragma once

#include "GlobalTypes.h"
#include "MMath.h"
#include <random>
#include "RTypes.h"
#include "RMath.h"
#include "RBspObject.h"

template <typename rngT>
float RandomAngle(rngT& rng)
{
	std::uniform_real_distribution<float> dist(0, TAU_FLOAT);
	return dist(rng);
}

template <typename rngT>
v3 GetSpreadDir(const v3& dir, float MaxSpread, rngT& rng)
{
	using namespace RealSpace2;

	std::uniform_real_distribution<float> dist(0, MaxSpread);
	auto Force = dist(rng);

	v3 right = Normalized(CrossProduct(dir, { 0, 0, 1 }));
	// Rotate down by the force. This controls the width of the spread.
	rmatrix mat = RotationMatrix(right, Force);
	v3 ret = dir * mat;

	// Rotate new vector randomly around original vector
	mat = RotationMatrix(dir, RandomAngle(rng));
	ret *= mat;

	return ret;
}

void CalcRangeShotControllability(v3& vOutDir, const v3& vSrcDir,
	int nControllability, u32 seed, float CtrlFactor);

#define SHOTGUN_BULLET_COUNT	12
#define SHOTGUN_DIFFUSE_RANGE	0.1f

template <typename rngT>
v3 GetShotgunPelletDir(const v3& dir, rngT& rng)
{
	return GetSpreadDir(dir, SHOTGUN_DIFFUSE_RANGE, rng);
}

inline auto GetShotgunPelletDirGenerator(const v3& orig_dir, u32 seed)
{
	return [&orig_dir, rng = std::mt19937(seed)]() mutable
	{
		return GetShotgunPelletDir(orig_dir, rng);
	};
}

enum ZOBJECTHITTEST {
	ZOH_NONE = 0,
	ZOH_BODY = 1,
	ZOH_HEAD = 2,
	ZOH_LEGS = 3
};

ZOBJECTHITTEST PlayerHitTest(const v3& head, const v3& foot,
	const v3& src, const v3& dest, v3* pOutPos = nullptr);

namespace RealSpace2
{
	class RBspObject;
}

template <typename ContainerT, typename PickInfoT>
bool PickHistory(typename ContainerT::value_type Exception,
	const v3& src, const v3& dest,
	RealSpace2::RBspObject* BspObject, PickInfoT& pickinfo, const ContainerT& Container,
	double Time, u32 PassFlag = RM_FLAG_ADDITIVE | RM_FLAG_USEOPACITY | RM_FLAG_HIDE)
{
	using namespace RealSpace2;

	decltype(Exception) HitObject = nullptr;
	v3 HitPos;
	pickinfo.info.t = 0;

	for (auto* Obj : Container)
	{
		if (Exception == Obj)
			continue;

		v3 TempHitPos;
		auto HitParts = Obj->HitTest(src, dest, Time, &TempHitPos);

		if (HitParts == ZOH_NONE)
			continue;

		if (Obj->IsDead())
			continue;

		if (!HitObject || Magnitude(TempHitPos - src) < Magnitude(HitPos - src))
		{
			HitObject = Obj;
			HitPos = TempHitPos;
			switch (HitParts)
			{
			case ZOH_HEAD: pickinfo.info.parts = eq_parts_head; break;
			case ZOH_BODY: pickinfo.info.parts = eq_parts_chest; break;
			case ZOH_LEGS:	pickinfo.info.parts = eq_parts_legs; break;
			}
			pickinfo.info.vOut = TempHitPos;
		}
	}

#define DIDNT_HIT_BSP()					\
	pickinfo.bBspPicked = false;		\
	pickinfo.pObject = HitObject;		\
	return pickinfo.pObject != nullptr;	\


	if (!BspObject)
	{
		DIDNT_HIT_BSP();
	}

	bool HitBsp = BspObject->PickTo(src, dest, &pickinfo.bpi, PassFlag);
	if (!HitBsp)
	{
		DIDNT_HIT_BSP();
	}

	if (HitObject && Magnitude(HitPos - src) < Magnitude(pickinfo.bpi.PickPos - src))
	{
		DIDNT_HIT_BSP();
	}

	pickinfo.bBspPicked = true;
	pickinfo.pObject = nullptr;

	return true;

#undef DIDNT_HIT_BSP
}

template <typename ObjectT, typename ContainerT, typename GetOriginT>
void GrenadeExplosion(const ObjectT& Owner, const ContainerT& Container, const v3& ExplosionPos,
	int Damage, float fRange, float fMinDamage, float fKnockBack, const GetOriginT& GetOrigin)
{
	using namespace RealSpace2;

	float fDist, fDamageRange;

	for (auto* Target : Container)
	{
		if (!Target || !Target->IsAlive())
			continue;

		v3 TargetOrigin;
		GetOrigin(*Target, TargetOrigin);

		fDist = Magnitude(ExplosionPos - (TargetOrigin + v3(0, 0, 80)));
		if (fDist > fRange)
			continue;

		auto dir = ExplosionPos - (TargetOrigin + v3(0, 0, 80));
		Normalize(dir);

		if (GetDistance(ExplosionPos, TargetOrigin + v3(0, 0, 50), TargetOrigin + v3(0, 0, 130)) < 50)
		{
			fDamageRange = 1.f;
		}
		else
		{
			constexpr auto MAX_DMG_RANGE = 50.f;

			fDamageRange = 1.f - (1.f - fMinDamage)*(max(fDist - MAX_DMG_RANGE, 0.f) / (fRange - MAX_DMG_RANGE));
		}

		float fActualDamage = Damage * fDamageRange;
		float fRatio = GetPiercingRatio(MWT_FRAGMENTATION, eq_parts_chest);
		Target->OnDamaged(Owner, ExplosionPos,
			ZD_EXPLOSION, MWT_FRAGMENTATION,
			static_cast<int>(fActualDamage), fRatio);
	}
}