#include "stdafx.h"
#include "ZApplication.h"
#include "ZGame.h"
#include "ZCharacter.h"
#include "RVisualMeshMgr.h"
#include "RealSpace2.h"
#include "MDebug.h"
#include "MObject.h"
#include "ZPost.h"
#include "ZGameInterface.h"
#include "RBspObject.h"
#include "zshadow.h"
#include "MProfiler.h"
#include "RShaderMgr.h"
#include "ZScreenEffectManager.h"
#include "RDynamicLight.h"
#include "ZConfiguration.h"
#include "RCollisionDetection.h"
#include "ZEffectStaticMesh.h"
#include "ZEffectAniMesh.h"
#include "ZModule_HPAP.h"
#include "ZModule_Movable.h"
#include "ZModule_Resistance.h"
#include "ZModule_ElementalDamage.h"
#include "ZModule_QuestStatus.h"
#include "ZGameConst.h"
#include "RGMain.h"
#include "Portal.h"
#include "ZMyBotCharacter.h"

#define ANGLE_TOLER			.1f
#define ANGLE_SPEED			12.f

#define ROLLBACK_TOLER		20.f

#define ENABLE_CHARACTER_COLLISION

bool Enable_Cloth = true;

bool CheckTeenVersionMesh(RMesh** ppMesh)
{
	RWeaponMotionType type = eq_weapon_etc;

	type = (*ppMesh)->GetMeshWeaponMotionType();

	if( ZApplication::GetGameInterface()->GetTeenVersion() ) {

		if(type==eq_wd_katana) {
			*ppMesh = ZGetWeaponMeshMgr()->Get( "katana_wood" );
			return true;
		}
		else if(type==eq_ws_dagger) {
			*ppMesh = ZGetWeaponMeshMgr()->Get( "dagger_wood" );
			return true;
		}
		else if(type==eq_wd_sword) {
			*ppMesh = ZGetWeaponMeshMgr()->Get( "sword_wood" );
			return true;
		
		}
		else if(type==eq_wd_blade) {
			*ppMesh = ZGetWeaponMeshMgr()->Get( "blade_wood" );
			return true;
		}
	}

	return false;
}

static void ChangeEquipParts(RVisualMesh* pVMesh, u32* pItemID)
{
	struct _ZPARTSPAIR
	{
		_RMeshPartsType			meshparts;
		MMatchCharItemParts		itemparts;
	};

	static _ZPARTSPAIR PartsPair[] = 
	{
		{eq_parts_head, MMCIP_HEAD},
		{eq_parts_chest, MMCIP_CHEST},
		{eq_parts_hands, MMCIP_HANDS},
		{eq_parts_legs, MMCIP_LEGS},
		{eq_parts_feet, MMCIP_FEET}
	};

	for (int i = 0; i < 5; i++)
	{
		if (pItemID[PartsPair[i].itemparts] != 0)
		{
			MMatchItemDesc* pDesc = MGetMatchItemDescMgr()->GetItemDesc(pItemID[PartsPair[i].itemparts]);
			if (pDesc != NULL)
			{
				pVMesh->SetParts(PartsPair[i].meshparts, pDesc->m_szMeshName);
			}
			else
			{

			}
		}
		else
		{
			pVMesh->SetBaseParts( PartsPair[i].meshparts );
		}
	}

	pVMesh->SetBaseParts(eq_parts_face);
}

void ChangeCharFace(RVisualMesh* pVMesh, MMatchSex nSex, int nFaceIndex)
{
	if ((nFaceIndex < 0) || (nFaceIndex >= MAX_COSTUME_FACE)) return;
	if (pVMesh == NULL) return;

	char szMeshName[256];
	
	if (nSex == MMS_MALE)
	{
		strcpy_safe(szMeshName, g_szFaceMeshName[nFaceIndex][MMS_MALE].c_str());
	}
	else
	{
		strcpy_safe(szMeshName, g_szFaceMeshName[nFaceIndex][MMS_FEMALE].c_str());
	}

	pVMesh->SetParts(eq_parts_face, szMeshName);
}

void ChangeCharHair(RVisualMesh* pVMesh, MMatchSex nSex, int nHairIndex)
{
	if ((nHairIndex < 0) || (nHairIndex >= MAX_COSTUME_HAIR)) return;
	if (pVMesh == NULL) return;

	char szMeshName[256];
	if (nSex == MMS_MALE)
	{
		strcpy_safe(szMeshName, g_szHairMeshName[nHairIndex][MMS_MALE].c_str());
	}
	else
	{
		strcpy_safe(szMeshName, g_szHairMeshName[nHairIndex][MMS_FEMALE].c_str());
	}

	pVMesh->SetParts(eq_parts_head, szMeshName);
}

void ZChangeCharParts(RVisualMesh* pVMesh, MMatchSex nSex, int nHair, int nFace, u32* pItemID)
{
	if (pVMesh == NULL)
	{
		_ASSERT(0);
		return;
	}

	ChangeEquipParts(pVMesh, pItemID);

	if (pItemID[MMCIP_HEAD] == 0)
	{
		ChangeCharHair(pVMesh, nSex, nHair);
	}
	
	ChangeCharFace(pVMesh, nSex, nFace);
}

void ZChangeCharWeaponMesh(RVisualMesh* pVMesh, u32 nWeaponID)
{
	if( pVMesh ) 
	{
		if (nWeaponID == 0) 
		{
			RWeaponMotionType type = eq_ws_dagger;
			pVMesh->AddWeapon(type , NULL);
			pVMesh->SelectWeaponMotion(type);

			return;
		}


		RWeaponMotionType type = eq_weapon_etc;
		MMatchItemDesc* pDesc = MGetMatchItemDescMgr()->GetItemDesc(nWeaponID);
		
		if (pDesc == NULL)
		{
			return;
		}

		RMesh* pMesh = ZGetWeaponMeshMgr()->Get( pDesc->m_szMeshName );

		if( pMesh ) 
		{
			type = pMesh->GetMeshWeaponMotionType();
			CheckTeenVersionMesh(&pMesh);
			pVMesh->AddWeapon(type , pMesh);
			pVMesh->SelectWeaponMotion(type);
		}
	}

}

MImplementRTTI(ZCharacter, ZCharacterObject);

ZCharacter::ZCharacter() : ZCharacterObject(),
m_DirectionLower(1, 0, 0), m_DirectionUpper(1, 0, 0), m_TargetDir(1, 0, 0),
m_bAdminHide(false)
{
	m_nMoveMode = MCMM_RUN;
	m_nMode		= MCM_OFFENSIVE;
	m_nState	= MCS_STAND;

	m_nVMID = -1;

	m_nSelectSlot = 0;

	m_Collision.fRadius = CHARACTER_RADIUS-2;
	m_Collision.fHeight = CHARACTER_HEIGHT;

	m_fLastKillTime = 0;
	m_nKillsThisRound = 0;
	m_LastDamageType = ZD_NONE;
	m_LastDamageWeapon = MWT_NONE;
	m_LastDamageDot = 0.f;
	m_LastDamageDistance = 0.f;

	m_LastThrower = MUID(0,0);

	m_bDie = false;

	m_bCommander = false;
	m_bTagger = false;

	m_bStylishShoted = false;
	m_bStun = false;

	m_nBlastType = 0;
	m_bBlast = false;
	m_bBlastFall = false;
	m_bBlastDrop = false;
	m_bBlastStand = false;
	m_bBlastAirmove = false;

	m_bSpMotion = false;
	m_SpMotion = ZC_STATE_TAUNT;

	m_nTeamID = MMT_ALL;
	m_bIsLowModel = false;
	
	m_AniState_Upper=ZC_STATE_UPPER_NONE;
	m_AniState_Lower=ZC_STATE_LOWER_NONE;

	m_pAnimationInfo_Lower=NULL;
	m_pAnimationInfo_Upper=NULL;

	m_vProxyPosition = { 0, 0, 0 };

	for(int i=0;i<6;i++) {
		m_t_parts[i]  = 2;
		m_t_parts2[i] = 0;
	}

	m_fLastReceivedTime=0;

	m_Accel = rvector(0.0f, 0.0f, 0.0f);
	m_bRendered = false;

	m_nStunType = ZST_NONE;
	m_nWallJumpDir = 0;

	m_RealPositionBefore = rvector(0.f,0.f,0.f);
	m_AnimationPositionDiff = rvector(0.f,0.f,0.f);

	m_fGlobalHP = 0.f;
	m_nReceiveHPCount = 0;

	m_fAttack1Ratio = 1.f;

	m_bDamaged = false;
	m_bFallingToNarak = false;

	m_nWhichFootSound = 0;

	m_fTimeOffset = 0;
	m_nTimeErrorCount = 0;
	m_fAccumulatedTimeError = 0;

	if (ZGetGameTypeManager()->IsQuestDerived(ZGetGameClient()->GetMatchStageSetting()->GetGameType()))
		m_pModule_QuestStatus = AddModule<ZModule_QuestStatus>();
	else
		m_pModule_QuestStatus = nullptr;

	m_szUserName[0]=0;
	m_szUserAndClanName[0]=0;

	SetInvincibleTime(0);

	m_bChatEffect = false;
}

ZCharacter::~ZCharacter()
{
	Destroy();
}

void ZCharacter::EmptyHistory()
{
	if (m_bInitialized == false) return;

	BasicInfoHistory.clear();
}

void ZCharacter::Stop()
{
	SetAnimationLower(ZC_STATE_LOWER_IDLE1);
}

bool ZCharacter::IsTeam(ZCharacter* pChar)
{
	if(pChar) {
		if( pChar->GetTeamID() == GetTeamID()) {
			return true;
		}
	}
	return false;
}

bool ZCharacter::IsMoveAnimation()
{
	return g_AnimationInfoTableLower[m_AniState_Lower].bMove;
}

void ZCharacter::SetAnimation(const char *AnimationName,bool bEnableCancel,int time)
{
	if (m_bInitialized == false) return;

	SetAnimation(ani_mode_lower,AnimationName,bEnableCancel,time);
}

void ZCharacter::SetAnimation(RAniMode mode, const char *AnimationName,bool bEnableCancel,int time)
{
	if (m_bInitialized == false) 
		return;

	if(!m_pVMesh) 
		return;

	if(time) {
		m_pVMesh->SetReserveAnimation(mode,AnimationName,time);
	}
	else {
		m_pVMesh->SetAnimation(mode,AnimationName,bEnableCancel);
	}
}

DWORD g_dwLastAnimationTime=GetGlobalTimeMS();

void ZCharacter::SetAnimationLower(ZC_STATE_LOWER nAni)
{
	if (m_bInitialized == false) return;
	if ((IsDead()) && (IsHero())) return;

	if(nAni==m_AniState_Lower) return;
	_ASSERT(nAni>=0 && nAni<ZC_STATE_LOWER_END);

	if( nAni< 0 || nAni >= ZC_STATE_LOWER_END )
	{
		return;
	}

	m_AniState_Lower = nAni;
	m_pAnimationInfo_Lower = &g_AnimationInfoTableLower[nAni];

	SetAnimation(ani_mode_lower, m_pAnimationInfo_Lower->Name, m_pAnimationInfo_Lower->bEnableCancel, 0);

	if (!g_AnimationInfoTableLower[nAni].bContinuos)
		m_RealPositionBefore = rvector(0, 0, 0);

#ifdef TRACE_ANIMATION
	{
		DWORD curtime = GetGlobalTimeMS();
		if(g_pGame->m_pMyCharacter==this)
			mlog("animation - %d %s   - %d frame , interval %d \n",nAni,
			m_pVMesh->GetFrameInfo(ani_mode_lower)->m_pAniSet->GetName(),m_pVMesh->GetFrameInfo(ani_mode_lower)->m_nFrame,
			curtime-g_dwLastAnimationTime);
		g_dwLastAnimationTime = curtime;
	}
#endif
}

void ZCharacter::SetAnimationUpper(ZC_STATE_UPPER nAni)
{
	if (m_bInitialized == false) return;
	if ((IsDead()) && (IsHero())) return;

	if(nAni==m_AniState_Upper) 	return;

	_ASSERT(nAni>=0 && nAni<ZC_STATE_UPPER_END);

	if( nAni< 0 || nAni >= ZC_STATE_UPPER_END )
	{
		return;
	}

#ifdef TRACE_ANIMATION
	{
		DWORD curtime = GetGlobalTimeMS();
		mlog("upper Animation Index : %d %s @ %d \n", nAni ,g_AnimationInfoTableUpper[nAni].Name,curtime-g_dwLastAnimationTime);
		if(m_AniState_Upper==3 && nAni==0)
		{
			bool a=false;
		}
		g_dwLastAnimationTime = curtime;
	}
#endif
	m_AniState_Upper=nAni;
	m_pAnimationInfo_Upper=&g_AnimationInfoTableUpper[nAni];
	if( m_pAnimationInfo_Upper == NULL || m_pAnimationInfo_Upper->Name == NULL )
	{
		mlog("Fail to Get Animation Info.. Ani Index : [%d]\n", nAni );
		return;
	}
	SetAnimation(ani_mode_upper,m_pAnimationInfo_Upper->Name,m_pAnimationInfo_Upper->bEnableCancel,0);
}

void ZCharacter::UpdateLoadAnimation()
{
	if (m_bInitialized == false) return;

	if(m_pAnimationInfo_Lower)
	{
		SetAnimation(m_pAnimationInfo_Lower->Name,m_pAnimationInfo_Lower->bEnableCancel,0);
		SetAnimationUpper(ZC_STATE_UPPER_NONE);
		SetAnimationUpper(ZC_STATE_UPPER_LOAD);
		m_bPlayDone_upper=false;
	}
}

// ported from 1.5, this replaces UpdateDirection
void ZCharacter::UpdateMotion(float fDelta)
{
	if (m_bInitialized == false) return;
	// 점프로 모션 바꾸기 - 이전상태 백업
	// 점프는 어떤 상태에서든 모션이바뀔수있으므로..
	// run , idle

	// 자신의 타겟방향에 캐릭터의 방향을 맞춘다..
	if (IsDead()) { //허리 변형 없다~

		m_pVMesh->m_vRotXYZ.x = 0.f;
		m_pVMesh->m_vRotXYZ.y = 0.f;
		m_pVMesh->m_vRotXYZ.z = 0.f;

		return;
	}

	if ((m_AniState_Lower == ZC_STATE_LOWER_IDLE1) ||
		(m_AniState_Lower == ZC_STATE_LOWER_RUN_FORWARD) ||
		(m_AniState_Lower == ZC_STATE_LOWER_RUN_BACK))
	{
		m_Direction = m_TargetDir;

		rvector targetdir = m_TargetDir;
		targetdir.z = 0;
		Normalize(targetdir);

		rvector dir = m_Accel;
		dir.z = 0;

		if (Magnitude(dir) < 10.f) 
			dir = targetdir;
		else
			Normalize(dir);

		bool bInversed = false;
		if (DotProduct(targetdir, dir) < -cos(PI_FLOAT / 4.f) + 0.01f)
		{
			dir = -dir;
			bInversed = true;
		}

		// fAngleLower 는 현재 발방향과 해야하는 발방향의 각도 차이
		float fAngleLower = GetAngleOfVectors(dir, m_DirectionLower);

		rmatrix mat;

#define ROTATION_SPEED	400.f


		if (fAngleLower < -5.f / 180.f * PI_FLOAT)
		{
			mat = RGetRotZRad(min(ROTATION_SPEED * fDelta / 180.f * PI_FLOAT, -fAngleLower));
			m_DirectionLower = m_DirectionLower * mat;
		}

		// 일정각도 이상되면 하체를 틀어준다
		if (fAngleLower > 5.f / 180.f * PI_FLOAT)
		{
			mat = RGetRotZRad(max(-ROTATION_SPEED * fDelta / 180.f * PI_FLOAT, -fAngleLower));
			m_DirectionLower = m_DirectionLower * mat;
		}

		// 상체가 향해야 하는 방향은 언제나 타겟방향
		float fAngle = GetAngleOfVectors(m_TargetDir, m_DirectionLower);

		// 그러나 하체와의 각도를 항상 일정각도 이하로 유지한다.

		if (fAngle < -65.f / 180.f * PI_FLOAT)
		{
			fAngle = -65.f / 180.f * PI_FLOAT;
			mat = RGetRotZRad(-65.f / 180.f * PI_FLOAT);
			m_DirectionLower = m_Direction * mat;
		}

		if (fAngle >= 65.f / 180.f * PI_FLOAT)
		{
			fAngle = 65.f / 180.f * PI_FLOAT;
			mat = RGetRotZRad(65.f / 180.f * PI_FLOAT);
			m_DirectionLower = m_Direction * mat;
		}

		m_pVMesh->m_vRotXYZ.x = -fAngle * 180 / PI_FLOAT * .9f;

		// 실제보다 약간 고개를 들어준다 :)
		m_pVMesh->m_vRotXYZ.y = (m_TargetDir.z + 0.05f) * 50.f;
	}
	else // 달리기/가만있기등의 애니메이션이 아니면 허리안돌린다.
	{
		m_Direction = m_TargetDir;
		m_DirectionLower = m_Direction;

		m_pVMesh->m_vRotXYZ.x = 0.f;
		m_pVMesh->m_vRotXYZ.y = 0.f;
		m_pVMesh->m_vRotXYZ.z = 0.f;
	}
}

//void ZCharacter::UpdateDirection(float fDelta, const v3& Direction)
//{
//	if (m_bInitialized==false) return;
//
//	if (IsDead()) {
//		m_pVMesh->m_vRotXYZ = { 0, 0, 0 };
//		return;
//	}
//
//	auto&& s = *ZGetGameClient()->GetMatchStageSetting();
//	if ((s.IsRefinedMode() && s.IsGladOnly() == false))
//	{
//		m_Direction = Direction;
//		m_DirectionLower = m_Direction;
//
//		m_pVMesh->m_vRotXYZ.x = 0.f;
//		m_pVMesh->m_vRotXYZ.y = (Direction.z + 0.05f) * 50.f;
//		m_pVMesh->m_vRotXYZ.z = 0.f;
//	}
//	else
//	{
//		if ((m_AniState_Lower == ZC_STATE_LOWER_IDLE1) ||
//			(m_AniState_Lower == ZC_STATE_LOWER_RUN_FORWARD) ||
//			(m_AniState_Lower == ZC_STATE_LOWER_RUN_BACK))
//		{
//			m_Direction = m_TargetDir;
//
//			rvector targetdir = m_TargetDir;
//			targetdir.z = 0;
//			Normalize(targetdir);
//
//			rvector dir = m_Accel;
//			dir.z = 0;
//
//			if (Magnitude(dir) < 10.f)
//				dir = targetdir;
//			else
//				Normalize(dir);
//
//			bool bInversed = false;
//			if (DotProduct(targetdir, dir) < -cos(PI_FLOAT / 4.f) + 0.01f)
//			{
//				dir = -dir;
//				bInversed = true;
//			}
//
//			float fAngleLower = GetAngleOfVectors(dir, m_DirectionLower);
//
//			rmatrix mat;
//
//#define ROTATION_SPEED	400.f
//
//			if (fAngleLower > 5.f / 180.f*PI_FLOAT)
//			{
//				mat = RGetRotZRad(max(-ROTATION_SPEED * fDelta / 180.f*PI_FLOAT, -fAngleLower));
//				m_DirectionLower = m_DirectionLower * mat;
//			}
//
//			if (fAngleLower < -5.f / 180.f*PI_FLOAT)
//			{
//				mat = RGetRotZRad(min(ROTATION_SPEED*fDelta / 180.f*PI_FLOAT, -fAngleLower));
//				m_DirectionLower = m_DirectionLower * mat;
//			}
//
//			float fAngle = GetAngleOfVectors(m_TargetDir, m_DirectionLower);
//
//			if (fAngle < -65.f / 180.f*PI_FLOAT)
//			{
//				fAngle = -65.f / 180.f*PI_FLOAT;
//				mat = RGetRotZRad(-65.f / 180.f*PI_FLOAT);
//				m_DirectionLower = m_Direction * mat;
//			}
//
//			if (fAngle >= 65.f / 180.f*PI_FLOAT)
//			{
//				fAngle = 65.f / 180.f*PI_FLOAT;
//				mat = RGetRotZRad(65.f / 180.f*PI_FLOAT);
//				m_DirectionLower = m_Direction * mat;
//			}
//
//			m_pVMesh->m_vRotXYZ.x = -fAngle * 180 / PI_FLOAT * .9f;
//
//			m_pVMesh->m_vRotXYZ.y = (m_TargetDir.z + 0.05f) * 50.f;
//		}
//		else
//		{
//			m_Direction = m_TargetDir;
//			m_DirectionLower = m_Direction;
//
//			m_pVMesh->m_vRotXYZ.x = 0.f;
//			m_pVMesh->m_vRotXYZ.y = 0.f;
//			m_pVMesh->m_vRotXYZ.z = 0.f;
//		}
//
//
//	}
//}

static void GetDTM(bool* pDTM,int mode,bool isman)
{
	if(!pDTM) return;

		     if(mode==0) { pDTM[0]=true; pDTM[1]=false; }
		else if(mode==1) { pDTM[0]=false;pDTM[1]=true;  }
		else if(mode==2) { pDTM[0]=false;pDTM[1]=false; }
		else if(mode==3) { pDTM[0]=true; pDTM[1]=true;  }
}

void ZCharacter::CheckDrawWeaponTrack()
{
	if(m_pVMesh==NULL) return;

	bool bDrawTracks = false;

	if (ZGetConfiguration()->GetDrawTrails()
#ifdef PORTAL
		&& !g_pPortal->IsDrawingFakeChar()
#endif
		)
	{
		if ((m_pVMesh->m_SelectWeaponMotionType == eq_wd_katana) ||
			(m_pVMesh->m_SelectWeaponMotionType == eq_wd_sword) ||
			(m_pVMesh->m_SelectWeaponMotionType == eq_wd_blade))
		{

			if ((ZC_STATE_LOWER_ATTACK1 <= m_AniState_Lower && m_AniState_Lower <= ZC_STATE_LOWER_GUARD_CANCEL) ||
				(ZC_STATE_UPPER_LOAD <= m_AniState_Upper && m_AniState_Upper <= ZC_STATE_UPPER_GUARD_CANCEL))
			{
				if (m_AniState_Upper != ZC_STATE_UPPER_GUARD_IDLE) {
					if ((m_AniState_Lower != ZC_STATE_LOWER_ATTACK1_RET) &&
						(m_AniState_Lower != ZC_STATE_LOWER_ATTACK2_RET) &&
						(m_AniState_Lower != ZC_STATE_LOWER_ATTACK3_RET) &&
						(m_AniState_Lower != ZC_STATE_LOWER_ATTACK4_RET))
						bDrawTracks = true;
				}
			}
		}
	}

	bool bDTM[2];

	bDTM[0] = true;
	bDTM[1] = true;

	bool bMan = IsMan();

	if(m_pVMesh->m_SelectWeaponMotionType == eq_wd_blade) 
	{
			 if( m_AniState_Lower == ZC_STATE_LOWER_ATTACK1 ) GetDTM(bDTM,0,bMan);
		else if( m_AniState_Lower == ZC_STATE_LOWER_ATTACK2 ) GetDTM(bDTM,1,bMan);
		else if( m_AniState_Lower == ZC_STATE_LOWER_ATTACK3 ) GetDTM(bDTM,2,bMan);
		else if( m_AniState_Lower == ZC_STATE_LOWER_ATTACK4 ) GetDTM(bDTM,3,bMan);
	}

	m_pVMesh->m_bDrawTracksMotion[0] = bDTM[0];
	m_pVMesh->m_bDrawTracksMotion[1] = bDTM[1];

	m_pVMesh->SetDrawTracks(bDrawTracks);

}

void ZCharacter::UpdateSpWeapon()
{
	if(!m_pVMesh) return;

	m_pVMesh->UpdateSpWeaponFire();

	if(m_pVMesh->m_bAddGrenade) {

		rvector vWeapon[2];

		vWeapon[0] = m_pVMesh->GetCurrentWeaponPosition();

		rvector nPos = m_pVMesh->GetBipTypePosition(eq_parts_pos_info_Spine1);
		rvector nDir = vWeapon[0] - nPos;

		Normalize(nDir);

		RBSPPICKINFO bpi;
		if(ZGetGame()->GetWorld()->GetBsp()->Pick(nPos,nDir,&bpi))
		{
			if (DotProduct(bpi.pInfo->plane, vWeapon[0]) < 0) {
				vWeapon[0] = bpi.PickPos - nDir;
			}
		}

		vWeapon[1] = m_TargetDir;
		vWeapon[1].z += 0.1f;

		Normalize(vWeapon[1]);

		if(m_UID==g_pGame->m_pMyCharacter->m_UID) {

			int type = ZC_WEAPON_SP_GRENADE;

			RVisualMesh* pWVMesh = m_pVMesh->GetSelectWeaponVMesh();

			if( pWVMesh ) {
				if(pWVMesh->m_pMesh) {
					if(strncmp( pWVMesh->m_pMesh->GetName(),"flashbang",9) == 0) {
						type = ZC_WEAPON_SP_FLASHBANG;
					}
					else if(strncmp( pWVMesh->m_pMesh->GetName(),"smoke",5) == 0) {
						type = ZC_WEAPON_SP_SMOKE;
					}
					else if(strncmp( pWVMesh->m_pMesh->GetName(),"tear_gas",8) == 0) {
						type = ZC_WEAPON_SP_TEAR_GAS;	
					}
				}
			}

			int sel_type = GetItems()->GetSelectedWeaponParts();

			ZPostShotSp(g_pGame->GetTime(),vWeapon[0],vWeapon[1],type,sel_type);
			m_pVMesh->m_bAddGrenade = false;
		}
	}
}

bool ZCharacter::IsMan() const
{
	if(m_pVMesh) {
		if(m_pVMesh->m_pMesh) {
			if(strcmp(m_pVMesh->m_pMesh->GetName(),"heroman1")==0) {
				return true;
			}
		}
	}
	return false;
}

void ZCharacter::OnDraw()
{
	m_bRendered = false;

	if (!m_bInitialized) return;
	if (!IsVisible()) return;
	if (IsAdminHide()) return;

	auto ZCharacterDraw = MBeginProfile("ZCharacter::Draw");

	if (m_pVMesh && !Enable_Cloth)
		m_pVMesh->DestroyCloth();

	if(m_nVMID == -1)
		return;

	// Create the bounding box
	rboundingbox bb;
	static constexpr auto Radius = 100;
	static constexpr auto Height = 190;
	bb.vmax = rvector(Radius, Radius, Height);
	bb.vmin = rvector(-Radius, -Radius, 0);
	auto ScaleAndTranslate = [&](auto& vec) {
		if (Scale != 1.0f)
			vec *= Scale;
		vec += m_Position;
	};
	ScaleAndTranslate(bb.vmax);
	ScaleAndTranslate(bb.vmin);

	// Don't draw if the bounding box isn't visible
	if (!ZGetGame()->GetWorld()->GetBsp()->IsVisible(bb)) return;
	if (!isInViewFrustum(bb, RGetViewFrustum())) return;

	auto ZCharacterDrawLight = MBeginProfile("ZCharacter::Draw::Light");

	Draw_SetLight(m_Position);

	MEndProfile(ZCharacterDrawLight);

	if (g_pGame->m_bShowWireframe)
	{
		RGetDevice()->SetRenderState(D3DRS_LIGHTING, FALSE);
		RGetDevice()->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_DIFFUSE);
		RGetDevice()->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_SELECTARG1);
	}

	// If we're falling into the abyss, disable fog
	// and the depth buffer (pits have a black texture in the bottom)
	bool bNarakSetState = m_bFallingToNarak && g_pGame->GetWorld()->IsFogVisible();
	if (bNarakSetState)
	{
		RGetDevice()->SetRenderState(D3DRS_FOGENABLE, FALSE);
		RSetWBuffer(false);
	}

	CheckDrawWeaponTrack();

	auto MaxVisibility = 1.0f;
	// In the skillmap gamemode, all player characters
	// that aren't the player are transparent
	if (!m_bHero && ZGetGame()->GetMatch()->GetMatchType() == MMATCH_GAMETYPE_SKILLMAP)
		MaxVisibility = 0.4f;

	if(IsDead())
	{
		// If we are dead, fade out
		constexpr auto TRAN_AFTER = 3.0f;
		constexpr auto VANISH_TIME = 2.0f;

		float fOpacity = max(0.f, min(MaxVisibility, (
			VANISH_TIME - (g_pGame->GetTime() - GetDeadTime() - TRAN_AFTER)) / VANISH_TIME));
		m_pVMesh->SetVisibility(fOpacity);
	}
	else if (!m_bHero) m_pVMesh->SetVisibility(MaxVisibility);

	auto ZCharacterDrawVisualMeshRender = MBeginProfile("ZCharacter::Draw::VisualMesh::Render");

	UpdateEnchant();

	auto cpos = ZApplication::GetGameInterface()->GetCamera()->GetPosition();
	cpos = m_vProxyPosition - cpos;
	float dist = Magnitude(cpos);

	m_pVMesh->SetClothValue(g_pGame != nullptr, fabs(dist));
	m_pVMesh->Render(false);

	m_bRendered = m_pVMesh->m_bIsRender;

	if(m_pVMesh->m_bIsRenderWeapon && (m_pVMesh->GetVisibility() > 0.05f))
		DrawEnchant(m_AniState_Lower,m_bCharged);

#ifdef PORTAL
	g_pPortal->DrawFakeCharacter(this);
#endif
	
	if (bNarakSetState)
		RSetWBuffer(true);

	MEndProfile(ZCharacterDrawVisualMeshRender);

	MEndProfile(ZCharacterDraw);
}


bool ZCharacter::CheckDrawGrenade() const
{
	if (m_Items.GetSelectedWeapon() == NULL) return false;

	if (m_pVMesh) {
		if (m_pVMesh->m_SelectWeaponMotionType == eq_wd_grenade) {
			if (m_Items.GetSelectedWeapon()->GetBulletAMagazine()) {
				return true;
			}
		}
	}
	return false;
}

bool ZCharacter::Pick(const rvector& pos, const rvector& dir, RPickInfo* pInfo)
{
	if (!m_bInitialized)
		return false;

	return ZObject::Pick(pos, dir, pInfo);
}

#define GRAVITY_CONSTANT	2500.f
#define DAMAGE_VELOCITY		1700.f
#define MAX_FALL_SPEED		3000.f
#define MAX_FALL_DAMAGE		50.f
#define BLASTED_KNOCKBACK_RATIO	3.f

void ZCharacter::UpdateHeight(float fDelta)
{
	if (m_bFallingToNarak) return;

	m_bJumpUp=(GetVelocity().z>0);

	if(GetVelocity().z<0 && GetDistToFloor()>35.f)
	{
		if(!m_bJumpDown) {
			m_bJumpDown=true;
			m_bJumpUp = false;
		}
	}

	if(!m_bWallJump)
	{
		if(m_pModule_Movable->isLanding())
		{
			if(m_Position.z + 100.f < m_pModule_Movable->GetFallHeight())
			{
				float fSpeed = fabs(GetVelocity().z);
				if (fSpeed > DAMAGE_VELOCITY)
				{
					float fDamage = MAX_FALL_DAMAGE *
						(fSpeed - DAMAGE_VELOCITY) / (MAX_FALL_SPEED - DAMAGE_VELOCITY);

				}

				RBspObject* r_map = g_pGame->GetWorld()->GetBsp();

				rvector vPos = GetPosition();
				rvector vDir = rvector(0.f,0.f,-1.f);
				vPos.z += 50.f;

				RBSPPICKINFO pInfo;

				if(r_map->Pick(vPos,vDir,&pInfo)) {
					vPos = pInfo.PickPos;

					vDir.x = pInfo.pInfo->plane.a;
					vDir.y = pInfo.pInfo->plane.b;
					vDir.z = pInfo.pInfo->plane.c;

					ZGetEffectManager()->AddLandingEffect(vPos,vDir);

					AniFrameInfo* pInfo = m_pVMesh->GetFrameInfo(ani_mode_lower);
					RAniSoundInfo* pSInfo = &pInfo->m_SoundInfo;

					if(pSInfo->Name[0]) {
						pSInfo->isPlay = true;
						UpdateSound();
					}
					else {
						strcpy_safe(pSInfo->Name,"man_jump");
						pSInfo->isPlay = true;
						UpdateSound();
					}
				}

			}
		}
	}

	return;
}

void ZCharacter::UpdateSpeed()
{
	if (!m_pVMesh)
		return;

	float speed = 4.8f;
	float speed_upper = 4.8f;

	if (GetItems() && GetItems()->GetSelectedWeapon() && GetItems()->GetSelectedWeapon()->GetDesc()) {

		if( GetItems()->GetSelectedWeapon()->GetDesc()->m_nType==MMIT_MELEE) {
			if( (m_AniState_Lower == ZC_STATE_LOWER_ATTACK1)	 ||
				(m_AniState_Lower == ZC_STATE_LOWER_ATTACK1_RET) ||  
				(m_AniState_Lower == ZC_STATE_LOWER_ATTACK2)	 ||
				(m_AniState_Lower == ZC_STATE_LOWER_ATTACK2_RET) ||  
				(m_AniState_Lower == ZC_STATE_LOWER_ATTACK3)	 ||
				(m_AniState_Lower == ZC_STATE_LOWER_ATTACK3_RET) ||  
				(m_AniState_Lower == ZC_STATE_LOWER_ATTACK4)	 ||
				(m_AniState_Lower == ZC_STATE_LOWER_ATTACK4_RET) ||  
				(m_AniState_Lower == ZC_STATE_LOWER_ATTACK5)	 ||
				(m_AniState_Lower == ZC_STATE_LOWER_JUMPATTACK)  ||
				(m_AniState_Upper == ZC_STATE_UPPER_SHOT) ) {

				MMatchItemDesc* pRangeDesc = GetItems()->GetSelectedWeapon()->GetDesc();

				int nWeaponDelay = GetSelectWeaponDelay(pRangeDesc);

				int max_frame = m_pVMesh->GetMaxFrame(ani_mode_upper);

				if (max_frame == 0)
					max_frame = m_pVMesh->GetMaxFrame(ani_mode_lower);

				if(max_frame) {
					int _time = (int)(max_frame / 4.8f); 

					int as = _time + nWeaponDelay;
					if(as < 1)	as = 1;

					float fas = 0.f;
					fas = ( _time / (float)( as));

					m_fAttack1Ratio = fas;
					speed = 4.8f * m_fAttack1Ratio;
				}

				if(speed < 0.1f)
					speed = 0.1f;

			} 
		}
	}

	if(m_AniState_Upper == ZC_STATE_UPPER_LOAD) 
	{
		speed = 4.8f * 1.2f;
		speed_upper = 4.8f * 1.2f;
	}

	m_pVMesh->SetSpeed(speed,speed);
}

void ZCharacter::OnUpdate(float fDelta)
{
	if (m_bInitialized==false) return;
	if (!IsVisible()) return;

	UpdateSpeed();

	if(m_pVMesh) {
		m_pVMesh->SetVisibility(1.f);  
		m_pVMesh->Frame();
	}

	UpdateSound();
	UpdateMotion(fDelta);

	if( m_pVMesh && Enable_Cloth && m_pVMesh->isChestClothMesh() )
	{
		if(IsDead())
		{
			rvector force = rvector(0, 0, -150);
			m_pVMesh->UpdateForce(force);
			m_pVMesh->SetClothState(CHARACTER_DIE_STATE );
		}
		else
		{
			rvector force = -GetVelocity() * 0.15;
			force.z += -90;
			m_pVMesh->UpdateForce( force );
		}
	}
	
	rvector vRot(0.0f, 0.0f, 0.0f);;
	rvector vProxyDirection(0.0f, 0.0f, 0.0f);

	ZObserver *pObserver = ZGetCombatInterface()->GetObserver();
	if (pObserver->IsVisible())
	{
		rvector dir;
		if (!GetHistory(&m_vProxyPosition, &dir, g_pGame->GetTime() - pObserver->GetDelay()))
			return;

		{
			vProxyDirection = m_DirectionLower;

			float fAngle = GetAngleOfVectors(dir, vProxyDirection);

			vRot.x = -fAngle * 180 / PI_FLOAT * .9f;
			vRot.y = (dir.z + 0.05f) * 50.f;
			vRot.z = 0.f;

			m_pVMesh->m_vRotXYZ = vRot;
		}
		//UpdateDirection(fDelta, dir);
	}
	else {
		m_vProxyPosition = GetPosition();
		vProxyDirection = m_DirectionLower;
	}

	if(IsDead()) {
		vProxyDirection = m_Direction;
	}

	vProxyDirection.z = 0;
	Normalize(vProxyDirection);

	if (m_nVMID == -1) return;

	rmatrix world;
	MakeWorldMatrix(&world, rvector(0, 0, 0), vProxyDirection, rvector(0, 0, 1));

	rvector MeshPosition;

	// Move origin with the animation when it moves the player (e.g. ground slash)
	if(IsMoveAnimation())
	{
		rvector footposition = m_pVMesh->GetFootPosition();

		rvector RealPosition = footposition * world;

		if(m_AniState_Lower==ZC_STATE_LOWER_RUN_WALL)
		{
			rvector headpos = rvector(0.f, 0.f, 0.f);

			if(m_pVMesh) {
				AniFrameInfo* pAniLow = m_pVMesh->GetFrameInfo(ani_mode_lower);
				AniFrameInfo* pAniUp = m_pVMesh->GetFrameInfo(ani_mode_upper);
				m_pVMesh->m_pMesh->SetAnimation(pAniLow->m_pAniSet, pAniUp->m_pAniSet);
				m_pVMesh->m_pMesh->SetFrame(pAniLow->m_nFrame, pAniUp->m_nFrame);
				m_pVMesh->m_pMesh->SetMeshVis(m_pVMesh->m_fVis);
				m_pVMesh->m_pMesh->SetVisualMesh(m_pVMesh);

				m_pVMesh->m_pMesh->RenderFrame();

				RMeshNode* pNode = NULL;

				pNode = m_pVMesh->m_pMesh->FindNode(eq_parts_pos_info_Head);

				if(pNode) {
					headpos.x = pNode->m_mat_result._41;
					headpos.y = pNode->m_mat_result._42;
					headpos.z = pNode->m_mat_result._43;
				}
			}
			rvector rootpos = 0.5f*(footposition + headpos) * world;

			MeshPosition = m_vProxyPosition + rvector(0, 0, 90) - rootpos;
		}
		else
			MeshPosition = m_vProxyPosition - RealPosition;

		m_AnimationPositionDiff = footposition - m_RealPositionBefore;

		m_AnimationPositionDiff = m_AnimationPositionDiff * world;

		m_RealPositionBefore = footposition;

	}
	else
		MeshPosition = m_vProxyPosition;

	MakeWorldMatrix(&world, MeshPosition, vProxyDirection, rvector(0, 0, 1));
	m_pVMesh->SetWorldMatrix(world);

	rvector cpos = ZApplication::GetGameInterface()->GetCamera()->GetPosition();
	cpos = m_vProxyPosition - cpos;
	float dist = Magnitude(cpos);

	m_bIsLowModel = false;
	if (m_bFallingToNarak) m_bIsLowModel = false;

	m_bDamaged = false;

	CheckLostConn();

	if(m_bCharging && (m_AniState_Lower!=ZC_STATE_CHARGE && m_AniState_Lower!=ZC_STATE_LOWER_ATTACK1)) {
		m_bCharging = false;
	}

	if(m_bCharged && g_pGame->GetTime() > m_fChargedFreeTime) {
		m_bCharged = false;
	}

	UpdateSpWeapon();
}

void ZCharacter::CheckLostConn()
{
	if (g_pGame->GetTime()-m_fLastReceivedTime > 1.f)
	{
		if(!m_bLostConEffect)
		{
			m_bLostConEffect=true;
			ZGetEffectManager()->AddLostConIcon(this);
		}
		SetVelocity(rvector(0,0,0));
	}else
		m_bLostConEffect=false;
}

float ZCharacter::GetMoveSpeedRatio()
{
	float fRatio = 1.f;

	MMatchItemDesc* pMItemDesc = GetSelectItemDesc();

	if(pMItemDesc)
		fRatio = pMItemDesc->m_nLimitSpeed/100.f;

	return m_pModule_Movable->GetMoveSpeedRatio()*fRatio;
}

void ZCharacter::UpdateVelocity(float fDelta)
{
	rvector dir=rvector(GetVelocity().x,GetVelocity().y,0);
	float fSpeed=Magnitude(dir);
	if (fSpeed > 0)
		Normalize(dir);

	float fRatio = GetMoveSpeedRatio();

	float max_speed = MAX_SPEED * fRatio;

	if(fSpeed>max_speed)
		fSpeed=max_speed;

	bool bTumble= !IsDead() && (m_bTumble ||
		(ZC_STATE_LOWER_TUMBLE_FORWARD<=m_AniState_Lower && m_AniState_Lower<=ZC_STATE_LOWER_TUMBLE_LEFT));

	if(m_bLand && !m_bWallJump && !bTumble)
	{
		rvector forward=m_TargetDir;
		forward.z=0;
		Normalize(forward);

		// 최대값을 비율로 제어한다.
		float run_speed = RUN_SPEED * fRatio;
		float back_speed = BACK_SPEED * fRatio;
		float stop_formax_speed = STOP_FORMAX_SPEED * (1/fRatio);  

		if(DotProduct(forward,dir)>cosf(10.f*PI_FLOAT /180.f))
		{
			if(fSpeed>run_speed)
				fSpeed=max(fSpeed-stop_formax_speed*fDelta,run_speed);
		}
		else
		{
			if(fSpeed>back_speed)
				fSpeed=max(fSpeed-stop_formax_speed*fDelta,back_speed);
		}
	}

	if(IS_ZERO(Magnitude(m_Accel)) && m_bLand && !m_bWallJump && !m_bWallJump2 && !bTumble
		&& (!m_bBlast || m_nBlastType != 1))
		fSpeed = std::max(fSpeed-STOP_SPEED*fDelta,0.0f);

	SetVelocity(dir.x*fSpeed, dir.y*fSpeed, GetVelocity().z);
}

void ZCharacter::UpdateAnimation()
{
	if (m_bInitialized==false) return;
	SetAnimationLower(ZC_STATE_LOWER_IDLE1);
}

#define CHARACTER_COLLISION_DIST	70.f

#define ST_MAX_WEAPON 200
#define ST_MAX_PARTS  200

struct WeaponST {
	int		id;
	char*	name;
	RWeaponMotionType weapontype;
};

WeaponST g_WeaponST[ ST_MAX_WEAPON ] = {
	{ 0,"pistol01",	eq_wd_katana },
	{ 1,"pistol02",	eq_wd_katana },
	{ 2,"katana01",	eq_wd_katana },
	{ 3,"rifle01",	eq_wd_rifle  },
};

void ZCharacter::SetTargetDir(rvector vTarget) {

	Normalize(vTarget);
	m_TargetDir = vTarget;
}

static RMesh* GetDefaultWeaponMesh(MMatchWeaponType Type)
{
	auto& Items = *MGetMatchItemDescMgr();
	auto it = std::find_if(std::begin(Items), std::end(Items), [&](auto&& Item) {
		return Item.second->m_nWeaponType == Type;
	});

	if (it == std::end(Items))
		return nullptr;

	return ZGetWeaponMeshMgr()->Get(it->second->m_szMeshName);
}

void ZCharacter::OnChangeWeapon(MMatchItemDesc* Weapon)
{
	if(m_bInitialized==false) 
		return;

	if( m_pVMesh ) {

		RWeaponMotionType type = eq_weapon_etc;

		auto WeaponModelName = Weapon->m_szMeshName;
		RMesh* pMesh = ZGetWeaponMeshMgr()->Get( WeaponModelName );
		if (!pMesh && ZGetGame()->IsReplay())
			pMesh = GetDefaultWeaponMesh(Weapon->m_nWeaponType);

		if( pMesh ) {

			type = pMesh->GetMeshWeaponMotionType();

			CheckTeenVersionMesh(&pMesh);

			m_pVMesh->AddWeapon(type , pMesh);
			m_pVMesh->SelectWeaponMotion(type);
			UpdateLoadAnimation();
		}

		if( eq_wd_katana == type )
		{
#ifdef _BIRDSOUND
			ZGetSoundEngine()->PlaySoundCharacter("fx_blade_sheath", m_Position, IsObserverTarget());
#else
			ZGetSoundEngine()->PlaySound("fx_blade_sheath", m_Position, IsObserverTarget());
#endif
		}
		else if( (eq_wd_dagger == type) || (eq_ws_dagger == type) )
		{
#ifdef _BIRDSOUND
			ZGetSoundEngine()->PlaySoundCharacter("fx_dagger_sheath",m_Position,IsObserverTarget());
#else
			ZGetSoundEngine()->PlaySound("fx_dagger_sheath",m_Position, IsObserverTarget());
#endif
		}
		else if( eq_wd_sword == type )
		{
#ifdef _BIRDSOUND
			ZGetSoundEngine()->PlaySoundCharacter("fx_dagger_sheath", m_Position,IsObserverTarget());
#else
			ZGetSoundEngine()->PlaySound("fx_dagger_sheath", m_Position, IsObserverTarget());
#endif
		}
		else if( eq_wd_blade == type )
		{
#ifdef _BIRDSOUND
			ZGetSoundEngine()->PlaySoundCharacter("fx_dagger_sheath", m_Position, IsObserverTarget());
#else
			ZGetSoundEngine()->PlaySound("fx_dagger_sheath", m_Position, IsObserverTarget());
#endif
		}
	}
	
}

static const char* GetPartsNextName(RMeshPartsType ptype, RVisualMesh* pVMesh, bool bReverse)
{
	static bool bFirst = true;
	static vector<RMeshNode*> g_table[6*2];
	static int g_parts[6*2];

	if(bFirst) {

		RMesh* pMesh = ZGetMeshMgr()->Get("heroman1");

		if(pMesh) { //man

			pMesh->GetPartsNode( eq_parts_chest,g_table[0]);
			pMesh->GetPartsNode( eq_parts_head ,g_table[1]);
			pMesh->GetPartsNode( eq_parts_hands,g_table[2]);
			pMesh->GetPartsNode( eq_parts_legs ,g_table[3]);
			pMesh->GetPartsNode( eq_parts_feet ,g_table[4]);
			pMesh->GetPartsNode( eq_parts_face ,g_table[5]);
		}

		pMesh = ZGetMeshMgr()->Get("herowoman1");
		
		if(pMesh) { //woman

			pMesh->GetPartsNode( eq_parts_chest,g_table[6]);
			pMesh->GetPartsNode( eq_parts_head ,g_table[7]);
			pMesh->GetPartsNode( eq_parts_hands,g_table[8]);
			pMesh->GetPartsNode( eq_parts_legs ,g_table[9]);
			pMesh->GetPartsNode( eq_parts_feet ,g_table[10]);
			pMesh->GetPartsNode( eq_parts_face ,g_table[11]);
		}

		bFirst = false;
	}

	int mode = 0;

		 if(ptype==eq_parts_chest)	mode = 0;
	else if(ptype==eq_parts_head)	mode = 1;
	else if(ptype==eq_parts_hands)	mode = 2;
	else if(ptype==eq_parts_legs)	mode = 3;
	else if(ptype==eq_parts_feet)	mode = 4;
	else if(ptype==eq_parts_face)	mode = 5;
	else return NULL;

	if(pVMesh) {
		if(pVMesh->m_pMesh) {
			if(strcmp(pVMesh->m_pMesh->GetName(),"heroman1")!=0) {
				mode +=6;
			}
		}
	}

	if(bReverse) {

		g_parts[mode]--;

		if(g_parts[mode] < 0) {
			g_parts[mode] = (int)g_table[mode].size()-1;
		}

	}
	else {

		g_parts[mode]++;

		if(g_parts[mode] > (int)g_table[mode].size()-1) {
			g_parts[mode] = 0;
		}
	}

	return g_table[mode][g_parts[mode]]->GetName();
}

void ZCharacter::OnChangeParts(RMeshPartsType partstype,int PartsID)
{
#ifndef _PUBLISH
	if (m_bInitialized==false) return;
	if( m_pVMesh ) {
		if(partstype > eq_parts_etc && partstype < eq_parts_left_pistol) {
			if(PartsID == 0) {
				m_pVMesh->SetBaseParts( partstype );
			}
			else {
				const char* Name = nullptr;

				if(MEvent::GetCtrlState()) {
					Name = GetPartsNextName( partstype,m_pVMesh ,true);
				}
				else {
					Name = GetPartsNextName( partstype,m_pVMesh ,false);
				}

				if(Name)
					m_pVMesh->SetParts( partstype, Name );
			}
		}
	}

	if(Enable_Cloth)
		m_pVMesh->ChangeChestCloth(1.f,1);
#endif
}

void ZCharacter::Die()
{
	OnDie();
}

void ZCharacter::OnDie()
{
	if (m_bInitialized==false) return;
	if (!IsVisible()) return;

	m_bDie = true;
	m_Collision.bCollideable = false;
	m_bPlayDone = false;
	
}

bool ZCharacter::GetHistory(rvector *pos, rvector *direction, float fTime, rvector* cameradir)
{
	if (!BasicInfoHistory.empty() && fTime >= BasicInfoHistory.front().RecvTime)
	{
		auto Set = [](auto* a, auto&& b) { if (a) *a = b; };
		Set(pos, m_Position);
		Set(direction, m_Direction);
		Set(cameradir, m_Direction);
		return true;
	}

	auto GetItemDesc = [&](auto slot) {
		return m_Items.GetDesc(slot); };

	const auto Sex = IsMan() ? MMS_MALE : MMS_FEMALE;

	BasicInfoHistoryManager::Info Info;
	Info.Pos = pos;
	Info.Dir = direction;
	Info.CameraDir = cameradir;
	return BasicInfoHistory.GetInfo(Info, fTime, std::ref(GetItemDesc), Sex, IsDead());
}

void ZCharacter::GetPositions(v3* Head, v3* Foot, double Time)
{
	auto GetItemDesc = [&](auto slot) {
		return m_Items.GetDesc(slot); };

	if (!BasicInfoHistory.empty() && Time <= BasicInfoHistory.front().RecvTime)
	{
		BasicInfoHistoryManager::Info Info;
		Info.Head = Head;
		Info.Pos = Foot;
		BasicInfoHistory.GetInfo(Info, Time, GetItemDesc, m_Property.nSex, IsDead());
		return;
	}

	if (Head)
	{
		if (m_pVMesh)
			*Head = m_pVMesh->GetHeadPosition();
		else
			*Head = m_Position + v3{0, 0, 180};
	}

	if (Foot)
		*Foot = m_Position;
}

void ZCharacter::Revival()
{
	if (m_bInitialized==false) return;

	InitStatus();

	m_bDie = false;
	m_Collision.bCollideable = true;

	if(IsAdminHide() || GetTeamID() == MMT_SPECTATOR)
		m_bDie = true;

	SetAnimationLower(ZC_STATE_LOWER_IDLE1);
}

void ZCharacter::SetDirection(const rvector& dir)
{
	m_Direction = dir;
	m_DirectionLower = dir;
	m_DirectionUpper = dir;
	m_TargetDir = dir;
}

void ZCharacter::OnKnockback(const rvector& dir, float fForce)
{
	if(IsHero())
		ZCharacterObject::OnKnockback(dir,fForce);
}

void ZCharacter::UpdateSound()
{
	if (!m_bInitialized) return;
	if(m_pVMesh) {
		char szSndName[128]; szSndName[0] = 0;
		RMATERIAL* pMaterial{};
		RBSPPICKINFO bpi;
		if(ZGetGame()->GetWorld()->GetBsp()->Pick(m_Position+rvector(0,0,100),rvector(0,0,-1),&bpi) && bpi.nIndex != -1) {
			pMaterial = ZGetGame()->GetWorld()->GetBsp()->GetMaterial(bpi.pNode, bpi.nIndex);
		}

		AniFrameInfo* pInfo = m_pVMesh->GetFrameInfo(ani_mode_lower);
		int nFrame = pInfo->m_nFrame;
		int nCurrFoot = 0;

		auto GetFrame = [&](float x) {
			return static_cast<int>(x / 30.f * 4800.f); 
		};

		if(m_AniState_Lower==ZC_STATE_LOWER_RUN_FORWARD ||
			m_AniState_Lower==ZC_STATE_LOWER_RUN_BACK) {
			
			if(GetFrame(8) < nFrame && nFrame < GetFrame(18) )
				nCurrFoot = 1;
		}

		if(m_AniState_Lower==ZC_STATE_LOWER_RUN_WALL_LEFT ||
			m_AniState_Lower==ZC_STATE_LOWER_RUN_WALL_RIGHT ) {

			if (nFrame < GetFrame(9) ) nCurrFoot = 1;
			else if (nFrame < GetFrame(17) ) nCurrFoot = 0;
			else if (nFrame < GetFrame(24) ) nCurrFoot = 1;
			else if (nFrame < GetFrame(32) ) nCurrFoot = 0;
			else if (nFrame < GetFrame(40) ) nCurrFoot = 1;
			else if (nFrame < GetFrame(48) ) nCurrFoot = 0;
			else if (nFrame < GetFrame(55) ) nCurrFoot = 1;
		}

		if(m_AniState_Lower==ZC_STATE_LOWER_RUN_WALL ) {

			if (nFrame < GetFrame(8) ) nCurrFoot = 1;
			else if (nFrame < GetFrame(16) ) nCurrFoot = 0;
			else if (nFrame < GetFrame(26) ) nCurrFoot = 1;
			else if (nFrame < GetFrame(40) ) nCurrFoot = 0;
		}

		if(m_nWhichFootSound!=nCurrFoot && pMaterial) {	
			if(m_nWhichFootSound==0)
			{	
				// 왼발
				rvector pos = m_pVMesh->GetLFootPosition();
				char *szSndName=g_pGame->GetSndNameFromBsp("man_fs_l", pMaterial);

#ifdef _BIRDSOUND
				ZApplication::GetSoundEngine()->PlaySoundCharacter(szSndName,pos,IsObserverTarget());
#else
				ZApplication::GetSoundEngine()->PlaySound(szSndName,pos,IsObserverTarget());
#endif
			}else
			{
				rvector pos = m_pVMesh->GetRFootPosition();
				char *szSndName=g_pGame->GetSndNameFromBsp("man_fs_r", pMaterial);
#ifdef _BIRDSOUND
				ZApplication::GetSoundEngine()->PlaySoundCharacter(szSndName,pos,IsObserverTarget());
#else
				ZApplication::GetSoundEngine()->PlaySound(szSndName,pos,IsObserverTarget());
#endif
			}
			m_nWhichFootSound=nCurrFoot;
		}
         
		RAniSoundInfo* pSInfo;
		RAniSoundInfo* pSInfoTable[2];

		rvector p;

		AniFrameInfo* pAniLow = m_pVMesh->GetFrameInfo(ani_mode_lower);
		AniFrameInfo* pAniUp  = m_pVMesh->GetFrameInfo(ani_mode_upper);

		pSInfoTable[0] = &pAniLow->m_SoundInfo;
		pSInfoTable[1] = &pAniUp->m_SoundInfo;

		for(int i=0;i<2;i++) {

			pSInfo = pSInfoTable[i];

			if(pSInfo->isPlay) 
			{
				p = pSInfo->Pos;

				if(pMaterial)
				{
					strcpy_safe(szSndName, g_pGame->GetSndNameFromBsp(pSInfo->Name, pMaterial));

					int nStr = (int)strlen(szSndName);
					strncpy_safe(m_pSoundMaterial, szSndName + (nStr - 6), 7);

					ZApplication::GetSoundEngine()->PlaySoundElseDefault(szSndName, pSInfo->Name, p, IsObserverTarget());
				}
				else {
					m_pSoundMaterial[0] = 0;

					strcpy_safe(szSndName, pSInfo->Name);
#ifdef _BIRDSOUND
					ZApplication::GetSoundEngine()->PlaySoundCharacter(szSndName,p,IsObserverTarget());
#else
					ZApplication::GetSoundEngine()->PlaySound(szSndName,p,IsObserverTarget());
#endif
				}

				pSInfo->Clear();
			}
		}
	}

	if ( m_bDamaged && (!IsDead()) && (GetHP() < 30.f))
	{
		if(GetProperty()->nSex==MMS_MALE)
		{
#ifdef _BIRDSOUND
			ZGetSoundEngine()->PlaySoundCharacter("ooh_male",m_Position,IsObserverTarget());
#else
			ZGetSoundEngine()->PlaySound("ooh_male",m_Position,IsObserverTarget());
#endif
		}
		else			
		{
#ifdef _BIRDSOUND
			ZGetSoundEngine()->PlaySoundCharacter("ooh_female",m_Position,IsObserverTarget());
#else
			ZGetSoundEngine()->PlaySound("ooh_female",m_Position,IsObserverTarget());
#endif
		}
		m_bDamaged = false;
	}
}

bool ZCharacter::DoingStylishMotion()
{
	if ((m_AniState_Lower >= ZC_STATE_LOWER_RUN_WALL_LEFT) && 
		(m_AniState_Lower <= ZC_STATE_LOWER_JUMP_WALL_BACK))
	{
		return true;
	}

	return false;
}

void ZCharacter::UpdateStylishShoted()
{
	if (DoingStylishMotion())
	{
		m_bStylishShoted = true;
	}
	else
	{
		m_bStylishShoted = false;
	}
}

void ZCharacter::InitHPAP()
{
	m_pModule_HPAP->SetMaxHP(m_Property.fMaxHP);
	m_pModule_HPAP->SetMaxAP(m_Property.fMaxAP);

	m_pModule_HPAP->SetHP(m_Property.fMaxHP);
	m_pModule_HPAP->SetAP(m_Property.fMaxAP);

	SetHP(m_Property.fMaxHP);
	SetAP(m_Property.fMaxAP);

}

void ZCharacter::InitBullet()
{
	if (!m_Items.GetItem(MMCIP_PRIMARY)->IsEmpty()) 
	{
		int nBullet = m_Items.GetItem(MMCIP_PRIMARY)->GetDesc()->m_nMaxBullet;
		m_Items.GetItem(MMCIP_PRIMARY)->InitBullet(nBullet);
	}
	if (!m_Items.GetItem(MMCIP_SECONDARY)->IsEmpty()) 
	{
		int nBullet = m_Items.GetItem(MMCIP_SECONDARY)->GetDesc()->m_nMaxBullet;
		m_Items.GetItem(MMCIP_SECONDARY)->InitBullet(nBullet);
	}
	if (!m_Items.GetItem(MMCIP_CUSTOM1)->IsEmpty()) 
	{
		int nBullet = m_Items.GetItem(MMCIP_CUSTOM1)->GetDesc()->m_nMaxBullet;
		m_Items.GetItem(MMCIP_CUSTOM1)->InitBullet(nBullet);
	}
	if (!m_Items.GetItem(MMCIP_CUSTOM2)->IsEmpty()) 
	{
		int nBullet = m_Items.GetItem(MMCIP_CUSTOM2)->GetDesc()->m_nMaxBullet;
		m_Items.GetItem(MMCIP_CUSTOM2)->InitBullet(nBullet);
	}
}


void ZCharacter::InitStatus()
{
	InitHPAP();
	InitBullet();

	SetVelocity(0,0,0);

	m_bTagger = false;
	m_bCommander = false;
	m_bDie = false;
	m_Collision.bCollideable = true;
	m_bStylishShoted = false;
	m_bStun = false;

	m_bBlast = false;
	m_bBlastFall = false;
	m_bBlastDrop = false;
	m_bBlastStand = false;
	m_bBlastAirmove = false;

	m_bSpMotion = false;
	m_SpMotion = ZC_STATE_TAUNT;

	m_fLastReceivedTime=0;

	m_fLastKillTime = 0;
	m_nKillsThisRound = 0;
	m_LastDamageType = ZD_NONE;
	SetLastThrower(MUID(0,0), 0.0f);

	EmptyHistory();

	if(m_pVMesh)
		m_pVMesh->SetVisibility(1);

	m_bLostConEffect = false;

	m_bCharged = false;
	m_bCharging = false;
	m_bFallingToNarak = false;

	if(IsAdminHide() || GetTeamID() == MMT_SPECTATOR) {
		m_bDie = true;
		SetHP(0);
		SetAP(0);
		SetVisible(false);
	}

#ifndef _PUBLISH
	char szLog[128];
	sprintf_safe(szLog, "ZCharacter::InitStatus() - %s(%u) Initialized \n", 
		GetProperty()->szName, m_UID.Low);
	OutputDebugString(szLog);
#endif

	InitModuleStatus();
}

void ZCharacter::TestChangePartsAll()
{
	if( IsMan() ) {

		OnChangeParts(eq_parts_chest,0);
		OnChangeParts(eq_parts_head	,0);
		OnChangeParts(eq_parts_hands,0);
		OnChangeParts(eq_parts_legs	,0);
		OnChangeParts(eq_parts_feet	,0);
		OnChangeParts(eq_parts_face	,0);

	}
	else {

		OnChangeParts(eq_parts_chest,0);
		OnChangeParts(eq_parts_head	,0);
		OnChangeParts(eq_parts_hands,0);
		OnChangeParts(eq_parts_legs	,0);
		OnChangeParts(eq_parts_feet	,0);
		OnChangeParts(eq_parts_face	,0);
	}
}

#define AddText(s) { str.Add(#s,false); str.Add(" :",false); str.Add(s);}
#define AddTextEnum(s,e) {str.Add(#s,false); str.Add(" :",false); str.Add(#e);}

void ZCharacter::OutputDebugString_CharacterState()
{
	return;

	RDebugStr str;

	str.Add("//////////////////////////////////////////////////////////////" );

	AddText( m_bInitialized );
	AddText( m_bHero );

	AddText( m_nVMID );

	AddText( m_UID.High );
	AddText( m_UID.Low  );
	AddText( m_nTeamID );

	str.AddLine();

	str.Add("######  m_Property  #######\n");


	AddText( m_Property.szName );
	AddText( m_Property.nSex );

	str.AddLine();

	str.Add("######  m_Status  #######\n");

	AddText( GetHP() );
	AddText( GetAP() );
	AddText( m_Status.nLife );
	AddText( m_Status.nKills );
	AddText( m_Status.nDeaths );
	AddText( m_Status.nLoadingPercent );
	AddText( m_Status.nCombo );
	AddText( m_Status.nMaxCombo );
	AddText( m_Status.nAllKill );
	AddText( m_Status.nExcellent );
	AddText( m_Status.nFantastic );
	AddText( m_Status.nHeadShot );
	AddText( m_Status.nUnbelievable );

	str.AddLine();

	str.Add("######  m_Items  #######\n");


	ZItem* pItem = m_Items.GetSelectedWeapon();

	// 선택된 무기
#define IF_SITEM_ENUM(a)		if(a==m_Items.GetSelectedWeaponType())		{ AddTextEnum(m_Items.GetSelectedWeaponType(),a); }
#define ELSE_IF_SITEM_ENUM(a)	else if(a==m_Items.GetSelectedWeaponType())	{ AddTextEnum(m_Items.GetSelectedWeaponType(),a); }

	IF_SITEM_ENUM(MMCIP_HEAD)
	ELSE_IF_SITEM_ENUM(MMCIP_CHEST)
	ELSE_IF_SITEM_ENUM(MMCIP_HANDS)
	ELSE_IF_SITEM_ENUM(MMCIP_LEGS)
	ELSE_IF_SITEM_ENUM(MMCIP_FEET)
	ELSE_IF_SITEM_ENUM(MMCIP_FINGERL)
	ELSE_IF_SITEM_ENUM(MMCIP_FINGERR)
	ELSE_IF_SITEM_ENUM(MMCIP_MELEE)
	ELSE_IF_SITEM_ENUM(MMCIP_PRIMARY)
	ELSE_IF_SITEM_ENUM(MMCIP_SECONDARY)
	ELSE_IF_SITEM_ENUM(MMCIP_CUSTOM1)
	ELSE_IF_SITEM_ENUM(MMCIP_CUSTOM2)



	AddText( m_bDie );
	AddText( m_bStylishShoted );
	AddText( IsVisible() );
	AddText( m_bStun );
	AddText( m_nStunType );
	AddText( m_bPlayDone );

	AddText( m_nKillsThisRound );
	AddText( m_fLastKillTime );

	str.AddLine(1);

#define IF_LD_ENUM(a)		if(a==m_LastDamageType)			{ AddTextEnum(m_LastDamageType,a); }
#define ELSE_IF_LD_ENUM(a)	else if(a==m_LastDamageType)	{ AddTextEnum(m_LastDamageType,a); }

	IF_LD_ENUM(ZD_NONE)
	ELSE_IF_LD_ENUM(ZD_BULLET)
	ELSE_IF_LD_ENUM(ZD_MELEE)
	ELSE_IF_LD_ENUM(ZD_FALLING)
	ELSE_IF_LD_ENUM(ZD_EXPLOSION)
	ELSE_IF_LD_ENUM(ZD_BULLET_HEADSHOT)
	ELSE_IF_LD_ENUM(ZD_KATANA_SPLASH)
	ELSE_IF_LD_ENUM(ZD_HEAL)
	ELSE_IF_LD_ENUM(ZD_REPAIR)

	AddText( m_LastDamageDir );
	AddText( GetSpawnTime() );
	AddText( GetDistToFloor() );
	AddText( m_bLand );
	AddText( m_bWallJump );
	AddText( m_nWallJumpDir );
	AddText( m_bJumpUp );
	AddText( m_bJumpDown );
	AddText( m_bWallJump2 );
	AddText( m_bBlast );
	AddText( m_bBlastFall );
	AddText( m_bBlastDrop );
	AddText( m_bBlastStand );
	AddText( m_bBlastAirmove );
	AddText( m_bSpMotion );
	AddText( m_bDynamicLight );
	AddText( m_iDLightType );
	AddText( m_fLightLife );
	AddText( m_vLightColor );
	AddText( m_fTime );
	AddText( m_bLeftShot );
	AddText( m_TargetDir );
	AddText( m_Position );
	AddText( m_Direction );
	AddText( m_DirectionLower );
	AddText( m_DirectionUpper );
	AddText( m_RealPositionBefore );
	AddText( m_Accel );

	str.AddLine(1);

#define IF_Upper_ENUM(a)		if(a==m_AniState_Upper)			{ AddTextEnum(m_AniState_Upper,a); }
#define ELSE_IF_Upper_ENUM(a)	else if(a==m_AniState_Upper)	{ AddTextEnum(m_AniState_Upper,a); }

#define IF_Lower_ENUM(a)		if(a==m_AniState_Lower)			{ AddTextEnum(m_AniState_Lower,a); }
#define ELSE_IF_Lower_ENUM(a)	else if(a==m_AniState_Lower)	{ AddTextEnum(m_AniState_Lower,a); }

		 IF_Upper_ENUM(ZC_STATE_UPPER_NONE)
	ELSE_IF_Upper_ENUM(ZC_STATE_UPPER_SHOT)
	ELSE_IF_Upper_ENUM(ZC_STATE_UPPER_RELOAD)
	ELSE_IF_Upper_ENUM(ZC_STATE_UPPER_LOAD)
	ELSE_IF_Upper_ENUM(ZC_STATE_UPPER_GUARD_START)
	ELSE_IF_Upper_ENUM(ZC_STATE_UPPER_GUARD_IDLE)
	ELSE_IF_Upper_ENUM(ZC_STATE_UPPER_GUARD_BLOCK1)
	ELSE_IF_Upper_ENUM(ZC_STATE_UPPER_GUARD_BLOCK1_RET)
	ELSE_IF_Upper_ENUM(ZC_STATE_UPPER_GUARD_BLOCK2)
	ELSE_IF_Upper_ENUM(ZC_STATE_UPPER_GUARD_CANCEL)

		 IF_Lower_ENUM(ZC_STATE_LOWER_NONE)
	ELSE_IF_Lower_ENUM(ZC_STATE_LOWER_IDLE1)
	ELSE_IF_Lower_ENUM(ZC_STATE_LOWER_IDLE2)
	ELSE_IF_Lower_ENUM(ZC_STATE_LOWER_IDLE3)
	ELSE_IF_Lower_ENUM(ZC_STATE_LOWER_IDLE4)

	ELSE_IF_Lower_ENUM(ZC_STATE_LOWER_RUN_FORWARD)
	ELSE_IF_Lower_ENUM(ZC_STATE_LOWER_RUN_BACK)
	ELSE_IF_Lower_ENUM(ZC_STATE_LOWER_RUN_LEFT)
	ELSE_IF_Lower_ENUM(ZC_STATE_LOWER_RUN_RIGHT)

	ELSE_IF_Lower_ENUM(ZC_STATE_LOWER_JUMP_UP)
	ELSE_IF_Lower_ENUM(ZC_STATE_LOWER_JUMP_DOWN)

	ELSE_IF_Lower_ENUM(ZC_STATE_LOWER_DIE1)
	ELSE_IF_Lower_ENUM(ZC_STATE_LOWER_DIE2)
	ELSE_IF_Lower_ENUM(ZC_STATE_LOWER_DIE3)
	ELSE_IF_Lower_ENUM(ZC_STATE_LOWER_DIE4)

	ELSE_IF_Lower_ENUM(ZC_STATE_LOWER_RUN_WALL_LEFT)
	ELSE_IF_Lower_ENUM(ZC_STATE_LOWER_RUN_WALL_LEFT_DOWN)
	ELSE_IF_Lower_ENUM(ZC_STATE_LOWER_RUN_WALL)
	ELSE_IF_Lower_ENUM(ZC_STATE_LOWER_RUN_WALL_DOWN)
	ELSE_IF_Lower_ENUM(ZC_STATE_LOWER_RUN_WALL_RIGHT)
	ELSE_IF_Lower_ENUM(ZC_STATE_LOWER_RUN_WALL_RIGHT_DOWN)
	ELSE_IF_Lower_ENUM(ZC_STATE_LOWER_TUMBLE_FORWARD)
	ELSE_IF_Lower_ENUM(ZC_STATE_LOWER_TUMBLE_BACK)
	ELSE_IF_Lower_ENUM(ZC_STATE_LOWER_TUMBLE_RIGHT)
	ELSE_IF_Lower_ENUM(ZC_STATE_LOWER_TUMBLE_LEFT)
	ELSE_IF_Lower_ENUM(ZC_STATE_LOWER_BIND)
	ELSE_IF_Lower_ENUM(ZC_STATE_LOWER_JUMP_WALL_FORWARD)
	ELSE_IF_Lower_ENUM(ZC_STATE_LOWER_JUMP_WALL_BACK)
	ELSE_IF_Lower_ENUM(ZC_STATE_LOWER_JUMP_WALL_LEFT)
	ELSE_IF_Lower_ENUM(ZC_STATE_LOWER_JUMP_WALL_RIGHT)

	ELSE_IF_Lower_ENUM(ZC_STATE_LOWER_ATTACK1)
	ELSE_IF_Lower_ENUM(ZC_STATE_LOWER_ATTACK1_RET)
	ELSE_IF_Lower_ENUM(ZC_STATE_LOWER_ATTACK2)
	ELSE_IF_Lower_ENUM(ZC_STATE_LOWER_ATTACK2_RET)
	ELSE_IF_Lower_ENUM(ZC_STATE_LOWER_ATTACK3)
	ELSE_IF_Lower_ENUM(ZC_STATE_LOWER_ATTACK3_RET)
	ELSE_IF_Lower_ENUM(ZC_STATE_LOWER_ATTACK4)
	ELSE_IF_Lower_ENUM(ZC_STATE_LOWER_ATTACK4_RET)
	ELSE_IF_Lower_ENUM(ZC_STATE_LOWER_ATTACK5)

	ELSE_IF_Lower_ENUM(ZC_STATE_LOWER_JUMPATTACK)
	ELSE_IF_Lower_ENUM(ZC_STATE_LOWER_UPPERCUT)

	ELSE_IF_Lower_ENUM(ZC_STATE_LOWER_GUARD_START)
	ELSE_IF_Lower_ENUM(ZC_STATE_LOWER_GUARD_IDLE)
	ELSE_IF_Lower_ENUM(ZC_STATE_LOWER_GUARD_BLOCK1)
	ELSE_IF_Lower_ENUM(ZC_STATE_LOWER_GUARD_BLOCK1_RET)
	ELSE_IF_Lower_ENUM(ZC_STATE_LOWER_GUARD_BLOCK2)
	ELSE_IF_Lower_ENUM(ZC_STATE_LOWER_GUARD_CANCEL)

	ELSE_IF_Lower_ENUM(ZC_STATE_LOWER_BLAST)
	ELSE_IF_Lower_ENUM(ZC_STATE_LOWER_BLAST_FALL)
	ELSE_IF_Lower_ENUM(ZC_STATE_LOWER_BLAST_DROP)
	ELSE_IF_Lower_ENUM(ZC_STATE_LOWER_BLAST_STAND)
	ELSE_IF_Lower_ENUM(ZC_STATE_LOWER_BLAST_AIRMOVE)

	ELSE_IF_Lower_ENUM(ZC_STATE_DAMAGE)
	ELSE_IF_Lower_ENUM(ZC_STATE_DAMAGE2)
	ELSE_IF_Lower_ENUM(ZC_STATE_DAMAGE_DOWN)

	ELSE_IF_Lower_ENUM(ZC_STATE_TAUNT)
	ELSE_IF_Lower_ENUM(ZC_STATE_BOW)
	ELSE_IF_Lower_ENUM(ZC_STATE_WAVE)
	ELSE_IF_Lower_ENUM(ZC_STATE_LAUGH)
	ELSE_IF_Lower_ENUM(ZC_STATE_CRY)
	ELSE_IF_Lower_ENUM(ZC_STATE_DANCE)

	if(m_pAnimationInfo_Upper) {
		AddText(m_pAnimationInfo_Upper->Name);
	}

	if(m_pAnimationInfo_Lower) {
		AddText(m_pAnimationInfo_Lower->Name);
	}

	str.AddLine(1);

		 if(m_nMoveMode==MCMM_WALK)	{ AddTextEnum(m_nMoveMode,MCMM_WALK); }
	else if(m_nMoveMode==MCMM_RUN)	{ AddTextEnum(m_nMoveMode,MCMM_RUN);  }

		 if(m_nMode==MCM_PEACE)		{ AddTextEnum(m_nMode,MCM_PEACE);	  }
	else if(m_nMode==MCM_OFFENSIVE)	{ AddTextEnum(m_nMode,MCM_OFFENSIVE); }

		 if(m_nState==MCS_STAND)	{ AddTextEnum(m_nState,MCS_STAND);	}
	else if(m_nState==MCS_SIT)		{ AddTextEnum(m_nState,MCS_SIT);	}
	else if(m_nState==MCS_DEAD)		{ AddTextEnum(m_nState,MCS_DEAD);	}

	str.AddLine(1);

	AddText( m_bBackMoving );
	AddText( m_fLastReceivedTime );
	AddText( m_fGlobalHP );
	AddText( m_nReceiveHPCount );
	AddText( m_nSelectSlot );

	str.PrintLog();

	if(m_pVMesh) {
		m_pVMesh->OutputDebugString_CharacterState();
	}
}

#undef AddText
#undef AddTextEnum

#undef IF_SITEM_ENUM
#undef ELSE_IF_SITEM_ENUM

#undef IF_LD_ENUM
#undef ELSE_IF_LD_ENUM

#undef IF_Upper_ENUM
#undef ELSE_IF_Upper_ENUM

#undef IF_Lower_ENUM
#undef ELSE_IF_Lower_ENUM

void ZCharacter::TestToggleCharacter()
{
	if(m_pVMesh->m_pMesh) {

		RMesh* pMesh = NULL;

		if( strcmp(m_pVMesh->m_pMesh->GetName(),"heroman1")==0 ) {
			pMesh = ZGetMeshMgr()->Get("herowoman1");
			m_pVMesh->m_pMesh = pMesh;
			m_pVMesh->ClearParts();
			TestChangePartsAll();
		}
		else {
			pMesh = ZGetMeshMgr()->Get("heroman1");
			m_pVMesh->m_pMesh = pMesh;
			m_pVMesh->ClearParts();
			TestChangePartsAll();
		}
	}
}

void ZCharacter::InitMesh()
{
	RMesh* pMesh;

	char szMeshName[64];
	if (m_Property.nSex == MMS_MALE)
	{
		strcpy_safe(szMeshName, "heroman1");
	}
	else
	{
		strcpy_safe(szMeshName, "herowoman1");
	}
	pMesh = ZGetMeshMgr()->Get(szMeshName);

	if(!pMesh) {
		mlog("AddCharacter 원하는 모델을 찾을수 없음\n");
	}

	int nVMID = g_pGame->m_VisualMeshMgr.Add(pMesh);

	if(nVMID==-1) {
		mlog("AddCharacter 캐릭터 생성 실패\n");
	}

	m_nVMID = nVMID;

	RVisualMesh* pVMesh = g_pGame->m_VisualMeshMgr.GetFast(nVMID);
	SetVisualMesh(pVMesh);
}

void ZCharacter::ChangeLowPolyModel()
{
	if(m_pVMesh==NULL)
		return;

	char szMeshName[64];

	bool cloth_model = false;

	if( m_pVMesh ) {
		cloth_model = m_pVMesh->IsClothModel();
	}

	if ( cloth_model ) {
		strcpy_safe(szMeshName, "heroman_low1");
	}
	else {
		strcpy_safe(szMeshName, "heroman_low2");
	}

	RMesh* pLowMesh = ZGetMeshMgr()->Get(szMeshName);

	m_pVMesh->SetLowPolyModel(pLowMesh);
}

void ZCharacter::InitProperties()
{
	MTD_CharInfo* pCharInfo = &m_InitialInfo;

	m_Property.SetName(pCharInfo->szName);
	m_Property.SetClanName(pCharInfo->szClanName);
	m_Property.nSex = (MMatchSex)pCharInfo->nSex;
	m_Property.nHair = pCharInfo->nHair;
	m_Property.nFace = pCharInfo->nFace;
	m_Property.nLevel = pCharInfo->nLevel;
	m_Property.fMaxHP = pCharInfo->nHP;
	m_Property.fMaxAP = pCharInfo->nAP;

	float fAddedAP = DEFAULT_CHAR_AP;
	for (int i = 0; i < MMCIP_END; i++)
	{
		if (!m_Items.GetItem(MMatchCharItemParts(i))->IsEmpty())
		{
			fAddedAP += m_Items.GetItem(MMatchCharItemParts(i))->GetDesc()->m_nAP;
		}
	}

	float fAddedHP = DEFAULT_CHAR_HP;
	for (int i = 0; i < MMCIP_END; i++)
	{
		if (!m_Items.GetItem(MMatchCharItemParts(i))->IsEmpty())
		{
			fAddedHP += m_Items.GetItem(MMatchCharItemParts(i))->GetDesc()->m_nHP;
		}
	}

	m_Property.fMaxAP += fAddedAP;
	m_Property.fMaxHP += fAddedHP;

	/*if(GetUserGrade() == MMUG_DEVELOPER) {
		strcpy_safe(m_szUserName,ZMsg(MSG_WORD_DEVELOPER));
		strcpy_safe(m_szUserAndClanName,ZMsg(MSG_WORD_DEVELOPER));
	}
	else if(GetUserGrade() == MMUG_ADMIN) {
		strcpy_safe(m_szUserName,ZMsg(MSG_WORD_ADMIN));
		strcpy_safe(m_szUserAndClanName,ZMsg(MSG_WORD_ADMIN));
	}
	else {*/
		strcpy_safe(m_szUserName,m_Property.szName);
		if(m_Property.szClanName[0])
			sprintf_safe(m_szUserAndClanName,"%s(%s)",m_Property.szName,m_Property.szClanName);
		else
			sprintf_safe(m_szUserAndClanName,"%s",m_Property.szName);
	//}

	MMatchObjCache* pObjCache = ZGetGameClient()->FindObjCache(GetUID());
	if (pObjCache && IsAdminGrade(pObjCache->GetUGrade()) && 
		pObjCache->CheckFlag(MTD_PlayerFlags_AdminHide))
		m_bAdminHide = true;
	else
		m_bAdminHide = false;

	auto& MatchSetting = *ZGetGameClient()->GetMatchStageSetting();
	if (MatchSetting.IsForcedHPAP())
	{
		m_Property.fMaxHP = MatchSetting.GetForcedHP();
		m_Property.fMaxAP = MatchSetting.GetForcedAP();
	}
}

bool ZCharacter::Create(const MTD_CharInfo& CharInfo)
{
	_ASSERT(!m_bInitialized);

	memcpy(&m_InitialInfo, &CharInfo, sizeof(MTD_CharInfo));

	for (int i = 0; i < MMCIP_END; i++)
		m_Items.EquipItem(MMatchCharItemParts(i), CharInfo.nEquipedItemDesc[i]);

	InitProperties();

	InitMesh();
	m_bInitialized = true;

	SetAnimationLower(ZC_STATE_LOWER_IDLE1);
	SetAnimationUpper(ZC_STATE_UPPER_NONE);

	InitMeshParts();

	CreateShadow();
	
	m_pSoundMaterial[0] = 0;

	if(Enable_Cloth)
	{
		m_pVMesh->ChangeChestCloth(1.f,1);
	}

	ChangeLowPolyModel();

	m_bIsLowModel = false;
	SetVisible(true);

	m_fAttack1Ratio = 1.f;

	ZGetEmblemInterface()->AddClanInfo(GetClanID());

	MMatchItemDesc *pDesc = m_Items.GetDesc(MMCIP_MELEE);
	if (pDesc)
	{
		switch (pDesc->m_nID)
		{
		case 8501:
			GetRGMain().SetSwordColor(GetUID(), 0xFFFFB7D5);
			break;
		case 8502:
			GetRGMain().SetSwordColor(GetUID(), 0xFF00FF00);
			break;
		case 8503:
			GetRGMain().SetSwordColor(GetUID(), 0xFF00FFFF);
			break;
		}
	}

	return true;
}

void ZCharacter::Destroy()
{
	if(m_bInitialized)
		ZGetEmblemInterface()->DeleteClanInfo(GetClanID());

	m_bInitialized = false;
}

void ZCharacter::InitMeshParts()
{
	RMeshPartsType mesh_parts_type;

	if (m_pVMesh)
	{
		for (int i = 0; i < MMCIP_END; i++)
		{
			switch (MMatchCharItemParts(i))
			{
			case MMCIP_HEAD:
				mesh_parts_type = eq_parts_head;
				break;
			case MMCIP_CHEST:
				mesh_parts_type = eq_parts_chest;
				break;
			case MMCIP_HANDS:
				mesh_parts_type = eq_parts_hands;
				break;
			case MMCIP_LEGS:
				mesh_parts_type = eq_parts_legs;
				break;
			case MMCIP_FEET:
				mesh_parts_type = eq_parts_feet;
				break;
			default:
				continue;
			}

			auto&& Item = GetItems()->GetItem(MMatchCharItemParts(i));
			if (!Item->IsEmpty())
			{
				m_pVMesh->SetParts(mesh_parts_type, Item->GetDesc()->m_szMeshName);
			}
			else
			{
				m_pVMesh->SetBaseParts(mesh_parts_type);
			}
		}

		if (GetItems()->GetItem(MMCIP_HEAD)->IsEmpty())
		{
			ChangeCharHair(m_pVMesh, m_Property.nSex, m_Property.nHair);	
		}
		
		ChangeCharFace(m_pVMesh, m_Property.nSex, m_Property.nFace);

	}


	SetAnimationUpper(ZC_STATE_UPPER_NONE);
	SetAnimationLower(ZC_STATE_LOWER_IDLE1);

	SelectWeapon();
}

void ZCharacter::SelectWeapon()
{
	if (!g_pGame->GetMatch()->IsRuleGladiator())
	{
		if (!m_Items.GetItem(MMCIP_PRIMARY)->IsEmpty()) ChangeWeapon(MMCIP_PRIMARY);
		else if (!m_Items.GetItem(MMCIP_SECONDARY)->IsEmpty()) ChangeWeapon(MMCIP_SECONDARY);
		else if (!m_Items.GetItem(MMCIP_MELEE)->IsEmpty()) ChangeWeapon(MMCIP_MELEE);
		else if (!m_Items.GetItem(MMCIP_CUSTOM1)->IsEmpty()) ChangeWeapon(MMCIP_CUSTOM1);
		else if (!m_Items.GetItem(MMCIP_CUSTOM2)->IsEmpty()) ChangeWeapon(MMCIP_CUSTOM2);
		else ChangeWeapon(MMCIP_PRIMARY);
	}
	else
	{
		if (!m_Items.GetItem(MMCIP_MELEE)->IsEmpty()) ChangeWeapon(MMCIP_MELEE);
		else if (!m_Items.GetItem(MMCIP_CUSTOM1)->IsEmpty()) ChangeWeapon(MMCIP_CUSTOM1);
		else if (!m_Items.GetItem(MMCIP_CUSTOM2)->IsEmpty()) ChangeWeapon(MMCIP_CUSTOM2);
		else ChangeWeapon(MMCIP_PRIMARY);
	}
}

void ZCharacter::ChangeWeapon(MMatchCharItemParts nParts)
{
	if(m_Items.GetSelectedWeaponParts()==nParts) return;

	if( nParts < 0 || nParts > MMCIP_END )
	{
		return;
	}
	if (m_Items.GetItem(nParts) == NULL) return;
	if (m_Items.GetItem(nParts)->GetDesc() == NULL) return;

	if (g_pGame->GetMatch()->IsRuleGladiator())
	{
		if ((nParts == MMCIP_PRIMARY) || (nParts == MMCIP_SECONDARY)) {
			return;
		}
	}

	// equipped item before change
	MMatchCharItemParts BackupParts = m_Items.GetSelectedWeaponParts();
		
	m_Items.SelectWeapon(nParts);

	if(m_Items.GetSelectedWeapon()==NULL) return;

	MMatchItemDesc* pSelectedItemDesc = m_Items.GetSelectedWeapon()->GetDesc();

	if (pSelectedItemDesc==NULL) {
		m_Items.SelectWeapon(BackupParts);
		mlog("선택된 무기의 데이터가 없다.\n");
		mlog("ZCharacter 무기상태와 RVisualMesh 의 무기상태가 틀려졌다\n");
		return;
	}

	OnChangeWeapon(pSelectedItemDesc);

	if(nParts!=MMCIP_MELEE)
		m_bCharged = false;
}

//#define _CHECKVALIDSHOTLOG


bool ZCharacter::CheckValidShotTime(int nItemID, float fTime, ZItem* pItem)
{
#ifdef _CHECKVALIDSHOTLOG
	char szTime[32]; _strtime(szTime);
	char szLog[256];
#endif

	if (pItem->GetDesc() == nullptr)
		return false;

	if (GetLastShotItemID() == nItemID) {

	// if time passed is less than item delay
		if (fTime - GetLastShotTime() < (float)pItem->GetDesc()->m_nDelay/1000.0f) {
			MMatchWeaponType nWeaponType = pItem->GetDesc()->m_nWeaponType;
			if ( (MWT_DAGGER <= nWeaponType && nWeaponType <= MWT_DOUBLE_KATANA) &&
				(fTime - GetLastShotTime() >= 0.23f) ) 
			{
				// continue Valid... (칼질 정확한 시간측정이 어려워 매직넘버사용.
			} else if ( (nWeaponType==MWT_DOUBLE_KATANA || nWeaponType==MWT_DUAL_DAGGER) &&
				(fTime - GetLastShotTime() >= 0.11f) ) 
			{
				// continue Valid... (칼질 정확한 시간측정이 어려워 매직넘버사용.
			} else {
#ifdef _CHECKVALIDSHOTLOG
				sprintf_safe(szLog, "IGNORE>> [%s] (%u:%u) Interval(%0.2f) Delay(%0.2f) \n", 
					szTime, GetUID().High, GetUID().Low, fTime - GetLastShotTime(), (float)pItem->GetDesc()->m_nDelay/1000.0f);
				OutputDebugString(szLog);	
#endif
				return false;
			}
		}
	}

#ifdef _CHECKVALIDSHOTLOG
	sprintf_safe(szLog, "[%s] (%u:%u) %u(%f)\n", 
			szTime, GetUID().High, GetUID().Low, nItemID, fTime);
	OutputDebugString(szLog);
#endif

	return true;
}

bool ZCharacter::IsObserverTarget()
{
	if (ZApplication::GetGameInterface()->GetCombatInterface()->GetObserver()->GetTargetCharacter() == this)
	{
		return true;
	}

	return false;
}

void ZCharacter::OnDamagedAnimation(ZObject *pAttacker,int type)
{
	if(pAttacker==NULL)
		return;

	if(!m_bBlastDrop)
	{
		rvector dir = m_Position-pAttacker->m_Position;
		Normalize(dir);

		m_bStun = true;
		SetVelocity(0,0,0);

		float fRatio = GetMoveSpeedRatio();

		if(type==SEM_WomanSlash5 || type==SEM_ManSlash5)
		{
			AddVelocity( dir * MAX_SPEED * fRatio );
			m_nStunType = ZST_SLASH;

			ZCharacterObject* pCObj = MDynamicCast(ZCharacterObject, pAttacker);

			if(pCObj) {
				ZC_ENCHANT etype = pCObj->GetEnchantType();
				if( etype == ZC_ENCHANT_LIGHTNING )
					m_nStunType = ZST_LIGHTNING;
			}
		} else {
			AddVelocity( dir * RUN_SPEED * fRatio );
			m_nStunType = (ZSTUNTYPE)((type) %2);
			if(type<=SEM_ManSlash4)
				m_nStunType=(ZSTUNTYPE)(1-m_nStunType);
		}
		
		m_bPlayDone = false;
	}
}

void ZCharacter::ActDead()
{
	if (m_bInitialized==false)	return;
	if (!IsVisible())			return;

	rvector vDir = m_LastDamageDir;
	vDir.z = 0.f;
	Normalize(vDir);
	vDir.z = 0.6f;
	Normalize(vDir);

	float fForce = 1.f;

	bool bKnockBack = false;

	SetDeadTime(g_pGame->GetTime());

	if ((GetStateLower() != ZC_STATE_LOWER_DIE1) && 
		(GetStateLower() != ZC_STATE_LOWER_DIE2) && 
		(GetStateLower() != ZC_STATE_LOWER_DIE3) && 
		(GetStateLower() != ZC_STATE_LOWER_DIE4) )
	{

		ZC_STATE_LOWER lower_motion;

		float dot = m_LastDamageDot;

		switch(m_LastDamageWeapon) {
		// melee
		case MWT_DAGGER:
		case MWT_DUAL_DAGGER: 
		case MWT_KATANA:
		case MWT_GREAT_SWORD:
		case MWT_DOUBLE_KATANA:
			bKnockBack = false;
			break;
		case MWT_PISTOL:
		case MWT_PISTOLx2:
		case MWT_REVOLVER:
		case MWT_REVOLVERx2:
		case MWT_SMG:
		case MWT_SMGx2:
		case MWT_RIFLE:
		case MWT_SNIFER:
			if( m_LastDamageDistance < 800.f )
			{
				// 400 ~ 900
				fForce = 300 + (1.f-(m_LastDamageDistance/800.f)) * 500.f;
				bKnockBack = true;
			}
			break;
		case MWT_SHOTGUN:
		case MWT_SAWED_SHOTGUN:
		case MWT_MACHINEGUN:
			if( m_LastDamageDistance < 1000.f )
			{
				// 500 ~ 1000
				fForce = 400 + (1.f-(m_LastDamageDistance/1000.f)) * 500.f;

				bKnockBack = true;
			}

			break;

		case MWT_ROCKET:
		case MWT_FRAGMENTATION:
			fForce = 600.f;
			bKnockBack = true;

			break;

		default:
			lower_motion = ZC_STATE_LOWER_DIE1;

		}

		if(m_LastDamageType == ZD_BULLET_HEADSHOT) {
			bKnockBack = true;
			fForce = 700.f;
		}

		if(bKnockBack) {
			ZObject::OnKnockback(vDir, fForce );
		}

		if(bKnockBack) {

			if(dot<0)	lower_motion = ZC_STATE_LOWER_DIE3;
			else		lower_motion = ZC_STATE_LOWER_DIE4;
		}
		else {

			if(dot<0)	lower_motion = ZC_STATE_LOWER_DIE1;
			else		lower_motion = ZC_STATE_LOWER_DIE2;
		}

		if (GetPosition().z <= DIE_CRITICAL_LINE)
		{
			lower_motion = ZC_STATE_PIT;
			m_bFallingToNarak = true;
		}
		SetAnimationLower(lower_motion);
	}

	if (GetStateUpper() != ZC_STATE_UPPER_NONE )
	{
		SetAnimationUpper(ZC_STATE_UPPER_NONE);
	}

	// excellent
#define EXCELLENT_TIME	3.0f
	ZCharacter *pLastAttacker = ZGetCharacterManager()->Find(GetLastAttacker());
	if(pLastAttacker && pLastAttacker!=this)
	{		
		if(g_pGame->GetTime()-pLastAttacker->m_fLastKillTime < EXCELLENT_TIME && ZApplication::GetGame()->GetMatch()->GetMatchType() != MMATCH_GAMETYPE_DUEL)
		{
			pLastAttacker->GetStatus()->nExcellent++;
			pLastAttacker->AddIcon(ZCI_EXCELLENT);
		}

		pLastAttacker->m_fLastKillTime=g_pGame->GetTime();		


		// fantastic
		if(!m_bLand && GetDistToFloor()>200.f && ZApplication::GetGame()->GetMatch()->GetMatchType() != MMATCH_GAMETYPE_DUEL)
		{
			pLastAttacker->GetStatus()->nFantastic++;
			pLastAttacker->AddIcon(ZCI_FANTASTIC);
		}

		// unbelievable
		if(pLastAttacker && ZApplication::GetGame()->GetMatch()->GetMatchType() != MMATCH_GAMETYPE_DUEL)
		{
			pLastAttacker->m_nKillsThisRound++;
			if(pLastAttacker->m_nKillsThisRound==3)
				pLastAttacker->GetStatus()->nUnbelievable++;
			if(pLastAttacker->m_nKillsThisRound>=3)
			{
				pLastAttacker->AddIcon(ZCI_UNBELIEVABLE);
			}
		}
	}
}

void ZCharacter::AddIcon(int nIcon)
{
	if(nIcon<0 || nIcon>=5) return;

	ZGetEffectManager()->AddCharacterIcon(this,nIcon);

	ZCharacter *pTargetCharacter = ZGetGameInterface()->GetCombatInterface()->GetTargetCharacter();
	if(pTargetCharacter == this)
	{
		ZGetScreenEffectManager()->AddPraise(nIcon);
	}
}

void ZCharacter::ToggleClothSimulation()
{
	if(!m_pVMesh) return;

	if( Enable_Cloth )
		m_pVMesh->ChangeChestCloth(1.f,1);
	else
		m_pVMesh->DestroyCloth();
}

void ZCharacter::Save(ReplayPlayerInfo& rpi)
{
	rpi.IsHero = IsHero();
	rpi.Info = m_InitialInfo;
	rpi.State.UID = m_UID;
	rpi.State.Property = m_Property;
	rpi.State.HP = m_pModule_HPAP->GetHP();
	rpi.State.AP = m_pModule_HPAP->GetAP();
	rpi.State.Status = m_Status;
	m_Items.Save(rpi.State.BulletInfos);
	rpi.State.Position = m_Position;
	rpi.State.Direction = m_Direction;
	rpi.State.Team = m_nTeamID;
	rpi.State.Dead = m_bDie;
	rpi.State.HidingAdmin = m_bAdminHide;
	rpi.State.LowerAnimation = u8(m_AniState_Lower);
	rpi.State.UpperAnimation = u8(m_AniState_Upper);
	rpi.State.SelectedItem = u8(m_Items.GetSelectedWeaponParts());
}

void ZCharacter::Load(const ReplayPlayerInfo& rpi)
{
	m_bHero = rpi.IsHero;
	m_InitialInfo = rpi.Info;
	m_UID = rpi.State.UID;
	m_Property = rpi.State.Property;
	m_pModule_HPAP->SetHP(rpi.State.HP);
	m_pModule_HPAP->SetAP(rpi.State.AP);
	m_Status = rpi.State.Status;
	m_Items.Load(rpi.State.BulletInfos);
	m_Position = rpi.State.Position;
	m_Direction = rpi.State.Direction;
	m_nTeamID = rpi.State.Team;
	m_bDie = rpi.State.Dead;
	m_bAdminHide = rpi.State.HidingAdmin;
	if (rpi.State.LowerAnimation != u8(-1))
	{
		// Change weapon first so that the SetAnimation* calls override the animations that
		// ChangeWeapon set (NONE and LOAD).
		ChangeWeapon(MMatchCharItemParts(rpi.State.SelectedItem));
		SetAnimationLower(ZC_STATE_LOWER(rpi.State.LowerAnimation));
		SetAnimationUpper(ZC_STATE_UPPER(rpi.State.UpperAnimation));
	}
}

void ZCharacter::OnLevelDown()
{
	m_Property.nLevel--;
}
void ZCharacter::OnLevelUp()
{
	m_Property.nLevel++;
	ZGetEffectManager()->AddLevelUpEffect(this);
}

void ZCharacter::LevelUp()
{
	OnLevelUp();
}
void ZCharacter::LevelDown()
{
	OnLevelDown();
}


RMesh *ZCharacter::GetWeaponMesh(MMatchCharItemParts parts)
{
	ZItem *pWeapon = m_Items.GetItem(parts);

	if(!pWeapon) return NULL;

	if( pWeapon->GetDesc()==NULL ) return NULL;

	RMesh* pMesh = ZGetWeaponMeshMgr()->Get( pWeapon->GetDesc()->m_szMeshName );
	return pMesh;
}

bool ZCharacter::IsRunWall()
{
	ZC_STATE_LOWER s = m_AniState_Lower;

	if( ( s == ZC_STATE_LOWER_RUN_WALL_LEFT ) || 
		( s == ZC_STATE_LOWER_RUN_WALL_LEFT_DOWN ) || 
		( s == ZC_STATE_LOWER_RUN_WALL ) || 
		( s == ZC_STATE_LOWER_RUN_WALL_DOWN_FORWARD ) || 
		( s == ZC_STATE_LOWER_RUN_WALL_DOWN ) || 
		( s == ZC_STATE_LOWER_RUN_WALL_RIGHT ) || 
		( s == ZC_STATE_LOWER_RUN_WALL_RIGHT_DOWN ) ||
		( s == ZC_STATE_LOWER_JUMP_WALL_FORWARD ) ||
		( s == ZC_STATE_LOWER_JUMP_WALL_BACK ) ||
		( s == ZC_STATE_LOWER_JUMP_WALL_LEFT ) ||
		( s == ZC_STATE_LOWER_JUMP_WALL_RIGHT ) ) {
		return true;
	}
	return false;
}

bool ZCharacter::IsMeleeWeapon()
{
	ZItem* pItem = m_Items.GetSelectedWeapon();

	if(pItem) {
		if(pItem->GetDesc()) {
			if(pItem->GetDesc()->m_nType == MMIT_MELEE) {		
				return true;
			}
		}
	}

	return false;
}

bool ZCharacter::IsCollideable()
{
	if (m_Collision.bCollideable)
	{
		return ((!IsDead() && !m_bBlastDrop));
	}

	return m_Collision.bCollideable;
}

bool ZCharacter::IsAttackable()
{
	if (IsDead()) return false;
	return true;
}

float ZCharacter::ColTest(const rvector& pos, const rvector& vec, float radius, rplane* out)
{
	return SweepTest(rsphere(pos, radius), vec, rsphere(m_Position, CHARACTER_COLLISION_DIST), out);
}

bool ZCharacter::IsGuardNonrecoilable() const
{
#ifndef GUARD_CANCEL_FIX
	return IsGuardRecoilable();
#else
#ifdef GUARD_START_CAN_BLOCK
	if (m_AniState_Lower == ZC_STATE_LOWER_GUARD_START ||
		m_AniState_Upper == ZC_STATE_UPPER_GUARD_START)
		return true;
#endif
	return ((ZC_STATE_LOWER_GUARD_IDLE<=m_AniState_Lower && m_AniState_Lower<=ZC_STATE_LOWER_GUARD_CANCEL) ||
		(ZC_STATE_UPPER_GUARD_IDLE<=m_AniState_Upper && m_AniState_Upper<=ZC_STATE_LOWER_GUARD_CANCEL));
#endif
}

bool ZCharacter::IsGuardRecoilable() const
{
	return ((ZC_STATE_LOWER_GUARD_IDLE<=m_AniState_Lower && m_AniState_Lower<=ZC_STATE_LOWER_GUARD_BLOCK2) ||
		(ZC_STATE_UPPER_GUARD_IDLE<=m_AniState_Upper && m_AniState_Upper<=ZC_STATE_UPPER_GUARD_BLOCK2));
}

void ZCharacter::AddMassiveEffect(const rvector &pos, const rvector &dir)
{
	ZGetEffectManager()->AddSwordWaveEffect(GetUID(), pos, dir);
}

void ZCharacter::InitRound()
{
	if(GetUserGrade()==MMUG_STAR) {
		ZGetEffectManager()->AddStarEffect(this);
	}

	if (m_bChatEffect) {
		ZGetEffectManager()->AddChatIcon(this);
	}
}

ZOBJECTHITTEST ZCharacter::HitTest(const rvector& origin, const rvector& to,float fTime,rvector *pOutPos)
{
	v3 Head, Foot;
	GetPositions(&Head, &Foot, fTime);
	return PlayerHitTest(Head, Foot, origin, to, pOutPos);
}

void ZCharacter::OnDamaged(ZObject* pAttacker, rvector srcPos, ZDAMAGETYPE damageType, MMatchWeaponType weaponType, float fDamage, float fPiercingRatio, int nMeleeType)
{
	if (m_bInitialized==false) return;
	if (!IsVisible() || IsDead()) return;

	// If this isn't called on MyCharacter, it's unreliable predicted damage.
	// Actual damage for other players is reported in HP/AP info packets.
	//if (this != ZGetGame()->m_pMyCharacter)
		//return;

	HandleDamage(pAttacker, srcPos, damageType, weaponType, fDamage, fPiercingRatio, nMeleeType);
}

void ZCharacter::HandleDamage(ZObject* pAttacker, rvector srcPos, ZDAMAGETYPE damageType, MMatchWeaponType weaponType, float fDamage, float fPiercingRatio, int nMeleeType)
{
	bool bCanAttack = g_pGame->IsAttackable(pAttacker, this)
		|| (pAttacker == this && (damageType == ZD_EXPLOSION || damageType == ZD_FALLING));

	if (damageType != ZD_FALLING)
		bCanAttack &= !isInvincible();

	rvector dir = GetPosition() - srcPos;
	Normalize(dir);

	m_LastDamageDir = dir;
	m_LastDamageType = damageType;
	m_LastDamageWeapon = weaponType;
	m_LastDamageDot = DotProduct(m_Direction, dir);
	m_LastDamageDistance = Magnitude(GetPosition() - srcPos);

	if (bCanAttack)
		ZObject::OnDamaged(pAttacker, srcPos, damageType, weaponType, fDamage, fPiercingRatio, nMeleeType);

	if (damageType == ZD_MELEE) OnDamagedAnimation(pAttacker, nMeleeType);

	m_bDamaged = true;
}

void ZCharacter::OnScream()
{
	if(GetProperty()->nSex==MMS_MALE)
		ZGetSoundEngine()->PlaySound("ooh_male",m_Position,IsObserverTarget());
	else			
		ZGetSoundEngine()->PlaySound("ooh_female",m_Position,IsObserverTarget());
}

void ZCharacter::UpdateTimeOffset(float PeerTime, float LocalTime)
{
	float fCurrentLocalTime = m_fTimeOffset + LocalTime;

	float fTimeError = PeerTime - fCurrentLocalTime;
	if (fabs(fTimeError)>3.f) {
		m_fTimeOffset = PeerTime - LocalTime;
		m_fAccumulatedTimeError = 0;
		m_nTimeErrorCount = 0;
	}
	else
	{
		m_fAccumulatedTimeError += fTimeError;
		m_nTimeErrorCount++;
		if (m_nTimeErrorCount > 10) {
			m_fTimeOffset += 0.5f * m_fAccumulatedTimeError / 10.f;
			m_fAccumulatedTimeError = 0;
			m_nTimeErrorCount = 0;
		}
	}

	m_fLastReceivedTime = LocalTime;
}

void ZCharacter::SetNetPosition(const rvector & position, const rvector & velocity, const rvector & dir)
{
	if (Magnitude(position - m_Position) > 20.0f)
		m_Position = position;
	SetVelocity(velocity);
	SetAccel({ 0, 0, 0 });

	m_TargetDir = dir;
}

void ZCharacter::OnMeleeGuardSuccess()
{
	ZGetSoundEngine()->PlaySound("fx_guard",m_Position,IsObserverTarget());
}

void ZCharacter::OnShot()
{
	if (m_bChatEffect) m_bChatEffect = false;
}

bool ZCharacter::isInvincible()
{
	return ((int)GetGlobalTimeMS() < (m_dwInvincibleStartTime + m_dwInvincibleDuration));
}