#pragma once

#include "RTypes.h"

class ZRangeWeaponHitDice
{
private:
	float		m_fHitProbability;

	float		m_fGlobalFactor;
	float		m_fDistance;
	float		m_fTargetHeight;
	float		m_fTargetWidth;
	float		m_fTargetSpeed;
	rvector		m_TargetPosition;
	rvector		m_SourcePosition;

	void MakeHitProbability();
public:
	ZRangeWeaponHitDice();
	void BuildTargetBounds(float fWidth, float fHeight);
	void BuildTargetSpeed(float fSpeed);
	void BuildTargetPosition(const rvector& vPosition);
	void BuildSourcePosition(const rvector& vPosition);
	void BuildGlobalFactor(float fGlobalFactor);
	
	rvector ReturnShotDir();
	float GetHitProbability() const { return m_fHitProbability; }
};