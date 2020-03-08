#include "stdafx.h"

#include "ZItem.h"
#include "crtdbg.h"

//////////////////////////////////////////////////////////////////////////////
// ZItem /////////////////////////////////////////////////////////////////////
ZItem::ZItem() : MMatchItem()
{
	m_nBullet = 0;
	m_nBulletAMagazine = 0;
}

ZItem::~ZItem()
{

}

void ZItem::InitBullet(int nBullet)
{
	if (m_pDesc==NULL) 
	{
//		_ASSERT(0);
		return;
	}

#ifdef _DEBUG
	m_nBulletAMagazine = 1000;
	m_nBullet = 1000;
	return;
#endif

	if(GetItemType() == MMIT_RANGE) 
	{
		m_nBulletAMagazine = m_pDesc->m_nMagazine;
		if (m_nBulletAMagazine > nBullet) m_nBulletAMagazine = nBullet;
		m_nBullet = nBullet - m_nBulletAMagazine;
	}
	else if(GetItemType() == MMIT_CUSTOM) 
	{
		m_nBulletAMagazine = m_pDesc->m_nMagazine;

		if (m_nBulletAMagazine > nBullet) m_nBulletAMagazine = nBullet;
		m_nBullet = m_nBulletAMagazine;
	}
}

bool ZItem::Shot()
{
	if (m_pDesc==NULL) 
	{
//		_ASSERT(0);
		return false;
	}

	_ASSERT((GetItemType() == MMIT_RANGE) || 
			(GetItemType() == MMIT_MELEE) ||
			(GetItemType() == MMIT_CUSTOM ));

	if (GetItemType() == MMIT_MELEE) return true;	// meele

	// custom item
	if (GetItemType() == MMIT_CUSTOM) 
	{

		if (m_nBulletAMagazine > 0) {
			m_nBulletAMagazine--;
		}
		else { 
			return false;
		}

		return true;
	}
	// Ranged
	if (m_nBulletAMagazine > 0) {
		m_nBulletAMagazine--;
	}
	else { 
		return false;
	}

	return true;
}

bool ZItem::Reload()
{
	if (m_pDesc == NULL)
	{
		_ASSERT(0);
		return false;
	}
	_ASSERT(GetItemType() == MMIT_RANGE);
	

 	int nAddedBullet = m_pDesc->m_nMagazine - m_nBulletAMagazine;
	if (nAddedBullet > m_nBullet) nAddedBullet = m_nBullet;
	if (nAddedBullet <= 0) return false;

	m_nBulletAMagazine += nAddedBullet;
	m_nBullet -= nAddedBullet;

	return true;
}

// is magazine full
bool ZItem::isReloadable()
{
	if (m_pDesc == NULL) {
		_ASSERT(0);
		return false;
	}

	if( GetItemType() == MMIT_CUSTOM ) { // 기타 무기들은 재장전이 없다~
		return false;
	}

	if(m_nBullet > 0) {
		if(m_pDesc->m_nMagazine!=m_nBulletAMagazine)
			return true;
	}

	return false;
}

float ZItem::GetPiercingRatio(MMatchWeaponType wtype,RMeshPartsType partstype)
{
	float fRatio = 0.5f;

	bool bHead = false;

	if(partstype == eq_parts_head) { // 헤드샷 구분~ 
		bHead = true;
	}

	switch(wtype) {

		case MWT_DAGGER:		// 단검
		case MWT_DUAL_DAGGER:	// 양손단검
			{
				if(bHead)	fRatio = 0.75f;
				else		fRatio = 0.7f;
			}
			break;
		case MWT_KATANA:		// 카타나
			{
				if(bHead)	fRatio = 0.65f;
				else		fRatio = 0.6f;
			}
			break;
		case MWT_DOUBLE_KATANA:
			{
				if(bHead)	fRatio = 0.65f;
				else		fRatio = 0.6f;
			}
			break;
		case MWT_GREAT_SWORD:	
			{
				if(bHead)	fRatio = 0.65f;
				else		fRatio = 0.6f;
			}
			break;

		case MWT_PISTOL:
		case MWT_PISTOLx2:
			{
				if(bHead)	fRatio = 0.7f;
				else		fRatio = 0.5f;
			}
			break;

		case MWT_REVOLVER:
		case MWT_REVOLVERx2:
			{
				if(bHead)	fRatio = 0.9f;
				else		fRatio = 0.7f;
			}
			break;

		case MWT_SMG:
		case MWT_SMGx2:			// 서브머신건
			{
				if(bHead)	fRatio = 0.5f;
				else		fRatio = 0.3f;
			}
			break;
		case MWT_SHOTGUN:	
		case MWT_SAWED_SHOTGUN:
			{
				if(bHead)	fRatio = 0.2f;
				else		fRatio = 0.2f;
			}
			break;
		case MWT_MACHINEGUN:	// 머신건
			{
				if(bHead)	fRatio = 0.8f;
				else		fRatio = 0.4f;
			}
			break;
		case MWT_RIFLE:			// 돌격소총
			{
				if(bHead)	fRatio = 0.8f;
				else		fRatio = 0.4f;
			}
			break;
		case MWT_SNIFER:		//우선은 라이플처럼...
			{
				if(bHead)	fRatio = 0.8f;
				else		fRatio = 0.4f;
			}
			break;
		case MWT_FRAGMENTATION:
		case MWT_FLASH_BANG:
		case MWT_SMOKE_GRENADE:
		case MWT_ROCKET:		// 로켓런쳐
			{
				if(bHead)	fRatio = 0.4f;
				else		fRatio = 0.4f;
			}
			break;
		default:
			// case eq_wd_item
			break;
	}
	return fRatio;
}

float ZItem::GetKnockbackForce() const
{
	float fKnockbackForce = 0.0f;

	MMatchItemDesc* pDesc = GetDesc();
	if(pDesc) {
		if (pDesc->m_nType == MMIT_MELEE) {
			fKnockbackForce = 200.0f;
		}
		else 
			if (pDesc->m_nType == MMIT_RANGE) {
				if (pDesc->m_pEffect != NULL)
					fKnockbackForce = (float)(pDesc->m_pEffect->m_nKnockBack);
				else
					fKnockbackForce = 200.0f;
			}
	}
	else {
		_ASSERT(0);
		return 0;
	}

	return fKnockbackForce;
}
