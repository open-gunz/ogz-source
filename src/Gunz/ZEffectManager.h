#pragma once

#include <list>
#include <unordered_map>

#include "ZItem.h"

#include "RTypes.h"
#include "RMesh.h"
#include "RMeshMgr.h"
#include "RVisualMeshMgr.h"

#include "ZEffectBase.h"
#include "ZEffectBulletMarkList.h"
#include "ZEffectLightFragmentList.h"
#include "ZEffectBillboardList.h"
#include "ZCharacter.h"


_USING_NAMESPACE_REALSPACE2

class ZEffectBillboardSource;
class ZEffectMeshSource;
class ZEffectWeaponEnchant;
class ZEffectAniMesh;

enum ZEffectAutoAddType {
	ZEffectAutoAddType_None = 0,
	ZEffectAutoAddType_Methor,
	ZEffectAutoAddType_End,
};

enum ZEffectDrawMode{
	ZEDM_ALPHAMAP=0,
	ZEDM_ADD,
	ZEDM_NONE,
	ZEDM_COUNT,
};

enum ZEffectType {
	ZET_NONE = 0,

	ZET_HEAL ,
	ZET_REPARE,

	ZET_END
};

enum ZCHARACTERICON {
	ZCI_ALLKILL	=0,
	ZCI_UNBELIEVABLE,
	ZCI_EXCELLENT,
	ZCI_FANTASTIC,
	ZCI_HEADSHOT,

	ZCI_END
};

class ZEffect
{
public:
	ZEffectDrawMode	m_nDrawMode;
	ZEffectType		m_nType;
public:
	ZEffect();
	virtual ~ZEffect(void);

	virtual bool CheckRenderAble(int level,float dist);
	virtual void CheckWaterSkip(int mode,float height);
	
	virtual bool Draw(u64 nTime);

	virtual void Update() {}

	virtual bool IsDeleteTime() { return false; }

	ZEffectDrawMode	GetDrawMode(void);

	bool	isEffectType(ZEffectType type);
	void    SetEffectType(ZEffectType type);

	virtual rvector GetSortPos();

	void SetDistOption(float l0,float l1,float l2);

	bool  m_bWaterSkip;
	bool  m_bisRendered;	// for debug
	bool  m_bRender;
	float m_fHideDist[3];
	float m_fDist;
};

typedef std::list<ZEffect*>	ZEffectList;

enum ZTargetType{
	ZTT_CHARACTER,
	ZTT_OBJECT,
	ZTT_CHARACTER_GUARD,
	ZTT_NOTHING,
};

enum SlashEffectMotion {

	SEM_None = 0,

	SEM_ManSlash1,
	SEM_ManSlash2,
	SEM_ManSlash3,
	SEM_ManSlash4,
	SEM_ManSlash5,

	SEM_ManDoubleSlash1,
	SEM_ManDoubleSlash2,
	SEM_ManDoubleSlash3,
	SEM_ManDoubleSlash4,
	SEM_ManDoubleSlash5,

	SEM_ManGreatSwordSlash1,
	SEM_ManGreatSwordSlash2,
	SEM_ManGreatSwordSlash3,
	SEM_ManGreatSwordSlash4,
	SEM_ManGreatSwordSlash5,

	SEM_ManUppercut,

	SEM_WomanSlash1,
	SEM_WomanSlash2,
	SEM_WomanSlash3,
	SEM_WomanSlash4,
	SEM_WomanSlash5,

	SEM_WomanDoubleSlash1,
	SEM_WomanDoubleSlash2,
	SEM_WomanDoubleSlash3,
	SEM_WomanDoubleSlash4,
	SEM_WomanDoubleSlash5,

	SEM_WomanGreatSwordSlash1,
	SEM_WomanGreatSwordSlash2,
	SEM_WomanGreatSwordSlash3,
	SEM_WomanGreatSwordSlash4,
	SEM_WomanGreatSwordSlash5,

	SEM_WomanUppercut,

	SEM_End,
};

#define MAX_PARTICLE_ANI_LIST	100

class ZParticleManager{
public:
	ZParticleManager() {
		
	}
	virtual ~ZParticleManager() {
		
	}

public:
	
	ZEffectBillboardTexAniList*	m_TexAni[MAX_PARTICLE_ANI_LIST];
	
};

#include "RGGlobal.h"

class ZEffectManager{
protected:
	/// Effect List
public:
	ZEffectList	m_Effects[ZEDM_COUNT];

protected:

	// Effect Billboard Sources
#define MUZZLESMOKE_COUNT	4
	ZEffectBillboardSource*	m_pEBSMuzzleSmoke[MUZZLESMOKE_COUNT];

#define MUZZLESMOKE_SHOTGUN_COUNT	2
	ZEffectBillboardSource*	m_pEBSMuzzleSmokeShotgun[MUZZLESMOKE_SHOTGUN_COUNT];

#define SMOKE_COUNT			4
	ZEffectBillboardSource*	m_pEBSSmokes[SMOKE_COUNT];
#define BLOOD_COUNT			5
	ZEffectBillboardSource*	m_pEBSBloods[BLOOD_COUNT];
#define BLOODMARK_COUNT		5
	ZEffectBillboardSource*	m_pEBSBloodMark[BLOODMARK_COUNT];
	ZEffectBillboardSource*	m_pEBSLightTracer;

	ZEffectBillboardSource*	m_pEBSBulletMark[3];

#define SHOTGUNFIRE_COUNT	4
	ZEffectBillboardSource*	m_pEBSShotGunFire[SHOTGUNFIRE_COUNT];
	
#define RIFLEFIRE_COUNT		4
	ZEffectBillboardSource*	m_pEBSRifleFire[RIFLEFIRE_COUNT][2];
							
#define GUNFIRE_COUNT		3
	ZEffectBillboardSource*	m_pEBSGunFire[GUNFIRE_COUNT][2];

	ZEffectBillboardSource*	m_pEBSDash[2];
	ZEffectBillboardSource*	m_pEBSRing[2];

	ZEffectBillboardSource*	m_pEBSLanding;

	ZEffectBillboardSource* m_pEBSWaterSplash;

	ZEffectBillboardSource* m_pEBSWorldItemEaten;

#define EMPTYCARTRIDGE_COUNT	2
	RMesh*	m_pMeshEmptyCartridges[EMPTYCARTRIDGE_COUNT];

	RMesh*	m_pSworddam[4];
	RMesh*	m_pSwordglaze;
	RMesh*	m_pRangeDamaged[6];

	RMesh*	m_pDashEffect;
	RMesh*	m_pGrenadeEffect;
	RMesh*	m_pGrenadeExpEffect;
	RMesh*	m_pRocketEffect;

	RMesh*	m_pFragment[6];

	RMesh*	m_pSwordDefenceEffect[3];
	RMesh*	m_pSwordWaveEffect[2];
	RMesh*	m_pPinkSwordWaveEffect;
	RMesh*	m_pGreenSwordWaveEffect;
	RMesh*	m_pBlueSwordWaveEffect;
	RMesh*	m_pSwordEnchantEffect[4];
	RMesh*	m_pMagicDamageEffect;
	RMesh*	m_pMagicEffectWall[3];
	RMesh*	m_pSwordUppercutEffect;
	RMesh*	m_pSwordUppercutDamageEffect;
	RMesh*	m_pLevelUpEffect[2];
	RMesh*	m_pReBirthEffect;
	RMesh*	m_pFlameMG;
	RMesh*	m_pFlamePistol;
	RMesh*	m_pFlameRifle;
	RMesh*	m_pFlameShotgun;
	RMesh*	m_pHealEffect;
	RMesh*	m_pEatBoxEffect;
	RMesh*	m_pExpanseAmmoEffect;
	RMesh*	m_pRepireEffect;
	RMesh*	m_pWaterSplash;
	RMesh*	m_pWorldItemEaten;
	RMesh*	m_pDaggerUpper;
	RMesh*  m_pBlizzardEffect;

	RMesh*	m_pSwordFire;
	RMesh*	m_pSwordElec;
	RMesh*	m_pSwordCold;
	RMesh*	m_pSwordPoison;

	RMesh* m_pChargingEffect;
	RMesh* m_pChargedEffect;

	RMesh*	m_pPinkChargingEffect;
	RMesh*	m_pGreenChargingEffect;
	RMesh*	m_pBlueChargingEffect;
	RMesh*	m_pPinkChargedEffect;
	RMesh*	m_pGreenChargedEffect;
	RMesh*	m_pBlueChargedEffect;


#define BULLETONWALL_COUNT	2
	RMesh*	m_pBulletOnWallEffect[BULLETONWALL_COUNT];

	RMesh*	m_pCharacterIcons[ZCI_END];
	RMesh*	m_pCommandIcons[2];

	RMesh*	m_pLostConIcon;
	RMesh*	m_pChatIcon;

	RMesh*	m_pBerserkerEffect;

	ZEffectWeaponEnchant* m_pWeaponEnchant[ZC_ENCHANT_END];

	RMeshMgr* m_pEffectMeshMgr;

	ZEffectBulletMarkList		m_BulletMarkList;
	ZEffectLightFragmentList	m_LightFragments;

#define ROCKET_SMOKE_COUNT	1

#define BILLBOARDLISTS_COUNT	SMOKE_COUNT+ROCKET_SMOKE_COUNT
	ZEffectBillboardList		m_BillboardLists[BILLBOARDLISTS_COUNT];

	ZEffectShadowList			m_ShadowList;

#define BILLBOARDTEXANILIST_COUNT 5
	ZEffectBillboardTexAniList	m_BillBoardTexAniList[BILLBOARDTEXANILIST_COUNT];

	void Add(ZEffect* pNew);

public:

	int m__skip_cnt;
	int m__cnt;
	int m__rendered;

public:

	ZEffectManager(void);
	virtual ~ZEffectManager(void);

	bool Create();
	void CheckWaterSkip(int mode,float height);

	int GetEffectCount(int mode) { return (int)m_Effects[mode].size(); }

	void OnInvalidate();
	void OnRestore();

	void Clear();
	
	void Update(float fElapsed);
	void Draw(u32 nTime);
	void Draw(u32 nTime,int mode,float height);

	int  DeleteSameType(ZEffectAniMesh* pNew);

	ZEffectWeaponEnchant* GetWeaponEnchant(ZC_ENCHANT type);

	RMeshMgr* GetEffectMeshMgr() { return m_pEffectMeshMgr; }

	bool RenderCheckEffectLevel();

	void AddLevelUpEffect(ZObject* pObj);
	void AddReBirthEffect(const rvector& Target);

	void AddLandingEffect(const rvector& Target, const rvector& TargetNormal);
	void AddGrenadeEffect(const rvector& Target, const rvector& TargetNormal);
	void AddRocketEffect(const rvector& Target, const rvector& TargetNormal);
	void AddRocketSmokeEffect(const rvector& Target);

	void AddMapSmokeSSEffect(const rvector& Target,const rvector& dir,const rvector& acc,DWORD color,DWORD delay,float fLife,float fStartScale,float fEndScale);
	void AddMapSmokeSTEffect(const rvector& Target,const rvector& dir,const rvector& acc,const rvector& acc2,DWORD color,DWORD delay,float fLife,float fStartScale,float fEndScale);
	void AddMapSmokeTSEffect(const rvector& Target,const rvector& dir,const rvector& acc,DWORD color,DWORD delay,float fLife,float fStartScale,float fEndScale);

	void AddSwordDefenceEffect(const rvector& Target, const rvector& vDir);
	void AddSwordWaveEffect(const MUID& UID, const rvector &Target, const rvector &Dir);
	void AddSwordEnchantEffect(ZC_ENCHANT type, const rvector& Target, DWORD start_time, float fScale = 1.0f);
	void AddMagicEffect(const rvector& Target, DWORD start_time, float fScale = 1.0f);

	void AddBlizzardEffect(const rvector& Target,int nCnt);
	void AddMethorEffect(const rvector& Target,int nCnt);

	void AddMagicEffectWall(int type,const rvector& Target,const rvector& vDir,DWORD start_time, float fScale=1.0f);

	void AddSwordUppercutDamageEffect(const rvector& Target,MUID uidTarget, DWORD time = 0 );

	void AddEatBoxEffect(const rvector& Target,ZObject* pObj);
	void AddHealEffect(const rvector& Target,ZObject* pObj);
	void AddRepireEffect(const rvector& Target,ZObject* pObj);
	void AddExpanseAmmoEffect(const rvector& Target,ZObject* pObj);

	void AddBulletMark(const rvector& Target, const rvector& TargetNormal);
	void AddShotEffect(rvector* pSource,int size,const rvector& Target, const rvector& TargetNormal, ZTargetType nTargetType,MMatchWeaponType wtype,ZObject* pObj,bool bDrawFireEffects,bool bDrawTargetEffects );
	void AddShotgunEffect(const rvector& pos,const rvector& out,const rvector& dir,ZObject* pObj);

	void AddTrackFire(const rvector& pos);
	void AddTrackCold(const rvector& pos);
	void AddTrackPoison(const rvector& pos);
	void AddTrackMagic(const rvector& pos);
	void AddTrackMethor(const rvector& pos);

	float GetEnchantDamageObjectSIze(ZObject* pObj);

	void AddEnchantFire2(ZObject* pObj);
	void AddEnchantCold2(ZObject* pObj);
	void AddEnchantPoison2(ZObject* pObj);

	void AddBloodEffect(const rvector& Target, const rvector& TargetNormal);
	void AddSlashEffect(const rvector& Target, const rvector& TargetNormal,int nType);
	void AddSlashEffectWall(const rvector& Target, const rvector& TargetNormal,int nType);
	void AddLightFragment(rvector Target,rvector TargetNormal);

	void AddDashEffect(const rvector& Target,const rvector& TargetNormal,ZObject* pObj);
	void AddSkillDashEffect(const rvector& Target,const rvector& TargetNormal,ZObject* pObj);

	void AddSmokeEffect( const rvector& Target );
	void AddSmokeEffect( ZEffectBillboardSource* pEffectBillboardSource, const rvector& Pos, const rvector& Velocity, const rvector& Accel, float fMinScale, float fMaxScale, float fLifeTime);
	void AddSmokeGrenadeEffect( rvector& Target );
	void AddGrenadeSmokeEffect(const rvector& Target ,float min,float max,float time);

	void AddWaterSplashEffect(const rvector& Target, const rvector& Scale );
	void AddWorldItemEatenEffect(const rvector& pos );

	void AddCharacterIcon(ZObject* pObj,int nIcon);
	void AddCommanderIcon(ZObject* pObj,int nTeam);
	void AddBerserkerIcon(ZObject* pObj);

	void AddChatIcon(ZObject* pObj);
	void AddLostConIcon(ZObject* pObj);

	void AddChargingEffect(ZObject* pObj);
	void AddChargedEffect(ZObject* pObj);

	void AddShadowEffect(rmatrix& m,DWORD _color);

	// 온게임넷 짱 아이콘
	void AddStarEffect(ZObject* pObj);

	// 일반적인 이펙트
	void Add(const char* szName,const rvector& pos, const rvector& dir,const MUID& uidOwner,int nLifeTime);
	void AddSp(const char* szName,int nCnt,const rvector& pos, const rvector& dir,const MUID& uidOwner);
	void AddPartsPosType(const char* szName,const MUID& uidOwner,RMeshPartsPosInfoType type,int nLifeTime);
};

// 이펙트 디테일 레벨..옵션

void SetEffectLevel(int level);
int	 GetEffectLevel();