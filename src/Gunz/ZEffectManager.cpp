#include "stdafx.h"
#include "ZPrerequisites.h"
#include "ZEffectManager.h"
#include "ZEffectBulletMark.h"
#include "ZEffectLightFragment.h"
#include "ZEffectSmoke.h"
#include "ZEffectLightTracer.h"
#include "ZEffectStaticMesh.h"
#include "ZEffectAniMesh.h"
#include "ZGame.h"
#include "ZConfiguration.h"
#include "RealSpace2.h"
#include "ZCharacter.h"
#include "ZMyCharacter.h"
#include "ZGameClient.h"
#include "RMeshMgr.h"
#include <crtdbg.h>
#include "MDebug.h"
#include "dxerr.h"
#include "RGMain.h"
#include "RBspObject.h"
#include "hsv.h"
#include "Config.h"

#ifndef _PUBLISH
class ZEffectValidator : public std::set<ZEffect*> 
{
public:
	void Add(ZEffect* pNew) {
		iterator itr = find(pNew);
		if(itr!=end())
		{
			ZEffect *pDup = *itr;

			_ASSERT(FALSE);
			mlog("effect duplicated.\n");
		}
		insert(pNew);
	}

	void Erase(ZEffect* pEffect) {
		if(find(pEffect)==end())
		{
			_ASSERT(FALSE);
			mlog("effect not exist.\n");
		}
		erase(pEffect);
	}

} g_EffectValidator;
#endif

static int g_nEffectLevel = Z_VIDEO_EFFECT_HIGH;

void SetEffectLevel(int level)
{
	g_nEffectLevel = level;
}

int	 GetEffectLevel()
{
	return g_nEffectLevel;
}

class ZEffectCharging : public ZEffectAniMesh, public CMemPoolSm<ZEffectCharging> {
public:
	ZEffectCharging(RMesh* pMesh, rvector& Pos, rvector& Dir,ZObject* pObj)
		: ZEffectAniMesh(pMesh,Pos,Dir)
	{
		if(pObj)
			m_uid = pObj->GetUID();
	}

	virtual bool Draw(u64 nTime) override
	{
		ZObject* pObj = ZGetObjectManager()->GetObject(m_uid);

		if(!pObj) return false;

		ZCharacter* pChar = MDynamicCast(ZCharacter, pObj);

		if(!pChar) return false;

		if(!pChar->m_bCharging) return false;

		rmatrix m = pObj->m_pVMesh->GetCurrentWeaponPositionMatrix();

		m_Pos	 = rvector(m._41,m._42,m._43);
		m_DirOrg = rvector(m._21,m._22,m._23);
		m_Up	 = rvector(m._11,m._12,m._13);

		ZEffectAniMesh::Draw(nTime);

		if( pObj->m_pVMesh->IsDoubleWeapon() ) {

			m = pObj->m_pVMesh->GetCurrentWeaponPositionMatrix(true);

			m_Pos	 = rvector(m._41,m._42,m._43);
			m_DirOrg = rvector(m._21,m._22,m._23);
			m_Up	 = rvector(m._11,m._12,m._13);

			ZEffectAniMesh::Draw(nTime);
		}

		return true;
	}
};

class ZEffectCharged : public ZEffectAniMesh, public CMemPoolSm<ZEffectCharged> {
public:
	ZEffectCharged(RMesh* pMesh, rvector& Pos, rvector& Dir,ZObject* pObj)
		: ZEffectAniMesh(pMesh,Pos,Dir)
	{
		if(pObj)
			m_uid = pObj->GetUID();
	}

	virtual bool Draw(u64 nTime) override
	{
		ZObject* pObj = ZGetObjectManager()->GetObject(m_uid);

		if( pObj ) {

			ZCharacter* pChar = MDynamicCast(ZCharacter, pObj);

			if(!pChar) return false;

			if(!pChar->m_bCharged)	return false;
			if(!pChar->IsRendered()) return true;
			if( pObj->m_pVMesh ) {

				rmatrix m = pObj->m_pVMesh->GetCurrentWeaponPositionMatrix();
				m_Pos	 = rvector(m._41,m._42,m._43);
				m_DirOrg = rvector(m._21,m._22,m._23);
				m_Up	 = rvector(m._11,m._12,m._13);

				ZEffectAniMesh::Draw(nTime);

				if (pObj->m_pVMesh->IsDoubleWeapon()) {
					m = pObj->m_pVMesh->GetCurrentWeaponPositionMatrix(true);

					m_Pos	 = rvector(m._41,m._42,m._43);
					m_DirOrg = rvector(m._21,m._22,m._23);
					m_Up	 = rvector(m._11,m._12,m._13);

					ZEffectAniMesh::Draw(nTime);
				}

				return true;
			}
		}
		return false;
	}
};

namespace RealSpace2
{
extern uint32_t BlendColor;
}

template <typename ParentType, typename GetterType>
class TexBlendEffect : public ParentType
{
	std::remove_reference_t<GetterType> Get;
public:

	template <typename... Args>
	TexBlendEffect(GetterType&& fn, Args&&... args) :
		Get{ std::forward<GetterType>(fn) },
		ParentType{ std::forward<Args>(args)... }
	{}

	virtual bool Draw(u64 nTime) override
	{
		uint32_t Color = Get();
		
		BlendColor = Color;

		auto ret = ParentType::Draw(nTime);

		BlendColor = 0;

		return ret;
	}

	// Need these to override the same operators in CMemPool[Sm]<ParentType> in the parent class
	// since the ones in the memory pool expect to be allocating space for instances
	// of the parent class's type, not of this type, which is larger.
	static void* operator new(size_t size){ return ::operator new(size); }
	static void  operator delete(void* ptr, size_t size) { ::operator delete(ptr, size); }
};

template <typename T, typename T2, typename... ArgsType>
TexBlendEffect<T, T2>* MakeTexBlendEffect(T2&& fn, ArgsType&&... args)
{
	return new TexBlendEffect<T, T2>(std::forward<T2>(fn), std::forward<ArgsType>(args)...);
}

ZEffect::ZEffect()
{
	m_nDrawMode = ZEDM_NONE;
	m_nType = ZET_NONE;

	m_fDist = 0.f;

	m_fHideDist[0] = 5000.f;
	m_fHideDist[1] = 3000.f;
	m_fHideDist[2] = 1000.f;

	m_bRender = true;
	m_bisRendered = false;
	m_bWaterSkip = false;
}

ZEffect::~ZEffect()
{

}

bool ZEffect::CheckRenderAble(int level,float dist)
{
	if(level < 0 || level > 2)
		return false;

	if(m_fHideDist[level] < dist ) {
		m_bRender = false;
		return false;
	}

	m_bRender = true;

	return true;
}

void ZEffect::CheckWaterSkip(int mode,float height) 
{
	if (mode == 0)
		m_bWaterSkip = true;
	else
		m_bWaterSkip = false;
}

bool ZEffect::Draw(u64 nTime)
{
	return true;
}

ZEffectDrawMode	ZEffect::GetDrawMode(void)
{
	return m_nDrawMode;
}

bool ZEffect::isEffectType(ZEffectType type)
{
	if(m_nType==type)
		return true;
	return false;
}

void ZEffect::SetEffectType(ZEffectType type)
{
	m_nType = type;
}

rvector ZEffect::GetSortPos() 
{
	return rvector(0.f,0.f,0.f);
}

void ZEffect::SetDistOption(float l0,float l1,float l2) 
{
	m_fHideDist[0] = l0;
	m_fHideDist[1] = l1;
	m_fHideDist[2] = l2;
}

bool CreateCommonRectVertexBuffer();	
void RealeaseCommonRectVertexBuffer();

ZEffectManager::ZEffectManager(void)
{
}

bool ZEffectManager::Create(void)
{
	char szFileName[256];

	for(int i=0; i<MUZZLESMOKE_COUNT; i++){
		sprintf_safe(szFileName, "SFX/muzzle_smoke0%d.tga", i+1);
		m_pEBSMuzzleSmoke[i]  = new ZEffectBillboardSource(szFileName);
	}

	for(int i=0; i<MUZZLESMOKE_SHOTGUN_COUNT; i++){
		sprintf_safe(szFileName, "SFX/muzzle_smoke4%d.tga", i+1);
		m_pEBSMuzzleSmokeShotgun[i]  = new ZEffectBillboardSource(szFileName);
	}

	for(int i=0; i<SMOKE_COUNT; i++){
		sprintf_safe(szFileName, "SFX/smoke0%d.tga", i+1);
		m_pEBSSmokes[i] = new ZEffectBillboardSource(szFileName);
	}
	for(int i=0; i<BLOOD_COUNT; i++){
		sprintf_safe(szFileName, "SFX/blood0%d.tga", i+1);
		m_pEBSBloods[i] = new ZEffectBillboardSource(szFileName);
	}
	for(int i=0; i<BLOODMARK_COUNT; i++){
		sprintf_safe(szFileName, "SFX/blood-mark0%d.tga", i+1);
		m_pEBSBloodMark[i] = new ZEffectBillboardSource(szFileName);
	}

	m_pEBSLightTracer = new ZEffectBillboardSource("SFX/gz_sfx_tracer.bmp");
	
	m_pEBSBulletMark[0] = new ZEffectBillboardSource("SFX/gz_sfx_shotgun_bulletmark01.tga");
	m_pEBSBulletMark[1] = new ZEffectBillboardSource("SFX/gz_sfx_shotgun_bulletmark02.tga");
	m_pEBSBulletMark[2] = new ZEffectBillboardSource("SFX/gz_effect004.tga");

	for(int i=0; i<RIFLEFIRE_COUNT; i++){
		sprintf_safe(szFileName, "SFX/gz_sfx_mf0%d.bmp", i+1);
		m_pEBSRifleFire[i][0] = new ZEffectBillboardSource(szFileName);
		sprintf_safe(szFileName, "SFX/gz_sfx_mf1%d.bmp", i+1);
		m_pEBSRifleFire[i][1] = new ZEffectBillboardSource(szFileName);
	}

	for(int i=0; i<SHOTGUNFIRE_COUNT; i++) {
		sprintf_safe(szFileName, "SFX/gz_sfx_mf4%d.bmp", i+1);
		m_pEBSShotGunFire[i] = new ZEffectBillboardSource(szFileName);
	}

	for(int i=0; i<GUNFIRE_COUNT; i++){
		sprintf_safe(szFileName, "SFX/gz_sfx_mf2%d.bmp", i+1);
		m_pEBSGunFire[i][0] = new ZEffectBillboardSource(szFileName);
		sprintf_safe(szFileName, "SFX/gz_sfx_mf3%d.bmp", i+1);
		m_pEBSGunFire[i][1] = new ZEffectBillboardSource(szFileName);
	}

	m_pEffectMeshMgr = new RMeshMgr;

	if(m_pEffectMeshMgr->LoadXmlList("SFX/effect_list.xml")==-1) {
		mlog("effect_list loding error\n");
	}

	m_pMeshEmptyCartridges[0] = m_pEffectMeshMgr->Get("empty_cartridge1");
	m_pMeshEmptyCartridges[1] = m_pEffectMeshMgr->Get("empty_cartridge2");

	m_pSworddam[0] = m_pEffectMeshMgr->Get("sword_damage1");
	m_pSworddam[1] = m_pEffectMeshMgr->Get("sword_damage2");
	m_pSworddam[2] = m_pEffectMeshMgr->Get("sword_damage3");
	m_pSworddam[3] = m_pEffectMeshMgr->Get("sword_damage4");

	m_pRangeDamaged[0] = m_pEffectMeshMgr->Get("ef_damage01.elu");
	m_pRangeDamaged[1] = m_pEffectMeshMgr->Get("ef_damage02.elu");
	m_pRangeDamaged[2] = m_pEffectMeshMgr->Get("ef_damage03.elu");
	m_pRangeDamaged[3] = m_pEffectMeshMgr->Get("ef_damage04.elu");
	m_pRangeDamaged[4] = m_pEffectMeshMgr->Get("ef_damage05.elu");
	m_pRangeDamaged[5] = m_pEffectMeshMgr->Get("ef_damage06.elu");

	m_pSwordglaze  = m_pEffectMeshMgr->Get("sword_glaze");

	m_pDashEffect				= m_pEffectMeshMgr->Get("dash_effect");
	m_pGrenadeEffect			= m_pEffectMeshMgr->Get("grenade_effect");
	m_pGrenadeExpEffect			= m_pEffectMeshMgr->Get("ef_gre_ex");
	m_pRocketEffect				= m_pEffectMeshMgr->Get("rocket_effect");
	m_pSwordDefenceEffect[0]	= m_pEffectMeshMgr->Get("sword_defence_effect");
	m_pSwordDefenceEffect[1]	= m_pEffectMeshMgr->Get("sword_defence_effect2");
	m_pSwordDefenceEffect[2]	= m_pEffectMeshMgr->Get("sword_defence_effect3");
	m_pFragment[0]				= m_pEffectMeshMgr->Get("fragment01");
	m_pFragment[1]				= m_pEffectMeshMgr->Get("fragment02");
	m_pFragment[2]				= m_pEffectMeshMgr->Get("fragment03");
	m_pFragment[3]				= m_pEffectMeshMgr->Get("fragment04");
	m_pFragment[4]				= m_pEffectMeshMgr->Get("fragment05");
	m_pFragment[5]				= m_pEffectMeshMgr->Get("fragment06");

	m_pSwordWaveEffect[0]		= m_pEffectMeshMgr->Get("sword_wave_effect");
	m_pSwordWaveEffect[1]		= m_pEffectMeshMgr->Get("sword_slash_effect");

	m_pChargingEffect = m_pEffectMeshMgr->Get("ef_spirits.elu");
	m_pChargedEffect = m_pEffectMeshMgr->Get("ef_spirits.elu_1.elu");

	m_pPinkSwordWaveEffect = m_pEffectMeshMgr->Get("pink_massive");
	m_pGreenSwordWaveEffect = m_pEffectMeshMgr->Get("green_massive");
	m_pBlueSwordWaveEffect = m_pEffectMeshMgr->Get("blue_massive");

	m_pPinkChargingEffect = m_pEffectMeshMgr->Get("pink_massive_charging");
	m_pGreenChargingEffect = m_pEffectMeshMgr->Get("green_massive_charging");
	m_pBlueChargingEffect = m_pEffectMeshMgr->Get("blue_massive_charging");

	m_pPinkChargedEffect = m_pEffectMeshMgr->Get("pink_massive_charged");
	m_pGreenChargedEffect = m_pEffectMeshMgr->Get("green_massive_charged");
	m_pBlueChargedEffect = m_pEffectMeshMgr->Get("blue_massive_charged");

	m_pSwordEnchantEffect[0]	= m_pEffectMeshMgr->Get("ef_sworddam_fire");
	m_pSwordEnchantEffect[1]	= m_pEffectMeshMgr->Get("ef_sworddam_ice");
	m_pSwordEnchantEffect[2]	= m_pEffectMeshMgr->Get("ef_sworddam_flash");
	m_pSwordEnchantEffect[3]	= m_pEffectMeshMgr->Get("ef_sworddam_poison");

	m_pMagicDamageEffect		= m_pEffectMeshMgr->Get("magicmissile_damage");

	m_pMagicEffectWall[0]		= m_pEffectMeshMgr->Get("fireball_work_wall");
	m_pMagicEffectWall[1]		= m_pEffectMeshMgr->Get("icemissile_wall");
	m_pMagicEffectWall[2]		= m_pEffectMeshMgr->Get("magicmissile_wall");

	m_pSwordUppercutEffect		= m_pEffectMeshMgr->Get("sword_uppercut_effect");
	m_pSwordUppercutDamageEffect= m_pEffectMeshMgr->Get("sword_uppercut_damage_effect");
	m_pFlameMG					= m_pEffectMeshMgr->Get("flame_mg");
	m_pFlamePistol				= m_pEffectMeshMgr->Get("flame_pistol");
	m_pFlameRifle				= m_pEffectMeshMgr->Get("flame_rifle");
	m_pFlameShotgun				= m_pEffectMeshMgr->Get("flame_shotgun");

	m_pLevelUpEffect[0]	= m_pEffectMeshMgr->Get("levelup");
	m_pLevelUpEffect[1] = m_pEffectMeshMgr->Get("levelup01");
	m_pReBirthEffect	= m_pEffectMeshMgr->Get("rebirth");
	m_pEatBoxEffect		= m_pEffectMeshMgr->Get("ef_eatbox");
	m_pHealEffect		= m_pEffectMeshMgr->Get("ef_heal");
	m_pRepireEffect		= m_pEffectMeshMgr->Get("ef_repair");
	m_pExpanseAmmoEffect	= m_pEffectMeshMgr->Get("ef_ammunition");

	m_pDaggerUpper		= m_pEffectMeshMgr->Get("ef_dagger_upper");

	m_pSwordFire		= m_pEffectMeshMgr->Get("ef_sword_fire");
	m_pSwordElec		= m_pEffectMeshMgr->Get("ef_sword_flash");
	m_pSwordCold		= m_pEffectMeshMgr->Get("ef_sword_ice");
	m_pSwordPoison		= m_pEffectMeshMgr->Get("ef_sword_poison");

	m_pBulletOnWallEffect[0]	= m_pEffectMeshMgr->Get("ef_effect001.elu");
	m_pBulletOnWallEffect[1]	= m_pEffectMeshMgr->Get("ef_effect002.elu");

	m_pEBSRing[0] = new ZEffectBillboardSource("SFX/gd_effect_001.tga");
	m_pEBSRing[1] = new ZEffectBillboardSource("SFX/gd_effect_002.tga");

	m_pEBSDash[0] = new ZEffectBillboardSource("SFX/gz_effect_dash01.tga");
	m_pEBSDash[1] = new ZEffectBillboardSource("SFX/gz_effect_dash02.tga");

	m_pEBSLanding = new ZEffectBillboardSource("SFX/ef_gz_footstep.tga");

	m_pEBSWaterSplash	= new ZEffectBillboardSource("SFX/gd_effect_006.tga");
	m_pWaterSplash	= m_pEffectMeshMgr->Get("water_splash");

	m_pEBSWorldItemEaten	= new ZEffectBillboardSource("SFX/ef_sw.bmp");
	m_pWorldItemEaten = m_pEffectMeshMgr->Get("ef_eatitem");

	m_pCharacterIcons[0] = m_pEffectMeshMgr->Get("ef_pre_all.elu");
	m_pCharacterIcons[1] = m_pEffectMeshMgr->Get("ef_pre_un.elu");
	m_pCharacterIcons[2] = m_pEffectMeshMgr->Get("ef_pre_exe.elu");
	m_pCharacterIcons[3] = m_pEffectMeshMgr->Get("ef_pre_fan.elu");
	m_pCharacterIcons[4] = m_pEffectMeshMgr->Get("ef_pre_head.elu");

	m_pCommandIcons[0] = m_pEffectMeshMgr->Get("red_commander");
	m_pCommandIcons[1] = m_pEffectMeshMgr->Get("blue_commander");

	m_pLostConIcon = m_pEffectMeshMgr->Get("ef_lostcon.elu");
	m_pChatIcon = m_pEffectMeshMgr->Get("ef_chat.elu");

	m_pBerserkerEffect	= m_pEffectMeshMgr->Get("ef_berserker");

	m_pBlizzardEffect = m_pEffectMeshMgr->Get("ef_blizzard");

	m__skip_cnt = 0;
	m__cnt = 0;
	m__rendered = 0;

	CreateCommonRectVertexBuffer();

	ZEffectBase::CreateBuffers();

	m_BulletMarkList.Create("SFX/gz_sfx_shotgun_bulletmark01.tga");
	m_LightFragments.Create("SFX/gz_sfx_tracer.bmp");
	m_LightFragments.SetScale(rvector(4.0f,0.8f,1.0f));

	m_BillboardLists[0].Create("SFX/muzzle_smoke01.tga");
	m_BillboardLists[1].Create("SFX/muzzle_smoke02.tga");
	m_BillboardLists[2].Create("SFX/muzzle_smoke03.tga");
	m_BillboardLists[3].Create("SFX/muzzle_smoke04.tga");

	m_BillboardLists[4].Create("SFX/smoke_rocket.tga");

	m_ShadowList.Create("SFX/gz_shadow.tga");

	m_BillBoardTexAniList[0].Create("SFX/gd_effect_020.bmp");
	m_BillBoardTexAniList[1].Create("SFX/gd_effect_021.bmp");
	m_BillBoardTexAniList[2].Create("SFX/gd_effect_019.bmp");
	m_BillBoardTexAniList[3].Create("SFX/ef_magicmissile.bmp");
	m_BillBoardTexAniList[4].Create("SFX/ef_methor_smoke.tga");

	m_BillBoardTexAniList[0].SetTile(4,4,0.25f,0.25f);
	m_BillBoardTexAniList[1].SetTile(4,4,0.25f,0.25f);
	m_BillBoardTexAniList[2].SetTile(4,4,0.25f,0.25f);
	m_BillBoardTexAniList[2].m_bFixFrame = true;
	m_BillBoardTexAniList[3].SetTile(2,2,0.375f,0.375f);
	m_BillBoardTexAniList[3].m_bFixFrame = true;
	m_BillBoardTexAniList[4].SetTile(4,4,0.25f,0.25f);

	rvector veczero = rvector(0.f,0.f,0.f);

	m_pWeaponEnchant[ZC_ENCHANT_NONE]		= NULL;
	m_pWeaponEnchant[ZC_ENCHANT_FIRE]		= new ZEffectWeaponEnchant( m_pSwordFire,
		veczero,veczero, NULL );
	m_pWeaponEnchant[ZC_ENCHANT_COLD]		= new ZEffectWeaponEnchant( m_pSwordCold,
		veczero,veczero, NULL );
	m_pWeaponEnchant[ZC_ENCHANT_LIGHTNING]	= new ZEffectWeaponEnchant( m_pSwordElec,
		veczero, veczero, NULL);
	m_pWeaponEnchant[ZC_ENCHANT_POISON]		= new ZEffectWeaponEnchant( m_pSwordPoison,
		veczero, veczero, NULL);

	return true;
}

ZEffectWeaponEnchant* ZEffectManager::GetWeaponEnchant(ZC_ENCHANT type)
{
	if(type < ZC_ENCHANT_END) {
		return m_pWeaponEnchant[type];
	}
	return NULL;
}

ZEffectManager::~ZEffectManager()
{
	Clear();

	int i;

	for(i=0; i<MUZZLESMOKE_COUNT; i++){
		delete m_pEBSMuzzleSmoke[i];
	}

	for(i=0; i<MUZZLESMOKE_SHOTGUN_COUNT; i++){
		delete m_pEBSMuzzleSmokeShotgun[i];
	}

	for(i=0; i<SMOKE_COUNT; i++){
		delete m_pEBSSmokes[i];
	}
	for(i=0; i<BLOOD_COUNT; i++){
		delete m_pEBSBloods[i];
	}
	for(i=0; i<BLOODMARK_COUNT; i++){
		delete m_pEBSBloodMark[i];
	}

	delete m_pEBSLightTracer;

	delete m_pEBSBulletMark[0];
	delete m_pEBSBulletMark[1];
	delete m_pEBSBulletMark[2];

	for(int i=0; i<RIFLEFIRE_COUNT; i++){
		delete m_pEBSRifleFire[i][0];
		delete m_pEBSRifleFire[i][1];
	}

	for(int i=0; i<SHOTGUNFIRE_COUNT;i++) {
		delete m_pEBSShotGunFire[i];
	}

	for(int i=0; i<GUNFIRE_COUNT; i++){
		delete m_pEBSGunFire[i][0];
		delete m_pEBSGunFire[i][1];
	}

	if(m_pEffectMeshMgr)
		delete m_pEffectMeshMgr;

	delete m_pEBSRing[0];
	delete m_pEBSRing[1];

	delete m_pEBSDash[0];
	delete m_pEBSDash[1];

	delete m_pEBSLanding;
	delete m_pEBSWaterSplash;

	delete m_pEBSWorldItemEaten;

	for(int i=1;i<REnchantType_End;i++) {
		if(m_pWeaponEnchant[i]) {
			delete m_pWeaponEnchant[i];
			m_pWeaponEnchant[i] = NULL;
		}
	}

	RealeaseCommonRectVertexBuffer();

	ZEffectBase::ReleaseBuffers();

	ZEffectWeaponEnchant::Release();
	ZEffectStaticMesh::Release();
	ZEffectSlash::Release();
	ZEffectDash::Release();

	ZEffectPartsTypePos::Release();

	ZEffectShot::Release();

	ZEffectWeaponEnchant::Release();

	ZEffectSmoke::Release();
	ZEffectLandingSmoke::Release();
	ZEffectSmokeGrenade::Release();
	ZEffectLightTracer::Release();

	ZEffectLightFragment::Release();
	ZEffectLightFragment2::Release();

	ZEFFECTBILLBOARDITEM::Release();
	ZEFFECTBULLETMARKITEM::Release();
	ZEFFECTBILLBOARDTEXANIITEM::Release();

	ZEffectCharging::Release();
	ZEffectCharged::Release();
	ZEffectBerserkerIconLoop::Release();
}

int ZEffectManager::DeleteSameType(ZEffectAniMesh* pNew)
{
	int d = pNew->GetDrawMode();

	int cnt=0;

	ZEffectList::iterator node;
	ZEffect* pEffect = NULL;

	for(node = m_Effects[d].begin(); node != m_Effects[d].end(); ) {

		pEffect = (*node);

		if( pEffect->isEffectType( pNew->m_nType ) ) {
				
			if( ((ZEffectAniMesh*)pEffect)->GetUID() == pNew->GetUID() ) {

#ifndef _PUBLISH
				g_EffectValidator.Erase(pEffect);
#endif
				delete pEffect;
				node = m_Effects[d].erase( node );
				cnt++;
				continue;

			}
		}

		++node;
	}
	return cnt;
}

void ZEffectManager::Add(ZEffect* pNew)
{
	if(pNew==NULL) return;

#define MAX_WATER_DEEP 150

	rvector	src_pos = pNew->GetSortPos();

	_ASSERT(pNew->GetDrawMode()<ZEDM_COUNT);
	m_Effects[pNew->GetDrawMode()].insert(m_Effects[pNew->GetDrawMode()].end(), pNew);

#ifndef _PUBLISH
	g_EffectValidator.Add(pNew);
#endif
}

void ZEffectManager::Clear()
{
	for( int d = 0; d < ZEDM_COUNT; d++ ) {

		ZEffectList::iterator node;
		ZEffect* pEffect = NULL;

		for(node = m_Effects[d].begin(); node != m_Effects[d].end(); ) {
			pEffect = (*node);

#ifndef _PUBLISH
			g_EffectValidator.Erase(pEffect);
#endif

			delete pEffect;
			node = m_Effects[d].erase(node);
		}
	}

	m_LightFragments.Clear();

	for(int i=0;i<BILLBOARDLISTS_COUNT;i++)
		m_BillboardLists[i].Clear();

	m_ShadowList.Clear();

	for(int i=0;i<BILLBOARDTEXANILIST_COUNT;i++)
		m_BillBoardTexAniList[i].Clear();

	m_BulletMarkList.Clear();
}

bool e_effect_sort_float(ZEffect* _a,ZEffect* _b) {
	if( _a->m_fDist > _b->m_fDist )
		return true;
	return false;
}

static int g_zeffectmanager_cnt = 0;

#define ZEFFECT_RENDER_MAX 4000.f

void ZEffectManager::CheckWaterSkip(int mode,float height)
{
	for( int d = 0; d < ZEDM_COUNT; d++ ) {

		ZEffectList::iterator node;
		ZEffect* pEffect = NULL;

		for(node = m_Effects[d].begin(); node != m_Effects[d].end(); ) {
			pEffect = (*node);
			if(pEffect)
				pEffect->CheckWaterSkip(mode,height);
			++node;
		}

		m_Effects[d].sort( e_effect_sort_float );
	}
}

void ZEffectManager::Draw(u32 nTime,int mode,float height)
{
	// TODO: Remove
	IDirect3DStateBlock9* pStateBlock = NULL;
	HRESULT hr = RGetDevice()->CreateStateBlock(D3DSBT_PIXELSTATE, &pStateBlock);
	if (hr!=D3D_OK) {
		static int nErrorLogCount = 0;
		if(nErrorLogCount<100) {
			mlog("CreateStateBlock failed : %s",DXGetErrorString(hr));
		}
	}

	if(pStateBlock)	pStateBlock->Capture();

	rvector camera_pos = RealSpace2::RCameraPosition;
	rvector t_vec;

	CheckWaterSkip( mode , height );

	if(mode==1)
		m_ShadowList.Draw();

	for( int d = 0; d < ZEDM_COUNT; d++ ) {

		ZEffectList::iterator node;
		ZEffect* pEffect = NULL;

		int cnt = 0;

		for(node = m_Effects[d].begin(); node != m_Effects[d].end(); ) {
			pEffect = (*node);

			if(pEffect==NULL) {
				int _size = (int)m_Effects[d].size();
				mlog("뜨아.. EffectManager NULL 문제 발생 ( %d list 요소) : size : %d \n",d,_size);
				++node;
			} else {
				if(!pEffect->m_bWaterSkip) {
					t_vec = camera_pos - pEffect->GetSortPos();
					pEffect->m_fDist = Magnitude(t_vec);
					pEffect->CheckRenderAble( g_nEffectLevel,pEffect->m_fDist );
				}
				++node;
			}
			cnt++;
		}
		m_Effects[d].sort(e_effect_sort_float);
	}
	
	m__skip_cnt = 0;
	m__cnt = 0;
	m__rendered = 0;

	for(int d=0; d<ZEDM_COUNT; d++) {

		if(d==ZEDM_NONE) {
			RGetDevice()->SetRenderState(D3DRS_ALPHATESTENABLE,	FALSE);
			RGetDevice()->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
		}
		else if(d==ZEDM_ALPHAMAP) {
			RGetDevice()->SetRenderState(D3DRS_ALPHAREF, (DWORD)0x00000000);
			RGetDevice()->SetRenderState(D3DRS_ALPHAFUNC, D3DCMP_NOTEQUAL );
			RGetDevice()->SetRenderState(D3DRS_ALPHATESTENABLE,	TRUE);

			RGetDevice()->SetTextureStageState( 0, D3DTSS_COLORARG1 , D3DTA_TEXTURE );
			RGetDevice()->SetTextureStageState( 0, D3DTSS_COLOROP	, D3DTOP_SELECTARG1 );
			RGetDevice()->SetTextureStageState( 0, D3DTSS_ALPHAARG1 , D3DTA_TEXTURE );
			RGetDevice()->SetTextureStageState( 0, D3DTSS_ALPHAARG2 , D3DTA_TFACTOR );
			RGetDevice()->SetTextureStageState( 0, D3DTSS_ALPHAOP , D3DTOP_MODULATE );

			RGetDevice()->SetSamplerState( 0, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP );
			RGetDevice()->SetSamplerState( 0, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP );

			RGetDevice()->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
			RGetDevice()->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
			RGetDevice()->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);

			RGetDevice()->SetRenderState(D3DRS_ZWRITEENABLE, FALSE);
		}
		else if(d==ZEDM_ADD) {
			RGetDevice()->SetRenderState(D3DRS_ALPHATESTENABLE,	TRUE);
			RGetDevice()->SetRenderState(D3DRS_ZWRITEENABLE, FALSE);

			RGetDevice()->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
			RGetDevice()->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_ONE);
			RGetDevice()->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_ONE);

			RGetDevice()->SetTextureStageState( 0, D3DTSS_COLORARG1 , D3DTA_TEXTURE );
			RGetDevice()->SetTextureStageState( 0, D3DTSS_COLOROP	, D3DTOP_SELECTARG1 );
			RGetDevice()->SetTextureStageState( 0, D3DTSS_ALPHAARG1 , D3DTA_TEXTURE );
			RGetDevice()->SetTextureStageState( 0, D3DTSS_ALPHAOP	, D3DTOP_SELECTARG1 );

			RGetDevice()->SetSamplerState( 0, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP );
			RGetDevice()->SetSamplerState( 0, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP );

		}
		else{
			_ASSERT(FALSE);
		}

		ZEffectList::iterator node;
		ZEffect* pEffect = NULL;

		for(node = m_Effects[d].begin(); node != m_Effects[d].end(); ) {

			pEffect = (*node);

			if( pEffect ) {

				if(pEffect->m_bRender==false)
					m__skip_cnt++;

				if( !pEffect->m_bWaterSkip ) {

					if( pEffect->Draw(nTime)==false ) {
						if(pEffect->m_bisRendered) m__rendered++;//for debug
#ifndef _PUBLISH
						g_EffectValidator.Erase(pEffect);
#endif
						delete pEffect;
						node = m_Effects[d].erase(node);
						m__cnt++;
					} else {
						if(pEffect->m_bisRendered) m__rendered++;//for debug
						++node;
						m__cnt++;
					}

				}
				else {
					++node;
				}
			}
		}

	}

	if( mode==1 ) {

		m_BulletMarkList.Draw();
		m_LightFragments.Draw();

		for(int i=0;i<BILLBOARDLISTS_COUNT;i++)
			m_BillboardLists[i].Draw();

		for(int i=0;i<BILLBOARDTEXANILIST_COUNT;i++)
			m_BillBoardTexAniList[i].Draw();
	}

	if(pStateBlock) pStateBlock->Apply();
	SAFE_RELEASE( pStateBlock  );
}

void ZEffectManager::Draw(u32 nTime)
{
	LPDIRECT3DDEVICE9 pDevice = RGetDevice();

	// TODO: Remove
	IDirect3DStateBlock9* pStateBlock = NULL;
	RGetDevice()->CreateStateBlock(D3DSBT_PIXELSTATE, &pStateBlock);
	if(pStateBlock)	pStateBlock->Capture();

	m_ShadowList.Draw();

	pDevice->SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_TEXTURE );
	pDevice->SetTextureStageState( 0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);
	pDevice->SetTextureStageState( 0, D3DTSS_COLOROP,   D3DTOP_MODULATE );
	pDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE );
	pDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE );
	pDevice->SetTextureStageState( 0, D3DTSS_ALPHAOP,   D3DTOP_MODULATE );
	pDevice->SetTextureStageState( 1, D3DTSS_COLOROP,	D3DTOP_DISABLE );
	pDevice->SetTextureStageState( 1, D3DTSS_ALPHAOP,   D3DTOP_DISABLE );

	rvector camera_pos = RealSpace2::RCameraPosition;
	rvector t_vec;

	for( int d = 0; d < ZEDM_COUNT; d++ ) {

		ZEffectList::iterator node;
		ZEffect* pEffect = NULL;

		int cnt = 0;

		for(node = m_Effects[d].begin(); node != m_Effects[d].end(); ) {
			pEffect = (*node);

			if(pEffect==NULL) {

				int _size = (int)m_Effects[d].size();
				mlog("뜨아.. EffectManager NULL 문제 발생 ( %d list 요소) : size : %d \n",d,_size);
				++node;

			} else {

				t_vec = camera_pos - pEffect->GetSortPos();
				pEffect->m_fDist = Magnitude(t_vec);

				pEffect->CheckRenderAble( g_nEffectLevel,pEffect->m_fDist );

				++node;
			}
			cnt++;
		}

		m_Effects[d].sort(e_effect_sort_float);
	}

	m__skip_cnt = 0;
	m__cnt = 0;
	m__rendered = 0;

	for(int d=0; d<ZEDM_COUNT; d++) {

		if(d==ZEDM_NONE) {
			RGetDevice()->SetRenderState(D3DRS_ALPHATESTENABLE,	FALSE);
			RGetDevice()->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
		}
		else if(d==ZEDM_ALPHAMAP) {
			RGetDevice()->SetRenderState(D3DRS_ALPHAREF, (DWORD)0x00000000);
			RGetDevice()->SetRenderState(D3DRS_ALPHAFUNC, D3DCMP_NOTEQUAL );
			RGetDevice()->SetRenderState(D3DRS_ALPHATESTENABLE,	TRUE);

			RGetDevice()->SetTextureStageState( 0, D3DTSS_COLORARG1 , D3DTA_TEXTURE );
			RGetDevice()->SetTextureStageState( 0, D3DTSS_COLOROP	, D3DTOP_SELECTARG1 );
			RGetDevice()->SetTextureStageState( 0, D3DTSS_ALPHAARG1 , D3DTA_TEXTURE );
			RGetDevice()->SetTextureStageState( 0, D3DTSS_ALPHAARG2 , D3DTA_TFACTOR );
			RGetDevice()->SetTextureStageState( 0, D3DTSS_ALPHAOP , D3DTOP_MODULATE );

			RGetDevice()->SetSamplerState( 0, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP );
			RGetDevice()->SetSamplerState( 0, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP );

			RGetDevice()->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
			RGetDevice()->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
			RGetDevice()->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);

			RGetDevice()->SetRenderState(D3DRS_ZWRITEENABLE, FALSE);
		}
		else if(d==ZEDM_ADD) {
			RGetDevice()->SetRenderState(D3DRS_ALPHATESTENABLE,	TRUE);
			RGetDevice()->SetRenderState(D3DRS_ZWRITEENABLE, FALSE);

			RGetDevice()->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
			RGetDevice()->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_ONE);
			RGetDevice()->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_ONE);

			RGetDevice()->SetTextureStageState( 0, D3DTSS_COLORARG1 , D3DTA_TEXTURE );
			RGetDevice()->SetTextureStageState( 0, D3DTSS_COLOROP	, D3DTOP_SELECTARG1 );
			RGetDevice()->SetTextureStageState( 0, D3DTSS_ALPHAARG1 , D3DTA_TEXTURE );
			RGetDevice()->SetTextureStageState( 0, D3DTSS_ALPHAOP	, D3DTOP_SELECTARG1 );

			RGetDevice()->SetSamplerState( 0, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP );
			RGetDevice()->SetSamplerState( 0, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP );

		}
		else{
			_ASSERT(FALSE);
		}

		ZEffectList::iterator node;
		ZEffect* pEffect = NULL;

		for(node = m_Effects[d].begin(); node != m_Effects[d].end(); ) {

			pEffect = (*node);

			if( pEffect ) {

				if(pEffect->m_bRender==false)
					m__skip_cnt++;

				if( pEffect->Draw(nTime)==false ) {
					if(pEffect->m_bisRendered) m__rendered++;//for debug
#ifndef _PUBLISH
					g_EffectValidator.Erase(pEffect);
#endif
					delete pEffect;
					node = m_Effects[d].erase(node);
					m__cnt++;
				} else {
					if(pEffect->m_bisRendered) m__rendered++;//for debug
					++node;
					m__cnt++;
				}
			}
		}
	}

	m_BulletMarkList.Draw();

	m_LightFragments.Draw();

	for(int i=0;i<BILLBOARDLISTS_COUNT;i++)
		m_BillboardLists[i].Draw();

	for(int i=0;i<BILLBOARDTEXANILIST_COUNT;i++)
		m_BillBoardTexAniList[i].Draw();

	if(pStateBlock) pStateBlock->Apply();
	SAFE_RELEASE( pStateBlock  );
}

void ZEffectManager::Update(float fElapsed)
{
	m_LightFragments.Update(fElapsed);
	for(int i=0;i<BILLBOARDLISTS_COUNT;i++)
		m_BillboardLists[i].Update(fElapsed);

	m_BulletMarkList.Update(fElapsed);

	for(int i=0;i<BILLBOARDTEXANILIST_COUNT;i++)
		m_BillBoardTexAniList[i].Update(fElapsed);

	m_ShadowList.Update(fElapsed);
}

void ZEffectManager::AddLevelUpEffect(ZObject* pObj)
{
	ZEffect* pNew = NULL;

	ZCharacter* pChar = MDynamicCast(ZCharacter, pObj);

	if(!pChar) return;

	rvector Target = pChar->GetPosition();
	rvector dir = pChar->GetLowerDir();

	pNew = new ZEffectLevelUp(m_pLevelUpEffect[0],Target,dir,rvector(0.f,0.f,0.f),pObj);
	((ZEffectLevelUp*)pNew)->SetAlignType(1);
	((ZEffectLevelUp*)pNew)->m_type = eq_parts_pos_info_Spine2;
	Add(pNew);

	pNew = new ZEffectLevelUp(m_pLevelUpEffect[1],Target,dir,rvector(0.f,0.f,0.f),pObj);
	((ZEffectLevelUp*)pNew)->SetAlignType(1);
	((ZEffectLevelUp*)pNew)->m_type = eq_parts_pos_info_Root;

	Add(pNew);
}

void ZEffectManager::AddReBirthEffect(const rvector& Target)
{
	ZEffect* pNew = NULL;

	rvector dir = rvector(0.f,1.f,0.f);
	pNew = new ZEffectSlash(m_pReBirthEffect,Target,dir);
	((ZEffectSlash*)pNew)->SetAlignType(1);
	Add(pNew);
}


void ZEffectManager::AddLightFragment(rvector Target,rvector TargetNormal)
{
#define TARGET_FIREFRAGMENT_COUNT		5
#define TARGET_FIREFRAGMENT_MAXCOUNT	16

	if(g_nEffectLevel > Z_VIDEO_EFFECT_NORMAL)
		return;
	
	else if( g_nEffectLevel == Z_VIDEO_EFFECT_NORMAL ) 
	{
		auto vec = g_pGame->m_pMyCharacter->GetPosition() - Target;
		float fDistacneSQ = MagnitudeSq(vec);
		if( fDistacneSQ > 640000 ) return;
	}

	int nFireFragmentCount = 0;
	int j = 0;
	while(nFireFragmentCount<TARGET_FIREFRAGMENT_COUNT && j<TARGET_FIREFRAGMENT_MAXCOUNT){
		rvector r(float(rand()%200-100), float(rand()%200-100), float(rand()%200-100));
		if(r.x==0 && r.y==0 && r.z==0) continue;
		if(DotProduct(r, TargetNormal)<0) continue;

#define RANDOM_POSITION_MAXDISTANCE	3

		rvector rp(
			RANDOM_POSITION_MAXDISTANCE*(rand()%200-100)/100.0f,
			RANDOM_POSITION_MAXDISTANCE*(rand()%200-100)/100.0f,
			RANDOM_POSITION_MAXDISTANCE*(rand()%200-100)/100.0f);

#define LIGHTTRACER_SPEED	4.0f	// m/s
		Normalize(r);

		m_LightFragments.Add(Target+rp,r*LIGHTTRACER_SPEED*50.f,rvector(0,0,-500.f),1.6f,1.6f,1.2f);

		nFireFragmentCount++;
	}
}

#pragma optimize ( "", off )

void ZEffectManager::AddDashEffect(const rvector& Target, const rvector& TargetNormal,ZObject* pObj)
{
	if(!pObj->IsVisible()) return;

	ZEffect* pNew = NULL;
	pNew = new ZEffectDash(m_pDashEffect,Target,TargetNormal,pObj->GetUID());
	((ZEffectSlash*)pNew)->SetAlignType(1);
	Add(pNew);
}
#pragma optimize ( "", on)

void ZEffectManager::AddSkillDashEffect(const rvector& Target, const rvector& TargetNormal,ZObject* pObj)
{
	ZEffect* pNew = NULL;
	pNew = new ZEffectDash(m_pDaggerUpper,Target,TargetNormal,pObj->GetUID());
	((ZEffectSlash*)pNew)->SetAlignType(1);
	Add(pNew);
}

void ZEffectManager::AddLandingEffect(const rvector& Target, const rvector& TargetNormal)
{
	if(g_nEffectLevel > Z_VIDEO_EFFECT_NORMAL)
		return;

#define LANDING_SMOKE_MAX_SCALE				70.0f	
#define LANDING_SMOKE_MIN_SCALE				70.0f	
#define LANDING_SMOKE_LIFE_TIME				3000

	ZEffect* pNew = NULL;

	auto vec = Target;
	vec.z += 5.f;

	pNew = new ZEffectLandingSmoke(m_pEBSLanding, vec, 
		TargetNormal, LANDING_SMOKE_MIN_SCALE, LANDING_SMOKE_MAX_SCALE, LANDING_SMOKE_LIFE_TIME);

	Add(pNew);

}

void ZEffectManager::AddBulletMark(const rvector& Target, const rvector& TargetNormal)
{
	if(g_nEffectLevel > Z_VIDEO_EFFECT_NORMAL) return;

	m_BulletMarkList.Add(Target+TargetNormal,TargetNormal);

	ZEffect* pNew = NULL;
	pNew = new ZEffectSlash(m_pBulletOnWallEffect[rand()%BULLETONWALL_COUNT],Target,TargetNormal);
	((ZEffectSlash*)pNew)->SetAlignType(1);
	Add(pNew);
}

void ZEffectManager::AddTrackMagic(const rvector &pos)
{
	int Add = rand() % 10;
	float fStartSize = 16 + Add;
	float fEndSize = 28 + Add;
	float fLife = 1.0f;

	int frame = rand()%4;
	rvector vel = rvector(0,0,-25.f);

	m_BillBoardTexAniList[3].Add( pos, vel,frame, 0.f,fStartSize , fEndSize, fLife );
}

void ZEffectManager::AddTrackFire(const rvector &pos)
{
	int Add = rand() % 20;
	float fStartSize = 10 + Add;
	float fEndSize = 20 + Add;
	float fLife = 0.4f;
	rvector vel = rvector(0,0,25);

	m_BillBoardTexAniList[1].Add( pos, vel, 0, 0.f,fStartSize , fEndSize, fLife );
}

void ZEffectManager::AddTrackCold(const rvector &pos)
{
	int Add = rand() % 10;
	float fStartSize = 8 + Add;
	float fEndSize = 14 + Add;
	float fLife = 1.0f;

	int frame = rand()%8;
	rvector vel = rvector(0,0,-25.f);

	m_BillBoardTexAniList[2].Add( pos, vel,frame, 0.f,fStartSize , fEndSize, fLife );
}

void ZEffectManager::AddTrackPoison(const rvector &pos)
{
	int Add = rand() % 10;
	float fStartSize = 10 + Add;
	float fEndSize = 20 + Add;
	float fLife = 1.0f;

	static int r_frame[4] = { 8,9,12,13 };

	int frame = rand()%4;

	rvector vel = rvector(0,0,0);

	m_BillBoardTexAniList[2].Add( pos, vel,r_frame[frame],0.f, fStartSize , fEndSize, fLife );
}

void ZEffectManager::AddTrackMethor(const rvector &pos)
{
	int Add = rand() % 60;
	float fStartSize = 80 + Add;
	float fEndSize = 100 + Add;
	float fLife = 1.0f;

	rvector vel = rvector(0,0,0);
	rvector add = 50.f*rvector(RANDOMFLOAT-0.5f,RANDOMFLOAT-0.5f,RANDOMFLOAT-0.5f);

	m_BillBoardTexAniList[4].Add( pos + add, vel, 0, 0.f,fStartSize , fEndSize, fLife );
	
	add = 50.f*rvector(RANDOMFLOAT-0.5f,RANDOMFLOAT-0.5f,RANDOMFLOAT-0.5f);
	m_BillboardLists[4].Add(pos,add,rvector(0,0,0), 30.f, 120.f, 3.f );
}


static RMeshPartsPosInfoType g_EnchantEffectPartsPos[5]=
{
	eq_parts_pos_info_Head,
	eq_parts_pos_info_Spine,
	eq_parts_pos_info_Spine1,
	eq_parts_pos_info_LUpperArm,
	eq_parts_pos_info_RUpperArm
};

#define EFRand rand() % 15
#define EFRandTime (1 + rand() % 6) / 10.f

rvector GetRandVec(int V)
{
	rvector v;

	v.x = rand()%10; if(rand()%2) v.x = -v.x;
	v.y = rand()%10; if(rand()%2) v.y = -v.y;
	v.z = rand()%10; if(rand()%2) v.z = -v.z;

	return v;
}

int GetRandType(int nRand,int nEx,int nMax)
{
	int hr = 0;
	int _rand = 0;

	for(int i=0;i<nMax;i++)
	{
		_rand = rand() % nRand;

		if(_rand != nEx)
			return _rand;
	}

	return _rand;
}

float ZEffectManager::GetEnchantDamageObjectSIze(ZObject* pObj)
{
	float fSize = 1.f;

	ZActor* pActor = MDynamicCast(ZActor,pObj);

	if(pActor&&pActor->GetNPCInfo()) { 

		fSize = pActor->GetNPCInfo()->fCollRadius / 35.f;
		fSize *= fSize;
	}

	return fSize;
}

void ZEffectManager::AddEnchantFire2(ZObject* pObj)
{
	if(pObj==NULL) return;

	float fSize = GetEnchantDamageObjectSIze( pObj );

	float fStartSize = (10.f + EFRand) * fSize;
	float fEndSize = (20.f + EFRand) * fSize;
	float fLife = 1.0f;

	rvector pos,parts_pos;
	rvector vel = rvector(0.f,0.f,25.f);

	static int partstype;
	
	partstype = GetRandType(5,partstype,10);

	rvector camera_dir = RCameraDirection * 20.f * fSize;

	parts_pos = pObj->m_pVMesh->GetBipTypePosition( g_EnchantEffectPartsPos[partstype] );

	if( Magnitude(parts_pos) < 0.1f )
		return;

	pos = parts_pos - camera_dir;

	pos += GetRandVec(10);

	m_BillBoardTexAniList[0].Add(pos,vel, 0, 0 ,fStartSize, fEndSize, fLife );
}

void ZEffectManager::AddEnchantCold2(ZObject* pObj)
{
	if(pObj==NULL) return;

	float fSize = GetEnchantDamageObjectSIze( pObj );

	float fStartSize = (10.f + EFRand) * fSize;
	float fEndSize = (20.f + EFRand) * fSize;
	float fLife = 1.0f;

	rvector pos;
	rvector vel = rvector(0.f,0.f,-25.f);

	static int partstype;

	partstype = GetRandType(5,partstype,10);

	rvector camera_dir = RCameraDirection * 20.f * fSize;

	pos = pObj->m_pVMesh->GetBipTypePosition( g_EnchantEffectPartsPos[partstype] ) - camera_dir;

	pos += GetRandVec(10);

	int nTex = rand() % 7;

	m_BillBoardTexAniList[2].Add(pos,vel, nTex, 0 ,fStartSize, fEndSize, fLife );
}

void ZEffectManager::AddEnchantPoison2(ZObject* pObj)
{
	if(pObj==NULL) return;

	float fSize = GetEnchantDamageObjectSIze( pObj );

	float fStartSize = (10.f + EFRand) * fSize;
	float fEndSize = (20.f + EFRand) * fSize;
	float fLife = 1.0f;

	rvector pos;
	rvector vel = rvector(0,0,0);

	static int partstype;
	
	partstype = GetRandType(5,partstype,10);

	rvector camera_dir = RCameraDirection * 20.f * fSize;

	pos = pObj->m_pVMesh->GetBipTypePosition( g_EnchantEffectPartsPos[partstype] ) - camera_dir;

	pos += GetRandVec(10);

	static int _tex_data[] = {8,9,12,13};

	int nTex = _tex_data[ rand()%4 ];

	m_BillBoardTexAniList[2].Add(pos,vel, nTex, 0 ,fStartSize, fEndSize, fLife );
}

void ZEffectManager::AddShotgunEffect(const rvector &pos, const rvector &out, const rvector &dir,ZObject* pChar )
{
	ZEffect * pNew = NULL;

#define SHOTGUN_SMOKE_LIFE_TIME		0.9f
#define SHOTGUN_SMOKE_MAX_SCALE		60.0f
#define SHOTGUN_SMOKE_MIN_SCALE		100.0f

	rvector _pos = pos;

	for(int i=0;i<1;i++)
	{
		_pos = rvector(pos.x + rand()%30,pos.y + rand()%30,pos.z + rand()%30);
		AddSmokeEffect(m_pEBSMuzzleSmokeShotgun[rand()%MUZZLESMOKE_SHOTGUN_COUNT], _pos, rvector(0,0,0), rvector(0,0,0), SHOTGUN_SMOKE_MIN_SCALE , SHOTGUN_SMOKE_MAX_SCALE, SHOTGUN_SMOKE_LIFE_TIME);
	}

	rvector _dir=rvector(0,0,1);

	if(pChar) {
		if(pChar->m_pVMesh) {
			rmatrix* mat = &pChar->m_pVMesh->m_WeaponDummyMatrix[weapon_dummy_muzzle_flash];

			_dir.x = mat->_21;
			_dir.y = mat->_22;
			_dir.z = mat->_23;
		}
	}


	pNew = NULL;
	pNew = new ZEffectShot(m_pFlameShotgun, pos, _dir , NULL);
	((ZEffectShot*)pNew)->SetAlignType(1);

	Add(pNew);

#define EM_VELOCITY		2.0f	// meter/sec
#define EM_RANDOM_SCALE	0.5f

	rvector right;
	CrossProduct(&right,-dir,rvector(0, 0, 1));
	Normalize(right);

	rvector EMRandom((rand()%100)/100.0f, (rand()%100)/100.0f, (rand()%100)/100.0f);
	EMRandom *= EM_RANDOM_SCALE;
	rvector EMVelocity = (right+rvector(0, 0, 1) + EMRandom) * EM_VELOCITY;
	pNew = new ZEffectStaticMesh( m_pMeshEmptyCartridges[1], out, EMVelocity, pChar->GetUID() );
	Add(pNew); pNew = NULL;
}

void ZEffectManager::AddShotEffect(rvector* pSource, int size, const rvector& Target,
	const rvector& TargetNormal, ZTargetType nTargetType, MMatchWeaponType wtype, ZObject* pObj,
	bool bDrawFireEffects, bool bDrawTargetEffects)
{
	rvector Source  = pSource[0];
	rvector SourceL = pSource[3];

	rvector right;
	CrossProduct(&right,TargetNormal,rvector(0, 0, 1));
	Normalize(right);

	rvector GTargetL = Target + right*(5+(rand()%10));
	rvector GTargetR = Target - right*(5+(rand()%10));

	rvector TargetDir = Target - Source;
	Normalize(TargetDir);

	ZEffect* pNew = NULL;

	if(rand()%5==0){
		pNew = new ZEffectLightTracer(m_pEBSLightTracer, Source, Target);
		Add(pNew); pNew = NULL;
	}

	if(bDrawFireEffects) {

#define SMOKE_MAX_SCALE				30.0f
#define SMOKE_MIN_SCALE				90.0f
#define SMOKE_LIFE_TIME				0.6f
#define SMOKE_SHOTGUN_LIFE_TIME		300
#define SMOKE_ACCEL					rvector(0,0,50.f)
#define SMOKE_VELOCITY				110.f

		if(wtype==MWT_ROCKET) {
			pNew = NULL;
		} 
		else if(wtype==MWT_MACHINEGUN) {

			rvector _Add;

			float min_scale = SMOKE_MIN_SCALE;
			float max_scale = SMOKE_MAX_SCALE;
			DWORD life = SMOKE_LIFE_TIME;

			for(int i=0;i<1;i++) {

				min_scale = SMOKE_MIN_SCALE * (rand()%2);
				max_scale = SMOKE_MAX_SCALE * (rand()%2);
				life	  = SMOKE_LIFE_TIME * (rand()%3);

					if(i%4==0)	_Add = rvector(-rand()%20 , rand()%20 , rand()%20 );
				else if(i%4==1)	_Add = rvector( rand()%20 ,-rand()%20 , rand()%20 );
				else if(i%4==2)	_Add = rvector( rand()%20 , rand()%20 ,-rand()%20 );
				else if(i%4==3)	_Add = rvector( rand()%20 , rand()%20 , rand()%20 );

				AddSmokeEffect(m_pEBSMuzzleSmoke[rand()%MUZZLESMOKE_COUNT], Source+_Add, SMOKE_VELOCITY*TargetDir,SMOKE_ACCEL, min_scale, max_scale, SMOKE_LIFE_TIME );
			}
		}
		else {
			AddSmokeEffect(m_pEBSMuzzleSmoke[rand()%MUZZLESMOKE_COUNT], Source, SMOKE_VELOCITY*TargetDir,SMOKE_ACCEL, SMOKE_MIN_SCALE, SMOKE_MAX_SCALE, SMOKE_LIFE_TIME);
		}

		rvector _dir=rvector(0,0,1);

		if(pObj) {
			if(pObj->m_pVMesh) {
				rmatrix* mat = &pObj->m_pVMesh->m_WeaponDummyMatrix[weapon_dummy_muzzle_flash];
				
				_dir.x = mat->_21;
				_dir.y = mat->_22;
				_dir.z = mat->_23;
			}
		}

		Normalize(_dir);

		switch(wtype) {
			case MWT_PISTOL:
			case MWT_PISTOLx2:
			case MWT_REVOLVER:
			case MWT_REVOLVERx2:
			case MWT_SMG:
			case MWT_SMGx2:
				{
					pNew = new ZEffectShot(m_pFlamePistol, Source , _dir, pObj);
					((ZEffectShot*)pNew)->SetAlignType(1);
				}
				break;
			case MWT_RIFLE:
				{
					pNew = new ZEffectShot(m_pFlameRifle, Source , _dir , pObj);
					((ZEffectShot*)pNew)->SetAlignType(1);
				}
				break;
			case MWT_MACHINEGUN:
				{
					pNew = new ZEffectShot(m_pFlameMG, Source , _dir , pObj);
					((ZEffectShot*)pNew)->SetAlignType(1);
				}
				break;
			default:
				pNew = NULL;
				break;
		}

		Add(pNew); pNew = NULL;

		if(size==6) {

			rvector _dir = GTargetR-Source;
			Normalize(_dir);

			pNew = new ZEffectShot(m_pFlamePistol, Source, _dir,pObj);
			((ZEffectShot*)pNew)->SetStartTime(120);
			((ZEffectShot*)pNew)->SetIsLeftWeapon(true);

			Add(pNew); pNew = NULL;

			AddSmokeEffect(m_pEBSMuzzleSmoke[rand()%MUZZLESMOKE_COUNT], Source, SMOKE_VELOCITY*TargetDir,SMOKE_ACCEL, SMOKE_MIN_SCALE, SMOKE_MAX_SCALE, SMOKE_LIFE_TIME);
		}

		bool bRender = true;

		if(g_nEffectLevel==Z_VIDEO_EFFECT_HIGH) {
			bRender = true;
		}
		else if(g_nEffectLevel==Z_VIDEO_EFFECT_NORMAL) {
			static bool toggle = true;
			if(toggle) {
				bRender = true;
			}
			else {
				bRender = false;
			}
			toggle = !toggle;
		}
		else if(g_nEffectLevel==Z_VIDEO_EFFECT_LOW) {
			bRender = false;
		}

	
		if(bRender) {
		
			ZCharacterObject* pCObj = MDynamicCast(ZCharacterObject, pObj);
			if(pCObj)
			{
				if(wtype!=MWT_SHOTGUN && pCObj->IsHero() )
				{
				
		#define EM_VELOCITY		2.0f	// meter/sec
		#define EM_RANDOM_SCALE	0.5f

					rvector EMRandom((rand()%100)/100.0f, (rand()%100)/100.0f, (rand()%100)/100.0f);
					EMRandom*=EM_RANDOM_SCALE;
					rvector EMVelocity = (right+rvector(0, 0, 1)+EMRandom)* EM_VELOCITY;
					pNew = new ZEffectStaticMesh(m_pMeshEmptyCartridges[0], pSource[1],
						EMVelocity, pObj->GetUID());
					Add(pNew); pNew = NULL;

					if(size==6) {
						rvector EMVelocityL = (-right+rvector(0, 0, 1)+EMRandom)* EM_VELOCITY;
						pNew = new ZEffectStaticMesh(m_pMeshEmptyCartridges[0], pSource[4], EMVelocityL, pObj->GetUID() );
						Add(pNew); pNew = NULL;
					}
				}

			}
		}
	}

	if(bDrawTargetEffects) {
		if(nTargetType==ZTT_CHARACTER_GUARD) {

			AddLightFragment(Target,TargetNormal);
		}

		if(nTargetType==ZTT_OBJECT){
			if(size==6) {
				AddBulletMark(GTargetL,TargetNormal);
				AddBulletMark(GTargetR,TargetNormal);
			}
			else {

				AddBulletMark(Target,TargetNormal);
			}

			if(wtype != MWT_SHOTGUN)
			{
#define TARGET_SMOKE_MAX_SCALE		50.0f
#define TARGET_SMOKE_MIN_SCALE		40.0f
#define TARGET_SMOKE_LIFE_TIME		0.9f
#define TARGET_SMOKE_VELOCITY		0.2f				// meter/sec
#define TARGET_SMOKE_ACCEL			rvector(0,0,100.f)	// meter/sec

				int max_cnt = 0;

					if(g_nEffectLevel==Z_VIDEO_EFFECT_HIGH)		max_cnt = 3;
				else if(g_nEffectLevel==Z_VIDEO_EFFECT_NORMAL)	max_cnt = 2;
				else if(g_nEffectLevel==Z_VIDEO_EFFECT_LOW)		max_cnt = 1;

				if(max_cnt) {
				
					for(int i=0; i<max_cnt; i++) {
						rvector p = Target+TargetNormal*TARGET_SMOKE_MIN_SCALE*float(i)*0.5f + rvector(fmod((float)rand(), TARGET_SMOKE_MIN_SCALE), fmod((float)rand(), TARGET_SMOKE_MIN_SCALE), fmod((float)rand(), TARGET_SMOKE_MIN_SCALE));
						float fSize = 1.0f+float(rand()%100)/100.0f;
						AddSmokeEffect(m_pEBSSmokes[rand()%SMOKE_COUNT], p, TargetNormal*TARGET_SMOKE_VELOCITY,rvector(0,100.f,0), TARGET_SMOKE_MIN_SCALE*fSize, TARGET_SMOKE_MAX_SCALE*fSize, TARGET_SMOKE_LIFE_TIME);
						
					}
				}
			}

		}
		else if(nTargetType==ZTT_CHARACTER){
#define TARGET_BLOOD_MAX_SCALE		50.0f
#define TARGET_BLOOD_MIN_SCALE		20.0f
#define TARGET_BLOOD_LIFE_TIME		500
#define TARGET_BLOOD_VELOCITY		4.0f	// meter/sec

			static DWORD last_add_time = GetGlobalTimeMS();
			static DWORD this_time;

			this_time = GetGlobalTimeMS();

			if(rand()%3==0)
			{
				pNew = new ZEffectSlash(m_pRangeDamaged[rand()%6],Target,TargetNormal);	
				((ZEffectSlash*)pNew)->SetScale(rvector(1.0f,1.0f,1.0f));
				Add(pNew);
			}
		}
	}
}

void ZEffectManager::AddSwordWaveEffect(const MUID& UID, const rvector &Target, const rvector &Dir)
{
	ZEffectSlash* pNew = nullptr;

	rvector dir = Dir;
	dir.z = 0.f;
	Normalize(dir);

	auto pair = GetRGMain().GetPlayerSwordColor(UID);

	if (pair.first)
	{
		hsv HSVColor;
		HSVColor.h = rand() % 360;
		HSVColor.s = 1.0;
		HSVColor.v = 1.0;

		auto GetRainbowColor = [HSVColor]() mutable
		{
			double Delta = GetRGMain().GetElapsedTime();

			HSVColor.h += 360.0 / 2.0 * Delta;
			HSVColor.h = fmod(HSVColor.h, 360.0);

			rgb RGBColor = hsv2rgb(HSVColor);

			uint32_t Color = 0xFF000000 | (int(RGBColor.r * 255) << 16) | (int(RGBColor.g * 255) << 8) | int(RGBColor.b * 255);

			return Color;
		};

		auto GetColor = [GetRainbowColor, Color = pair.second]() mutable
		{
			if (Color == 0x12345678)
				return GetRainbowColor();

			return Color;
		};

		pNew = MakeTexBlendEffect<ZEffectSlash>(GetColor, m_pSwordWaveEffect[1], Target, dir);
	}
	else
	{
		pNew = new ZEffectSlash(m_pSwordWaveEffect[1], Target, dir);
	}

	pNew->SetAlignType(1);

	Add(pNew);
}

void ZEffectManager::AddSwordEnchantEffect(ZC_ENCHANT type, const rvector& Target, DWORD start_time, float fScale)
{
	ZEffect* pNew = NULL;

	rvector dir = rvector(0.f,1.f,0.f);

	RMesh* pMesh = NULL;

		 if( type ==  ZC_ENCHANT_FIRE )			pMesh = m_pSwordEnchantEffect[0];
	else if( type ==  ZC_ENCHANT_COLD )			pMesh = m_pSwordEnchantEffect[1];
	else if( type ==  ZC_ENCHANT_LIGHTNING )	pMesh = m_pSwordEnchantEffect[2];
	else if( type ==  ZC_ENCHANT_POISON )		pMesh = m_pSwordEnchantEffect[3];
	else 
		return;


	pNew = new ZEffectSlash(pMesh,Target,dir);
	((ZEffectSlash*)pNew)->SetScale(rvector(fScale,fScale,fScale));
	((ZEffectSlash*)pNew)->SetAlignType(1);
	((ZEffectSlash*)pNew)->SetStartTime(start_time);
	Add(pNew);
}

void ZEffectManager::AddMagicEffect(const rvector& Target,DWORD start_time, float fScale)
{
	ZEffect* pNew = NULL;

	rvector dir = rvector(0.f,1.f,0.f);

	pNew = new ZEffectSlash(m_pMagicDamageEffect,Target,dir);
	((ZEffectSlash*)pNew)->SetScale(rvector(fScale,fScale,fScale));
	((ZEffectSlash*)pNew)->SetAlignType(1);
	((ZEffectSlash*)pNew)->SetStartTime(start_time);
	Add(pNew);
}

void ZEffectManager::AddMethorEffect(const rvector& Target,int nCnt)
{
	ZEffect* pNew = NULL;

	rvector dir		= rvector(0.f,1.f,0.f);
	rvector AddPos	= rvector(0.f,0.f,0.f);

	RMesh* pMesh = m_pEffectMeshMgr->Get("ef_methor");
	
	pNew = new ZEffectSlash(pMesh,Target,dir);

	((ZEffectSlash*)pNew)->SetAlignType(1);
	((ZEffectSlash*)pNew)->SetStartTime(0);
	((ZEffectSlash*)pNew)->m_nAutoAddEffect = ZEffectAutoAddType_Methor;

	Add(pNew);
}

void ZEffectManager::AddBlizzardEffect(const rvector& Target,int nCnt)
{
	ZEffect* pNew = NULL;

	rvector dir = rvector(0.f,1.f,0.f);
	rvector AddPos;

	for (int i = 0; i < nCnt; i++) {
		AddPos.x = rand() % 100;
		AddPos.y = rand() % 100;
		AddPos.z = 0.f;

		if(rand()%2) AddPos.x = -AddPos.x;
		if(rand()%2) AddPos.y = -AddPos.y;

		AddPos = AddPos + Target;

		pNew = new ZEffectSlash(m_pBlizzardEffect,AddPos,dir);

		((ZEffectSlash*)pNew)->SetAlignType(1);
		((ZEffectSlash*)pNew)->SetStartTime( i * 200 );

		Add(pNew);
	}
}


void ZEffectManager::AddMagicEffectWall(int type, const rvector& Target, const rvector& vDir,DWORD start_time, float fScale)
{
	ZEffect* pNew = NULL;

	RMesh* pMesh = NULL;

		 if( type ==  0 )	pMesh = m_pMagicEffectWall[0];
	else if( type ==  1 )	pMesh = m_pMagicEffectWall[1];
	else if( type ==  2 )	pMesh = m_pMagicEffectWall[2];
	else 
		return;


	pNew = new ZEffectSlash(pMesh,Target,vDir);
	((ZEffectSlash*)pNew)->SetScale(rvector(fScale,fScale,fScale));
	((ZEffectSlash*)pNew)->SetAlignType(1);
	((ZEffectSlash*)pNew)->SetStartTime(start_time);
	Add(pNew);

}


void ZEffectManager::AddSwordDefenceEffect(const rvector& Target, const rvector& vDir)
{
	ZEffect* pNew = NULL;

#define _SPREAD_SWORD_DEFENCE_EFFECT 50

	rvector add;

	add.x = rand()%_SPREAD_SWORD_DEFENCE_EFFECT;
	add.y = rand()%_SPREAD_SWORD_DEFENCE_EFFECT;
	add.z = rand()%_SPREAD_SWORD_DEFENCE_EFFECT;

	if(rand()%2) add.x=-add.x;
	if(rand()%2) add.y=-add.y;
	if(rand()%2) add.z=-add.z;

	float rot_angle = 3.14f/8.f * (rand()%16);

	int mode = rand()%3;

	pNew = new ZEffectSlash(m_pSwordDefenceEffect[mode],Target+add,vDir);
	((ZEffectSlash*)pNew)->SetRotationAngle(rot_angle);
	((ZEffectSlash*)pNew)->SetAlignType(1);
	Add(pNew);
}

void ZEffectManager::AddWaterSplashEffect(const rvector& Target, const rvector& Scale  )
{
	rvector dir = -RealSpace2::RCameraDirection;
 	ZEffect* pNew	= new ZEffectSlash( m_pWaterSplash, Target, dir );
	((ZEffectSlash*)pNew)->SetScale( Scale );

	Add(pNew);
}

void ZEffectManager::AddRocketEffect(const rvector& Target, const rvector& TargetNormal)
{
	ZEffect* pNew = NULL;

	rvector vv = Target;
	vv.z -= 50;
	pNew = new ZEffectSlash(m_pRocketEffect,vv,TargetNormal);
	((ZEffectSlash*)pNew)->SetAlignType(1);
	((ZEffectSlash*)pNew)->SetDistOption(29999.f,29999.f,29999.f);
	Add(pNew);

	rvector _add;
	float _min,_max;
	DWORD _life;

	for (int i = 0; i < 2; i++) {
		_add.x = rand()%30;
		_add.y = rand()%30;
		_add.z = rand()%30;

		_min = 100 + (rand()%100)/2.f;
		_max = 200 + (rand()%200)/3.f;

		_life = 3000.f + (1000*(rand()%6/3.f));

		AddGrenadeSmokeEffect(Target+_add,_min,_max,_life);
	}

}

bool ZEffectManager::RenderCheckEffectLevel()
{
	bool bRender = true;

	if(g_nEffectLevel==Z_VIDEO_EFFECT_HIGH) {	bRender = true;	}
	else if(g_nEffectLevel==Z_VIDEO_EFFECT_NORMAL) {
		static bool toggle = true;
		if(toggle)	bRender = true;
		else 		bRender = false;
		toggle = !toggle;
	}
	else if(g_nEffectLevel==Z_VIDEO_EFFECT_LOW) { bRender = false; }

	return bRender;
}

void ZEffectManager::AddMapSmokeSTEffect(const rvector& Target, const rvector& dir,
	const rvector& acc, const rvector& acc2,
	DWORD scolor, DWORD delay, float fLife, float fStartScale, float fEndScale)
{
	if( !RenderCheckEffectLevel() )	return;
	
	static DWORD _color[] = {
		0x858585,
		0x909090,
		0x959595,
		0xa0a0a0,
		0xa5a5a5,
		0xb0b0b0,
		0xb5b5b5,
		0xc5c5c5,
	};

	m_BillboardLists[4].SetVanishTime(2.9f);

	rvector _Acc = acc;

	DWORD tcolor = _color[rand()%8];

	if(scolor!=0x01010101) {

		D3DXCOLOR src_color(tcolor);
		D3DXCOLOR dest_color(scolor);

		float r = src_color.r * dest_color.r;
		float g = src_color.g * dest_color.g;
		float b = src_color.b * dest_color.b;

#define CLIP_COLOR(c) min(max(c,0.f),1.f)

		tcolor=D3DCOLOR_COLORVALUE( CLIP_COLOR(r),CLIP_COLOR(g),CLIP_COLOR(b),0.f);
	}

	ZEFFECTBILLBOARDITEM* pBItem =
		m_BillboardLists[4].Add(Target,dir,_Acc, fStartScale, fEndScale, fLife ,tcolor,false);

	if(pBItem) {
		pBItem->bUseSteamSmoke = true;
		pBItem->accel2 = acc2;
	}
}

void ZEffectManager::AddMapSmokeTSEffect(const rvector& Target,const rvector& dir,const rvector& acc,DWORD scolor,DWORD delay,float fLife,float fStartScale,float fEndScale)
{
	if( !RenderCheckEffectLevel() )	return;

	static DWORD _color[] = {
		0x656565,
		0x707070,
		0x757575,
		0x808080,
		0x858585,
		0x909090,
		0x959595,
		0xa5a5a5,
	};
	
	m_BillboardLists[4].SetVanishTime(2.9f);

	rvector _Acc = acc;

	DWORD tcolor = _color[rand()%8];

	if(scolor!=0x01010101) {

		D3DXCOLOR src_color(tcolor);
		D3DXCOLOR dest_color(scolor);

		float r = src_color.r * dest_color.r;
		float g = src_color.g * dest_color.g;
		float b = src_color.b * dest_color.b;

#define CLIP_COLOR(c) min(max(c,0.f),1.f)

		tcolor=D3DCOLOR_COLORVALUE( CLIP_COLOR(r),CLIP_COLOR(g),CLIP_COLOR(b),0.f);
	}

	m_BillboardLists[4].Add(Target,dir,_Acc, fStartScale, fEndScale, fLife ,tcolor,true);
}

void ZEffectManager::AddMapSmokeSSEffect(const rvector& Target, const rvector& dir,
	const rvector& acc, DWORD scolor, DWORD delay, float fLife, float fStartScale, float fEndScale)
{
	if( !RenderCheckEffectLevel() )	return;

	static DWORD _color[] = {
		0x858585,
		0x909090,
		0x959595,
		0xa0a0a0
	};

	m_BillboardLists[4].SetVanishTime(2.9f);

	rvector _Acc = acc;

	DWORD tcolor = _color[rand()%4];

	if(scolor!=0x01010101) {
		
		D3DXCOLOR src_color(tcolor);
		D3DXCOLOR dest_color(scolor);

		float r = src_color.r * dest_color.r;
		float g = src_color.g * dest_color.g;
		float b = src_color.b * dest_color.b;

#define CLIP_COLOR(c) min(max(c,0.f),1.f)

		tcolor=D3DCOLOR_COLORVALUE( CLIP_COLOR(r),CLIP_COLOR(g),CLIP_COLOR(b),0.f);
	}

	m_BillboardLists[4].Add(Target,dir,_Acc, fStartScale, fEndScale, fLife ,tcolor,false);
}

void ZEffectManager::AddRocketSmokeEffect(const rvector& Target)
{
	bool bRender = true;

	if(g_nEffectLevel==Z_VIDEO_EFFECT_HIGH) {
		bRender = true;
	}
	else if(g_nEffectLevel==Z_VIDEO_EFFECT_NORMAL) {
		static bool toggle = true;
		if(toggle) {
			bRender = true;
		}
		else {
			bRender = false;
		}
		toggle = !toggle;
	}
	else if(g_nEffectLevel==Z_VIDEO_EFFECT_LOW) {
		bRender = false;
	}

	m_BillboardLists[4].SetVanishTime(2.9f);
	if(bRender) {

		rvector add = 50.f*rvector(RANDOMFLOAT-0.5f,RANDOMFLOAT-0.5f,RANDOMFLOAT-0.5f);
		m_BillboardLists[4].Add(Target,add,rvector(0,0,0), 30.f, 120.f, 3.f );

	}
}

void ZEffectManager::AddGrenadeEffect(const rvector& Target, const rvector& TargetNormal)
{
	ZEffect* pNew = NULL;

	if(g_nEffectLevel != Z_VIDEO_EFFECT_LOW) {

	rvector up = rvector(0.f,0.f,1.f);

	float distance = 0.f;

	RBSPPICKINFO info;

	if( ZGetGame()->GetWorld()->GetBsp()->Pick( Target, -up, &info ) ) {
		auto vec = Target - info.PickPos;
		distance = MagnitudeSq(vec);
	}

	if(distance < 150.f) {

		rvector pos = rvector(0.f,0.f,0.f);
		rvector up = rvector(0.3f,0.3f,0.3f);
		rvector scale;

        for(int i=0;i<5;i++) {

			up = rvector(0.3f,0.3f,0.3f);

			float s = 0.5 + (rand()%10)/10.f;

			scale = rvector(s,s,s);

			up.x += (float)(rand()%10)/10.f;
			if((rand()%2)==0) up.x =-up.x; 

			up.y += (float)(rand()%10)/10.f;
			if((rand()%2)==0) up.y =-up.y; 

			up.z += (float)(rand()%10)/10.f;

			float speed = 2.8f + rand()%3;
			Normalize(up);
			
			pos.x = rand()%20;
			pos.y = rand()%20;
			pos.z = rand()%50;

			pNew = new ZEffectSlash( m_pGrenadeExpEffect,Target+pos,TargetNormal );

			((ZEffectSlash*)pNew)->SetUpVector(up);
			((ZEffectSlash*)pNew)->SetScale(scale);
			((ZEffectSlash*)pNew)->GetVMesh()->SetSpeed(speed);

			Add(pNew);
		}

	}

	if(distance < 10.f) {

		pNew = new ZEffectBulletMark( m_pEBSBulletMark[2], Target+up,up );
		((ZEffectBulletMark*)pNew)->m_Scale = rvector(120.f,120.f,120.f);
		Add(pNew);

	}

	rvector _add;
	float _min,_max,_life;

	for(int i=0;i<4;i++) {

		_add.x = rand()%60;
		_add.y = rand()%60;
		_add.z = rand()%60;

		_min = 100 + (rand()%100)/2.f;
		_max = 200 + (rand()%200)/3.f;

		_life = 3000.f + (1000*(rand()%6/3.f));

		AddGrenadeSmokeEffect(Target+_add,_min,_max,_life);

	}

	AddLightFragment(Target,TargetNormal);

	}

	rvector vv = Target;
	vv.z+=50;
	pNew = new ZEffectSlash( m_pGrenadeEffect,vv,TargetNormal );	
	((ZEffectSlash*)pNew)->SetDistOption(29999.f,29999.f,29999.f);
	((ZEffectSlash*)pNew)->SetAlignType(1);
	Add(pNew);
}


void ZEffectManager::AddBloodEffect(const rvector& Target, const rvector& TargetNormal)
{
}

void ZEffectManager::AddSlashEffect(const rvector& Target, const rvector& TargetNormal,int nType)
{
	ZEffect* pNew = NULL;

	bool _add = false;
	bool _add_uppercut = false;

	float rot_angle = 0.f;

	int mode = rand()%3;

	switch(nType) {

		case SEM_None:			_add = false;													break;	

		case SEM_ManSlash1:		_add = false;													break;
		case SEM_ManSlash2:		_add = true;	rot_angle = 0.f;								break;
		case SEM_ManSlash3:		_add = true;	rot_angle = 3.14f;								break;
		case SEM_ManSlash4:		_add = true;	rot_angle = 3.14f/2.f;							break;
		case SEM_ManSlash5:		_add = true;	rot_angle = -3.14f/2.f;							break;

		case SEM_ManDoubleSlash1:		_add = false;											break;
		case SEM_ManDoubleSlash2:		_add = true;	rot_angle = 0.f;						break;
		case SEM_ManDoubleSlash3:		_add = true;	rot_angle = 3.14f;						break;
		case SEM_ManDoubleSlash4:		_add = true;	rot_angle = 3.14f/2.f;					break;
		case SEM_ManDoubleSlash5:		_add = true;	rot_angle = -3.14f/2.f;					break;

		case SEM_ManGreatSwordSlash1:		_add = false;										break;
		case SEM_ManGreatSwordSlash2:		_add = true;	rot_angle = 0.f;					break;
		case SEM_ManGreatSwordSlash3:		_add = true;	rot_angle = 3.14f;					break;
		case SEM_ManGreatSwordSlash4:		_add = true;	rot_angle = 3.14f/2.f;				break;
		case SEM_ManGreatSwordSlash5:		_add = true;	rot_angle = -3.14f/2.f;				break;

		case SEM_ManUppercut:	_add_uppercut = true;	rot_angle = -3.14f/2.f;					break;

		case SEM_WomanSlash1:	_add = true;	rot_angle = 3.14f + 3.14f/2.f;		mode = 0;	break;
		case SEM_WomanSlash2:	_add = true;	rot_angle = 3.14f;								break;
		case SEM_WomanSlash3:	_add = false;													break;
		case SEM_WomanSlash4:	_add = false;													break;
		case SEM_WomanSlash5:	_add = false;													break;

		case SEM_WomanDoubleSlash1:	_add = true;	rot_angle = 3.14f + 3.14f/2.f;	mode = 0;	break;
		case SEM_WomanDoubleSlash2:	_add = true;	rot_angle = 3.14f;							break;
		case SEM_WomanDoubleSlash3:	_add = false;												break;
		case SEM_WomanDoubleSlash4:	_add = false;												break;
		case SEM_WomanDoubleSlash5:	_add = false;												break;

		case SEM_WomanGreatSwordSlash1:	_add = true;	rot_angle = 3.14f + 3.14f/2.f;mode = 0;	break;
		case SEM_WomanGreatSwordSlash2:	_add = true;	rot_angle = 3.14f;						break;
		case SEM_WomanGreatSwordSlash3:	_add = false;											break;
		case SEM_WomanGreatSwordSlash4:	_add = false;											break;
		case SEM_WomanGreatSwordSlash5:	_add = false;											break;

		case SEM_WomanUppercut:	_add_uppercut = true;	rot_angle = -3.14f/2.f;					break;
	}

	if(!_add_uppercut) {

		pNew = new ZEffectSlash(m_pSworddam[mode],Target,TargetNormal);	
		((ZEffectSlash*)pNew)->SetScale(rvector(1.0f,1.0f,1.0f));
		((ZEffectSlash*)pNew)->SetAlignType(1);
		Add(pNew);

	}

	if(_add) {
		pNew = new ZEffectSlash(m_pSwordglaze,Target,TargetNormal);	
		((ZEffectSlash*)pNew)->SetRotationAngle(rot_angle);
		Add(pNew);
	}

	if(_add_uppercut) {
		pNew = new ZEffectSlash(m_pSwordUppercutEffect,Target,TargetNormal);
		((ZEffectSlash*)pNew)->SetRotationAngle(rot_angle);
		((ZEffectSlash*)pNew)->SetScale(rvector(1.5f,1.5f,1.5f));
		((ZEffectSlash*)pNew)->SetAlignType(1);
		Add(pNew);
	}
}

void ZEffectManager::AddSwordUppercutDamageEffect(const rvector& Target,MUID uidTarget,DWORD time)
{
	ZEffect* pNew = NULL;

	rvector dir = -RealSpace2::RCameraDirection;
	dir.z = 0.f;
	pNew = new ZEffectDash(m_pSwordUppercutDamageEffect,Target,dir,uidTarget);
	((ZEffectDash*)pNew)->SetAlignType(1);

	if(time)
		((ZEffectDash*)pNew)->SetStartTime(time);

	Add(pNew);
}

void ZEffectManager::AddEatBoxEffect(const rvector& Target,ZObject* pObj)
{
	ZEffect* pNew = NULL;

	rvector pos = Target;

	rvector dir = -RealSpace2::RCameraDirection;
	dir.z = 0.f;
	pos.z -= 120.f;
	pNew = new ZEffectDash(m_pEatBoxEffect,pos,dir,pObj->GetUID());
	pNew->SetEffectType(ZET_HEAL);
	((ZEffectDash*)pNew)->SetAlignType(1);
	DeleteSameType((ZEffectAniMesh*)pNew);
	Add(pNew);
}

void ZEffectManager::AddHealEffect(const rvector& Target,ZObject* pObj)
{
	ZEffect* pNew = NULL;

	rvector pos = Target;

	rvector dir = -RealSpace2::RCameraDirection;
	dir.z = 0.f;
	pos.z -= 120.f;
	pNew = new ZEffectDash(m_pHealEffect,pos,dir,pObj->GetUID());
	pNew->SetEffectType(ZET_HEAL);
	((ZEffectDash*)pNew)->SetAlignType(1);
	DeleteSameType((ZEffectAniMesh*)pNew);
	Add(pNew);
}

void ZEffectManager::AddRepireEffect(const rvector& Target,ZObject* pObj)
{
	ZEffect* pNew = NULL;

	rvector pos = Target;

	rvector dir = -RealSpace2::RCameraDirection;
	dir.z = 0.f;
	pos.z -= 120.f;
	pNew = new ZEffectDash(m_pRepireEffect,pos,dir,pObj->GetUID());
	pNew->SetEffectType(ZET_HEAL);
	((ZEffectSlash*)pNew)->SetAlignType(1);
	DeleteSameType((ZEffectAniMesh*)pNew);
	Add(pNew);
}

void ZEffectManager::AddExpanseAmmoEffect(const rvector& Target,ZObject* pObj )
{
	ZEffect* pNew = NULL;

	rvector pos = Target;

	rvector dir = -RealSpace2::RCameraDirection;
	dir.z = 0.f;
	pos.z -= 120.f;
	pNew = new ZEffectDash( m_pExpanseAmmoEffect, pos, dir, pObj->GetUID() );
	((ZEffectSlash*)pNew)->SetAlignType(2);
	Add(pNew);
}

void ZEffectManager::AddSlashEffectWall(const rvector& Target, const rvector& TargetNormal,int nType)
{
	ZEffect* pNew = NULL;

	bool _add = false;
	bool _add_left = false;

	float rot_angle = 0.f;
	float rot_angle_left = 0.f;

	int mode = 3;

#ifdef INSTANT_SLASH_DECAL
	u32 startTime = 0;
#else
	u32 startTime = 250;
#endif

#define _CASE_DEF(_c ,_rot,_rot_left,tadd,taddleft ,_stime ) case _c: rot_angle = _rot;rot_angle_left = _rot_left; _add = tadd;_add_left = taddleft; startTime = _stime; break;

	switch(nType) {

		_CASE_DEF(SEM_None     ,0.f,0.f,false,false,250);

		_CASE_DEF(SEM_ManSlash1,-2.8f,0.f,true,false,250);
		_CASE_DEF(SEM_ManSlash2,-1.3f,0.f,true,false,250);
		_CASE_DEF(SEM_ManSlash3, 1.3f,0.f,true,false,250);
		_CASE_DEF(SEM_ManSlash4, 0.8f,0.f,true,false,250);
		_CASE_DEF(SEM_ManSlash5,-1.5f,0.f,true,false,250);

		_CASE_DEF(SEM_ManDoubleSlash1,-0.1f, 0.0f,true,false , 10);
		_CASE_DEF(SEM_ManDoubleSlash2, 2.5f, 0.0f,true,false ,250);
		_CASE_DEF(SEM_ManDoubleSlash3, 0.0f, 0.0f,false,false,250);
		_CASE_DEF(SEM_ManDoubleSlash4, 3.2f,-0.1f,true,true  ,250);
		_CASE_DEF(SEM_ManDoubleSlash5,-1.5f, 0.0f,true,false ,250);

		_CASE_DEF(SEM_ManGreatSwordSlash1, 0.7f,0.f,true,false,250);
		_CASE_DEF(SEM_ManGreatSwordSlash2, 3.5f,0.f,true,false,250);
		_CASE_DEF(SEM_ManGreatSwordSlash3,-0.3f,0.f,true,false,250);
		_CASE_DEF(SEM_ManGreatSwordSlash4, 3.3f,0.f,true,false,250);
		_CASE_DEF(SEM_ManGreatSwordSlash5,-1.5f,0.f,true,false,250);

		_CASE_DEF(SEM_ManUppercut,-1.5f,0.f,true,false,250);

		_CASE_DEF(SEM_WomanSlash1,-1.0f,0.f,true,false,250);
		_CASE_DEF(SEM_WomanSlash2, 2.3f,0.f,true,false,250);
		_CASE_DEF(SEM_WomanSlash3,-0.5f,0.f,true,false,250);
		_CASE_DEF(SEM_WomanSlash4, 2.5f,0.f,true,false,250);
		_CASE_DEF(SEM_WomanSlash5,-1.5f,0.f,true,false,250);

		_CASE_DEF(SEM_WomanDoubleSlash1,-0.1f, 0.0f,true,false , 10);
		_CASE_DEF(SEM_WomanDoubleSlash2, 2.5f, 0.0f,true,false ,250);
		_CASE_DEF(SEM_WomanDoubleSlash3, 0.0f, 0.0f,false,false,250);
		_CASE_DEF(SEM_WomanDoubleSlash4, 3.2f,-0.1f,true,true  ,250);
		_CASE_DEF(SEM_WomanDoubleSlash5,-1.5f, 0.0f,true,false ,250);

		_CASE_DEF(SEM_WomanGreatSwordSlash1, 0.7f,0.f,true,false,250);
		_CASE_DEF(SEM_WomanGreatSwordSlash2, 4.0f,0.f,true,false,250);
		_CASE_DEF(SEM_WomanGreatSwordSlash3,-0.3f,0.f,true,false,250);
		_CASE_DEF(SEM_WomanGreatSwordSlash4, 1.2f,0.f,true,false,250);
		_CASE_DEF(SEM_WomanGreatSwordSlash5,-1.5f,0.f,true,false,250);

		_CASE_DEF(SEM_WomanUppercut,-1.5f,0.f,true,false,250);
	}

	startTime = 0;

	if(_add) {
	
		pNew = new ZEffectSlash(m_pSworddam[mode],Target,TargetNormal);

		((ZEffectSlash*)pNew)->SetRotationAngle(rot_angle);
		((ZEffectSlash*)pNew)->SetStartTime(startTime);
		((ZEffectSlash*)pNew)->SetAlignType(1);

		Add(pNew);
	}

	if(_add_left) {

		pNew = new ZEffectSlash(m_pSworddam[mode],Target,TargetNormal);

		((ZEffectSlash*)pNew)->SetRotationAngle(rot_angle_left);
		((ZEffectSlash*)pNew)->SetStartTime(startTime);
		((ZEffectSlash*)pNew)->SetAlignType(1);

		Add(pNew);
	}

}

void ZEffectManager::OnInvalidate()
{
	ZEffectBase::OnInvalidate();
}

void ZEffectManager::OnRestore()
{
	ZEffectBase::OnRestore();
}

void ZEffectManager::AddShadowEffect(rmatrix& m,DWORD _color)
{
	m_ShadowList.Add(m,_color);
}

void ZEffectManager::AddSmokeEffect(const rvector& Target )
{
	rvector v = rvector( 0,0,0 );
	ZEffect* pNew	= new ZEffectSmoke( m_pEBSSmokes[0], Target, v, 10, 3000, 50000 );
	Add( pNew );
}

void ZEffectManager::AddSmokeEffect( ZEffectBillboardSource* pEffectBillboardSource, const rvector& Pos, const rvector& Velocity, const rvector &Accel, float fMinScale, float fMaxScale, float fLifeTime )
{
	m_BillboardLists[rand()%SMOKE_COUNT].Add(Pos,Velocity,Accel,fMinScale,fMaxScale,fLifeTime);
}

#define MAX_SG_VELOCITY	10
void ZEffectManager::AddSmokeGrenadeEffect( rvector& Target  )
{
	rvector v;
	
	srand( GetGlobalTimeMS() );
	v.x	= (rand() % MAX_SG_VELOCITY) * 2 - MAX_SG_VELOCITY;
	v.y	= (rand() % MAX_SG_VELOCITY) * 2 - MAX_SG_VELOCITY;
	v.z	= 0.f;
	Normalize(v);

	ZEffect* pNew	= new ZEffectSmokeGrenade( m_pEBSSmokes[0], Target, v, 10, 1000, 20000 );
	((ZEffectSmokeGrenade*)pNew)->SetDistOption(29999.f,29999.f,29999.f);
	Add( pNew );
}

void ZEffectManager::AddGrenadeSmokeEffect(const rvector& Target ,float min,float max,float time)
{
	rvector v;
	
	srand( GetGlobalTimeMS() );
	v.x	= (rand() % MAX_SG_VELOCITY) * 2 - MAX_SG_VELOCITY;
	v.y	= (rand() % MAX_SG_VELOCITY) * 2 - MAX_SG_VELOCITY;
	v.z	= 0.f;
	Normalize(v);

	ZEffect* pNew	= new ZEffectSmokeGrenade( m_pEBSSmokes[0], Target, v, min, max, time );
	Add( pNew );
}

void ZEffectManager::AddWorldItemEatenEffect(const rvector& pos )
{
	ZEffect* pNew = new ZEffectSlash(m_pWorldItemEaten, pos, RCameraDirection );
	((ZEffectSlash*)pNew)->SetAlignType(2);
	Add(pNew);
}

void ZEffectManager::AddCharacterIcon(ZObject* pObj,int nIcon)
{
	if(!pObj->GetInitialized()) return;
	if(!pObj->GetVisualMesh()->m_bIsRender) return;

	ZEffect* pNew = NULL;
	pNew = new ZEffectIcon(m_pCharacterIcons[nIcon],pObj);
	((ZEffectIcon*)pNew)->SetAlignType(2);
	Add(pNew);
}

class ZEffectIconLoop : public ZEffectIcon {
private:
public:
	ZEffectIconLoop(RMesh* pMesh, ZObject* pObj) 
		: ZEffectIcon(pMesh,pObj)
	{}

		virtual bool Draw(u64 nTime) override
		{
			ZObject* pObj = ZGetObjectManager()->GetObject(m_uid);

			if( pObj ) {
				if(!pObj->IsRendered())
					return true;
				if( pObj->m_pVMesh ) {
					m_Pos = pObj->m_pVMesh->GetBipTypePosition(m_type);
					m_DirOrg = -pObj->m_Direction;
					ZEffectAniMesh::Draw(nTime);
					return true;
				}
			}
			return false;
		}
};

class ZEffectIconLoopStar : public ZEffectIconLoop {
private:
public:
	ZEffectIconLoopStar(RMesh* pMesh, ZObject* pObj)
		: ZEffectIconLoop(pMesh,pObj)
	{}

		virtual bool Draw(u64 nTime) override
		{
			MMatchObjCache* pCache = ZGetGameClient()->FindObjCache(m_uid);
			if (pCache && pCache->GetUGrade() != MMUG_STAR)
				return false;

			return ZEffectIconLoop::Draw(nTime);
		}
};

void ZEffectManager::AddBerserkerIcon(ZObject* pObj)
{
	ZEffect* pNew;

	for (int i = 0; i < eq_parts_pos_info_end; i++)
	{
		if (
			(i == eq_parts_pos_info_RFoot) || 
			(i == eq_parts_pos_info_LFoot) ||
			(i == eq_parts_pos_info_RToe0) || 
			(i == eq_parts_pos_info_LToe0) ||
			(i == eq_parts_pos_info_RToe0Nub) || 
			(i == eq_parts_pos_info_LToe0Nub) ||
			(i == eq_parts_pos_info_LFingerNub) ||
			(i == eq_parts_pos_info_RFingerNub) ||
			(i == eq_parts_pos_info_etc) ||
			(i == eq_parts_pos_info_LClavicle) ||
			(i == eq_parts_pos_info_RClavicle) ||
			(i == eq_parts_pos_info_Effect)) continue;


		pNew = new ZEffectBerserkerIconLoop(m_pBerserkerEffect,pObj);
		((ZEffectBerserkerIconLoop*)pNew)->SetAlignType(2);
		((ZEffectBerserkerIconLoop*)pNew)->m_type = _RMeshPartsPosInfoType(i);
		Add(pNew);
	}
}

void ZEffectManager::AddCommanderIcon(ZObject* pObj,int nTeam)
{
	if(nTeam<0 || nTeam>=2) return;

	ZEffect* pNew = NULL;
	pNew = new ZEffectIconLoop(m_pCommandIcons[nTeam],pObj);
	((ZEffectIconLoop*)pNew)->SetAlignType(2);
	((ZEffectIconLoop*)pNew)->m_type = eq_parts_pos_info_Spine1;
	Add(pNew);

	pNew = new ZEffectIconLoop(m_pCommandIcons[nTeam],pObj);
	((ZEffectIconLoop*)pNew)->SetAlignType(2);
	((ZEffectIconLoop*)pNew)->m_type = eq_parts_pos_info_Root;
	Add(pNew);
}

void ZEffectManager::AddChatIcon(ZObject* pObj)
{
	class ZEffectChatIconLoop : public ZEffectIcon {
	public:
		ZEffectChatIconLoop(RMesh* pMesh, ZObject* pObj) 
			: ZEffectIcon(pMesh,pObj) {
			}

			virtual bool Draw(u64 nTime) override
			{
				ZObject* pObj = ZGetObjectManager()->GetObject(m_uid);

				ZCharacter* pChar = MDynamicCast(ZCharacter, pObj);

				if( pChar ) {
					if(!pChar->m_bChatEffect) return false;
					if(!pChar->m_bRendered)
						return true;
					if( pObj->m_pVMesh ) {
						m_Pos = pObj->GetVisualMesh()->GetHeadPosition()+rvector(0,0,60);

						RGetDevice()->SetRenderState(D3DRS_ZWRITEENABLE, TRUE);

						ZEffectAniMesh::Draw(nTime);

						RGetDevice()->SetRenderState(D3DRS_ZWRITEENABLE, FALSE);

						return true;
					}
				}
				return false;
			}
	};

	ZEffect* pNew = NULL;
	pNew = new ZEffectChatIconLoop(m_pChatIcon,pObj);
	((ZEffectChatIconLoop*)pNew)->SetAlignType(2);
	Add(pNew);
}

void ZEffectManager::AddLostConIcon(ZObject* pObj)
{
	class ZEffectLostConIconLoop : public ZEffectIcon {
	public:
		ZEffectLostConIconLoop(RMesh* pMesh, ZObject* pObj) 
			: ZEffectIcon(pMesh,pObj) {
			}

			virtual bool Draw(u64 nTime) override
			{
				ZObject* pObj = ZGetObjectManager()->GetObject(m_uid);

				ZCharacter* pChar = MDynamicCast(ZCharacter, pObj);

				if( pChar ) {
					if(pChar->IsDie()) return false;
					if(!pChar->m_bLostConEffect) return false;
					if(!pChar->m_bRendered)
						return true;
					if( pObj->m_pVMesh ) {
						m_Pos = pObj->GetVisualMesh()->GetHeadPosition()+rvector(0,0,60);
						ZEffectAniMesh::Draw(nTime);
						return true;
					}
				}
				return false;
			}
	};

	ZEffect* pNew = NULL;
	pNew = new ZEffectLostConIconLoop(m_pLostConIcon,pObj);
	((ZEffectLostConIconLoop*)pNew)->SetAlignType(2);
	Add(pNew);
}

void ZEffectManager::AddChargingEffect( ZObject *pObj )
{
	ZCharacter *pChar = static_cast<ZCharacter *>(pObj);

	rvector TargetNormal = rvector(1,0,0);

	ZEffectCharging* pNew;

	auto pair = GetRGMain().GetPlayerSwordColor(pObj->GetUID());

	if (pair.first)
	{
		hsv HSVColor;
		HSVColor.h = rand() % 360;
		HSVColor.s = 1.0;
		HSVColor.v = 1.0;

		auto GetRainbowColor = [HSVColor]() mutable
		{
			double Delta = GetRGMain().GetElapsedTime();

			HSVColor.h += 360.0 / 2.0 * Delta;
			HSVColor.h = fmod(HSVColor.h, 360.0);

			rgb RGBColor = hsv2rgb(HSVColor);

			uint32_t Color = 0xFF000000 | (int(RGBColor.r * 255) << 16) | (int(RGBColor.g * 255) << 8) | int(RGBColor.b * 255);

			return Color;
		};

		auto GetColor = [GetRainbowColor, Color = pair.second]() mutable
		{
			if (Color == 0x12345678)
				return GetRainbowColor();

			return Color;
		};

		pNew = MakeTexBlendEffect<ZEffectCharging>(GetColor, m_pChargingEffect, pObj->m_Position, TargetNormal, pObj);
	}
	else
		pNew = new ZEffectCharging(m_pChargingEffect,pObj->m_Position,TargetNormal,pObj);

	pNew->SetAlignType(1);
	Add(pNew);
}

void ZEffectManager::AddChargedEffect( ZObject *pObj )
{
	ZCharacter *pChar = static_cast<ZCharacter *>(pObj);

	rvector TargetNormal = rvector(1,0,0);

	ZEffectCharged* pNew;

	auto pair = GetRGMain().GetPlayerSwordColor(pObj->GetUID());

	if (pair.first)
	{
		hsv HSVColor;
		HSVColor.h = rand() % 360;
		HSVColor.s = 1.0;
		HSVColor.v = 1.0;

		auto GetRainbowColor = [HSVColor]() mutable
		{
			double Delta = GetRGMain().GetElapsedTime();

			HSVColor.h += 360.0 / 2.0 * Delta;
			HSVColor.h = fmod(HSVColor.h, 360.0);

			rgb RGBColor = hsv2rgb(HSVColor);

			uint32_t Color = 0xFF000000 | (int(RGBColor.r * 255) << 16) |
				(int(RGBColor.g * 255) << 8) | int(RGBColor.b * 255);

			return Color;
		};

		auto GetColor = [GetRainbowColor, Color = pair.second]() mutable
		{
			if (Color == 0x12345678)
				return GetRainbowColor();

			return Color;
		};

		pNew = MakeTexBlendEffect<ZEffectCharged>(GetColor, m_pChargedEffect, pObj->m_Position, TargetNormal, pObj);
	}
	else
		pNew = new ZEffectCharged(m_pChargedEffect, pObj->m_Position, TargetNormal, pObj);

	pNew->SetAlignType(1);
	Add(pNew);
}

void ZEffectManager::AddStarEffect( ZObject *pObj )
{
	ZEffect* pNew = NULL;
	pNew = new ZEffectIconLoopStar(m_pEffectMeshMgr->Get("event_ongame_jjang"),pObj);
	((ZEffectIconLoop*)pNew)->SetAlignType(1);
	((ZEffectIconLoop*)pNew)->m_type = eq_parts_pos_info_Spine2;
	Add(pNew);
}

void ZEffectManager::Add(const char* szName,const rvector& pos, const rvector& dir,const MUID& uidOwner,int nLifeTime)
{
	ZEffect* pNew = NULL;
	RMesh *pMesh = m_pEffectMeshMgr->Get((char*)szName);
	if(!pMesh) return;

	pNew = new ZEffectSlash(pMesh,(rvector&)pos,(rvector&)dir);
	((ZEffectSlash*)pNew)->SetAlignType(1);
	((ZEffectSlash*)pNew)->SetLifeTime(nLifeTime);
	
	Add(pNew);
}

void ZEffectManager::AddSp(const char* szName,int nCnt,const rvector& pos, const rvector& dir,const MUID& uidOwner)
{
	if(_stricmp(szName,"BlizzardEffect")==0) {
		AddBlizzardEffect((rvector&)pos , nCnt );
	}
	else if(_stricmp(szName,"MethorEffect")==0) {
		AddMethorEffect((rvector&)pos , nCnt );
	}
	else {

	}
}

void ZEffectManager::AddPartsPosType(const char* szName,const MUID& uidOwner,RMeshPartsPosInfoType type,int nLifeTime)
{
	ZEffect* pNew = NULL;
	RMesh *pMesh = m_pEffectMeshMgr->Get((char*)szName);
	if(!pMesh) return;

	ZObject* pObj = ZGetObjectManager()->GetObject(uidOwner);

	ZCharacterObject* pCObj = MDynamicCast(ZCharacterObject, pObj);

	rvector pos = pCObj->GetPosition();
	rvector dir = pCObj->GetDirection();

	pNew = new ZEffectPartsTypePos(pMesh,(rvector&)pos,(rvector&)dir,rvector(0.f,0.f,0.f),pObj);
	((ZEffectPartsTypePos*)pNew)->SetAlignType(1);
	((ZEffectPartsTypePos*)pNew)->m_type = type;
	((ZEffectPartsTypePos*)pNew)->SetLifeTime(nLifeTime);
	Add(pNew);
}