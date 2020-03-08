#include "stdafx.h"
#include "ZSecurity.h"
#include "MDataChecker.h"
#include "MZFileSystem.h"
#include "MMatchItem.h"
#include "ZCharacter.h"
#include "ZApplication.h"
#include "ZItemDesc.h"
#include "ZModule_HPAP.h"
#include <list>

void ZSetupDataChecker_Global(MDataChecker* pDataChecker)
{
	pDataChecker->Clear();

	MMatchItemDescMgr* pItemMgr = MMatchItemDescMgr::GetInstance();
	for (MMatchItemDescMgr::iterator i=pItemMgr->begin(); i!=pItemMgr->end(); i++) {
		MMatchItemDesc* pItem = (*i).second;
		pDataChecker->AddCheck((BYTE*)&pItem->m_nDamage, sizeof(int));
		pDataChecker->AddCheck((BYTE*)&pItem->m_nDelay, sizeof(int));
		pDataChecker->AddCheck((BYTE*)&pItem->m_nMagazine, sizeof(int));
		pDataChecker->AddCheck((BYTE*)&pItem->m_nMaxBullet, sizeof(int));
		pDataChecker->AddCheck((BYTE*)&pItem->m_nReloadTime, sizeof(int));
		pDataChecker->AddCheck((BYTE*)&pItem->m_nHP, sizeof(int));
		pDataChecker->AddCheck((BYTE*)&pItem->m_nAP, sizeof(int));
	}
}

void ZSetupDataChecker_Game(MDataChecker* pDataChecker)
{
	pDataChecker->Clear();
	
	ZCharacterItem* pCharItem = ZApplication::GetGame()->m_pMyCharacter->GetItems();
	ZItem* pPrimaryItem = pCharItem->GetItem(MMCIP_PRIMARY);
	if (pPrimaryItem) {
		pDataChecker->AddCheck((BYTE*)pPrimaryItem->GetBulletPointer(), sizeof(int));
		pDataChecker->AddCheck((BYTE*)pPrimaryItem->GetAMagazinePointer(), sizeof(int));
	}
	ZItem* pSecondaryItem = pCharItem->GetItem(MMCIP_SECONDARY);
	if (pPrimaryItem) {
		pDataChecker->AddCheck((BYTE*)pSecondaryItem->GetBulletPointer(), sizeof(int));
		pDataChecker->AddCheck((BYTE*)pSecondaryItem->GetAMagazinePointer(), sizeof(int));
	}
	ZItem* pCustom1Item = pCharItem->GetItem(MMCIP_CUSTOM1);
	if (pPrimaryItem) {
		pDataChecker->AddCheck((BYTE*)pCustom1Item->GetBulletPointer(), sizeof(int));
		pDataChecker->AddCheck((BYTE*)pCustom1Item->GetAMagazinePointer(), sizeof(int));
	}
	ZItem* pCustom2Item = pCharItem->GetItem(MMCIP_CUSTOM2);
	if (pPrimaryItem) {
		pDataChecker->AddCheck((BYTE*)pCustom2Item->GetBulletPointer(), sizeof(int));
		pDataChecker->AddCheck((BYTE*)pCustom2Item->GetAMagazinePointer(), sizeof(int));
	}
}
