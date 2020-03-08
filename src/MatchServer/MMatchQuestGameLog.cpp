#include "stdafx.h"
#include "MMatchQuestGameLog.h"
#include "MQuestConst.h"
#include "MAsyncDBJob.h"


void MQuestPlayerLogInfo::AddUniqueItem( const u32 nItemID, int nCount )
{
	if (IsQuestItemID(nItemID))
	{
		// 유니크 아이템들은 다른 테이블에 추가적인 정보를 저장하기위해서 따로 저장해 놓아야 함.
		MQuestItemDesc* pQItemDesc = GetQuestItemDescMgr().FindQItemDesc( nItemID );
		if( 0 == pQItemDesc )
			return;

		if( !pQItemDesc->m_bUnique )
			return;
	}

	m_UniqueItemList.insert( map<u32, int>::value_type(nItemID, nCount) );
}


/////////////////////////////////////////////////////////////////////////////////////////////////////

MMatchQuestGameLogInfoManager::MMatchQuestGameLogInfoManager() : m_nMasterCID( 0 ), m_nScenarioID( 0 ), m_dwStartTime( 0 ), m_dwEndTime( 0 ), m_nTotalRewardQItemCount( 0 )
{
	memset( m_szStageName, 0, 64 );
}


MMatchQuestGameLogInfoManager::~MMatchQuestGameLogInfoManager()
{
	Clear();
}


void MMatchQuestGameLogInfoManager::AddQuestPlayer( const MUID& uidPlayer, MMatchObject* pPlayer )
{
	if( (0 == pPlayer) || (uidPlayer != pPlayer->GetUID()) )
		return;

	MQuestPlayerLogInfo* pQuestPlayerLogInfo = new MQuestPlayerLogInfo;
	if( 0 == pQuestPlayerLogInfo )
		return;

	pQuestPlayerLogInfo->SetCID( pPlayer->GetCharInfo()->m_nCID );

	insert( value_type(uidPlayer, pQuestPlayerLogInfo) );
}

///
// 퀘스트를 클리어하고서 보상받은 아이템을 등록함.
// 끝나고 로그에 저장됨.
///
bool MMatchQuestGameLogInfoManager::AddRewardQuestItemInfo( const MUID& uidPlayer, MQuestItemMap* pObtainQuestItemList )
{
	MQuestPlayerLogInfo* pQuestPlayerLogInfo = Find( uidPlayer );
	if( 0 == pQuestPlayerLogInfo )
		return false;

	if( 0 == pObtainQuestItemList )
		return false;

	// 만약을 위해서 퀘스트 아이템에 관련된 모든 정보를 지운다.
	pQuestPlayerLogInfo->ClearQItemInfo();

	MQuestItemMap::iterator itQItem, endQItem;
	endQItem = pObtainQuestItemList->end();
	for( itQItem = pObtainQuestItemList->begin(); itQItem != endQItem; ++itQItem )
	{
		if( GetQuestItemDescMgr().FindQItemDesc(itQItem->second->GetItemID())->m_bUnique )
			pQuestPlayerLogInfo->AddUniqueItem( itQItem->second->GetItemID(), itQItem->second->GetCount() );

		m_nTotalRewardQItemCount += itQItem->second->GetCount();
	}

	return true;
}

bool MMatchQuestGameLogInfoManager::AddRewardZItemInfo( const MUID& uidPlayer, MQuestRewardZItemList* pObtainZItemList )
{
	MQuestPlayerLogInfo* pQuestPlayerLogInfo = Find( uidPlayer );
	if (( 0 == pQuestPlayerLogInfo ) || ( 0 == pObtainZItemList )) return false;

	for(MQuestRewardZItemList::iterator itor = pObtainZItemList->begin(); itor != pObtainZItemList->end(); ++itor )
	{
		RewardZItemInfo iteminfo = (*itor);
		MMatchItemDesc* pItemDesc = MGetMatchItemDescMgr()->GetItemDesc(iteminfo.nItemID);
		if (pItemDesc == NULL) continue;

		// 유니크 로그 생성
		pQuestPlayerLogInfo->AddUniqueItem( iteminfo.nItemID, 1);
	}

	return true;
}

void MMatchQuestGameLogInfoManager::Clear()
{
	if( empty() )
		return;

	// Player제거.
	MMatchQuestGameLogInfoManager::iterator It, End;
	for( It = begin(), End = end(); It != End; ++It )
		delete It->second;
	
	clear();

	m_nTotalRewardQItemCount = 0;
}

MQuestPlayerLogInfo* MMatchQuestGameLogInfoManager::Find( const MUID& uidPlayer )
{
	MMatchQuestGameLogInfoManager::iterator It = find( uidPlayer );
	if( end() == It )
		return 0;

	MQuestPlayerLogInfo* pQuestPlayerLogInfo = It->second;
	if( 0 == pQuestPlayerLogInfo )
		return 0;

	return pQuestPlayerLogInfo;
}


///
// First : 2005.04.18 추교성.
// Last  : 2005.04.18 추교성.
//
// 퀘스트가 완료되면 저장되있던 정보를 가지고 디비에 로그를 남기는 작업을 함.
///
bool MMatchQuestGameLogInfoManager::PostInsertQuestGameLog()
{
	auto nElapsedPlayTime = static_cast<int>((m_dwEndTime - m_dwStartTime) / 60000);

	MAsyncDBJob_InsertQuestGameLog* pAsyncDbJob_InsertGameLog = new MAsyncDBJob_InsertQuestGameLog;
	if( 0 == pAsyncDbJob_InsertGameLog )
		return false;

	pAsyncDbJob_InsertGameLog->Input( m_szStageName, 
									  m_nScenarioID, 
									  m_nMasterCID, 
									  this,
                                      m_nTotalRewardQItemCount, 
									  nElapsedPlayTime );

	MMatchServer::GetInstance()->PostAsyncJob( pAsyncDbJob_InsertGameLog );

	return true;
}


void MMatchQuestGameLogInfoManager::SetStageName( const char* pszStageName )
{
	if( (0 == pszStageName) || (64 < strlen(pszStageName)) )
		return;

	strcpy_safe( m_szStageName, pszStageName );
}