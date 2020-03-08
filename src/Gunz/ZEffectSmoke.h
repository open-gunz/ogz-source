#pragma once

#include "ZEffectBillboard.h"
#include "RTypes.h"

#include "mempool.h"

class ZEffectSmoke : public ZEffectBillboard , public CMemPoolSm<ZEffectSmoke>
{
protected:
	u64 m_nStartTime;
	float m_fMinScale;
	float m_fMaxScale;
	u64 m_nLifeTime;
	rvector	m_OrigPos;
	rvector	m_Velocity;
public:
	ZEffectSmoke(ZEffectBillboardSource* pEffectBillboardSource,
		const rvector& Pos, const rvector& Velocity,
		float fMinScale, float fMaxScale,
		u64 nLifeTime);

	virtual bool Draw(u64 nTime) override;
};

class ZEffectLandingSmoke : public ZEffectBillboard , public CMemPoolSm<ZEffectLandingSmoke>
{
protected:
	u64 m_nStartTime;
	float m_fMinScale;
	float m_fMaxScale;
	u64 m_nLifeTime;
	rvector	m_OrigPos;
	rvector	m_Velocity;

public:
	ZEffectLandingSmoke(ZEffectBillboardSource* pEffectBillboardSource,
		const rvector& Pos, const rvector& Velocity,
		float fMinScale, float fMaxScale,
		u64 nLifeTime);

	virtual bool Draw(u64 nTime) override;
};
 
class ZEffectSmokeGrenade : public ZEffectBillboard , public CMemPoolSm<ZEffectSmokeGrenade>
{
protected:
	u64 m_nStartTime;
	float m_fMinScale;
	float m_fMaxScale;
	u64 m_nLifeTime;
	rvector	m_OrigPos;
	rvector	m_Velocity;

public:
	virtual bool Draw(u64 nTime) override;

public:
	ZEffectSmokeGrenade(ZEffectBillboardSource* pEffectBillboardSource, 
		const rvector& Pos, const rvector& Velocity,
		float fMinScale, float fMaxScale,
		u64 nLifeTime);
};