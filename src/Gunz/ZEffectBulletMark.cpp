#include "stdafx.h"

#include <MMSystem.h>

#include "ZEffectBulletMark.h"

#define BULLETMARK_SCALE	5.0f

static int g_bullet_mark_cnt = 0;

ZEffectBulletMark::ZEffectBulletMark(ZEffectBillboardSource* pEffectBillboardSource, const rvector& Pos, const rvector& Normal)
: ZEffectBillboard(pEffectBillboardSource)
{
	m_nStartTime = GetGlobalTimeMS();
	m_Pos = Pos;
	m_Normal = Normal;
	m_Scale.x = m_Scale.y = m_Scale.z = BULLETMARK_SCALE;
	m_nDrawMode = ZEDM_ALPHAMAP;
}

#define BULLETMARK_LIFE_TIME		10000		// BULLETMARK Life Time
#define BULLETMARK_VANISH_TIME		1000		// BULLETMARK Life Time

bool ZEffectBulletMark::Draw(u64 nTime)
{
	auto dwDiff = nTime-m_nStartTime;

	if(BULLETMARK_LIFE_TIME-dwDiff<BULLETMARK_VANISH_TIME){
		m_fOpacity = (BULLETMARK_LIFE_TIME-dwDiff)/(float)BULLETMARK_VANISH_TIME;
	}
	if(dwDiff>BULLETMARK_LIFE_TIME) return false;

	ZEffectBillboard::Draw(nTime);

	return true;
}