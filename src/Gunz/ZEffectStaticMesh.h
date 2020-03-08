#pragma once

#include "ZEffectManager.h"
#include "RVisualMeshMgr.h"

#include "mempool.h"

class ZObject;

class ZEffectMesh : public ZEffect{
protected:
	RealSpace2::RVisualMesh m_VMesh;
	rvector	m_Pos;
	rvector	m_Velocity;
	rvector	m_RotationAxis;
	rvector	m_Up;
	float	m_fRotateAngle;
	float	m_nStartTime;

public:
	ZEffectMesh(RMesh* pMesh, const rvector& Pos, const rvector& Velocity);
	virtual bool Draw(u64 nTime) override { return true; }

	virtual rvector GetSortPos() {
		return m_Pos;
	}

	virtual void CheckWaterSkip(int mode,float height) {
		if (mode == 0)
		{
			if (m_Pos.z >= height)
				m_bWaterSkip = true;
			else
				m_bWaterSkip = false;
		}
		else
		{
			if (m_Pos.z >= height)
				m_bWaterSkip = false;
			else
				m_bWaterSkip = true;
		}
	}
};

class ZEffectStaticMesh : public ZEffectMesh, public CMemPoolSm<ZEffectStaticMesh>
{
protected:
	MUID	m_uid;
public:
	ZEffectStaticMesh(RMesh* pMesh, const rvector& Pos, const rvector& Velocity, MUID uid);
	virtual bool Draw(u64 nTime) override;
};