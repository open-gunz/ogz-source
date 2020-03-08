#pragma once

#include "targetver.h"

#include "GlobalTypes.h"
#include "MMatchItem.h"
#include "MMath.h"
#include "AnimationStuff.h"
#include "RMeshUtil.h"
#include "MMatchUtil.h"
#include "BasicInfo.h"

enum MMatchWeaponType;

typedef enum _RMeshPartsType RMeshPartsType;
enum RWeaponMotionType;

RWeaponMotionType WeaponTypeToMotionType(MMatchWeaponType WeaponType);

float GetPiercingRatio(MMatchWeaponType wtype, RMeshPartsType partstype);

enum ZDAMAGETYPE {
	ZD_NONE = -1,
	ZD_BULLET,
	ZD_MELEE,
	ZD_FALLING,
	ZD_EXPLOSION,
	ZD_BULLET_HEADSHOT,
	ZD_KATANA_SPLASH,
	ZD_HEAL,
	ZD_REPAIR,
	ZD_MAGIC,

	ZD_END
};

enum ZC_SHOT_SP_TYPE {
	ZC_WEAPON_SP_NONE = 0,

	// grenade type

	ZC_WEAPON_SP_GRENADE,
	ZC_WEAPON_SP_ROCKET,
	ZC_WEAPON_SP_FLASHBANG,
	ZC_WEAPON_SP_SMOKE,
	ZC_WEAPON_SP_TEAR_GAS,

	// item type

	ZC_WEAPON_SP_ITEMKIT,	// medikit, repairkit, bulletkit

	ZC_WEAPON_SP_END,
};