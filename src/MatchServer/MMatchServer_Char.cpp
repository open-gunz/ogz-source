#include "stdafx.h"
#include "MMatchServer.h"
#include "MSharedCommandTable.h"
#include "MErrorTable.h"
#include "MBlobArray.h"
#include "MObject.h"
#include "MMatchObject.h"
#include "MMatchItem.h"
#include "MAgentObject.h"
#include "MMatchNotify.h"
#include "MMatchObjCache.h"
#include "MMatchStage.h"
#include "MMatchTransDataType.h"
#include "MMatchFormula.h"
#include "MMatchConfig.h"
#include "MCommandCommunicator.h"
#include "MMatchShop.h"
#include "MMatchTransDataType.h"
#include "MDebug.h"
#include "MMatchAuth.h"
#include "MMatchStatus.h"
#include "MAsyncDBJob.h"
#include "MAsyncDBJob_FriendList.h"
#include "MAsyncDBJob_CharFinalize.h"
#include "MMatchUtil.h"
#include "MMatchRuleBaseQuest.h"
#include "MMatchLocale.h"


void MMatchServer::OnRequestAccountCharList(const MUID& uidPlayer)
{
	MMatchObject* pObj = GetObject(uidPlayer);
	if (pObj == NULL) return;

	const MMatchBlockType MBlockType = pObj->GetAccountInfo()->m_BlockType;
 	if( (MMBT_NO != MBlockType) && !IsAdminGrade(pObj) )
	{
		// block!!

		bool bIsEndTimeExpire = false;

		if( !IsExpiredBlockEndTime(pObj->GetAccountInfo()->m_EndBlockDate) )
		{
			if( !bIsEndTimeExpire )
			{
				u32 dwMsgID = 0;

				pObj->GetDisconnStatusInfo().SetMsgID( dwMsgID );
				pObj->GetDisconnStatusInfo().SetStatus( MMDS_DISCONN_WAIT );
				return;
			}
		}
		else
		{
			pObj->GetAccountInfo()->m_BlockType = MMBT_NO;

			MAsyncDBJob_ResetAccountBlock* pResetBlockJob = new MAsyncDBJob_ResetAccountBlock;
			pResetBlockJob->Input( pObj->GetAccountInfo()->m_nAID, MMBT_NO );

			PostAsyncJob( pResetBlockJob );
		}
	}

	// Async DB //////////////////////////////

	pObj->UpdateTickLastPacketRecved();

	if( 0 != pObj->GetCharInfo() )
	{
		MAsyncDBJob_UpdateQuestItemInfo* pQItemUpdateJob = new MAsyncDBJob_UpdateQuestItemInfo;
		if( 0 == pQItemUpdateJob )
			return;
		if( !pQItemUpdateJob->Input(pObj->GetCharInfo()->m_nCID, 
									pObj->GetCharInfo()->m_QuestItemList, 
									pObj->GetCharInfo()->m_QMonsterBible) )
		{
			mlog( "MMatchServer::OnAsyncGetAccountCharList - 객체 생성 실패.\n" );
			delete pQItemUpdateJob;
			return;
		}

		pObj->GetCharInfo()->m_QuestItemList.SetDBAccess( false );

		PostAsyncJob( pQItemUpdateJob );
	}

	MAsyncDBJob_GetAccountCharList* pJob=new MAsyncDBJob_GetAccountCharList(uidPlayer,pObj->GetAccountInfo()->m_nAID);
	PostAsyncJob(pJob);
}

void MMatchServer::OnRequestAccountCharInfo(const MUID& uidPlayer, int nCharNum)
{
	// Async DB //////////////////////////////
	MMatchObject* pObj = GetObject(uidPlayer);
	if (pObj == NULL) return;

	MAsyncDBJob_GetAccountCharInfo* pJob=new MAsyncDBJob_GetAccountCharInfo(uidPlayer,pObj->GetAccountInfo()->m_nAID, nCharNum);
	PostAsyncJob(pJob);
}


void MMatchServer::OnRequestSelectChar(const MUID& uidPlayer, const int nCharIndex)
{
	MMatchObject* pObj = GetObject(uidPlayer);
	if ((pObj == NULL) || (pObj->GetAccountInfo()->m_nAID < 0)) return;
	if ((nCharIndex < 0) || (nCharIndex >= MAX_CHAR_COUNT)) return;

	// Async DB //////////////////////////////
	MAsyncDBJob_GetCharInfo* pJob=new MAsyncDBJob_GetCharInfo(uidPlayer, pObj->GetAccountInfo()->m_nAID, nCharIndex);
	pJob->SetCharInfo(new MMatchCharInfo);
	PostAsyncJob(pJob);
}


void MMatchServer::OnRequestDeleteChar(const MUID& uidPlayer, const int nCharIndex, const char* szCharName)
{
	ResponseDeleteChar(uidPlayer, nCharIndex, szCharName);
}

bool MMatchServer::ResponseDeleteChar(const MUID& uidPlayer, const int nCharIndex, const char* szCharName)
{
	MMatchObject* pObj = GetObject(uidPlayer);
	if ((pObj == NULL) || (pObj->GetAccountInfo()->m_nAID < 0)) return false;
	if ((nCharIndex < 0) || (nCharIndex >= MAX_CHAR_COUNT)) return false;

    // 생성조건 통과 - Post AsyncJob
	MAsyncDBJob_DeleteChar* pJob = new MAsyncDBJob_DeleteChar(uidPlayer, pObj->GetAccountInfo()->m_nAID, nCharIndex, szCharName);
	PostAsyncJob(pJob);

	return true;
}


void MMatchServer::OnRequestCreateChar(const MUID& uidPlayer, const int nCharIndex, const char* szCharName,
						 const unsigned int nSex, const unsigned int nHair, const unsigned int nFace, 
						 const unsigned int nCostume)
{
	MMatchSex sex = (nSex == 0) ? MMS_MALE : MMS_FEMALE;
	ResponseCreateChar(uidPlayer, nCharIndex, szCharName, MMatchSex(nSex), nHair, nFace, nCostume);
}


bool MMatchServer::ResponseCreateChar(const MUID& uidPlayer, const int nCharIndex, const char* szCharName,
						MMatchSex nSex, const unsigned int nHair, const unsigned int nFace, 
						const unsigned int nCostume)
{
	MMatchObject* pObj = GetObject(uidPlayer);
	if (pObj == NULL) return false;

	if ((pObj->GetAccountInfo()->m_nAID < 0) ||
	   (nCharIndex < 0) || (nCharIndex >= MAX_CHAR_COUNT) || (nHair >= MAX_COSTUME_HAIR) ||
	   (nFace >= MAX_COSTUME_FACE) || (nCostume >= MAX_COSTUME_TEMPLATE)) 
	{
		int nResult = -1;	// false
		MCommand* pNewCmd = CreateCommand(MC_MATCH_RESPONSE_CREATE_CHAR, MUID(0,0));
		pNewCmd->AddParameter(new MCommandParameterInt(nResult));			// result
		pNewCmd->AddParameter(new MCommandParameterString(szCharName));		// 만들어진 캐릭터 이름
		RouteToListener(pObj, pNewCmd);
		return false;
	}

	int nResult = -1;	// false

	nResult = ValidateMakingName(szCharName, MIN_CHARNAME, MAX_CHARNAME);
	if (nResult != MOK) {
		MCommand* pNewCmd = CreateCommand(MC_MATCH_RESPONSE_CREATE_CHAR, MUID(0,0));
		pNewCmd->AddParameter(new MCommandParameterInt(nResult));			// result
		pNewCmd->AddParameter(new MCommandParameterString(szCharName));		// 만들어진 캐릭터 이름
		RouteToListener(pObj, pNewCmd);

		return false;
	}	

	// 생성조건 통과 - Post AsyncJob
	MAsyncDBJob_CreateChar* pJob = new MAsyncDBJob_CreateChar(uidPlayer, pObj->GetAccountInfo()->m_nAID, 
											szCharName, nCharIndex, nSex, nHair, nFace, nCostume);
	

#ifdef _DEBUG
	// 제가 주석처리했습니다 - bird
	mlog( "Selected character name : %s (", szCharName);

	for ( int i = 0;  i < (int)strlen( szCharName);  i++)
	{
		mlog( "%02X ", szCharName[ i] & 0x00FF);
	}

	mlog( ")  (len = %d)\n", (int)strlen( szCharName));
#endif

	PostAsyncJob(pJob);

	return true;
}


void MMatchServer::OnCharClear(const MUID& uidPlayer)
{
	MMatchObject* pObj = GetObject(uidPlayer);
	if (pObj)
	{
		CharFinalize(pObj->GetUID());
		ObjectRemove(pObj->GetUID(), NULL);
	}
//	LOG("Communication UID (%u:%u) Cleared", uidComm.High, uidComm.Low);
}

bool MMatchServer::CharInitialize(const MUID& uidPlayer)
{
	MMatchObject* pObj = GetObject(uidPlayer);
	if (pObj == NULL) return false;

	MMatchCharInfo*	pCharInfo = pObj->GetCharInfo();
	if (pCharInfo == NULL) return false;

	pCharInfo->m_nConnTime = GetTickTime();
	pCharInfo->EquipFromItemList();


	// check items weight
	int nWeight, nMaxWeight;
	pCharInfo->GetTotalWeight(&nWeight, &nMaxWeight);
	if (nWeight > nMaxWeight)
	{
		if (!GetDBMgr()->ClearAllEquipedItem(pCharInfo->m_nCID))
		{
			mlog("DB Query(ClearAllEquipedItem) Failed\n");
		}
		pCharInfo->m_EquipedItem.Clear();
	}

	CheckExpiredItems(pObj);


	// if player belonges to a clan
	if (pCharInfo->m_ClanInfo.IsJoined())
	{
		// player connected message
		MCommand* pNew = CreateCommand(MC_MATCH_CLAN_MEMBER_CONNECTED, MUID(0,0));
		pNew->AddParameter(new MCommandParameterString((char*)pCharInfo->m_szName));
		RouteToClan(pCharInfo->m_ClanInfo.m_nClanID, pNew);

		m_ClanMap.AddObject(uidPlayer, pObj);

		// 만약 클랜전 서버일 경우 클랜전 정보도 받아온다. - 엠블렘때문에 그냥 서버도 클랜정보 읽어옴
		//if ( (MGetServerConfig()->GetServerMode() == MSM_CLAN) || (MGetServerConfig()->GetServerMode() == MSM_TEST) )
		//{
			MMatchClan* pClan = m_ClanMap.GetClan(pCharInfo->m_ClanInfo.m_nClanID);

			if (pClan)
			{
				if (!pClan->IsInitedClanInfoEx())
				{
					pClan->InitClanInfoFromDB();
				}
			}
		//}
	}

	return true;
}

void MMatchServer::CheckExpiredItems(MMatchObject* pObj)
{
	MMatchCharInfo*	pCharInfo = pObj->GetCharInfo();
	if (!pCharInfo->m_ItemList.HasRentItem()) return;

	vector<u32> vecExpiredItemIDList;
	vector<MUID> vecExpiredItemUIDList;				// 만료된 아이템 UID

	// 기간 만료 아이템이 있는지 체크하고 있으면 아이템 해제하고 통지한다.
	for (MMatchItemMap::iterator itor = pCharInfo->m_ItemList.begin(); itor != pCharInfo->m_ItemList.end(); ++itor)
	{
		MMatchItem* pCheckItem = (*itor).second;
		if (pCheckItem->IsRentItem())
		{
			// 인스턴스 생성되고나서 지난 시간
			auto nPassTime = MGetTimeDistance(pCheckItem->GetRentItemRegTime(), GetTickTime());
			int nPassMinuteTime = static_cast<int>(nPassTime / (1000 * 60));

			if ((pCheckItem->GetRentMinutePeriodRemainder()-nPassMinuteTime) <= 0)
			{
				// 장비중이면 벗긴다.
				MMatchCharItemParts nCheckParts = MMCIP_END;
				if (pCharInfo->m_EquipedItem.IsEquipedItem(pCheckItem, nCheckParts))
				{
					ResponseTakeoffItem(pObj->GetUID(), nCheckParts);
				}

				// 아이템 제거
				int nExpiredItemID = pCheckItem->GetDescID();
				if (nExpiredItemID != 0)
				{
					vecExpiredItemUIDList.push_back(pCheckItem->GetUID());
					vecExpiredItemIDList.push_back(nExpiredItemID);
				}
			}
		}
	}

	// 만료된 아이템이 있으면 통지
	if (!vecExpiredItemIDList.empty())
	{
		int nExpiredItemUIDListCount = (int)vecExpiredItemUIDList.size();
		for (int i = 0; i < nExpiredItemUIDListCount; i++)
		{
			// 실제로 여기서 아이템을 지운다.
			RemoveCharItem(pObj, vecExpiredItemUIDList[i]);
		}

		ResponseExpiredItemIDList(pObj, vecExpiredItemIDList);
	}
}

void MMatchServer::ResponseExpiredItemIDList(MMatchObject* pObj, vector<u32>& vecExpiredItemIDList)
{
	int nBlobSize = (int)vecExpiredItemIDList.size();
	MCommand* pNewCmd = CreateCommand(MC_MATCH_EXPIRED_RENT_ITEM, MUID(0,0));
	
	void* pExpiredItemIDArray = MMakeBlobArray(sizeof(u32), nBlobSize);

	for (int i = 0; i < nBlobSize; i++)
	{
		u32 *pItemID = (u32*)MGetBlobArrayElement(pExpiredItemIDArray, i);
		*pItemID = vecExpiredItemIDList[i];
	}
	pNewCmd->AddParameter(new MCommandParameterBlob(pExpiredItemIDArray, MGetBlobArraySize(pExpiredItemIDArray)));
	MEraseBlobArray(pExpiredItemIDArray);
	RouteToListener(pObj, pNewCmd);
}

// 수정되면 true
bool MMatchServer::CorrectEquipmentByLevel(MMatchObject* pPlayer, MMatchCharItemParts nPart, int nLegalItemLevelDiff)	
{
	if (!IsEnabledObject(pPlayer)) return false;

	if (pPlayer->GetCharInfo()->m_EquipedItem.IsEmpty(nPart) == false) {
		MMatchItem* pItem = pPlayer->GetCharInfo()->m_EquipedItem.GetItem(nPart);
		if (pItem->GetDesc())
		{
			if (pItem->GetDesc()->m_nResLevel > (pPlayer->GetCharInfo()->m_nLevel+nLegalItemLevelDiff)) {
				ResponseTakeoffItem(pPlayer->GetUID(), nPart);
				return true;
			}
		}
	}
	return false;
}

bool MMatchServer::CharFinalize(const MUID& uidPlayer)
{
	MMatchObject* pObj = GetObject(uidPlayer);
	if (pObj == NULL) return false;

	if (MGetServerConfig()->GetServerMode() == MSM_EVENT) {	// EVENT때 레벨제한 없이 장착한 아이템을 벗긴다
		CorrectEquipmentByLevel(pObj, MMCIP_MELEE);
		CorrectEquipmentByLevel(pObj, MMCIP_PRIMARY);
		CorrectEquipmentByLevel(pObj, MMCIP_SECONDARY);
		CorrectEquipmentByLevel(pObj, MMCIP_CUSTOM1);
		CorrectEquipmentByLevel(pObj, MMCIP_CUSTOM2);
	}

	// xp, bp, KillCount, DeathCount 캐슁 업데이트
	UpdateCharDBCachingData(pObj);

	MMatchCharInfo*	pCharInfo = pObj->GetCharInfo();
	if (pCharInfo == NULL) return false;
	

	// 클랜에 가입되어 있으면 MMatchMap에서 삭제
	if (pCharInfo->m_ClanInfo.IsJoined())
	{
		m_ClanMap.RemoveObject(uidPlayer, pObj);
	}

	// 캐릭끝날때 로그 남긴다
	if (pCharInfo->m_nCID != 0)
	{
		u32 nPlayTime = 0;
		auto nNowTime = GetTickTime();

		if (pCharInfo->m_nConnTime != 0)
		{
			nPlayTime = MGetTimeDistance(pCharInfo->m_nConnTime, nNowTime) / 1000;
		}

		MAsyncDBJob_CharFinalize* pJob = new MAsyncDBJob_CharFinalize();
		pJob->Input(pCharInfo->m_nCID, 
					nPlayTime, 
					pCharInfo->m_nConnKillCount, 
					pCharInfo->m_nConnDeathCount,
					pCharInfo->m_nConnXP, 
					pCharInfo->m_nXP,
					pCharInfo->m_QuestItemList,
					pCharInfo->m_QMonsterBible,
					pCharInfo->m_DBQuestCachingData.IsRequestUpdateWhenLogout() );
		PostAsyncJob(pJob);

		pCharInfo->m_DBQuestCachingData.Reset();
/*
#ifdef _DEBUG
		{
			// 업데이트 정보가 정상적으로 되는지 로그를 남김.
			char szDbgOut[ 1000 ] = {0};
			MQuestItemMap::iterator it, end;

			strcat( szDbgOut, "CharFinalize\n" );
			strcat( szDbgOut, pObj->GetName() );
			strcat( szDbgOut, "\n" );
			strcat( szDbgOut, "BP:");

			char szBP[ 20 ] = {0};
			sprintf_safe( szBP, "BP:%d\n", pCharInfo->m_nBP );

			strcat( szDbgOut, szBP );
			
			it = pObj->GetCharInfo()->m_QuestItemList.begin();
			end = pObj->GetCharInfo()->m_QuestItemList.end();

			for( ; it != end; ++it )
			{
				char tmp[ 100 ] = {0};
				sprintf_safe( tmp, "QuestItemName:%s : Count:%d\n", it->second->GetDesc()->m_szQuestItemName, it->second->GetCount() );
				strcat( szDbgOut, tmp );
			}
			strcat( szDbgOut, "\n" );

			mlog( szDbgOut );
			// MMatchServer::GetInstance()->LOG( MMatchServer::LOG_ALL, szDbgOut );
		}
#endif
*/
/*
		if (!GetDBMgr()->UpdateCharPlayTime(pCharInfo->m_nCID, nPlayTime))
		{
			mlog("DB Query(CharFinalize > UpdateCharPlayTime) Failed\n");
		}
		if (!GetDBMgr()->InsertPlayerLog(pCharInfo->m_nCID,
			nPlayTime, pCharInfo->m_nConnKillCount, pCharInfo->m_nConnDeathCount,
			pCharInfo->m_nConnXP, pCharInfo->m_nXP ))
		{
			mlog("DB Query(CharFinalize > InsertPlayerLog) Failed\n");
		}
*/
	}

	// pObj에서 동적으로 생성한 CharInfo, FriendInfo를 해제시킨다.



	return true;
}

void MMatchServer::OnRequestMySimpleCharInfo(const MUID& uidPlayer)
{
	ResponseMySimpleCharInfo(uidPlayer);
}

void MMatchServer::ResponseMySimpleCharInfo(const MUID& uidPlayer)
{
	MMatchObject* pObj = GetObject(uidPlayer);
	if (! IsEnabledObject(pObj)) return;

	MMatchCharInfo* pCharInfo = pObj->GetCharInfo();

	MCommand* pNewCmd = CreateCommand(MC_MATCH_RESPONSE_MY_SIMPLE_CHARINFO, MUID(0,0));

	void* pMyCharInfoArray = MMakeBlobArray(sizeof(MTD_MySimpleCharInfo), 1);
	MTD_MySimpleCharInfo* pMyCharInfo = (MTD_MySimpleCharInfo*)MGetBlobArrayElement(pMyCharInfoArray, 0);

	pMyCharInfo->nXP = pCharInfo->m_nXP;
	pMyCharInfo->nBP = pCharInfo->m_nBP;
	pMyCharInfo->nLevel = pCharInfo->m_nLevel;

	pNewCmd->AddParameter(new MCommandParameterBlob(pMyCharInfoArray, MGetBlobArraySize(pMyCharInfoArray)));
	MEraseBlobArray(pMyCharInfoArray);

	RouteToListener(pObj, pNewCmd);
}

void MMatchServer::OnRequestCopyToTestServer(const MUID& uidPlayer)
{
#ifndef _DEBUG
	return;
#endif

	MMatchObject* pObj = GetObject(uidPlayer);
	if (pObj == NULL) return;
	MMatchCharInfo*	pCharInfo = pObj->GetCharInfo();
	if (pCharInfo == NULL) return;


	int nResult = MERR_UNKNOWN;

	ResponseCopyToTestServer(uidPlayer, nResult);
}

void MMatchServer::ResponseCopyToTestServer(const MUID& uidPlayer, const int nResult)
{
	MMatchObject* pObj = GetObject(uidPlayer);
	if (pObj == NULL) return;

	MCommand* pNewCmd = CreateCommand(MC_MATCH_RESPONSE_COPY_TO_TESTSERVER, MUID(0,0));
	pNewCmd->AddParameter(new MCommandParameterInt(nResult));		// result
	RouteToListener(pObj, pNewCmd);
}

void MMatchServer::OnFriendAdd(const MUID& uidPlayer, const char* pszName)
{
	MMatchObject* pObj = GetObject(uidPlayer);
	if (! IsEnabledObject(pObj)) return;

	if (pObj->GetFriendInfo() == NULL) return;

	MMatchObject* pTargetObj = GetPlayerByName(pszName);
	if (pTargetObj == NULL) {
		NotifyMessage(pObj->GetUID(), MATCHNOTIFY_GENERAL_USER_NOTFOUND);
		return;
	}

	if ((IsAdminGrade(pObj) == false) && (IsAdminGrade(pTargetObj) == true)) {
		NotifyMessage(pObj->GetUID(), MATCHNOTIFY_GENERAL_USER_NOTFOUND);
		return;
	}

	MMatchFriendNode* pNode = pObj->GetFriendInfo()->Find(pszName);
	if (pNode) {
		NotifyMessage(uidPlayer, MATCHNOTIFY_FRIEND_ALREADY_EXIST);
		return;
	}
	if (pObj->GetFriendInfo()->m_FriendList.size() > MAX_FRIEND_COUNT) {
		NotifyMessage(uidPlayer, MATCHNOTIFY_FRIEND_TOO_MANY_ADDED);
		return;
	}

	int nCID = pObj->GetCharInfo()->m_nCID;
	int nFriendCID = 0;
	if (GetDBMgr()->GetCharCID(pszName, &nFriendCID) == false) {
		NotifyMessage(uidPlayer, MATCHNOTIFY_CHARACTER_NOT_EXIST);
		return;
	}

	if (GetDBMgr()->FriendAdd(nCID, nFriendCID, 0) == false) {
		mlog("DB Query(FriendAdd) Failed\n");
		return;
	}

	pObj->GetFriendInfo()->Add(nFriendCID, 0, pszName);
	NotifyMessage(uidPlayer, MATCHNOTIFY_FRIEND_ADD_SUCCEED);
}

void MMatchServer::OnFriendRemove(const MUID& uidPlayer, const char* pszName)
{
	MMatchObject* pObj = GetObject(uidPlayer);
	if (! IsEnabledObject(pObj)) return;
	if (pObj->GetFriendInfo() == NULL) return;

	MMatchFriendNode* pNode = pObj->GetFriendInfo()->Find(pszName);
	if (pNode == NULL) {
		NotifyMessage(uidPlayer, MATCHNOTIFY_FRIEND_NOT_EXIST);
		return;
	}

	int nCID = pObj->GetCharInfo()->m_nCID;
	int nFriendCID = 0;
	if (GetDBMgr()->GetCharCID(pszName, &nFriendCID) == false) {
		NotifyMessage(uidPlayer, MATCHNOTIFY_CHARACTER_NOT_EXIST);
		return;
	}

	if (GetDBMgr()->FriendRemove(nCID, nFriendCID) == false) {
		mlog("DB Query(FriendRemove) Failed\n");
		return;
	}

	pObj->GetFriendInfo()->Remove(pszName);
	NotifyMessage(uidPlayer, MATCHNOTIFY_FRIEND_REMOVE_SUCCEED);
}

void MMatchServer::OnFriendList(const MUID& uidPlayer)
{
	MMatchObject* pObj = GetObject(uidPlayer);
	if (! IsEnabledObject(pObj)) return;

	// ASync DB
	if (!pObj->DBFriendListRequested())
	{
		MAsyncDBJob_FriendList* pJob=new MAsyncDBJob_FriendList(uidPlayer, pObj->GetCharInfo()->m_nCID);
		pJob->SetFriendInfo(new MMatchFriendInfo);
		PostAsyncJob(pJob);
	}
	else if (!pObj->GetFriendInfo())
	{
		// 아직 DB에서 FriendList를 안받아왔으면 그냥 리턴
		return;
	}

	FriendList(uidPlayer);
}

void MMatchServer::FriendList(const MUID& uidPlayer)
{
	MMatchObject* pObj = GetObject(uidPlayer);
	if (! IsEnabledObject(pObj)) return;
	if (pObj->GetFriendInfo() == NULL) return;

	// Update Friends Status and Description
	pObj->GetFriendInfo()->UpdateDesc();

	MMatchFriendList* pList = &pObj->GetFriendInfo()->m_FriendList;

	void* pListArray = MMakeBlobArray(sizeof(MFRIENDLISTNODE), (int)pList->size());
	int nIndex=0;
	for (MMatchFriendList::iterator i=pList->begin(); i!=pList->end(); i++) {
		MMatchFriendNode* pNode = (*i);
		MFRIENDLISTNODE* pListNode = (MFRIENDLISTNODE*)MGetBlobArrayElement(pListArray, nIndex++);
		pListNode->nState = pNode->nState;
		strcpy_safe(pListNode->szName, pNode->szName);
		strcpy_safe(pListNode->szDescription, pNode->szDescription);
	}

	MCommand* pCmd = CreateCommand(MC_MATCH_RESPONSE_FRIENDLIST, MUID(0,0));
	pCmd->AddParameter(new MCommandParameterBlob(pListArray, MGetBlobArraySize(pListArray)));
	MEraseBlobArray(pListArray);
	RouteToListener(pObj, pCmd);
}

void MMatchServer::OnFriendMsg(const MUID& uidPlayer, const char* szMsg)
{
	MMatchObject* pObj = GetObject(uidPlayer);
	if (! IsEnabledObject(pObj)) return;

}

void MMatchServer::OnRequestCharInfoDetail(const MUID& uidChar, const char* szCharName)
{
	ResponseCharInfoDetail(uidChar, szCharName);
}

void MMatchServer::ResponseCharInfoDetail(const MUID& uidChar, const char* szCharName)
{
	MMatchObject* pObject = GetObject(uidChar);
	if (! IsEnabledObject(pObject)) return;

	MMatchObject* pTarObject = GetPlayerByName(szCharName);
	if (! IsEnabledObject(pTarObject))
	{
		RouteResponseToListener(pObject, MC_MATCH_RESPONSE_RESULT, MERR_NO_TARGET);
		return;
	}


	// Client에 선택한 캐릭터 정보 전송
	MTD_CharInfo_Detail trans_charinfo_detail;
	CopyCharInfoDetailForTrans(&trans_charinfo_detail, pTarObject->GetCharInfo(), pTarObject);
	

	MCommand* pNewCmd = CreateCommand(MC_MATCH_RESPONSE_CHARINFO_DETAIL, MUID(0,0));

	void* pCharInfoArray = MMakeBlobArray(sizeof(MTD_CharInfo_Detail), 1);
	MTD_CharInfo_Detail* pTransCharInfoDetail = (MTD_CharInfo_Detail*)MGetBlobArrayElement(pCharInfoArray, 0);
	memcpy(pTransCharInfoDetail, &trans_charinfo_detail, sizeof(MTD_CharInfo_Detail));
	pNewCmd->AddParameter(new MCommandParameterBlob(pCharInfoArray, MGetBlobArraySize(pCharInfoArray)));
	MEraseBlobArray(pCharInfoArray);

	RouteToListener(pObject, pNewCmd);
}

