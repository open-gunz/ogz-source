#ifndef MOBJECTTYPES_H
#define MOBJECTTYPES_H

/// 오브젝트의 타입
enum MObjectType 
{
	MOT_NONE		= 0,
	MOT_PC			= 1,
	MOT_NPC			= 2,		
	MOT_ITEM		= 3,
	MOT_CHARACTER
};

/// 캐릭터 종족
enum MCharacterRace
{
	MCR_NONE		= 0,
	MCR_HUMAN,
	MCR_ELF,
	MCR_DARKELF,
	MCR_DWARF,
	MCR_OGRE
};
typedef u32 MCharacterRaces;

/// 캐릭터 클래스
enum MCharacterClass
{
	MCC_NONE			= 0,
	MCC_FIGHTER,
	MCC_ROGUE,
	MCC_ACOLYTE,
	MCC_MAGE,

	// 2차직업
	MCC_KNIGHT,
	MCC_PALADIN,
	MCC_BESERKER,
	MCC_WARRIOR,
	MCC_RANGER,
	MCC_ASSASSIN,
	MCC_HUNTER,
	MCC_SHADOWWALKER,
	MCC_SCOUT,
	MCC_THIEF,
	MCC_CLERIC,
	MCC_MONK,
	MCC_DOCTOR,
	MCC_SHAMON,
	MCC_DRUID,
	MCC_SORCERER,
	MCC_ENCHANTER,
	MCC_WIZARD,
	MCC_MAGICIAN,
	MCC_WARLOCK
};
typedef u32 MCharacterClasses;

/// 캐릭터의 타입
enum MCharacterType
{
	MCT_NONE		= 0,
	MCT_HUMANOID	= 1,	// 플레이어
};

enum MCharacterMoveMode
{
	MCMM_WALK		= 0,
	MCMM_RUN		= 1
};

enum MCharacterMode
{
	MCM_PEACE		= 0,
	MCM_OFFENSIVE	= 1
};

enum MCharacterState
{
	MCS_STAND		= 0,
	MCS_SIT			= 1,
	MCS_DEAD		= 2
};

/// 캐릭터 상태
enum MCharacterAbility
{
	MCAB_LEVITATE			= 0,
	MCAB_MOUNT				= 1,
	MCAB_INVISIBILITY		= 2,
	MCAB_STEALTH			= 4,
	MCAB_SEE_INVISIBILITY	= 8,
	MCAB_DETECT				= 16,
	MCAB_INVINCIBILITY		= 32,
	MCAB_DISABLE_ACTIVE		= 64,
	MCAB_DISABLE_PASSIVE	= 128,
	MCAB_STUN				= 256,
	MCAB_SLEEP				= 512
};
typedef u32 MCharacterStates;


/*
/// 캐릭터 상태값
enum MCharacterStatus
{
	MCS_NONE			= 0,
	MCS_SIT				= 1,
	MCS_RUN				= 2,
	MCS_MOVE			= 3,

	MCS_ATTACK			= 5,
	MCS_SKILL			= 6,
	MCS_USEDISCIPLINE	= 7,
	MCS_CONCENTRATE		= 8,

	MCS_LEVITATE		= 17,
	MCS_POISON			= 18,
	MCS_CURSE			= 19,
	MCS_STEALTH			= 20,
	MCS_INVISIBILITY	= 21,
	MCS_INVINCIBILITY	= 22,
	MCS_PARALYSIS		= 23,
	MCS_SILENCE			= 24,
	MCS_STUN			= 25,
	MCS_SLEEP			= 26
};
*/



/// 캐릭터 기본 특성치
struct MCharacterBasicAttr
{
	int			nSTR;
	int			nCON;
	int			nDEX;
	int			nAGI;
	int			nINT;
	int			nCHA;
	MCharacterBasicAttr(): nSTR(0), nCON(0), 
			   nDEX(0), nAGI(0), nINT(0), nCHA(0) {    }
};

struct MCharacterRepeatInfo
{
	int			nLastTime;
	float		fHP;
	float		fEN;
	MCharacterRepeatInfo(): nLastTime(0), fHP(0), fEN(0) {	}
};

struct MCHARACTERDATA
{
	// 식별
	char		szName[24];
	char		szSurname[24];
	int			iClass;				// enum
	int			iRace;				// enum
	int			iType;				// enum
	int			iSex;				// enum
	int			iSpecialization;	// enum
	int			iBindingSpot;		// pos

	// 경험치
	int			iCurrentXP;
	int			iNextXP;

	// 성향
	int			iRelationships;		// list
	int			iRshipRaise;		// list
	int			iRshipLower;		// list

	// 저항력
	int			iMR;
	int			iFR;
	int			iCR;
	int			iPR;
	int			iLR;

	// 마법
	int			iBUFFS;		// list
	int			iSkillList;	// list

	// 방어력
	int			iAC;
	int			iMFlee;
	int			iRFlee;
	int			iSFlee;
	int			iHPRegen;
	int			iENRegen;

	// 공격력
	int			iMATK;
	int			iRATK;
	int			iMCritical;
	int			iRCritical;
	int			iMAspd;
	int			iRAspd;
	int			iMAccu;
	int			iRAccu;

	// 이동
	int			iDSight;
	int			iNSight;
	int			iSpd;

	// 상태
	int			iLevitate;			// flag
	int			iInvisibility;		// flag
	int			iInvincibility;		// flag
	int			iStun;				// flag
	int			iStealth;			// flag
	int			iPoisoned;			// flag

	// 이뮨
	int			iIVMelee;			// flag
	int			iIVRange;			// flag
	int			iIVMagic;			// flag
	int			iIVCold;			// flag
	int			iIVLightning;		// flag
	int			iIVPoison;			// flag
	int			iIVFire;			// flag
	int			iIVStun;			// flag
	int			iIVSleep;			// flag
	int			iIVCrit;			// flag
	int			iIVParalysis;		// flag

	// 전문화
	int			iOCS;
	int			iDCS;
	int			iRCS;
	int			iECS;
	int			iOMS;
	int			iDMS;
	int			iEMS;
};


#endif