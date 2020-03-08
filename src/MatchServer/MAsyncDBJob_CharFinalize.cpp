#include "stdafx.h"
#include "MAsyncDBJob_CharFinalize.h"
#include "MMatchConfig.h"

void MAsyncDBJob_CharFinalize::Run(void* pContext)
{
	auto* pDBMgr = static_cast<IDatabase*>(pContext);

	MASYNC_RESULT nResult = MASYNC_RESULT_SUCCEED;

	if (!pDBMgr->UpdateCharPlayTime(m_nCID, m_nPlayTime))
	{
		nResult = MASYNC_RESULT_FAILED;
//		mlog("DB Query(CharFinalize > UpdateCharPlayTime) Failed\n");
	}
	if (!pDBMgr->InsertPlayerLog(m_nCID,
								 m_nPlayTime, 
								 m_nConnKillCount, 
								 m_nConnDeathCount,
                                 m_nConnXP, 
								 m_nXP ))
	{
		nResult = MASYNC_RESULT_FAILED;
//		mlog("DB Query(CharFinalize > InsertPlayerLog) Failed\n");
	}

#ifdef _QUEST_ITEM
	// 퀘스트 서버일 경우만 퀘스트 아이템 리스트를 업데이트 함.
	if ( QuestTestServer() )
	{
		if( m_QuestItemMap.IsDoneDbAccess() && m_bIsRequestQItemUpdate)
		{
			if( !pDBMgr->UpdateQuestItem(m_nCID, m_QuestItemMap, m_QuestMonster) )
				nResult = MASYNC_RESULT_FAILED;
		}
	}
#endif

	SetResult(nResult);
}


bool MAsyncDBJob_CharFinalize::Input(int nCID, 
 									 u32 nPlayTime, 
									 int nConnKillCount, 
									 int nConnDeathCount, 
									 int nConnXP, 
									 int nXP,
									 MQuestItemMap& rfQuestItemMap,
									 MQuestMonsterBible& rfQuestMonster,
									 const bool bIsRequestQItemUpdate )
{
	m_nCID = nCID;
	m_nPlayTime = nPlayTime;
	m_nConnKillCount = nConnKillCount;
	m_nConnDeathCount = nConnDeathCount;
	m_nConnXP = nConnXP;
	m_nXP = nXP;


#ifdef _QUEST_ITEM
	// 퀘스트 서버일 경우만 퀘스트 아이템 리스트를 업데이트 함.
	if( QuestTestServer() ) 
	{
		m_bIsRequestQItemUpdate = bIsRequestQItemUpdate;

		if( bIsRequestQItemUpdate )
		{
			m_QuestItemMap.SetDBAccess( rfQuestItemMap.IsDoneDbAccess() );

			MQuestItemMap::iterator	it, end;

			m_QuestItemMap.Clear();

			it  = rfQuestItemMap.begin();
			end = rfQuestItemMap.end();
			for( ; it != end; ++it )
			{
				MQuestItem* pQuestItem = it->second;
				m_QuestItemMap.CreateQuestItem( pQuestItem->GetItemID(), pQuestItem->GetCount(), pQuestItem->IsKnown());
			}

			memset( &m_QuestMonster, 0, sizeof(rfQuestMonster) );
			memcpy( &m_QuestMonster, &rfQuestMonster, sizeof(rfQuestMonster) );
		}
	}
#endif

	return true;
}
