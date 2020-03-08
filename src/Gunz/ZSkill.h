#pragma once

#include <map>
#include <vector>
#include "ZTimer.h"
using namespace std;

class ZObject;

enum ZSKILLRESISTTYPE {
	ZSR_NONE		= 0,
	ZSR_FIRE		= 1,
	ZSR_COLD		= 2,
	ZSR_LIGHTNING	= 3,
	ZSR_POISON		= 4
};

enum ZSKILLEFFECTTYPE {		// 어떤 대상에게 사용되는지 결정
	ZSE_ENEMY		= 0,		// 일반적인 적
	ZSE_ENEMY_AREA	= 1,		// 적 영역
	ZSE_OWNER_AREA	= 2,		// 캐스터 주변 영역
	ZSE_WHOLE_AREA	= 3,		// 전체
	ZSE_SLASH_AREA	= 4,		// 칼 휘두르는 범위

	ZSE_ALLIED		= 5,		// 아군
	ZSE_ALLIED_AREA	= 6,		// 아군 영역
};

enum ZSKILLEFFECTTRAILTYPE {
	ZSTE_NONE	= 0,

	ZSTE_FIRE	= 1,
	ZSTE_COLD	= 2,
	ZSTE_MAGIC	= 3,

	ZSTE_END,
};

enum ZSKILLEFFECTTARGETPOSTYPE {
	ZSTP_NONE		= 0,

	ZSTP_SOURCE		= 1,
	ZSTP_TARGET		= 2,
	ZSTP_TARGETHEAD = 3,

	ZSTP_EFFECT		= 4,
//	ZSTP_LHEALING	= 4,
//	ZSTP_HEALING	= 5,
	ZSTP_SPINE1		= 5,
	
	ZSTP_HEAD		= 6,
	ZSTP_LHAND		= 7,			
	ZSTP_RHAND		= 8,
	ZSTP_LFOOT		= 9,
	ZSTP_RFOOT		= 10,

	ZSTP_END,
};

struct ZSkillRepeat
{
	float	fDelay;
	rvector	vAngle;
};

// 기획서에 있는것이외에 효과시작시간, 대상이 있는 스킬인경우 캐스팅가능한 거리도 필요할듯.
// 또 이펙트에 대한 기술도 필요할듯.
class ZSkillDesc {
public:
	ZSkillDesc();

public:

	int					nID;
	char				szName[128];
	ZSKILLEFFECTTYPE	nEffectType;
	bool				bHitCheck;
	bool				bGuidable;
	float				fVelocity;			// 발사되는 무기의 속도
	int					nDelay;
	int					nLifeTime;
	int					nEffectStartTime;	// 추가 : 효과 시작시간
	int					nEffectTime;		// 효과 지속시간
	float				fEffectArea;
	float				fEffectAreaMin;		// 효과 최소거리 - 지금은 Melee형 Skill에만 사용한다.
	float				fEffectAngle;		// 효과 판정 각도 - 지금은 Melee형 Skill에만 사용한다. 기본은 90도
	ZSKILLRESISTTYPE	ResistType;
	int					nDifficulty;
	bool				bCameraShock;
	float				fCameraPower;
	float				fCameraDuration;
	float				fCameraRange;

	int					nModDamage;
	int					nModLastDamage;
	int					nModDoT;
	int					nModCriticalRate;	// 관통률(%) 전체 데미지중 HP데미지의 비율
	int					nModSpeed;
	bool				bModAntiMotion;
	bool				bModRoot;
	int					nModHeal;
	int					nModRepair;
	float				fModKnockback;
	char				szMessage[256];

	int					nCastingAnimation;	// 추가 : 캐스팅 애니메이션
	int					nTargetAnimation;

	char				szEffectSound[64];		// 효과 시작될때 나오는 사운드
	char				szExplosionSound[64];	// 폭발할때 나오는 사운드

	ZSKILLEFFECTTARGETPOSTYPE	nCastingEffectType;
	ZSKILLEFFECTTARGETPOSTYPE	nCastingPreEffectType;
	ZSKILLEFFECTTARGETPOSTYPE	nEffectStartPosType;

	rvector				vCastingEffectAddPos;
	char				szCastingEffect[64];
	char				szCastingEffectSp[64];//특수한 이펙트들..이름으로 특정한 함수와 연결하고 싶은경우...
	char				szCastingPreEffect[64];// 스킬 발동과 동시에 보여질것...
	int					nCastingEffectSpCount;
	
	char				szTrailEffect[64];
	char				szTargetEffect[64];

	ZSKILLEFFECTTRAILTYPE nTrailEffectType;
	float				fTrailEffectScale;
	bool				bDrawTrack;				// 미사일류 Track 그릴지 여부
	float				fColRadius;				// 미사일류 충돌 범위 반지름
	vector<ZSkillRepeat>		RepeatList;		// 한스킬에 여러개 발사하려고 할 때 사용

	bool IsAlliedTarget() {
		if(nEffectType==ZSE_ALLIED || nEffectType==ZSE_ALLIED_AREA) return true;
		return false;
	}

	bool IsAreaTarget() {
		if(nEffectType==ZSE_ENEMY || nEffectType==ZSE_ALLIED) return false;
		return true;
	}

	bool IsEffectiveTo(ZObject *pTarget);

	bool CheckResist(ZObject *pCurrent,float *pfDamage);

};

class ZSkillManager : public map < int , ZSkillDesc* > {
public:
	virtual ~ZSkillManager();
	bool Create();
	void Destroy();
};

class ZSkill {
private:
	ZSkillDesc *m_pDesc;
	float		m_fLastBeginTime;
	float		m_fNextDamageTime;
	bool		m_bEnable;
	ZObject		*m_pOwner;

	rvector			m_TargetPos;
	MUID			m_uidTarget;
	int				m_nUseNum;
	ZUpdateTimer	m_RepeatUTimer;
public:
	ZSkill();
	virtual ~ZSkill();

	bool Init(int nID, ZObject *pOwner);

	bool IsReady();
	bool IsEnable();

	void InitStatus();
	bool Update(float fElapsed);

	ZSkillDesc *GetDesc() { return m_pDesc; }
	MUID		GetTarget()	const { return m_uidTarget; }
	auto& GetTargetPos() const { return m_TargetPos; }

	void Execute(const MUID& uidTarget, const rvector& targetPos );
	void PreExecute(const MUID& uidTarget, const rvector& targetPos );
	void LastExecute(const MUID& uidTarget, const rvector& targetPos );
	void Cancel(const MUID& uidTarget);

	bool IsUsable(ZObject *pTarget);

protected:

	bool GetPartsTypePos(ZObject* pTargetObject, ZSKILLEFFECTTARGETPOSTYPE nSkillEffectPosType, 
						 RMeshPartsPosInfoType& type, rvector& vPos, rvector& vDir);
	bool GetPartsTypePos(RMeshPartsPosInfoType& type, MUID& uid, const MUID& uidTarget);
	bool CheckRange(const rvector& center, ZObject *pCurrent);
	void Use(const MUID& uidTarget, const rvector& targetPos);
	void Repeat();
	void GetMissilePosDir(rvector& outDir, rvector& outPos, const rvector& TargetPos);
};