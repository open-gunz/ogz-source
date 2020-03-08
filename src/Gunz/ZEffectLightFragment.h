#pragma once

#include "ZEffectBillboard.h"
#include "RTypes.h"

#include "mempool.h"

class ZEffectLightFragment : public ZEffectBillboard , public CMemPoolSm<ZEffectLightFragment>
{
protected:
	u64 m_nStartTime;
	u64 m_nPrevTime;

	rvector	m_OrigPos;
	rvector	m_Velocity;

public:
	ZEffectLightFragment(ZEffectBillboardSource* pEffectBillboardSource,
		const rvector& Pos, const rvector& Velocity);

	virtual bool Draw(u64 nTime) override;
};

class ZEffectLightFragment2 : public ZEffectBillboard2 , public CMemPoolSm<ZEffectLightFragment2>
{
protected:
	u64 m_nStartTime;
	u64 m_nPrevTime;

	rvector	m_OrigPos;
	rvector	m_Velocity;

public:
	ZEffectLightFragment2(LPDIRECT3DTEXTURE9 pEffectBillboardTexture,
		const rvector& Pos, const rvector& Velocity);

	virtual bool Draw(u64 nTime) override;
};