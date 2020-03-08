#pragma once

#include <vector>
#include <list>
#include <map>
#include <unordered_map>

#include "GlobalTypes.h"
#include "MUID.h"
#include "MMatchWorldItemDesc.h"

using MMatchWorldItemMap = std::unordered_map<unsigned short, MMatchWorldItem>;

class MMatchWorldItemManager
{
private:
	class MMatchStage*					m_pMatchStage = nullptr;
	MMatchWorldItemMap					m_ItemMap;

	std::vector<MMatchWorldItemSpawnInfo> m_SpawnInfos;
	u64									m_nLastTime = 0;

	short								m_nUIDGenerate = 0;
	bool								m_bStarted = false;

	void AddItem(const unsigned short nItemID, short nSpawnIndex, 
				 const float x, const float y, const float z);
	void AddItem(const unsigned short nItemID, short nSpawnIndex, 
				 const float x, const float y, const float z, int nLifeTime, int* pnExtraValues);
	void DelItem(short nUID);
	void DelItem(MMatchWorldItemMap::iterator it);
	void Spawn(int nSpawnIndex);
	void Clear();
	void SpawnInfoInit();
	void ClearItems();

	void RouteSpawnWorldItem(MMatchWorldItem* pWorldItem);
	void RouteObtainWorldItem(const MUID& uidPlayer, int nWorldItemUID);
	void RouteRemoveWorldItem(int nWorldItemUID);
public:
	MMatchWorldItemManager(MMatchStage* Stage) : m_pMatchStage(Stage) {}

	void OnRoundBegin();
	void OnStageBegin(class MMatchStageSetting* pStageSetting);
	void OnStageEnd();
	void Update();

	bool Obtain(MMatchObject* pObj, short nItemUID);
	void SpawnDynamicItem(const int nItemID, const float x, const float y, const float z);
	void SpawnDynamicItem(const int nItemID, const float x, const float y, const float z, 
						  int nLifeTime, int* pnExtraValues );
	void RouteAllItems(MMatchObject* pObj);
};