#include "stdafx.h"
#include "MMatchServer.h"
#include "MAsyncDBJob.h"
#include "MAsyncDBJob_FriendList.h"
#include "MAsyncDBJob_BringAccountItem.h"
#include "MAsyncDBJob_GetLoginInfo.h"
#include "MAsyncDBJob_InsertConnLog.h"
#include "MBlobArray.h"
#include "MMatchFormula.h"
#include "MAsyncDBJob_Event.h"

void MMatchServer::PostAsyncJob(MAsyncJob* pJob)
{
	m_AsyncProxy.PostJob(pJob);
}

void MMatchServer::ProcessAsyncJob()
{
	while(MAsyncJob* pJob = m_AsyncProxy.GetJobResult()) 
	{
		switch(pJob->GetJobID()) {
		case MASYNCJOB_GETACCOUNTCHARLIST:
			{
				OnAsyncGetAccountCharList(pJob);
			}
			break;				
		case MASYNCJOB_GETACCOUNTCHARINFO:
			{
				OnAsyncGetAccountCharInfo(pJob);
			}
			break;				
		case MASYNCJOB_GETCHARINFO:
			{
				OnAsyncGetCharInfo(pJob);
			}
			break;
		case MASYNCJOB_FRIENDLIST:
			{
				OnAsyncGetFriendList(pJob);
			}
			break;			
		case MASYNCJOB_CREATECHAR:
			{
				OnAsyncCreateChar(pJob);
			}
			break;
		case MASYNCJOB_GETLOGININFO:
			{
				OnAsyncGetLoginInfo(pJob);
			}
			break;
		case MASYNCJOB_DELETECHAR:
			{
				OnAsyncDeleteChar(pJob);
			}
			break;
		case MASYNCJOB_WINTHECLANGAME:
			{
				OnAsyncWinTheClanGame(pJob);
			}
			break;
		case MASYNCJOB_UPDATECHARINFODATA:
			{
				OnAsyncUpdateCharInfoData(pJob);
			}
			break;
		case MASYNCJOB_CHARFINALIZE:
			{
				OnAsyncCharFinalize(pJob);
			}
			break;
		case MASYNCJOB_BRINGACCOUNTITEM:
			{
				OnAsyncBringAccountItem(pJob);
			}
			break;
		case MASYNCJOB_INSERTCONNLOG:
			{
				OnAsyncInsertConnLog(pJob);
			}
			break;
		case MASYNCJOB_INSERTGAMELOG:
			{
				OnAsyncInsertGameLog(pJob);
			}
			break;
		case MASYNCJOB_CREATECLAN:
			{
				OnAsyncCreateClan(pJob);
			}
			break;
		case MASYNCJOB_EXPELCLANMEMBER:
			{
				OnAsyncExpelClanMember(pJob);
			}
			break;
			/*
		case MASYNCJOB_INSERTQUESTGAMELOG :
			{

			}
			break;
		case MASYNCJOB_UPDATEQUESTITEMINFO :
			{
			}
			break;
			*/

		case MASYNCJOB_PROBABILITYEVENTPERTIME :
			{
				OnAsyncInsertEvent( pJob );
			}
			break;

		case MASYNCJOB_UPDATEIPTOCOUNTRYLIST :
			{
				OnAsyncUpdateIPtoCoutryList( pJob );
			};
			break;

		case MASYNCJOB_UPDATEBLOCKCOUNTRYCODELIST :
			{
				OnAsyncUpdateBlockCountryCodeList( pJob );
			}
			break;

		case MASYNCJOB_UPDATECUSTOMIPLIST :
			{
				OnAsyncUpdateCustomIPList( pJob );
			}
			break;
		};

		delete pJob;
	}
}



void MMatchServer::OnAsyncGetLoginInfo(MAsyncJob* pJobInput)
{
	MAsyncDBJob_GetLoginInfo* pJob = (MAsyncDBJob_GetLoginInfo*)pJobInput;

	if (pJob->GetResult() != MASYNC_RESULT_SUCCEED) 
	{
		delete pJob->GetAccountInfo();

		// 접속 끊어버리자
		Disconnect(pJob->GetCommUID());
		return;
	}

	MUID CommUID = pJob->GetCommUID();
	MMatchAccountInfo* pAccountInfo = pJob->GetAccountInfo();


#ifndef _DEBUG
	MMatchObject* pCopyObj = GetPlayerByAID(pAccountInfo->m_nAID);
	if (pCopyObj != NULL) 
	{
		DisconnectObject(pCopyObj->GetUID());
	}
#endif

	if ((pAccountInfo->m_nUGrade == MMUG_BLOCKED) || (pAccountInfo->m_nUGrade == MMUG_PENALTY))
	{
		MCommand* pCmd = CreateCmdMatchResponseLoginFailed(CommUID, MERR_CLIENT_MMUG_BLOCKED);
		Post(pCmd);

		delete pJob->GetAccountInfo();
		return;
	}

	AddObjectOnMatchLogin(CommUID, pJob->GetAccountInfo(), pJob->IsFreeLoginIP(), pJob->GetCountryCode3(), pJob->GetChecksumPack());

	if (pJob->GetAccountInfo())
	{
		delete pJob->GetAccountInfo();
	}
}

void MMatchServer::OnAsyncGetAccountCharList(MAsyncJob* pJobResult)
{
	MAsyncDBJob_GetAccountCharList* pJob = (MAsyncDBJob_GetAccountCharList*)pJobResult;

	if (pJob->GetResult() != MASYNC_RESULT_SUCCEED) {
		char szTime[128];
		strftime(szTime, sizeof(szTime), "%T", localtime(&unmove(time(0))));

		mlog("[%s] Async DB Query(ResponseAccountCharList) Failed\n", szTime);
		return;
	}		

	MMatchObject* pObj = GetObject(pJob->GetUID());
	if (pObj == NULL) return;
	if (pJob->GetResultCommand() == NULL) return;

	pObj->CheckNewbie(pJob->GetCharMaxLevel());

	RouteToListener(pObj, pJob->GetResultCommand());
}

void MMatchServer::OnAsyncGetAccountCharInfo(MAsyncJob* pJobResult)
{
	MAsyncDBJob_GetAccountCharInfo* pJob = (MAsyncDBJob_GetAccountCharInfo*)pJobResult;

	if (pJob->GetResult() != MASYNC_RESULT_SUCCEED) {
		char szTime[128];
		strftime(szTime, sizeof(szTime), "%T", localtime(&unmove(time(0))));

		mlog("[%s] Async DB Query(ResponseAccountCharInfo) Failed\n", szTime);
		return;
	}		

	MMatchObject* pObj = GetObject(pJob->GetUID());
	if (pObj == NULL) return;
	if (pJob->GetResultCommand() == NULL) return;

	RouteToListener(pObj, pJob->GetResultCommand());
}


void MMatchServer::OnAsyncGetCharInfo(MAsyncJob* pJobResult)
{
	MAsyncDBJob_GetCharInfo* pJob = (MAsyncDBJob_GetCharInfo*)pJobResult;

	if (pJob->GetResult() != MASYNC_RESULT_SUCCEED) {
		mlog("DB Query(OnAsyncGetCharInfo > GetCharInfoByAID) Failed\n");
		return;
	}

	MMatchObject* pObj = GetObject(pJob->GetUID());
	if (pObj == NULL) return;

	if (pObj->GetCharInfo())
	{
		if (pObj->GetCharInfo()->m_nCID != 0)
		{
			CharFinalize(pObj->GetUID());
		}

		pObj->FreeCharInfo();
		pObj->FreeFriendInfo();
	}

	if (pJob->GetCharInfo() == NULL)
	{
		mlog("pJob->GetCharInfo() IS NULL\n");
		return;
	}
	pObj->SetCharInfo(pJob->GetCharInfo());		// Save Async Result
//	pObj->SetFriendInfo(pJob->GetFriendInfo());	// Save Async Result

	if (CharInitialize(pJob->GetUID()) == false)
	{
		mlog("OnAsyncGetCharInfo > CharInitialize failed");
		return;
	}

	// Client에 선택한 캐릭터 정보 전송
	MTD_CharInfo trans_charinfo;
	CopyCharInfoForTrans(&trans_charinfo, pJob->GetCharInfo(), pObj);
	
	MCommand* pNewCmd = CreateCommand(MC_MATCH_RESPONSE_SELECT_CHAR, MUID(0,0));
	pNewCmd->AddParameter(new MCommandParameterInt(MOK));		// result

	void* pCharArray = MMakeBlobArray(sizeof(MTD_CharInfo), 1);
	MTD_CharInfo* pTransCharInfo = (MTD_CharInfo*)MGetBlobArrayElement(pCharArray, 0);
	memcpy(pTransCharInfo, &trans_charinfo, sizeof(MTD_CharInfo));
	pNewCmd->AddParameter(new MCommandParameterBlob(pCharArray, MGetBlobArraySize(pCharArray)));
	MEraseBlobArray(pCharArray);


	// 내 캐릭터의 추가 정보
	void* pMyExtraInfoArray = MMakeBlobArray(sizeof(MTD_MyExtraCharInfo), 1);
	MTD_MyExtraCharInfo* pMyExtraInfo = (MTD_MyExtraCharInfo*)MGetBlobArrayElement(pMyExtraInfoArray, 0);
	int nPercent = MMatchFormula::GetLevelPercent(trans_charinfo.nXP, (int)trans_charinfo.nLevel);
	pMyExtraInfo->nLevelPercent = (char)nPercent;
	pNewCmd->AddParameter(new MCommandParameterBlob(pMyExtraInfoArray, MGetBlobArraySize(pMyExtraInfoArray)));
	MEraseBlobArray(pMyExtraInfoArray);

	RouteToListener(pObj, pNewCmd);

#ifdef _DELETE_CLAN
	if( MMCDS_NORMAL != pJob->GetDeleteState() )
	{
		if( MMCDS_WAIT == pJob->GetDeleteState() )
		{
			// 글랜 폐쇄 날짜를 알려줌.
			
			MCommand* pCmdDelClan = CreateCommand( MC_MATCH_CLAN_ACCOUNCE_DELETE, pObj->GetUID() );
			pCmdDelClan->AddParameter( new MCmdParamStr(pObj->GetCharInfo()->m_ClanInfo.m_strDeleteDate.c_str()) );
			Post( pCmdDelClan );
		}
		else if( MMCDS_DELETE == pJob->GetDeleteState() )
		{
			// 클랜 폐쇄 시킴.
		}
	}
#endif
}

void MMatchServer::OnAsyncGetFriendList(MAsyncJob* pJobInput)
{
	MAsyncDBJob_FriendList* pJob = (MAsyncDBJob_FriendList*)pJobInput;

	if (pJob->GetResult() != MASYNC_RESULT_SUCCEED) 
	{
		return;
	}

	MMatchObject* pObj = GetObject(pJob->GetUID());
	if (!IsEnabledObject(pObj)) return;

	pObj->SetFriendInfo(pJob->GetFriendInfo());	// Save Async Result

	FriendList(pObj->GetUID());
}

void MMatchServer::OnAsyncCreateChar(MAsyncJob* pJobResult)
{
	MAsyncDBJob_CreateChar* pJob = (MAsyncDBJob_CreateChar*)pJobResult;

	if (pJob->GetResult() != MASYNC_RESULT_SUCCEED) {
		char szTime[128];
		strftime(szTime, sizeof(szTime), "%T", localtime(&unmove(time(0))));

		mlog("[%s] Async DB Query(CreateChar) Failed\n", szTime);
		return;
	}		

	MMatchObject* pObj = GetObject(pJob->GetUID());
	if (pObj == NULL) return;
	if (pJob->GetResultCommand() == NULL) return;

	RouteToListener(pObj, pJob->GetResultCommand());
}

void MMatchServer::OnAsyncDeleteChar(MAsyncJob* pJobResult)
{
	MAsyncDBJob_DeleteChar* pJob = (MAsyncDBJob_DeleteChar*)pJobResult;

	if (pJob->GetResult() != MASYNC_RESULT_SUCCEED) {
		char szTime[128];
		strftime(szTime, sizeof(szTime), "%T", localtime(&unmove(time(0))));

		mlog("[%s] Async DB Query(DeleteChar) Failed\n", szTime);
	}		

	MMatchObject* pObj = GetObject(pJob->GetUID());
	if (pObj == NULL) return;

	RouteResponseToListener(pObj, MC_MATCH_RESPONSE_DELETE_CHAR, pJob->GetDeleteResult());
}

void MMatchServer::OnAsyncWinTheClanGame(MAsyncJob* pJobInput)
{
	if (pJobInput->GetResult() != MASYNC_RESULT_SUCCEED) {
		char szTime[128];
		strftime(szTime, sizeof(szTime), "%T", localtime(&unmove(time(0))));

		mlog("[%s] Async DB Query(OnAsyncWinTheClanGame) Failed\n", szTime);
		return;
	}		

}


void MMatchServer::OnAsyncUpdateCharInfoData(MAsyncJob* pJobInput)
{
	if (pJobInput->GetResult() != MASYNC_RESULT_SUCCEED) {
		char szTime[128];
		strftime(szTime, sizeof(szTime), "%T", localtime(&unmove(time(0))));

		mlog("[%s] Async DB Query(OnAsyncUpdateCharInfoData) Failed\n", szTime);
		return;
	}		

}

void MMatchServer::OnAsyncCharFinalize(MAsyncJob* pJobInput)
{
	if (pJobInput->GetResult() != MASYNC_RESULT_SUCCEED) {
		char szTime[128];
		strftime(szTime, sizeof(szTime), "%T", localtime(&unmove(time(0))));

		mlog("[%s] Async DB Query(OnAsyncCharFinalize) Failed\n", szTime);
		return;
	}		

}

void MMatchServer::OnAsyncBringAccountItem(MAsyncJob* pJobResult)
{
	MAsyncDBJob_BringAccountItem* pJob = (MAsyncDBJob_BringAccountItem*)pJobResult;

	MMatchObject* pObj = GetObject(pJob->GetUID());
	if (!IsEnabledObject(pObj)) return;

	int nRet = MERR_UNKNOWN;

	if (pJob->GetResult() == MASYNC_RESULT_SUCCEED) 
	{
		u32 nNewCIID =	pJob->GetNewCIID();
		u32 nNewItemID =	pJob->GetNewItemID();
		bool bIsRentItem =				pJob->GetRentItem();
		int nRentMinutePeriodRemainder = pJob->GetRentMinutePeriodRemainder();




		// 오브젝트에 아이템 추가
		MUID uidNew = MMatchItemMap::UseUID();
		pObj->GetCharInfo()->m_ItemList.CreateItem(uidNew, nNewCIID, nNewItemID, bIsRentItem, nRentMinutePeriodRemainder);

		nRet = MOK;
	}		

	ResponseCharacterItemList(pJob->GetUID());	// 새로 바뀐 아이템 리스트도 다시 뿌려준다.


	MCommand* pNew = CreateCommand(MC_MATCH_RESPONSE_BRING_ACCOUNTITEM, MUID(0,0));
	pNew->AddParameter(new MCmdParamInt(nRet));
	RouteToListener(pObj, pNew);


}

void MMatchServer::OnAsyncInsertConnLog(MAsyncJob* pJobResult)
{
	if (pJobResult->GetResult() != MASYNC_RESULT_SUCCEED) {
		char szTime[128];
		strftime(szTime, sizeof(szTime), "%T", localtime(&unmove(time(0))));

		mlog("[%s] Async DB Query(OnAsyncInsertConnLog) Failed\n", szTime);
		return;
	}		

}

void MMatchServer::OnAsyncInsertGameLog(MAsyncJob* pJobResult)
{
	if (pJobResult->GetResult() != MASYNC_RESULT_SUCCEED) {
		char szTime[128];
		strftime(szTime, sizeof(szTime), "%T", localtime(&unmove(time(0))));

		mlog("[%s] Async DB Query(OnAsyncInsertGameLog) Failed\n", szTime);
		return;
	}		
}

void MMatchServer::OnAsyncCreateClan(MAsyncJob* pJobResult)
{
	MAsyncDBJob_CreateClan* pJob = (MAsyncDBJob_CreateClan*)pJobResult;
	
	MUID uidMaster = pJob->GetMasterUID();
	MMatchObject* pMasterObject = GetObject(uidMaster);
	

	if (pJob->GetResult() != MASYNC_RESULT_SUCCEED) {
		if (IsEnabledObject(pMasterObject))
		{
			RouteResponseToListener(pMasterObject, MC_MATCH_CLAN_RESPONSE_AGREED_CREATE_CLAN, MERR_CLAN_CANNOT_CREATE);
		}
		return;
	}		

	int nNewCLID = pJob->GetNewCLID();

	if ( (pJob->GetDBResult() == false) || (nNewCLID ==0) )
	{
		if (IsEnabledObject(pMasterObject))
		{
			RouteResponseToListener(pMasterObject, MC_MATCH_CLAN_RESPONSE_AGREED_CREATE_CLAN, MERR_CLAN_CANNOT_CREATE);
		}
		return;
	}


	// 마스터의 바운티를 깎는다.
	if (IsEnabledObject(pMasterObject))
	{
		pMasterObject->GetCharInfo()->IncBP(-CLAN_CREATING_NEED_BOUNTY);
		ResponseMySimpleCharInfo(pMasterObject->GetUID());
	
		UpdateCharClanInfo(pMasterObject, nNewCLID, pJob->GetClanName(), MCG_MASTER);
	}


#if CLAN_SPONSORS_COUNT > 0
	MMatchObject* pSponsorObjects[CLAN_SPONSORS_COUNT];
	_ASSERT(CLAN_SPONSORS_COUNT == 4);

	pSponsorObjects[0] = GetObject(pJob->GetMember1UID());
	pSponsorObjects[1] = GetObject(pJob->GetMember2UID());
	pSponsorObjects[2] = GetObject(pJob->GetMember3UID());
	pSponsorObjects[3] = GetObject(pJob->GetMember4UID());

	for (int i = 0; i < CLAN_SPONSORS_COUNT; i++)
	{
		if (IsEnabledObject(pSponsorObjects[i]))
		{
			UpdateCharClanInfo(pSponsorObjects[i], nNewCLID, pJob->GetClanName(), MCG_MEMBER);
			RouteResponseToListener(pSponsorObjects[i], MC_MATCH_RESPONSE_RESULT, MRESULT_CLAN_CREATED);
		}
	}
#endif

	if (IsEnabledObject(pMasterObject))
	{
		RouteResponseToListener(pMasterObject, MC_MATCH_CLAN_RESPONSE_AGREED_CREATE_CLAN, MOK);
	}
}

void MMatchServer::OnAsyncExpelClanMember(MAsyncJob* pJobResult)
{
	auto* pJob = static_cast<MAsyncDBJob_ExpelClanMember*>(pJobResult);

	MMatchObject* pAdminObject = GetObject(pJob->GetAdminUID());

	int ErrCode;

	switch (pJob->GetDBResult())
	{
	case ExpelResult::OK:
	{
		MMatchObject* pMemberObject = GetPlayerByName(pJob->GetTarMember());
		if (IsEnabledObject(pMemberObject))
			UpdateCharClanInfo(pMemberObject, 0, "", MCG_NONE);

		ErrCode = MOK;
		break;
	}
	case ExpelResult::NoSuchMember:
		ErrCode = MERR_CLAN_CANNOT_EXPEL_FOR_NO_MEMBER;
		break;
	case ExpelResult::TooLowGrade:
		ErrCode = MERR_CLAN_CANNOT_CHANGE_GRADE;
		break;
	default:
		ErrCode = MERR_UNKNOWN;
		break;
	}

	if (IsEnabledObject(pAdminObject))
		RouteResponseToListener(pAdminObject, MC_MATCH_CLAN_ADMIN_RESPONSE_EXPEL_MEMBER, ErrCode);
}



void MMatchServer::OnAsyncInsertEvent( MAsyncJob* pJobResult )
{
	if( 0 == pJobResult )
		return;

	MAsyncDBJob_ProbabiltyEventPerTime* pEventJob = 
		reinterpret_cast< MAsyncDBJob_ProbabiltyEventPerTime* >( pJobResult );

	if( pEventJob->GetAnnounce().empty() )
		return;

	if( MASYNC_RESULT_SUCCEED == pJobResult->GetResult() )
	{
		MCommand* pCmd;
		AsyncEventObjVec::const_iterator it, end;
		const AsyncEventObjVec& EventObjList = pEventJob->GetEventObjList();

		end = EventObjList.end();
		for( it = EventObjList.begin(); it != end; ++it )
		{
			if( MUID(0, 0) != it->uidUser )
			{
				pCmd = CreateCommand( MC_MATCH_ANNOUNCE, it->uidUser );
				if( 0 != pCmd )
				{
					pCmd->AddParameter( new MCmdParamUInt(0) );
					pCmd->AddParameter( new MCmdParamStr(pEventJob->GetAnnounce().c_str()) );
					Post( pCmd );
				}
			}
		}
	}
}


void MMatchServer::OnAsyncUpdateIPtoCoutryList( MAsyncJob* pJobResult )
{
	MCommand* pCmd = CreateCommand( MC_LOCAL_UPDATE_IP_TO_COUNTRY, GetUID() );
	if( 0 != pCmd )
		Post( pCmd );
}


void MMatchServer::OnAsyncUpdateBlockCountryCodeList( MAsyncJob* pJobResult )
{
	MCommand* pCmd = CreateCommand( MC_LOCAL_UPDATE_BLOCK_COUTRYCODE, GetUID() );
	if( 0 != pCmd )
		Post( pCmd );
}


void MMatchServer::OnAsyncUpdateCustomIPList( MAsyncJob* pJobResult )
{
	MCommand* pCmd = CreateCommand( MC_LOCAL_UPDATE_CUSTOM_IP, GetUID() );
	if( 0 != pCmd )
		Post( pCmd );
}