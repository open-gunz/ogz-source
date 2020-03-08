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
#include "MLadderMgr.h"
#include "MTeamGameStrategy.h"




/////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////
int MMatchServer::ValidateChallengeLadderGame(MMatchObject** ppMemberObject, int nMemberCount)
{
	MBaseTeamGameStrategy* pTeamGameStrategy = MBaseTeamGameStrategy::GetInstance(MGetServerConfig()->GetServerMode());
	if (pTeamGameStrategy)
	{
		int nRet = pTeamGameStrategy->ValidateChallenge(ppMemberObject, nMemberCount);
		return nRet;
	};

	return MOK;
}

///////////////////////////////////////////////////////////////////////////
// LadderStage
bool MMatchServer::LadderJoin(const MUID& uidPlayer, const MUID& uidStage, MMatchTeam nTeam)
{
	MMatchObject* pObj = GetObject(uidPlayer);
	if (pObj == NULL) return false;

	if (pObj->GetStageUID() != MUID(0,0))
		StageLeave(pObj->GetUID(), pObj->GetStageUID());

	MMatchStage* pStage = FindStage(uidStage);
	if (pStage == NULL) return false;

	pObj->OnStageJoin();

	// Join
	pStage->AddObject(uidPlayer, pObj);
	pObj->SetStageUID(uidStage);
	pObj->SetStageState(MOSS_READY);
	pObj->SetLadderChallenging(false);
	pStage->PlayerTeam(uidPlayer, nTeam);
	pStage->PlayerState(uidPlayer, MOSS_READY);
	

	MCommand* pCmd = CreateCommand(MC_MATCH_LADDER_PREPARE, uidPlayer);
	pCmd->AddParameter(new MCmdParamUID(uidStage));
	pCmd->AddParameter(new MCmdParamInt(nTeam));
	Post(pCmd);

	return true;
}

void MMatchServer::LadderGameLaunch(MLadderGroup* pGroupA, MLadderGroup* pGroupB)
{
	if ((MGetServerConfig()->GetServerMode() != MSM_LADDER) && 
		(MGetServerConfig()->GetServerMode() != MSM_CLAN) &&
		(MGetServerConfig()->GetServerMode() != MSM_TEST))  return;

	MUID uidStage = MUID(0,0);
	if (StageAdd(NULL, "LADDER_GAME", true, "", &uidStage) == false) {
		// Group 해체
		GetLadderMgr()->CancelChallenge(pGroupA->GetID(), "");
		GetLadderMgr()->CancelChallenge(pGroupB->GetID(), "");
		return;
	}
	MMatchStage* pStage = FindStage(uidStage);
	if (pStage == NULL) {
		// Group 해체
		GetLadderMgr()->CancelChallenge(pGroupA->GetID(), "");
		GetLadderMgr()->CancelChallenge(pGroupB->GetID(), "");
		return;
	}

	// A 그룹 입장
	for (list<MUID>::iterator i=pGroupA->GetPlayerListBegin(); i!= pGroupA->GetPlayerListEnd(); i++)
	{
		MUID uidPlayer = (*i);
		LadderJoin(uidPlayer, uidStage, MMT_RED);
	}
	// B 그룹 입장
	for (list<MUID>::iterator i=pGroupB->GetPlayerListBegin(); i!= pGroupB->GetPlayerListEnd(); i++)
	{
		MUID uidPlayer = (*i);
		LadderJoin(uidPlayer, uidStage, MMT_BLUE);
	}

	// Agent 준비
//	ReserveAgent(pStage);

	//////////////////////////////////////////////////////////////////////////////
	int nRandomMap = 0;
	// 클랜전은 Stage의 팀정보에 CLID까지 설정해야한다.
	MBaseTeamGameStrategy* pTeamGameStrategy = MBaseTeamGameStrategy::GetInstance(MGetServerConfig()->GetServerMode());
	if (pTeamGameStrategy)
	{
		nRandomMap = pTeamGameStrategy->GetRandomMap((int)pGroupA->GetPlayerCount());
	};


	MMATCH_GAMETYPE nGameType = MMATCH_GAMETYPE_DEATHMATCH_TEAM;

	// Game 설정
	pStage->SetStageType(MST_LADDER);
	pStage->ChangeRule(nGameType);

	// 클랜전은 Stage의 팀정보에 CLID까지 설정해야한다.
	if (pTeamGameStrategy)
	{
		MMatchLadderTeamInfo a_RedLadderTeamInfo, a_BlueLadderTeamInfo;
		pTeamGameStrategy->SetStageLadderInfo(&a_RedLadderTeamInfo, &a_BlueLadderTeamInfo, pGroupA, pGroupB);

		pStage->SetLadderTeam(&a_RedLadderTeamInfo, &a_BlueLadderTeamInfo);
	};

	MMatchStageSetting* pSetting = pStage->GetStageSetting();
	pSetting->SetMasterUID(MUID(0,0));
	pSetting->SetMapIndex(nRandomMap);
	pSetting->SetGameType(nGameType);

	pSetting->SetLimitTime(3);	
	pSetting->SetRoundMax(99);		// 최대 99라운드까지 진행할 수 있다.
	

	MCommand* pCmd = CreateCmdResponseStageSetting(uidStage);
	RouteToStage(uidStage, pCmd);	// Stage Setting 전송


	// 디비에 로그를 남긴다.
	// test 맵등은 로그 남기지 않는다.
	if ( (MIsCorrectMap(nRandomMap)) && (MGetGameTypeMgr()->IsCorrectGameType(nGameType)) )
	{
		if (pStage->StartGame() == true) {		// 게임시작
			// Send Launch Command
			ReserveAgent(pStage);

			MCommand* pCmd = CreateCommand(MC_MATCH_LADDER_LAUNCH, MUID(0,0));
			pCmd->AddParameter(new MCmdParamUID(uidStage));
			pCmd->AddParameter(new MCmdParamStr( const_cast<char*>(pStage->GetMapName()) ));
			RouteToStage(uidStage, pCmd);

			// Ladder Log 남긴다.
		} else {
			// Group 해체
			GetLadderMgr()->CancelChallenge(pGroupA->GetID(), "");
			GetLadderMgr()->CancelChallenge(pGroupB->GetID(), "");
		}
	}
}

void MMatchServer::OnLadderRequestChallenge(const MUID& uidPlayer, void* pMemberNamesBlob, u32 nOptions)
{
	if ((MGetServerConfig()->GetServerMode() != MSM_LADDER) && 
		(MGetServerConfig()->GetServerMode() != MSM_CLAN) &&
		(MGetServerConfig()->GetServerMode() != MSM_TEST)) return;

	MMatchObject* pLeaderObject = GetPlayerByCommUID(uidPlayer);
	if (! IsEnabledObject(pLeaderObject)) return;

	if (!MGetServerConfig()->IsEnabledCreateLadderGame())
	{
		RouteResponseToListener(pLeaderObject, MC_MATCH_LADDER_RESPONSE_CHALLENGE, MERR_LADDER_CANNOT_CHALLENGE);
		return;
	}

	int nBlobCount = MGetBlobArrayCount(pMemberNamesBlob);
	int nMemberCount = nBlobCount;
	if (nMemberCount <= 0) return;


	MMatchObject* pMemberObjects[MAX_CLANBATTLE_TEAM_MEMBER];
	for (int i = 0; i < nMemberCount; i++)
	{
		MTD_LadderTeamMemberNode* pNode = (MTD_LadderTeamMemberNode*)MGetBlobArrayElement(pMemberNamesBlob, i);
		if (pNode == NULL) break;
		if ((strlen(pNode->szName) <= 0) || (strlen(pNode->szName) >= MATCHOBJECT_NAME_LENGTH)) return;

		pMemberObjects[i] = GetPlayerByName(pNode->szName);

		// 한명이라도 존재하지 않으면 안된다
		if (! IsEnabledObject(pMemberObjects[i]))
		{
			// 메세지 보내주고 끝.
			RouteResponseToListener(pLeaderObject, MC_MATCH_LADDER_RESPONSE_CHALLENGE, MERR_LADDER_CANNOT_CHALLENGE);
			return;
		}
	}


	int nRet = ValidateChallengeLadderGame(pMemberObjects, nMemberCount);
	if (nRet != MOK)
	{
		RouteResponseToListener(pLeaderObject, MC_MATCH_LADDER_RESPONSE_CHALLENGE, nRet);
		return;
	}

	int nTeamID = 0;

	MBaseTeamGameStrategy* pTeamGameStrategy = NULL;

	pTeamGameStrategy = MBaseTeamGameStrategy::GetInstance(MGetServerConfig()->GetServerMode());
	if (pTeamGameStrategy)
	{
        nTeamID = pTeamGameStrategy->GetNewGroupID(pLeaderObject, pMemberObjects, nMemberCount);
	}
	if (nTeamID == 0) return;

	// 실제로 Challenge한다.
	// Ensure All Player Not in LadderGroup
	MLadderGroup* pGroup = GetLadderMgr()->CreateLadderGroup();
	pGroup->SetID(nTeamID);

	// balancedMatching 설정
	if (nOptions == 1)
	{
		pGroup->SetBalancedMatching(true);
	}
	else
	{
		pGroup->SetBalancedMatching(false);	
	}

	if (pTeamGameStrategy)
	{
		pTeamGameStrategy->SetLadderGroup(pGroup, pMemberObjects, nMemberCount);
	}

	for (int i=0; i<nMemberCount; i++) {
		pGroup->AddPlayer( pMemberObjects[i] );
	}

	GetLadderMgr()->Challenge(pGroup);
}

void MMatchServer::OnLadderRequestCancelChallenge(const MUID& uidPlayer)
{
	MMatchObject* pObj = GetObject(uidPlayer);
	if (!IsEnabledObject(pObj)) return;
	if (pObj->GetLadderGroupID() == 0) return;

	GetLadderMgr()->CancelChallenge(pObj->GetLadderGroupID(), pObj->GetCharInfo()->m_szName);
}

void MMatchServer::OnRequestProposal(const MUID& uidProposer, const int nProposalMode, const int nRequestID, 
		                const int nReplierCount, void* pReplierNamesBlob)
{
	MMatchObject* pProposerObject = GetObject(uidProposer);
	if (! IsEnabledObject(pProposerObject)) return;


	if ((nReplierCount > MAX_REPLIER) || (nReplierCount < 0))
	{
		_ASSERT(0);	// 16명이상 동의할 수 없음
		return;
	}


	if (!MGetServerConfig()->IsEnabledCreateLadderGame())
	{
		// 메세지 보내주고 끝.
		MCommand* pNewCmd = CreateCommand(MC_MATCH_RESPONSE_PROPOSAL, MUID(0,0));
		pNewCmd->AddParameter(new MCommandParameterInt(MERR_LADDER_CANNOT_CHALLENGE));
		pNewCmd->AddParameter(new MCommandParameterInt(nProposalMode));
		pNewCmd->AddParameter(new MCommandParameterInt(nRequestID));
		RouteToListener(pProposerObject, pNewCmd);
		return;
	}


	int nBlobCount = MGetBlobArrayCount(pReplierNamesBlob);
	if (nBlobCount != nReplierCount) return;

	MMatchObject* ppReplierObjects[MAX_REPLIER];

	for (int i = 0; i < nReplierCount; i++)
	{
		MTD_ReplierNode* pNode = (MTD_ReplierNode*)MGetBlobArrayElement(pReplierNamesBlob, i);
		if (pNode == NULL) return;
		if ((strlen(pNode->szName) <= 0) || (strlen(pNode->szName) >= MATCHOBJECT_NAME_LENGTH)) return;

		ppReplierObjects[i] = GetPlayerByName(pNode->szName);

		// 답변자가 한명이라도 존재하지 않으면 안된다
		if (!IsEnabledObject(ppReplierObjects[i]))
		{
			// 메세지 보내주고 끝.
			MCommand* pNewCmd = CreateCommand(MC_MATCH_RESPONSE_PROPOSAL, MUID(0,0));
			pNewCmd->AddParameter(new MCommandParameterInt(MERR_NO_TARGET));
			pNewCmd->AddParameter(new MCommandParameterInt(nProposalMode));
			pNewCmd->AddParameter(new MCommandParameterInt(nRequestID));
			RouteToListener(pProposerObject, pNewCmd);

			return;
		}
	}

	int nRet = MERR_UNKNOWN;
	// 상황에 맞게 validate 한다.

	switch (nProposalMode)
	{
	case MPROPOSAL_LADDER_INVITE:
		{
			MLadderGameStrategy* pLadderGameStrategy = MLadderGameStrategy::GetInstance();
			nRet = pLadderGameStrategy->ValidateRequestInviteProposal(pProposerObject, ppReplierObjects, nReplierCount);
		}
		break;
	case MPROPOSAL_CLAN_INVITE:
		{
			MClanGameStrategy* pClanGameStrategy = MClanGameStrategy::GetInstance();
			nRet = pClanGameStrategy->ValidateRequestInviteProposal(pProposerObject, ppReplierObjects, nReplierCount);
		}
		break;
	};

	if (nRet != MOK)
	{
		MCommand* pNewCmd = CreateCommand(MC_MATCH_RESPONSE_PROPOSAL, MUID(0,0));
		pNewCmd->AddParameter(new MCommandParameterInt(nRet));
		pNewCmd->AddParameter(new MCommandParameterInt(nProposalMode));
		pNewCmd->AddParameter(new MCommandParameterInt(nRequestID));
		RouteToListener(pProposerObject, pNewCmd);
		return;
	}


	int nMemberCount = nReplierCount+1;		// 제안자까지 
	void* pBlobMembersNameArray = MMakeBlobArray(sizeof(MTD_ReplierNode), nMemberCount);

	MTD_ReplierNode* pProposerNode = (MTD_ReplierNode*)MGetBlobArrayElement(pBlobMembersNameArray, 0);
	strcpy_safe(pProposerNode->szName, pProposerObject->GetCharInfo()->m_szName);

	for (int k = 0; k < nReplierCount; k++)
	{
		MTD_ReplierNode* pMemberNode = (MTD_ReplierNode*)MGetBlobArrayElement(pBlobMembersNameArray, k+1);
		strcpy_safe(pMemberNode->szName, ppReplierObjects[k]->GetCharInfo()->m_szName);
	}

	// 답변자에게 동의를 물어본다.
	for (int i = 0; i < nReplierCount; i++)
	{
		MCommand* pNewCmd = CreateCommand(MC_MATCH_ASK_AGREEMENT, MUID(0,0));
		pNewCmd->AddParameter(new MCommandParameterUID(uidProposer));
//		pNewCmd->AddParameter(new MCommandParameterString(pProposerObject->GetCharInfo()->m_szName));
		pNewCmd->AddParameter(new MCommandParameterBlob(pBlobMembersNameArray, MGetBlobArraySize(pBlobMembersNameArray)));

		pNewCmd->AddParameter(new MCommandParameterInt(nProposalMode));
		pNewCmd->AddParameter(new MCommandParameterInt(nRequestID));
		RouteToListener(ppReplierObjects[i], pNewCmd);


	}
	MEraseBlobArray(pBlobMembersNameArray);


	// 제안자에게 응답 보내줌
	MCommand* pNewCmd = CreateCommand(MC_MATCH_RESPONSE_PROPOSAL, MUID(0,0));
	pNewCmd->AddParameter(new MCommandParameterInt(nRet));
	pNewCmd->AddParameter(new MCommandParameterInt(nProposalMode));
	pNewCmd->AddParameter(new MCommandParameterInt(nRequestID));
	RouteToListener(pProposerObject, pNewCmd);

}

void MMatchServer::OnReplyAgreement(MUID& uidProposer, MUID& uidReplier, const char* szReplierName, 
		                const int nProposalMode, const int nRequestID, const bool bAgreement)
{
	MMatchObject* pProposerObject = GetObject(uidProposer);
	if (! IsEnabledObject(pProposerObject)) return;

	
	MCommand* pNewCmd = CreateCommand(MC_MATCH_REPLY_AGREEMENT, MUID(0,0));
	pNewCmd->AddParameter(new MCommandParameterUID(uidProposer));
	pNewCmd->AddParameter(new MCommandParameterUID(uidReplier));
	pNewCmd->AddParameter(new MCommandParameterString(szReplierName));
	pNewCmd->AddParameter(new MCommandParameterInt(nProposalMode));
	pNewCmd->AddParameter(new MCommandParameterInt(nRequestID));
	pNewCmd->AddParameter(new MCommandParameterBool(bAgreement));

	RouteToListener(pProposerObject, pNewCmd);	
}
