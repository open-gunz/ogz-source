#include "stdafx.h"
#include "DBQuestCachingData.h"
#include "MMatchConfig.h"
#include "MAsyncDBJob.h"

void DBQuestCachingData::IncreasePlayCount(const int nCount)
{
	m_nPlayCount += nCount;
	DoUpdateDBCharQuestItemInfo();
}


void DBQuestCachingData::IncreaseShopTradeCount(const int nCount)
{
	m_nShopTradeCount += nCount;
	DoUpdateDBCharQuestItemInfo();
}


void DBQuestCachingData::IncreaseRewardCount(const int nCount)
{
	m_nRewardCount += nCount;
	DoUpdateDBCharQuestItemInfo();
}


bool DBQuestCachingData::CheckUniqueItem(MQuestItem* pQuestItem)
{
	if ((0 == pQuestItem) || (0 == pQuestItem->GetDesc()))
		return false;

	if (pQuestItem->GetDesc()->m_bUnique)
		m_bEnableUpdate = true;

	DoUpdateDBCharQuestItemInfo();

	return m_bEnableUpdate;
}


void DBQuestCachingData::Reset()
{
	m_dwLastUpdateTime = GetGlobalTimeMS();
	m_nPlayCount = 0;
	m_nShopTradeCount = 0;
	m_bEnableUpdate = false;
	m_nRewardCount = 0;
}


bool DBQuestCachingData::DoUpdateDBCharQuestItemInfo()
{
	// 퀘스트 서버인지 먼저 검사.
	if (MSM_TEST != MGetServerConfig()->GetServerMode())
		return false;

	// 정상적인 Object인지 검사.
	if (!IsEnabledObject(m_pObject))
		return false;

	// 현재 상태가 업데이트 가능한지 검사.
	if (!IsRequestUpdate())
	{
		// 다음 업데이트를 검사를 위해서 마지막 업데이트 검사 시간을 저장해 놓음.
		m_dwLastUpdateTime = GetGlobalTimeMS();
		return false;
	}

	MAsyncDBJob_UpdateQuestItemInfo* pAsyncJob = new MAsyncDBJob_UpdateQuestItemInfo;
	if (0 == pAsyncJob)
	{
		mlog("DBQuestCachingData::DoUpdateDBCharQuestItemInfo - QuestItemUpdate async작업 실패.\n");
		return false;
	}
	if (!pAsyncJob->Input(m_pObject->GetCharInfo()->m_nCID,
		m_pObject->GetCharInfo()->m_QuestItemList,
		m_pObject->GetCharInfo()->m_QMonsterBible))
	{
		return false;
	}

	MMatchServer::GetInstance()->PostAsyncJob(pAsyncJob);

#ifdef _DEBUG
	{
		// 업데이트 정보가 정상적으로 되는지 로그를 남김.
		char szDbgOut[1000] = { 0 };
		MQuestItemMap::iterator it, end;

		strcat_safe(szDbgOut, "Quest Item Caching UpdateDB\n");
		strcat_safe(szDbgOut, m_pObject->GetName());
		strcat_safe(szDbgOut, "\n");

		it = m_pObject->GetCharInfo()->m_QuestItemList.begin();
		end = m_pObject->GetCharInfo()->m_QuestItemList.end();

		for (; it != end; ++it)
		{
			char tmp[100] = { 0 };
			sprintf_safe(tmp, "%s : %d\n", it->second->GetDesc()->m_szQuestItemName, it->second->GetCount());
			strcat_safe(szDbgOut, tmp);
		}
		strcat_safe(szDbgOut, "\n");
		MMatchServer::GetInstance()->LOG(MMatchServer::LOG_ALL, szDbgOut);
	}
#endif

	// 업데이트가 성공하면 다음 검사를 위해서 다시 설정함.
	Reset();

	return true;
}