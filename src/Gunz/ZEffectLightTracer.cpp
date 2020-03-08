#include "stdafx.h"

#include "ZEffectLightTracer.h"
#include "RealSpace2.h"

namespace ZEffectLightTracerConst
{
constexpr float LIGHTTRACER_LENGTH = 100;
constexpr float LIGHTTRACER_WIDTH = 1;
constexpr float LIGHTTRACER_SPEED = 10.0f;	// cm/msec
}

ZEffectLightTracer::ZEffectLightTracer(ZEffectBillboardSource* pEffectBillboardSource,
	const rvector& Start, const rvector& End)
: ZEffectBillboard(pEffectBillboardSource)
{
	using namespace ZEffectLightTracerConst;

	m_nStartTime = GetGlobalTimeMS();
	m_Start = Start;
	m_End = End;
	m_Scale.x = LIGHTTRACER_LENGTH;
	m_Scale.y = LIGHTTRACER_WIDTH;
	m_Scale.z = 1;

	m_nDrawMode = ZEDM_ADD;

	m_LightTracerDir = m_End-m_Start;
	m_fLength = Magnitude(m_LightTracerDir);
	Normalize(m_LightTracerDir);

	m_Pos = m_Start;
}

bool ZEffectLightTracer::Draw(u64 nTime)
{
	using namespace ZEffectLightTracerConst;

	auto dwDiff = nTime-m_nStartTime;

	rvector right = CrossProduct(m_LightTracerDir, RCameraDirection);
	m_Normal = CrossProduct(m_LightTracerDir, right);
	m_Up = CrossProduct(m_Normal, m_LightTracerDir);
	m_Pos += (m_LightTracerDir*LIGHTTRACER_SPEED*(float)dwDiff);
	m_fLength -= (LIGHTTRACER_SPEED*dwDiff);

	if(m_fLength<0) return false;

	ZEffectBillboard::Draw(nTime);

	return true;
}