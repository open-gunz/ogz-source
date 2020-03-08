#include "stdafx.h"
#include "MQuestLevel.h"
#include "MMatchTransDataType.h"
#include "MMath.h"
#include "MQuestFormula.h"
#include "MQuestNPC.h"
#include "MQuestConst.h"


MQuestNPCQueue::MQuestNPCQueue() : m_nCursor(0)
{

}

MQuestNPCQueue::~MQuestNPCQueue()
{

}

void MQuestNPCQueue::Make(int nQLD, MQuestNPCSetInfo* pNPCSetInfo, MQUEST_NPC nKeyNPC)
{
	if (pNPCSetInfo == NULL) return;

	m_nCursor = 0;
	int nSize = nQLD;

	m_Queue.reserve(nSize);			// QLD가 나올 NPC 개수를 의미한다.
	m_Queue.resize(nSize, pNPCSetInfo->nBaseNPC);

	int nNPCSetCount = (int)pNPCSetInfo->vecNPCs.size();
	int cursor = 0;
	for (int i = 0; i < nNPCSetCount; i++)
	{
		MNPCSetNPC npc = pNPCSetInfo->vecNPCs[i];

		//float fSpawnRate = (float)RandomNumber((int)(npc.fMinRate*100.0f), (int)(npc.fMaxRate*100.0f));
		float fSpawnRate = (float)(RandomNumber(npc.nMinRate, npc.nMaxRate) / 100.0f);
		int nSpawnCount = (int)floor(nSize * fSpawnRate);

		// 만약 비율상 1개도 안나오는 NPC가 있으면 1개라도 나올 수 있는 비율을 계산한다.
		if (nSpawnCount <= 0)
		{
			if (RandomNumber(0.0f, 1.0f) < float(nSize / 100.0f))
			{
				nSpawnCount = 1;
			}
		}
		if ((npc.nMaxSpawnCount > 0) && (nSpawnCount > npc.nMaxSpawnCount))
			nSpawnCount = npc.nMaxSpawnCount;

		for (int j = 0; j < nSpawnCount; j++)
		{
			if (cursor < nSize)
			{
				m_Queue[cursor] = npc.nNPC;
				cursor++;
			}
		}
	}

	// shuffle
	for (int i = 0; i < nSize; i++)
	{
		int nTarIndex = RandomNumber(i, nSize-1);
		MQUEST_NPC temp = m_Queue[nTarIndex];
		m_Queue[nTarIndex] = m_Queue[i];
		m_Queue[i] = temp;
	}

	// 키 NPC가 있으면 제일 처음에 넣는다.
	if (nKeyNPC != NPC_NONE)
	{
		m_Queue[0] = nKeyNPC;
	}
}

bool MQuestNPCQueue::Pop(MQUEST_NPC& outNPC)
{
	if (IsEmpty()) return false;

	outNPC = m_Queue[m_nCursor];
	m_nCursor++;

	return true;
}

bool MQuestNPCQueue::GetFirst(MQUEST_NPC& outNPC)
{
	if (IsEmpty()) return false;
	outNPC = m_Queue[m_nCursor];

	return true;
}

void MQuestNPCQueue::Clear()
{
	m_Queue.clear();
	m_nCursor = 0;
}

bool MQuestNPCQueue::IsEmpty()
{
	if ((m_Queue.empty()) || (m_nCursor >= GetCount())) return true;
	return false;
}

int MQuestNPCQueue::GetCount()
{
	return (int)m_Queue.size();
}

//////////////////////////////////////////////////////////////////////////////////
MQuestLevel::MQuestLevel()
{

}

MQuestLevel::~MQuestLevel()
{

}

void MQuestLevel::Make_MTDQuestGameInfo(MTD_QuestGameInfo* pout)
{
	pout->nNPCInfoCount = (decltype(pout->nNPCInfoCount))m_StaticInfo.NPCs.size();
	_ASSERT (pout->nNPCInfoCount < MAX_QUEST_NPC_INFO_COUNT);

	int idx = 0;
	for (set<MQUEST_NPC>::iterator itor = m_StaticInfo.NPCs.begin(); itor != m_StaticInfo.NPCs.end(); ++itor)
	{
		pout->nNPCInfo[idx] = (*itor);
		idx++;
	}

	pout->nMapSectorCount = (decltype(pout->nMapSectorCount))m_StaticInfo.SectorList.size();
	_ASSERT(pout->nMapSectorCount < MAX_QUEST_MAP_SECTOR_COUNT);

	for (int i = 0; i < pout->nMapSectorCount; i++)
	{
		pout->nMapSectorID[i] = m_StaticInfo.SectorList[i].nSectorID;
		pout->nMapSectorLinkIndex[i] = m_StaticInfo.SectorList[i].nNextLinkIndex;
	}

	pout->nNPCCount = (unsigned short)m_NPCQueue.GetCount();
	pout->fNPC_TC = m_StaticInfo.fNPC_TC;
	pout->nQL = m_StaticInfo.nQL;
}


void MQuestLevel::Init(int nScenarioID, int nDice)
{
	MMatchQuest* pQuest = MMatchServer::GetInstance()->GetQuest();
	m_StaticInfo.pScenario = pQuest->GetScenarioInfo(nScenarioID);
	m_StaticInfo.nDice = nDice - 1;		// 0부터 5까지

	if (m_StaticInfo.pScenario)
	{
		InitSectors();
		InitNPCs();
	}

	m_DynamicInfo.nCurrSectorIndex = 0;

	InitStaticInfo();	// 난이도 상수, NPC 난이도 조절 계수 등을 설정
	InitCurrSector();
}

bool MQuestLevel::InitSectors()
{
	if (m_StaticInfo.pScenario == NULL) 
	{
		_ASSERT(0);
		return false;
	}

	MMatchQuest* pQuest = MMatchServer::GetInstance()->GetQuest();

	int nSectorCount = m_StaticInfo.pScenario->GetSectorCount(m_StaticInfo.nDice);
	int nKeySector = m_StaticInfo.pScenario->Maps[m_StaticInfo.nDice].nKeySectorID;


	m_StaticInfo.SectorList.reserve(nSectorCount);
	m_StaticInfo.SectorList.resize(nSectorCount);

	int nSectorIndex = nSectorCount-1;

	int nSectorID = nKeySector;
	int nLinkIndex = 0;
	for (int i = 0; i < nSectorCount; i++)
	{
		MQuestMapSectorInfo* pSector = pQuest->GetSectorInfo(nSectorID);
		if (pSector == NULL) 
		{
			_ASSERT(0);
			return false;
		}

		// 섹터 정보 입력
		MQuestLevelSectorNode node;
		node.nSectorID = nSectorID;
		node.nNextLinkIndex = nLinkIndex;
		m_StaticInfo.SectorList[nSectorIndex] = node;


		if (i != (nSectorCount-1)) 
		{
			// 현재 섹터노드를 바탕으로 이전 섹터와 링크를 결정한다.
			int nBacklinkCount = (int)pSector->VecBacklinks.size();
			if (nBacklinkCount > 0)
			{
				bool bSameNode = false;
				int nLoopCount = 0;
				do
				{
					nLoopCount++;

					int backlink_index = RandomNumber(0, (nBacklinkCount-1));
					nSectorID = pSector->VecBacklinks[backlink_index].nSectorID;
					nLinkIndex = pSector->VecBacklinks[backlink_index].nLinkIndex;

					// 같은 노드가 두번 반복해서 걸리지 않도록 한다.
					if ((nBacklinkCount > 1) && ((nSectorIndex+1) < nSectorCount))
					{
						if (nSectorID == m_StaticInfo.SectorList[nSectorIndex+1].nSectorID)
						{
							bSameNode = true;
						}
					}
				}
				while ((bSameNode) && (nLoopCount < 2));	// 이전 노드랑 같은 노드가 걸리면 반복

			}
			else
			{
				// 역링크가 하나라도 있어야 한다.
				_ASSERT(0);
				return false;
			}

			nSectorIndex--;
		}
	}

	return true;
}

bool MQuestLevel::InitNPCs()
{
	if (m_StaticInfo.pScenario == NULL) 
	{
		_ASSERT(0);
		return false;
	}

	int nDice = m_StaticInfo.nDice;
	MMatchQuest* pQuest = MMatchServer::GetInstance()->GetQuest();
	int nArraySize = (int)m_StaticInfo.pScenario->Maps[nDice].vecNPCSetArray.size();
	for (int i = 0; i < nArraySize; i++)
	{
		int nNPCSetID = m_StaticInfo.pScenario->Maps[nDice].vecNPCSetArray[i];
		MQuestNPCSetInfo* pNPCSetInfo = pQuest->GetNPCSetInfo(nNPCSetID);
		if (pNPCSetInfo == NULL) 
		{
			_ASSERT(0);
			return false;
		}
		
		// base npc는 따로 넣는다.
		m_StaticInfo.NPCs.insert(set<MQUEST_NPC>::value_type(pNPCSetInfo->nBaseNPC));

		int nNPCSize = (int)pNPCSetInfo->vecNPCs.size();
		for (int j = 0; j < nNPCSize; j++)
		{
			MQUEST_NPC npc = (MQUEST_NPC)pNPCSetInfo->vecNPCs[j].nNPC;
			m_StaticInfo.NPCs.insert(set<MQUEST_NPC>::value_type(npc));
		}
	}

	
	return true;
}


int MQuestLevel::GetMapSectorCount()
{
	return (int)m_StaticInfo.SectorList.size();
}


bool MQuestLevel::MoveToNextSector()
{
	if ((m_DynamicInfo.nCurrSectorIndex+1) >= GetMapSectorCount()) return false;
	
	MMatchQuest* pQuest = MMatchServer::GetInstance()->GetQuest();

	m_DynamicInfo.nCurrSectorIndex++;

	InitCurrSector();
	return true;
}

void MQuestLevel::InitCurrSector()
{
	// npc queue 세팅
	MMatchQuest* pQuest = MMatchServer::GetInstance()->GetQuest();
	int nNPCSetID = m_StaticInfo.pScenario->Maps[m_StaticInfo.nDice].vecNPCSetArray[m_DynamicInfo.nCurrSectorIndex];
	MQuestNPCSetInfo* pNPCSetInfo = pQuest->GetNPCSetInfo(nNPCSetID);

	m_NPCQueue.Clear();

	m_DynamicInfo.bCurrBossSector = false;

	// 만약 키 NPC가 있고, 마지막 섹터이면 키 NPC 세팅
	if ((m_StaticInfo.pScenario->Maps[m_StaticInfo.nDice].nKeyNPCID != 0) &&
		(m_DynamicInfo.nCurrSectorIndex == GetMapSectorCount() - 1))
	{
		m_NPCQueue.Make(m_StaticInfo.nQLD, pNPCSetInfo, MQUEST_NPC(m_StaticInfo.pScenario->Maps[m_StaticInfo.nDice].nKeyNPCID));
		if (m_StaticInfo.pScenario->Maps[m_StaticInfo.nDice].bKeyNPCIsBoss)
		{
			m_DynamicInfo.bCurrBossSector = true;
		}
	}
	else
	{
		m_NPCQueue.Make(m_StaticInfo.nQLD, pNPCSetInfo);
	}

	// spawn index 세팅
	memset(m_SpawnInfos, 0, sizeof(m_SpawnInfos));

	int nSectorID = m_StaticInfo.SectorList[m_DynamicInfo.nCurrSectorIndex].nSectorID;
	m_DynamicInfo.pCurrSector = pQuest->GetSectorInfo(nSectorID);
}

void MQuestLevel::InitStaticInfo()
{
	if (m_StaticInfo.pScenario)
	{
		m_StaticInfo.nQL = m_StaticInfo.pScenario->nQL;

		m_StaticInfo.nQLD = (int)(MQuestFormula::CalcQLD(m_StaticInfo.nQL) * m_StaticInfo.pScenario->fDC);
		m_StaticInfo.nLMT = (int)(MQuestFormula::CalcLMT(m_StaticInfo.nQL) * m_StaticInfo.pScenario->fDC);


		m_StaticInfo.fNPC_TC = MQuestFormula::CalcTC(m_StaticInfo.nQL);
	}
}

int MQuestLevel::GetCurrSectorIndex()
{
	return m_DynamicInfo.nCurrSectorIndex;
}

int MQuestLevel::GetSpawnPositionCount(MQuestNPCSpawnType nSpawnType)
{
	if (m_DynamicInfo.pCurrSector)
	{
		return m_DynamicInfo.pCurrSector->nSpawnPointCount[nSpawnType];
	}

	return 0;
}

int MQuestLevel::GetRecommendedSpawnPosition(MQuestNPCSpawnType nSpawnType, u64 nTickTime)
{
	if (m_DynamicInfo.pCurrSector)
	{
		int nRecommendIndex = m_SpawnInfos[nSpawnType].nIndex;

		// 스폰포지션을 추천받았을때 스폰시간도 세팅한다.
		if (nRecommendIndex < MAX_SPAWN_COUNT)
		{
			m_SpawnInfos[nSpawnType].nRecentSpawnTime[nRecommendIndex] = nTickTime;
		}

		m_SpawnInfos[nSpawnType].nIndex++;

		int nSpawnMax = m_DynamicInfo.pCurrSector->nSpawnPointCount[nSpawnType];
		if (m_SpawnInfos[nSpawnType].nIndex >= nSpawnMax) m_SpawnInfos[nSpawnType].nIndex = 0;

		return nRecommendIndex;
	}

	return 0;
}

bool MQuestLevel::IsEnableSpawnNow(MQuestNPCSpawnType nSpawnType, u64 nNowTime)
{
	if (m_DynamicInfo.pCurrSector)
	{
		int idx = m_SpawnInfos[nSpawnType].nIndex;
		if ((nNowTime - m_SpawnInfos[nSpawnType].nRecentSpawnTime[idx]) > SAME_SPAWN_DELAY_TIME) return true;
	}

	return false;
}

void MQuestLevel::OnItemCreated(u32 nItemID, int nRentPeriodHour)
{
	MQuestLevelItem* pNewItem = new MQuestLevelItem();
	pNewItem->nItemID = nItemID;
	pNewItem->nRentPeriodHour = nRentPeriodHour;
	pNewItem->bObtained = false;

	m_DynamicInfo.ItemMap.insert(make_pair(nItemID, pNewItem));
}

bool MQuestLevel::OnItemObtained( MMatchObject* pPlayer, u32 nItemID )
{
	if( 0 == pPlayer ) return false;

	for (MQuestLevelItemMap::iterator itor = m_DynamicInfo.ItemMap.lower_bound(nItemID);
		itor != m_DynamicInfo.ItemMap.upper_bound(nItemID); ++itor)
	{
		MQuestLevelItem* pQuestItem = (*itor).second;
		if (!pQuestItem->bObtained)
		{
			pQuestItem->bObtained = true;
			// pPlayer->GetCharInfo()->m_QMonsterBible.SetPage( 
			return true;
		}
	}
	
	// 만약 false이면 플레이어가 치팅을 하는 것임..-_-;
	return false;
}