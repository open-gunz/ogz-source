#pragma once

#include "ZModule.h"
#include "ZModuleID.h"

class ZModule_Movable : public ZModule {
private:
	float	m_fMaxSpeed;
	bool	m_bGravity;

	float	m_fDistToFloor;
	rplane	m_FloorPlane;

	rvector m_lastMove;

	bool	m_bFalling;
	float	m_fFallHeight;

	bool	m_bLanding;
	bool	m_bAdjusted;
	float	m_fLastAdjustedTime;

	rvector m_Velocity;

	bool	m_bRestrict;
	float	m_fRestrictTime;
	float	m_fRestrictDuration;
	float	m_fMoveSpeedRatio;

protected:
	void OnAdd();

public:

	DECLARE_ID(ZMID_MOVABLE)
	ZModule_Movable();

	virtual void OnUpdate(float Elapsed) override final;
	virtual void InitStatus() override final;

	auto& GetVelocity() const { return m_Velocity; }
	void SetVelocity(const rvector &vel) { m_Velocity=vel; }
	void SetVelocity(float x,float y,float z) { m_Velocity=rvector(x,y,z); }

	auto& GetLastMove() const { return m_lastMove; }

	bool Move(rvector &diff);

	void UpdateGravity(float fDelta);

	float GetDistToFloor() { return m_fDistToFloor; }

	float GetFallHeight() { return m_fFallHeight; }
	bool isLanding() { return m_bLanding; }
	bool isAdjusted() { return m_bAdjusted; }
	float GetAdjustedTime() { return m_fLastAdjustedTime; }

	float GetMoveSpeedRatio() { return m_fMoveSpeedRatio; }
	void SetMoveSpeedRatio(float fRatio, float fDuration);

protected:
	void UpdatePosition(float fDelta);
};