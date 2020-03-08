#include "stdafx.h"
#include "MMatchServer.h"
#include "MMatchRuleQuest.h"
#include "MQuestLevel.h"
#include "MQuestLevelGenerator.h"
#include "MBlobArray.h"
#include "MQuestFormula.h"
#include "MCommandCommunicator.h"
#include "MSharedCommandTable.h"
#include "MMatchTransDataType.h"
#include "MMatchConfig.h"
#include "MMatchFormula.h"
#include "MQuestItem.h"
#include "MMath.h"
#include "MAsyncDBJob.h"
#include "MQuestNPCSpawnTrigger.h"
#include "MQuestItem.h"

MMatchRuleQuest::MMatchRuleQuest(MMatchStage* pStage) : MMatchRuleBaseQuest(pStage), m_pQuestLevel(NULL),
														m_nCombatState(MQUEST_COMBAT_NONE), m_nPrepareStartTime(0),
														m_nCombatStartTime(0), m_nQuestCompleteTime(0), m_nPlayerCount( 0 )
{
	for( int i = 0; i < MAX_SACRIFICE_SLOT_COUNT; ++i )
		m_SacrificeSlot[ i ].Release();

	m_StageGameInfo.nQL = 0;
	m_StageGameInfo.nPlayerQL = 0;
	m_StageGameInfo.nMapsetID = 1;
	m_StageGameInfo.nScenarioID = MMatchServer::GetInstance()->GetQuest()->GetScenarioCatalogue()->GetDefaultStandardScenarioID();
}

MMatchRuleQuest::~MMatchRuleQuest()
{
	ClearQuestLevel();
}

// Route 씨리즈 시작 /////////////////////////////////////////////////////////////////
void MMatchRuleQuest::RouteMapSectorStart()
{
	MCommand* pCmd = MMatchServer::GetInstance()->CreateCommand(MC_QUEST_SECTOR_START, MUID(0,0));
	char nSectorIndex = char(m_pQuestLevel->GetCurrSectorIndex());
	pCmd->AddParameter(new MCommandParameterChar(nSectorIndex));
	MMatchServer::GetInstance()->RouteToStage(GetStage()->GetUID(), pCmd);
}

void MMatchRuleQuest::RouteCombatState()
{
	MCommand* pCmd = MMatchServer::GetInstance()->CreateCommand(MC_QUEST_COMBAT_STATE, MUID(0,0));
	pCmd->AddParameter(new MCommandParameterChar(char(m_nCombatState)));
	MMatchServer::GetInstance()->RouteToStage(GetStage()->GetUID(), pCmd);
}

void MMatchRuleQuest::RouteMovetoPortal(const MUID& uidPlayer)
{
	if (m_pQuestLevel == NULL) return;

	int nCurrSectorIndex = m_pQuestLevel->GetCurrSectorIndex();

	MCommand* pCmd = MMatchServer::GetInstance()->CreateCommand(MC_QUEST_MOVETO_PORTAL, MUID(0,0));
	pCmd->AddParameter(new MCommandParameterChar(char(nCurrSectorIndex)));
	pCmd->AddParameter(new MCommandParameterUID(uidPlayer));
	MMatchServer::GetInstance()->RouteToStage(GetStage()->GetUID(), pCmd);
}

void MMatchRuleQuest::RouteReadyToNewSector(const MUID& uidPlayer)
{
	if (m_pQuestLevel == NULL) return;

	MCommand* pCmd = MMatchServer::GetInstance()->CreateCommand(MC_QUEST_READYTO_NEWSECTOR, MUID(0,0));
	pCmd->AddParameter(new MCommandParameterUID(uidPlayer));
	MMatchServer::GetInstance()->RouteToStage(GetStage()->GetUID(), pCmd);
}

void MMatchRuleQuest::RouteObtainQuestItem(u32 nQuestItemID)
{
	MCommand* pCmd = MMatchServer::GetInstance()->CreateCommand(MC_QUEST_OBTAIN_QUESTITEM, MUID(0,0));
	pCmd->AddParameter(new MCmdParamUInt(nQuestItemID));
	MMatchServer::GetInstance()->RouteToStage(GetStage()->GetUID(), pCmd);
}

void MMatchRuleQuest::RouteObtainZItem(u32 nItemID)
{
	MCommand* pCmd = MMatchServer::GetInstance()->CreateCommand(MC_QUEST_OBTAIN_ZITEM, MUID(0,0));
	pCmd->AddParameter(new MCmdParamUInt(nItemID));
	MMatchServer::GetInstance()->RouteToStage(GetStage()->GetUID(), pCmd);
}

void MMatchRuleQuest::RouteGameInfo()
{
	MCommand* pCmd = MMatchServer::GetInstance()->CreateCommand(MC_QUEST_GAME_INFO, MUID(0,0));

	void* pBlobGameInfoArray = MMakeBlobArray(sizeof(MTD_QuestGameInfo), 1);
	MTD_QuestGameInfo* pGameInfoNode = (MTD_QuestGameInfo*)MGetBlobArrayElement(pBlobGameInfoArray, 0);

	if (m_pQuestLevel)
	{
		m_pQuestLevel->Make_MTDQuestGameInfo(pGameInfoNode);
	}

	pCmd->AddParameter(new MCommandParameterBlob(pBlobGameInfoArray, MGetBlobArraySize(pBlobGameInfoArray)));
	MEraseBlobArray(pBlobGameInfoArray);

	MMatchServer::GetInstance()->RouteToStage(GetStage()->GetUID(), pCmd);
}

void MMatchRuleQuest::RouteCompleted()
{
	MCommand* pCmd = MMatchServer::GetInstance()->CreateCommand(MC_QUEST_COMPLETED, MUID(0,0));

	int nSize = (int)m_PlayerManager.size();
	void* pBlobRewardArray = MMakeBlobArray(sizeof(MTD_QuestReward), nSize);

	int idx = 0;
	for (MQuestPlayerManager::iterator itor = m_PlayerManager.begin(); itor != m_PlayerManager.end(); ++itor)
	{
		MQuestPlayerInfo* pPlayerInfo = (*itor).second;
		MTD_QuestReward* pRewardNode = (MTD_QuestReward*)MGetBlobArrayElement(pBlobRewardArray, idx);
		idx++;

		pRewardNode->uidPlayer = (*itor).first;
		pRewardNode->nXP = pPlayerInfo->nXP;
		pRewardNode->nBP = pPlayerInfo->nBP;
	}

	pCmd->AddParameter(new MCommandParameterBlob(pBlobRewardArray, MGetBlobArraySize(pBlobRewardArray)));
	MEraseBlobArray(pBlobRewardArray);

	MMatchServer::GetInstance()->RouteToStage(GetStage()->GetUID(), pCmd);
}

void MMatchRuleQuest::RouteFailed()
{
	MCommand* pCmd = MMatchServer::GetInstance()->CreateCommand(MC_QUEST_FAILED, MUID(0,0));
	MMatchServer::GetInstance()->RouteToStage(GetStage()->GetUID(), pCmd);

#ifdef _DEBUG
	mlog( "MMatchRuleQuest::RouteFailed - %s 퀘스트 게임 실패.\n\n", GetStage()->GetName() );
#endif

}

void MMatchRuleQuest::RouteStageGameInfo()
{
	MCommand* pCmd = MMatchServer::GetInstance()->CreateCommand(MC_QUEST_STAGE_GAME_INFO, MUID(0,0));
	pCmd->AddParameter(new MCmdParamChar(char(m_StageGameInfo.nQL)));
	pCmd->AddParameter(new MCmdParamChar(char(m_StageGameInfo.nMapsetID)));
	pCmd->AddParameter(new MCmdParamUInt(m_StageGameInfo.nScenarioID));
	MMatchServer::GetInstance()->RouteToStage(GetStage()->GetUID(), pCmd);
}

void MMatchRuleQuest::RouteSectorBonus(const MUID& uidPlayer, u32 nEXPValue)
{
	MMatchObject* pPlayer = MMatchServer::GetInstance()->GetObject(uidPlayer);	
	if (!IsEnabledObject(pPlayer)) return;

	MCommand* pNewCmd = MMatchServer::GetInstance()->CreateCommand(MC_QUEST_SECTOR_BONUS, MUID(0,0));
	pNewCmd->AddParameter(new MCmdParamUID(uidPlayer));
	pNewCmd->AddParameter(new MCmdParamUInt(nEXPValue));
	MMatchServer::GetInstance()->RouteToListener( pPlayer, pNewCmd );
}

// Route 씨리즈 끝 ///////////////////////////////////////////////////////////////////

void MMatchRuleQuest::OnBegin()
{
	MakeQuestLevel();

	MMatchRuleBaseQuest::OnBegin();		// 여기서 게임정보도 보냄 - 순서에 주의

	// 게임을 완료 하였을시 시작할때의 인원수에 따라서 보상을 위해서 현재 유저 수를 저장한다.
	m_nPlayerCount = static_cast< int >( m_PlayerManager.size() );

	// 게임시작하면 슬롯을 모두 비워줘야 함.
	// 희생아이템 로그 정보는 DestroyAllSlot()에서 m_QuestGameLogInfoMgr로 저장.
	DestroyAllSlot();

	// 게임 시작전에 Log에 필요한 정보를 수집함.
	CollectStartingQuestGameLogInfo();

	SetCombatState(MQUEST_COMBAT_PREPARE);
}

void MMatchRuleQuest::OnEnd()
{
	ClearQuestLevel();

	MMatchRuleBaseQuest::OnEnd();
}

bool MMatchRuleQuest::OnRun()
{
	bool ret = MMatchRuleBaseQuest::OnRun();
	if (ret == false) return false;

	if (GetRoundState() == MMATCH_ROUNDSTATE_PLAY)
	{
		CombatProcess();
	}

	return true;
}


// 지금은 좀 꼬여있음.
void MMatchRuleQuest::CombatProcess()
{
	switch (m_nCombatState)
	{
	case MQUEST_COMBAT_PREPARE:			// 모두들 섹터로 들어오기를 기다리는 시기
		{
			if (CheckReadytoNewSector())		// 모두 다 섹터에 들어올때까지 PREPARE
			{
				SetCombatState(MQUEST_COMBAT_PLAY);				
			};
		}
		break;
	case MQUEST_COMBAT_PLAY:			// 실제 게임 플레이 시기
		{
			COMBAT_PLAY_RESULT nResult = CheckCombatPlay();
			switch(nResult)
			{
			case CPR_PLAYING:
				{
					ProcessCombatPlay();
				}
				break;
			case CPR_COMPLETE:
				{
					if (CheckQuestCompleteDelayTime())
					{
						SetCombatState(MQUEST_COMBAT_COMPLETED);
					}
				}
				break;
			case CPR_FAILED:
				{
					// 여기까지 오기전에 이 상위 클래스에서 유저의 생존여부를 검사해서 게임을 끝내버림... - by 추교성.
					// OnFail을 OnCheckRoundFinish에서 처리하는 방향으로 수정했음.
					// SetCombatState(MQUEST_COMBAT_NONE);
					// m_bQuestCompleted = false;
					// OnFailed();
				}
				break;
			};
		}
		break;
	case MQUEST_COMBAT_COMPLETED:			// 게임이 끝나고 다음 링크로 건너가는 시기
		{
			// 퀘스트 클리어가 아니고 다음 섹터가 남아 있으면 바로 PREPARE상태가 된다.
			if (!m_bQuestCompleted)
			{
                SetCombatState(MQUEST_COMBAT_PREPARE);
			}
		}
		break;
	};
}


void MMatchRuleQuest::OnBeginCombatState(MQuestCombatState nState)
{
	switch (nState)
	{
	case MQUEST_COMBAT_PREPARE:
		{
			m_nPrepareStartTime = MMatchServer::GetInstance()->GetTickTime();
		}
		break;
	case MQUEST_COMBAT_PLAY:
		{
			m_nCombatStartTime = MMatchServer::GetInstance()->GetTickTime();
			// 월드아이템 초기화
			m_pStage->m_WorldItemManager.OnRoundBegin();

			RouteMapSectorStart();

			// 모두 부활
			if (m_pQuestLevel->GetCurrSectorIndex() != 0)
				RefreshPlayerStatus();
		}
		break;
	case MQUEST_COMBAT_COMPLETED:
		{
			if (CheckQuestCompleted())
			{
				OnCompleted();
			}
			else if( !CheckPlayersAlive() )
			{
				// 게임이 중간에 끝남.
				OnFailed();
			}
			else
			{
				OnSectorCompleted();
			}
		}
		break;
	};
}

void MMatchRuleQuest::OnEndCombatState(MQuestCombatState nState)
{
	switch (nState)
	{
	case MQUEST_COMBAT_PREPARE:
		break;
	case MQUEST_COMBAT_PLAY:
		break;
	case MQUEST_COMBAT_COMPLETED:
		break;
	};
}

MMatchRuleQuest::COMBAT_PLAY_RESULT MMatchRuleQuest::CheckCombatPlay()
{
	if (m_pQuestLevel->GetDynamicInfo()->bCurrBossSector && m_NPCManager.BossDead)
	{
		return CPR_COMPLETE;
	}

	if ((m_pQuestLevel->GetNPCQueue()->IsEmpty()) && (m_NPCManager.GetNPCObjectCount() <= 0))
	{
		return CPR_COMPLETE;
	}

	// 모든 유저가 죽었으면 게임 실패로 설정함.
	if( !CheckPlayersAlive() )
	{
		return CPR_FAILED;
	}

	return CPR_PLAYING;
}

void MMatchRuleQuest::OnCommand(MCommand* pCommand)
{
	MMatchRuleBaseQuest::OnCommand(pCommand);
}


///
// First : 
// Last  : 2005.04.27 추교성.
//
// 희생아이템을 슬롯에 올려놓으면, QL계산과 희생아이템 테이블에서 아이템에 해당하는 테이블이 있는지 검사하기 위해 호출됨.
//  아이템을 슬롯에 올려놓을때는 QL만을 계산을 함. 희생아이템 테이블 검색 결과는 사용되지 않음.
//  게임을 시작할시에는 희생아이템 테이블 검색 결과가 정상일때만 게임을 시작함.
///
bool MMatchRuleQuest::MakeQuestLevel()
{
	// 이전의 퀘스트 레벨 정보는 제거함.
	if( 0 != m_pQuestLevel )
	{
		delete m_pQuestLevel;
		m_pQuestLevel = 0;
	}

	MQuestLevelGenerator	LG;

	LG.BuildPlayerQL(m_StageGameInfo.nPlayerQL);
	LG.BuildMapset(m_StageGameInfo.nMapsetID);
	
	for (int i = 0; i < MAX_SCENARIO_SACRI_ITEM; i++)
	{
		LG.BuildSacriQItem(m_SacrificeSlot[i].GetItemID());
	}

	m_pQuestLevel = LG.MakeLevel();


	// 첫섹터부터 보스방일 수 있으므로..
	InitJacoSpawnTrigger();

	return true;
}

void MMatchRuleQuest::ClearQuestLevel()
{
	if (m_pQuestLevel)
	{
		delete m_pQuestLevel;
		m_pQuestLevel = NULL;
	}
}




void MMatchRuleQuest::MoveToNextSector()
{
	// m_pQuestLevel도 다음맵으로 이동해준다.
	m_pQuestLevel->MoveToNextSector();

	for (MQuestPlayerManager::iterator itor = m_PlayerManager.begin(); itor != m_PlayerManager.end(); ++itor)
	{
		MQuestPlayerInfo* pPlayerInfo = (*itor).second;
		pPlayerInfo->bMovedtoNewSector = false;
	}

	InitJacoSpawnTrigger();	
}

void MMatchRuleQuest::InitJacoSpawnTrigger()
{
	// 만약 다음 섹터가 보스섹터이면 JacoTrigger 발동
	if (m_pQuestLevel->GetDynamicInfo()->bCurrBossSector)
	{
		int nDice = m_pQuestLevel->GetStaticInfo()->nDice;
		MQuestScenarioInfoMaps* pMap = &m_pQuestLevel->GetStaticInfo()->pScenario->Maps[nDice];

		SpawnTriggerInfo info;
		memset(&info, 0, sizeof(SpawnTriggerInfo));

		info.nSpawnNPCCount = pMap->nJacoCount;
		info.nSpawnTickTime = pMap->nJacoSpawnTickTime;
		info.nCurrMinNPCCount = pMap->nJacoMinNPCCount;
		info.nCurrMaxNPCCount = pMap->nJacoMaxNPCCount;

		m_JacoSpawnTrigger.Clear();
		m_JacoSpawnTrigger.BuildCondition(info);

		for (vector<MQuestScenarioInfoMapJaco>::iterator itor = pMap->vecJacoArray.begin(); itor != pMap->vecJacoArray.end(); ++itor)
		{
			SpawnTriggerNPCInfoNode node;
			node.nNPCID = (*itor).nNPCID;
			node.fRate = (*itor).fRate;

			m_JacoSpawnTrigger.BuildNPCInfo(node);
		}
	}
}

void MMatchRuleQuest::SetCombatState(MQuestCombatState nState)
{
	if (m_nCombatState == nState) return;

	OnEndCombatState(m_nCombatState);
	m_nCombatState = nState;
	OnBeginCombatState(m_nCombatState);

	RouteCombatState();
}


bool MMatchRuleQuest::CheckReadytoNewSector()
{
	auto nNowTime = MMatchServer::GetInstance()->GetTickTime();
	if ((nNowTime - m_nPrepareStartTime) > PORTAL_MOVING_TIME)
	{
		return true;
	}

	for (MQuestPlayerManager::iterator itor = m_PlayerManager.begin(); itor != m_PlayerManager.end(); ++itor)
	{
		MQuestPlayerInfo* pPlayerInfo = (*itor).second;
		if ((pPlayerInfo->pObject->CheckAlive()) && (pPlayerInfo->bMovedtoNewSector == false)) return false;
	}
	
	return true;
}

// 섹터 클리어
void MMatchRuleQuest::OnSectorCompleted()
{
	// 섹터 보너스
	MQuestScenarioInfo* pScenario = m_pQuestLevel->GetStaticInfo()->pScenario;
	if (pScenario)
	{
		int nSectorXP = pScenario->nSectorXP;
		int nSectorBP = pScenario->nSectorBP;

		// 섹터 보너스값 계산
		if (nSectorXP < 0)
		{
			int nSectorCount = (int)m_pQuestLevel->GetStaticInfo()->SectorList.size();
			nSectorXP = MQuestFormula::CalcSectorXP(pScenario->nXPReward, nSectorCount);
		}
		// 섹터 보너스값 계산
		if (nSectorBP < 0)
		{
			int nSectorCount = (int)m_pQuestLevel->GetStaticInfo()->SectorList.size();
			nSectorBP = MQuestFormula::CalcSectorXP(pScenario->nBPReward, nSectorCount);
		}

		if ((nSectorXP > 0) || (nSectorBP > 0))
		{
			for (MQuestPlayerManager::iterator itor = m_PlayerManager.begin(); itor != m_PlayerManager.end(); ++itor)
			{
				int nAddedSectorXP = nSectorXP;
				int nAddedSectorBP = nSectorBP;

				MMatchObject* pPlayer = (*itor).second->pObject;
				if ((!IsEnabledObject(pPlayer)) || (!pPlayer->CheckAlive())) continue;

				// 경험치, 바운티 보너스 계산
				const float fXPBonusRatio = MMatchFormula::CalcXPBonusRatio(pPlayer, MIBT_QUEST);
				const float fBPBonusRatio = MMatchFormula::CalcBPBounsRatio(pPlayer, MIBT_QUEST);
				nAddedSectorXP += (int)(nAddedSectorXP * fXPBonusRatio);
				nAddedSectorBP += (int)(nAddedSectorBP * fBPBonusRatio);

				// 실제 적용
				MGetMatchServer()->ProcessPlayerXPBP(m_pStage, pPlayer, nAddedSectorXP, nAddedSectorBP);

				// 라우팅
				int nExpPercent = MMatchFormula::GetLevelPercent(pPlayer->GetCharInfo()->m_nXP, 
																pPlayer->GetCharInfo()->m_nLevel);
				u32 nExpValue = MakeExpTransData(nAddedSectorXP, nExpPercent);
				RouteSectorBonus(pPlayer->GetUID(), nExpValue);
			}
		}
	}

	// 죽은 사람 부활시킨다.
//	RefreshPlayerStatus();

	MoveToNextSector();
}

// 퀘스트 성공시
void MMatchRuleQuest::OnCompleted()
{
	MMatchRuleBaseQuest::OnCompleted();

#ifdef _QUEST_ITEM
	// 여기서 DB로 QuestGameLog생성.
	PostInsertQuestGameLogAsyncJob();	
	SetCombatState(MQUEST_COMBAT_NONE);
#endif
	
}

// 퀘스트 실패시
void MMatchRuleQuest::OnFailed()
{
	SetCombatState(MQUEST_COMBAT_NONE);
	m_bQuestCompleted = false;

	MMatchRuleBaseQuest::OnFailed();

	PostInsertQuestGameLogAsyncJob();
}

// 퀘스트가 모두 끝났는지 체크
bool MMatchRuleQuest::CheckQuestCompleted()
{
	if (m_pQuestLevel)
	{
		auto nStartTime = GetStage()->GetStartTime();
		auto nNowTime = MMatchServer::GetInstance()->GetTickTime();

		auto nCheckTime = QUEST_COMBAT_PLAY_START_DELAY * m_pQuestLevel->GetMapSectorCount();

		if (MGetTimeDistance(nStartTime, nNowTime) < nCheckTime) return false;


		if (m_pQuestLevel->GetMapSectorCount() == (m_pQuestLevel->GetCurrSectorIndex()+1))
		{
			return true;
		}
	}

	return false;
}

// 마지막 섹터는 아이템을 먹을 수 있도록 딜레이 시간을 둔다.
bool MMatchRuleQuest::CheckQuestCompleteDelayTime()
{
	if ((m_pQuestLevel) && (m_pQuestLevel->GetMapSectorCount() == (m_pQuestLevel->GetCurrSectorIndex()+1)))
	{
		unsigned long int nNowTime = MMatchServer::GetInstance()->GetTickTime();
		if (m_nQuestCompleteTime == 0)
			m_nQuestCompleteTime = nNowTime;
		if (MGetTimeDistance(m_nQuestCompleteTime, nNowTime) > QUEST_COMPLETE_DELAY)
			return true; 
		return false;
	}

	return true;
}

void MMatchRuleQuest::ProcessCombatPlay()
{
	ProcessNPCSpawn();

}

void MMatchRuleQuest::MakeNPCnSpawn(MQUEST_NPC nNPCID, bool bAddQuestDropItem)
{
	MQuestNPCSpawnType nSpawnType = MNST_MELEE;
	MQuestNPCInfo* pNPCInfo = MMatchServer::GetInstance()->GetQuest()->GetNPCInfo(nNPCID);
	if (pNPCInfo)
	{
		nSpawnType = pNPCInfo->GetSpawnType();
		int nPosIndex = m_pQuestLevel->GetRecommendedSpawnPosition(nSpawnType, MMatchServer::GetInstance()->GetTickTime());

		MMatchNPCObject* pNPCObject = SpawnNPC(nNPCID, nPosIndex);

		if (pNPCObject)
		{
			// drop item 결정
			MQuestDropItem item;
			int nDropTableID = pNPCInfo->nDropTableID;
			int nQL = m_pQuestLevel->GetStaticInfo()->nQL;
			MMatchServer::GetInstance()->GetQuest()->GetDropTable()->Roll(item, nDropTableID, nQL);

			// AddQuestDropItem=false이면 월드아이템만 드롭한다.
			if ((bAddQuestDropItem==true) || (item.nDropItemType == QDIT_WORLDITEM))
			{
				pNPCObject->SetDropItem(&item);

				// 만들어진 아이템은 level에 넣어놓는다.
				if ((item.nDropItemType == QDIT_QUESTITEM) || (item.nDropItemType == QDIT_ZITEM))
				{
					m_pQuestLevel->OnItemCreated((u32)(item.nID), item.nRentPeriodHour);
				}
			}
		}
	}
}

void MMatchRuleQuest::ProcessNPCSpawn()
{
	if (CheckNPCSpawnEnable())
	{
		MQUEST_NPC npc;
		if (m_pQuestLevel->GetNPCQueue()->Pop(npc))
		{
			MakeNPCnSpawn(npc, true);
		}
	}
	else
	{
		// 보스방일 경우 Queue에 있는 NPC들을 모두 스폰시켰으면 Jaco들을 스폰시킨다.
		if (m_pQuestLevel->GetDynamicInfo()->bCurrBossSector)
		{
			// 보스가 살아있고 기본적으로 나올 NPC가 다 나온다음에 졸병들 스폰
			if ((m_NPCManager.GetBossCount() > 0) && (m_pQuestLevel->GetNPCQueue()->IsEmpty()))
			{
				int nAliveNPCCount = m_NPCManager.GetNPCObjectCount();
				

				if (m_JacoSpawnTrigger.CheckSpawnEnable(nAliveNPCCount))
				{
					int nCount = (int)m_JacoSpawnTrigger.GetQueue().size();
					for (int i = 0; i < nCount; i++)
					{
						MQUEST_NPC npc = m_JacoSpawnTrigger.GetQueue()[i];
						MakeNPCnSpawn(npc, false);
					}
				}
			}
		}
	}
}


bool MMatchRuleQuest::CheckNPCSpawnEnable()
{
	if (m_pQuestLevel->GetNPCQueue()->IsEmpty()) return false;

	if (m_NPCManager.GetNPCObjectCount() >= m_pQuestLevel->GetStaticInfo()->nLMT) return false;
	auto nNowTime = MMatchServer::GetInstance()->GetTickTime();

	if (nNowTime - m_nCombatStartTime < QUEST_COMBAT_PLAY_START_DELAY)
	{
		return false;
	}


	return true;

}

void MMatchRuleQuest::OnRequestTestSectorClear()
{
	ClearAllNPC();

	SetCombatState(MQUEST_COMBAT_COMPLETED);
}

void MMatchRuleQuest::OnRequestTestFinish()
{
	ClearAllNPC();

	m_pQuestLevel->GetDynamicInfo()->nCurrSectorIndex = m_pQuestLevel->GetMapSectorCount()-1;

	SetCombatState(MQUEST_COMBAT_COMPLETED);
}


void MMatchRuleQuest::OnRequestMovetoPortal(const MUID& uidPlayer)
{
//	MQuestPlayerInfo* pPlayerInfo = m_PlayerManager.GetPlayerInfo(uidPlayer);

	RouteMovetoPortal(uidPlayer);
}




void MMatchRuleQuest::OnReadyToNewSector(const MUID& uidPlayer)
{
	MQuestPlayerInfo* pPlayerInfo = m_PlayerManager.GetPlayerInfo(uidPlayer);
	if (pPlayerInfo)
	{
        pPlayerInfo->bMovedtoNewSector = true;
	}

	RouteReadyToNewSector(uidPlayer);
}

bool MMatchRuleQuest::OnCheckRoundFinish()
{
	return MMatchRuleBaseQuest::OnCheckRoundFinish();
}

void MMatchRuleQuest::DistributeReward()
{
	if (!m_pQuestLevel) return;

	// 현재 서버가 퀘스트 서버일 경우에만 가능하게 함.
	if( MSM_TEST != MGetServerConfig()->GetServerMode() )  return;

	MQuestScenarioInfo* pScenario = m_pQuestLevel->GetStaticInfo()->pScenario;
	if (!pScenario) return;

	MMatchObject*					pPlayer;



	const int nRewardXP		= pScenario->nXPReward;
	const int nRewardBP		= pScenario->nBPReward;
	const int nScenarioQL	= pScenario->nQL;

	MakeRewardList();		// 아이템 배분

	for (MQuestPlayerManager::iterator itor = m_PlayerManager.begin(); itor != m_PlayerManager.end(); ++itor)
	{
		MQuestPlayerInfo* pPlayerInfo = (*itor).second;

		// 경험치, 바운티 배분
		DistributeXPnBP( pPlayerInfo, nRewardXP, nRewardBP, nScenarioQL );

		pPlayer = MMatchServer::GetInstance()->GetObject((*itor).first);
		if( !IsEnabledObject(pPlayer) ) continue;

		// 퀘스트 아이템 배분
		void* pSimpleQuestItemBlob = NULL;
		if (!DistributeQItem( pPlayerInfo, &pSimpleQuestItemBlob )) continue;

		// 일반 아이템 배분
		void* pSimpleZItemBlob = NULL;
		if (!DistributeZItem( pPlayerInfo, &pSimpleZItemBlob )) continue;

		// DB동기화 여부 검사.
		pPlayer->GetCharInfo()->GetDBQuestCachingData().IncreasePlayCount();

		// 커맨드 생성.
		RouteRewardCommandToStage( pPlayer, (*itor).second->nXP, (*itor).second->nBP, pSimpleQuestItemBlob, pSimpleZItemBlob );

		MEraseBlobArray( pSimpleQuestItemBlob );
	}

}


void MMatchRuleQuest::InsertNoParamQItemToPlayer( MMatchObject* pPlayer, MQuestItem* pQItem )
{
	if( !IsEnabledObject(pPlayer) || (0 == pQItem) ) return;

	MQuestItemMap::iterator itMyQItem = pPlayer->GetCharInfo()->m_QuestItemList.find( pQItem->GetItemID() );

	if( pPlayer->GetCharInfo()->m_QuestItemList.end() != itMyQItem )
	{
		// 기존에 가지고 있던 퀘스트 아이템. 수량만 증가 시켜주면 됨.
		const int nOver = itMyQItem->second->Increase( pQItem->GetCount() );
		if( 0 < nOver )
			pQItem->Decrease( nOver );
	}
	else
	{
		// 처음 획득한 퀘스트 아이템. 새로 추가시켜 줘야 함.
		if( !pPlayer->GetCharInfo()->m_QuestItemList.CreateQuestItem(pQItem->GetItemID(), pQItem->GetCount(), pQItem->IsKnown()) )
			mlog( "MMatchRuleQuest::DistributeReward - %d번호 아이템의 Create( ... )함수 호출 실패.\n", pQItem->GetItemID() );
	}
}


void MMatchRuleQuest::MakeRewardList()
{
	int								nPos;
	int								nPlayerCount;
	int								nLimitRandNum;
	MQuestItem*						pRewardQItem;
	MQuestLevelItemMap::iterator	itObtainQItem, endObtainQItem;
	MQuestLevelItem*				pObtainQItem;

	nPlayerCount	= static_cast< int >( m_PlayerManager.size() );
	endObtainQItem	= m_pQuestLevel->GetDynamicInfo()->ItemMap.end();
	nLimitRandNum	= m_nPlayerCount - 1;

	vector<MQuestPlayerInfo*>	a_vecPlayerInfos;
	for (MQuestPlayerManager::iterator itor = m_PlayerManager.begin(); itor != m_PlayerManager.end(); ++itor)
	{
		MQuestPlayerInfo* pPlayerInfo = (*itor).second;

		// 혹시 예전 게임의 리워드 아이템이 남아있을지 모르니 초기화.
		pPlayerInfo->RewardQuestItemMap.Clear();
		pPlayerInfo->RewardZItemList.clear();

		a_vecPlayerInfos.push_back(pPlayerInfo);
	}

	for( itObtainQItem = m_pQuestLevel->GetDynamicInfo()->ItemMap.begin(); itObtainQItem != endObtainQItem; ++itObtainQItem )
	{
		pObtainQItem = itObtainQItem->second;

		// 획득하지 못했으면 무시.
		if (!pObtainQItem->bObtained) continue;	

		if (pObtainQItem->IsQuestItem())
		{
			// 퀘스트 아이템 -----------------------------------------------------

			// 시작할때의 인원을가지고 roll을 함.
			nPos = RandomNumber( 0, nLimitRandNum );

			// 현재 남아있는 인원보다 클경우 그냥 버림.
			if (( nPos < nPlayerCount ) && (nPos < (int)a_vecPlayerInfos.size()))
			{
				// 퀘스트 아이템일 경우 처리
				MQuestItemMap* pRewardQuestItemMap = &a_vecPlayerInfos[ nPos ]->RewardQuestItemMap;

				pRewardQItem = pRewardQuestItemMap->Find( pObtainQItem->nItemID );
				if( 0!= pRewardQItem )
					pRewardQItem->Increase(); // 이전에 획득한 아이템.
				else
				{
					// 처음 획득.
					if( !pRewardQuestItemMap->CreateQuestItem(pObtainQItem->nItemID, 1) )
					{
						mlog( "MMatchRuleQuest::MakeRewardList - ItemID:%d 처음 획득한 아이템 생성 실패.\n", pObtainQItem->nItemID );
						continue;
					}
				}
			}
		}
		else
		{
			// 일반 아이템일 경우 처리 -------------------------------------------

			
			
			RewardZItemInfo iteminfo;
			iteminfo.nItemID = pObtainQItem->nItemID;
			iteminfo.nRentPeriodHour = pObtainQItem->nRentPeriodHour;

			int nLoopCounter = 0;
			const int MAX_LOOP_COUNT = 5;

			// 최대 5번까지 랜덤으로 아이템의 성별이 같은 사람을 찾는다.
			while (nLoopCounter < MAX_LOOP_COUNT)
			{
				nLoopCounter++;

				// 시작할때의 인원을가지고 roll을 함.
				nPos = RandomNumber( 0, nLimitRandNum );

				// 현재 남아있는 인원보다 클경우 그냥 버림.
				if (( nPos < nPlayerCount ) && (nPos < (int)a_vecPlayerInfos.size()))
				{
					MQuestPlayerInfo* pPlayerInfo = a_vecPlayerInfos[ nPos ];
					MQuestRewardZItemList* pRewardZItemList = &pPlayerInfo->RewardZItemList;

					// 성별이 같아야만 가질 수 있다.
					if (IsEnabledObject(pPlayerInfo->pObject))
					{
						if (IsEquipableItem(iteminfo.nItemID, MAX_LEVEL, pPlayerInfo->pObject->GetCharInfo()->m_nSex))
						{
							pRewardZItemList->push_back(iteminfo);
							break;
						}
					}
				}
			}
		}

	}
}


///< 경험치와 바운티 배분 옮김. -by 추교성.
void MMatchRuleQuest::DistributeXPnBP( MQuestPlayerInfo* pPlayerInfo, const int nRewardXP, const int nRewardBP, const int nScenarioQL )
{
	float fXPRate, fBPRate;

	MQuestFormula::CalcRewardRate(fXPRate, 
								  fBPRate,
								  nScenarioQL, 
								  pPlayerInfo->nQL,
								  pPlayerInfo->nDeathCount, 
								  pPlayerInfo->nUsedPageSacriItemCount, 
								  pPlayerInfo->nUsedExtraSacriItemCount);

	pPlayerInfo->nXP = int(nRewardXP * fXPRate);
	pPlayerInfo->nBP = int(nRewardBP * fBPRate);


	// 실제로 경험치, 바운티 지급
	if (IsEnabledObject(pPlayerInfo->pObject))
	{
		// 경험치 보너스 계산
		const float fXPBonusRatio = MMatchFormula::CalcXPBonusRatio(pPlayerInfo->pObject, MIBT_QUEST);
		const float fBPBonusRatio = MMatchFormula::CalcBPBounsRatio(pPlayerInfo->pObject, MIBT_QUEST);

		int nExpBonus = (int)(pPlayerInfo->nXP * fXPBonusRatio);
		pPlayerInfo->nXP += nExpBonus;

		int nBPBonus = (int)(pPlayerInfo->nBP * fBPBonusRatio);
		pPlayerInfo->nBP += nBPBonus;

		MMatchServer::GetInstance()->ProcessPlayerXPBP(m_pStage, pPlayerInfo->pObject, pPlayerInfo->nXP, pPlayerInfo->nBP);
	}
}

// 퀘스트 아이템 배분
bool MMatchRuleQuest::DistributeQItem( MQuestPlayerInfo* pPlayerInfo, void** ppoutSimpleQuestItemBlob)
{
	MMatchObject* pPlayer = pPlayerInfo->pObject;
	if (!IsEnabledObject(pPlayer)) return false;

	MQuestItemMap* pObtainQuestItemMap = &pPlayerInfo->RewardQuestItemMap;

	// Client로 전송할수 있는 형태로 Quest item정보를 저장할 Blob생성.
	void* pSimpleQuestItemBlob = MMakeBlobArray( sizeof(MTD_QuestItemNode), static_cast<int>(pObtainQuestItemMap->size()) );
	if( 0 == pSimpleQuestItemBlob )
	{
		mlog( "MMatchRuleQuest::DistributeReward - Quest item 정보를 보낼 Blob생성에 실패.\n" );
		return false;
	}

	// 로그를 위해서 해당 유저가 받을 아이템의 정보를 저장해 놓음.
	if( !m_QuestGameLogInfoMgr.AddRewardQuestItemInfo(pPlayer->GetUID(), pObtainQuestItemMap) )
	{
		mlog( "m_QuestGameLogInfoMgr -해당 유저의 로그객체를 찾는데 실패." );
	}

	int nBlobIndex = 0;
	for(MQuestItemMap::iterator itQItem = pObtainQuestItemMap->begin(); itQItem != pObtainQuestItemMap->end(); ++itQItem )
	{
		MQuestItem* pQItem = itQItem->second;
		MQuestItemDesc* pQItemDesc = pQItem->GetDesc();
		if( 0 == pQItemDesc )
		{
			mlog( "MMatchRuleQuest::DistributeReward - %d 아이템의 디스크립션 셋팅이 되어있지 않음.\n", pQItem->GetItemID() );
			continue;
		}

		// 유니크 아이템인지 검사를 함.
		pPlayer->GetCharInfo()->m_DBQuestCachingData.CheckUniqueItem( pQItem );
		// 보상받은 횟수를 검사를 함.
		pPlayer->GetCharInfo()->m_DBQuestCachingData.IncreaseRewardCount();

		if( MMQIT_MONBIBLE == pQItemDesc->m_nType )
		{
			// 몬스터 도감 처리.
			if( !pPlayer->GetCharInfo()->m_QMonsterBible.IsKnownMonster(pQItemDesc->m_nParam) )
				pPlayer->GetCharInfo()->m_QMonsterBible.WriteMonsterInfo( pQItemDesc->m_nParam );
		}
		else if( 0 != pQItemDesc->m_nParam )
		{
			// Param값이 설정되어 있는 아이템은 따로 처리를 해줘야 함.				
		}
		else
		{
			// DB에 저장이 되는 퀘스트 아이템만 유저한테 저장함.
			InsertNoParamQItemToPlayer( pPlayer, pQItem );
		}

		MTD_QuestItemNode* pQuestItemNode;
		pQuestItemNode = reinterpret_cast< MTD_QuestItemNode* >( MGetBlobArrayElement(pSimpleQuestItemBlob, nBlobIndex++) );
		Make_MTDQuestItemNode( pQuestItemNode, pQItem->GetItemID(), pQItem->GetCount() );
	}

	*ppoutSimpleQuestItemBlob = pSimpleQuestItemBlob;
	return true;
}

bool MMatchRuleQuest::DistributeZItem( MQuestPlayerInfo* pPlayerInfo, void** ppoutQuestRewardZItemBlob)
{
	MMatchObject* pPlayer = pPlayerInfo->pObject;
	if (!IsEnabledObject(pPlayer)) return false;

	MQuestRewardZItemList* pObtainZItemList = &pPlayerInfo->RewardZItemList;

	// Client로 전송할수 있는 형태로 Quest item정보를 저장할 Blob생성.
	void* pSimpleZItemBlob = MMakeBlobArray( sizeof(MTD_QuestZItemNode), (int)(pObtainZItemList->size()) );
	if( 0 == pSimpleZItemBlob )
	{
		mlog( "MMatchRuleQuest::DistributeZItem - Ztem 정보를 보낼 Blob생성에 실패.\n" );
		return false;
	}

	// 캐시 아이템 획득 로그를 남기기 위함.
	if( !m_QuestGameLogInfoMgr.AddRewardZItemInfo(pPlayer->GetUID(), pObtainZItemList) )
	{
		mlog( "m_QuestGameLogInfoMgr -해당 유저의 로그객체를 찾는데 실패." );
	}

	int nBlobIndex = 0;
	for(MQuestRewardZItemList::iterator itor = pObtainZItemList->begin(); itor != pObtainZItemList->end(); ++itor )
	{
		RewardZItemInfo iteminfo = (*itor);
		MMatchItemDesc* pItemDesc = MGetMatchItemDescMgr()->GetItemDesc(iteminfo.nItemID);
		if (pItemDesc == NULL) continue;

		if (!IsEquipableItem(iteminfo.nItemID, MAX_LEVEL, pPlayer->GetCharInfo()->m_nSex)) 
			continue;

		// 실제로 아이템 등록
		MMatchServer::GetInstance()->InsertCharItem(pPlayer->GetUID(), iteminfo.nItemID, true, iteminfo.nRentPeriodHour);

		// 블롭생성
		MTD_QuestZItemNode* pZItemNode = (MTD_QuestZItemNode*)(MGetBlobArrayElement(pSimpleZItemBlob, nBlobIndex++));
		pZItemNode->m_nItemID = iteminfo.nItemID;
		pZItemNode->m_nRentPeriodHour = iteminfo.nRentPeriodHour;
	}

	*ppoutQuestRewardZItemBlob = pSimpleZItemBlob;

	return true;
}

void MMatchRuleQuest::RouteRewardCommandToStage( MMatchObject* pPlayer, const int nRewardXP, const int nRewardBP, void* pSimpleQuestItemBlob, void* pSimpleZItemBlob)
{
	if( !IsEnabledObject(pPlayer) || (0 == pSimpleQuestItemBlob) )
		return;

	MCommand* pNewCmd = MMatchServer::GetInstance()->CreateCommand( MC_MATCH_USER_REWARD_QUEST, MUID(0, 0) );
	if( 0 == pNewCmd )
		return;

	pNewCmd->AddParameter( new MCmdParamInt(nRewardXP) );
	pNewCmd->AddParameter( new MCmdParamInt(nRewardBP) );
	pNewCmd->AddParameter( new MCommandParameterBlob(pSimpleQuestItemBlob, MGetBlobArraySize(pSimpleQuestItemBlob)) );
	pNewCmd->AddParameter( new MCommandParameterBlob(pSimpleZItemBlob, MGetBlobArraySize(pSimpleZItemBlob)) );

	MMatchServer::GetInstance()->RouteToListener( pPlayer, pNewCmd );
}



void MMatchRuleQuest::OnRequestPlayerDead(const MUID& uidVictim)
{
	MQuestPlayerManager::iterator itor = m_PlayerManager.find(uidVictim);
	if (itor != m_PlayerManager.end())
	{
		MQuestPlayerInfo* pPlayerInfo = (*itor).second;
		pPlayerInfo->nDeathCount++;
	}
}


void MMatchRuleQuest::OnObtainWorldItem(MMatchObject* pObj, int nItemID, int* pnExtraValues)
{
	if( 0 == pObj )
		return;
	
	if (m_nCombatState != MQUEST_COMBAT_PLAY) return;

	int nQuestItemID = pnExtraValues[0];
	int nRentPeriodHour = pnExtraValues[1];

	if (m_pQuestLevel->OnItemObtained(pObj, (u32)nQuestItemID))
	{
		// true값이면 실제로 먹은것임.

		if (IsQuestItemID(nQuestItemID))
            RouteObtainQuestItem((u32)nQuestItemID);
		else 
			RouteObtainZItem((u32)nQuestItemID);
	}
}


void MMatchRuleQuest::OnRequestDropSacrificeItemOnSlot( const MUID& uidSender, const int nSlotIndex, const u32 nItemID )
{
	if( MSM_TEST == MGetServerConfig()->GetServerMode() ) 
	{
		OnResponseDropSacrificeItemOnSlot( uidSender, nSlotIndex, nItemID );
	}
}


void MMatchRuleQuest::OnResponseDropSacrificeItemOnSlot( const MUID& uidSender, const int nSlotIndex, const u32 nItemID )
{
	if( (MAX_SACRIFICE_SLOT_COUNT > nSlotIndex) && (0 <= nSlotIndex) ) 
	{
		// 중복 검사.
		// if( IsSacrificeItemDuplicated(uidSender, nSlotIndex, nItemID) )
		//	return;
		
		MQuestItemDesc* pQItemDesc = GetQuestItemDescMgr().FindQItemDesc( nItemID );
		if( 0 == pQItemDesc )
		{
			// ItemID가 비 정상적이거나 ItemID에 해당하는 Description이 없음.
			// 여하튼 error...

			mlog( "MMatchRuleBaseQuest::SetSacrificeItemOnSlot - ItemID가 비 정상적이거나 %d에 해당하는 Description이 없음.\n", nItemID );
			ASSERT( 0 );
			return;
		}

		// 아이템의 타입이 희생아이템인 경우만 실행.
		if( pQItemDesc->m_bSecrifice )
		{
			MMatchObject* pPlayer = MMatchServer::GetInstance()->GetObject( uidSender );
			if( !IsEnabledObject(pPlayer) )
			{
				mlog( "MMatchRuleBaseQuest::SetSacrificeItemOnSlot - 비정상 유저.\n" );
				return;
			}

			MMatchStage* pStage = MMatchServer::GetInstance()->FindStage( pPlayer->GetStageUID() );
			if( 0 == pStage )
				return;

			// 아무나 슬롯에 접근할수 있음.

			MQuestItem* pQuestItem = pPlayer->GetCharInfo()->m_QuestItemList.Find( nItemID );
			if( 0 == pQuestItem )
				return;
			
			// 수량이 충분한지 검사.
			int nMySacriQItemCount = CalcuOwnerQItemCount( uidSender, nItemID );
			if( -1 == nMySacriQItemCount )
				return;
			if( nMySacriQItemCount >= pQuestItem->GetCount() )
			{
				// 수량이 부족해서 올리지 못했다고 통보함.
				MCommand* pCmdMore = MMatchServer::GetInstance()->CreateCommand( MC_MATCH_RESPONSE_DROP_SACRIFICE_ITEM, MUID(0, 0) );
				if( 0 == pCmdMore )
					return;

				pCmdMore->AddParameter( new MCmdParamInt(NEED_MORE_QUEST_ITEM) );
				pCmdMore->AddParameter( new MCmdParamUID(uidSender) );
				pCmdMore->AddParameter( new MCmdParamInt(nSlotIndex) );
				pCmdMore->AddParameter( new MCmdParamInt(nItemID) );

				MMatchServer::GetInstance()->RouteToListener( pPlayer, pCmdMore );
				return;
			}

			MCommand* pCmdOk = MMatchServer::GetInstance()->CreateCommand( MC_MATCH_RESPONSE_DROP_SACRIFICE_ITEM, MUID(0, 0) );
			if( 0 == pCmdOk )
			{
				return;
			}

			pCmdOk->AddParameter( new MCmdParamInt(MOK) );
			pCmdOk->AddParameter( new MCmdParamUID(uidSender) );
			pCmdOk->AddParameter( new MCmdParamInt(nSlotIndex) );
			pCmdOk->AddParameter( new MCmdParamInt(nItemID) );
			
			MMatchServer::GetInstance()->RouteToStage( pStage->GetUID(), pCmdOk );
			
			// 일반적인 처리.
			m_SacrificeSlot[ nSlotIndex ].SetAll( uidSender, nItemID );

			// 슬롯의 정보가 업데이트되면 업데이트된 정보를 다시 보내줌.
			RefreshStageGameInfo();
		}
		else
		{
			// 희새아이템이 아님.
			ASSERT( 0 );
			return;
		}// if( pQItemDesc->m_bSecrifice )
	}
	else
	{
		// 슬롯의 인덱스가 비 정상적임.
		mlog( "MMatchRuleBaseQuest::OnResponseDropSacrificeItemOnSlot - %d번 슬롯 인덱스는 유효하지 않는 인덱스임.\n", nSlotIndex );
		ASSERT( 0 );
		return;
	}
}


void MMatchRuleQuest::OnRequestCallbackSacrificeItem( const MUID& uidSender, const int nSlotIndex, const u32 nItemID )
{
	if( MSM_TEST == MGetServerConfig()->GetServerMode() ) 
	{
		OnResponseCallBackSacrificeItem( uidSender, nSlotIndex, nItemID );
	}
}


void MMatchRuleQuest::OnResponseCallBackSacrificeItem( const MUID& uidSender, const int nSlotIndex, const u32 nItemID )
{
	// 아무나 접근할수 있음.

	if( (0 == nItemID) || (0 == m_SacrificeSlot[nSlotIndex].GetItemID()) )
		return;

	if( nItemID != m_SacrificeSlot[nSlotIndex].GetItemID() )
		return;

	MMatchObject* pPlayer = MMatchServer::GetInstance()->GetObject( uidSender );
	if( !IsEnabledObject(pPlayer) )
	{
		mlog( "MMatchRuleBaseQuest::OnResponseCallBackSacrificeItem - 비정상적인 유저.\n" );
		return;
	}

	MMatchStage* pStage = MMatchServer::GetInstance()->FindStage( pPlayer->GetStageUID() );
	if( 0 == pStage )
		return;

	MCommand* pCmdOk = MMatchServer::GetInstance()->CreateCommand( MC_MATCH_RESPONSE_CALLBACK_SACRIFICE_ITEM, MUID(0, 0) );
	if( 0 == pCmdOk )
	{
		return;
	}

	pCmdOk->AddParameter( new MCmdParamInt(MOK) );
	pCmdOk->AddParameter( new MCmdParamUID(uidSender) );									// 아이템 회수를 요청한 아이디.
	pCmdOk->AddParameter( new MCmdParamInt(nSlotIndex) );
	pCmdOk->AddParameter( new MCmdParamInt(nItemID) );

	MMatchServer::GetInstance()->RouteToStage( pPlayer->GetStageUID(), pCmdOk );
	
	m_SacrificeSlot[ nSlotIndex ].Release();	

	// 슬롯의 정보가 업데이트되면 QL을 다시 보내줌.
	RefreshStageGameInfo();
}


bool MMatchRuleQuest::IsSacrificeItemDuplicated( const MUID& uidSender, const int nSlotIndex, const u32 nItemID )
{
	if( (uidSender == m_SacrificeSlot[nSlotIndex].GetOwnerUID()) && (nItemID == m_SacrificeSlot[nSlotIndex].GetItemID()) )
	{
		// 같은 아이템을 올려놓으려고 했기에 그냥 무시해 버림.

		return true;
	}

	return false;
}


/*
 * 스테이지를 나가기전에 처리해야 할 일이 있을경우 여기에 정리함.
 */
void MMatchRuleQuest::PreProcessLeaveStage( const MUID& uidLeaverUID )
{
	MMatchRuleBaseQuest::PreProcessLeaveStage( uidLeaverUID );

	MMatchObject* pPlayer = MMatchServer::GetInstance()->GetObject( uidLeaverUID );
	if( !IsEnabledObject(pPlayer) )
		return;
	
	if( MSM_TEST == MGetServerConfig()->GetServerMode() ) 
	{
		// 스테이지를 나가려는 유저가 이전에 흐생 아이템을 스롯에 올려 놓았는지 검사를 함.
		// 만약 올려놓은 아이템이 있다면 자동으로 회수를 함. - 대기상태일때만 적용
		if (GetStage()->GetState() == STAGE_STATE_STANDBY) 
		{
			// 슬롯이 비어있으면 무시.
			if( (!m_SacrificeSlot[0].IsEmpty()) || (!m_SacrificeSlot[1].IsEmpty()) )
			{	
				for( int i = 0; i < MAX_SACRIFICE_SLOT_COUNT; ++i )
				{
					if( uidLeaverUID == m_SacrificeSlot[i].GetOwnerUID() )
						m_SacrificeSlot[ i ].Release();
				}

				MMatchStage* pStage = MMatchServer::GetInstance()->FindStage( pPlayer->GetStageUID() );
				if( 0 == pStage )
					return;

				// 변경된 슬롯 정보를 보내줌.
				OnResponseSacrificeSlotInfoToStage( pStage->GetUID() );
			}
		}
	}
}


void MMatchRuleQuest::DestroyAllSlot()
{
	// 여기서 슬롯에 올려져있는 아이템을 소멸시킴.

	MMatchObject*	pOwner;
	MQuestItem*		pQItem;
	MUID			uidOwner;
	u32	nItemID;

	for( int i = 0; i < MAX_SACRIFICE_SLOT_COUNT; ++i )
	{
		if( MUID(0, 0) == m_SacrificeSlot[i].GetOwnerUID() )
			continue;

		uidOwner = m_SacrificeSlot[ i ].GetOwnerUID();
		
		// 정상적인 아이템 소유자인지 검사.
		pOwner = MMatchServer::GetInstance()->GetObject( uidOwner );
		if( !IsEnabledObject(pOwner) )
		{
			continue;
		}

		nItemID = m_SacrificeSlot[ i ].GetItemID();

		// 소유자의 정상적인 아이템인지 검사.
		pQItem = pOwner->GetCharInfo()->m_QuestItemList.Find( nItemID );
		if( 0 == pQItem )
		{
			continue;
		}

		m_SacrificeSlot[ i ].Release();

		pQItem->Decrease();
	}
}


///
// First	: 추교성.
// Last		: 추교성.
//
// QL정보의 요청을 처리함. 기본적으로 요청자의 스테이지에 통보함.
///

void MMatchRuleQuest::OnRequestQL( const MUID& uidSender )
{
	if( MSM_TEST == MGetServerConfig()->GetServerMode() ) 
	{
		MMatchObject* pPlayer = MMatchServer::GetInstance()->GetObject( uidSender );
		if( 0 == pPlayer )
		{
			mlog( "MMatchRuleQuest::OnRequestQL - 비정상 유저.\n" );
			return;
		}

		OnResponseQL_ToStage( pPlayer->GetStageUID() );
	}
}


///
// First : 추교성.
// Last  : 추교성.
//
// 요청자의 스테이지에 QL정보를 통보.
///
void MMatchRuleQuest::OnResponseQL_ToStage( const MUID& uidStage )
{
	MMatchStage* pStage = MMatchServer::GetInstance()->FindStage( uidStage );
	if( 0 == pStage )
	{
		mlog( "MMatchRuleQuest::OnRequestQL - 스테이지 검사 실패.\n" );
		return;
	}

	RefreshStageGameInfo();
}

///
// First : 추교성.
// Last  : 추교성.
//
// 현재 스롯의 정보를 요청. 기본적으로 스테이지에 알림.
///
void MMatchRuleQuest::OnRequestSacrificeSlotInfo( const MUID& uidSender )
{
	if( MSM_TEST == MGetServerConfig()->GetServerMode() ) 
	{
		MMatchObject* pPlayer = MMatchServer::GetInstance()->GetObject( uidSender );
		if( 0 == pPlayer )
			return;

		MMatchStage* pStage = MMatchServer::GetInstance()->FindStage( pPlayer->GetStageUID() );
		if( 0 == pStage )
			return;

		OnResponseSacrificeSlotInfoToStage( pStage->GetUID() );
	}
}


///
// First : 추교성.
// Last  : 추교성.
//
// 현재 스롯의 정보를 요청자에 알림.
///
void MMatchRuleQuest::OnResponseSacrificeSlotInfoToListener( const MUID& uidSender )
{
	MMatchObject* pPlayer = MMatchServer::GetInstance()->GetObject( uidSender );
	if( !IsEnabledObject(pPlayer) )
	{
		return;
	}

	MMatchStage* pStage = MMatchServer::GetInstance()->FindStage( pPlayer->GetStageUID() );
	if( 0 == pStage )
		return;

	MCommand* pCmd = MMatchServer::GetInstance()->CreateCommand( MC_MATCH_RESPONSE_SLOT_INFO, MUID(0, 0) );
	if( 0 == pCmd )
		return;

	pCmd->AddParameter( new MCmdParamUID(m_SacrificeSlot[0].GetOwnerUID()) );
	pCmd->AddParameter( new MCmdParamInt(m_SacrificeSlot[0].GetItemID()) );
	pCmd->AddParameter( new MCmdParamUID(m_SacrificeSlot[1].GetOwnerUID()) );
	pCmd->AddParameter( new MCmdParamInt(m_SacrificeSlot[1].GetItemID()) );

	MMatchServer::GetInstance()->RouteToListener( pPlayer, pCmd );
}


///
// First : 추교성.
// Last  : 추교성.
//
// 현재 스롯의 정보를 스테이지에 알림.
///
void MMatchRuleQuest::OnResponseSacrificeSlotInfoToStage( const MUID& uidStage )
{
	MMatchStage* pStage = MMatchServer::GetInstance()->FindStage( uidStage );
	if( 0 == pStage )
		return;

	MCommand* pCmd = MMatchServer::GetInstance()->CreateCommand( MC_MATCH_RESPONSE_SLOT_INFO, MUID(0, 0) );
	if( 0 == pCmd )
		return;

	pCmd->AddParameter( new MCmdParamUID(m_SacrificeSlot[0].GetOwnerUID()) );
	pCmd->AddParameter( new MCmdParamInt(m_SacrificeSlot[0].GetItemID()) );
	pCmd->AddParameter( new MCmdParamUID(m_SacrificeSlot[1].GetOwnerUID()) );
	pCmd->AddParameter( new MCmdParamInt(m_SacrificeSlot[1].GetItemID()) );

	MMatchServer::GetInstance()->RouteToStage( uidStage, pCmd );
}


void MMatchRuleQuest::PostInsertQuestGameLogAsyncJob()
{
	if( MSM_TEST == MGetServerConfig()->GetServerMode() ) 
	{
		CollectEndQuestGameLogInfo();
		m_QuestGameLogInfoMgr.PostInsertQuestGameLog();
	}
}




int MMatchRuleQuest::CalcuOwnerQItemCount( const MUID& uidPlayer, const u32 nItemID )
{
	if(  0 == MMatchServer::GetInstance()->GetObject(uidPlayer) )
		return -1;

	int nCount = 0;
	for( int i = 0; i < MAX_SACRIFICE_SLOT_COUNT; ++i )
	{
		if( (uidPlayer == m_SacrificeSlot[i].GetOwnerUID()) &&
			(nItemID == m_SacrificeSlot[i].GetItemID()) )
		{
			++nCount;
		}
	}

	return nCount;
}


///
// First : 2005.04.18 추교성.
// Last  : 2005.04.18 추교성.
//
// 게임을 시작하기전에 준배향 하는 작업을 수행함.
// 준비 작업중 실패가 있을시는 게임을 시작하지 못하게 해야 함.
///
bool MMatchRuleQuest::PrepareStart()
{
	if( MSM_TEST == MGetServerConfig()->GetServerMode() ) 
	{
		MakeStageGameInfo();

		if ((m_StageGameInfo.nQL < 0) || (m_StageGameInfo.nQL > MAX_QL)) return false;
		if ((m_StageGameInfo.nScenarioID <= 0) || (m_StageGameInfo.nMapsetID <= 0)) return false;

		return true;
	}
	
	return false;
}

void MMatchRuleQuest::MakeStageGameInfo()
{	
	if( MSM_TEST == MGetServerConfig()->GetServerMode() ) 
	{
		if( (GetStage()->GetState() != STAGE_STATE_STANDBY) && (STAGE_STATE_COUNTDOWN != GetStage()->GetState()) )
		{
#ifdef _DEBUG
			mlog( "MMatchRuleQuest::MakeStageGameInfo - %d STAGE_STATE_STANDBY가 아님.\n", GetStage()->GetState() );
#endif
			return;
		}

		// 슬롯에 Level에 맞는 정상적인 아이템이 올려져 있는지 검사가 필요함.
		// 비정상 아이템이 올려져 있을경우 아이템 회수 요청을 해줘야 함.
		int nOutResultQL = -1;

		int nMinPlayerLevel = 1;
		MMatchStage* pStage = GetStage();
		if (pStage != NULL)
		{
			nMinPlayerLevel = pStage->GetMinPlayerLevel();

			// 방장이 운영자이면 최소레벨은 운영자 레벨로 임의지정한다.
			MMatchObject* pMaster = MMatchServer::GetInstance()->GetObject(pStage->GetMasterUID());
			if (IsAdminGrade(pMaster))
			{
				nMinPlayerLevel = pMaster->GetCharInfo()->m_nLevel;
			}
		}

		int nPlayerQL = MQuestFormula::CalcQL( nMinPlayerLevel );
		m_StageGameInfo.nPlayerQL = nPlayerQL;

		unsigned int SQItems[MAX_SCENARIO_SACRI_ITEM];
		for (int i = 0; i < MAX_SCENARIO_SACRI_ITEM; i++)
		{
			SQItems[i] = (unsigned int)m_SacrificeSlot[i].GetItemID();
		}

		MMatchQuest* pQuest = MMatchServer::GetInstance()->GetQuest();
		unsigned int nScenarioID = pQuest->GetScenarioCatalogue()->MakeScenarioID(m_StageGameInfo.nMapsetID,
																				  nPlayerQL, SQItems);

		m_StageGameInfo.nMapsetID = QuestMapNameToID(pStage->GetMapName());

		m_StageGameInfo.nScenarioID = nScenarioID;
		MQuestScenarioInfo* pScenario = pQuest->GetScenarioCatalogue()->GetInfo(nScenarioID);
		if (pScenario)
		{
			m_StageGameInfo.nQL = pScenario->nQL;
		}
		else
		{
			m_StageGameInfo.nQL = nPlayerQL;
		}
	}
}

void MMatchRuleQuest::RefreshStageGameInfo()
{
	MakeStageGameInfo();
	RouteStageGameInfo();
}

void MMatchRuleQuest::OnChangeCondition()
{
	RefreshStageGameInfo();
}

void MMatchRuleQuest::CollectStartingQuestGameLogInfo()
{
	// 수집하기전에 이전의 정보를 반드시 지워야 함.
	m_QuestGameLogInfoMgr.Clear();

	if( QuestTestServer() ) 
	{
		// Master CID
		MMatchObject* pMaster = MMatchServer::GetInstance()->GetObject( GetStage()->GetMasterUID() );
		if( IsEnabledObject(pMaster) )
			m_QuestGameLogInfoMgr.SetMasterCID( pMaster->GetCharInfo()->m_nCID );

		m_QuestGameLogInfoMgr.SetScenarioID( m_pQuestLevel->GetStaticInfo()->pScenario->nID );

		// Stage name 저장.
		m_QuestGameLogInfoMgr.SetStageName( GetStage()->GetName() );

		// 시작할때의 유저 정보를 저장함.
		for(MQuestPlayerManager::iterator it = m_PlayerManager.begin() ; 
				it != m_PlayerManager.end(); ++it )
		{
			m_QuestGameLogInfoMgr.AddQuestPlayer( it->second->pObject->GetUID(), it->second->pObject );
		}

		m_QuestGameLogInfoMgr.SetStartTime( GetGlobalTimeMS() );
	}
}


void MMatchRuleQuest::CollectEndQuestGameLogInfo()
{
	m_QuestGameLogInfoMgr.SetEndTime( GetGlobalTimeMS() );
}


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////	