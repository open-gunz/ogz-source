#include "stdafx.h"
#include "MMatchRuleSurvival.h"
#include "MBlobArray.h"

void MMatchRuleSurvival::OnBegin()
{
	MMatchRuleBaseQuest::OnBegin();

	m_QuestRound.Reset();
	m_bReservedNextRound = false;
}

void MMatchRuleSurvival::OnEnd()
{
	MMatchRuleBaseQuest::OnEnd();
}

bool MMatchRuleSurvival::OnRun()
{
	bool ret = MMatchRuleBaseQuest::OnRun();
	if (ret == false) return false;

	if (GetRoundState() == MMATCH_ROUNDSTATE_PLAY)
	{
		ProcessRound();
	}

	return true;
}



void MMatchRuleSurvival::OnCommand(MCommand* pCommand)
{
	MMatchRuleBaseQuest::OnCommand(pCommand);
}

MMatchRuleSurvival::MMatchRuleSurvival(MMatchStage* pStage) : MMatchRuleBaseQuest(pStage),
							m_nRountStartTime(0), m_nReversedNextRoundTime(0), m_bReservedNextRound(false)
{

}

MMatchRuleSurvival::~MMatchRuleSurvival()
{

}


void MMatchRuleSurvival::ProcessRound()
{
	ProcessNPCSpawn();

	if (!m_bReservedNextRound)
	{
		if ((m_nNPCSpawnCount >= m_QuestRound.ClearConditionNPCCount(m_nFirstPlayerCount)) && 
			(m_NPCManager.GetNPCObjectCount() <= 0))
		{
			m_nReversedNextRoundTime = GetGlobalTimeMS();
			m_bReservedNextRound = true;
		}
	}
	else 
	{
		if ((GetGlobalTimeMS() - m_nReversedNextRoundTime) > 5000)
		{
			m_bReservedNextRound = false;
			QuestRoundStart();
		}
	}
}


void MMatchRuleSurvival::QuestRoundStart()
{
	RefreshPlayerStatus();

	m_nNPCSpawnCount=0;
	m_QuestRound.Increase();
	m_nRountStartTime = GetGlobalTimeMS();

	m_pStage->m_WorldItemManager.OnRoundBegin();

	RouteQuestRoundStart();
}


void MMatchRuleSurvival::RouteQuestRoundStart()
{
	MCommand* pCmd = MMatchServer::GetInstance()->CreateCommand(MC_QUEST_ROUND_START, MUID(0,0));
	pCmd->AddParameter(new MCmdParamUChar((unsigned char)m_QuestRound.GetRound()));
	MMatchServer::GetInstance()->RouteToStage(GetStage()->GetUID(), pCmd);

}


bool MMatchRuleSurvival::CheckNPCSpawnEnable()
{
	return false;

	auto nNowTime = GetGlobalTimeMS();

	if (m_nNPCSpawnCount >= m_QuestRound.ClearConditionNPCCount(m_nFirstPlayerCount)) return false;

	const u32 NPC_SPAWN_ROUND_START_DELAY = 3000;
	if ((nNowTime - m_nRountStartTime) <= NPC_SPAWN_ROUND_START_DELAY) return false;

	if ((int)(nNowTime - m_nLastNPCSpawnTime) >= m_nSpawnTime)
	{
		int nMaxNPC = m_QuestRound.MaxCurrNPCCount(m_nFirstPlayerCount);

		if (m_NPCManager.GetNPCObjectCount() < nMaxNPC)
		{
			m_nLastNPCSpawnTime = nNowTime;
			return true;
		}
	}

	return false;
}


void MMatchRuleSurvival::ProcessNPCSpawn()
{
	if (CheckNPCSpawnEnable())
	{
		m_nSpawnTime = m_QuestRound.SpawnTime();

		MQUEST_NPC nNPCType = m_QuestRound.RandomNPC();
		int nPosIndex = m_QuestRound.GetRandomSpawnPosIndex();

		SpawnNPC(nNPCType, nPosIndex);
	}
}


void MMatchRuleSurvival::RouteGameInfo()
{
	MCommand* pCmd = MMatchServer::GetInstance()->CreateCommand(MC_QUEST_GAME_INFO, MUID(0,0));

	void* pBlobGameInfoArray = MMakeBlobArray(sizeof(MTD_QuestGameInfo), 1);
	MTD_QuestGameInfo* pGameInfoNode = (MTD_QuestGameInfo*)MGetBlobArrayElement(pBlobGameInfoArray, 0);

	// test sample
	memset(pGameInfoNode, 0, sizeof(pGameInfoNode));
	pGameInfoNode->nNPCInfoCount = 2;
	pGameInfoNode->nNPCInfo[0] = NPC_GOBLIN;
	pGameInfoNode->nNPCInfo[1] = NPC_GOBLIN_GUNNER;
	pGameInfoNode->nMapSectorCount = 3;
	pGameInfoNode->nMapSectorID[0] = 0;
	pGameInfoNode->nMapSectorID[1] = 1;
	pGameInfoNode->nMapSectorID[2] = 0;


	pCmd->AddParameter(new MCommandParameterBlob(pBlobGameInfoArray, MGetBlobArraySize(pBlobGameInfoArray)));
	MEraseBlobArray(pBlobGameInfoArray);

	MMatchServer::GetInstance()->RouteToStage(GetStage()->GetUID(), pCmd);
}

void MMatchRuleSurvival::RouteStageGameInfo()
{

}

void MMatchRuleSurvival::DistributeReward()
{

}
