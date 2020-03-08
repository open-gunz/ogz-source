#include "stdafx.h"
#include "ZGame.h"
#include "ZScreenEffectManager.h"
#include "RealSpace2.h"
#include "MDebug.h"
#include "Mint.h"
#include "ZApplication.h"
#include "ZSoundEngine.h"
#include "ZMyInfo.h"
#include "ZQuest.h"
#include "ZRuleDuel.h"
#include "ZConfiguration.h"
#include "RGMain.h"

constexpr float ScreenFOVInRadians = ToRadian(70);

void DrawGuage(float x,float y,float fWidth,float fHeight,float fLeanDir,DWORD color);

ZScreenEffect::ZScreenEffect(RMesh *pMesh,rvector offset)
{
	m_nDrawMode = ZEDM_NONE ;
	m_fDist = 0.f;

	m_Offset=offset;

	m_VMesh.Create(pMesh);
	m_VMesh.SetAnimation("play");
	m_VMesh.SetCheckViewFrustum(false);

}

bool ZScreenEffect::Draw(u64 nTime)
{
	return DrawCustom(nTime, m_Offset);
}

void ZScreenEffect::Update()
{
	m_VMesh.Frame();
}

bool ZScreenEffect::IsDeleteTime()
{
	if(m_VMesh.isOncePlayDone())
		return true;
	return false;
}

bool ZScreenEffect::DrawCustom(u64 nTime, const rvector& vOffset, float fAngle)
{
	RGetDevice()->SetRenderState(D3DRS_ZENABLE, FALSE);
	RGetDevice()->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);

	rmatrix World;
	GetIdentityMatrix(World);

	if (fAngle != 0.0f)
	{
		World = RGetRotZRad(fAngle);
	}

	const rvector eye(0, 0, -650), at(0, 0, 0), up(0, 1, 0);
	auto View = ViewMatrix(eye, Normalized(at - eye), up);
	auto Offset = TranslationMatrix(vOffset);
	rmatrix OriginalProjection;
	float FOV = ScreenFOVInRadians;

	if (ZGetConfiguration()->GetInterfaceFix())
	{
		FOV = FixedFOV(FOV);
	}
	else
	{
		auto Ratio = 4.0f / 3 / (float(RGetScreenWidth()) / RGetScreenHeight());
		auto Scale = ScalingMatrix({1, Ratio, 1, 1});

		View = Scale * View;
	}
	
	bool ChangeProj = ZGetConfiguration()->GetInterfaceFix() || FOV != GetFOV();
	if (ChangeProj)
	{
		OriginalProjection = RGetTransform(D3DTS_PROJECTION);
		auto&& Proj = PerspectiveProjectionMatrixViewport(RGetScreenWidth(), RGetScreenHeight(),
			FOV, 5, 10000);
		RSetTransform(D3DTS_PROJECTION, Proj);
	}

	View = Offset * View;

	RSetTransform(D3DTS_VIEW, View);

	m_VMesh.SetWorldMatrix(World);
	m_VMesh.Render();
	
	if (ChangeProj)
	{
		RSetTransform(D3DTS_PROJECTION, OriginalProjection);
	}

	if(m_VMesh.isOncePlayDone()) {
		return false;
	}

	return true;
}

ZComboEffect::ZComboEffect(RMesh *pMesh,rvector offset)
:ZScreenEffect(pMesh,offset)
{
	bDelete=false;
	fDeleteTime=g_pGame->GetTime()+10.f;
}

void ZComboEffect::SetFrame(int nFrame)
{
	AniFrameInfo* pInfo = GetVMesh()->GetFrameInfo(ani_mode_lower);

	pInfo->m_nFrame =
		max(min(pInfo->m_nFrame,2400),nFrame);
	fDeleteTime=g_pGame->GetTime()+10.f;
}

bool ZComboEffect::Draw(u64 nTime)
{
	if (bDelete && g_pGame->GetTime() >= fDeleteTime)
		return false;

	ZScreenEffect::Draw(nTime);
	return true;
}

void ZComboEffect::DeleteAfter(float fTime)
{
	bDelete=true;
	fDeleteTime=g_pGame->GetTime()+fTime;
}

ZBossGaugeEffect::ZBossGaugeEffect(RMesh *pMesh,rvector offset)
					:ZScreenEffect(pMesh,offset), m_bShocked(false), m_fShockStartTime(0.0f),
					m_fShockPower(0.0f), m_fLastTime(0.0f), m_ShockOffset(0.0f, 0.0f, 0.0f), m_ShockVelocity(0.0f, 0.0f, 0.0f),
					m_nVisualValue(-1)
{}


void ZBossGaugeEffect::Shock(float fPower)
{
	m_bShocked = true;

	m_fShockStartTime=g_pGame->GetTime();
	m_fLastTime = g_pGame->GetTime();
	m_fShockPower = max((min(20.0f, 20.0f + fPower)), 70.0f);
}

bool ZBossGaugeEffect::Draw(u64 nTime)
{
	MUID uidBoss = ZGetQuest()->GetGameInfo()->GetBoss();
	if (uidBoss == MUID(0,0)) return true;

	const float fShockDuration=	0.5f;
	const rvector ShockOffset=rvector(0,0,0);
	const rvector ShockVelocity=rvector(0,0,0);

	rvector offset = rvector(0.0f, 0.0f, 0.0f);

	float fElapsed = g_pGame->GetTime() - m_fLastTime;

	if (m_bShocked)
	{
		float fA=RandomNumber(0.0f, 1.0f)*2*PI_FLOAT;
		float fB=RandomNumber(0.0f, 1.0f)*2* PI_FLOAT;
		rvector velocity=rvector(cos(fA)*cos(fB), sin(fA)*sin(fB), 0.0f);

		float fPower=(g_pGame->GetTime() - m_fShockStartTime) / fShockDuration;
		if(fPower>1.f)
		{
			m_bShocked=false;
		}
		else
		{
			fPower=1.f-fPower;
			fPower=pow(fPower,1.5f);
			m_ShockVelocity = (RandomNumber(0.0f, 1.0f) * m_fShockPower * velocity);
			m_ShockOffset += fElapsed * m_ShockVelocity;
			offset = fPower * m_ShockOffset;
		}
	}

	m_fLastTime = g_pGame->GetTime();
	offset.z = 0.0f;


	bool ret = ZScreenEffect::DrawCustom(0, offset);

	ZObject* pBoss = ZGetObjectManager()->GetObject(uidBoss);
	if ((pBoss) && (pBoss->IsNPC()))
	{
		ZActor* pBossActor = (ZActor*)pBoss;
		int nMax = pBossActor->GetActualMaxHP() + pBossActor->GetActualMaxAP();
		int nCurr = min(pBossActor->GetHP() + pBossActor->GetAP(), nMax);

		if ((m_nVisualValue < 0) || (m_nVisualValue > nCurr) || (nCurr - m_nVisualValue > 100))
		{
			m_nVisualValue = nCurr;
		}

		if (m_nVisualValue > 0)
		{
			const int width =433;
			const int height = 12;

			int x = (800 - width) * 0.5f;
			int y = 600 * 0.028f;

			float fGaugeWidth = width * (m_nVisualValue / (float)nMax);

			DWORD color = D3DCOLOR_ARGB(255, 0xBB, 0, 0);

			RGetDevice()->SetFVF( D3DFVF_XYZRHW | D3DFVF_TEX1 | D3DFVF_DIFFUSE );
			RGetDevice()->SetTexture(0,NULL);

			float fx = 183.0f/800.0f + offset.x/980.0f;
			float fy = 574.0f/600.0f - offset.y/720.0f;

			DrawGuage(fx, fy, fGaugeWidth/800.0f, 7.0f/600.0f, 0.0f, color);
		}
	}


	return ret;
}

ZKOEffect::ZKOEffect(RMesh* pMesh, rvector offset) : ZScreenEffect(pMesh, offset) {}

void ZKOEffect::InitFrame()
{
	m_VMesh.Stop(ani_mode_lower);
	m_VMesh.m_pMesh->SetFrame(0 , 0);
	m_VMesh.Play(ani_mode_lower);
}

void ZKOEffect::SetFrame(int nFrame)
{
	m_VMesh.m_pMesh->SetFrame(nFrame , 0);
}

int ZKOEffect::GetFrame()
{
	AniFrameInfo* pInfo = GetVMesh()->GetFrameInfo(ani_mode_lower);
	return pInfo->m_nFrame;
}

ZTDMBlinkEffect::ZTDMBlinkEffect(RMesh* pMesh, rvector offset) : ZScreenEffect(pMesh, offset) {}

void ZTDMBlinkEffect::SetAnimationSpeed(int nKillsDiff)
{
	float speed = 4.8f;
	if (nKillsDiff > 5)
		speed = 9.6f;

	m_VMesh.SetSpeed(speed);
}

ZScreenEffectManager::ZScreenEffectManager()
{

	m_WeaponType = MWT_NONE;
	m_SelectItemDesc = NULL;

	m_pGuageTexture = NULL;
	m_pHPPanel=NULL;
	m_pScorePanel=NULL;
	m_pReload = NULL;
	m_pEmpty = NULL;

	for(int i=0;i<MWT_END;i++)
		m_pWeaponIcons[i]=NULL;

	m_pEffectMeshMgr=NULL;

	m_pSpectator = NULL;

	m_bGameStart = false;
	m_nHpReset = 0;

	m_bShowReload = false;
	m_bShowEmpty = false;

	m_pQuestEffectMeshMgr = NULL;
	m_pBossHPPanel = NULL;
	m_pArrow = NULL;
	for (int i = 0; i < 10; i++) m_pKONumberEffect[i] = NULL;
	m_pKO = NULL;
	m_nKO = 0;
}

ZScreenEffectManager::~ZScreenEffectManager()
{
	DestroyQuestRes();
	Destroy();
}

void ZScreenEffectManager::Destroy()
{
	Clear();

	if(m_pGuageTexture)
	{
		RDestroyBaseTexture(m_pGuageTexture);
		m_pGuageTexture=NULL;
	}

	SAFE_DELETE(m_pSpectator);
	SAFE_DELETE(m_pHPPanel);
	SAFE_DELETE(m_pScorePanel);
	SAFE_DELETE(m_pReload);
	SAFE_DELETE(m_pEmpty);

	SAFE_DELETE(m_pWeaponIcons[MWT_DAGGER]);
	SAFE_DELETE(m_pWeaponIcons[MWT_DUAL_DAGGER]);
	SAFE_DELETE(m_pWeaponIcons[MWT_KATANA]);
	SAFE_DELETE(m_pWeaponIcons[MWT_GREAT_SWORD]);
	SAFE_DELETE(m_pWeaponIcons[MWT_DOUBLE_KATANA]);

	SAFE_DELETE(m_pWeaponIcons[MWT_PISTOL]);
	SAFE_DELETE(m_pWeaponIcons[MWT_PISTOLx2]);
	SAFE_DELETE(m_pWeaponIcons[MWT_REVOLVER]);
	SAFE_DELETE(m_pWeaponIcons[MWT_REVOLVERx2]);
	SAFE_DELETE(m_pWeaponIcons[MWT_SMG]);

	SAFE_DELETE(m_pWeaponIcons[MWT_SMGx2]);
	SAFE_DELETE(m_pWeaponIcons[MWT_SHOTGUN]);
	SAFE_DELETE(m_pWeaponIcons[MWT_SAWED_SHOTGUN]);

	SAFE_DELETE(m_pWeaponIcons[MWT_RIFLE]);
	SAFE_DELETE(m_pWeaponIcons[MWT_MACHINEGUN]);
	SAFE_DELETE(m_pWeaponIcons[MWT_ROCKET]);
	SAFE_DELETE(m_pWeaponIcons[MWT_SNIFER]);

	SAFE_DELETE(m_pWeaponIcons[MWT_MED_KIT]);
	SAFE_DELETE(m_pWeaponIcons[MWT_REPAIR_KIT]);
	SAFE_DELETE(m_pWeaponIcons[MWT_BULLET_KIT]);
	SAFE_DELETE(m_pWeaponIcons[MWT_FLASH_BANG]);
	SAFE_DELETE(m_pWeaponIcons[MWT_FRAGMENTATION]);
	SAFE_DELETE(m_pWeaponIcons[MWT_SMOKE_GRENADE]);
	SAFE_DELETE(m_pWeaponIcons[MWT_FOOD]);
	
	SAFE_DELETE(m_pKO);

	for (int i = 0; i < 10; i++) 
	{
		SAFE_DELETE(m_pKONumberEffect[i]);
	}

 	SAFE_DELETE(m_pTDScoreBoard);
	SAFE_DELETE(m_pTDScoreBlink_R);
	SAFE_DELETE(m_pTDScoreBlink_B);


	SAFE_DELETE(m_pEffectMeshMgr);	
	SAFE_DELETE(m_pQuestEffectMeshMgr);
}

void ZScreenEffectManager::Clear()
{
	m_eraseQueue.clear();
	while(!empty())
	{
		delete *begin();
		pop_front();
	}

	for(int i=0;i<COMBOEFFECTS_COUNT;i++)
		m_pComboEffects[i]=NULL;
}


bool ZScreenEffectManager::Create()
{
	u64 _begin_time, _end_time;
#define BEGIN_ { _begin_time = GetGlobalTimeMS(); }
#define END_(x) { _end_time = GetGlobalTimeMS(); float f_time = (_end_time - _begin_time) / 1000.f; mlog("%s : %f \n", x,f_time ); }

	BEGIN_;

	m_pEffectMeshMgr = new RMeshMgr;
	if(m_pEffectMeshMgr->LoadXmlList("interface/default/combat/screeneffects.xml")==-1) {
		mlog("combat list loding error\n");
		SAFE_DELETE(m_pEffectMeshMgr);
		return false;
	}

	m_pHPPanel=new ZScreenEffect(m_pEffectMeshMgr->Get("hppanel"));
	m_pScorePanel=new ZScreenEffect(m_pEffectMeshMgr->Get("ef_in_tab.elu"));
	m_pSpectator=new ZScreenEffect(m_pEffectMeshMgr->Get("spectator"));

	m_pEmpty = new ZScreenEffect(m_pEffectMeshMgr->Get("empty"));
	m_pReload = new ZScreenEffect(m_pEffectMeshMgr->Get("reload"));

	m_pWeaponIcons[MWT_DAGGER]			= new ZScreenEffect(m_pEffectMeshMgr->Get("ef_in_icon_dagger.elu"));
	m_pWeaponIcons[MWT_DUAL_DAGGER]		= new ZScreenEffect(m_pEffectMeshMgr->Get("ef_in_icon_d_dagger.elu"));
	m_pWeaponIcons[MWT_KATANA]			= new ZScreenEffect(m_pEffectMeshMgr->Get("ef_in_icon_katana.elu"));
	m_pWeaponIcons[MWT_GREAT_SWORD]		= new ZScreenEffect(m_pEffectMeshMgr->Get("ef_in_icon_sword.elu"));
	m_pWeaponIcons[MWT_DOUBLE_KATANA]	= new ZScreenEffect(m_pEffectMeshMgr->Get("ef_in_icon_blade.elu"));

	m_pWeaponIcons[MWT_PISTOL]			= new ZScreenEffect(m_pEffectMeshMgr->Get("ef_in_icon_pistol.elu"));
	m_pWeaponIcons[MWT_PISTOLx2]		= new ZScreenEffect(m_pEffectMeshMgr->Get("ef_in_icon_d_pistol.elu"));
	m_pWeaponIcons[MWT_REVOLVER]		= new ZScreenEffect(m_pEffectMeshMgr->Get("ef_in_icon_pistol.elu"));
	m_pWeaponIcons[MWT_REVOLVERx2]		= new ZScreenEffect(m_pEffectMeshMgr->Get("ef_in_icon_d_pistol.elu"));
	m_pWeaponIcons[MWT_SMG]				= new ZScreenEffect(m_pEffectMeshMgr->Get("ef_in_icon_smg.elu"));

	m_pWeaponIcons[MWT_SMGx2]			= new ZScreenEffect(m_pEffectMeshMgr->Get("ef_in_icon_d_smg.elu"));
	m_pWeaponIcons[MWT_SHOTGUN]			= new ZScreenEffect(m_pEffectMeshMgr->Get("ef_in_icon_shotgun.elu"));
	m_pWeaponIcons[MWT_SAWED_SHOTGUN]	= new ZScreenEffect(m_pEffectMeshMgr->Get("ef_in_icon_shotgun.elu"));

	m_pWeaponIcons[MWT_RIFLE]			= new ZScreenEffect(m_pEffectMeshMgr->Get("ef_in_icon_rifle.elu"));
	m_pWeaponIcons[MWT_MACHINEGUN]		= new ZScreenEffect(m_pEffectMeshMgr->Get("ef_in_icon_machinegun.elu"));
	m_pWeaponIcons[MWT_ROCKET]			= new ZScreenEffect(m_pEffectMeshMgr->Get("ef_in_icon_rocket.elu"));
	m_pWeaponIcons[MWT_SNIFER]			= new ZScreenEffect(m_pEffectMeshMgr->Get("ef_in_icon_rifle.elu"));

	m_pWeaponIcons[MWT_MED_KIT]			= new ZScreenEffect(m_pEffectMeshMgr->Get("ef_in_icon_medikit.elu"));
	m_pWeaponIcons[MWT_REPAIR_KIT]		= new ZScreenEffect(m_pEffectMeshMgr->Get("ef_in_icon_repairkit.elu"));
	m_pWeaponIcons[MWT_FLASH_BANG]		= new ZScreenEffect(m_pEffectMeshMgr->Get("ef_in_icon_flashbang.elu"));
	m_pWeaponIcons[MWT_FRAGMENTATION]	= new ZScreenEffect(m_pEffectMeshMgr->Get("ef_in_icon_grenade.elu"));
	m_pWeaponIcons[MWT_SMOKE_GRENADE]	= new ZScreenEffect(m_pEffectMeshMgr->Get("ef_in_icon_flashbang.elu"));
	m_pWeaponIcons[MWT_FOOD]			= new ZScreenEffect(m_pEffectMeshMgr->Get("ef_in_icon_food.elu"));
	m_pWeaponIcons[MWT_BULLET_KIT]		= new ZScreenEffect(m_pEffectMeshMgr->Get("ef_in_icon_magazine.elu"));

	m_pHit = m_pEffectMeshMgr->Get("hit");
	m_pComboBeginEffect = m_pEffectMeshMgr->Get("combo_begin");
	m_pComboEndEffect = m_pEffectMeshMgr->Get("combo_end");
	for(int i=0;i<10;i++)
	{
		char meshname[256];
		sprintf_safe(meshname,"combo%d",i);
		m_pComboNumberEffect[i] = m_pEffectMeshMgr->Get(meshname);

		sprintf_safe(meshname,"exp%d",i);
		m_pExpNumberEffect[i] = m_pEffectMeshMgr->Get(meshname);
	}

	m_pExpPlusEffect = m_pEffectMeshMgr->Get("exp+");
	m_pExpMinusEffect = m_pEffectMeshMgr->Get("exp-");

	for(int i=0;i<COMBOEFFECTS_COUNT;i++)
		m_pComboEffects[i]=NULL;

	m_pPraiseEffect[0] = m_pEffectMeshMgr->Get("allkill");
	m_pPraiseEffect[1] = m_pEffectMeshMgr->Get("unbelievable");
	m_pPraiseEffect[2] = m_pEffectMeshMgr->Get("excellent");
	m_pPraiseEffect[3] = m_pEffectMeshMgr->Get("fantastic");
	m_pPraiseEffect[4] = m_pEffectMeshMgr->Get("headshot");

	m_pGoodEffect = m_pEffectMeshMgr->Get("good");
	m_pNiceEffect = m_pEffectMeshMgr->Get("nice");
	m_pGreatEffect = m_pEffectMeshMgr->Get("great");
	m_pWonderfullEffect = m_pEffectMeshMgr->Get("wonderful");

	m_pCoolEffect = m_pEffectMeshMgr->Get("cool");

	m_pAlertEffect[0] = m_pEffectMeshMgr->Get("alert_front");
	m_pAlertEffect[1] = m_pEffectMeshMgr->Get("alert_right");
	m_pAlertEffect[2] = m_pEffectMeshMgr->Get("alert_back");
	m_pAlertEffect[3] = m_pEffectMeshMgr->Get("alert_left");

	m_CurrentComboLevel=ZCL_NONE;

	m_pGuageTexture=RCreateBaseTexture("Interface/Default/COMBAT/gauge.bmp");

	m_fGuageHP=m_fGuageAP=m_fGuageEXP=0.f;
	m_fCurGuageHP=m_fCurGuageAP=-1.f;

	m_pTDScoreBoard = new ZScreenEffect(m_pEffectMeshMgr->Get("td_scoreboard"));
	m_pTDScoreBlink_B = new ZTDMBlinkEffect(m_pEffectMeshMgr->Get("td_scoreblink_b"));
	m_pTDScoreBlink_R = new ZTDMBlinkEffect(m_pEffectMeshMgr->Get("td_scoreblink_r"));

	END_("Screen Effect Manager Create");
	return true;

#undef BEGIN_
#undef END_
}

void ZScreenEffectManager::Add(ZEffect *pEffect)
{	
	push_back(pEffect); 
}

void DrawGuage(float x,float y,float fWidth,float fHeight,float fLeanDir,DWORD color)
{
	struct TLVERTEX {
		float x, y, z, w;
		DWORD color;
		float u,v;
	} ;

	TLVERTEX ver[4];

#define SETVERTEX(_a,_x,_y,_z,_u,_v,_color) { ver[_a].x=_x;ver[_a].y=_y;ver[_a].z=_z;ver[_a].u=_u;ver[_a].v=_v;ver[_a].color=_color;  ver[_a].w=.1f; }

	float fLean=fHeight*(float)MGetWorkspaceHeight()*fLeanDir;
	
	int corrected_workspace_w = MGetCorrectedWorkspaceWidth();
	int start = (MGetWorkspaceWidth() - corrected_workspace_w) / 2;

	int x1,y1,x2,y2;
	x1 = x * corrected_workspace_w + start;
	y1 = y*(float)MGetWorkspaceHeight();
	x2 = (x + fWidth) * corrected_workspace_w + start;
	y2 = (y + fHeight)*(float)MGetWorkspaceHeight();

	SETVERTEX(0,x1		,y1,0,	0,0,color);
	SETVERTEX(1,x2		,y1,0,	1,0,color);
	SETVERTEX(2,x1+fLean,y2,0,	0,1,color);
	SETVERTEX(3,x2+fLean,y2,0,	1,1,color);

	HRESULT hr=RGetDevice()->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP,2,ver,sizeof(TLVERTEX));

#undef SETVERTEX
}

void ZScreenEffectManager::ReSetHpPanel()
{
	if(m_pHPPanel->GetVMesh()) {

		AniFrameInfo* pAni = m_pHPPanel->GetVMesh()->GetFrameInfo( ani_mode_lower );

		pAni->m_nFrame = 0;

		if( pAni->m_isPlayDone ) {
			pAni->m_isPlayDone = false;
		}

		pAni->m_pAniSet = NULL;

		m_pHPPanel->GetVMesh()->SetAnimation("play");
	}

	m_nHpReset = 1;
}

void ZScreenEffectManager::SetGuage_HP(float fHP) 
{
	if(m_fCurGuageHP==-1)
		m_fCurGuageHP = fHP;

	m_fGuageHP=fHP; 

	if( m_fCurGuageHP < m_fGuageHP) {
		m_fCurGuageHP = fHP;
	}
}

void ZScreenEffectManager::SetGuage_AP(float fAP)	
{
	if(m_fCurGuageAP==-1)
		m_fCurGuageAP = fAP;

	m_fGuageAP=fAP; 

	if( m_fCurGuageAP < m_fGuageAP) {
		m_fCurGuageAP = fAP;
	}
}

int ZScreenEffectManager::DrawResetGuages()
{
	RGetDevice()->SetFVF( D3DFVF_XYZRHW | D3DFVF_TEX1 | D3DFVF_DIFFUSE );

	DWORD color = 0xffffffff;

	static float _hp = 0.f;
	static float _ap = 0.f;
	static DWORD _backtime = GetGlobalTimeMS();

	DWORD newtime = GetGlobalTimeMS();

	if(_backtime==0)
		_backtime = newtime;

	DWORD delta = newtime - _backtime;

	_hp += 0.002f * delta;
	_ap += 0.002f * delta;

	_backtime = newtime;

	if(m_pGuageTexture)
		RGetDevice()->SetTexture(0,m_pGuageTexture->GetTexture());
	else
		RGetDevice()->SetTexture(0,NULL);

	color = D3DCOLOR_ARGB(255,  0,128,255);

	DrawGuage(70.f/800.f , 23.f/600.f , min(1.f,_hp) * 138.f/800.f , 13.f/600.f , 1.f ,color);

	// ap
	color = D3DCOLOR_ARGB(255, 68,193, 62);

	DrawGuage(84.f/800.f , 50.f/600.f , min(1.f,_ap) * 138.f/800.f , 13.f/600.f , -1.f ,color);

	// exp
	color = D3DCOLOR_ARGB(255,200,200,200);

	DrawGuage(66.f/800.f , 70.f/600.f , min(1.f,m_fGuageEXP) * 138.f/800.f , 4.f/600.f , -1.f ,color);

	if(_hp > 1.0f)
	{
		_hp = 0.f;
		_ap = 0.f;
		_backtime = 0;

		return 0;
	}

	return 1;
}

void ZScreenEffectManager::DrawGuages()
{
	RGetDevice()->SetFVF( D3DFVF_XYZRHW | D3DFVF_TEX1 | D3DFVF_DIFFUSE );

	DWORD color = 0xffffffff;

	bool render_cur_hp = false;
	bool render_cur_ap = false;

	if(m_fCurGuageHP > 1.0f)
		m_fCurGuageHP = 1.0f;

	if(m_fCurGuageAP > 1.0f)
		m_fCurGuageAP = 1.0f;

	if(m_fCurGuageHP > m_fGuageHP ) {
		render_cur_hp = true;
		m_fCurGuageHP -= 0.01f;
	}
	else {
		m_fCurGuageHP = m_fGuageHP;
	}

	if(m_fCurGuageAP > m_fGuageAP ) {
		render_cur_ap = true;
		m_fCurGuageAP -= 0.01f;
	}
	else {
		m_fCurGuageAP = m_fGuageAP;
	}

	if(m_pGuageTexture)
		RGetDevice()->SetTexture(0,m_pGuageTexture->GetTexture());
	else
		RGetDevice()->SetTexture(0,NULL);

	// hp
	if(m_fGuageHP == 1.0f )		color = D3DCOLOR_ARGB(255,  0,128,255);
	else if(m_fGuageHP > 0.7f)	color = D3DCOLOR_ARGB(255, 69,177,186);
	else if(m_fGuageHP > 0.3f)	color = D3DCOLOR_ARGB(255,231,220, 24);
	else						color = D3DCOLOR_ARGB(255,233, 44, 22);

	DrawGuage(70.f/800.f , 23.f/600.f , min(1.f,m_fGuageHP) * 138.f/800.f , 13.f/600.f , 1.f ,color);

	// ap
	color = D3DCOLOR_ARGB(255, 68,193, 62);

	DrawGuage(84.f/800.f , 50.f/600.f , min(1.f,m_fGuageAP) * 138.f/800.f , 13.f/600.f , -1.f ,color);

	// exp
	color = D3DCOLOR_ARGB(255,200,200,200);

	DrawGuage(66.f/800.f , 70.f/600.f , min(1.f,m_fGuageEXP) * 138.f/800.f , 4.f/600.f , -1.f ,color);

	// alpha
	RGetDevice()->SetRenderState( D3DRS_ALPHABLENDENABLE, TRUE);
	RGetDevice()->SetRenderState( D3DRS_SRCBLEND,   D3DBLEND_SRCALPHA );
	RGetDevice()->SetRenderState( D3DRS_DESTBLEND,  D3DBLEND_INVSRCALPHA );

	if(render_cur_hp) {

		color = 0x60ef0000;

		float x = min(1.f,m_fGuageHP) * 138.f/800.f;
		float w = (m_fCurGuageHP-m_fGuageHP) * 138.f/800.f;

		DrawGuage( 70.f/800.f+x, 23.f/600.f , w , 13.f/600.f , 1.f ,color);
	}	

	if(render_cur_ap) {

		color = 0x60ef0000;

		float x = min(1.f,m_fGuageAP) * 138.f/800.f;	
		float w = (m_fCurGuageAP-m_fGuageAP) * 138.f/800.f;

		DrawGuage(84.f/800.f+x , 50.f/600.f , w , 13.f/600.f , -1.f ,color);
	}

	RGetDevice()->SetRenderState( D3DRS_ALPHABLENDENABLE, FALSE);

}

void ZScreenEffectManager::Draw()
{
	ZCharacter *pTargetCharacter = ZGetGameInterface()->GetCombatInterface()->GetTargetCharacter();
	if(!pTargetCharacter || !pTargetCharacter->GetInitialized()) return;

	RSetProjection(ScreenFOVInRadians, 5, 10000);

	if(!ZGetGameInterface()->GetCombatInterface()->GetObserverMode())
	{
		RGetDevice()->SetRenderState(D3DRS_ALPHATESTENABLE,	FALSE);
		RGetDevice()->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);

		if(pTargetCharacter) {

			ZItem* pSelectedItem = pTargetCharacter->GetItems()->GetSelectedWeapon();

			if(pSelectedItem){
				if( pSelectedItem->GetItemType() != MMIT_MELEE ) {
					if (pSelectedItem->GetBulletAMagazine() <= 0) {
						if(pSelectedItem->isReloadable()==false) {
							m_bShowReload = false;
							m_bShowEmpty = true;
						}
						else {
							m_bShowReload = true;
							m_bShowEmpty = false;

						}
					}
					else {
						m_bShowReload = false;
						m_bShowEmpty = false;
					}
				}
				else {
					m_bShowReload = false;
					m_bShowEmpty = false;
				}
			}
		}

		if (ZApplication::GetGame()->GetMatch()->GetMatchType() != MMATCH_GAMETYPE_DUEL ||
			!pTargetCharacter->IsObserverTarget())
		{
			m_pHPPanel->Update();
			m_pHPPanel->Draw(0);

			bool bDrawGuages = false;

			if(m_pHPPanel->GetVMesh())
				if (m_pHPPanel->GetVMesh()->GetFrameInfo(ani_mode_lower)->m_isPlayDone)
					bDrawGuages = true;

			if(bDrawGuages) {

				if(m_nHpReset) {
					m_nHpReset = DrawResetGuages();
				}
				else {
					DrawGuages();
				}
			}

		}
		
		if(m_WeaponType != MWT_NONE) {
			if( m_pWeaponIcons[m_WeaponType] )
			{
				m_pWeaponIcons[m_WeaponType]->Update();
				m_pWeaponIcons[m_WeaponType]->Draw(0);
			}
		}

		if( m_bShowReload ) {
			if(m_pReload)
			{
				m_pReload->Update();
				m_pReload->Draw(0);
			}
		}
		else if(m_bShowEmpty) {
			if(m_pEmpty)
			{
				m_pEmpty->Update();
				m_pEmpty->Draw(0);
			}
		}
	}

	LPDIRECT3DDEVICE9 pd3dDevice=RGetDevice();
	pd3dDevice->SetSamplerState( 0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR );
	pd3dDevice->SetSamplerState( 0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR );

	if ( !g_pGame->IsReplay() || g_pGame->IsShowReplayInfo())
	{
		DrawEffects();

		DrawCombo();
	}

	DrawQuestEffects();
	DrawDuelEffects();
	DrawTDMEffects();
}

void ZScreenEffectManager::DrawSpectator()
{
	if(!ZGetMyInfo()->IsAdminGrade())
	{
		m_pSpectator->Update();
		m_pSpectator->Draw(GetGlobalTimeMS());
	}

	if(m_WeaponType != MWT_NONE) 
	{
		if( m_pWeaponIcons[m_WeaponType] )
			m_pWeaponIcons[m_WeaponType]->DrawCustom(0, rvector(0.0f, 80.0f, 0.0f));
	}

	DrawEffects();
}

void ZScreenEffectManager::ResetSpectator()
{
	m_pSpectator->GetVMesh()->ClearFrame();
}

void ZScreenEffectManager::DrawEffects()
{
	ZEffect* pEffect = NULL;

	for( iterator i=begin(); i!=end();i++)
	{
		pEffect = *i;
		pEffect->Draw(0);
	}
}

void ZScreenEffectManager::UpdateEffects()
{
	for( list<ZEffectList::iterator>::iterator i = m_eraseQueue.begin(); i!=m_eraseQueue.end();i++)
	{
		ZEffectList::iterator ieffect = *i;
		delete *ieffect;
		erase(ieffect);
	}
	m_eraseQueue.clear();

	ZEffect* pEffect = NULL;

	for( iterator i=begin(); i!=end(); ++i)
	{
		pEffect = *i;

		pEffect->Update();
		if(pEffect->IsDeleteTime())
			m_eraseQueue.push_back(i);
	}

	if (!ZGetGameTypeManager()->IsQuestDerived(ZGetGameClient()->GetMatchStageSetting()->GetGameType())) return;

	if (m_pBossHPPanel)
	{
		m_pBossHPPanel->Update();
	}

	if (ZGetGameInterface()->GetCombatInterface())
	{
		if (!ZGetGameInterface()->GetCombatInterface()->GetObserverMode())
		{
			for (auto* p : m_pKONumberEffect)
				if (p)
					p->Update();

			if (m_pKO)
				m_pKO->Update();
		}
	}
}


void ZScreenEffectManager::AddRoundStart(int nRound)
{
#define ROUND_NUMBER_SPACE	60.f

	if(nRound<0) return;

	ZEffect* pNew = NULL;

	char buffer[32];
	sprintf_safe(buffer,"%d",nRound);
	int nCount=(int)strlen(buffer);

	int nOver=max(nCount-2,0);

	for(int i=0;i<nCount;i++)
	{
		char meshname[256];
		sprintf_safe(meshname,"round%d",buffer[i]-'0');
		RMesh *pMesh = m_pEffectMeshMgr->Get(meshname);
		if(pMesh)
			Add(new ZScreenEffect(pMesh , rvector(ROUND_NUMBER_SPACE*(float)(i-nCount+1+nOver),0,0)));
	}

	RMesh *pMesh = m_pEffectMeshMgr->Get("round");
	if(pMesh)
		Add(new ZScreenEffect(pMesh));

	ZGetGameInterface()->PlayVoiceSound( VOICE_GET_READY, 500);
}

void ZScreenEffectManager::DrawCombo()
{
	int nFrame,nLastDigit=0;
	for(int i=0;i<COMBOEFFECTS_COUNT;i++)
	{
		if(m_pComboEffects[i]) {

			nLastDigit=i;

			nFrame = m_pComboEffects[i]->GetVMesh()->GetFrameInfo(ani_mode_lower)->m_nFrame;

			if(m_pComboEffects[i]->GetVMesh()->isOncePlayDone()) {

				m_pComboEffects[i]->DeleteAfter();
				m_pComboEffects[i]=NULL;
				if(i==0)
				{
					if(m_pComboEffects[1])
						m_pComboEffects[1]->DeleteAfter();
					m_pComboEffects[1]=new ZComboEffect(m_pComboEndEffect);
					Add(m_pComboEffects[1]);
				}
			}
		}
	}

	for(int i=2;i<nLastDigit;i++) {
		if(m_pComboEffects[i]) {
				m_pComboEffects[i]->SetFrame(nFrame);
		}
	}
}

void ZScreenEffectManager::SetCombo(int nCombo)
{
	static int combonumbers[COMBOEFFECTS_COUNT]={0,};

	if (nCombo > MAX_COMBO) nCombo = MAX_COMBO;

	ZCOMBOLEVEL thislevel;
	if(nCombo<5) thislevel=ZCL_NONE;else
		if(nCombo<10) thislevel=ZCL_GOOD;else
			if(nCombo<15) thislevel=ZCL_NICE;else
				if(nCombo<20) thislevel=ZCL_GREAT;else
					thislevel=ZCL_WONDERFUL;

	if(thislevel>m_CurrentComboLevel)
	{
		switch(thislevel) {
		case ZCL_GOOD	: AddGood();	
			break;
		case ZCL_NICE	: AddNice();
			break;
		case ZCL_GREAT	: AddGreat();
			break;
		case ZCL_WONDERFUL: AddWonderful();
			break;
		}
		m_CurrentComboLevel=thislevel;
	}

	if(thislevel==ZCL_NONE)
		m_CurrentComboLevel=ZCL_NONE;

	if(nCombo<3) return;

	if(m_pComboEffects[0]==NULL)
	{
		m_pComboEffects[0]=new ZComboEffect(m_pComboBeginEffect);
		Add(m_pComboEffects[0]);
	}

	char buffer[32];
	sprintf_safe(buffer,"%d",nCombo);
	int nCount=(int)strlen(buffer);

	for(int i=0;i<nCount;i++)
	{
		int ncurrent=buffer[i]-'0';
		if(combonumbers[i]!=ncurrent || m_pComboEffects[i+2]==NULL)
		{
			combonumbers[i]=ncurrent;

			if(m_pComboEffects[i+2]!=NULL)
			{
				RVisualMesh *pMesh=m_pComboEffects[i+2]->GetVMesh();

				if (pMesh)
				{
					AniFrameInfo* pInfo = pMesh->GetFrameInfo(ani_mode_lower);

					if(pInfo->m_pAniSet)
						pInfo->m_nFrame = pInfo->m_pAniSet->GetMaxFrame() - 4800.f*.2f;
				}

				m_pComboEffects[i+2]->DeleteAfter(1.f);
			}

			m_pComboEffects[i+2]=new ZComboEffect(m_pComboNumberEffect[ncurrent],rvector(-10.f+40.f*float(i-1),0,0));
			Add(m_pComboEffects[i+2]);
		}
	}
}

void ZScreenEffectManager::AddExpEffect(int nExp)
{
#define EXP_NUMBER_SPACE	30.f

	if(nExp==0) return;
	if (nExp > 0) PlaySoundScoreGet();

	ZEffect* pNew = NULL;

	char buffer[32];
	sprintf_safe(buffer,"%d",abs(nExp));
	int nCount=(int)strlen(buffer);


	for(int i=0;i<nCount;i++)
	{
		float fOffset=EXP_NUMBER_SPACE*(float)(i-nCount+2);
		Add(new ZScreenEffect(m_pExpNumberEffect[buffer[i]-'0'], rvector(fOffset,0,0)));
	}

	float fOffset=EXP_NUMBER_SPACE*(float)(3-nCount);
	Add(new ZScreenEffect(nExp>0 ? m_pExpPlusEffect : m_pExpMinusEffect , rvector(fOffset,0,0)));
}

void ZScreenEffectManager::DrawScoreBoard()
{
	m_pScorePanel->Draw(0);
}

void ZScreenEffectManager::AddAlert(const rvector& vVictimPos, const rvector& vVictimDir, const rvector& vAttackerPos)
{
	rvector my_dir = vVictimDir;
	rvector my_pos = vVictimPos;
	rvector attackPos = vAttackerPos;

	my_pos.z = 0.0f;
	attackPos.z = 0.0f;

	Normalize(my_dir);

	rvector dir = attackPos - my_pos;
	Normalize(dir);


	rvector vector1 = my_dir, vector2 = dir;
	vector1.y = -vector1.y;
	vector2.y = -vector2.y;
	float cosAng1 = DotProduct(vector1, vector2); 

	float r;
	if (-vector1.y*vector2.x + vector1.x*vector2.y > 0.0f)
	{
		r = (float)(acos(cosAng1));
	}
	else
	{
		r = -(float)(acos(cosAng1)); 
	}

	float t = (PI_FLOAT / 4.0f);

	int nIndex = -1;
	if (((r > 0) && (r < t)) || ((r <= 0) && (r > -t))) nIndex = 0;
	else if ((r <= -t) && (r > -t*3)) nIndex = 1;
	else if (((r >= t*3) && (r <= t*4)) || ((r <= -t*3) && (r > -t*4))) nIndex = 2;
	else if ((r >= t) && (r < t*3)) nIndex = 3;

	if ((nIndex >= 0) && (nIndex < 4))
	{
		for (iterator itor = begin(); itor != end(); ++itor)
		{
			ZScreenEffect* pEffect = (ZScreenEffect*)(*itor);
			if (pEffect->GetVMesh()->m_pMesh == m_pAlertEffect[nIndex]) return;
		}

		Add(new ZScreenEffect(m_pAlertEffect[nIndex]));
	}

}

void ZScreenEffectManager::PlaySoundScoreFlyby()
{
#ifdef _BIRDSOUND
	ZGetSoundEngine()->PlaySound("if_score_flyby");
#else
	ZGetSoundEngine()->PlaySound("if_score_flyby",false, 2000);
#endif
}

void ZScreenEffectManager::PlaySoundScoreGet()
{
	ZGetSoundEngine()->PlaySound("if_score_get");
}

void ZScreenEffectManager::AddPraise(int nPraise)
{
	if(nPraise<0 || nPraise>=ZCI_END) return;

	PlaySoundScoreFlyby(); 

	AddScreenEffect(m_pPraiseEffect[nPraise]);

	switch (nPraise)
	{
	case ZCI_ALLKILL:		ZGetGameInterface()->PlayVoiceSound(VOICE_KILLEDALL, 2000);	break;
	case ZCI_UNBELIEVABLE:	ZGetGameInterface()->PlayVoiceSound(VOICE_UNBELIEVABLE, 1300);	break;
	case ZCI_EXCELLENT:		ZGetGameInterface()->PlayVoiceSound(VOICE_DOUBLE_KILL, 1000);	break;
	case ZCI_FANTASTIC:		ZGetGameInterface()->PlayVoiceSound(VOICE_FANTASTIC, 1500);	break;
	case ZCI_HEADSHOT:		ZGetGameInterface()->PlayVoiceSound(VOICE_HEADSHOT, 700);	break;
	};

}

void ZScreenEffectManager::SetGuageExpFromMyInfo()
{
	int nExpPercent = ZGetMyInfo()->GetLevelPercent();
	float fRatio = (float)(nExpPercent) / 100.0f;
	SetGuage_EXP(fRatio);
}

void ZScreenEffectManager::AddGood()
{	
	AddScreenEffect(m_pGoodEffect); 
}
void ZScreenEffectManager::AddNice()
{	
	AddScreenEffect(m_pNiceEffect); 
}
void ZScreenEffectManager::AddGreat()
{	
	AddScreenEffect(m_pGreatEffect); 
}
void ZScreenEffectManager::AddWonderful()
{	
	AddScreenEffect(m_pWonderfullEffect); 
}
void ZScreenEffectManager::AddCool()
{	
	AddScreenEffect(m_pCoolEffect); 
}

void ZScreenEffectManager::AddRock()
{	
	AddScreenEffect("rock"); 
	ZGetGameInterface()->PlayVoiceSound(VOICE_LETS_ROCK, 500);
}

bool ZScreenEffectManager::CreateQuestRes()
{
	assert(m_pQuestEffectMeshMgr == NULL);
	assert(m_pBossHPPanel == NULL);
	assert(m_pArrow == NULL);
	assert(m_pKO == NULL);

	m_nKO = 0;

	m_pQuestEffectMeshMgr = new RMeshMgr();
	if(m_pQuestEffectMeshMgr->LoadXmlList("interface/default/combat/screeneffects_quest.xml")==-1) {
		mlog("quest combat list loding error\n");
		SAFE_DELETE(m_pQuestEffectMeshMgr);
		return false;
	}

	auto p = m_pQuestEffectMeshMgr->Get("ko");
	if (!p)
		MLog("ZScreenEffectManager::CreateQuestRes - Failed to load 'ko'\n");
	else
		m_pKO = new ZKOEffect(p);

	for (int i = 0; i < 10; i++)
	{
		_ASSERT(m_pKONumberEffect[i] == NULL);
		char name[64];
		sprintf_safe(name, "ko%d", i);

		p = m_pQuestEffectMeshMgr->Get(name);
		if (!p)
		{
			MLog("ZScreenEffectManager::CreateQuestRes - Failed to load '%s'\n", name);
			continue;
		}

		m_pKONumberEffect[i] = new ZKOEffect(p);
	}

	p = m_pQuestEffectMeshMgr->Get("boss_hppanel");
	if (!p)
		MLog("ZScreenEffectManager::CreateQuestRes - Failed to load 'boss_hppanel'\n");
	else
		m_pBossHPPanel = new ZBossGaugeEffect(p);

	p = m_pQuestEffectMeshMgr->Get("arrow");
	if (!p)
		MLog("ZScreenEffectManager::CreateQuestRes - Failed to load 'arrow'\n");
	else
		m_pArrow = new ZScreenEffect(p);

	return true;
}

void ZScreenEffectManager::DestroyQuestRes()
{
	SAFE_DELETE(m_pBossHPPanel);
	SAFE_DELETE(m_pArrow);
	SAFE_DELETE(m_pKO);

	for (int i = 0; i < 10; i++)
	{
		SAFE_DELETE(m_pKONumberEffect[i]);
	}

	SAFE_DELETE(m_pQuestEffectMeshMgr);
}

void ZScreenEffectManager::DrawQuestEffects()
{
	if (!ZGetGameTypeManager()->IsQuestDerived(ZGetGameClient()->GetMatchStageSetting()->GetGameType())) return;

	if (m_pBossHPPanel)
	{
		m_pBossHPPanel->Draw(0);
	}

	if(!ZGetGameInterface()->GetCombatInterface()->GetObserverMode())
	{
		DrawKO();

		if ( ZGetQuest()->IsRoundClear() )
		{
			rvector to = ZGetQuest()->GetGameInfo()->GetPortalPos();
			DrawArrow(to);
		}
	}	
}

void ZScreenEffectManager::AddKO(int nKills)
{
	m_nKO += nKills;

	for (int i = 0; i < 10; i++)
	{
		m_pKONumberEffect[i]->InitFrame();
	}
}

void ZScreenEffectManager::SetKO(int nKills)
{
	m_nKO = nKills;
}

void ZScreenEffectManager::DrawKO()
{
	if ((m_pKO == NULL) || (m_nKO <= 0)) return;

	char buffer[32];
	sprintf_safe(buffer,"%d", m_nKO);
	int nCount=(int)strlen(buffer);


	unsigned int nNowTime = GetGlobalTimeMS();

	int nFirstNumberFrame = -1;

	for(int i=0;i<nCount;i++)
	{
		int nIndex = buffer[i]-'0';
		if (nIndex < 0 || nIndex > 9)
			continue;


		if (i > 0)
		{
			AniFrameInfo* pInfo = m_pKONumberEffect[nIndex]->GetVMesh()->GetFrameInfo(ani_mode_lower);
			pInfo->m_nFrame = nFirstNumberFrame;
			
		}

		float fOffset= 40 * (float)(i-nCount+2) - 40;
		m_pKONumberEffect[nIndex]->DrawCustom(nNowTime, rvector(fOffset, 0.0f, 0.0f));

		if (i == 0)
		{
			nFirstNumberFrame = m_pKONumberEffect[nIndex]->GetFrame();
		}

		m_pKONumberEffect[nIndex]->InitFrame();
	}

	AniFrameInfo* pInfo = m_pKONumberEffect[buffer[0]-'0']->GetVMesh()->GetFrameInfo(ani_mode_lower);
	pInfo->m_nFrame = nFirstNumberFrame;

	m_pKO->Draw(nNowTime);
}

void ZScreenEffectManager::DrawArrow(rvector& vTargetPos)
{
	ZCharacter *pTargetCharacter = ZGetGameInterface()->GetCombatInterface()->GetTargetCharacter();
	if(!pTargetCharacter || !pTargetCharacter->GetInitialized()) return;

	rvector at = pTargetCharacter->GetPosition();
	rvector to = vTargetPos;

	rvector dir1, dir2;
	dir1 = pTargetCharacter->GetDirection();
	dir2 = to - at;

	float fAng = GetAngleOfVectors(dir2, dir1);

	const float fOffsetY = 285.0f;

	if (m_pArrow)
	{
		m_pArrow->DrawCustom(0, rvector(0.0f, fOffsetY, 0.0f), fAng);
	}
}

void ZScreenEffectManager::ShockBossGauge(float fPower)
{
	if (m_pBossHPPanel) m_pBossHPPanel->Shock(fPower);
}


void ZScreenEffectManager::DrawDuelEffects()
{
	return;
	if (ZGetGameClient()->GetMatchStageSetting()->GetGameType() != MMATCH_GAMETYPE_DUEL) return;

	if (ZGetCombatInterface()->GetObserver()->IsVisible()) return;

	if (g_pGame->m_pMyCharacter->GetKills() > 0)
	{
		char buffer[32];
		sprintf_safe(buffer,"%d", g_pGame->m_pMyCharacter->GetKills());
		int nCount=(int)strlen(buffer);

		unsigned int nNowTime = GetGlobalTimeMS();

		int nFirstNumberFrame = -1;

		for(int i=0;i<nCount;i++)
		{
			int nIndex = buffer[i]-'0';
			if (nIndex < 0 || nIndex > 9)
				continue;

			if (i > 0)
			{
				AniFrameInfo* pInfo = m_pKONumberEffect[nIndex]->GetVMesh()->GetFrameInfo(ani_mode_lower);
				pInfo->m_nFrame = nFirstNumberFrame;

			}

			float fOffset= 40 * (float)(i-nCount+2) - 40;
			m_pKONumberEffect[nIndex]->Update();
			m_pKONumberEffect[nIndex]->DrawCustom(nNowTime, rvector(fOffset, 0.0f, 0.0f));

			if (i == 0)
			{
				nFirstNumberFrame = m_pKONumberEffect[nIndex]->GetFrame();
			}

			m_pKONumberEffect[nIndex]->InitFrame();
		}

		AniFrameInfo* pInfo = m_pKONumberEffect[buffer[0]-'0']->GetVMesh()->GetFrameInfo(ani_mode_lower);
		pInfo->m_nFrame = nFirstNumberFrame;

		m_pKO->Update();
		m_pKO->Draw(nNowTime);	
	}
}

void ZScreenEffectManager::UpdateDuelEffects()
{
	for (int i = 0; i < 10; i++)
	{
		m_pKONumberEffect[i]->InitFrame();
	}

	ZRuleDuel* pDuel = (ZRuleDuel*)ZGetGame()->GetMatch()->GetRule();

	char buffer[32];
	sprintf_safe(buffer, "%d", pDuel->QInfo.m_nVictory);
	int nCount = (int)strlen(buffer);

	for (int i = 0; i < nCount; i++)
	{
		char meshname[256];
		sprintf_safe(meshname, "duel%d", buffer[i] - '0');
		RMesh *pMesh = m_pEffectMeshMgr->Get(meshname);
		if (pMesh)
			Add(new ZScreenEffect(pMesh, rvector(60 * (float)(i - nCount + 2), 0, 0)));
	}
}


void ZScreenEffectManager::DrawTDMEffects()
{
	if (ZGetGameClient()->GetMatchStageSetting()->GetGameType() != MMATCH_GAMETYPE_DEATHMATCH_TEAM2) return;

	auto nNowTime = GetGlobalTimeMS();

	m_pTDScoreBoard->Update();
	m_pTDScoreBoard->Draw(nNowTime);

	int nBlueKills = ZGetGame()->GetMatch()->GetTeamKills(MMT_BLUE);
	int nRedKills = ZGetGame()->GetMatch()->GetTeamKills(MMT_RED);
	int diff = abs(nRedKills - nBlueKills);


	if (nBlueKills > nRedKills)
	{
		m_pTDScoreBlink_B->SetAnimationSpeed(diff);
		m_pTDScoreBlink_B->Update();
		m_pTDScoreBlink_B->Draw(nNowTime);
	}
	else if (nRedKills > nBlueKills)
	{
		m_pTDScoreBlink_R->SetAnimationSpeed(diff);
		m_pTDScoreBlink_R->Update();
		m_pTDScoreBlink_R->Draw(nNowTime);
	}
}