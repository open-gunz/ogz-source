#pragma once

#include <deque>

#include "MRTTI.h"
#include "ZCharacter.h"

enum ZChangeWeaponType
{
	ZCWT_NONE = 0,
	ZCWT_PREV,
	ZCWT_NEXT,
	ZCWT_MELEE,
	ZCWT_PRIMARY,
	ZCWT_SECONDARY,
	ZCWT_CUSTOM1,
	ZCWT_CUSTOM2,
	ZCWT_END,
};

enum ZUsingStamina{
	ZUS_Tumble = 0,
	ZUS_Jump,
};

enum ZDELAYEDWORK {
	ZDW_RECOIL,
	ZDW_UPPERCUT,
	ZDW_DASH,
	ZDW_MASSIVE,
	ZDW_RG_SLASH,
	ZDW_RG_MASSIVE,
	ZDW_RG_RECOIL,
};

struct ZDELAYEDWORKITEM {
	float fTime;
	ZDELAYEDWORK nWork;      
	void *Data;
};

using ZDELAYEDWORKLIST = std::deque<ZDELAYEDWORKITEM>;

class ZMyCharacter : public ZCharacter {
	MDeclareRTTI;
protected:
	virtual void OnDraw();
public:
#ifdef _DEBUG
	bool m_bGuardTest;
#endif

	float	m_fDeadTime;

	float	m_fNextShotTimeType[MMCIP_END];
	float	m_fNextShotTime;
	float	m_fWallJumpTime;

	int		m_nTumbleDir;

	float LastDamagedTime = std::numeric_limits<float>::lowest();

	union {
		struct {
			bool	m_bWallHang:1;

			bool	m_bLimitJump:1;
			bool	m_bLimitTumble:1;
			bool	m_bLimitWall:1;

			bool	m_bMoveLimit:1;
			bool	m_bMoving:1;

			bool	m_bReleasedJump:1;
			bool	m_bJumpQueued:1;
			bool	m_bWallJumpQueued:1;
			bool	m_bHangSuccess:1;
			bool	m_bSniferMode:1;

			bool	m_bEnterCharge:1;

			bool	m_bJumpShot:1;
			bool	m_bShot:1;
			bool	m_bShotReturn:1;

			bool	m_bSkill:1;
			bool	m_b1ShotSended:1;

			bool	m_bSplashShot:1;
			bool	m_bGuard:1;
			bool	m_bGuardBlock_ret:1;
			bool	m_bGuardStart:1;
			bool	m_bGuardCancel:1;
			bool	m_bGuardKey:1;
			bool	m_bGuardByKey:1;
			bool	m_bDrop:1;
			bool	m_bSlash:1;
			bool	m_bJumpSlash:1;
			bool	m_bJumpSlashLanding:1;
			bool	m_bReserveDashAttacked:1;

			bool	m_bLButtonPressed:1;
			bool	m_bLButtonFirstPressed:1;
			bool	m_bLButtonQueued:1;

			bool	m_bRButtonPressed:1;
			bool	m_bRButtonFirstPressed:1;
			bool	m_bRButtonFirstReleased:1;
		};

		DWORD dwFlags[2];
	};

	bool bWasPressingSecondaryLastFrame;

	float	m_fSplashShotTime;

	float	m_fLastJumpPressedTime;
	float	m_fJump2Time;
	float	m_fHangTime;
	rvector m_HangPos;

	int		m_nWallJump2Dir;

	float	m_fLastLButtonPressedTime;
	float	m_fLastRButtonPressedTime;

	int		m_nShot;
	int		m_nJumpShotType;
	float	m_f1ShotTime;

	float	m_fSkillTime;

	float	m_fLastShotTime;
	int		m_nGuardBlock;
	float	m_fGuardStartTime;

	float	m_fDropTime;

	float	m_bJumpSlashTime;

	rvector	m_vReserveDashAttackedDir;
	float	m_fReserveDashAttackedTime;
	MUID	m_uidReserveDashAttacker;

	float	m_fStunEndTime;

	void WallJump2();

	ZMyCharacter();

	void InitSpawn();

	void ProcessInput(float fDelta);

	bool CheckWall(rvector& Pos);

	virtual void UpdateAnimation() override final;
	virtual void OnUpdate(float fTime) override;

	void UpdateLimit();
	
	virtual void OnChangeWeapon(MMatchItemDesc* Weapon) override;

	void Animation_Reload();

	void OnTumble(int nDir);
	virtual void OnBlast(rvector &dir) override final;
	void OnDashAttacked(rvector &dir);
	void ReserveDashAttacked(MUID uid, float time,rvector &dir);

	virtual void Revival() override final;
	virtual void InitBullet() override final;
	virtual void InitStatus() override final;
	virtual void InitRound() override final;
	
	virtual void SetDirection(const rvector& dir) override final;
	virtual void OnDamagedAnimation(ZObject *pAttacker,int type) override final;

	void OutputDebugString_CharacterState();

	float GetCAFactor() { return m_fCAFactor; }
	virtual bool IsGuardNonrecoilable() const override final;
	virtual bool IsGuardRecoilable() const override final;

	void ShotBlocked();

	void ReleaseButtonState();

	void AddRecoilTarget(ZCharacter *pTarget);

	virtual void OnGuardSuccess() override;
	virtual void OnDamaged(ZObject* pAttacker, rvector srcPos,
		ZDAMAGETYPE damageType, MMatchWeaponType weaponType,
		float fDamage, float fPiercingRatio = 1.f, int nMeleeType = -1) override final;
	virtual void OnKnockback(const rvector& dir, float fForce) override final;
	virtual void OnMeleeGuardSuccess() override final;

	virtual void OnStun(float fTime) override final;

	bool IsDirLocked() {
		return (m_bSkill || m_bWallJump || m_bWallJump2 || m_bWallHang ||
			m_bTumble || m_bBlast || m_bBlastStand || m_bBlastDrop)
			&& !IsDie();
	}

private:
	float m_fCAFactor; // Controllability factor for weapon spread
	float m_fElapsedCAFactorTime;

	ZDELAYEDWORKLIST m_DelayedWorkList;

	void OnDelayedWork(ZDELAYEDWORKITEM& Item);
	void AddDelayedWork(float fTime,ZDELAYEDWORK nWork, void *Data = 0);
	void ProcessDelayedWork();

	virtual void OnDie() override final;
	void CalcRangeShotControllability(rvector& vOutDir, const rvector& vSrcDir, int nControllability, u32 Seed);
	void IncreaseCAFactor();
	float GetControllabilityFactor();
	void UpdateCAFactor(float fDelta);
	void ReleaseLButtonQueue();

	void UpdateButtonState();

	void ProcessShot();
	void ProcessGadget();
	void ProcessGuard();

	void OnGadget_Hanging();
	void OnGadget_Snifer();

	void OnShotCustom();
	void OnShotItem();
	void OnShotRocket();
	void OnShotMelee();
	void OnShotRange();

	void Charged();
	void EnterCharge();
	void Discharged();
	void ChargedShot();
	void JumpChargedShot();

	float GetGravityConst();
};

#ifndef _PUBLISH
class ZDummyCharacter : public ZMyCharacter
{
private:
	virtual void OnUpdate(float fDelta) override final;

	float m_fNextAniTime;
	float m_fElapsedTime;

	float m_fNextShotTime;
	float m_fShotElapsedTime;

	float m_fShotDelayElapsedTime;

	bool m_bShotting;
	bool m_bShotEnable;
public:
	ZDummyCharacter();
	
	void SetShotEnable(bool bEnable) { m_bShotEnable = bEnable; }
};
#endif