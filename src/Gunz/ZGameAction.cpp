#include "stdafx.h"
#include "ZGameAction.h"
#include "ZGame.h"
#include "ZGameClient.h"
#include "ZEffectManager.h"
#include "ZApplication.h"
#include "ZSoundEngine.h"
#include "ZMyCharacter.h"
#include "ZPost.h"
#include "ZModule_ElementalDamage.h"

#define MAX_ENCHANT_DURATION	10.f

bool ZGameAction::OnCommand(MCommand* pCommand)
{
	switch (pCommand->GetID())
	{
		HANDLE_COMMAND(MC_PEER_ENCHANT_DAMAGE	,OnEnchantDamage)
		HANDLE_COMMAND(MC_PEER_REACTION			,OnReaction)
		HANDLE_COMMAND(MC_PEER_SKILL			,OnPeerSkill)
	}

	return false;
}

bool ZGameAction::OnReaction(MCommand* pCommand)
{
	float fTime;
	int nReactionID;

	pCommand->GetParameter(&fTime,			0, MPT_FLOAT);		// 시간
	pCommand->GetParameter(&nReactionID,	1, MPT_INT);

	ZCharacter *pChar=ZGetCharacterManager()->Find(pCommand->GetSenderUID());
	if(!pChar) return true;

	switch(nReactionID)
	{
		case ZR_CHARGING	: {
			pChar->m_bCharging=true;
			if(!pChar->IsHero())
				pChar->SetAnimationLower(ZC_STATE_CHARGE);
			ZGetEffectManager()->AddChargingEffect(pChar);
		}break;
		case ZR_CHARGED		: {
			pChar->m_bCharged=true;
			pChar->m_fChargedFreeTime = g_pGame->GetTime() + fTime;
			ZGetEffectManager()->AddChargedEffect(pChar);

			ZGetSoundEngine()->PlaySound("fx2/FX_ChargeComplete", pChar->GetPosition());
		}break;
		case ZR_BE_UPPERCUT	: {
			rvector tpos = pChar->GetPosition();
			tpos.z += 130.f;
			ZGetEffectManager()->AddSwordUppercutDamageEffect(tpos,pChar->GetUID());
			ZGetSoundEngine()->PlaySound("uppercut", tpos);
		}break;
		case ZR_DISCHARGED	: {
			pChar->m_bCharged=false;
		}break;
	}

	return true;
}

bool ZGameAction::OnPeerSkill(MCommand* pCommand)
{
	float fTime;
	int nSkill,sel_type;

	pCommand->GetParameter(&fTime, 0, MPT_FLOAT);
	pCommand->GetParameter(&nSkill, 1, MPT_INT);
	pCommand->GetParameter(&sel_type, 2, MPT_INT);

	ZCharacter* pOwnerCharacter = ZGetCharacterManager()->Find(pCommand->GetSenderUID());
	if (pOwnerCharacter == NULL) return true;

	switch(nSkill)	{
		// 띄우기 스킬
		case ZC_SKILL_UPPERCUT		: OnPeerSkill_Uppercut(pOwnerCharacter);break;
			// 강베기 스플래시
		case ZC_SKILL_SPLASHSHOT	: OnPeerSkill_LastShot(fTime,pOwnerCharacter);break;
			// 단검 특수공격
		case ZC_SKILL_DASH			: OnPeerSkill_Dash(pOwnerCharacter);break;
	}

	return true;
}

void ZGameAction::OnPeerSkill_LastShot(float fShotTime,ZCharacter *pOwnerCharacter)
{
	if (!pOwnerCharacter) return;
	ZItem *pItem = pOwnerCharacter->GetItems()->GetItem(MMCIP_MELEE);
	if (!pItem) return;

	MMatchItemDesc* pDesc = pItem->GetDesc();
	if (!pDesc) return;

	const float fRange = 300.f;

	fShotTime -= pOwnerCharacter->GetTimeOffset();

	rvector OwnerPosition, OwnerDir;
	if (!pOwnerCharacter->GetHistory(&OwnerPosition, &OwnerDir, fShotTime))
		return;

	rvector waveCenter = OwnerPosition;

	rvector _vdir = OwnerDir;
	_vdir.z = 0;

	ZC_ENCHANT zc_en_type = pOwnerCharacter->GetEnchantType();

	ZGetSoundEngine()->PlaySound(pOwnerCharacter->IsObserverTarget() ? "we_smash_2d" : "we_smash", waveCenter);

	ZGetEffectManager()->AddSwordWaveEffect(pOwnerCharacter->GetUID(), waveCenter, pOwnerCharacter->GetDirection());

	for (ZObjectManager::iterator itor = ZGetObjectManager()->begin();
	itor != ZGetObjectManager()->end(); ++itor)
	{
		ZObject* pTar = (*itor).second;

		if (pTar == NULL) continue;
		if (pOwnerCharacter == pTar) continue;

		if (pTar != g_pGame->m_pMyCharacter &&
			(!pTar->IsNPC() || !((ZActor*)pTar)->IsMyControl())) continue;

		if (!ZGetGame()->IsAttackable(pOwnerCharacter, pTar)) continue;

		rvector TargetPosition, TargetDir;

		if (pTar->IsDie()) continue;

		if (!pTar->GetHistory(&TargetPosition, &TargetDir, fShotTime)) continue;

		rvector checkPosition = TargetPosition + rvector(0, 0, 80);
		float fDist = Magnitude(waveCenter - checkPosition);

		if (fDist < fRange) {

			if ((pTar) && (pTar != pOwnerCharacter)) {

				if (g_pGame->CheckWall(pOwnerCharacter, pTar) == false)
				{
					if (pTar->IsGuardNonrecoilable() && DotProduct(pTar->m_Direction, OwnerDir) < 0)
					{
						rvector addVel = pTar->GetPosition() - waveCenter;
						Normalize(addVel);
						addVel = 500.f*addVel;
						addVel.z = 200.f;
						pTar->AddVelocity(addVel);
					}
					else
					{
						rvector tpos = pTar->GetPosition();

						tpos.z += 130.f;

						if (zc_en_type == ZC_ENCHANT_NONE) {

							ZGetEffectManager()->AddSwordUppercutDamageEffect(tpos, pTar->GetUID());
						}
						else {

							ZGetEffectManager()->AddSwordEnchantEffect(zc_en_type, pTar->GetPosition(), 20);
						}

						tpos -= pOwnerCharacter->m_Direction * 50.f;

						rvector fTarDir = pTar->GetPosition() - pOwnerCharacter->GetPosition();
						Normalize(fTarDir);

#define MAX_DMG_RANGE	50.f
#define MIN_DMG			0.3f

						float fDamageRange = 1.f - (1.f - MIN_DMG)*(max(fDist - MAX_DMG_RANGE, 0.f) / (fRange - MAX_DMG_RANGE));

#define SPLASH_DAMAGE_RATIO	.4f
#define SLASH_DAMAGE	3
						int damage = (int)pDesc->m_nDamage * fDamageRange;

						if (zc_en_type == ZC_ENCHANT_NONE)
							damage *= SLASH_DAMAGE;

						pTar->OnDamaged(pOwnerCharacter, pOwnerCharacter->GetPosition(), ZD_KATANA_SPLASH, MWT_KATANA, damage, SPLASH_DAMAGE_RATIO);
						pTar->OnDamagedAnimation(pOwnerCharacter, SEM_WomanSlash5);

						ZPostPeerEnchantDamage(pOwnerCharacter->GetUID(), pTar->GetUID());
					}
				}
			}
		}
	}
#define KATANA_SHOCK_RANGE		1000.f

	float fPower = (KATANA_SHOCK_RANGE - Magnitude(g_pGame->m_pMyCharacter->GetPosition() + rvector(0, 0, 50) - OwnerPosition)) / KATANA_SHOCK_RANGE;
	if (fPower > 0)
		ZGetGameInterface()->GetCamera()->Shock(fPower*500.f, .5f, rvector(0.0f, 0.0f, -1.0f));
}

void ZGameAction::OnPeerSkill_Uppercut(ZCharacter *pOwnerCharacter)
{
	if (!ZGetGameClient()->GetMatchStageSetting()->CanFlip())
		return;
	if (ZGetGame()->GetMatch()->GetMatchType() == MMATCH_GAMETYPE_SKILLMAP)
		return;

	float fShotTime=g_pGame->GetTime();
	rvector OwnerPosition,OwnerDir;
	OwnerPosition = pOwnerCharacter->GetPosition();
	OwnerDir = pOwnerCharacter->m_Direction;
	OwnerDir.z=0; 
	Normalize(OwnerDir);

	if (!pOwnerCharacter->IsNPC())
	{
		if (pOwnerCharacter->GetProperty()->nSex == MMS_MALE)
			ZGetSoundEngine()->PlaySound("fx2/MAL_shot_01", pOwnerCharacter->GetPosition());
		else
			ZGetSoundEngine()->PlaySound("fx2/FEM_shot_01", pOwnerCharacter->GetPosition());
	}

	for (ZObjectManager::iterator itor = ZGetObjectManager()->begin();
		itor != ZGetObjectManager()->end(); ++itor)
	{
		ZObject* pTar = (*itor).second;
		if (pOwnerCharacter == pTar) continue;

		rvector TargetPosition,TargetDir;

		if(pTar->IsDie()) continue;

		if( !pTar->GetHistory(&TargetPosition,&TargetDir,fShotTime)) continue;

		float fDist = Magnitude(OwnerPosition + OwnerDir*10.f - TargetPosition);

		if (fDist < 200.0f) {

			if ((pTar) && (pTar != pOwnerCharacter))
			{
				bool bCheck = false;

				if (ZApplication::GetGame()->GetMatch()->IsTeamPlay())
				{
					if (IsPlayerObject(pTar)) {
						if( pOwnerCharacter->IsTeam( (ZCharacter*)pTar ) == false){
							bCheck = true;
						}
					}
					else {
						bCheck = true;
					}
				}
				else if (ZApplication::GetGame()->GetMatch()->IsQuestDrived())
				{
					if (!IsPlayerObject(pTar)) bCheck = true;
				}
				else {
					bCheck = true;
				}

				if(g_pGame->CheckWall(pOwnerCharacter,pTar)==true)
					bCheck = false;

				if( bCheck) {

					rvector fTarDir = pTar->GetPosition() - (pOwnerCharacter->GetPosition() - 50.f*OwnerDir);
					Normalize(fTarDir);
					float fDot = DotProduct(OwnerDir, fTarDir);
					if (fDot > 0)
					{
						int cm = g_pGame->SelectSlashEffectMotion(pOwnerCharacter);

						rvector tpos = pTar->GetPosition();

						tpos.z += 130.f;

						tpos -= pOwnerCharacter->m_Direction * 50.f;

						ZGetEffectManager()->AddBloodEffect( tpos , -fTarDir);
						ZGetEffectManager()->AddSlashEffect( tpos , -fTarDir , cm );

						g_pGame->CheckCombo(pOwnerCharacter, pTar , true);
						if (pTar == g_pGame->m_pMyCharacter) 
						{
							g_pGame->m_pMyCharacter->SetLastThrower(pOwnerCharacter->GetUID(), g_pGame->GetTime()+1.0f);
							ZPostReaction(g_pGame->GetTime(),ZR_BE_UPPERCUT);
						}
						pTar->OnBlast(OwnerDir);

						if (!pTar->IsNPC())
						{
							if (((ZCharacter*)pTar)->GetProperty()->nSex == MMS_MALE)
								ZGetSoundEngine()->PlaySound("fx2/MAL07", pTar->GetPosition());
							else
								ZGetSoundEngine()->PlaySound("fx2/FEM07", pTar->GetPosition());
						}
					}
				}
			}
		}
	}
}

void ZGameAction::OnPeerSkill_Dash(ZCharacter *pOwnerCharacter)
{
	if(pOwnerCharacter->GetStateLower() != ZC_STATE_LOWER_UPPERCUT) return;

	float fShotTime=g_pGame->GetTime();
	rvector OwnerPosition,OwnerDir;
	OwnerPosition = pOwnerCharacter->GetPosition();
	OwnerDir = pOwnerCharacter->m_Direction;
	OwnerDir.z=0; 
	Normalize(OwnerDir);

	ZItem *pItem = pOwnerCharacter->GetItems()->GetItem(MMCIP_MELEE);
	if(!pItem) return;
	MMatchItemDesc *pDesc = pItem->GetDesc();
	if(!pDesc) { _ASSERT(FALSE); return; }

//	ZGetEffectManager()->AddSkillDashEffect(pOwnerCharacter->GetPosition(),pOwnerCharacter->m_Direction,pOwnerCharacter);

//	for (ZCharacterManager::iterator itor = ZGetCharacterManager()->begin();
//		itor != ZGetCharacterManager()->end(); ++itor)
	for (ZObjectManager::iterator itor = ZGetObjectManager()->begin();
		itor != ZGetObjectManager()->end(); ++itor)
	{
		ZObject* pTar = (*itor).second;

		if (pOwnerCharacter == pTar) continue;

		rvector TargetPosition,TargetDir;

		if(pTar->IsDie()) continue;

		if( !pTar->GetHistory(&TargetPosition,&TargetDir,fShotTime)) continue;

		float fDist = Magnitude(OwnerPosition + OwnerDir*10.f - TargetPosition);

		if (fDist < 600.0f) {// 6m

			if ((pTar) && (pTar != pOwnerCharacter)) {

				bool bCheck = false;
				if (ZApplication::GetGame()->GetMatch()->IsTeamPlay()){
					if (IsPlayerObject(pTar)) {
						if( pOwnerCharacter->IsTeam( (ZCharacter*)pTar ) == false){
							bCheck = true;
						}
					}
					else {
						bCheck = true;
					}
				}
				else {
					bCheck = true;
				}

				if(g_pGame->CheckWall(pOwnerCharacter,pTar)==true)
					bCheck = false;

				if( bCheck) {
					rvector fTarDir = pTar->GetPosition() - pOwnerCharacter->GetPosition();
					Normalize(fTarDir);

					float fDot = DotProduct(OwnerDir, fTarDir);

					bool bDamage = false;

					if( fDist < 100.f) { // 1m
						if(fDot > 0.f) {
							bDamage = true;
						}
					}
					else if(fDist < 300.f) {
						if(fDot > 0.5f) {
							bDamage = true;
						}
					}
					else {// 2m ~ 6m
						if(fDot > 0.96f) {
							bDamage = true;
						}
					}

					if ( bDamage ) {

						int cm = g_pGame->SelectSlashEffectMotion(pOwnerCharacter);

						float add_time = 0.3f * (fDist / 600.f);
						float time = g_pGame->GetTime() + add_time;

						rvector tpos = pTar->GetPosition();

						tpos.z += 180.f;

						ZGetEffectManager()->AddSwordUppercutDamageEffect(tpos,pTar->GetUID(),(DWORD)(add_time*1000));

						tpos -= pOwnerCharacter->m_Direction * 50.f;

						ZGetSoundEngine()->PlaySound("uppercut", tpos );

						if (pTar == g_pGame->m_pMyCharacter) {
							rvector _dir = pTar->GetPosition() - pOwnerCharacter->GetPosition();
							_dir.z = 0.f;

							g_pGame->m_pMyCharacter->ReserveDashAttacked( pOwnerCharacter->GetUID(), time,_dir );
						}
						pTar->OnBlastDagger(OwnerDir,OwnerPosition);

						float fDamage = pDesc->m_nDamage * 1.5f;
						float fRatio = pItem->GetPiercingRatio( pDesc->m_nWeaponType , eq_parts_chest );

						if(ZGetGame()->IsAttackable(pOwnerCharacter,pTar))
							pTar->OnDamagedSkill(pOwnerCharacter,pOwnerCharacter->GetPosition(),ZD_MELEE,MWT_DAGGER,fDamage,fRatio);

						g_pGame->CheckCombo(pOwnerCharacter, pTar,true);
					}

				}//IsTeam
			}
		}
	}
}


bool ZGameAction::OnEnchantDamage(MCommand* pCommand)
{
	MUID ownerUID;
	MUID targetUID;
	pCommand->GetParameter(&ownerUID,	0, MPT_UID);
	pCommand->GetParameter(&targetUID,	1, MPT_UID);

	ZCharacter* pOwnerCharacter = ZGetCharacterManager()->Find(ownerUID);
	ZObject* pTarget= ZGetObjectManager()->GetObject(targetUID);

	if (pOwnerCharacter == NULL || pTarget == NULL ) return true;

	bool bMyChar = (targetUID == ZGetMyUID());
	
	MMatchItemDesc* pDesc = pOwnerCharacter->GetEnchantItemDesc();
	if(pDesc)
	{
		bool bObserverTarget = pTarget->GetUID()==ZGetCombatInterface()->GetTargetUID();
		rvector soundPos = pTarget->GetPosition();
		switch(pOwnerCharacter->GetEnchantType())
		{
			case ZC_ENCHANT_FIRE : {
					ZGetSoundEngine()->PlaySound(bObserverTarget ? "we_enfire_2d" : "we_enfire",soundPos);
					ZModule_FireDamage *pMod = (ZModule_FireDamage*)pTarget->GetModule(ZMID_FIREDAMAGE);
					if(pMod) pMod->BeginDamage( pOwnerCharacter->GetUID(), bMyChar ? pDesc->m_nDamage : 0 ,0.001f * (float)pDesc->m_nDelay);
				}break;
			case ZC_ENCHANT_COLD : {
				ZGetSoundEngine()->PlaySound(bObserverTarget ? "we_enice_2d" : "we_enice",soundPos);
					ZModule_ColdDamage *pMod = (ZModule_ColdDamage*)pTarget->GetModule(ZMID_COLDDAMAGE);
					if(pMod) pMod->BeginDamage( 0.01f*(float)pDesc->m_nLimitSpeed , 0.001f * (float)pDesc->m_nDelay);
				}break;
			case ZC_ENCHANT_POISON : {
					ZGetSoundEngine()->PlaySound(bObserverTarget ? "we_enpoison_2d" : "we_enpoison",soundPos);
					ZModule_PoisonDamage *pMod = (ZModule_PoisonDamage*)pTarget->GetModule(ZMID_POISONDAMAGE);
					if(pMod) pMod->BeginDamage( pOwnerCharacter->GetUID(), bMyChar ? pDesc->m_nDamage : 0 ,0.001f * (float)pDesc->m_nDelay);
				}break;
			case ZC_ENCHANT_LIGHTNING : {
					ZGetSoundEngine()->PlaySound(bObserverTarget ? "we_enlight_2d" : "we_enlight",soundPos);
					ZModule_LightningDamage *pMod = (ZModule_LightningDamage*)pTarget->GetModule(ZMID_LIGHTNINGDAMAGE);
					if(pMod) pMod->BeginDamage( pOwnerCharacter->GetUID(), bMyChar ? pDesc->m_nDamage : 0 ,0.001f * (float)pDesc->m_nDelay);
				}break;
		};
	}

	return true;
}