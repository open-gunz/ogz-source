/*
last modify : 정동섭 @ 2006/3/16
desc : 무기 사용 키 커스터마이즈 관련
*/

#include "stdafx.h"
#include "ZGameInterface.h"
#include "ZMyCharacter.h"
#include "ZSoundEngine.h"
#include "ZApplication.h"
#include "ZPost.h"
#include "ZActionDef.h"
#include "MProfiler.h"
#include "ZScreenEffectManager.h"
#include "ZConfiguration.h"
#include "ZGameAction.h"
#include "ZModule_HPAP.h"
#include "MDebug.h"
#include "MMath.h"
#include "ZGameConst.h"
#include "ZInput.h"
#include "HitRegistration.h"
#include "MUtil.h"
#include "RBspObject.h"
#include "ZPickInfo.h"
#include "reinterpret.h"
#include "RGMain.h"

#define CHARGE_SHOT
// Ninja jumping
//#define UNLIMITED_JUMP
// How long you can hold a static block before it goes down
#define GUARD_DURATION		2.f
#define TUMBLE_DELAY_TIME	.5f
#define AIR_MOVE			0.05f
#define JUMP_QUEUE_TIME		0.3f
#define JUMP2_WALL_VELOCITY 300.f
#define JUMP2_VELOCITY		1400.f
#define JUMP_VELOCITY		900.f
#define WALL_JUMP2_SIDE_VELOCITY	700.f
#define WALL_JUMP_VELOCITY	350.f
#define BLAST_VELOCITY		1700.f
#define SHOT_QUEUE_TIME		0.4f
#define COLLISION_POSITION rvector(0,0,60)
#define CAFACTOR_DEC_DELAY_TIME	0.2f

MImplementRTTI(ZMyCharacter, ZCharacter);

ZMyCharacter::ZMyCharacter()
{
	m_fCAFactor = 1.0f;
	m_fElapsedCAFactorTime = 0.0f;

#ifdef _DEBUG
	m_bGuardTest = false;
#endif
	InitStatus();
	m_Position = rvector(0, 0, 500);

	InitRound();

	m_fWallJumpTime = 0.f;
	m_nTumbleDir = 0;
	m_fHangTime = 0.f;
	m_nWallJump2Dir = 0;

	m_fDropTime = 0.f;

	m_bSniferMode = false;

	m_bPlayDone = false;
	m_bPlayDone_upper = false;

	m_bMoveLimit = false;

	m_bReserveDashAttacked = false;
	m_vReserveDashAttackedDir = rvector(0.f, 0.f, 0.f);
	m_fReserveDashAttackedTime = 0.f;
	m_uidReserveDashAttacker = MUID(0, 0);

	m_bGuardKey = false;
	m_bGuardByKey = false;
	m_pModule_HPAP->SetRealDamage(true);

	m_pModule_Movable->Active = false;

	m_Collision.fRadius = CHARACTER_RADIUS;
	m_Collision.fHeight = CHARACTER_HEIGHT;
}

void ZMyCharacter::InitRound()
{
	ZCharacter::InitRound();

	for (int i = 0; i < MMCIP_END; i++)
		m_fNextShotTimeType[i] = 0.f;

	m_bReserveDashAttacked = false;
}

void ZMyCharacter::InitSpawn()
{
	for (int i = 0; i < MMCIP_END; i++)
		m_fNextShotTimeType[i] = 0.f;

	m_bReserveDashAttacked = false;
}

void ZMyCharacter::OnDraw()
{
	if (m_bSniferMode) return;

	ZCharacter::OnDraw();
}

void ZMyCharacter::ProcessInput(float fDelta)
{
	if (ZApplication::GetGame()->GetMatch()->GetRoundState() == MMATCH_ROUNDSTATE_PREPARE) return;
	if (m_bInitialized == false) return;
	if (ZGetGame()->IsReservedSuicide()) return;

	UpdateButtonState();

	ZCombatInterface* pCombatInterface = ZGetGameInterface()->GetCombatInterface();
	if (pCombatInterface && pCombatInterface->IsChatVisible())
	{
		ReleaseLButtonQueue();
		return;
	}

	static float rotatez = 0.f, rotatex = 9 * PI_FLOAT / 10.f;

	m_Accel = rvector(0, 0, 0);

	rvector right;
	rvector forward = RCameraDirection;
	forward.z = 0;
	Normalize(forward);
	CrossProduct(&right, rvector(0, 0, 1), forward);

	if (!IsDie() && !m_bStun && !m_bBlastDrop && !m_bBlastStand)
	{
		bool ButtonPressed = false;

		if (!m_bWallJump && !m_bTumble && !m_bSkill && !m_bMoveLimit &&
			!m_bBlast && !m_bBlastFall && !m_bBlastAirmove && !m_bCharging &&
			!m_bSlash && !m_bJumpSlash && !m_bJumpSlashLanding)
		{
			auto AddAccel = [&](auto& vec)
			{
				m_Accel += vec;
				ButtonPressed = true;
			};
			if (ZIsActionKeyDown(ZACTION_FORWARD) == true)
				AddAccel(forward);
			if (ZIsActionKeyDown(ZACTION_BACK) == true)
				AddAccel(-forward);
			if (ZIsActionKeyDown(ZACTION_RIGHT) == true)
				AddAccel(right);
			if (ZIsActionKeyDown(ZACTION_LEFT) == true)
				AddAccel(-right);
		}

		if (ButtonPressed)
			Normalize(m_Accel);

		m_bBackMoving = ZIsActionKeyDown(ZACTION_BACK);

		float fRatio = GetMoveSpeedRatio();

		if (!m_bLand) {
			if (Magnitude(rvector(GetVelocity().x, GetVelocity().y, 0)) < RUN_SPEED * fRatio)
				m_Accel *= AIR_MOVE;
			else
				m_Accel *= 0;
		}

		bool bWallJump = false;
		int nWallJumpDir = -1;

		if (ZIsActionKeyDown(ZACTION_JUMP) == true)
		{
			if (m_bReleasedJump && !m_bLimitJump)
			{
				m_fLastJumpPressedTime = g_pGame->GetTime();
				m_bJumpQueued = true;
				m_bWallJumpQueued = true;
				m_bReleasedJump = false;
			}
		}
		else
			m_bReleasedJump = true;

		if (m_bJumpQueued && (g_pGame->GetTime() - m_fLastJumpPressedTime > JUMP_QUEUE_TIME))
			m_bJumpQueued = false;

		if (m_bJumpQueued && !m_bTumble && !m_bDrop && !m_bShot && !m_bShotReturn && !m_bSkill &&
			!m_bBlast && !m_bBlastFall && !m_bBlastDrop && !m_bBlastStand && !m_bBlastAirmove &&
			!m_bSlash && !m_bJumpSlash && !m_bJumpSlashLanding)
		{
			if (!m_bWallJump && !m_bGuard && !m_bLimitWall)
			{
				if (m_bWallJumpQueued && DotProduct(GetVelocity(), m_Direction) > 0 &&
					GetVelocity().z > -10.f)
				{
					rvector pickorigin, dir;
					rvector front = m_Direction;
					Normalize(front);

					dir = front;
					dir.z = 0;
					Normalize(dir);

					float fRatio = GetMoveSpeedRatio();

					if (m_bLand && DotProduct(GetVelocity(), dir) > RUN_SPEED * fRatio * .8f)
					{
						pickorigin = m_Position;

						RBSPPICKINFO bpi1, bpi2;
						auto* bsp = ZGetGame()->GetWorld()->GetBsp();
						bool bPicked1 = bsp->Pick(pickorigin + rvector(0, 0, 100), dir, &bpi1);
						bool bPicked2 = bsp->Pick(pickorigin + rvector(0, 0, 180), dir, &bpi2);
						if (bPicked1 && bPicked2)
						{
							rvector backdir = -dir;
							float fDist1 = Magnitude(pickorigin + rvector(0, 0, 100) - bpi1.PickPos);
							float fDist2 = Magnitude(pickorigin + rvector(0, 0, 180) - bpi2.PickPos);

							if (fDist1 < 120 &&
								DotPlaneNormal(bpi1.pInfo->plane, backdir) > cos(10.f / 180.f * PI_FLOAT) &&
								fDist2 < 120 &&
								DotPlaneNormal(bpi2.pInfo->plane, backdir) > cos(10.f / 180.f * PI_FLOAT))
							{
								bWallJump = true;
								nWallJumpDir = 1;
							}
						}
					}

					auto right = Normalized(CrossProduct({ 0, 0, 1 }, front));

					pickorigin = m_Position + rvector(0, 0, 150);

					for (int i = 0; i < 2; i++)
					{
						dir = (i == 0) ? -right : right;

						RBSPPICKINFO bpi;
						bool bPicked = ZGetGame()->GetWorld()->GetBsp()->Pick(pickorigin, dir, &bpi);
						if (bPicked)
						{
							rvector backdir = -dir;
							float fDist = Magnitude(pickorigin - bpi.PickPos);
							rvector normal = rvector(bpi.pInfo->plane.a, bpi.pInfo->plane.b, bpi.pInfo->plane.c);
							float fDot = DotProduct(normal, backdir);

							rvector wallright, jumpdir;
							CrossProduct(&wallright, rvector(0, 0, 1), normal);
							jumpdir = (i == 0) ? -wallright : wallright;

							float fRatio = GetMoveSpeedRatio();

							if (fDist < 100.f &&
								DotProduct(GetVelocity(), jumpdir) > RUN_SPEED * fRatio *.8f &&
								fDot > cos(55.f / 180.f*PI_FLOAT) &&
								fDot < cos(25.f / 180.f*PI_FLOAT) &&
								DotProduct(jumpdir, dir) < 0)
							{
								rvector neworigin = pickorigin + 300.f*jumpdir;

								RBSPPICKINFO bpi;
								bPicked = ZGetGame()->GetWorld()->GetBsp()->Pick(neworigin, dir, &bpi);
								if (bPicked && fabsf(Magnitude(bpi.PickPos - neworigin) - fDist) < 10)
								{
									rvector targetpos = pickorigin + 300.f*jumpdir;
									bool bAdjusted = ZGetGame()->GetWorld()->GetBsp()->CheckWall(pickorigin,
										targetpos, CHARACTER_RADIUS - 5, 60);
									if (!bAdjusted)
									{
										bWallJump = true;
										nWallJumpDir = (i == 0) ? 0 : 2;

										SetTargetDir(jumpdir);
										float speed = Magnitude(GetVelocity());
										SetVelocity(jumpdir*speed);
									}
								}
							}
						}
					}
				}
			}

			rvector PickedNormal = rvector(0, 0, 0);
			rvector pickorigin = m_Position + rvector(0, 0, 90);
			RBSPPICKINFO bpi;
			auto& Vel = GetVelocity();
			auto VelLengthSquared = MagnitudeSq(Vel);
			if (VelLengthSquared != 0 &&
				ZGetGame()->GetWorld()->GetBsp()->Pick(pickorigin, Vel / sqrt(VelLengthSquared), &bpi))
				PickedNormal = rvector(bpi.pInfo->plane.a, bpi.pInfo->plane.b, bpi.pInfo->plane.c);

			float fDotJump2 = DotProduct(PickedNormal, m_Direction);
			float fProjSpeed = DotProduct(PickedNormal, GetVelocity());

			rvector characterright;
			CrossProduct(&characterright, m_Direction, rvector(0, 0, 1));
			float fDotJump2right = DotProduct(PickedNormal, characterright);

			bool bJump2 = (g_pGame->GetTime() - m_pModule_Movable->GetAdjustedTime() < 0.2f
				&& Magnitude(pickorigin - bpi.PickPos) < 100.f
				&& DotProduct(rvector(0, 0, -1), PickedNormal) < 0.1f
				&& (fDotJump2 > .8f || fDotJump2<-.8f || fDotJump2right>.8f || fDotJump2right<-.8f)
				&& g_pGame->GetTime() - m_fJump2Time>.5f)
				&& fProjSpeed < -100.f
				&& GetDistToFloor() > 30.f
				&& !m_bWallJump && !bWallJump && !m_bLand && !m_bGuard;

			if (m_bWallJump && !m_bWallJump2 && (m_fWallJumpTime + .4f < g_pGame->GetTime()))
			{
				WallJump2();
			}
			else if (bWallJump)
			{
				m_bJumpQueued = false;
				m_bWallJump = true;
				m_nWallJumpDir = nWallJumpDir;
				m_fWallJumpTime = g_pGame->GetTime();
				m_bWallJump2 = false;
				m_bWallHang = false;

				if (nWallJumpDir == 1)
				{
					SetVelocity(0, 0, 0);
				}
				else
				{
					if (m_bLand)
						AddVelocity(rvector(0, 0, WALL_JUMP_VELOCITY));
				}
				m_bLand = false;

			}
			else if (bJump2)
			{
				m_bJumpQueued = false;

				Normalize(PickedNormal);
				float fAbsorb = DotProduct(PickedNormal, GetVelocity());
				rvector newVelocity = GetVelocity();
				newVelocity -= fAbsorb*PickedNormal;

				newVelocity *= 1.1f;

				newVelocity += PickedNormal*JUMP2_WALL_VELOCITY;
				newVelocity.z = JUMP2_VELOCITY;
				SetVelocity(newVelocity);

				m_fJump2Time = g_pGame->GetTime();

				m_bLand = false;
				m_bWallHang = false;

				if (!m_bJumpShot)
					m_bWallJump2 = true;
				m_bPlayDone = false;
				if (fDotJump2 > .8f)
					m_nWallJump2Dir = 0;
				else
					if (fDotJump2 < -.8f)
						m_nWallJump2Dir = 1;
					else
						if (fDotJump2right < -.8f)
							m_nWallJump2Dir = 2;
						else
							if (fDotJump2right > .8f)
								m_nWallJump2Dir = 3;

				if (GetStateUpper() == ZC_STATE_UPPER_SHOT)
					SetAnimationUpper(ZC_STATE_UPPER_NONE);

			}
			else if (m_bWallHang)
			{
				if (m_bHangSuccess)
				{
					m_bJumpQueued = false;

					rvector PickedNormal = -m_Direction; PickedNormal.z = 0;

					if (ZIsActionKeyDown(ZACTION_FORWARD))
					{
						m_nWallJump2Dir = 6;
						SetVelocity(PickedNormal*50.f);
					}
					else
					{
						m_nWallJump2Dir = 7;
						AddVelocity(PickedNormal*300.f);
					}
					AddVelocity(rvector(0, 0, 1400));

					m_fJump2Time = g_pGame->GetTime();

					m_bLand = false;
					m_bWallHang = false;

					m_bWallJump2 = true;
					m_bPlayDone = false;
				}
			}
			else
#ifndef UNLIMITED_JUMP
				if (m_bLand)
#endif
				{
					m_bJumpQueued = false;

					rvector right;
					rvector forward = m_Direction;
					CrossProduct(&right, rvector(0, 0, 1), forward);

					rvector vel = rvector(GetVelocity().x, GetVelocity().y, 0);
					float fVel = Magnitude(vel);

					rvector newVelocity = vel*0.5f + m_Accel*fVel*.45f + rvector(0, 0, JUMP_VELOCITY);
					SetVelocity(newVelocity);
					m_bLand = false;

					m_fJump2Time = g_pGame->GetTime();
				}
		}
	}

	m_Accel *= 7000.f * fDelta;
	AddVelocity(m_Accel);

	ProcessGuard();
	ProcessShot();
	ProcessGadget();
}

void ZMyCharacter::OnShotMelee()
{
	if (m_bSkill || m_bStun || m_bShotReturn || m_bJumpShot || m_bGuard ||
		m_bSlash || m_bJumpSlash || m_bJumpSlashLanding) return;

	if (m_bShot && !m_bPlayDone)
		return;

	if (m_pVMesh->m_SelectWeaponMotionType == eq_wd_dagger ||
		m_pVMesh->m_SelectWeaponMotionType == eq_ws_dagger) { // dagger

		if (GetStateUpper() == ZC_STATE_UPPER_SHOT)
			return;

		if (m_bCharged)
		{
			if (GetVelocity().z > 100.f || GetDistToFloor() > 80.f)
				JumpChargedShot();
			else
				ChargedShot();
			return;
		}

		SetAnimationUpper(ZC_STATE_UPPER_SHOT);
		m_bWallJump = false;
		m_bWallJump2 = false;
#ifdef CHARGE_SHOT
		m_bEnterCharge = true;
#endif
	}
	else {
		if (!m_bShot)
			m_nShot = 0;

		int nShotCount = 4;

		if (m_nShot == nShotCount) {
			return;
		}

		if (!m_bJumpShot && (GetVelocity().z > 100.f || GetDistToFloor() > 80.f))
		{
			if (m_bCharged)
			{
				JumpChargedShot();
				return;
			}

			m_bShot = false;
			m_bShotReturn = false;
			m_bWallJump = false;
			m_bWallJump2 = false;
			m_bJumpShot = true;
			m_bPlayDone = false;

			if (IsRunWall() && IsMeleeWeapon()) {
				m_nJumpShotType = 1;
			}
			else {
				m_nJumpShotType = 0;
			}

			m_fNextShotTimeType[MMCIP_MELEE] = g_pGame->GetTime() + 0.8f;

		}
		else {
			if (!m_bLand && GetDistToFloor() > 20.f) {
				if (m_nShot < 2) {
					ReleaseLButtonQueue();
					return;
				}
			}

			if (m_bCharged) {
				ChargedShot();
				return;
			}

			m_bShot = true;
			m_bShotReturn = false;
			m_bPlayDone = false;

#ifdef CHARGE_SHOT
			if (m_nShot == 0)
				m_bEnterCharge = true;
#endif

			m_nShot = m_nShot % nShotCount + 1;
		}
	}

	m_bTumble = false;
	ReleaseLButtonQueue();

	int sel_type = GetItems()->GetSelectedWeaponParts();

	if (m_nShot == 1 && !m_bJumpShot) {
		m_b1ShotSended = false;
		m_f1ShotTime = g_pGame->GetTime();
	}
	else {
		if (ZGetGameClient()->GetMatchStageSetting()->IsVanillaMode())
		{
			ZPostShotMelee(g_pGame->GetTime(), m_Position, m_nShot);
			AddDelayedWork(g_pGame->GetTime() + 0.2f, ZDW_RECOIL);
		}
		else
		{
			AddDelayedWork(g_pGame->GetTime() + 0.1 + max(m_nShot - 1, 0) * .05,
				ZDW_RG_SLASH, reinterpret_cast<void *>(m_nShot));
		}
	}
}

void ZMyCharacter::ReleaseLButtonQueue()
{
	m_bLButtonQueued = false;
}

void ZMyCharacter::OnShotRange()
{
	ZItem* pSelectedItem = GetItems()->GetSelectedWeapon();

	if (!pSelectedItem)
	{
		mlog("ZMyCharacter::OnShotRange -- pSelectedItem is null\n");
		return;
	}

	if (pSelectedItem->GetDesc()->m_nWeaponType == MWT_SNIFER && !m_bSniferMode)
		return;

	if (pSelectedItem->GetBulletAMagazine() <= 0)
		return;

	auto Lower = GetStateLower();
	if (Lower != ZC_STATE_LOWER_TUMBLE_FORWARD &&
		Lower != ZC_STATE_LOWER_TUMBLE_BACK &&
		Lower != ZC_STATE_LOWER_TUMBLE_RIGHT &&
		Lower != ZC_STATE_LOWER_TUMBLE_LEFT)
		SetAnimationUpper(ZC_STATE_UPPER_SHOT);

	MPOINT Cp = MPOINT(0, 0);
	ZPICKINFO zpi;
	rvector pickpos;

	ZCombatInterface* ci = ZApplication::GetGameInterface()->GetCombatInterface();

	if (ci) {

		Cp = ci->GetCrosshairPoint();

		rvector pos, dir;
		RGetScreenLine(Cp.x, Cp.y, &pos, &dir);

		rvector mypos = m_Position + rvector(0, 0, 100);
		rplane myplane = PlaneFromPointNormal(mypos, dir);

		rvector checkpos, checkposto = pos + 10000.f*dir;
		IntersectLineSegmentPlane(&checkpos, myplane, pos, checkposto);

		if (g_pGame->Pick(this, checkpos, dir, &zpi, RM_FLAG_ADDITIVE | RM_FLAG_HIDE | RM_FLAG_PASSBULLET))
		{
			if (zpi.bBspPicked)
				pickpos = zpi.bpi.PickPos;
			else
				if (zpi.pObject)
					pickpos = zpi.info.vOut;
		}
		else
		{
			pickpos = pos + dir*10000.f;
		}
	}

	bool skip_controllability = false;

	rvector vWeaponPos;
	rvector dir;

	if (CheckWall(vWeaponPos))
		skip_controllability = true;

	dir = pickpos - vWeaponPos;
	Normalize(dir);

	MMatchWeaponType wtype = MWT_NONE;

	if (pSelectedItem->GetDesc())
		wtype = pSelectedItem->GetDesc()->m_nWeaponType;

	u32 seed = reinterpret<u32>(g_pGame->GetTime());

	if (wtype == MWT_SHOTGUN || wtype == MWT_SAWED_SHOTGUN)
	{
		int sel_type = GetItems()->GetSelectedWeaponParts();

		rvector vTo = pickpos;

		rvector nPos = m_pVMesh->GetBipTypePosition(eq_parts_pos_info_Spine1);
		rvector nDir = pickpos - nPos;
		Normalize(nDir);

		if (skip_controllability || (vWeaponPos.z < m_Position.z + 20.f))
		{
			vWeaponPos = nPos + 30.f * nDir;
			vTo = nPos + 10000.f * nDir;
		}

		ZPostShot(g_pGame->GetTime(), vWeaponPos, vTo, sel_type);
	}
	else
	{
		rvector r;

		if (pSelectedItem->GetDesc() != NULL && !skip_controllability)
		{
			CalcRangeShotControllability(r, dir, pSelectedItem->GetDesc()->m_nControllability, seed);
		}
		else
		{
			r = dir;
		}

		int sel_type = GetItems()->GetSelectedWeaponParts();

		ZPostShot(g_pGame->GetTime(), vWeaponPos, vWeaponPos + 10000.f*r, sel_type);
	}
}

void ZMyCharacter::CalcRangeShotControllability(rvector& vOutDir, const rvector& vSrcDir, int nControllability, u32 Seed)
{
	::CalcRangeShotControllability(vOutDir, vSrcDir, nControllability, Seed, m_fCAFactor);
	IncreaseCAFactor();
}

void ZMyCharacter::IncreaseCAFactor()
{
	m_fElapsedCAFactorTime = 0.0f;
	m_fCAFactor += GetControllabilityFactor();
	if (m_fCAFactor > 1.0f) m_fCAFactor = 1.0f;
}

void ZMyCharacter::OnShotItem()
{
	ZItem* pSelectedItem = GetItems()->GetSelectedWeapon();

	if (!pSelectedItem || pSelectedItem->GetBulletAMagazine() <= 0) {
		return;
	}

	SetAnimationUpper(ZC_STATE_UPPER_SHOT);

	int type = ZC_WEAPON_SP_ITEMKIT;

	rvector pos = rvector(0.0f, 0.0f, 0.0f);
	int sel_type = GetItems()->GetSelectedWeaponParts();

	if (pSelectedItem->GetDesc()->m_nWeaponType == MWT_MED_KIT ||
		pSelectedItem->GetDesc()->m_nWeaponType == MWT_REPAIR_KIT ||
		pSelectedItem->GetDesc()->m_nWeaponType == MWT_BULLET_KIT ||
		pSelectedItem->GetDesc()->m_nWeaponType == MWT_FOOD)
	{
		pos = m_Position + (m_Direction * 150);
		pos.z = m_Position.z;

		rvector vWeapon;

		vWeapon = m_Position + rvector(0, 0, 130) + (m_Direction * 50);

		rvector nPos = m_pVMesh->GetBipTypePosition(eq_parts_pos_info_Spine1);
		rvector nDir = vWeapon - nPos;

		Normalize(nDir);

		RBSPPICKINFO bpi;
		if (ZGetWorld()->GetBsp()->Pick(nPos, nDir, &bpi)) {
			if (DotProduct(bpi.pInfo->plane, vWeapon) < 0) {
				vWeapon = bpi.PickPos - nDir;
			}
		}

		pos = vWeapon;

		rvector velocity = (GetVelocity() * 0.5f) + m_TargetDir * 100;
		ZPostShotSp(g_pGame->GetTime(), pos, velocity, ZC_WEAPON_SP_ITEMKIT, sel_type);
		return;
	}

	rvector vWeaponPos;
	CheckWall(vWeaponPos);

	MPOINT Cp = MPOINT(0, 0);
	ZPICKINFO zpi;
	rvector pickpos;

	ZCombatInterface* ci = ZApplication::GetGameInterface()->GetCombatInterface();

	if (ci) {

		Cp = ci->GetCrosshairPoint();

		rvector pos, dir;
		RGetScreenLine(Cp.x, Cp.y, &pos, &dir);

		rvector mypos = m_Position + rvector(0, 0, 100);
		rplane myplane = PlaneFromPointNormal(mypos, dir);

		rvector checkpos, checkposto = pos + 100000.f*dir;
		IntersectLineSegmentPlane(&checkpos, myplane, pos, checkposto);

		if (g_pGame->Pick(this, checkpos, dir, &zpi))
		{
			if (zpi.bBspPicked)
				pickpos = zpi.bpi.PickPos;
			else
				if (zpi.pObject)
					pickpos = zpi.info.vOut;
		}
		else
		{
			pickpos = pos + dir*10000.f;
		}
	}

	rvector v1;
	GetWeaponTypePos(weapon_dummy_muzzle_flash, &v1);

	if (zpi.bBspPicked && DotProduct(zpi.bpi.pInfo->plane, v1) < 0)
		v1 = m_Position + rvector(0, 0, 110);

	rvector dir = pickpos - v1;

	Normalize(dir);
	ZPostShotSp(g_pGame->GetTime(), v1, dir, type, sel_type);
}

void ZMyCharacter::OnShotCustom()
{
	ZItem* pSelectedItem = GetItems()->GetSelectedWeapon();

	if (pSelectedItem == NULL || pSelectedItem->GetBulletAMagazine() <= 0) {
		return;
	}

	MMatchItemDesc* pCustomDesc = pSelectedItem->GetDesc();
	DWORD nWeaponDelay = pCustomDesc->m_nDelay;

	SetAnimationUpper(ZC_STATE_UPPER_SHOT);

	if (m_pVMesh->IsSelectWeaponGrenade()) {
		m_pVMesh->m_bGrenadeFire = true;
		m_pVMesh->m_GrenadeFireTime = GetGlobalTimeMS();
	}
}

bool ZMyCharacter::CheckWall(rvector& Pos)
{
	rvector vWPos;
	GetWeaponTypePos(weapon_dummy_muzzle_flash, &vWPos);

	rvector nPos = m_pVMesh->GetBipTypePosition(eq_parts_pos_info_Spine1);
	rvector nDir = vWPos - nPos;
	Normalize(nDir);

	RBSPPICKINFO bpi;

	memset(&bpi, 0, sizeof(RBSPPICKINFO));

	if (ZGetGame()->GetWorld()->GetBsp()->Pick(nPos, nDir, &bpi)) {
		if (bpi.pInfo) {
			if (DotProduct(bpi.pInfo->plane, vWPos) < 0) {
				Pos = bpi.PickPos - 2.f*nDir;
				return true;
			}
		}
	}

	Pos = vWPos;

	return false;
}


void ZMyCharacter::OnShotRocket()
{
	ZItem* pSelectedItem = GetItems()->GetSelectedWeapon();

	if (pSelectedItem == NULL || pSelectedItem->GetBulletAMagazine() <= 0) {

		return;
	}

	MPOINT Cp = MPOINT(0, 0);
	RBSPPICKINFO bpi;

	memset(&bpi, 0, sizeof(RBSPPICKINFO));

	ZCombatInterface* ci = ZApplication::GetGameInterface()->GetCombatInterface();

	if (ci) {

		Cp = ci->GetCrosshairPoint();

		rvector pos, dir;
		RGetScreenLine(Cp.x, Cp.y, &pos, &dir);

		if (!ZGetGame()->GetWorld()->GetBsp()->Pick(pos, dir, &bpi)) {
			bpi.PickPos = pos + dir*10000.f;

		}
	}

	rvector vWeaponPos;
	CheckWall(vWeaponPos);

	rvector dir = bpi.PickPos - vWeaponPos;
	Normalize(dir);

	int sel_type = GetItems()->GetSelectedWeaponParts();

	ZPostShotSp(g_pGame->GetTime(), vWeaponPos, dir, ZC_WEAPON_SP_ROCKET, sel_type);
}

void ZMyCharacter::OnGadget_Hanging()
{
	switch (m_pVMesh->m_SelectWeaponMotionType)
	{
	case eq_wd_katana:
	case eq_wd_blade:
	case eq_wd_sword:
	case eq_ws_dagger:
	case eq_wd_dagger:
		break;
	default:
		return;
	};

	if (IsDie() || m_bWallJump || m_bGuard || m_bDrop || m_bTumble || m_bSkill ||
		m_bBlast || m_bBlastFall || m_bBlastDrop || m_bBlastStand || m_bBlastAirmove) return;
	if (GetStateLower() == ZC_STATE_LOWER_JUMPATTACK) return;
	if (m_bWallJump2 && (g_pGame->GetTime() - m_fJump2Time) < .40f) return;

	m_bWallJump2 = false;

	if (!m_bWallHang)
	{
		if (GetDistToFloor() > 100.f) {
			m_fHangTime = g_pGame->GetTime();
			m_bWallHang = true;
			m_bHangSuccess = false;
		}
	}
	else {
		if (g_pGame->GetTime() - m_fHangTime > .4f && !m_bHangSuccess) {
			rvector pickorigin, dir;
			rvector front = m_Direction;
			front.z = 0;
			Normalize(front);
			dir = front;
			pickorigin = m_Position + rvector(0, 0, 210);

			RBSPPICKINFO bpi;
			bool bPicked = ZGetGame()->GetWorld()->GetBsp()->Pick(pickorigin, dir, &bpi);

			if (bPicked && Magnitude(pickorigin - bpi.PickPos) < 100.f)
			{
				m_bHangSuccess = true;
				rplane plane = bpi.pInfo->plane;
				ZGetEffectManager()->AddLightFragment(bpi.PickPos, rvector(plane.a, plane.b, plane.c));

#ifndef _BIRDSOUND
				ZGetSoundEngine()->PlaySound("hangonwall", bpi.PickPos);
#endif
				SetLastThrower(MUID(0, 0), 0.0f);
			}
			else
				m_bHangSuccess = false;
		}
	}
}

void ZMyCharacter::OnGadget_Snifer()
{
	m_bSniferMode = !m_bSniferMode;

	ZCombatInterface* ci = ZApplication::GetGameInterface()->GetCombatInterface();
	if (ci)
	{
		if (m_bSniferMode)
		{
			ci->OnGadget(MWT_SNIFER);
		}
		else
		{
			ci->OnGadgetOff();
		}
	}
}


void ZMyCharacter::ProcessGadget()
{
	if (m_bWallJump || m_bGuard || m_bStun || m_bShot || m_bSkill ||
		m_bSlash || m_bJumpSlash || m_bJumpSlashLanding) return;

	if (GetItems()->GetSelectedWeapon() == NULL) return;

	if (IsDie() || m_bDrop || m_bBlast || m_bBlastDrop || m_bBlastStand)
		return;

	if (GetStateUpper() == ZC_STATE_UPPER_RELOAD || GetStateUpper() == ZC_STATE_UPPER_LOAD)
		return;

	bool bPressingSecondary = ZIsActionKeyDown(ZACTION_USE_WEAPON2);
	bool bSecondary = bPressingSecondary && !bWasPressingSecondaryLastFrame;

	if (bSecondary && m_bLand) {

		MMatchWeaponType type = MWT_NONE;

		int sel_type = GetItems()->GetSelectedWeaponParts();
		ZItem* pSItem = GetItems()->GetSelectedWeapon();

		if (pSItem && pSItem->GetDesc())
			type = pSItem->GetDesc()->m_nWeaponType;
		switch (type) {
		case MWT_DAGGER:
		case MWT_DUAL_DAGGER:
			m_bSkill = true;
			m_fSkillTime = g_pGame->GetTime();
			m_bTumble = false;
			AddDelayedWork(g_pGame->GetTime() + 0.18f, ZDW_DASH);
			break;
		case MWT_KATANA:
		case MWT_DOUBLE_KATANA:
			m_bSkill = true;
			m_fSkillTime = g_pGame->GetTime();
			m_bTumble = false;
			AddDelayedWork(g_pGame->GetTime() + 0.18f, ZDW_UPPERCUT);
			break;
		}
	}

	if (!bPressingSecondary)
	{
		m_bWallHang = false;
		m_bHangSuccess = false;
		bWasPressingSecondaryLastFrame = bPressingSecondary;
		return;
	}

	if (bSecondary)
	{
		ZItem* pSelectedItem = GetItems()->GetSelectedWeapon();
		if (pSelectedItem)
		{
			if (pSelectedItem->GetDesc()) {

				switch (pSelectedItem->GetDesc()->m_nWeaponType)
				{
				case MWT_SNIFER:
				{
					OnGadget_Snifer();
				}
				break;
				}
			}
		}
	}

	if (m_pVMesh)
	{
		if (!m_bLand)
			OnGadget_Hanging();
	}
	else
	{
		assert(false);
	}

	bWasPressingSecondaryLastFrame = bPressingSecondary;
}

void ZMyCharacter::ProcessGuard()
{
	if (IsDie() || m_bWallJump || m_bDrop || m_bWallJump2 || m_bTumble ||
		m_bBlast || m_bBlastFall || m_bBlastDrop || m_bBlastStand || m_bBlastAirmove ||
		m_bSlash || m_bJumpSlash || m_bJumpSlashLanding) return;

	ZItem* pSItem = GetItems()->GetSelectedWeapon();
	if (!pSItem) return;

	if (pSItem->IsEmpty() || pSItem->GetDesc() == NULL) return;

	if (pSItem->GetDesc()->m_nType != MMIT_MELEE) return;

	MMatchWeaponType type = pSItem->GetDesc()->m_nWeaponType;
	if (type != MWT_KATANA && type != MWT_DOUBLE_KATANA) return;


	if (m_bGuard) {
		ReleaseLButtonQueue();
	}

#define GUARD_TIME	0.1f

	bool bBothPressed = m_bLButtonPressed && m_bRButtonPressed &&
		(m_bLButtonFirstPressed || m_bRButtonFirstPressed);

	float fTime = g_pGame->GetTime();
	bool bGuardTime = ((fTime - m_f1ShotTime) <= GUARD_TIME && m_bRButtonFirstPressed) ||
		((fTime - m_fSkillTime) <= GUARD_TIME && m_bLButtonFirstPressed) ||
		(m_bLButtonFirstPressed && m_bRButtonFirstPressed);

	if (!m_bGuard && m_bGuardKey) {
		bBothPressed = true;
		bGuardTime = true;
	}

	if (bBothPressed)
	{
		if (!m_bGuard && bGuardTime)
		{
			m_bSkill = false;
			m_bShot = false;
			m_nShot = 0;
			m_bGuard = true;
			m_bGuardStart = true;
			m_bWallHang = false;
			m_bJumpShot = false;
			m_fGuardStartTime = g_pGame->GetTime();

			if (m_bGuardKey) m_bGuardByKey = true;
			else			m_bGuardByKey = false;

			return;
		}
		ReleaseLButtonQueue();
	}
}

void ZMyCharacter::OnChangeWeapon(MMatchItemDesc* Weapon)
{
	ZCharacter::OnChangeWeapon(Weapon);

	ReleaseLButtonQueue();
	m_fNextShotTime = 0.f;
	m_fCAFactor = GetControllabilityFactor();
	m_fElapsedCAFactorTime = 0.0f;

	m_bSniferMode = false;

	ZCombatInterface* ci = ZApplication::GetGameInterface()->GetCombatInterface();
	if (ci)
	{
		ci->OnGadgetOff();
	}
}

#define AddText(s) {str.Add(#s,false); str.Add(" :",false); str.Add(s);}

void ZMyCharacter::OutputDebugString_CharacterState()
{
	RDebugStr str;

	ZCharacter::OutputDebugString_CharacterState();

	AddText(m_fDeadTime);
	AddText(m_fNextShotTime);
	AddText(m_fWallJumpTime);
	AddText(m_bWallHang);
	AddText(m_bTumble);
	AddText(m_nTumbleDir);
	AddText(m_bMoving);
	AddText(m_bReleasedJump);
	AddText(m_bJumpQueued);
	AddText(m_bWallJumpQueued);
	AddText(m_fLastJumpPressedTime);

	AddText(m_fJump2Time);
	AddText(m_fHangTime);
	AddText(m_HangPos);
	AddText(m_bHangSuccess);
	AddText(m_nWallJump2Dir);
	AddText(m_fLastLButtonPressedTime);
	AddText(m_bEnterCharge);
	AddText(m_bJumpShot);
	AddText(m_bShot);
	AddText(m_bShotReturn);
	AddText(m_nShot);
	AddText(m_bSkill);

	AddText(m_bSplashShot);
	AddText(m_fSplashShotTime);
	AddText(m_fLastShotTime);
	AddText(m_bGuard);
	AddText(m_nGuardBlock);
	AddText(m_bGuardBlock_ret);
	AddText(m_bGuardStart);
	AddText(m_bGuardCancel);
	AddText(m_bDrop);
	AddText(m_fDropTime);

	str.PrintLog();
}

#undef AddText

void ZMyCharacter::ProcessShot()
{
	if (ZApplication::GetGame()->GetMatch()->GetRoundState() == MMATCH_ROUNDSTATE_PREPARE) return;
	if (m_bInitialized == false) return;

	if (!m_bLButtonPressed) m_bEnterCharge = false;

	if (m_bLButtonFirstPressed && (g_pGame->GetTime() - m_fLastLButtonPressedTime < SHOT_QUEUE_TIME))
		m_bLButtonQueued = true;

	if (GetItems()->GetSelectedWeapon() == NULL ||
		GetItems()->GetSelectedWeapon()->IsEmpty()) return;

	if (GetItems()->GetSelectedWeapon()->GetDesc()->m_nType == MMIT_MELEE)
	{
		bool bJumpAttack = false;
		if (GetStateLower() == ZC_STATE_LOWER_JUMPATTACK) {
			bJumpAttack = true;
		}
		else if ((GetStateLower() == ZC_STATE_LOWER_JUMP_UP) || (GetStateLower() == ZC_STATE_LOWER_JUMP_DOWN)) {
			if (GetStateUpper() == ZC_STATE_UPPER_SHOT) {
				bJumpAttack = true;
			}
		}

		if (bJumpAttack) {
			ReleaseLButtonQueue();
			if (!m_bLButtonPressed)
				return;
		}
		else {
			if (!m_bLButtonQueued)
				return;
		}
	}
	else
	{
		if (ZApplication::GetGame()->GetMatch()->IsRuleGladiator() && !IsAdmin()) return;
		if (!m_bLButtonPressed) return;
	}

	if (GetItems()->GetSelectedWeapon()->GetDesc()->m_nType != MMIT_MELEE)
	{
		if (m_bWallJump) {
			if (m_nWallJumpDir == 1)
				return;
		}
	}

	if (m_bWallHang || m_bStun) return;

	if (IsDie() || m_bDrop || m_bBlast || m_bBlastFall || m_bBlastDrop || m_bBlastStand || m_bSpMotion)
		return;

	if (GetStateUpper() == ZC_STATE_UPPER_RELOAD || GetStateUpper() == ZC_STATE_UPPER_LOAD)
		return;

	int nParts = (int)GetItems()->GetSelectedWeaponParts();

	if (nParts<0 || nParts > MMCIP_END - 1) return;

	if (GetItems()->GetSelectedWeapon()->GetDesc()->m_nType != MMIT_MELEE)
		if (m_fNextShotTimeType[nParts] > g_pGame->GetTime())
			return;

	ZItem* pSelectedItem = GetItems()->GetSelectedWeapon();

	if ((pSelectedItem == NULL) || (pSelectedItem->GetDesc() == NULL))
		return;

	MMatchItemDesc* pRangeDesc = pSelectedItem->GetDesc();
	DWORD nWeaponDelay = pRangeDesc->m_nDelay;

	m_fNextShotTimeType[nParts] = g_pGame->GetTime() + (float)(nWeaponDelay)*0.001f;

	if (GetItems()->GetSelectedWeapon()->GetBulletAMagazine() <= 0)
	{
		if (m_pVMesh->m_SelectWeaponMotionType != eq_wd_grenade &&
			m_pVMesh->m_SelectWeaponMotionType != eq_wd_item &&
			m_pVMesh->m_SelectWeaponMotionType != eq_wd_katana &&
			m_pVMesh->m_SelectWeaponMotionType != eq_ws_dagger &&
			m_pVMesh->m_SelectWeaponMotionType != eq_wd_dagger &&
			m_pVMesh->m_SelectWeaponMotionType != eq_wd_sword &&
			m_pVMesh->m_SelectWeaponMotionType != eq_wd_blade)
		{
			if (GetItems()->GetSelectedWeapon()->GetBullet() <= 0) {
				ZGetSoundEngine()->PlaySEDryFire(GetItems()->GetSelectedWeapon()->GetDesc(), 0, 0, 0, true);
			}
			else {
				m_bSpMotion = false;
				ZPostReload();
			}
		}
	}

	switch (m_pVMesh->m_SelectWeaponMotionType) {
	case eq_wd_grenade:
		OnShotCustom();
		break;
	case eq_wd_item:
		if (m_bLand) OnShotItem();
		break;
	case eq_wd_rlauncher:
		OnShotRocket();
		break;
	case eq_wd_katana:
	case eq_ws_dagger:
	case eq_wd_dagger:
	case eq_wd_sword:
	case eq_wd_blade:
		OnShotMelee();
		break;
	default:
		OnShotRange();
	}

	UpdateStylishShoted();
}

void ZMyCharacter::UpdateAnimation()
{
	if (m_bInitialized == false) return;

	bool bTaunt = false;

	if (m_bStun)
	{
		switch (m_nStunType)
		{
		case 0: SetAnimationLower(ZC_STATE_DAMAGE); break;
		case 1: SetAnimationLower(ZC_STATE_DAMAGE2); break;
		case 2: SetAnimationLower(ZC_STATE_DAMAGE_DOWN); break;
		case 3: SetAnimationLower(ZC_STATE_DAMAGE_BLOCKED); break;
		case 4: SetAnimationLower(ZC_STATE_DAMAGE_LIGHTNING); break;
		case 5: SetAnimationLower(ZC_STATE_DAMAGE_STUN); break;
		}
	}
	else if (m_bBlastAirmove)
	{
		SetAnimationLower(ZC_STATE_LOWER_BLAST_AIRMOVE);
	}
	else if (m_bBlastStand)
	{
		SetAnimationLower(ZC_STATE_LOWER_BLAST_STAND);
	}
	else if (m_bBlastFall)
	{
		if (m_nBlastType == 0) SetAnimationLower(ZC_STATE_LOWER_BLAST_FALL);
		else if (m_nBlastType == 1) SetAnimationLower(ZC_STATE_LOWER_BLAST_DROP_DAGGER);
	}
	else if (m_bBlastDrop)
	{
		if (m_nBlastType == 0) SetAnimationLower(ZC_STATE_LOWER_BLAST_DROP);
		else if (m_nBlastType == 1) SetAnimationLower(ZC_STATE_LOWER_BLAST_DROP_DAGGER);

	}
	else if (m_bBlast)
	{
		if (m_nBlastType == 0) SetAnimationLower(ZC_STATE_LOWER_BLAST);
		else if (m_nBlastType == 1) SetAnimationLower(ZC_STATE_LOWER_BLAST_DAGGER);

	}
	else if (m_bSkill)
	{
		SetAnimationLower(ZC_STATE_LOWER_UPPERCUT);
	}
	else if (m_bJumpShot)
	{
		SetAnimationLower(ZC_STATE_LOWER_JUMPATTACK);
	}
	else if (m_bGuard)
	{
		if (m_bJumpUp)
		{
			SetAnimationLower(ZC_STATE_LOWER_JUMP_UP);
		}
		else if (m_bJumpDown)
		{
			SetAnimationLower(ZC_STATE_LOWER_JUMP_DOWN);
		}
		else if (m_bMoving)
		{
			if (m_bBackMoving)
			{
				SetAnimationLower(ZC_STATE_LOWER_RUN_BACK);
			}
			else
			{
				SetAnimationLower(ZC_STATE_LOWER_RUN_FORWARD);
			}
		}
		else
		{
			SetAnimationLower(ZC_STATE_LOWER_IDLE1);
		}

		if (m_bGuardStart)
		{
			SetAnimationUpper(ZC_STATE_UPPER_GUARD_START);
		}
		else if (m_bGuardCancel)
		{
			SetAnimationUpper(ZC_STATE_UPPER_GUARD_CANCEL);
		}
		else if (m_nGuardBlock)
		{
			switch (m_nGuardBlock)
			{
			case 1: SetAnimationUpper(ZC_STATE_UPPER_GUARD_BLOCK1); break;
			case 2: SetAnimationUpper(ZC_STATE_UPPER_GUARD_BLOCK2); break;
			}
		}
		else if (m_bGuardBlock_ret)
		{
			SetAnimationUpper(ZC_STATE_UPPER_GUARD_BLOCK1_RET);
		}
		else
		{
			SetAnimationUpper(ZC_STATE_UPPER_GUARD_IDLE);
		}
	}
	else if (m_bShotReturn)
	{
		switch (m_nShot)
		{
		case 1: SetAnimationLower(ZC_STATE_LOWER_ATTACK1_RET); break;
		case 2: SetAnimationLower(ZC_STATE_LOWER_ATTACK2_RET); break;
		case 3: SetAnimationLower(ZC_STATE_LOWER_ATTACK3_RET); break;
		case 4: SetAnimationLower(ZC_STATE_LOWER_ATTACK4_RET); break;
		}
	}
	else if (m_bShot)
	{
		switch (m_nShot)
		{
		case 1: SetAnimationLower(ZC_STATE_LOWER_ATTACK1); break;
		case 2: SetAnimationLower(ZC_STATE_LOWER_ATTACK2); break;
		case 3: SetAnimationLower(ZC_STATE_LOWER_ATTACK3); break;
		case 4: SetAnimationLower(ZC_STATE_LOWER_ATTACK4); break;
		case 5: SetAnimationLower(ZC_STATE_LOWER_ATTACK5); break;
		}
	}
	else if (m_bWallJump2)
	{
		switch (m_nWallJump2Dir)
		{
		case 0: SetAnimationLower(ZC_STATE_LOWER_JUMP_WALL_FORWARD); break;
		case 1: SetAnimationLower(ZC_STATE_LOWER_JUMP_WALL_BACK); break;
		case 2: SetAnimationLower(ZC_STATE_LOWER_JUMP_WALL_LEFT); break;
		case 3: SetAnimationLower(ZC_STATE_LOWER_JUMP_WALL_RIGHT); break;
		case 4: SetAnimationLower(ZC_STATE_LOWER_RUN_WALL_LEFT_DOWN); break;
		case 5: SetAnimationLower(ZC_STATE_LOWER_RUN_WALL_RIGHT_DOWN); break;
		case 6: SetAnimationLower(ZC_STATE_LOWER_RUN_WALL_DOWN_FORWARD); break;
		case 7: SetAnimationLower(ZC_STATE_LOWER_RUN_WALL_DOWN); break;
		}
	}
	else if (m_bWallHang)
	{
		SetAnimationLower(ZC_STATE_LOWER_BIND);
	}
	else if (m_bTumble)
	{
		switch (m_nTumbleDir)
		{
		case 0: SetAnimationLower(ZC_STATE_LOWER_TUMBLE_FORWARD); break;
		case 1: SetAnimationLower(ZC_STATE_LOWER_TUMBLE_BACK); break;
		case 2: SetAnimationLower(ZC_STATE_LOWER_TUMBLE_RIGHT); break;
		case 3: SetAnimationLower(ZC_STATE_LOWER_TUMBLE_LEFT); break;
		}
	}
	else if (m_bWallJump)
	{
		switch (m_nWallJumpDir)
		{
		case 0: SetAnimationLower(ZC_STATE_LOWER_RUN_WALL_LEFT); break;
		case 1: SetAnimationLower(ZC_STATE_LOWER_RUN_WALL); break;
		case 2: SetAnimationLower(ZC_STATE_LOWER_RUN_WALL_RIGHT); break;
		}
	}
	else if (m_bJumpSlash)
	{
		SetAnimationLower(ZC_STATE_JUMP_SLASH1);
	}
	else if (m_bJumpSlashLanding)
	{
		SetAnimationLower(ZC_STATE_JUMP_SLASH2);
	}
	else if (m_bSlash)
	{
		SetAnimationLower(ZC_STATE_SLASH);
	}
	else if (m_bJumpUp)
	{
		SetAnimationLower(ZC_STATE_LOWER_JUMP_UP);
	}
	else if (m_bJumpDown)
	{
		SetAnimationLower(ZC_STATE_LOWER_JUMP_DOWN);
	}
	else if (m_bCharging)
	{
		SetAnimationLower(ZC_STATE_CHARGE);
	}
	else if (m_bMoving)
	{
		if (m_bBackMoving)
		{
			SetAnimationLower(ZC_STATE_LOWER_RUN_BACK);
		}
		else
		{
			SetAnimationLower(ZC_STATE_LOWER_RUN_FORWARD);
		}
	}
	else if (m_bSpMotion)
	{
		SetAnimationLower(m_SpMotion);

		bTaunt = true;
	}
	else
	{
		SetAnimationLower(ZC_STATE_LOWER_IDLE1);
	}

	if (m_bSpMotion && !bTaunt)
	{
		m_bSpMotion = false;
	}
}

void ZMyCharacter::WallJump2()
{
	m_bWallJump2 = true;
	m_bJumpQueued = false;
	m_bWallJump = false;
	m_bPlayDone = false;

	rvector jump2 = -rvector(m_Direction.x, m_Direction.y, 0);
	Normalize(jump2);

	if (m_nWallJumpDir == 1)
	{
		if (ZIsActionKeyDown(ZACTION_FORWARD))
		{
			m_nWallJump2Dir = 6;
			SetVelocity(jump2*50.f);
		}
		else
		{
			m_nWallJump2Dir = 7;
			AddVelocity(jump2*300.f);
		}
		AddVelocity(rvector(0, 0, 1400));
	}
	else
	{
		float fSecondJumpSpeed = WALL_JUMP2_SIDE_VELOCITY;

		rvector right;
		CrossProduct(&right, m_Direction, rvector(0, 0, 1));
		jump2 = (m_nWallJumpDir == 0) ? -right : right;
		if (m_nWallJumpDir == 0)
			m_nWallJump2Dir = 4;
		else
			m_nWallJump2Dir = 5;

		AddVelocity(fSecondJumpSpeed*jump2);
		AddVelocity(rvector(0, 0, 1300));
	}

	m_fJump2Time = g_pGame->GetTime();
}

void ZMyCharacter::UpdateLimit()
{
	MMatchItemDesc* pDesc = GetSelectItemDesc();

	m_bLimitJump = false;
	m_bLimitTumble = false;
	m_bLimitWall = false;

	if (pDesc) {

		if (pDesc->m_nLimitJump)			m_bLimitJump = true;
		if (pDesc->m_nLimitTumble)		m_bLimitTumble = true;
		if (pDesc->m_nLimitWall)			m_bLimitWall = true;
	}

	if (IsActiveModule(ZMID_COLDDAMAGE)) {
		m_bLimitTumble = true;
		m_bLimitWall = true;
	}
}

void ZMyCharacter::UpdateButtonState()
{
	bool bLButtonPressed = ZIsActionKeyDown(ZACTION_USE_WEAPON);

	m_bLButtonFirstPressed = bLButtonPressed && !m_bLButtonPressed;
	if (m_bLButtonFirstPressed)
		m_fLastLButtonPressedTime = g_pGame->GetTime();

	m_bLButtonPressed = bLButtonPressed;


	bool bRButtonPressed = ZIsActionKeyDown(ZACTION_USE_WEAPON2);

	m_bRButtonFirstReleased = (m_bRButtonPressed && !bRButtonPressed);

	m_bRButtonFirstPressed = bRButtonPressed && !m_bRButtonPressed;
	if (m_bRButtonFirstPressed)
		m_fLastRButtonPressedTime = g_pGame->GetTime();

	m_bRButtonPressed = bRButtonPressed;

	if (m_bLButtonQueued && (g_pGame->GetTime() - m_fLastLButtonPressedTime > SHOT_QUEUE_TIME)) {
		m_bLButtonQueued = false;
	}
}

void ZMyCharacter::OnUpdate(float fDelta)
{
	if (m_bInitialized == false) return;

	_BP("ZMyCharacter::Update");

	if (g_pGame->IsReplay()) {
		ZCharacter::OnUpdate(fDelta);
		return;
	}

	CameraDir = m_TargetDir;

	if (m_bReserveDashAttacked) {
		if (g_pGame->GetTime() > m_fReserveDashAttackedTime) {
			OnDashAttacked(m_vReserveDashAttackedDir);
			m_bReserveDashAttacked = false;
		}
	}

	UpdateLimit();

	if ((m_bWallHang && m_bHangSuccess) || m_bShot)
		SetVelocity(0, 0, 0);

	UpdateVelocity(fDelta);

	fDelta = min(fDelta, 1.f);

	if (IsDie() && ((m_bLand && m_bPlayDone))) {
		SetVelocity(0, 0, 0);
	}

	m_bMoving = Magnitude(rvector(GetVelocity().x, GetVelocity().y, 0)) > .1f;

	const int MAX_MOVE_FRAME = 150;
	static float fAccmulatedDelta = 0.f;

	fAccmulatedDelta += fDelta;

	if (fAccmulatedDelta > (1.f / (float)MAX_MOVE_FRAME))
	{
		m_pModule_Movable->OnUpdate(fAccmulatedDelta);

		if (Magnitude(m_pModule_Movable->GetLastMove()) < 0.01f)
			SetVelocity(0, 0, 0);

		if (GetDistToFloor() > 0.1f || GetVelocity().z > 0.1f)
		{
			m_pModule_Movable->UpdateGravity(GetGravityConst()*fAccmulatedDelta);
			m_bLand = false;
		}

		fAccmulatedDelta = 0;
	}

	UpdateHeight(fDelta);

	rvector diff = fDelta*GetVelocity();

	bool bUp = (diff.z > 0.01f);
	bool bDownward = (diff.z < 0.01f);

	if ((m_Position.z > DIE_CRITICAL_LINE) &&
		(GetDistToFloor() < 0 || (bDownward && m_pModule_Movable->GetLastMove().z >= 0)))
	{
		if (!m_bWallJump)
		{
			if (GetVelocity().z < 1.f && GetDistToFloor() < 1.f)
			{
				m_bLand = true;
				m_bWallJump = false;
				m_bJumpDown = false;
				m_bJumpUp = false;
				m_bWallJump2 = false;
				m_bBlastAirmove = false;
				m_bWallHang = false;

				if (GetVelocity().z < 0)
					SetVelocity(GetVelocity().x, GetVelocity().y, 0);

				if (GetLastThrowClearTime() < g_pGame->GetTime())
					SetLastThrower(MUID(0, 0), 0.0f);
			}
		}
	}

	UpdateCAFactor(fDelta);

	if (GetDistToFloor() < 0 && !IsDie())
	{
		float fAdjust = 400.f*fDelta;
		rvector diff = rvector(0, 0, min(-GetDistToFloor(), fAdjust));
		Move(diff);
	}

	if (IsMoveAnimation())
	{
		rvector origdiff = fDelta*GetVelocity();

		rvector diff = m_AnimationPositionDiff;
		diff.z += origdiff.z;

		if (GetDistToFloor() < 0 && diff.z < 0) diff.z = -GetDistToFloor();

		Move(diff);
	}

#ifndef _PUBLISH
	if (!MEvent::IsKeyDown(VK_MENU))
#endif
		if (!IsDirLocked())
			SetTargetDir(ZGetCamera()->GetCurrentDir());

	float fWallJumpTime = (m_nWallJumpDir == 1) ? 1.5f : 2.3f;
	float fSecondJumpTime = (m_nWallJumpDir == 1) ? 0.95f : 2.1f;

	if (m_fWallJumpTime + fWallJumpTime < g_pGame->GetTime() && m_bWallJump)
	{
		m_bWallJump = false;
		m_bLand = true;
		m_bJumpDown = false;
		m_bJumpUp = false;

		SetVelocity(GetVelocity().x, GetVelocity().y, 0);
	}

	if (m_bWallJump)
	{
		bool bEndWallJump2 = m_fWallJumpTime + fSecondJumpTime < g_pGame->GetTime() && !m_bWallJump2;

		if (m_fWallJumpTime + 0.3f < g_pGame->GetTime())
		{
			rvector dir2d = rvector(m_Direction.x, m_Direction.y, 0);
			Normalize(dir2d);

			if (m_nWallJumpDir == 1)
			{
				RBSPPICKINFO bpi;
				bool bPicked = ZGetGame()->GetWorld()->GetBsp()->Pick(m_Position + rvector(0, 0, 40), dir2d, &bpi);
				if (!bPicked || Magnitude(bpi.PickPos - m_Position)>100)
					bEndWallJump2 |= true;

				rvector targetpos = m_Position + rvector(0, 0, 160);
				bool bAdjusted = ZGetGame()->GetWorld()->GetBsp()->CheckWall(m_Position + rvector(0, 0, 130), targetpos, CHARACTER_RADIUS - 1, 50);
				if (bAdjusted)
					bEndWallJump2 |= true;
			}
			else
			{
				rvector right;
				CrossProduct(&right, rvector(0, 0, 1), dir2d);

				rvector dir = (m_nWallJumpDir == 0) ? -right : right;

				RBSPPICKINFO bpi;
				bool bPicked = ZGetGame()->GetWorld()->GetBsp()->Pick(m_Position, dir, &bpi);
				if (!bPicked || Magnitude(bpi.PickPos - m_Position) > 100)
					bEndWallJump2 |= true;

				rvector targetpos = m_Position + rvector(0, 0, 100) + 100.f*dir2d;
				bool bAdjusted = ZGetGame()->GetWorld()->GetBsp()->CheckWall(m_Position + rvector(0, 0, 100), targetpos, CHARACTER_RADIUS - 1, 50);
				if (bAdjusted)
					bEndWallJump2 |= true;
			}
		}

		if (bEndWallJump2)
			WallJump2();
	}

	int sel_type = GetItems()->GetSelectedWeaponParts();

	if (m_bShot && m_nShot == 1 && !m_b1ShotSended && g_pGame->GetTime() - m_f1ShotTime > GUARD_TIME) {
		m_b1ShotSended = true;
		if (ZGetGameClient()->GetMatchStageSetting()->IsVanillaMode())
		{
			ZPostShotMelee(g_pGame->GetTime(), m_Position, 1);
			AddDelayedWork(g_pGame->GetTime() + 0.2f, ZDW_RECOIL);
		}
		else
		{
			AddDelayedWork(g_pGame->GetTime() + 0.1, ZDW_RG_SLASH, (void *)1);
		}
	}

	if (m_bStun && m_nStunType == ZST_LOOP && g_pGame->GetTime() > m_fStunEndTime) {
		m_bStun = false;
	}

	if (
#ifdef _DEBUG
		!m_bGuardTest &&
#endif		
		m_bGuard && g_pGame->GetTime() - m_fGuardStartTime > GUARD_DURATION) {
		m_nGuardBlock = 0;
		m_bGuardStart = false;
		m_bGuardCancel = true;
		m_bGuardKey = false;
		m_bGuardByKey = false;
	}

	if (!m_bLButtonPressed || GetItems()->GetSelectedWeaponParts() != MMCIP_MELEE)
		m_bCharging = false;

	if (m_bSplashShot && g_pGame->GetTime() - m_fSplashShotTime > .3f)
	{
		m_bSplashShot = false;
		ZPostSkill(g_pGame->GetTime(), ZC_SKILL_SPLASHSHOT, sel_type);
	}

	AniFrameInfo* pAniLow = m_pVMesh->GetFrameInfo(ani_mode_lower);

	if (pAniLow && pAniLow->m_isPlayDone) {
		if (m_bSpMotion) {
			if (GetStateLower() == m_SpMotion)
				m_bSpMotion = false;
		}
	}

	bool bContinueShot = false;

	if (m_bPlayDone_upper) {
		if (GetStateUpper() == ZC_STATE_UPPER_SHOT)
		{
			int nParts = (int)GetItems()->GetSelectedWeaponParts();
			if (nParts == MMCIP_MELEE)
			{
				ZItem* pSelectedItem = GetItems()->GetSelectedWeapon();
				_ASSERT(pSelectedItem && pSelectedItem->GetDesc());

				if (pSelectedItem) {
					MMatchItemDesc* pRangeDesc = pSelectedItem->GetDesc();
					DWORD nWeaponDelay = pRangeDesc->m_nDelay;

					m_fNextShotTimeType[nParts] = g_pGame->GetTime() + (float)(nWeaponDelay)*0.001f;
				}

				if (m_bEnterCharge)
					EnterCharge();
			}
			else if ((nParts == MMCIP_CUSTOM1) || (nParts == MMCIP_CUSTOM2)) {
				ZItem* pItem = GetItems()->GetSelectedWeapon();
				if (pItem && pItem->GetBulletAMagazine() <= 0)
					ZGetGameInterface()->ChangeWeapon(ZCWT_PREV);
			}
		}
		else
			if (GetStateUpper() == ZC_STATE_UPPER_RELOAD)
				g_pGame->OnReloadComplete(this);
			else
				if (m_bGuard) {

					if (m_bGuardBlock_ret) {

						m_bGuardBlock_ret = false;
					}
					else if (m_nGuardBlock) {

						if (m_nGuardBlock < 2 && g_pGame->GetTime() - m_fLastShotTime < 0.2f) {
							m_nGuardBlock++;
							m_bPlayDone_upper = false;
						}
						else {

							if (m_nGuardBlock == 1)
								m_bGuardBlock_ret = true;

							m_bPlayDone_upper = false;
							m_nGuardBlock = 0;
						}
					}

					m_bGuardStart = false;

					if (m_bGuardCancel) {

						m_bGuardCancel = false;
						m_bGuard = false;

					}

				}

		SetAnimationUpper(ZC_STATE_UPPER_NONE);
	}

	AniFrameInfo* pAniUp = m_pVMesh->GetFrameInfo(ani_mode_upper);

	if (pAniUp) {
		if (pAniUp->m_pAniSet == NULL) {
			SetAnimationUpper(ZC_STATE_UPPER_NONE);
		}
	}

	MMatchWeaponType wtype = MWT_NONE;

	ZItem* pSItem = GetItems()->GetSelectedWeapon();

	if (pSItem && pSItem->GetDesc()) {
		wtype = pSItem->GetDesc()->m_nWeaponType;
	}

	if (m_bPlayDone)
	{
		if (m_bWallJump) {
			WallJump2();
		}

		m_bTumble = false;

		if (m_bCharging)
		{
			if (GetStateUpper() == ZC_STATE_CHARGE)
				Charged();
			m_bCharging = false;
		}

		m_bSlash = false;

		m_bJumpSlashLanding = false;

		if (m_bJumpSlash && (m_bLand || (g_pGame->GetTime() - m_bJumpSlashTime > 2.f))) {
			m_bJumpSlash = false;
			m_bJumpSlashLanding = true;
		}

		if (m_bShotReturn) {

			m_nShot = 0;
			m_bShotReturn = false;
			SetVelocity(GetVelocity().x, GetVelocity().y, 0);
			m_bMoving = false;
		}

		if (m_bShot) {

			if (m_bEnterCharge) {

				EnterCharge();
			}
			else if (m_nShot == 4) {

				m_bShot = false;
				m_bShotReturn = true;
				m_bPlayDone = false;

			}
			else {

				if (m_nShot == 5)
				{
					m_nShot = 0;
					m_bShot = false;
					m_bJumpDown = false;
				}
				else
				{
					if (!m_bLButtonQueued && m_nShot < 5)
					{
						m_bShot = false;
						m_bShotReturn = true;
					}

					if (m_nShot == 3)
						if (m_pVMesh->m_SelectWeaponMotionType == eq_wd_sword) {
							m_nShot = 0;
							m_bShot = false;
							m_bShotReturn = false;
						}

				}
			}
		}

		if (m_bJumpShot)
		{
			m_bJumpShot = false;
		}

		m_bSkill = false;
		m_bBlastStand = false;

		if (m_bBlast)
		{
			m_bBlast = false;

			if (g_pGame->GetTime() - m_fLastJumpPressedTime < 0.5f)
				m_bBlastAirmove = true;
			else
				m_bBlastFall = true;
		}

		if (m_bBlastFall && !m_bDrop && m_bLand)
		{
			m_bDrop = true;
			m_bBlastFall = false;
			m_bBlastDrop = true;
			m_fDropTime = g_pGame->GetTime();
		}

		if (m_bStun && m_nStunType != ZST_LOOP) {
			m_bStun = false;
		}
	}

#ifdef _DEBUG
	if (m_bGuardTest)
	{
		m_fLastShotTime = g_pGame->GetTime();
	}
#endif

	if (m_bGuard && !m_bGuardCancel
#ifdef _DEBUG
		&& !m_bGuardTest
#endif
		) {

		bool bGuardCancel = false;

		if (m_bGuardByKey) {
			if (m_bGuardKey == false)
				bGuardCancel = true;
		}
		else {
			if ((!m_bLButtonPressed || !m_bRButtonPressed)) {
				bGuardCancel = true;
			}
		}

		if (bGuardCancel)
		{
			m_nGuardBlock = 0;
			m_bGuardStart = false;
			m_bGuardCancel = true;
			m_bGuardKey = false;
			m_bGuardByKey = false;
			SetAnimationUpper(ZC_STATE_UPPER_NONE);
		}
		}

	if (m_bGuard)
	{
		if (m_nGuardBlock == 0 && !m_bGuardBlock_ret && g_pGame->GetTime() - m_fLastShotTime < 0.2f)
		{
			m_nGuardBlock = 1;
		}
	}

	if (m_bDrop && (g_pGame->GetTime() - m_fDropTime > 1.f))
	{
		m_bDrop = false;
		m_bBlastDrop = false;
		m_bBlastStand = true;
	}

	if (m_bBlastFall && m_bLand && pAniLow->m_nFrame > 160 * 15)
	{
		m_bDrop = true;
		m_bBlastFall = false;
		m_bBlastDrop = true;
		m_fDropTime = g_pGame->GetTime();
	}

	if (m_bCharging && !m_bCharged && GetStateLower() == ZC_STATE_CHARGE &&
		pAniLow->m_nFrame > 160 * 52)
		Charged();

	if (m_bJumpSlash && m_bLand && pAniLow->m_nFrame > 160 * 11)
	{
		m_bJumpSlash = false;
		m_bJumpSlashLanding = true;
	}

	UpdateAnimation();

	ZCharacter::OnUpdate(fDelta);

	m_bPlayDone = pAniLow->m_isPlayDone;
	m_bPlayDone_upper =
		pAniUp->m_pAniSet == NULL ? false : pAniUp->m_isPlayDone;

	ProcessDelayedWork();

	_EP("ZMyCharacter::Update");
	}

void ZMyCharacter::Animation_Reload()
{
	m_bPlayDone_upper = false;
	SetAnimationUpper(ZC_STATE_UPPER_RELOAD);
}

void ZMyCharacter::ReserveDashAttacked(MUID uid, float time, rvector &dir)
{
	m_uidReserveDashAttacker = uid;
	m_bReserveDashAttacked = true;
	m_vReserveDashAttackedDir = dir;
	m_fReserveDashAttackedTime = time;
}

void ZMyCharacter::OnDashAttacked(rvector &dir)
{
	if (m_bBlast || m_bBlastDrop || m_bBlastStand || isInvincible())
		return;

	m_bSkill = false;
	m_bStun = false;
	m_bShot = false;

	m_bBlast = true;
	m_nBlastType = 1;

	AddVelocity(rvector(dir.x*2000.f, dir.y*2000.f, dir.z*2000.f));
	Normalize(dir);

	float fRatio = GetMoveSpeedRatio();

	AddVelocity(dir * RUN_SPEED * fRatio);

	SetLastThrower(m_uidReserveDashAttacker, g_pGame->GetTime() + 1.0f);

	m_bWallHang = false;
	m_bHangSuccess = false;

	m_bSlash = false;
	m_bJumpSlash = false;
	m_bJumpSlashLanding = false;
}

void ZMyCharacter::OnBlast(rvector &dir)
{
	if (m_bBlast || m_bBlastDrop || m_bBlastStand || isInvincible())
		return;

	m_bSkill = false;
	m_bStun = false;
	m_bShot = false;

	m_bBlast = true;
	m_nBlastType = 0;

	m_bLand = false;

	SetVelocity(dir * 300.f + rvector(0, 0, BLAST_VELOCITY));

	m_bWallHang = false;
	m_bHangSuccess = false;

	m_bSlash = false;
	m_bJumpSlash = false;
	m_bJumpSlashLanding = false;

	m_bGuard = false;
	SetAnimationUpper(ZC_STATE_UPPER_NONE);
}

void ZMyCharacter::OnTumble(int nDir)
{
#define SWORD_DASH		1000.f
#define GUN_DASH        900.f
	if (IsDie() || m_bWallJump || m_bGuard || m_bDrop || m_bWallJump2 || m_bTumble || m_bWallHang ||
		m_bBlast || m_bBlastFall || m_bBlastDrop || m_bBlastStand || m_bBlastAirmove ||
		m_bCharging || m_bSlash || m_bJumpSlash || m_bJumpSlashLanding ||
		m_bStun || GetStateLower() == ZC_STATE_LOWER_UPPERCUT) return;

	if (m_bLimitTumble)
		return;

	rvector right;
	rvector forward = RCameraDirection;
	forward.z = 0;
	Normalize(forward);
	CrossProduct(&right, rvector(0, 0, 1), forward);

	float fSpeed;
	if (GetItems()->GetSelectedWeapon() != NULL &&
		(GetItems()->GetSelectedWeapon()->IsEmpty() == false) &&
		GetItems()->GetSelectedWeapon()->GetDesc()->m_nType == MMIT_MELEE)
	{
		fSpeed = SWORD_DASH;

		rvector vPos = GetPosition();
		rvector vDir = RealSpace2::RCameraDirection;
		rvector vRight;

		CrossProduct(&vRight, RealSpace2::RCameraUp, RealSpace2::RCameraDirection);

		if (nDir == 0) {
			vDir = -RealSpace2::RCameraDirection;
		}
		else if (nDir == 1) {
			vDir = RealSpace2::RCameraDirection;
		}
		else if (nDir == 2) {
			vDir = -vRight;
		}
		else if (nDir == 3) {
			vDir = vRight;
		}

		vDir.z = 0.f;
		Normalize(vDir);

		int sel_type = GetItems()->GetSelectedWeaponParts();

		if (!m_bWallHang && !m_bSkill && !m_bShot && !m_bShotReturn)
			ZPostDash(vPos, vDir, sel_type);
	}
	else
	{
		fSpeed = GUN_DASH;
	}

	switch (nDir)
	{
	case 0:  SetVelocity(forward*fSpeed); break;
	case 1:  SetVelocity(-forward*fSpeed); break;
	case 2:  SetVelocity(right*fSpeed); break;
	case 3:  SetVelocity(-right*fSpeed); break;
	}

	m_bTumble = true;
	m_nTumbleDir = nDir;
	m_bSpMotion = false;
}

void ZMyCharacter::InitStatus()
{
	m_f1ShotTime = 0;
	m_fSkillTime = 0;

	m_fDeadTime = 0;

	m_bHero = true;
	m_fLastJumpPressedTime = -1.f;
	m_fJump2Time = 0.f;
	m_bWallJump2 = false;
	m_bLand = true;
	m_bWallJump = false;
	m_bJumpUp = false;
	m_bJumpDown = false;
	m_bWallHang = false;
	m_bHangSuccess = false;
	m_bTumble = false;
	m_bShot = false;
	m_bShotReturn = false;
	m_nShot = 0;
	m_fLastLButtonPressedTime = -1.f;
	ReleaseLButtonQueue();

	m_bJumpShot = false;
	m_bSkill = false;
	m_bJumpQueued = false;
	m_bDrop = false;

	m_nJumpShotType = 0;

	m_bSplashShot = false;
	m_fSplashShotTime = 0.f;

	m_bGuard = false;
	m_nGuardBlock = 0;
	m_bGuardStart = false;
	m_bGuardCancel = false;

	m_fLastShotTime = 0.f;
	m_fNextShotTime = 0.f;

	m_bSlash = false;
	m_bJumpSlash = false;
	m_bJumpSlashLanding = false;

	m_bEnterCharge = false;

	ZCharacter::InitStatus();

}

void ZMyCharacter::Revival()
{
	ZCharacter::Revival();

#ifdef FLASH_WINDOW_ON_RESPAWN
	FlashWindow(g_hWnd, FALSE);
#endif
}

void ZMyCharacter::InitBullet()
{
	ZCharacter::InitBullet();

	MDataChecker* pChecker = ZApplication::GetGame()->GetDataChecker();

	if (!m_Items.GetItem(MMCIP_PRIMARY)->IsEmpty())
	{
		pChecker->RenewCheck((BYTE*)m_Items.GetItem(MMCIP_PRIMARY)->GetBulletPointer(), sizeof(int));
		pChecker->RenewCheck((BYTE*)m_Items.GetItem(MMCIP_PRIMARY)->GetAMagazinePointer(), sizeof(int));
	}
	if (!m_Items.GetItem(MMCIP_SECONDARY)->IsEmpty())
	{
		pChecker->RenewCheck((BYTE*)m_Items.GetItem(MMCIP_SECONDARY)->GetBulletPointer(), sizeof(int));
		pChecker->RenewCheck((BYTE*)m_Items.GetItem(MMCIP_SECONDARY)->GetAMagazinePointer(), sizeof(int));
	}
	if (!m_Items.GetItem(MMCIP_CUSTOM1)->IsEmpty())
	{
		pChecker->RenewCheck((BYTE*)m_Items.GetItem(MMCIP_CUSTOM1)->GetBulletPointer(), sizeof(int));
		pChecker->RenewCheck((BYTE*)m_Items.GetItem(MMCIP_CUSTOM1)->GetAMagazinePointer(), sizeof(int));
	}
	if (!m_Items.GetItem(MMCIP_CUSTOM2)->IsEmpty())
	{
		pChecker->RenewCheck((BYTE*)m_Items.GetItem(MMCIP_CUSTOM2)->GetBulletPointer(), sizeof(int));
		pChecker->RenewCheck((BYTE*)m_Items.GetItem(MMCIP_CUSTOM2)->GetAMagazinePointer(), sizeof(int));
	}
}

void ZMyCharacter::SetDirection(const rvector& dir)
{
	ZCharacter::SetDirection(dir);
	ZCamera* pCamera = ZApplication::GetGameInterface()->GetCamera();
	pCamera->SetDirection(dir);
}

void ZMyCharacter::OnDamagedAnimation(ZObject *pAttacker, int type)
{
	const auto GameType = ZGetGame()->GetMatch()->GetMatchType();
	if (GameType == MMATCH_GAMETYPE_SKILLMAP)
		return;

	if (GameType == MMATCH_GAMETYPE_TRAINING &&
		GetRGMain().TrainingSettings.NoStuns)
		return;

	ZCharacter::OnDamagedAnimation(pAttacker, type);

	m_bShot = false;
	m_bWallHang = false;
	m_bCharging = false;
	m_bSkill = false;
}

void ZMyCharacter::OnDie()
{
	ZCharacter::OnDie();
	m_fDeadTime = g_pGame->GetTime();

	m_bWallHang = false;
	m_bWallJump = false;

	SetHP(0);
}

#define DAMAGE_VELOCITY		1700.f
#define MAX_FALL_SPEED		3000.f
#define MAX_FALL_DAMAGE		50.f
#define BLASTED_KNOCKBACK_RATIO	3.f

#define CA_FACTOR_PISTOL		0.3f
#define CA_FACTOR_SMG			0.3f
#define CA_FACTOR_REVOLVER		0.35f
#define CA_FACTOR_SHOTGUN		1.0f
#define CA_FACTOR_RIFLE			0.25f
#define CA_FACTOR_MACHINEGUN	1.0f
#define CA_FACTOR_ROCKET		1.0f

float ZMyCharacter::GetControllabilityFactor()
{
	MMatchWeaponType wtype = MWT_NONE;

	ZItem* pSItem = GetItems()->GetSelectedWeapon();

	if (pSItem) {
		if (pSItem->GetDesc())
			wtype = pSItem->GetDesc()->m_nWeaponType;
	}

	{
		switch (wtype)
		{

		case MWT_PISTOL:
		case MWT_PISTOLx2:
		{
			return CA_FACTOR_PISTOL;
		}
		break;

		case MWT_REVOLVER:
		case MWT_REVOLVERx2:
		{
			return CA_FACTOR_REVOLVER;
		}
		break;

		case MWT_SMG:
		case MWT_SMGx2:
		{
			return CA_FACTOR_SMG;
		}
		break;

		case MWT_SHOTGUN:
		case MWT_SAWED_SHOTGUN:
		{
			return CA_FACTOR_SHOTGUN;
		}
		break;

		case MWT_MACHINEGUN:
		{
			return CA_FACTOR_MACHINEGUN;
		}
		break;

		case MWT_RIFLE:
		case MWT_SNIFER:
		{
			return CA_FACTOR_RIFLE;
		}
		break;

		case MWT_ROCKET:
		{
			return CA_FACTOR_ROCKET;
		}
		break;
		}
	}

	return 1.0f;
}

void ZMyCharacter::UpdateCAFactor(float fDelta)
{
	const float fCurrWeaponCAFactor = GetControllabilityFactor();
	bool bPressed = ZIsActionKeyDown(ZACTION_USE_WEAPON);
	if (bPressed) return;

	m_fElapsedCAFactorTime += fDelta;
	while (m_fElapsedCAFactorTime > CAFACTOR_DEC_DELAY_TIME)
	{
		m_fCAFactor -= fCurrWeaponCAFactor;
		m_fElapsedCAFactorTime -= CAFACTOR_DEC_DELAY_TIME;
	}

	if (m_fCAFactor < fCurrWeaponCAFactor) m_fCAFactor = fCurrWeaponCAFactor;
}




#ifndef _PUBLISH

#include "Physics.h"

////////////////////////////////////////////////////////////
ZDummyCharacter::ZDummyCharacter() : ZMyCharacter()
{
	// 랜덤으로 아무거나 입도록 만든다
#define _DUMMY_CHARACTER_PRESET		5
	u32 nMeleePreset[_DUMMY_CHARACTER_PRESET] = { 1, 11, 3, 14, 15 };
	u32 nPrimaryPreset[_DUMMY_CHARACTER_PRESET] = { 4010, 4013, 5004, 6004, 9001 };
	u32 nSecondaryPreset[_DUMMY_CHARACTER_PRESET] = { 9003, 9004, 9006, 7002, 6006 };
	u32 nChestPreset[_DUMMY_CHARACTER_PRESET] = { 21001, 21002, 21004, 21005, 21006 };
	u32 nLegsPreset[_DUMMY_CHARACTER_PRESET] = { 23001, 23005, 23002, 23004, 23003 };
	u32 nHandsPreset[_DUMMY_CHARACTER_PRESET] = { 22001, 22002, 22003, 22004, 22501 };
	u32 nFeetPreset[_DUMMY_CHARACTER_PRESET] = { 24001, 24002, 24003, 24004, 24005 };


	static int m_stIndex = 0; m_stIndex++;

	MTD_CharInfo info;
	char szTempName[128]; sprintf_safe(szTempName, "noname%d", m_stIndex); strcpy_safe(info.szName, szTempName);
	info.nSex = 0;

	for (int j = 0; j < MMCIP_END; j++) info.nEquipedItemDesc[j] = 0;
	info.nEquipedItemDesc[MMCIP_MELEE] = nMeleePreset[RandomNumber(0, 4)];
	info.nEquipedItemDesc[MMCIP_PRIMARY] = nPrimaryPreset[RandomNumber(0, 4)];
	info.nEquipedItemDesc[MMCIP_SECONDARY] = nSecondaryPreset[RandomNumber(0, 4)];
	info.nEquipedItemDesc[MMCIP_CHEST] = nChestPreset[RandomNumber(0, 4)];
	info.nEquipedItemDesc[MMCIP_HANDS] = nHandsPreset[RandomNumber(0, 4)];
	info.nEquipedItemDesc[MMCIP_LEGS] = nLegsPreset[RandomNumber(0, 4)];
	info.nEquipedItemDesc[MMCIP_FEET] = nFeetPreset[RandomNumber(0, 4)];

	Create(info);
	SetVisible(true);

	m_Items.GetItem(MMCIP_PRIMARY)->InitBullet(999999);
	m_Items.GetItem(MMCIP_SECONDARY)->InitBullet(999999);
	m_Items.SelectWeapon(MMCIP_PRIMARY);
	m_Items.GetItem(MMCIP_PRIMARY)->Reload();


	m_fNextAniTime = 5.0f;
	m_fElapsedTime = 0.0f;
	m_fNextShotTime = 5.0f;
	m_fShotElapsedTime = 0.0f;
	m_fShotDelayElapsedTime = 0.0f;
	m_bShotting = false;
	m_bShotEnable = false;
}

#include "Physics.h"

void ZDummyCharacter::OnUpdate(float fDelta)
{
	ZCharacter::OnUpdate(fDelta);



	m_fElapsedTime += fDelta;
	m_fShotDelayElapsedTime += fDelta;
	m_fShotElapsedTime += fDelta;


	if (m_fElapsedTime >= m_fNextAniTime)
	{
		int nAni = rand() % ZC_STATE_LOWER_END;
		SetAnimationLower(ZC_STATE_LOWER(nAni));

		m_fNextAniTime = RandomNumber(2.0f, 6.0f);
		m_fElapsedTime = 0.0f;

	}


	if (m_fShotElapsedTime >= m_fNextShotTime)
	{
		m_bShotting = !m_bShotting;
		m_fNextShotTime = RandomNumber(3.0f, 10.0f);
		m_fShotElapsedTime = 0.0f;
	}

	if ((m_bShotEnable) && (m_bShotting))
	{
		if (m_fShotDelayElapsedTime >= ((float)m_Items.GetItem(MMCIP_PRIMARY)->GetDesc()->m_nDelay / 1000.0f))
		{
			m_Items.GetItem(MMCIP_PRIMARY)->Reload();
			float fShotTime = g_pGame->GetTime();
			g_pGame->OnPeerShot(m_UID, fShotTime, m_Position, m_Direction, GetItems()->GetSelectedWeaponParts());
			m_fShotDelayElapsedTime = 0.0f;
		}

	}
}


#endif // _PUBLISH

bool ZMyCharacter::IsGuardNonrecoilable() const
{
#ifdef GUARD_START_CAN_BLOCK
	return m_bGuard;
#else
	return m_bGuard && !m_bGuardStart;
#endif
}

bool ZMyCharacter::IsGuardRecoilable() const
{
#ifndef GUARD_CANCEL_FIX
	return IsGuardNonrecoilable();
#else
	return m_bGuard && !m_bGuardStart && !m_bGuardCancel;
#endif
}

void ZMyCharacter::ProcessDelayedWork()
{
	auto pred = [&](auto&& Item) {
		if (g_pGame->GetTime() > Item.fTime)
		{
			OnDelayedWork(Item);
			return true;
		}
		return false;
};
	m_DelayedWorkList.erase(std::remove_if(m_DelayedWorkList.begin(), m_DelayedWorkList.end(), pred), m_DelayedWorkList.end());
}

void ZMyCharacter::AddDelayedWork(float Time, ZDELAYEDWORK Work, void *Data)
{
	m_DelayedWorkList.push_back({ Time, Work, Data });
}

void ZMyCharacter::OnDelayedWork(ZDELAYEDWORKITEM& Item)
{
	switch (Item.nWork) {
	case ZDW_RECOIL:
	{
		for (ZCharacterManager::iterator itor = ZGetCharacterManager()->begin();
		itor != ZGetCharacterManager()->end(); ++itor)
		{
			ZCharacter* pTar = (*itor).second;
			if (this == pTar || pTar->IsDie()) continue;

			rvector diff = GetPosition() + m_Direction*10.f - pTar->GetPosition();
			diff.z *= .5f;
			float fDist = Magnitude(diff);

			if (fDist < 200.0f) {

				bool bCheck = false;

				if (ZApplication::GetGame()->GetMatch()->IsTeamPlay()) {
					if (IsTeam(pTar) == false) {
						bCheck = true;
					}
				}
				else {
					bCheck = true;
				}

				if (g_pGame->CheckWall(this, pTar) == true)
					bCheck = false;

				if (bCheck) {

					rvector fTarDir = pTar->GetPosition() - GetPosition();
					Normalize(fTarDir);
					float fDot = DotProduct(m_Direction, fTarDir);
					if (fDot > 0.1 && DotProduct(m_Direction, pTar->m_Direction) < 0)
					{
						if (pTar->IsGuardRecoilable())
							ShotBlocked();
					}
				}
			}
		}
	}
	break;
	case ZDW_UPPERCUT:
		if (m_bSkill) {
			MMatchWeaponType type = MWT_NONE;

			int sel_type = GetItems()->GetSelectedWeaponParts();
			ZItem* pSItem = GetItems()->GetSelectedWeapon();

			if (pSItem && pSItem->GetDesc())
				type = pSItem->GetDesc()->m_nWeaponType;

			if (type == MWT_KATANA || type == MWT_DOUBLE_KATANA) {
				ZPostSkill(g_pGame->GetTime(), ZC_SKILL_UPPERCUT, sel_type);
			}
		}
		break;
	case ZDW_DASH:
		if (m_bSkill) {
			MMatchWeaponType type = MWT_NONE;

			int sel_type = GetItems()->GetSelectedWeaponParts();
			ZItem* pSItem = GetItems()->GetSelectedWeapon();

			if (pSItem && pSItem->GetDesc())
				type = pSItem->GetDesc()->m_nWeaponType;

			if (type == MWT_DAGGER) {
				ZPostSkill(g_pGame->GetTime(), ZC_SKILL_DASH, sel_type);
			}
			else if (type == MWT_DUAL_DAGGER) {
				ZPostSkill(g_pGame->GetTime(), ZC_SKILL_DASH, sel_type);
			}
		}
		break;
	case ZDW_MASSIVE:
		Discharged();
		if (m_bSlash || m_bJumpSlash)
			ZPostSkill(g_pGame->GetTime(), ZC_SKILL_SPLASHSHOT, GetItems()->GetSelectedWeaponParts());
		break;
	case ZDW_RG_SLASH:
		ZPostRGSlash(m_Position, m_Direction, reinterpret_cast<int>(Item.Data));
		break;
	case ZDW_RG_MASSIVE:
		Discharged();
		if (m_bSlash || m_bJumpSlash)
			ZPostRGMassive(m_Position, m_Direction);
		break;
	case ZDW_RG_RECOIL:
	{
		rvector AttackerPosition = GetPosition();
		rvector AttackerDirection = GetDirection();

		rvector HAttackerDirection = AttackerDirection;
		HAttackerDirection.z = 0;
		Normalize(HAttackerDirection);

		auto* pTarget = static_cast<ZCharacter*>(Item.Data);

		rvector TargetPosition = pTarget->GetPosition();

		float dist = GetMeleeDistance(AttackerPosition, TargetPosition);
		if (dist > 240)
			return;

		rvector HDirToTarget = TargetPosition - AttackerPosition;
		HDirToTarget.z = 0;
		Normalize(HDirToTarget);

		if (DotProduct(HAttackerDirection, HDirToTarget) < 0.5) // angle > 60 degrees
			return;

		rvector TargetDirection = pTarget->GetDirection();

		if (DotProduct(AttackerDirection, TargetDirection) > 0) // angle < 90 degrees
			return;

		ShotBlocked();
	}
	break;
	default:
		MLog("Invalid delayed work %d\n", Item.nWork);
		assert(false);
	}
}

void ZMyCharacter::AddRecoilTarget(ZCharacter *pTarget)
{
	AddDelayedWork(ZGetGame()->GetTime() + 0.1, ZDW_RG_RECOIL, pTarget);
}

void ZMyCharacter::EnterCharge()
{
	if (!m_bCharging)
		ZPostReaction(g_pGame->GetTime(), ZR_CHARGING);

	m_bShot = false;
	m_nShot = 0;
	m_bCharging = true;
	m_bPlayDone = false;
	m_bEnterCharge = false;
}

void ZMyCharacter::ChargedShot()
{
	m_bSlash = true;
	m_bTumble = false;

	// Temporary fix for double massive bug.
	m_bCharged = false;

	SetVelocity(0, 0, 0);

	ReleaseLButtonQueue();

	if (ZGetGameClient()->GetMatchStageSetting()->IsVanillaMode())
	{
		AddDelayedWork(g_pGame->GetTime() + 0.25f, ZDW_RECOIL);
		AddDelayedWork(g_pGame->GetTime() + 0.3f, ZDW_MASSIVE);
	}
	else
	{
		AddDelayedWork(g_pGame->GetTime() + 0.3 + 0.15, ZDW_RG_MASSIVE);
	}
}

void ZMyCharacter::JumpChargedShot()
{
	m_bWallJump = false;
	m_bWallJump2 = false;
	m_bJumpSlash = true;
	m_bTumble = false;
	m_bCharged = false;
	ReleaseLButtonQueue();

	if (ZGetGameClient()->GetMatchStageSetting()->IsVanillaMode())
	{
		AddDelayedWork(g_pGame->GetTime() + 0.15f, ZDW_RECOIL);
		AddDelayedWork(g_pGame->GetTime() + 0.2f, ZDW_MASSIVE);
	}
	else
	{
		AddDelayedWork(g_pGame->GetTime() + 0.2 + 0.15, ZDW_RG_MASSIVE);
	}

	m_bJumpSlashTime = g_pGame->GetTime();
}

void ZMyCharacter::ShotBlocked()
{
	m_bStun = true;
	SetVelocity(0, 0, 0);
	m_nStunType = ZST_BLOCKED;
	m_bShot = false;
	m_bSlash = false;
	m_bJumpSlash = false;
}

void ZMyCharacter::Charged()
{
	m_bCharged = true;
	m_fChargedFreeTime = g_pGame->GetTime() + CHARGED_TIME;

	ZPostReaction(CHARGED_TIME, ZR_CHARGED);
}

void ZMyCharacter::OnMeleeGuardSuccess()
{
	m_fGuardStartTime = g_pGame->GetTime() - GUARD_DURATION;
	m_bGuardCancel = true;
	m_bCharged = true;
	m_fChargedFreeTime = g_pGame->GetTime() + COUNTER_CHARGED_TIME;

	ZPostReaction(COUNTER_CHARGED_TIME, ZR_CHARGED);

	ZCharacter::OnMeleeGuardSuccess();
}

void ZMyCharacter::Discharged()
{
	m_bCharged = false;
	ZPostReaction(g_pGame->GetTime(), ZR_DISCHARGED);
}

float ZMyCharacter::GetGravityConst()
{
	if (m_bWallHang && m_bHangSuccess) return 0;
	if (m_bShot) return 0;

	if (m_bBlastFall) return .7f;

	if (m_bWallJump)
	{
		if (m_nWallJumpDir == 1 || GetVelocity().z < 0)
			return 0;
		else
			return .1f;
	}

	if (m_bSlash)
	{
		MMatchItemDesc *pDesc = GetItems()->GetItem(MMCIP_MELEE)->GetDesc();
		if (pDesc->m_nWeaponType == MWT_DOUBLE_KATANA) {
			AniFrameInfo* pAniLow = m_pVMesh->GetFrameInfo(ani_mode_lower);
			if (pAniLow->m_nFrame < 160 * 11) return 0;
		}
	}

	if (m_bSkill) {
		MMatchItemDesc *pDesc = GetItems()->GetItem(MMCIP_MELEE)->GetDesc();
		if (pDesc->m_nWeaponType == MWT_DOUBLE_KATANA) {
			AniFrameInfo* pAniLow = m_pVMesh->GetFrameInfo(ani_mode_lower);
			if (pAniLow->m_nFrame < 160 * 20) return 0;
		}
	}

	return 1.f;
}

void ZMyCharacter::OnGuardSuccess()
{
	m_fLastShotTime = g_pGame->GetTime();
}

void ZMyCharacter::OnDamaged(ZObject* pAttacker, rvector srcPos, ZDAMAGETYPE damageType, MMatchWeaponType weaponType, float fDamage, float fPiercingRatio, int nMeleeType)
{
	const auto GameType = ZGetGame()->GetMatch()->GetMatchType();
	if (GameType == MMATCH_GAMETYPE_SKILLMAP)
		return;

	if (GameType == MMATCH_GAMETYPE_TRAINING &&
		GetRGMain().TrainingSettings.Godmode)
	{
		if (damageType == ZD_MELEE)
		{
			OnDamagedAnimation(pAttacker, nMeleeType);
		}
		return;
	}

	ZCharacter::OnDamaged(pAttacker, srcPos, damageType, weaponType, fDamage, fPiercingRatio, nMeleeType);
	ZGetScreenEffectManager()->AddAlert(GetPosition(), m_Direction, srcPos);

	if (damageType == ZD_EXPLOSION)
	{
		if (GetVelocity().z > 0 && pAttacker != NULL)
			SetLastThrower(pAttacker->GetUID(), g_pGame->GetTime() + 1.0f);
	}

	LastDamagedTime = ZGetGame()->GetTime();
}

void ZMyCharacter::OnKnockback(const rvector& dir, float fForce)
{
	const auto GameType = ZGetGame()->GetMatch()->GetMatchType();
	if (GameType == MMATCH_GAMETYPE_SKILLMAP)
		return;

	if (GameType == MMATCH_GAMETYPE_TRAINING &&
		GetRGMain().TrainingSettings.NoStuns)
		return;

	if (m_bBlast || m_bBlastFall) {
		rvector vKnockBackDir = dir;
		Normalize(vKnockBackDir);
		vKnockBackDir *= (fForce * BLASTED_KNOCKBACK_RATIO);
		vKnockBackDir.x = vKnockBackDir.x * 0.2f;
		vKnockBackDir.y = vKnockBackDir.y * 0.2f;
		SetVelocity(vKnockBackDir);
	}
	else {
		ZCharacter::OnKnockback(dir, fForce);
	}
}

void ZMyCharacter::ReleaseButtonState()
{
	m_bLButtonFirstPressed = false;
	m_bLButtonPressed = false;

	m_bRButtonFirstReleased = true;
	m_bRButtonFirstPressed = false;
	m_bRButtonPressed = false;

	m_bLButtonQueued = false;
}

void ZMyCharacter::OnStun(float fTime)
{
	m_fStunEndTime = g_pGame->GetTime() + fTime;
	m_bStun = true;
	m_nStunType = ZST_LOOP;
}
