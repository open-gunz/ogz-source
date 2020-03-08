#ifndef _ZWEAPONSCREENEFFECT_H
#define _ZWEAPONSCREENEFFECT_H

#include "ZPrerequisites.h"

class ZScreenEffect;

class ZWeaponScreenEffect
{
private:
	MMatchWeaponType		m_nWeaponType;
	MBitmap*				m_pSniferPanel;
	//ZScreenEffect*			m_pSniferPanelEffect;


	void InitWeapon(MMatchWeaponType nWeaponType);
	void FinalWeapon(MMatchWeaponType nWeaponType);
public:
	ZWeaponScreenEffect();
	virtual ~ZWeaponScreenEffect();
public:
	void Draw(MDrawContext* pDC);
	void OnGadget(MMatchWeaponType nWeaponType);
	void OnGadgetOff();
	void Create();
	void Destroy();
};




#endif
