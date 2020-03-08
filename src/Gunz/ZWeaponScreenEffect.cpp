#include "stdafx.h"
#include "ZWeaponScreenEffect.h"
#include "ZGame.h"
#include "ZMyInfo.h"
#include "ZMyItemList.h"

ZWeaponScreenEffect::ZWeaponScreenEffect()
{
	m_nWeaponType = MWT_NONE;
	m_pSniferPanel = NULL;
}

ZWeaponScreenEffect::~ZWeaponScreenEffect()
{

}

void ZWeaponScreenEffect::Draw(MDrawContext* pDC)
{
	return;

	switch (m_nWeaponType)
	{
	case MWT_SNIFER:
		{
			if (m_pSniferPanel)
			{
				pDC->SetBitmap(m_pSniferPanel);
				pDC->Draw(0, 0, RGetScreenWidth(), RGetScreenHeight());
			}

		}
		break;
	}
}

void ZWeaponScreenEffect::InitWeapon(MMatchWeaponType nWeaponType)
{
	return;

	switch (nWeaponType)
	{
	case MWT_SNIFER:
		{
		SetFOV(12.0f / 180.0f * PI_FLOAT);
		}
		break;
	}

}

void ZWeaponScreenEffect::FinalWeapon(MMatchWeaponType nWeaponType)
{
	return;

	switch (nWeaponType)
	{
	case MWT_SNIFER:
		{
		SetFOV(70.0f / 180.0f * PI_FLOAT);
		}
		break;
	}
}

void ZWeaponScreenEffect::OnGadget(MMatchWeaponType nWeaponType)
{
	return;

	if (m_nWeaponType == nWeaponType) return;
	if (m_nWeaponType != MWT_NONE) OnGadgetOff();

	InitWeapon(nWeaponType);
	m_nWeaponType = nWeaponType;
}

void ZWeaponScreenEffect::OnGadgetOff()
{
	return;

	FinalWeapon(m_nWeaponType);
	m_nWeaponType = MWT_NONE;
}

void ZWeaponScreenEffect::Create()
{
	return;

	MMatchWeaponType nPrimaryWeaponType = MWT_NONE, 
					 nSecondaryWeaponType = MWT_NONE, 
					 nMeleeWeaponType = MWT_NONE;

	u32 nItemID;
	nItemID = ZGetMyInfo()->GetItemList()->GetEquipedItemID(MMCIP_PRIMARY);
	MMatchItemDesc* pItemDesc = NULL;
	pItemDesc = MGetMatchItemDescMgr()->GetItemDesc(nItemID);
	if (pItemDesc)
	{
		nPrimaryWeaponType = pItemDesc->m_nWeaponType;
	}

	nItemID = ZGetMyInfo()->GetItemList()->GetEquipedItemID(MMCIP_SECONDARY);
	
	pItemDesc = MGetMatchItemDescMgr()->GetItemDesc(nItemID);
	if (pItemDesc)
	{
		nSecondaryWeaponType = pItemDesc->m_nWeaponType;
	}
	nItemID = ZGetMyInfo()->GetItemList()->GetEquipedItemID(MMCIP_MELEE);
	
	pItemDesc = MGetMatchItemDescMgr()->GetItemDesc(nItemID);
	if (pItemDesc)
	{
		nMeleeWeaponType = pItemDesc->m_nWeaponType;
	}

	if ((nPrimaryWeaponType == MWT_SNIFER) || (nSecondaryWeaponType == MWT_SNIFER))
	{
		if (m_pSniferPanel) delete m_pSniferPanel;
		m_pSniferPanel = new MBitmapR2;
		((MBitmapR2*)m_pSniferPanel)->Create("snifer.png", RGetDevice(), "interface/Default/Combat/snifer.png");
	}
}

void ZWeaponScreenEffect::Destroy()
{
	return;

	if (m_pSniferPanel)
	{
		delete m_pSniferPanel;
		m_pSniferPanel = NULL;
	}
}