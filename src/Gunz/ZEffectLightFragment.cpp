#include "stdafx.h"

#include <MMSystem.h>
#include "ZEffectLightFragment.h"
#include "RealSpace2.h"
#include "Physics.h"

namespace ZEffectLightFragmentConst
{
constexpr float LIGHTTRACER_LENGTH = 4.0f;
constexpr float LIGHTTRACER_WIDTH = 0.8f;
constexpr float LIGHTFRAGMENT_LIFETIME = 600;
}

ZEffectLightFragment::ZEffectLightFragment(ZEffectBillboardSource* pEffectBillboardSource,
	const rvector& Pos, const rvector& Velocity)
	: ZEffectBillboard(pEffectBillboardSource)
{
	using namespace ZEffectLightFragmentConst;

	m_nStartTime = GetGlobalTimeMS();
	m_nPrevTime = m_nStartTime;
	m_OrigPos = m_Pos = Pos;
	m_Velocity = Velocity;
	m_Scale.x = LIGHTTRACER_LENGTH;
	m_Scale.y = LIGHTTRACER_WIDTH;
	m_Scale.z = 1;

	m_nDrawMode = ZEDM_ADD;
}

bool ZEffectLightFragment::Draw(u64 nTime)
{
	using namespace ZEffectLightFragmentConst;

	auto dwDiff = nTime-m_nStartTime;
	auto dwPrevDiff = nTime-m_nPrevTime;

	float fSec = (float)dwDiff/1000.0f;
	rvector Distance = ParabolicMotion(m_Velocity, fSec) * 100;
	rvector NewPos = m_OrigPos + Distance;
	rvector Acceleration = NewPos - m_Pos;
	m_Pos = NewPos;
	m_fOpacity = (LIGHTFRAGMENT_LIFETIME-dwDiff)/(float)LIGHTFRAGMENT_LIFETIME;

	rvector right = CrossProduct(Acceleration, RCameraDirection);
	m_Normal = CrossProduct(Acceleration, right);
	m_Up = CrossProduct(m_Normal, Acceleration);

	if(dwDiff>LIGHTFRAGMENT_LIFETIME) return false;

	ZEffectBillboard::Draw(nTime);
	m_nPrevTime = nTime;

	return true;
}

ZEffectLightFragment2::ZEffectLightFragment2(LPDIRECT3DTEXTURE9 pEffectBillboardTexture,
	const rvector& Pos, const rvector& Velocity)
	: ZEffectBillboard2(pEffectBillboardTexture)
{
	using namespace ZEffectLightFragmentConst;

	m_nStartTime = GetGlobalTimeMS();
	m_nPrevTime = m_nStartTime;
	m_OrigPos = m_Pos = Pos;
	m_Velocity = Velocity;
	m_Scale.x = LIGHTTRACER_WIDTH;
	m_Scale.y = LIGHTTRACER_WIDTH;
	m_Scale.z = 1;

	m_nDrawMode = ZEDM_ALPHAMAP;
}

bool ZEffectLightFragment2::Draw(u64 nTime)
{
	using namespace ZEffectLightFragmentConst;

	auto dwDiff = nTime - m_nStartTime;
	auto dwPrevDiff = nTime - m_nPrevTime;

	float fSec = (float)dwDiff/1000.0f;
	rvector Distance = ParabolicMotion(m_Velocity, fSec) * 100;
	rvector NewPos = m_OrigPos + Distance;
	rvector Acceleration = NewPos - m_Pos;
	m_Pos = NewPos;
	m_fOpacity = (LIGHTFRAGMENT_LIFETIME - dwDiff) / (float)LIGHTFRAGMENT_LIFETIME;

	rvector right = CrossProduct(Acceleration, RCameraDirection);
	m_Normal = CrossProduct(Acceleration, right);
	m_Up = CrossProduct(m_Normal, Acceleration);

	if (dwDiff > LIGHTFRAGMENT_LIFETIME) return false;

	ZEffectBillboard2::Draw(nTime);
	m_nPrevTime = nTime;

	return true;
}