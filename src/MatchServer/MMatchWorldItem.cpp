#include "stdafx.h"
#include "MMatchWorldItem.h"
#include "MMatchServer.h"
#include "MMatchStage.h"
#include "MMatchObject.h"
#include "MSharedCommandTable.h"
#include "MBlobArray.h"
#include "MZFileSystem.h"
#include "MUID.h"
#include "MMatchWorldItemDesc.h"
#include "RTypes.h"
#include <algorithm>

void MMatchWorldItemManager::ClearItems()
{
	m_nLastTime = 0;
	m_nUIDGenerate = 0;
	m_ItemMap.clear();
}

void MMatchWorldItemManager::Clear()
{
	ClearItems();
	m_SpawnInfos.clear();
}

void MMatchWorldItemManager::OnRoundBegin()
{
	ClearItems();
	SpawnInfoInit();
}

void MMatchWorldItemManager::SpawnInfoInit()
{
	if (!m_pMatchStage) return;

	for (auto& SpawnInfo : m_SpawnInfos)
	{
		SpawnInfo.nElapsedTime = 0;
		SpawnInfo.bExist = false;
		SpawnInfo.bUsed = true;

		// Don't spawn ammo worlditems in gladiator rooms
		if (IsGameRuleGladiator(m_pMatchStage->GetStageSetting()->GetGameType()))
		{
			auto* wi = MGetMatchWorldItemDescMgr()->GetItemDesc(SpawnInfo.nItemID);
			if (wi && wi->m_nItemType == WIT_BULLET)
			{
				SpawnInfo.bUsed = false;
			}
		}
	}
}

void MMatchWorldItemManager::Update()
{
	if (!m_bStarted) return;
	if (m_pMatchStage == NULL) return;

	auto nNowTime = GetGlobalTimeMS();
	int nDeltaTime = 0;

	if (m_nLastTime != 0)
		nDeltaTime = static_cast<int>(nNowTime - m_nLastTime);	

	// Respawn map worlditems
	for (size_t i = 0; i < m_SpawnInfos.size(); i++)
	{
		auto& SpawnInfo = m_SpawnInfos[i];
		if (!SpawnInfo.bUsed || SpawnInfo.bExist)
			continue;

		SpawnInfo.nElapsedTime += nDeltaTime;

		if (SpawnInfo.nElapsedTime < SpawnInfo.nCoolTime)
			continue;

		Spawn(i);
	}

	// Remove worlditems that have been alive for too long
	for (auto it = m_ItemMap.begin(); it != m_ItemMap.end(); it++)
	{
		auto& WorldItem = it->second;
		
		if (WorldItem.nStaticSpawnIndex < 0 && WorldItem.nLifeTime > 0)
		{
			WorldItem.nLifeTime -= nDeltaTime;

			if (WorldItem.nLifeTime <= 0)
			{
				RouteRemoveWorldItem(WorldItem.nUID);
				it = m_ItemMap.erase(it);
				if (it == m_ItemMap.end())
					break;
			}
		}
	}
	
	if (m_pMatchStage->GetStageSetting()->GetNetcode() == NetcodeType::ServerBased)
	{
		auto WORLDITEM_PICKUP_RADIUS = 100.0f;
		for (auto it = m_ItemMap.begin(); it != m_ItemMap.end();)
		{
			auto& WorldItem = it->second;

			bool PickedUp = false;
			for (auto* Player : m_pMatchStage->GetObjectList())
			{
				if (RealSpace2::Magnitude(WorldItem.Origin - Player->GetPosition()) < WORLDITEM_PICKUP_RADIUS)
				{
					// This takes care of deleting them also
					it = std::next(it);
					Obtain(Player, WorldItem.nUID);
					PickedUp = true;
					break;
				}
			}

			if (!PickedUp)
				it++;
		}
	}

	m_nLastTime = nNowTime;
}

static void Make_MTDWorldItem(MTD_WorldItem* pOut, MMatchWorldItem* pWorldItem)
{
	pOut->nUID = pWorldItem->nUID;
	pOut->nItemID = pWorldItem->nItemID;
	if ((pWorldItem->nStaticSpawnIndex < 0) && (pWorldItem->nLifeTime > 0))
		pOut->nItemSubType = MTD_Dynamic;
	else
		pOut->nItemSubType = MTD_Static;

	pOut->x = (short)Roundf(pWorldItem->Origin.x);
	pOut->y = (short)Roundf(pWorldItem->Origin.y);
	pOut->z = (short)Roundf(pWorldItem->Origin.z);
}

void MMatchWorldItemManager::RouteAllItems(MMatchObject* pObj)
{
	int nItemSize = (int)m_ItemMap.size();
	if (nItemSize <= 0) return;

	void* pItemArray = MMakeBlobArray(sizeof(MTD_WorldItem), nItemSize);
	

	int nIndex = 0;
	for (auto& WorldItem : MakePairValueAdapter(m_ItemMap))
	{
		MTD_WorldItem* pNode = (MTD_WorldItem*)MGetBlobArrayElement(pItemArray, nIndex++);
		Make_MTDWorldItem(pNode, &WorldItem);

		// What is this even doing?
		if (m_pMatchStage->GetRule()->GetGameType() == MMATCH_GAMETYPE_DUEL && WorldItem.nItemID < 100)
			return;
	}

	MCommand* pCmd = MMatchServer::GetInstance()->CreateCommand(MC_MATCH_SPAWN_WORLDITEM, MUID(0,0));
	pCmd->AddParameter(new MCommandParameterBlob(pItemArray, MGetBlobArraySize(pItemArray)));
	MEraseBlobArray(pItemArray);

	MMatchServer::GetInstance()->RouteToListener(pObj, pCmd);
}

void MMatchWorldItemManager::AddItem(const unsigned short nItemID, short nSpawnIndex, 
									 const float x, const float y, const float z)
{
	if (!m_pMatchStage) return;

	m_nUIDGenerate++;

	MMatchWorldItem NewWorldItem;
	NewWorldItem.nUID = m_nUIDGenerate;
	NewWorldItem.nItemID = nItemID;
	NewWorldItem.nStaticSpawnIndex = nSpawnIndex;
	NewWorldItem.Origin = { x, y, z };
	NewWorldItem.nLifeTime = -1;
	for (auto& val : NewWorldItem.nExtraValue)
		val = 0;

	m_ItemMap.emplace(m_nUIDGenerate, NewWorldItem);

	RouteSpawnWorldItem(&NewWorldItem);
}

void MMatchWorldItemManager::AddItem(const unsigned short nItemID, short nSpawnIndex, 
									 const float x, const float y, const float z, int nLifeTime, 
									 int* pnExtraValues )
{
	if (m_pMatchStage == NULL) return;
	m_nUIDGenerate++;

	MMatchWorldItem NewWorldItem;
	NewWorldItem.nUID = m_nUIDGenerate;
	NewWorldItem.nItemID = nItemID;
	NewWorldItem.nStaticSpawnIndex = nSpawnIndex;
	NewWorldItem.Origin = { x, y, z };
	NewWorldItem.nLifeTime= nLifeTime;
	for (size_t i = 0; i < std::size(NewWorldItem.nExtraValue); i++)
		NewWorldItem.nExtraValue[i] = pnExtraValues[i];

	m_ItemMap.emplace(m_nUIDGenerate, NewWorldItem);

	RouteSpawnWorldItem(&NewWorldItem);
}


void MMatchWorldItemManager::OnStageBegin(MMatchStageSetting* pStageSetting)
{
	if (m_pMatchStage == NULL) return;

	int nMapID = pStageSetting->GetMapIndex();
	bool bIsTeamPlay = pStageSetting->IsTeamPlay();

	Clear();

	if ((nMapID < 0) || (nMapID >= MMATCH_MAP_COUNT)) return;
	if (MGetGameTypeMgr()->IsWorldItemSpawnEnable(pStageSetting->GetGameType()))
	{
		MMatchMapsWorldItemSpawnInfoSet* pSpawnInfoSet = &MGetMapsWorldItemSpawnInfo()->m_MapsSpawnInfo[nMapID];

		if (bIsTeamPlay)
		{
			int nSpawnCount = MGetMapsWorldItemSpawnInfo()->m_MapsSpawnInfo[nMapID].m_nTeamSpawnCount;
			if (nSpawnCount > 0)
			{
				m_SpawnInfos.resize(nSpawnCount);
				
				for (int i = 0; i < nSpawnCount; i++)
				{
					m_SpawnInfos[i].x = pSpawnInfoSet->TeamSpawnInfo[i].x;
					m_SpawnInfos[i].y = pSpawnInfoSet->TeamSpawnInfo[i].y;
					m_SpawnInfos[i].z = pSpawnInfoSet->TeamSpawnInfo[i].z;
					m_SpawnInfos[i].nCoolTime = pSpawnInfoSet->TeamSpawnInfo[i].nCoolTime;
					m_SpawnInfos[i].nItemID = pSpawnInfoSet->TeamSpawnInfo[i].nItemID;
				}
			}
		}
		else
		{
			int nSpawnCount = MGetMapsWorldItemSpawnInfo()->m_MapsSpawnInfo[nMapID].m_nSoloSpawnCount;

			if (nSpawnCount > 0)
			{
				m_SpawnInfos.resize(nSpawnCount);

				for (int i = 0; i < nSpawnCount; i++)
				{
					m_SpawnInfos[i].x = pSpawnInfoSet->SoloSpawnInfo[i].x;
					m_SpawnInfos[i].y = pSpawnInfoSet->SoloSpawnInfo[i].y;
					m_SpawnInfos[i].z = pSpawnInfoSet->SoloSpawnInfo[i].z;
					m_SpawnInfos[i].nCoolTime = pSpawnInfoSet->SoloSpawnInfo[i].nCoolTime;
					m_SpawnInfos[i].nItemID = pSpawnInfoSet->SoloSpawnInfo[i].nItemID;
				}
			}
		}
	}

	SpawnInfoInit();

	m_bStarted = true;
}

void MMatchWorldItemManager::OnStageEnd()
{
	m_bStarted = false;
}

void MMatchWorldItemManager::Spawn(int nSpawnIndex)
{
	m_SpawnInfos[nSpawnIndex].bExist = true;
	m_SpawnInfos[nSpawnIndex].nElapsedTime = 0;

	AddItem(m_SpawnInfos[nSpawnIndex].nItemID, nSpawnIndex, 
		    m_SpawnInfos[nSpawnIndex].x, m_SpawnInfos[nSpawnIndex].y, m_SpawnInfos[nSpawnIndex].z);
}

void MMatchWorldItemManager::DelItem(short nUID)
{
	auto it = m_ItemMap.find(nUID);
	if (it != m_ItemMap.end())
	{
		DelItem(it);
	}
}

void MMatchWorldItemManager::DelItem(MMatchWorldItemMap::iterator it)
{
	int nSpawnIndex = it->second.nStaticSpawnIndex;

	if (nSpawnIndex >= 0 && nSpawnIndex < static_cast<int>(m_SpawnInfos.size()))
	{
		m_SpawnInfos[nSpawnIndex].bExist = false;
		m_SpawnInfos[nSpawnIndex].nElapsedTime = 0;
	}

	m_ItemMap.erase(it);
}


bool MMatchWorldItemManager::Obtain(MMatchObject* pObj, short nItemUID)
{
	if (!m_pMatchStage) return false;

	auto it = m_ItemMap.find(nItemUID);
	if (it == m_ItemMap.end())
		return false;

	auto& WorldItem = it->second;

	auto* Rule = m_pMatchStage->GetRule();
	if (Rule)
		Rule->OnObtainWorldItem(pObj, WorldItem.nItemID, WorldItem.nExtraValue);

	RouteObtainWorldItem(pObj->GetUID(), static_cast<int>(nItemUID));

	if (m_pMatchStage->GetStageSetting()->GetNetcode() == NetcodeType::ServerBased)
	{
		auto desc_it = MGetMatchWorldItemDescMgr()->find(WorldItem.nItemID);
		if (desc_it != MGetMatchWorldItemDescMgr()->end())
		{
			pObj->Heal(static_cast<int>(desc_it->second->m_fAmount));
		}
	}

	DelItem(nItemUID);

	return true;
}

void MMatchWorldItemManager::SpawnDynamicItem(const int nItemID, const float x, const float y, const float z)
{
	if (m_pMatchStage == NULL) return;

	AddItem(nItemID, -1, x, y, z);
}

void MMatchWorldItemManager::SpawnDynamicItem(const int nItemID, const float x, const float y, const float z, 
											  int nLifeTime, int* pnExtraValues )
{
	if (m_pMatchStage == NULL) return;

	AddItem(nItemID, -1, x, y, z, nLifeTime, pnExtraValues );
}

void MMatchWorldItemManager::RouteObtainWorldItem(const MUID& uidPlayer, int nWorldItemUID)
{
	MCommand* pCmd = MMatchServer::GetInstance()->CreateCommand(MC_MATCH_OBTAIN_WORLDITEM, MUID(0,0));
	pCmd->AddParameter(new MCmdParamUID(uidPlayer));
	pCmd->AddParameter(new MCmdParamInt(nWorldItemUID));
	MMatchServer::GetInstance()->RouteToBattle(m_pMatchStage->GetUID(), pCmd);
}

void MMatchWorldItemManager::RouteRemoveWorldItem(int nWorldItemUID)
{
	MCommand* pCmd = MMatchServer::GetInstance()->CreateCommand(MC_MATCH_REMOVE_WORLDITEM, MUID(0,0));
	pCmd->AddParameter(new MCmdParamInt(nWorldItemUID));
	MMatchServer::GetInstance()->RouteToBattle(m_pMatchStage->GetUID(), pCmd);
}

void MMatchWorldItemManager::RouteSpawnWorldItem(MMatchWorldItem* pWorldItem)
{
	if (m_pMatchStage->GetRule()->GetGameType() == MMATCH_GAMETYPE_DUEL && pWorldItem->nItemID < 100)
		return;

	MCommand* pCmd = MMatchServer::GetInstance()->CreateCommand(MC_MATCH_SPAWN_WORLDITEM, MUID(0,0));

	void* pItemArray = MMakeBlobArray(sizeof(MTD_WorldItem), 1);
	MTD_WorldItem* pNode = (MTD_WorldItem*)MGetBlobArrayElement(pItemArray, 0);

	Make_MTDWorldItem(pNode, pWorldItem);

	pCmd->AddParameter(new MCommandParameterBlob(pItemArray, MGetBlobArraySize(pItemArray)));
	MEraseBlobArray(pItemArray);

	MMatchServer::GetInstance()->RouteToBattle(m_pMatchStage->GetUID(), pCmd);
}