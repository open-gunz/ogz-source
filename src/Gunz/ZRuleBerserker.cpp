#include "stdafx.h"
#include "ZRuleBerserker.h"

#define BERSERKER_UPDATE_HEALTH_TIME		5.0f
#define BERSERKER_UPDATE_HEALTH				10
#define BERSERKER_BONUS_HEALTH				50

ZRuleBerserker::ZRuleBerserker(ZMatch* pMatch) : ZRule(pMatch), m_uidBerserker(0,0)
{

}

ZRuleBerserker::~ZRuleBerserker()
{


}

bool ZRuleBerserker::OnCommand(MCommand* pCommand)
{
	if (!g_pGame) return false;

	switch (pCommand->GetID())
	{
	case MC_MATCH_ASSIGN_BERSERKER:
		{
			MUID uidBerserker;
			pCommand->GetParameter(&uidBerserker,		0, MPT_UID);

			AssignBerserker(uidBerserker);
		}
		break;
	case MC_MATCH_GAME_DEAD:
		{
			MUID uidAttacker, uidVictim;
			u32 nAttackerArg, nVictimArg;

			pCommand->GetParameter(&uidAttacker, 0, MPT_UID);
			pCommand->GetParameter(&nAttackerArg, 1, MPT_UINT);
			pCommand->GetParameter(&uidVictim, 2, MPT_UID);
			pCommand->GetParameter(&nVictimArg, 3, MPT_UINT);


			bool bSuicide = false;
			if (uidAttacker == uidVictim) bSuicide = true;

			if ((uidAttacker != MUID(0,0)) && (uidAttacker == m_uidBerserker))
			{
				if (!bSuicide)
				{
					ZCharacter* pAttacker = g_pGame->m_CharacterManager.Find(uidAttacker);
					BonusHealth(pAttacker);
				}
			}
		}
		break;
	}

	return false;
}

void ZRuleBerserker::OnResponseRuleInfo(MTD_RuleInfo* pInfo)
{
	MTD_RuleInfo_Berserker* pBerserkerRule = (MTD_RuleInfo_Berserker*)pInfo;
	AssignBerserker(pBerserkerRule->uidBerserker);
}


void ZRuleBerserker::AssignBerserker(MUID& uidBerserker)
{
	if (!g_pGame) return;

	for (ZCharacterManager::iterator itor = g_pGame->m_CharacterManager.begin();
		itor != g_pGame->m_CharacterManager.end(); ++itor)
	{
		ZCharacter* pCharacter = (*itor).second;
		pCharacter->SetTagger(false);
	}

	ZCharacter* pBerserkerChar = g_pGame->m_CharacterManager.Find(uidBerserker);
	if (pBerserkerChar)
	{
		ZGetEffectManager()->AddBerserkerIcon(pBerserkerChar);
		pBerserkerChar->SetTagger(true);
		
		if (!pBerserkerChar->IsDead())
		{
			float fMaxHP = pBerserkerChar->GetProperty()->fMaxHP;
			float fMaxAP = pBerserkerChar->GetProperty()->fMaxAP;
			pBerserkerChar->SetHP(fMaxHP);
			pBerserkerChar->SetAP(fMaxAP);

			if ( uidBerserker == ZGetMyUID())
				ZGetGameInterface()->PlayVoiceSound( VOICE_GOT_BERSERKER, 1600);
			else
				ZGetGameInterface()->PlayVoiceSound( VOICE_BERSERKER_DOWN, 1200);
		}

	}

	m_uidBerserker = uidBerserker;
	m_fElapsedHealthUpdateTime = 0.0f;
}

void ZRuleBerserker::OnUpdate(float fDelta)
{
	m_fElapsedHealthUpdateTime += fDelta;

	if (BERSERKER_UPDATE_HEALTH_TIME < m_fElapsedHealthUpdateTime)
	{
		m_fElapsedHealthUpdateTime = 0.0f;

		ZCharacter* pBerserker = g_pGame->m_CharacterManager.Find(m_uidBerserker);
		PenaltyHealth(pBerserker);
	}
}

void ZRuleBerserker::BonusHealth(ZCharacter* pBerserker)
{
	if (pBerserker)
	{
		if (pBerserker->IsDead()) return;

		float fBonusAP = 0.0f;
		float fBonusHP = BERSERKER_BONUS_HEALTH;

		float fMaxHP = pBerserker->GetProperty()->fMaxHP;

		if ((fMaxHP - pBerserker->GetHP()) < BERSERKER_BONUS_HEALTH)
		{
			fBonusHP = fMaxHP - pBerserker->GetHP();
			fBonusAP = BERSERKER_BONUS_HEALTH - fBonusHP;
		}

		pBerserker->SetHP(pBerserker->GetHP() + fBonusHP);
		pBerserker->SetAP(pBerserker->GetAP() + fBonusAP);
	}

}

void ZRuleBerserker::PenaltyHealth(ZCharacter* pBerserker)
{
	if (pBerserker)
	{
		if (pBerserker->GetAP() > 0)
		{
			float fAP = max(0, pBerserker->GetAP() - BERSERKER_UPDATE_HEALTH);
			pBerserker->SetAP(fAP);
		}
		else
		{
			float fHP = max(1, pBerserker->GetHP() - BERSERKER_UPDATE_HEALTH);
            pBerserker->SetHP(fHP);
		}

		pBerserker->SetLastAttacker(pBerserker->GetUID());
	}
}