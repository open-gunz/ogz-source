#include "stdafx.h"
#include "MMatchServer.h"
#include "MAsyncDBJob.h"
#include "MSharedCommandTable.h"
#include "MBlobArray.h"
#include "MMatchConfig.h"
#include "MMatchQuestGameLog.h"

#include <algorithm>
using std::max;
using std::min;

void MAsyncDBJob_Test::Run(void* pContext)
{
}

void MAsyncDBJob_GetAccountCharList::Run(void* pContext)
{
	auto* pDBMgr = static_cast<IDatabase*>(pContext);

	MTD_AccountCharInfo charlist[MAX_CHAR_COUNT];
	int nCharCount = 0;

	if (!pDBMgr->GetAccountCharList(m_nAID, charlist, &nCharCount))
	{
		SetResult(MASYNC_RESULT_FAILED);
		return;
	}

	MMatchServer* pServer = MMatchServer::GetInstance();

	MCommand* pNewCmd = pServer->CreateCommand(MC_MATCH_RESPONSE_ACCOUNT_CHARLIST, MUID(0,0));
	void* pCharArray = MMakeBlobArray(sizeof(MTD_AccountCharInfo), nCharCount);

	for (int i = 0; i < nCharCount; i++)
	{
		MTD_AccountCharInfo* pTransCharInfo = (MTD_AccountCharInfo*)MGetBlobArrayElement(pCharArray, i);
		memcpy(pTransCharInfo, &charlist[i], sizeof(MTD_AccountCharInfo));

		m_nCharMaxLevel = max(m_nCharMaxLevel, int(pTransCharInfo->nLevel));
	}

	pNewCmd->AddParameter(new MCommandParameterBlob(pCharArray, MGetBlobArraySize(pCharArray)));
	MEraseBlobArray(pCharArray);

	m_pResultCommand = pNewCmd;
	SetResult(MASYNC_RESULT_SUCCEED);
}

void MAsyncDBJob_GetCharInfo::Run(void* pContext)
{
	_ASSERT(m_pCharInfo);
	auto* pDBMgr = static_cast<IDatabase*>(pContext);

	int nWaitHourDiff;

	if (!pDBMgr->GetCharInfoByAID(m_nAID, m_nCharIndex, m_pCharInfo, nWaitHourDiff))
	{
		SetResult(MASYNC_RESULT_FAILED);
		return;
	}

#ifdef _DELETE_CLAN
	// 클랜에 가입된 캐릭터이면 폐쇄요청된 클랜인지 검사를 해줘야 한다.
	if( 0 != m_pCharInfo->m_ClanInfo.m_nClanID )
	{
		// nWaitHourDiff의 값이 0이면 정상 클랜.
		// 0 이상이면 폐쇄요청이 접수된 클랜이다.
		// 폐쇄 요청된 클랜을 정상 클랜으로 바꿔주기 위해서는,
		//  Clan테이블의 DeleteDate를 NULL로 셋팅해 줘야 한다. - by SungE.

		if( UNDEFINE_DELETE_HOUR == nWaitHourDiff )
		{
			// 정상 클랜.
		}
		else if( 0 > nWaitHourDiff )
		{
			SetDeleteState( MMCDS_WAIT );
		}
		//else if( MAX_WAIT_CLAN_DELETE_HOUR < nWaitHourDiff )
		//{
		//	// 클랜정보를 DB에서 삭제.
		//	// 이작업은 DB의 Agent server작업으로 일괄 처리히한다.
		//}
		else if( 0 <= nWaitHourDiff)
		{
			// 아직 DB는 삭제하지 않고, 유저만 일반 유저로 처리함.
			SetDeleteState( MMCDS_NORMAL );
			m_pCharInfo->m_ClanInfo.Clear();
		}
	}
#endif

	// 디비에서 아이템 정보를 가져온다.
	// 이것은 퍼포먼스 문제로 나중에 플레이어가 자기 아이템 보기 할때만 가져와야 할듯
	m_pCharInfo->ClearItems();
	if (!pDBMgr->GetCharItemInfo(*m_pCharInfo))
	{
		SetResult(MASYNC_RESULT_FAILED);
		return;
	}

#ifdef _QUEST_ITEM
	if( MSM_TEST == MGetServerConfig()->GetServerMode() )
	{
		m_pCharInfo->m_QuestItemList.Clear();
		if( !pDBMgr->GetCharQuestItemInfo(m_pCharInfo) )
		{
			mlog( "MAsyncDBJob_GetCharInfo::Run - 디비에서 퀘스트 아이템 목록을 가져오는데 실패했음.\n" );
			SetResult(MASYNC_RESULT_FAILED);
			return;
		}
	}
#endif

	SetResult(MASYNC_RESULT_SUCCEED);
}



void MAsyncDBJob_UpdateCharClanContPoint::Run(void* pContext)
{
	auto* pDBMgr = static_cast<IDatabase*>(pContext);


	if (!pDBMgr->UpdateCharClanContPoint(m_nCID, m_nCLID, m_nAddedContPoint))
	{
		SetResult(MASYNC_RESULT_FAILED);
		return;
	}


	SetResult(MASYNC_RESULT_SUCCEED);
}



void MAsyncDBJob_GetAccountCharInfo::Run(void* pContext)
{
	auto* pDBMgr = static_cast<IDatabase*>(pContext);

	MTD_CharInfo charinfo;

	if (!pDBMgr->GetAccountCharInfo(m_nAID, m_nCharNum, &charinfo))
	{
		MGetMatchServer()->Log(MMatchServer::LOG_ALL, "GetAccountCharInfo failed");
		SetResult(MASYNC_RESULT_FAILED);
		return;
	}

	MMatchServer* pServer = MMatchServer::GetInstance();

	MCommand* pNewCmd = pServer->CreateCommand(MC_MATCH_RESPONSE_ACCOUNT_CHARINFO, MUID(0,0));

	pNewCmd->AddParameter(new MCommandParameterChar((char)m_nCharNum));

	void* pCharArray = MMakeBlobArray(sizeof(MTD_CharInfo), 1);

	MTD_CharInfo* pTransCharInfo = (MTD_CharInfo*)MGetBlobArrayElement(pCharArray, 0);
	memcpy(pTransCharInfo, &charinfo, sizeof(MTD_CharInfo));

	pNewCmd->AddParameter(new MCommandParameterBlob(pCharArray, MGetBlobArraySize(pCharArray)));
	MEraseBlobArray(pCharArray);

	m_pResultCommand = pNewCmd;
	SetResult(MASYNC_RESULT_SUCCEED);
}

void MAsyncDBJob_CreateChar::Run(void* pContext)
{
	auto* pDBMgr = static_cast<IDatabase*>(pContext);

	m_nResult = pDBMgr->CreateCharacter(m_nAID, m_szCharName, m_nCharNum,
		m_nSex, m_nHair, m_nFace, m_nCostume);

	if (m_nResult == MOK)
	{
		// 생성되면 로그로 남긴다.
		pDBMgr->InsertCharMakingLog(m_nAID, m_szCharName, CharMakingType::Create);
	} else if (m_nResult == MERR_UNKNOWN) {
		SetResult(MASYNC_RESULT_FAILED);
	}

	// Make Result
	MMatchServer* pServer = MMatchServer::GetInstance();

	MCommand* pNewCmd = pServer->CreateCommand(MC_MATCH_RESPONSE_CREATE_CHAR, MUID(0,0));
	pNewCmd->AddParameter(new MCommandParameterInt(m_nResult));			// result
	pNewCmd->AddParameter(new MCommandParameterString(m_szCharName));	// 만들어진 캐릭터 이름

	m_pResultCommand = pNewCmd;
	SetResult(MASYNC_RESULT_SUCCEED);
}

void MAsyncDBJob_DeleteChar::Run(void* pContext)
{
	auto* pDBMgr = static_cast<IDatabase*>(pContext);

	int nResult = MERR_UNKNOWN;
	if (pDBMgr->DeleteCharacter(m_nAID, m_nCharNum, m_szCharName))
	{
		pDBMgr->InsertCharMakingLog(m_nAID, m_szCharName, CharMakingType::Delete);
		m_nDeleteResult = MOK;
		SetResult(MASYNC_RESULT_SUCCEED);
	}
	else
	{
		m_nDeleteResult = MERR_CANNOT_DELETE_CHAR;
		SetResult(MASYNC_RESULT_FAILED);
		return;
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////
void MAsyncDBJob_InsertGameLog::Run(void* pContext)
{
	auto* pDBMgr = static_cast<IDatabase*>(pContext);

	pDBMgr->InsertGameLog(m_szGameName, m_szMap,
							m_szGameType,
							m_nRound,
							m_nMasterCID,
							m_nPlayerCount,
							m_szPlayers);

	SetResult(MASYNC_RESULT_SUCCEED);
}

bool MAsyncDBJob_InsertGameLog::Input(const char* szGameName,
										const char* szMap,
										const char* szGameType,
										const int nRound,
										const unsigned int nMasterCID,
										const int nPlayerCount,
										const char* szPlayers)
{
	strcpy_safe(m_szGameName, szGameName);
	strcpy_safe(m_szMap, szMap);
	strcpy_safe(m_szGameType, szGameType);
	m_nRound = nRound;
	m_nMasterCID = nMasterCID;
	m_nPlayerCount = nPlayerCount;
	strcpy_safe(m_szPlayers, szPlayers);

	return true;
}


/////////////////////////////////////////////////////////////////////////////////////////////////
void MAsyncDBJob_CreateClan::Run(void* pContext)
{
	auto* pDBMgr = static_cast<IDatabase*>(pContext);

	if (!pDBMgr->CreateClan(m_szClanName,
							m_nMasterCID,
							m_nMember1CID,
							m_nMember2CID,
							m_nMember3CID,
							m_nMember4CID,
							&m_bDBResult,
							&m_nNewCLID))
	{
		SetResult(MASYNC_RESULT_FAILED);

		return;
	}

	SetResult(MASYNC_RESULT_SUCCEED);
}

bool MAsyncDBJob_CreateClan::Input(const char* szClanName,
									const int nMasterCID,
									const int nMember1CID,
									const int nMember2CID,
									const int nMember3CID,
									const int nMember4CID,
									const MUID& uidMaster,
									const MUID& uidMember1,
									const MUID& uidMember2,
									const MUID& uidMember3,
									const MUID& uidMember4)
{
	strcpy_safe(m_szClanName, szClanName);
	m_nMasterCID = nMasterCID;
	m_nMember1CID = nMember1CID;
	m_nMember2CID = nMember2CID;
	m_nMember3CID = nMember3CID;
	m_nMember4CID = nMember4CID;

	m_uidMaster = uidMaster;
	m_uidMember1 = uidMember1;
	m_uidMember2 = uidMember2;
	m_uidMember3 = uidMember3;
	m_uidMember4 = uidMember4;

	return true;
}

/////////////////////////////////////////////////////////////////////////////////////////////////
void MAsyncDBJob_ExpelClanMember::Run(void* pContext)
{
	auto* pDBMgr = static_cast<IDatabase*>(pContext);

	DBResult = pDBMgr->ExpelClanMember(m_nCLID, m_nClanGrade, m_szTarMember);
	if (DBResult != ExpelResult::OK)
	{
		SetResult(MASYNC_RESULT_FAILED);
		return;
	}

	SetResult(MASYNC_RESULT_SUCCEED);
}

bool MAsyncDBJob_ExpelClanMember::Input(const MUID& uidAdmin, int nCLID, int nClanGrade, const char* szTarMember)
{
	m_uidAdmin = uidAdmin;
	strcpy_safe(m_szTarMember, szTarMember);
	m_nCLID = nCLID;
	m_nClanGrade = nClanGrade;

	return true;
}


/////////////////////////////////////////////////////////////////////////////////////////////////

MAsyncDBJob_InsertQuestGameLog::~MAsyncDBJob_InsertQuestGameLog()
{
	vector< MQuestPlayerLogInfo* >::iterator it, end;
	it = m_Player.begin();
	end = m_Player.end();
	for(; it != end; ++it )
		delete (*it);
}


bool MAsyncDBJob_InsertQuestGameLog::Input( const char* pszStageName,
										    const int nScenarioID,
										    const int nMasterCID,
											MMatchQuestGameLogInfoManager* pQGameLogInfoMgr,
											const int nTotalRewardQItemCount,
											const int nElapsedPlayerTime )
{
	if( 0 == pQGameLogInfoMgr )
		return false;

	strcpy_safe( m_szStageName, pszStageName );

	m_nScenarioID				= nScenarioID;
	m_nMasterCID				= nMasterCID;
    m_nElapsedPlayTime			= nElapsedPlayerTime;
	m_nTotalRewardQItemCount	= nTotalRewardQItemCount;

	int										i;
	MQuestPlayerLogInfo*					pPlayer;
	MMatchQuestGameLogInfoManager::iterator itPlayer, endPlayer;

	m_Player.clear();
	memset( m_PlayersCID, 0, 12 );

	i = 0;
	itPlayer  = pQGameLogInfoMgr->begin();
	endPlayer = pQGameLogInfoMgr->end();
	for(; itPlayer != endPlayer; ++itPlayer )
	{
		if( nMasterCID != itPlayer->second->GetCID() )
			m_PlayersCID[ i++ ] = itPlayer->second->GetCID();

		if( itPlayer->second->GetUniqueItemList().empty() )
			continue;	// 비어있을경우 그냥 무시.

		if( 0 == (pPlayer = new MQuestPlayerLogInfo) )
		{
			mlog( "MAsyncDBJob_InsertQuestGameLog::Input - 메모리 할당 실패.\n" );
			continue;
		}

		pPlayer->SetCID( itPlayer->second->GetCID() );
		pPlayer->GetUniqueItemList().insert( itPlayer->second->GetUniqueItemList().begin(),
											 itPlayer->second->GetUniqueItemList().end() );

		m_Player.push_back( pPlayer );
	}

	return true;
}


void MAsyncDBJob_InsertQuestGameLog::Run( void* pContext )
{
	if( MSM_TEST == MGetServerConfig()->GetServerMode() )
	{
		IDatabase* pDBMgr = static_cast< IDatabase* >( pContext );

		int nQGLID;

		// 우선 퀘스트 게임 로그를 저장함.
		if( !pDBMgr->InsertQuestGameLog(m_szStageName,
			m_nScenarioID,
			m_nMasterCID, m_PlayersCID[0], m_PlayersCID[1], m_PlayersCID[2],
			m_nTotalRewardQItemCount,
			m_nElapsedPlayTime,
			nQGLID) )
		{
			SetResult(MASYNC_RESULT_FAILED);
			return;
		}

		// 유니크 아이템에 관한 데이터는 QUniqueItemLog에 따로 저장을 해줘야 함.
		int											i;
		int											nCID;
		int											nQIID;
		int											nQUItemCount;
		QItemLogMapIter								itQUItem, endQUItem;
		vector< MQuestPlayerLogInfo* >::iterator	itPlayer, endPlayer;

		itPlayer  = m_Player.begin();
		endPlayer = m_Player.end();

		for( ; itPlayer != endPlayer; ++itPlayer )
		{
			if( (*itPlayer)->GetUniqueItemList().empty() )
				continue;	// 유니크 아이템을 가지고 있지 않으면 무시.

			nCID		= (*itPlayer)->GetCID();
			itQUItem	= (*itPlayer)->GetUniqueItemList().begin();
			endQUItem	= (*itPlayer)->GetUniqueItemList().end();

			for( ; itQUItem != endQUItem; ++itQUItem )
			{
				nQIID			= itQUItem->first;
				nQUItemCount	= itQUItem->second;

				for( i = 0; i < nQUItemCount; ++i )
				{
					if( !pDBMgr->InsertQUniqueGameLog(nQGLID, nCID, nQIID) )
					{
						mlog( "MAsyncDBJob_InsertQuestGameLog::Run - InsertQUniqueGameLog failed. CID = %d, QIID = %d\n",
							nCID, nQIID );

						SetResult(MASYNC_RESULT_FAILED);
					}
				}
			}
		}
	}

	SetResult(MASYNC_RESULT_SUCCEED);
}

///////////////////////////////////////////////////////////////////////////////////////////////////

MAsyncDBJob_UpdateQuestItemInfo::~MAsyncDBJob_UpdateQuestItemInfo()
{
	m_QuestItemList.Clear();
}


bool MAsyncDBJob_UpdateQuestItemInfo::Input( const int nCID, MQuestItemMap& QuestItemList, MQuestMonsterBible& QuestMonster )
{
	m_nCID = nCID;

	MQuestItemMap::iterator it, end;

	m_QuestItemList.Clear();

	it  = QuestItemList.begin();
	end = QuestItemList.end();

	for( ; it != end; ++it )
	{
		MQuestItem* pQuestItem = it->second;
		m_QuestItemList.CreateQuestItem( pQuestItem->GetItemID(), pQuestItem->GetCount(), pQuestItem->IsKnown() );
	}

	m_QuestItemList.SetDBAccess( QuestItemList.IsDoneDbAccess() );

	memcpy( &m_QuestMonster, &QuestMonster, sizeof(MQuestMonsterBible) );
	return true;
}


void MAsyncDBJob_UpdateQuestItemInfo::Run( void* pContext )
{
	if( MSM_TEST == MGetServerConfig()->GetServerMode() )
	{
		if( m_QuestItemList.IsDoneDbAccess() )
		{
			auto* pDBMgr = static_cast<IDatabase*>( pContext );
			if( !pDBMgr->UpdateQuestItem(m_nCID, m_QuestItemList, m_QuestMonster) )
			{
				SetResult( MASYNC_RESULT_FAILED );
				return;
			}
		}
	}

	SetResult( MASYNC_RESULT_SUCCEED );
}


bool MAsyncDBJob_SetBlockAccount::Input( const u32 dwAID,
										 const u32 dwCID,
										 const u8 btBlockType,
										 const u8 btBlockLevel,
										 const string& strComment,
										 const string& strIP,
										 const string& strEndDate )
{
	m_dwAID			= dwAID;
	m_dwCID			= dwCID;
	m_btBlockType 	= btBlockType;
	m_btBlockLevel	= btBlockLevel;
	m_strComment	= strComment;
	m_strIP			= strIP;
	m_strEndDate	= strEndDate;

	return true;
}

void MAsyncDBJob_SetBlockAccount::Run( void* pContext )
{
	auto* pDBMgr = static_cast<IDatabase*>( pContext );

	if( MMBL_ACCOUNT == m_btBlockLevel )
	{
		if( !pDBMgr->SetBlockAccount( m_dwAID, m_dwCID, m_btBlockType, m_strComment, m_strIP, m_strEndDate) )
		{
			SetResult( MASYNC_RESULT_FAILED );
			return;
		}
	}
	else if( MMBL_LOGONLY == m_btBlockLevel )
	{
		if( pDBMgr->InsertBlockLog(m_dwAID, m_dwCID, m_btBlockType, m_strComment, m_strIP) )
		{
			SetResult( MASYNC_RESULT_FAILED );
			return;
		}
	}

	SetResult( MASYNC_RESULT_SUCCEED );
}


bool MAsyncDBJob_ResetAccountBlock::Input( const u32 dwAID, const u8 btBlockType )
{
	m_dwAID			= dwAID;
	m_btBlockType	= btBlockType;

	return true;
}


void MAsyncDBJob_ResetAccountBlock::Run( void* pContext )
{
	auto* pDBMgr = static_cast<IDatabase*>( pContext );

	if( !pDBMgr->ResetAccountBlock(m_dwAID, m_btBlockType) )
	{
		SetResult( MASYNC_RESULT_FAILED );
		return;
	}

	SetResult( MASYNC_RESULT_SUCCEED );
}
