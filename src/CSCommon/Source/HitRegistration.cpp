#include "stdafx.h"
#include "HitRegistration.h"
#include "RMath.h"
#include "RBspObject.h"

using namespace RealSpace2;

void CalcRangeShotControllability(v3& vOutDir, const v3& vSrcDir,
	int nControllability, u32 seed, float CtrlFactor)
{
	std::mt19937 rng;
	rng.seed(seed);

	float MaxSpread{};
	if (nControllability > 0)
		MaxSpread = nControllability * CtrlFactor / 1000.f;

	vOutDir = GetSpreadDir(vSrcDir, MaxSpread, rng);
}

ZOBJECTHITTEST PlayerHitTest(const v3& head, const v3& foot,
	const v3& src, const v3& dest, v3* pOutPos)
{
	v3 footpos, headpos;

	footpos = foot;
	headpos = head;

	footpos.z += 5.f;
	headpos.z += 5.f;

	auto rootpos = (footpos + headpos)*0.5f;

	auto nearest = GetNearestPoint(headpos, src, dest);
	float fDist = Magnitude(nearest - headpos);
	float fDistToChar = Magnitude(nearest - src);

	v3 ap, cp;

	// Head
	if (fDist < 15.f)
	{
		if (pOutPos) *pOutPos = nearest;
		return ZOH_HEAD;
	}

	auto dir = dest - src;
	Normalize(dir);

	auto rootdir = (rootpos - headpos);
	Normalize(rootdir);
	fDist = GetDistanceBetweenLineSegment(src, dest, headpos + 20.f*rootdir,
		rootpos - 20.f*rootdir, &ap, &cp);

	// Body
	if (fDist < 30)
	{
		auto ap2cp = ap - cp;
		float fap2cpsq = MagnitudeSq(ap2cp);
		float fdiff = sqrtf(30.f*30.f - fap2cpsq);

		if (pOutPos) *pOutPos = ap - dir*fdiff;
		return ZOH_BODY;
	}

	fDist = GetDistanceBetweenLineSegment(src, dest, rootpos - 20.f*rootdir,
		footpos, &ap, &cp);

	// Legs
	if (fDist < 30)
	{
		auto ap2cp = ap - cp;
		float fap2cpsq = MagnitudeSq(ap2cp);
		float fdiff = sqrtf(30.f*30.f - fap2cpsq);

		if (pOutPos) *pOutPos = ap - dir*fdiff;
		return ZOH_LEGS;
	}

	return ZOH_NONE;
}

void RBspObject_PickTo(RBspObject* a, v3 b, v3 c, RBSPPICKINFO* d, u32 e)
{
	a->PickTo(b, c, d, e);
}
