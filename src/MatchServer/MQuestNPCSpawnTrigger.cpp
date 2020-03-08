#include "stdafx.h"
#include "MQuestNPCSpawnTrigger.h"
#include "MMath.h"
#include "MQuestNPC.h"
#include "MQuestConst.h"


MQuestNPCSpawnTrigger::MQuestNPCSpawnTrigger() : m_nLastTime(0), m_fRateSum(0.0f)
{
	
}

MQuestNPCSpawnTrigger::~MQuestNPCSpawnTrigger()
{

}

void MQuestNPCSpawnTrigger::BuildNPCInfo(SpawnTriggerNPCInfoNode& NPCInfo)
{
	m_NPCInfo.push_back(NPCInfo);
	m_fRateSum += NPCInfo.fRate;
}

void MQuestNPCSpawnTrigger::BuildCondition(SpawnTriggerInfo& Info)
{
	m_Info = Info;
}


bool MQuestNPCSpawnTrigger::CheckSpawnEnable(int nCurrNPCCount)
{
	auto nNowTime = GetGlobalTimeMS();

	if (nNowTime - m_nLastTime < m_Info.nSpawnTickTime) return false;

	m_nLastTime = nNowTime;


	if ((nCurrNPCCount >= m_Info.nCurrMinNPCCount) && (nCurrNPCCount <= m_Info.nCurrMaxNPCCount))
	{
		MakeSpawnNPCs();

		return true;
	}

	return false;
}

void MQuestNPCSpawnTrigger::MakeSpawnNPCs()
{
	m_NPCQueue.clear();
	m_NPCQueue.reserve(m_Info.nSpawnNPCCount);

	for (int i = 0; i < m_Info.nSpawnNPCCount; i++)
	{

		m_NPCQueue.push_back(GetRandomNPC());
	}
}

void MQuestNPCSpawnTrigger::Clear()
{
	m_NPCInfo.clear();
	m_NPCQueue.clear();
	memset(&m_Info, 0, sizeof(SpawnTriggerInfo));
	m_nLastTime = 0;
	m_fRateSum = 0.0f;
}

MQUEST_NPC MQuestNPCSpawnTrigger::GetRandomNPC()
{
	float fRandNum = RandomNumber(0.001f, m_fRateSum);
	float f = 0.0f;

	for (vector<SpawnTriggerNPCInfoNode>::iterator itor = m_NPCInfo.begin(); itor != m_NPCInfo.end(); ++itor)
	{
		f += (*itor).fRate;

		if (fRandNum <= f) return (*itor).nNPCID;
	}

	MQUEST_NPC nDefaultNPC = NPC_GOBLIN;
	if (!m_NPCInfo.empty()) nDefaultNPC = (*m_NPCInfo.begin()).nNPCID;

	return nDefaultNPC;
}