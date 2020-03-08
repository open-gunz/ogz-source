#pragma once

#include "GlobalTypes.h"
#include "ZEffectBillboard.h"
#include "mempool.h"

class ZEffectLightTracer : public ZEffectBillboard , public CMemPoolSm<ZEffectLightTracer>
{
protected:
	u64 m_nStartTime;

	rvector	m_LightTracerDir;
	rvector	m_Start, m_End;
	float		m_fLength;

public:
	ZEffectLightTracer(ZEffectBillboardSource* pEffectBillboardSource, const rvector& Start, const rvector& End);

	virtual bool Draw(u64 nTime) override;
};