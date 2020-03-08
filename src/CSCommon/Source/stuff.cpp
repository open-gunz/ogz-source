#include "stdafx.h"
#include "stuff.h"
#include "RMeshUtil.h"

RWeaponMotionType WeaponTypeToMotionType(MMatchWeaponType WeaponType)
{
	switch (WeaponType)
	{
	case MWT_ROCKET:
		return eq_wd_rlauncher;
	case MWT_SHOTGUN:
		return eq_wd_shotgun;
	case MWT_RIFLE:
		return eq_wd_rifle;
	case MWT_DAGGER:
		return eq_wd_dagger;
	case MWT_KATANA:
		return eq_wd_katana;
	case MWT_PISTOL:
		return eq_ws_pistol;
	case MWT_PISTOLx2:
		return eq_wd_pistol;
	case MWT_SMG:
		return eq_ws_smg;
	case MWT_SMGx2:
		return eq_wd_smg;
	case MWT_MACHINEGUN:
		return eq_wd_shotgun;
	case MWT_MED_KIT:
	case MWT_REPAIR_KIT:
	case MWT_BULLET_KIT:
	case MWT_FLASH_BANG:
	case MWT_FRAGMENTATION:
	case MWT_SMOKE_GRENADE:
	case MWT_FOOD:
	case MWT_SKILL:
		return eq_wd_item;
	case MWT_GREAT_SWORD:
		return eq_wd_sword;
	case MWT_DOUBLE_KATANA:
		return eq_wd_blade;
	};

	return eq_weapon_etc;
}

float GetPiercingRatio(MMatchWeaponType wtype, RMeshPartsType partstype)
{
	float fRatio = 0.5f;

	bool bHead = false;

	if (partstype == eq_parts_head) { // 헤드샷 구분~ 
		bHead = true;
	}

	switch (wtype) {

	case MWT_DAGGER:		// 단검
	case MWT_DUAL_DAGGER:	// 양손단검
	{
		if (bHead)	fRatio = 0.75f;
		else		fRatio = 0.7f;
	}
	break;
	case MWT_KATANA:		// 카타나
	{
		if (bHead)	fRatio = 0.65f;
		else		fRatio = 0.6f;
	}
	break;
	case MWT_DOUBLE_KATANA:
	{
		if (bHead)	fRatio = 0.65f;
		else		fRatio = 0.6f;
	}
	break;
	case MWT_GREAT_SWORD:
	{
		if (bHead)	fRatio = 0.65f;
		else		fRatio = 0.6f;
	}
	break;

	case MWT_PISTOL:
	case MWT_PISTOLx2:
	{
		if (bHead)	fRatio = 0.7f;
		else		fRatio = 0.5f;
	}
	break;

	case MWT_REVOLVER:
	case MWT_REVOLVERx2:
	{
		if (bHead)	fRatio = 0.9f;
		else		fRatio = 0.7f;
	}
	break;

	case MWT_SMG:
	case MWT_SMGx2:			// 서브머신건
	{
		if (bHead)	fRatio = 0.5f;
		else		fRatio = 0.3f;
	}
	break;
	case MWT_SHOTGUN:
	case MWT_SAWED_SHOTGUN:
	{
		if (bHead)	fRatio = 0.2f;
		else		fRatio = 0.2f;
	}
	break;
	case MWT_MACHINEGUN:	// 머신건
	{
		if (bHead)	fRatio = 0.8f;
		else		fRatio = 0.4f;
	}
	break;
	case MWT_RIFLE:			// 돌격소총
	{
		if (bHead)	fRatio = 0.8f;
		else		fRatio = 0.4f;
	}
	break;
	case MWT_SNIFER:		//우선은 라이플처럼...
	{
		if (bHead)	fRatio = 0.8f;
		else		fRatio = 0.4f;
	}
	break;
	case MWT_FRAGMENTATION:
	case MWT_FLASH_BANG:
	case MWT_SMOKE_GRENADE:
	case MWT_ROCKET:		// 로켓런쳐
	{
		if (bHead)	fRatio = 0.4f;
		else		fRatio = 0.4f;
	}
	break;
	default:
		// case eq_wd_item
		break;
	}
	return fRatio;
}