#include "stdafx.h"
#include "ZRangeWeaponHitDice.h"
#include "MMath.h"

ZRangeWeaponHitDice::ZRangeWeaponHitDice() :m_fHitProbability(0.0f), 
											m_fGlobalFactor(1.0f), 
											m_fTargetHeight(180.0f),
											m_fTargetSpeed(0.0f), 
											m_TargetPosition(rvector(0,0,0)), 
											m_SourcePosition(rvector(0,0,0)),
											m_fDistance(0.0f)
{

}

void ZRangeWeaponHitDice::BuildTargetBounds(float fWidth, float fHeight)
{
	m_fTargetWidth  = fWidth;
	m_fTargetHeight = fHeight;
}

void ZRangeWeaponHitDice::BuildTargetSpeed(float fSpeed)
{
	m_fTargetSpeed = fSpeed;
}

void ZRangeWeaponHitDice::BuildTargetPosition(const rvector& vPosition)
{
	m_TargetPosition = vPosition;
}

void ZRangeWeaponHitDice::BuildSourcePosition(const rvector& vPosition)
{
	m_SourcePosition = vPosition;
}

void ZRangeWeaponHitDice::BuildGlobalFactor(float fGlobalFactor)
{
	m_fGlobalFactor = fGlobalFactor;
}

void ZRangeWeaponHitDice::MakeHitProbability()
{
	m_fHitProbability = m_fGlobalFactor;	
}

rvector ZRangeWeaponHitDice::ReturnShotDir()
{
	MakeHitProbability();

	rvector vTargetPos = m_TargetPosition;
	float fRandX=0.0f, fRandY=0.0f;

	float fRandMaxX = m_fTargetWidth+50.0f;
	float fRandMaxY = m_fTargetHeight+20.0f;

	
	if (RandomNumber(0.0f, 1.0f) <= m_fHitProbability)
	{
		float fHeight = m_fTargetHeight * 0.6f * 0.5f;
		float fWidth = m_fTargetWidth * 0.6f * 0.5f;
		// ИэСп
		fRandX = RandomNumber(-fWidth, fWidth);
		fRandY = RandomNumber(-fHeight, fHeight);
	}
	else
	{
		float fHeight = m_fTargetHeight * 0.8f * 0.5f;
		float fWidth = m_fTargetWidth * 0.8f * 0.5f;

		fRandX = RandomNumber(fWidth, fRandMaxX);
		fRandY = RandomNumber(fHeight, fRandMaxY);

		int dice = Dice(1, 2, 0);
		if (dice == 1) fRandX = -fRandX;
		dice = Dice(1, 2, 0);
		if (dice == 1) fRandY = -fRandY;
	}

	rmatrix mat;
	rvector modelDir = vTargetPos - m_SourcePosition;
	Normalize(modelDir);

	rvector tar(0,0,0);
	tar.x += fRandX;
	tar.y += fRandY;

	MakeWorldMatrix(&mat, m_TargetPosition, modelDir, rvector(0,0,1));
	tar = TransformCoord(tar, mat);

	rvector dir = tar - m_SourcePosition;
	Normalize(dir);

	return dir;
}

