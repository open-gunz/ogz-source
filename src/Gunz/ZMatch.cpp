#include "stdafx.h"

#include "ZMatch.h"
#include "ZGame.h"
#include "ZCharacterManager.h"
#include "ZCharacter.h"
#include "ZPost.h"
#include "ZGameInterface.h"
#include "ZApplication.h"
#include "ZGameClient.h"
#include "ZScreenEffectManager.h"
#include "ZActionDef.h"
#include "ZWorldItem.h"
#include "ZApplication.h"
#include "ZRule.h"
#include "ZMapDesc.h"
#include "MMatchGlobal.h"
#include "ZRuleDuel.h"
#include "ZInput.h"

#define READY_COUNT	5
#define ZDELAY_AFTER_DYING		RESPAWN_DELAYTIME_AFTER_DYING
#define ZDELAY_AFTER_DYING_MAX	20000	

ZMatch::ZMatch() : m_pStageSetting{ ZGetGameClient()->GetMatchStageSetting() } {}

bool ZMatch::Create()
{
	m_nNowTime = 0;
	m_nCurrRound = 0;
	memset(m_nTeamScore, 0, sizeof(m_nTeamScore));
	memset(m_nTeamKillCount, 0, sizeof(m_nTeamKillCount));

	m_pRule = ZRule::CreateRule(this, GetMatchType());

	return true;
}

void ZMatch::Destroy()
{
	if (m_pRule)
	{
		delete m_pRule; m_pRule = NULL;
	}

}

void ZMatch::SetRound(int nCurrRound)
{
	m_nCurrRound = nCurrRound;
}


void ZMatch::Update(float fDelta)
{
	m_nNowTime = GetGlobalTimeMS();

	switch (GetRoundState())
	{
	case MMATCH_ROUNDSTATE_COUNTDOWN:
		{
			if ((m_nNowTime - m_nStartTime) > ((READY_COUNT+1) * 1000))
			{
				m_nStartTime = m_nNowTime;
			}
		}
		break;
	case MMATCH_ROUNDSTATE_PLAY:
		{
			ProcessRespawn();
		}
		break;
	}

	if (m_pRule) m_pRule->Update(fDelta);
}


void ZMatch::ProcessRespawn()
{
#ifdef _QUEST
	if (ZGetGameTypeManager()->IsQuestDerived(GetMatchType())) return;
#endif

	if (!IsWaitForRoundEnd() && g_pGame->m_pMyCharacter)
	{
		static bool bLastDead = false;
		if (g_pGame->m_pMyCharacter->IsDead())
		{
			if (bLastDead == false)
			{
				m_nLastDeadTime = m_nNowTime;
			}

			m_nSoloSpawnTime = m_nNowTime - m_nLastDeadTime;

			if (m_nSoloSpawnTime >= ZDELAY_AFTER_DYING_MAX)
			{
				SoloSpawn();
			}
			else if (m_nSoloSpawnTime >= ZDELAY_AFTER_DYING)
			{
				static bool st_bCapturedActionKey = false;
				bool bNow = ZIsActionKeyDown(ZACTION_USE_WEAPON) ||
					ZIsActionKeyDown(ZACTION_JUMP);

				if ((st_bCapturedActionKey == true) && (bNow == false))
				{
					SoloSpawn();

				}

				st_bCapturedActionKey = bNow;
			}


		}

		bLastDead = g_pGame->m_pMyCharacter->IsDead();
	}

}

int ZMatch::GetRoundReadyCount()
{
	return ( READY_COUNT - (GetGlobalTimeMS() - m_nStartTime) / 1000 );
}

void ZMatch::OnDrawGameMessage() {}

void ZMatch::SoloSpawn()
{
	auto MyTeam = ZApplication::GetGame()->m_pMyCharacter->GetTeamID();
	if (GetMatchType() == MMATCH_GAMETYPE_DUEL || MyTeam == MMT_SPECTATOR) return;

	rvector pos = rvector(0.0f, 0.0f, 0.0f);
	rvector dir = rvector(0.0f, 1.0f, 0.0f);

	ZMapSpawnData* pSpawnData;
		
	if (IsTeamPlay())
	{
		int nSpawnIndex[2] = { 0, 0 };
		for (int i = 0; i < 2; i++)
			if (MyTeam == MMT_RED + i)
				pSpawnData = g_pGame->GetMapDesc()->GetSpawnManager()->GetTeamData(i, nSpawnIndex[i]);
	}
	else
		pSpawnData = ZApplication::GetGame()->GetMapDesc()->GetSpawnManager()->GetSoloRandomData();
		
	if (!pSpawnData)
	{
		if (ZGetApplication()->IsDeveloperMode())
			ZPostSpawn(v3{ 0, 0, 100 }, v3{ 1, 0, 0 });
		else
			assert(false);
	}
	else
	{
		if (ZGetApplication()->IsDeveloperMode()) {
			ZPostSpawn(pSpawnData->m_Pos, pSpawnData->m_Dir);
		} else {
			if (g_pGame->GetSpawnRequested() == false) {
				ZPostRequestSpawn(ZGetMyUID(), pSpawnData->m_Pos, pSpawnData->m_Dir);
				g_pGame->SetSpawnRequested(true);
			}
		}	
	}
		
	m_nSoloSpawnTime = -1;
}



void ZMatch::InitCharactersPosition()
{
	if (IsTeamPlay())
	{
		int nSpawnIndex[] = { 0, 0 };

		for (auto& pair : g_pGame->m_CharacterManager)
		{
			ZCharacter* pCharacter = pair.second;
			for (size_t i = 0; i < std::size(nSpawnIndex); i++)
			{
				if (pCharacter->GetTeamID() == MMT_RED + i)
				{
					ZMapSpawnData* pSpawnData = g_pGame->GetMapDesc()->GetSpawnManager()->GetTeamData(i, nSpawnIndex[i]);
					if (pSpawnData != NULL)
					{
						pCharacter->SetPosition(pSpawnData->m_Pos);
						pCharacter->SetDirection(pSpawnData->m_Dir);

						nSpawnIndex[i]++;
					}
				}
			}
		}

		return;
	}

	if (ZApplication::GetGame()->GetMatch()->GetMatchType() == MMATCH_GAMETYPE_DUEL)
	{
		ZRuleDuel* pDuel = (ZRuleDuel*)ZGetGameInterface()->GetGame()->GetMatch()->GetRule();
		if (pDuel)
		{
			int nIndex = 2;
			if (pDuel->QInfo.m_uidChampion == ZGetMyUID())
				nIndex = 0;
			else if (pDuel->QInfo.m_uidChallenger == ZGetMyUID())
				nIndex = 1;

			if (MIsMapOnlyDuel(ZGetGameClient()->GetMatchStageSetting()->GetMapIndex()))
			{
				ZMapSpawnData* pSpawnData = g_pGame->GetMapDesc()->GetSpawnManager()->GetData(nIndex);
				if (pSpawnData != NULL)
				{
					g_pGame->m_pMyCharacter->SetPosition(pSpawnData->m_Pos);
					g_pGame->m_pMyCharacter->SetDirection(pSpawnData->m_Dir);
				}
			}
			else
			{
				ZMapSpawnData* pSpawnData = g_pGame->GetMapDesc()->GetSpawnManager()->GetTeamData(nIndex, 0);
				if (pSpawnData != NULL)
				{
					g_pGame->m_pMyCharacter->SetPosition(pSpawnData->m_Pos);
					g_pGame->m_pMyCharacter->SetDirection(pSpawnData->m_Dir);
				}
			}

			return;
		}
	}

	ZMapSpawnData* pSpawnData = g_pGame->GetMapDesc()->GetSpawnManager()->GetSoloRandomData();

	v3 pos{0, 0, 100}, dir{ 1, 0, 0 };
	if (pSpawnData)
	{
		pos = pSpawnData->m_Pos;
		dir = pSpawnData->m_Dir;
	}

	g_pGame->m_pMyCharacter->SetPosition(pos);
	g_pGame->m_pMyCharacter->SetDirection(dir);
}

void ZMatch::InitRound()
{
	g_pGame->InitRound();

	InitCharactersPosition();
	InitCharactersProperties();

	ZGetWorldItemManager()->Reset();

	rvector pos = g_pGame->m_pMyCharacter->GetPosition();
	rvector dir = g_pGame->m_pMyCharacter->GetLowerDir();

	m_nRoundKills = 0;

	bool isObserver = false;

	if (ZApplication::GetInstance()->GetLaunchMode() == ZApplication::ZLAUNCH_MODE_DEBUG) {
		ZPostSpawn(pos, dir);
	} else {
		if (g_pGame->GetSpawnRequested() == false) {
			if (GetMatchType() == MMATCH_GAMETYPE_DUEL)
			{
				for (ZCharacterManager::iterator itor = g_pGame->m_CharacterManager.begin();
					itor != g_pGame->m_CharacterManager.end(); ++itor)
				{
					ZCharacter* pCharacter = (*itor).second;
					pCharacter->ForceDie();
					pCharacter->SetVisible(false);
				}
			}
			else if (g_pGame->m_pMyCharacter->GetTeamID() != MMT_SPECTATOR)
			{
				ZPostRequestSpawn(ZGetMyUID(), pos, dir);
				g_pGame->SetSpawnRequested(true);
			}
		}
	}

	// AdminHide
	MMatchObjCache* pObjCache = ZGetGameClient()->FindObjCache(ZGetMyUID());
	if (pObjCache && pObjCache->CheckFlag(MTD_PlayerFlags_AdminHide) ||
		ZGetGame()->m_pMyCharacter->GetTeamID() == MMT_SPECTATOR) {
		ZGetGameInterface()->GetCombatInterface()->SetObserverMode(true);
	} else {
		if (!isObserver)
			g_pGame->ReleaseObserver();
		else
		{
			ZGetGameInterface()->GetCombatInterface()->SetObserverMode(true);
			g_pGame->ReserveObserver();
			g_pGame->m_pMyCharacter->ForceDie();

		}
	}
	memset(m_nTeamKillCount, 0, sizeof(m_nTeamKillCount));
}


void ZMatch::InitCharactersProperties()
{
	for (auto* pCharacter : MakePairValueAdapter(g_pGame->m_CharacterManager))
	{
		pCharacter->InitStatus();
		pCharacter->SetVisible(true);
	}
}

void ZMatch::SetRoundState(MMATCH_ROUNDSTATE nRoundState, int nArg)
{
	if (m_nRoundState == nRoundState) return;
	m_nRoundState = nRoundState;

#ifndef _PUBLISH
	char szLog[128];
	sprintf_safe(szLog, "RoundState:%d À¸·Î ¹Ù²ñ\n", m_nRoundState);
	OutputDebugString(szLog);
#endif

	switch(m_nRoundState) 
	{
	case MMATCH_ROUNDSTATE_COUNTDOWN : 
		{
			InitRound();
		}
		break;
	case MMATCH_ROUNDSTATE_FINISH:
		{
			g_pGame->FlushObserverCommands();

			if (GetMatchType() != MMATCH_GAMETYPE_DUEL)
			{
				if (m_nCurrRound+1 >= GetRoundCount())
				{
					ZGetGameInterface()->FinishGame();
				}
			}
			else
			{
				for (ZCharacterManager::iterator itor = g_pGame->m_CharacterManager.begin();
					itor != g_pGame->m_CharacterManager.end(); ++itor)
				{
					ZCharacter* pCharacter = (*itor).second;
					if (pCharacter->GetKills() >= GetRoundCount())
					{
						ZGetGameInterface()->FinishGame();
						break;
					}
				}				
			}

			if (IsTeamPlay())
			{
				if (nArg != MMATCH_ROUNDRESULT_DRAW)
				{
					MMatchTeam nTeamWon = (nArg == MMATCH_ROUNDRESULT_REDWON ? MMT_RED : MMT_BLUE);
					if (nTeamWon == MMT_RED)
						m_nTeamScore[MMT_RED]++;
					else if (nTeamWon == MMT_BLUE)
						m_nTeamScore[MMT_BLUE]++;
				}
			}
		}
		break;
	};
}

const char* ZMatch::GetTeamName(int nTeamID)
{
	switch (nTeamID)
	{
	case MMT_SPECTATOR:
		return MMATCH_SPECTATOR_STR;
	case MMT_RED:
		return MMATCH_TEAM1_NAME_STR;
	case MMT_BLUE:
		return MMATCH_TEAM2_NAME_STR;
	default:
		return "";
	}
	return "";
}


int ZMatch::GetRoundCount()
{
	return m_pStageSetting->GetStageSetting()->nRoundMax;
}


void ZMatch::GetTeamAliveCount(int* pnRedTeam, int* pnBlueTeam)
{
	int nRedTeam = 0, nBlueTeam = 0;
	if (IsTeamPlay())
	{
		for (ZCharacterManager::iterator itor = g_pGame->m_CharacterManager.begin();
			itor != g_pGame->m_CharacterManager.end(); ++itor)
		{
			ZCharacter* pCharacter = (*itor).second;
			if (!pCharacter->IsDead())
			{
				if (pCharacter->GetTeamID() == 0)
				{
					nRedTeam++;
				}
				else
				{
					nBlueTeam++;
				}
			}
		}
	}

	*pnRedTeam = nRedTeam;
	*pnBlueTeam = nBlueTeam;
}

void ZMatch::RespawnSolo(bool bForce)
{
	if ((!IsWaitForRoundEnd() && g_pGame->m_pMyCharacter->IsDead()) || bForce)
	{
		SoloSpawn();
	}
}

void ZMatch::OnForcedEntry(ZCharacter* pCharacter)
{
	if (!pCharacter)
	{
		assert(false);
		return;
	}

	if (pCharacter == ZApplication::GetGame()->m_pMyCharacter)
	{
		// AdminHide
		MMatchObjCache* pObjCache = ZGetGameClient()->FindObjCache(ZGetMyUID());
		if (pObjCache && pObjCache->CheckFlag(MTD_PlayerFlags_AdminHide)) {
			ZGetGameInterface()->GetCombatInterface()->SetObserverMode(true);
		} else {
			if (IsWaitForRoundEnd())
			{
				pCharacter->SetVisible(false);
				pCharacter->ForceDie();
				ZApplication::GetGameInterface()->GetCombatInterface()->SetObserverMode(true);
			}	
			else
			{
				InitRound();
			}
		}
	}
	else
	{
		if (IsWaitForRoundEnd() && (GetRoundState() != MMATCH_ROUNDSTATE_FREE))
		{
			pCharacter->SetVisible(false);
			pCharacter->ForceDie();
		}
	}
}

int ZMatch::GetRemainedSpawnTime()
{
	int nTimeSec = -1;

	if (GetRoundState() == MMATCH_ROUNDSTATE_PLAY)
	{
		if (!IsWaitForRoundEnd())
		{
			if (g_pGame->m_pMyCharacter->IsDead())
			{
				if (m_nSoloSpawnTime < 0) return -1;
				int nElapsedTime = m_nSoloSpawnTime;
				if (nElapsedTime > ZDELAY_AFTER_DYING) nElapsedTime = ZDELAY_AFTER_DYING;

				nTimeSec = (((ZDELAY_AFTER_DYING - nElapsedTime)+999) / 1000);
			}
		}
	}

	return nTimeSec;
}

void ZMatch::SetRoundStartTime()
{
	m_dwStartTime = GetGlobalTimeMS();
}

DWORD ZMatch::GetRemaindTime()
{
	return ( GetGlobalTimeMS() - m_dwStartTime);
}

bool ZMatch::OnCommand(MCommand* pCommand)
{
	if (m_pRule) return m_pRule->OnCommand(pCommand);
	return false;
}

void ZMatch::OnResponseRuleInfo(MTD_RuleInfo* pInfo)
{
	if (pInfo->nRuleType != GetMatchType()) return;

	if (m_pRule)
	{
		m_pRule->OnResponseRuleInfo(pInfo);
	}
}