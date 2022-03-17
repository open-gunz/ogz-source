#pragma once

#include <list>
#include <string>

#include "MRTTI.h"
#include "ZCharacterObject.h"
#include "MUID.h"
#include "RTypes.h"
#include "RVisualMeshMgr.h"

#include "MObjectTypes.h"
#include "ZItem.h"
#include "ZCharacterItem.h"
#include "RCharCloth.h"
#include "ZFile.h"
#include "Mempool.h"

#include "ZModule_HPAP.h"

#include "ZCharacterStructs.h"
#include "AnimationStuff.h"
#include "BasicInfoHistory.h"
#include "ZReplayStructs.h"

_USING_NAMESPACE_REALSPACE2

#define MAX_SPEED			1000.f
// Speed when you're running forward
#define RUN_SPEED			630.f
// Speed in any other direction
#define BACK_SPEED			450.f
#define ACCEL_SPEED			7000.f
#define STOP_SPEED			3000.f
#define STOP_FORMAX_SPEED	7100.f

// These pertain to the collision cylinder
#define CHARACTER_RADIUS	35.f
#define CHARACTER_HEIGHT	180.0f

#define ARRIVAL_TOLER		5.f

enum ZC_SKILL {

	ZC_SKILL_NONE = 0,

	ZC_SKILL_UPPERCUT,
	ZC_SKILL_SPLASHSHOT,
	ZC_SKILL_DASH,
	ZC_SKILL_CHARGEDSHOT,

	ZC_SKILL_END
};

enum ZC_DIE_ACTION
{
	ZC_DIE_ACTION_RIFLE = 0,
	ZC_DIE_ACTION_KNIFE,
	ZC_DIE_ACTION_SHOTGUN,
	ZC_DIE_ACTION_ROCKET,

	ZC_DIE_ACTION_END
};

enum ZC_SPMOTION_TYPE {

	ZC_SPMOTION_TAUNT = 0,
	ZC_SPMOTION_BOW,
	ZC_SPMOTION_WAVE,
	ZC_SPMOTION_LAUGH,
	ZC_SPMOTION_CRY,
	ZC_SPMOTION_DANCE,

	ZC_SPMOTION_END
};

enum ZC_WEAPON_SLOT_TYPE {

	ZC_SLOT_MELEE_WEAPON = 0,
	ZC_SLOT_PRIMARY_WEAPON,
	ZC_SLOT_SECONDARY_WEAPON,
	ZC_SLOT_ITEM1,
	ZC_SLOT_ITEM2,

	ZC_SLOT_END,
};

enum ZSTUNTYPE {
	ZST_NONE	=	-1,
	ZST_DAMAGE1	=	0,
	ZST_DAMAGE2,
	ZST_SLASH,
	ZST_BLOCKED,
	ZST_LIGHTNING,
	ZST_LOOP,
};


struct ZSlot {
	int m_WeaponID = 0;
};

#define CHARACTER_ICON_DELAY		2.f
#define CHARACTER_ICON_FADE_DELAY	.2f
#define CHARACTER_ICON_SIZE			32.f

class ZModule_HPAP;
class ZModule_QuestStatus;

class ZCharacter : public ZCharacterObject
{
	MDeclareRTTI;
public:
	ZCharacter();
	virtual ~ZCharacter() override;

	BasicInfoHistoryManager BasicInfoHistory;

	virtual bool GetHistory(v3* pos, v3* direction, float fTime, v3* cameradir = nullptr) override;
	void GetPositions(v3* Head, v3* Foot, double Time);

	bool Create(const MTD_CharInfo& pCharInfo);
	void Destroy();
	
	void InitMeshParts();
	void SelectWeapon();
	
	void EmptyHistory();

	void Draw() { OnDraw(); }

	void AddIcon(int nIcon);

	void SetInvincibleTime(int nDuration)
	{
		m_dwInvincibleStartTime = GetGlobalTimeMS();
		m_dwInvincibleDuration = nDuration;
	}

	bool isInvincible();
	
	bool IsMan() const;

	virtual void OnUpdate(float fDelta) override;
	void UpdateSpeed();
	float GetMoveSpeedRatio();

	void UpdateVelocity(float fDelta);
	void UpdateHeight(float fDelta);
	void UpdateMotion(float fDelta);
	//void UpdateDirection(float fDelta, const v3& Direction);
	virtual void UpdateAnimation();

	void UpdateLoadAnimation();

	void Stop();

	void CheckDrawWeaponTrack();
	void UpdateSpWeapon();

	void SetAnimation(const char *AnimationName, bool bEnableCancel, int tick);
	void SetAnimation(RAniMode mode, const char *AnimationName, bool bEnableCancel, int tick);

	void SetAnimationLower(ZC_STATE_LOWER nAni);
	void SetAnimationUpper(ZC_STATE_UPPER nAni);
	
	ZC_STATE_LOWER GetStateLower() const { return m_AniState_Lower; }
	ZC_STATE_UPPER GetStateUpper() const { return m_AniState_Upper; }

	auto IsUpperPlayDone() const { return m_bPlayDone_upper; }

	bool IsMoveAnimation();

	bool IsTeam(ZCharacter* pChar);

	bool IsRunWall();
	bool IsMeleeWeapon();
	virtual bool IsCollideable() override;

	void SetTargetDir(rvector vDir); 

	virtual bool Pick(const rvector& pos, const rvector& dir, RPickInfo* pInfo = NULL) override;

	virtual void OnChangeWeapon(MMatchItemDesc* Weapon);
	void OnChangeParts(RMeshPartsType type,int PartsID);
	void OnAttack(int type,rvector& pos);
	void OnShot();

	void ChangeWeapon(MMatchCharItemParts nParts);


	auto GetLastShotItemID() const { return m_nLastShotItemID; }
	auto GetLastShotTime() const { return m_fLastShotTime; }
	bool CheckValidShotTime(int nItemID, float fTime, ZItem* pItem);
	void UpdateValidShotTime(int nItemID, float fTime)
	{
		m_nLastShotItemID = nItemID;
		m_fLastShotTime = fTime;
	}

	bool IsDead() override { return m_bDie; } const
	auto IsAlive() const { return !m_bDie; }
	void ForceDie() { SetHP(0); m_bDie = true; }

	void SetAccel(const rvector& accel) { m_Accel = accel; }
	virtual void SetDirection(const rvector& dir) override;

	int		m_nSelectSlot;
	ZSlot	m_Slot[ZC_SLOT_END];

	auto* GetStatus() { return &m_Status; }
	auto* GetStatus() const { return &m_Status; }

	auto* GetProperty() { return &m_Property; }
	auto* GetProperty() const { return &m_Property; }

	auto GetUserGrade() const { return m_InitialInfo.nUGradeID; }
	auto GetClanID() const { return m_InitialInfo.nClanCLID; }

	void SetName(const char* szName) { strcpy_safe(m_Property.szName, szName); }

#undef GetUserName
	const char *GetUserName() const { return m_szUserName;	}
	const char *GetUserAndClanName() const { return m_szUserAndClanName; }
	bool IsAdmin() const { return m_InitialInfo.nUGradeID == MMUG_DEVELOPER || m_InitialInfo.nUGradeID == MMUG_ADMIN; }
	bool IsAdminHide() const { return m_bAdminHide;	}
	void SetAdminHide(bool bHide) { m_bAdminHide = bHide; }

	int GetHP() const { return m_pModule_HPAP->GetHP(); }
	int GetAP() const { return m_pModule_HPAP->GetAP(); }
	void SetHP(int nHP) { m_pModule_HPAP->SetHP(nHP); }
	void SetAP(int nAP) { m_pModule_HPAP->SetAP(nAP); }

	int GetKills() const { return GetStatus()->nKills; }

	bool CheckDrawGrenade() const;

	bool GetStylishShoted() const { return m_bStylishShoted; }
	void UpdateStylishShoted();
	
	MUID GetLastAttacker() const { return m_pModule_HPAP->GetLastAttacker(); }
	void SetLastAttacker(const MUID& uid) { m_pModule_HPAP->SetLastAttacker(uid); }
	auto GetLastDamageType() const { return m_LastDamageType; }
	void SetLastDamageType(ZDAMAGETYPE type) { m_LastDamageType = type; }

	bool DoingStylishMotion();
	
	bool IsObserverTarget();

	virtual MMatchTeam GetTeamID() const override { return m_nTeamID; }
	void SetTeamID(MMatchTeam nTeamID) { m_nTeamID = nTeamID; }
	bool IsSameTeam(const ZCharacter* pCharacter) const
	{ 
		if (pCharacter->GetTeamID() == -1) return false;
		if (pCharacter->GetTeamID() == GetTeamID()) return true; 
		return false;
	}
	bool IsTagger() const { return m_bTagger; }
	void SetTagger(bool bTagger) { m_bTagger = bTagger; }

	void SetLastThrower(MUID uid, float fTime) { m_LastThrower = uid; m_tmLastThrowClear = fTime; }
	const MUID& GetLastThrower() const { return m_LastThrower; }
	float GetLastThrowClearTime() const { return m_tmLastThrowClear; }

	virtual void Revival();
	void Die();
	void ActDead();
	virtual void InitHPAP();
	virtual void InitBullet();
	virtual void InitStatus();
	virtual void InitRound();

	void TestChangePartsAll();
	void TestToggleCharacter();

	virtual void OutputDebugString_CharacterState();

	void ToggleClothSimulation();
	void ChangeLowPolyModel();
	bool IsFallingToNarak() const { return m_bFallingToNarak; }

	MMatchItemDesc* GetSelectItemDesc() {
		if(GetItems())
			if(GetItems()->GetSelectedWeapon())
				return GetItems()->GetSelectedWeapon()->GetDesc();
		return NULL;
	}

	void LevelUp();
	void LevelDown();

	void Save(ReplayPlayerInfo& Info);
	void Load(const ReplayPlayerInfo& Info);

	RMesh *GetWeaponMesh(MMatchCharItemParts parts);

	virtual float ColTest(const rvector& pos, const rvector& vec, float radius, rplane* out=0) override;
	virtual bool IsAttackable() override;

	virtual bool IsGuardNonrecoilable() const override;
	virtual bool IsGuardRecoilable() const override;
	virtual void OnMeleeGuardSuccess() override;

	void AddMassiveEffect(const rvector &pos, const rvector &dir);

	virtual void OnDamagedAnimation(ZObject *pAttacker, int type) override;

	virtual ZOBJECTHITTEST HitTest(const rvector& origin, const rvector& to,
		float fTime, rvector *pOutPos = nullptr) override;

	virtual void OnKnockback(const rvector& dir, float fForce) override;
	virtual void OnDamaged(ZObject* pAttacker, rvector srcPos, ZDAMAGETYPE damageType, MMatchWeaponType weaponType,
		float fDamage, float fPiercingRatio=1.f, int nMeleeType=-1) override;
	virtual void OnScream() override;

	void HandleDamage(ZObject* pAttacker, rvector srcPos, ZDAMAGETYPE damageType, MMatchWeaponType weaponType,
		float fDamage, float fPiercingRatio = 1.f, int nMeleeType = -1);

	void UpdateTimeOffset(float PeerTime, float LocalTime);

	void SetNetPosition(const rvector& position, const rvector& velocity, const rvector& dir);

	auto& GetLowerDir() const { return m_DirectionLower; }
	auto& GetTargetDir() const { return m_TargetDir; }
	auto GetTimeOffset() const { return m_fTimeOffset; }
	auto LostConnection() const { return m_bLostConEffect; }

	auto GetScale() const { return Scale; }
	void SetScale(float f)
	{
		Scale = f;
		if (m_pVMesh)
			m_pVMesh->SetScale({ f, f, f });
	}

	v3 CameraDir{ 0, 0, 0 };

	union {
		struct {
			bool	m_bLand : 1;
			bool	m_bWallJump : 1;
			bool	m_bJumpUp : 1;
			bool	m_bJumpDown : 1;
			bool	m_bWallJump2 : 1;
			bool	m_bTumble : 1;
			bool	m_bBlast : 1;
			bool	m_bBlastFall : 1;
			bool	m_bBlastDrop : 1;
			bool	m_bBlastStand : 1;
			bool	m_bBlastAirmove : 1;
			bool	m_bSpMotion : 1;
			bool	m_bCommander : 1;
			bool	m_bCharging : 1;
			bool	m_bCharged : 1;
			bool	m_bLostConEffect : 1;
			bool	m_bChatEffect : 1;
			bool	m_bBackMoving : 1;
		};
		u32 dwFlagsPublic;
	};

	float m_fChargedFreeTime;
	int m_nWallJumpDir;
	int m_nBlastType;

	ZC_STATE_LOWER	m_SpMotion;

	int m_t_parts[6];
	int m_t_parts2[6];

	int m_nVMID;

	i32 Ping{999};

	bool IsBot = false;

protected:
	void UpdateSound();

	void InitMesh();
	void InitProperties();

	void CheckLostConn();
	void OnLevelDown();
	void OnLevelUp();
	virtual void OnDraw() override;
	virtual void OnDie() override;

	rvector m_Accel;
	rvector m_AnimationPositionDiff;

	rvector m_TargetDir;

	ZModule_QuestStatus		*m_pModule_QuestStatus;

	ZCharacterProperty		m_Property;
	ZCharacterStatus		m_Status;

	MTD_CharInfo			m_InitialInfo;

	char	m_szUserName[MATCHOBJECT_NAME_LENGTH];
	char	m_szUserAndClanName[MATCHOBJECT_NAME_LENGTH];

	union {
		struct {
			bool	m_bAdminHide : 1;
			bool	m_bDie : 1;
			bool	m_bStylishShoted : 1;
			bool	m_bFallingToNarak : 1;
			bool	m_bStun : 1;
			bool	m_bDamaged : 1;

			bool	m_bPlayDone : 1;
			bool	m_bPlayDone_upper : 1;
			bool	m_bIsLowModel : 1;
			bool	m_bTagger : 1;

		};
		DWORD dwFlagsProtected;
	};

	ZSTUNTYPE	m_nStunType;

	int			m_nKillsThisRound;
	float		m_fLastKillTime;
	ZDAMAGETYPE	m_LastDamageType;
	MMatchWeaponType m_LastDamageWeapon;
	rvector		m_LastDamageDir;
	float		m_LastDamageDot;
	float		m_LastDamageDistance;

	MUID		m_LastThrower;
	float		m_tmLastThrowClear;

	int			m_nWhichFootSound;

	u64			m_dwInvincibleStartTime;
	u32			m_dwInvincibleDuration;

	rvector m_DirectionLower, m_DirectionUpper;

private:
	// The origin of the character model.
	v3 m_vProxyPosition{ 0, 0, 0 };

	MMatchTeam		m_nTeamID;

	MCharacterMoveMode		m_nMoveMode;
	MCharacterMode			m_nMode;
	MCharacterState			m_nState;

	float	m_fAttack1Ratio;

	float	m_fLastReceivedTime;

	float	m_fTimeOffset;
	float	m_fAccumulatedTimeError;
	int		m_nTimeErrorCount;

	float	m_fGlobalHP;
	int		m_nReceiveHPCount;

	int		m_nLastShotItemID;
	float	m_fLastShotTime;

	rvector m_RealPositionBefore;

	ZC_STATE_UPPER	m_AniState_Upper;
	ZC_STATE_LOWER	m_AniState_Lower;
	ZANIMATIONINFO *m_pAnimationInfo_Upper, *m_pAnimationInfo_Lower;

	float Scale = 1.0f;
};

void ZChangeCharParts(RVisualMesh* pVMesh, MMatchSex nSex, int nHair, int nFace, u32* pItemID);
void ZChangeCharWeaponMesh(RVisualMesh* pVMesh, u32 nWeaponID);
bool CheckTeenVersionMesh(RMesh** ppMesh);