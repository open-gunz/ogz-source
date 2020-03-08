#include "stdafx.h"
#include "ZQuestGameInfo.h"

ZQuestGameInfo::ZQuestGameInfo() : m_bInitialized(false), m_fNPC_TC(1.0f), m_nCurrSectorIndex(0), m_nNPCCount(0)
{
}

ZQuestGameInfo::~ZQuestGameInfo()
{

}

void ZQuestGameInfo::Init(MTD_QuestGameInfo* pMTDQuestGameInfo)
{
	m_NPCInfoVector.clear();
	m_MapSectorVector.clear();
	m_Bosses.clear();


	for (int i = 0; i < pMTDQuestGameInfo->nNPCInfoCount; i++)
	{
		m_NPCInfoVector.push_back(MQUEST_NPC(pMTDQuestGameInfo->nNPCInfo[i]));
	}

	for (int i = 0; i < pMTDQuestGameInfo->nMapSectorCount; i++)
	{
		MQuestLevelSectorNode node;
		node.nSectorID = pMTDQuestGameInfo->nMapSectorID[i];
		node.nNextLinkIndex = pMTDQuestGameInfo->nMapSectorLinkIndex[i];
		m_MapSectorVector.push_back(node);
	}

	m_nQL = pMTDQuestGameInfo->nQL;
	m_nNPCCount = pMTDQuestGameInfo->nNPCCount;
	m_nNPCKilled = 0;
	m_fNPC_TC = pMTDQuestGameInfo->fNPC_TC;

#ifdef _DEBUG
	mlog("%d , %d\n", GetNPCInfoCount(), GetMapSectorCount());
	for (int i = 0; i < GetNPCInfoCount(); i++)
	{
		mlog("npc(%d)\n", (int)(GetNPCInfo(i)));
	}

	for (int i = 0; i < GetMapSectorCount(); i++)
	{
		mlog("map(%d)\n", GetMapSectorID(i));
	}
#endif

	m_nCurrSectorIndex = 0;
	m_nNumOfObtainQuestItem = 0;

	m_bInitialized = true;
}

void ZQuestGameInfo::Final()
{
	m_bInitialized = false;
}

void ZQuestGameInfo::OnMovetoNewSector(int nSectorIndex)
{
	m_nCurrSectorIndex = nSectorIndex;
	m_Bosses.clear();
}

MUID ZQuestGameInfo::GetBoss()
{
	if (m_Bosses.empty()) return MUID(0,0);

	return m_Bosses[0];
}