#include "stdafx.h"
#include "ZActor.h"
#include "ZApplication.h"
#include "ZGame.h"
#include "ZObjectManager.h"
#include "ZEnemy.h"
#include "ZEnvObject.h"
#include "ZGameClient.h"
#include "ZCharacter.h"
#include "ZModule_HPAP.h"
#include "ZModule_Resistance.h"
#include "ZModule_ElementalDamage.h"
#include "ZModule_Skills.h"
#include "ZQuest.h"
#include "ZInput.h"
#include "ZPickInfo.h"
#include "ZScreenEffectManager.h"

MImplementRTTI(ZActor, ZCharacterObjectHistory);

ZActor::ZActor() : m_nFlags(0), m_nNPCType(NPC_GOBLIN_KING), m_pNPCInfo(NULL),
m_pModule_Skills(NULL), m_fSpeed(0.0f), m_pBrain(NULL)
{
	m_bIsNPC = true;

	memset(m_nLastTime, 0, sizeof(m_nLastTime));

	m_Position = rvector(0.0f, 0.0f, 0.0f);
	m_Direction = rvector(1.0f, 0.0f, 0.0f);
	m_TargetDir = rvector(1.0f, 0.0f, 0.0f);
	m_Accel = rvector(0.0f, 0.0f, 0.0f);

	m_Animation.Init(this);

	SetVisible(true);

	m_bInitialized = true;

	m_fDelayTime = 0.0f;
	m_fTC = 1.0f;
	m_nQL = 0;

	SetFlag(AF_LAND, true);

	m_bTestControl = false;

	m_TempBackupTime = -1;

	m_vAddBlastVel = rvector(0.f,0.f,0.f);
	m_fAddBlastVelTime = 0.f;

	strcpy_safe(m_szOwner,"unknown");

	m_TaskManager.SetOnFinishedCallback(OnTaskFinishedCallback);

	m_nDamageCount = 0;
}

ZActor::~ZActor()
{
	EmptyHistory();

	if (m_pBrain)
	{
		delete m_pBrain; m_pBrain = NULL;
	}
}

void ZActor::InitProperty() 
{
}

void ZActor::InitStatus() 
{
	int nMaxHP = m_pNPCInfo->nMaxHP;
	int nMaxAP = m_pNPCInfo->nMaxAP;

	nMaxHP = ZActor::CalcMaxHP(m_nQL, nMaxHP);
	nMaxAP = ZActor::CalcMaxAP(m_nQL, nMaxAP);

	m_pModule_HPAP->SetMaxHP(nMaxHP);
	m_pModule_HPAP->SetMaxAP(nMaxAP);

	if (ZGetQuest()->GetCheet(ZQUEST_CHEET_WEAKNPCS) == true) nMaxHP = 1;

	m_pModule_HPAP->SetHP(nMaxHP);
	m_pModule_HPAP->SetAP(nMaxAP);
	m_pModule_HPAP->SetRealDamage(true);

	EmptyHistory();
}

void ZActor::SetMyControl(bool bMyControl)
{
	SetFlag(AF_MY_CONTROL, bMyControl);
	EmptyHistory();
}

bool ZActor::IsDieAnimationDone()
{
	if(m_Animation.GetCurrState() == ZA_ANIM_DIE) {
		return m_pVMesh->isOncePlayDone();
	}	
	return false;
}

void ZActor::OnDraw()
{
	if (m_pVMesh == NULL) return;

	Draw_SetLight(m_Position );

	if( IsDieAnimationDone() )
	{
		constexpr auto TRAN_AFTER = 0.5f;
		constexpr auto VANISH_TIME = 1.f;

		if(m_TempBackupTime==-1) m_TempBackupTime = g_pGame->GetTime();

		float fOpacity = max(0.f,min(1.0f,(	VANISH_TIME-(g_pGame->GetTime()-m_TempBackupTime - TRAN_AFTER))/VANISH_TIME));

		m_pVMesh->SetVisibility(fOpacity);
	}
	else {
		if(!m_bHero) m_pVMesh->SetVisibility(1.f);
		m_TempBackupTime = -1;
	}

	m_pVMesh->Render();
}

void ZActor::TestControl(float fDelta)
{
	if (!MEvent::IsKeyDown(VK_SHIFT)) return;

	rvector m_Accel=rvector(0,0,0);

	rvector right;
	rvector forward=rvector(1,0,0);
	forward.z=0;
	Normalize(forward);
	CrossProduct(&right,rvector(0,0,1),forward);

	if(ZIsActionKeyDown(ZACTION_FORWARD)==true)	m_Accel+=forward;
	if(ZIsActionKeyDown(ZACTION_BACK)==true)		m_Accel-=forward;
	if(ZIsActionKeyDown(ZACTION_LEFT)==true)		m_Accel-=right;
	if(ZIsActionKeyDown(ZACTION_RIGHT)==true)	m_Accel+=right;

	Normalize(m_Accel);

	m_Accel*=(ACCEL_SPEED*fDelta*5.0f);
	SetVelocity(m_Accel);


}

void ZActor::OnUpdate(float fDelta)
{
	if(m_pVMesh) {
		m_pVMesh->SetVisibility(1.f);
	}

	if (CheckFlag(AF_MY_CONTROL))
	{
		m_TaskManager.Run(fDelta);
		CheckDead(fDelta);

		ProcessNetwork(fDelta);

		// for test - bird
		if (m_bTestControl)
		{
			TestControl(fDelta);
		}
		else
		{
			__BP(60,"ZActor::OnUpdate::ProcessAI");
			if(isThinkAble())
				ProcessAI(fDelta);
			__EP(60);
		}

		ProcessMovement(fDelta);
	}
	
	ProcessMotion(fDelta);

	if (CheckFlag(AF_MY_CONTROL))
	{
		UpdateHeight(fDelta);
	}
}

void ZActor::InitFromNPCType(MQUEST_NPC nNPCType, float fTC, int nQL)
{
	m_pNPCInfo = ZGetGameInterface()->GetQuest()->GetNPCInfo(nNPCType);
	_ASSERT(m_pNPCInfo);

	InitMesh(m_pNPCInfo->szMeshName, nNPCType);

	if(m_pNPCInfo->nNPCAttackTypes & NPC_ATTACK_MELEE ) {
		m_Items.EquipItem(MMCIP_MELEE, m_pNPCInfo->nWeaponItemID);
		m_Items.SelectWeapon(MMCIP_MELEE);
	}

	if(m_pNPCInfo->nNPCAttackTypes & NPC_ATTACK_RANGE ) {
		m_Items.EquipItem(MMCIP_PRIMARY, m_pNPCInfo->nWeaponItemID);
		m_Items.SelectWeapon(MMCIP_PRIMARY);
	}

	if(m_pNPCInfo->nSkills) {
		m_pModule_Skills = AddModule<ZModule_Skills>();
		m_pModule_Skills->Init(m_pNPCInfo->nSkills,m_pNPCInfo->nSkillIDs);
	}

	m_Collision.fRadius = m_pNPCInfo->fCollRadius;
	m_Collision.fHeight = m_pNPCInfo->fCollHeight;
	m_fTC = fTC;
	m_nQL = nQL;
	m_fSpeed = ZBrain::MakeSpeed(m_pNPCInfo->fSpeed);
	SetTremblePower(m_pNPCInfo->fTremble);
	
	if (m_pVMesh && m_pNPCInfo)
	{
		rvector scale;
		scale.x = m_pNPCInfo->vScale.x;
		scale.y = m_pNPCInfo->vScale.y;
		scale.z = m_pNPCInfo->vScale.z;
		m_pVMesh->SetScale(scale);

		if (scale.z != 1.0f)
		{
			m_Collision.fHeight *= scale.z;
		}
		if ((scale.x != 1.0f) || (scale.y != 1.0f))
		{
			float length = max(scale.x, scale.y);
			m_Collision.fRadius *= length;
		}
	}

	m_pBrain = ZBrain::CreateBrain(nNPCType);
	m_pBrain->Init(this);

	_ASSERT(m_pNPCInfo != NULL);
}

void ZActor::InitMesh(char* szMeshName, MQUEST_NPC nNPCType)
{
	// for test
	RMesh* pMesh;

	pMesh = ZGetNpcMeshMgr()->Get(szMeshName);
	if(!pMesh) 
	{
		_ASSERT(0);
		mlog("ZActor::InitMesh() -  원하는 모델을 찾을수 없음\n");
		return;
	}

	int nVMID = g_pGame->m_VisualMeshMgr.Add(pMesh);
	if(nVMID==-1) mlog("ZActor::InitMesh() - 캐릭터 생성 실패\n");

	RVisualMesh* pVMesh = g_pGame->m_VisualMeshMgr.GetFast(nVMID);

	SetVisualMesh(pVMesh);

	pVMesh->SetScale(rvector(15,15,15));

	m_Animation.Set(ZA_ANIM_RUN);

	pVMesh->m_FrameInfo[0].m_pAniIdEventSet = ZGetAniEventMgr()->GetAniIDEventSet((int)nNPCType);
}

void ZActor::UpdateHeight(float fDelta)
{
	if (GetDistToFloor() > 10.f || GetVelocity().z > 0.1f)
	{
        SetFlag(AF_LAND, false);
	}else {
		if(!CheckFlag(AF_LAND))
		{
			SetFlag(AF_LAND, true);
			m_Animation.Input(ZA_EVENT_REACH_GROUND);
		}
	}

	if(!CheckFlag(AF_LAND))
		m_pModule_Movable->UpdateGravity(fDelta);



	bool bJumpUp=(GetVelocity().z>0.0f);
	bool bJumpDown = false;

	if(m_pModule_Movable->isLanding())
	{
		SetFlag(AF_LAND, true);
		m_Animation.Input(ZA_EVENT_REACH_GROUND);
		SetVelocity(0,0,0);

		if(m_Position.z + 100.f < m_pModule_Movable->GetFallHeight())
		{
			float fSpeed=fabs(GetVelocity().z);

			RBspObject* r_map = ZGetGame()->GetWorld()->GetBsp();

			rvector vPos = m_Position;
			rvector vDir = rvector(0.f,0.f,-1.f);
			vPos.z += 50.f;

			RBSPPICKINFO pInfo;

			if(r_map->Pick(vPos,vDir,&pInfo)) 
			{
				vPos = pInfo.PickPos;

				vDir.x = pInfo.pInfo->plane.a;
				vDir.y = pInfo.pInfo->plane.b;
				vDir.z = pInfo.pInfo->plane.c;

				ZGetEffectManager()->AddLandingEffect(vPos,vDir);
			}
		}
	}

	rvector diff=fDelta*GetVelocity();

	bool bUp = (diff.z>0.01f);
	bool bDownward= (diff.z<0.01f);

	if(GetDistToFloor()<0 || (bDownward && m_pModule_Movable->GetLastMove().z>=0))
	{
		if(GetVelocity().z<1.f && GetDistToFloor()<1.f)
		{
			if(GetVelocity().z<0)
				SetVelocity(GetVelocity().x,GetVelocity().y,0);
		}
	}

	if(GetDistToFloor()<0 && !IsDie())
	{
		float fAdjust=400.f*fDelta;
		rvector diff=rvector(0,0,min(-GetDistToFloor(),fAdjust));
		Move(diff);
	}
}

void ZActor::UpdatePosition(float fDelta)
{
	if( CheckFlag(AF_MY_CONTROL) ) 
	{
		if ((CheckFlag(AF_BLAST) == true) && (GetCurrAni() == ZA_ANIM_BLAST) && (GetVelocity().z < 0.0f))
		{
			m_Animation.Input(ZA_EVENT_REACH_PEAK);
		}

		if ((CheckFlag(AF_BLAST_DAGGER) == true) && (GetCurrAni() == ZA_ANIM_BLAST_DAGGER) )
		{
			if( Magnitude(GetVelocity()) < 20.f )
			{
				m_Animation.Input(ZA_EVENT_REACH_GROUND_DAGGER);
				SetFlag(AF_BLAST_DAGGER,false);
			}
		}
	}

	m_pModule_Movable->Update(fDelta);

}

void ZActor::OnBlast(rvector &dir)
{
	if (!CheckFlag(AF_MY_CONTROL)) return;

	if (m_pNPCInfo->bNeverBlasted) return;

	rvector act_dir = GetDirection();
	act_dir.x = -dir.x;
	act_dir.y = -dir.y;
	SetDirection(act_dir);


	SetVelocity(dir * 300.f + rvector(0,0,1700.f));
	m_fDelayTime =3.0f;

	SetFlag(AF_BLAST, true);
	SetFlag(AF_LAND, false);

	m_Animation.Input(ZA_EVENT_BLAST);	
}

void ZActor::OnBlastDagger(rvector &dir,rvector& pos)
{
	if (!CheckFlag(AF_MY_CONTROL)) return;

	if (m_pNPCInfo->bNeverBlasted) return;

	rvector act_dir = GetDirection();
	act_dir.x = -dir.x;
	act_dir.y = -dir.y;
	SetDirection(act_dir);

	SetVelocity(dir * 300.f + rvector(0,0,100.f));

	m_vAddBlastVel = GetPosition() - pos;
	m_vAddBlastVel.z = 0.f;

	Normalize(m_vAddBlastVel);

	m_fAddBlastVelTime = g_pGame->GetTime();

	m_fDelayTime = 3.0f;

	SetFlag(AF_BLAST_DAGGER, true);
	SetFlag(AF_LAND, false);

	m_Animation.Input(ZA_EVENT_BLAST_DAGGER);
}

void ZActor::ProcessAI(float fDelta)
{
	if (!CheckFlag(AF_DEAD)) 
	{
		if (m_pBrain) m_pBrain->Think(fDelta);
	}

}

ZActor* ZActor::CreateActor(MQUEST_NPC nNPCType, float fTC, int nQL)
{
	ZActor* pNewActor=NULL;

	pNewActor = new ZHumanEnemy();

	if (pNewActor)
	{
		pNewActor->InitFromNPCType(nNPCType, fTC, nQL);
		pNewActor->InitProperty();
		pNewActor->InitStatus();

		if( pNewActor->m_pNPCInfo && pNewActor->m_pNPCInfo->bShadow ) {
			pNewActor->CreateShadow();
		}
	}


	return pNewActor;
}

void ZActor::PostBasicInfo()
{
	DWORD nNowTime = GetGlobalTimeMS();
	if (GetInitialized() == false) return;

	if(IsDie() && ZGetGame()->GetTime() - GetDeadTime()>5.f) return;
	int nMoveTick = (ZGetGameClient()->GetAllowTunneling() == false) ? PEERMOVE_TICK : PEERMOVE_AGENT_TICK;

	if ((int)(nNowTime - m_nLastTime[ACTOR_LASTTIME_BASICINFO]) >= nMoveTick)
	{
		m_nLastTime[ACTOR_LASTTIME_BASICINFO] = nNowTime;

		ZACTOR_BASICINFO pbi;
		pbi.fTime = ZGetGame()->GetTime();
		pbi.uidNPC = GetUID();

		pbi.posx = m_Position.x;
		pbi.posy = m_Position.y;
		pbi.posz = m_Position.z;

		pbi.velx = GetVelocity().x;
		pbi.vely = GetVelocity().y;
		pbi.velz = GetVelocity().z;

		pbi.dirx = GetDirection().x*32000.0f;
		pbi.diry = GetDirection().y*32000.0f;
		pbi.dirz = GetDirection().z*32000.0f;

		pbi.anistate = GetCurrAni();

		ZPOSTCMD1(MC_QUEST_PEER_NPC_BASICINFO, MCommandParameterBlob(&pbi,sizeof(ZACTOR_BASICINFO)));

		ZBasicInfoItem Item;
		Item.info.position = m_Position;
		Item.info.direction = GetDirection();
		Item.info.velocity = GetVelocity();
		Item.fSendTime = Item.fReceivedTime = ZGetGame()->GetTime();
		AddToHistory(Item);
	}
}


void ZActor::InputBasicInfo(ZBasicInfo* pni, BYTE anistate)
{
	if (!CheckFlag(AF_MY_CONTROL))
	{
		SetPosition(pni->position);
		SetVelocity(pni->velocity);
		SetDirection(pni->direction);
		m_Animation.ForceAniState(anistate);
		m_fLastBasicInfo = g_pGame->GetTime();
	}
}


bool ZActor::ProcessMotion(float fDelta)
{
	if (!m_pVMesh) return false;

	m_pVMesh->Frame();

	rvector pos = m_Position;
	rvector dir = m_Direction;
	dir.z = 0;

	rmatrix world;
	MakeWorldMatrix(&world,rvector(0,0,0), dir,rvector(0,0,1));

	rvector MeshPosition ;
	MeshPosition = pos;

	MakeWorldMatrix(&world,pos,dir,rvector(0,0,1));
	m_pVMesh->SetWorldMatrix(world);

	UpdatePosition(fDelta);

	if(IsActiveModule(ZMID_LIGHTNINGDAMAGE)==false) {

		if (m_pVMesh->isOncePlayDone())
		{
			m_Animation.Input(ZA_ANIM_DONE);
		}
	}

	return true;
}


void ZActor::ProcessMovement(float fDelta)
{
	bool bMoving = CheckFlag(AF_MOVING);
	bool bLand = CheckFlag(AF_LAND);

	if ( CheckFlag(AF_MOVING) && CheckFlag(AF_LAND) &&
		((GetCurrAni() == ZA_ANIM_WALK) || (GetCurrAni() == ZA_ANIM_RUN)))
	{
		float fSpeed = m_fSpeed;
		if (GetCurrAni() == ZA_ANIM_RUN) fSpeed = m_fSpeed;

		const float fAccel = 10000.f;

		AddVelocity(m_Direction * fAccel * fDelta);

		rvector vel = GetVelocity();
		if (Magnitude(vel) > fSpeed) {
			Normalize(vel);
			vel *= fSpeed;
			SetVelocity(vel);
		}
		
		return;
	}

	if(CheckFlag(AF_BLAST_DAGGER)) {
		float fSpeed = m_fSpeed;

#define BLAST_DAGGER_MAX_TIME 0.8f

		float fTime = max((1.f - (g_pGame->GetTime() - m_fAddBlastVelTime) / BLAST_DAGGER_MAX_TIME),0.0f);

		if( fTime < 0.4f )
			fTime = 0.f;

		float fPower = 400.f * fTime * fTime * fDelta * 80.f;

		if(fPower==0.f)
			SetFlag(AF_BLAST_DAGGER,false);

		rvector vel = m_vAddBlastVel * fPower;

		SetVelocity( vel );

		return;
	}

	if (ZActorAnimation::IsAttackAnimation(GetCurrAni()))
	{
		SetVelocity(rvector(0,0,0));
	}
	else
	{
		rvector dir=rvector(GetVelocity().x,GetVelocity().y,0);
		float fSpeed=Magnitude(dir);
		Normalize(dir);

		float fRatio = m_pModule_Movable->GetMoveSpeedRatio();

		float max_speed = 600.f	* fRatio;

		if(fSpeed>max_speed)
			fSpeed=max_speed;

#define NPC_STOP_SPEED			2000.f

		fSpeed = std::max(fSpeed-NPC_STOP_SPEED*fDelta, 0.0f);
		SetVelocity(dir.x*fSpeed, dir.y*fSpeed, GetVelocity().z);
	}
}

void ZActor::RunTo(rvector& dir)
{
	dir.z = 0.0f;
	Normalize(dir);

	SetDirection(dir);

	if (CheckFlag(AF_MOVING) == true) return;
	if (m_Animation.Input(ZA_INPUT_RUN) == false) return;



	SetFlag(AF_MOVING, true);
}

bool ZActor::IsDie() 
{ 
	if(CheckFlag(AF_MY_CONTROL))
		return CheckFlag(AF_DEAD); 

	if(m_Animation.GetCurrState() == ZA_ANIM_DIE) {
		return true;
	}
	return false;
}

void ZActor::Stop(bool bWithAniStop)
{
	SetVelocity(0,0,0);
	SetFlag(AF_MOVING, false);

	if (bWithAniStop) m_Animation.Input(ZA_INPUT_WALK_DONE);
}

void ZActor::RotateTo(const rvector& dir)
{
	SetDirection(dir);
	m_Animation.Input(ZA_INPUT_ROTATE);
}

void ZActor::Attack_Melee()
{
	m_Animation.Input(ZA_INPUT_ATTACK_MELEE);

	ZPOSTCMD1(MC_QUEST_PEER_NPC_ATTACK_MELEE, MCommandParameterUID(GetUID()));
}

void ZActor::Attack_Range(rvector& dir)
{
	m_Animation.Input(ZA_INPUT_ATTACK_RANGE);

	SetDirection(dir);
	rvector pos;
	pos = m_Position + rvector(0, 0, 100);
	ZPostNPCRangeShot(GetUID(), g_pGame->GetTime(), pos, pos + 10000.f*dir, MMCIP_PRIMARY);
}

void ZActor::Skill(int nSkill)
{
	ZSkillDesc *pDesc = m_pModule_Skills->GetSkill(nSkill)->GetDesc();
	if(pDesc) {
		if( pDesc->nCastingAnimation == 1 )
			m_Animation.Input(ZA_EVENT_SPECIAL1);
		else if (pDesc->nCastingAnimation == 2 )
			m_Animation.Input(ZA_EVENT_SPECIAL2);
		else if (pDesc->nCastingAnimation == 3 )
			m_Animation.Input(ZA_EVENT_SPECIAL3);
		else if (pDesc->nCastingAnimation == 4 )
			m_Animation.Input(ZA_EVENT_SPECIAL4);


		else { _ASSERT(FALSE); }
	}
}

void ZActor::DebugTest()
{
#ifndef _DEBUG
	return;
#endif

	if (m_pBrain) m_pBrain->DebugTest();
}


ZOBJECTHITTEST ZActor::HitTest(const rvector& origin, const rvector& to, float fTime, rvector *pOutPos)
{
	rvector footpos,actor_dir;
	if(!GetHistory(&footpos,&actor_dir,fTime)) return ZOH_NONE;

	if (m_pNPCInfo->bColPick)
	{
		rvector dir = to - origin;
		Normalize(dir);

		RPickInfo pickinfo;
		memset(&pickinfo,0,sizeof(RPickInfo));

		if (m_pVMesh->Pick(origin, dir, &pickinfo))
		{
			*pOutPos = pickinfo.vOut;
			if ((pickinfo.parts == eq_parts_head) || (pickinfo.parts == eq_parts_face))
			{
				return ZOH_HEAD;
			}
			else
			{
				return ZOH_BODY;
			}
		}
	}
	else
	{
		rvector headpos = footpos;
		if (m_pVMesh)
		{
			headpos.z += m_Collision.fHeight - 5.0f;
		}

		rvector ap,cp;
		float fDist=GetDistanceBetweenLineSegment(origin,to,footpos,headpos,&ap,&cp);
		float fDistToThis=Magnitude(origin-cp);
		if(fDist < (m_Collision.fRadius-5.0f))
		{
			rvector dir = to - origin;
			Normalize(dir);

			rvector ap2cp = ap - cp;
			float fap2cpsq = MagnitudeSq(ap2cp);
			float fdiff = sqrt(m_Collision.fRadius*m_Collision.fRadius - fap2cpsq);

			if(pOutPos) *pOutPos = ap-dir*fdiff;;

			return ZOH_BODY;
		}
	}

	return ZOH_NONE;
}

void ZActor::OnDamaged(ZObject* pAttacker, rvector srcPos, ZDAMAGETYPE damageType, MMatchWeaponType weaponType,
	float fDamage, float fPiercingRatio, int nMeleeType)
{
	if (!CheckFlag(AF_SOUND_WOUNDED))
	{
		bool bMyKill = false;
		if (pAttacker)
		{
			bMyKill = (pAttacker == g_pGame->m_pMyCharacter);
		}

		rvector pos_sound = GetPosition();
		pos_sound.z += m_Collision.fHeight - 10.0f;
		ZApplication::GetSoundEngine()->PlayNPCSound(m_pNPCInfo->nID, NPC_SOUND_WOUND, pos_sound, bMyKill);
		SetFlag(AF_SOUND_WOUNDED, true);		
	}

	if ((m_pNPCInfo->nGrade == NPC_GRADE_BOSS) || (m_pNPCInfo->nGrade == NPC_GRADE_LEGENDARY))
	{
		if (ZGetQuest()->GetGameInfo()->GetBoss() == GetUID())
		{
			ZGetScreenEffectManager()->ShockBossGauge(fDamage);
		}
	}

	if ((damageType == ZD_BULLET) || (damageType == ZD_BULLET_HEADSHOT))
	{
		m_nDamageCount++;
	}

	if(CheckFlag(AF_MY_CONTROL))
	{
		bool bSkipDamagedAnimation = false;

		if(m_pNPCInfo->bNeverAttackCancel ) {

			bSkipDamagedAnimation = ZActorAnimation::IsSkippableDamagedAnimation(GetCurrAni());
		}

		if(bSkipDamagedAnimation==false) {
		
			if((damageType==ZD_MELEE) || (damageType==ZD_KATANA_SPLASH)) {

				ZCharacterObject* pCObj = MDynamicCast(ZCharacterObject, pAttacker);

				bool bLightningDamage = false;

				if(pCObj) {
					ZC_ENCHANT etype = pCObj->GetEnchantType();
					if( etype == ZC_ENCHANT_LIGHTNING )
						bLightningDamage = true;	
				}

				if(bLightningDamage && (damageType==ZD_KATANA_SPLASH)) {
					m_Animation.Input(ZA_EVENT_LIGHTNING_DAMAGED);
				}
				else {
					if(nMeleeType%2)
						m_Animation.Input(ZA_EVENT_MELEE_DAMAGED1);
					else
						m_Animation.Input(ZA_EVENT_MELEE_DAMAGED2);

				}
				SetVelocity(0,0,0);
			}
			else {
				if( GetNPCInfo()->bNeverPushed == false)
				{
					if (m_nDamageCount >= 5)
					{
						m_Animation.Input(ZA_EVENT_RANGE_DAMAGED);
						m_nDamageCount = 0;
					}
				}
			}
		}
	}

	ZObject::OnDamaged(pAttacker,srcPos,damageType,weaponType,fDamage,fPiercingRatio,nMeleeType);
}

void ZActor::OnKnockback(const rvector& dir, float fForce)
{
	if(!CheckFlag(AF_MY_CONTROL)) return;

#define NPC_KNOCKBACK_FACTOR	1.0f

	ZCharacterObject::OnKnockback(dir,NPC_KNOCKBACK_FACTOR*fForce);
}

void ZActor::CheckDead(float fDelta)
{
	if (!CheckFlag(AF_MY_CONTROL)) return;
		 
	if (!CheckFlag(AF_DEAD))
	{
		if(m_pModule_HPAP->GetHP()<=0) {
			SetDeadTime(ZGetGame()->GetTime());
			m_Animation.Input(ZA_EVENT_DEATH);
			SetFlag(AF_DEAD, true);

			MUID uidKiller = this->GetLastAttacker();
			ZPostQuestPeerNPCDead(uidKiller, GetUID());

			m_TaskManager.Clear();
		}
	}
	else
	{
		if (!CheckFlag(AF_REQUESTED_DEAD))
		{
			if (ZGetGame()->GetTime()-GetDeadTime() > GetNPCInfo()->fDyingTime)
			{
				MUID uidAttacker = GetLastAttacker();
				if (uidAttacker == MUID(0, 0))
				{
					uidAttacker = ZGetGameClient()->GetPlayerUID();
				}

				ZPostQuestRequestNPCDead(uidAttacker, GetUID(), GetPosition());
				SetFlag(AF_REQUESTED_DEAD, true);
			}
		}
	}
}



void ZActor::ProcessNetwork(float fDelta)
{
	PostBasicInfo();
}


bool ZActor::IsAttackable()
{
	ZA_ANIM_STATE nAnimState = m_Animation.GetCurrState();
	if ( (nAnimState == ZA_ANIM_IDLE) || (nAnimState == ZA_ANIM_WALK) 
		|| (nAnimState == ZA_ANIM_RUN) 
		) return true;

	return false;
}

bool ZActor::IsCollideable()
{
	if (m_Collision.bCollideable)
	{
		ZA_ANIM_STATE nAnimState = m_Animation.GetCurrState();
		if (nAnimState == ZA_ANIM_DIE) return false;

		return (!IsDie());
	}

	return m_Collision.bCollideable;
}

bool ZActor::isThinkAble()
{
	if (!CheckFlag(AF_MY_CONTROL))		return false;
	if (CheckFlag(AF_BLAST_DAGGER))	return false;

	return true;
}


void ZActor::OnDie()
{
}

void ZActor::OnPeerDie(MUID& uidKiller)
{
	bool bMyKill = (ZGetGameClient()->GetPlayerUID() == uidKiller);

	rvector pos_sound = GetPosition();
	pos_sound.z += m_Collision.fHeight - 10.0f;
	ZApplication::GetSoundEngine()->PlayNPCSound(m_pNPCInfo->nID, NPC_SOUND_DEATH, pos_sound, bMyKill);

}

void ZActor::OnTaskFinishedCallback(ZActor* pActor, ZTASK_ID nLastID)
{
	if (pActor) pActor->OnTaskFinished(nLastID);
}

void ZActor::OnTaskFinished(ZTASK_ID nLastID)
{
	if (m_pBrain) m_pBrain->OnBody_OnTaskFinished(nLastID);
}


bool ZActor::CanSee(ZObject* pTarget)
{
	rvector vTargetDir = pTarget->GetPosition() - GetPosition();
	rvector vBodyDir = GetDirection();
	vBodyDir.z = vTargetDir.z = 0.0f;

	float angle = fabs(GetAngleOfVectors(vTargetDir, vBodyDir));
	if (angle <= m_pNPCInfo->fViewAngle) return true;

	return false;
}

bool ZActor::CanAttackRange(ZObject* pTarget)
{
	ZPICKINFO pickinfo;
	memset(&pickinfo,0,sizeof(ZPICKINFO));

	rvector pos,dir;
	pos = GetPosition() + rvector(0,0,50);
	dir = pTarget->GetPosition() - GetPosition();
	Normalize(dir);

	const DWORD dwPickPassFlag=RM_FLAG_ADDITIVE | RM_FLAG_HIDE | RM_FLAG_PASSROCKET | RM_FLAG_PASSBULLET;

	if (ZApplication::GetGame()->Pick(this, pos, dir, &pickinfo, dwPickPassFlag))
	{
		if (pickinfo.pObject)
		{
			if (pickinfo.pObject == pTarget) return true;
		}
	}

	return false;
}

bool ZActor::CanAttackMelee(ZObject* pTarget, ZSkillDesc *pSkillDesc)
{
	if (pSkillDesc == NULL)
	{
		float dist = Magnitude(pTarget->m_Position - m_Position);
		if (dist < m_pNPCInfo->fAttackRange) 
		{
			if (CanSee(pTarget)) return true;
		}
	}
	else
	{
		rvector Pos = GetPosition();
		rvector Dir = GetDirection();
		Dir.z=0; 
		Normalize(Dir);

		float fDist = Magnitude(Pos - pTarget->GetPosition());
		float fColMinRange = pSkillDesc->fEffectAreaMin * 100.0f;
		float fColMaxRange = pSkillDesc->fEffectArea * 100.0f;
		if ((fDist < fColMaxRange + pTarget->GetCollRadius()) && (fDist >= fColMinRange))
		{
			rvector vTargetDir = pTarget->GetPosition() - GetPosition();
			rvector vBodyDir = GetDirection();
			vBodyDir.z = vTargetDir.z = 0.0f;

			float angle = fabs(GetAngleOfVectors(vTargetDir, vBodyDir));
			if (angle <= pSkillDesc->fEffectAngle) return true;
		}
	}

	return false;
}