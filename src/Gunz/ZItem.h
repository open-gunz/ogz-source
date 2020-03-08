#pragma once

#include "MMatchItem.h"

class ZItem : public MMatchItem
{
private:
protected:
	int		m_nBullet;
	int		m_nBulletAMagazine;
public:
	ZItem();
	virtual ~ZItem();

	void InitBullet(int nBullet);
	bool Shot();
	bool Reload();
	bool isReloadable();

	int GetBullet() const { return m_nBullet; }
	void SetBullet(int nBullet) { m_nBullet = nBullet; }
	int GetBulletAMagazine() const { return m_nBulletAMagazine; }
	void SetBulletAMagazine(int nBulletAMagazine)	{ m_nBulletAMagazine = nBulletAMagazine; }

	int* GetBulletPointer()		{ return &m_nBullet; }
	int* GetAMagazinePointer()	{ return &m_nBulletAMagazine; }

	static float GetPiercingRatio(MMatchWeaponType wtype,RMeshPartsType partstype);
	float GetKnockbackForce() const;
};