#include "stdafx.h"
#include "MTeamGameStrategy.h"
#include "MMatchServer.h"
#include "MSharedCommandTable.h"
#include "MErrorTable.h"
#include "MBlobArray.h"
#include "MObject.h"
#include "MMatchObject.h"
#include "MMatchStage.h"
#include "MMatchConfig.h"
#include "MCommandCommunicator.h"
#include "MMatchTransDataType.h"
#include "MDebug.h"
#include "MLadderMgr.h"
#include "MLadderGroup.h"
#include "MMatchLocale.h"

/////////////////////////////////////////////////////////////////////////////////////////////////////////
MBaseTeamGameStrategy* MBaseTeamGameStrategy::GetInstance(MMatchServerMode nServerMode)
{
	switch (nServerMode)
	{
	case MSM_LADDER:
		return MLadderGameStrategy::GetInstance();
	case MSM_CLAN:
		//what does this do?
		return MClanGameStrategy::GetInstance();
	case MSM_TEST:
		return MClanGameStrategy::GetInstance();
	default:
		_ASSERT(0);
	}
	return NULL;
}


int MLadderGameStrategy::ValidateChallenge(MMatchObject** ppMemberObject, int nMemberCount)
{
	if (nMemberCount > MAX_LADDER_TEAM_MEMBER) return MERR_LADDER_NO_TEAM_MEMBER;
	int nCIDs[MAX_LADDER_TEAM_MEMBER];
	
	for (int i = 0; i < nMemberCount; i++)
	{
		if (! IsEnabledObject(ppMemberObject[i])) return MERR_LADDER_NO_TEAM_MEMBER;
		if (ppMemberObject[i]->IsLadderChallenging() != false) return MERR_LADDER_EXIST_CANNOT_CHALLENGE_MEMBER;

		nCIDs[i] = ppMemberObject[i]->GetCharInfo()->m_nCID;
	}

	int nTeamID = MMatchServer::GetInstance()->GetLadderTeamIDFromDB(nMemberCount, nCIDs, nMemberCount);
	if (nTeamID == 0) return MERR_LADDER_WRONG_TEAM_MEMBER;

	return MOK;
}

int MLadderGameStrategy::ValidateRequestInviteProposal(MMatchObject* pProposerObject, MMatchObject** ppReplierObjects,
													   const int nReplierCount)
{
	int nRet = MERR_UNKNOWN;

	MMatchObject* ppTeamMemberObjects[MAX_REPLIER];
	int nTeamMemberCount = nReplierCount+1;	// 제안자까지..
	if (nTeamMemberCount <= MAX_LADDER_TEAM_MEMBER)
	{
		ppTeamMemberObjects[0] = pProposerObject;
		for (int i = 0; i < nReplierCount; i++)
		{
			ppTeamMemberObjects[i+1] = ppReplierObjects[i];
		}
		nRet = ValidateChallenge(ppTeamMemberObjects, nTeamMemberCount);

		#ifdef LIMIT_ACTIONLEAGUE
		{
			char szMembers[4][MATCHOBJECT_NAME_LENGTH] = {0,};
			char* szMemberTable[4] = { szMembers[0], szMembers[1], szMembers[2], szMembers[3] };

			if (MMatchServer::GetInstance()->GetDBMgr()->GetLadderTeamMemberByCID(pProposerObject->GetCharInfo()->m_nCID, 
				NULL, szMemberTable, 4)) 
			{
				char szAnnounce[1024];
				sprintf_safe(szAnnounce, "^1당신의 팀멤버는 ");
				for (int i=0; i<4; i++) {
					if (strlen(szMembers[i]) <= 0) break;
					strcat(szAnnounce, szMembers[i]);
					strcat(szAnnounce, " ");
				}
				strcat(szAnnounce, "입니다.");
				MMatchServer::GetInstance()->Announce(pProposerObject, szAnnounce);
			} else {
				MMatchServer::GetInstance()->Announce(pProposerObject, "^1액션리그를 신청한 캐릭터가 아닙니다.");
				return MERR_UNKNOWN;
			}

			bool bAllMember = true;
			for (int i=0; i<nReplierCount; i++) {
				bool bMember = false;
				for (int j=0; j<4; j++) {
					if (strcmp(ppReplierObjects[i]->GetName(), szMembers[j]) == 0) {
						bMember = true;
						break;
					}
				}
				if (bMember == false) {
					bAllMember = false;
				}
			}
			if (bAllMember == true) {
				nRet = MOK;	// Sub Team 구성가능하도록함
			} else {
				MMatchServer::GetInstance()->Announce(pProposerObject, "팀멤버가 아닌 사람과 팀을 이룰 수 없습니다.");
				return MERR_UNKNOWN;
			}				
		}
		#endif
	}

	return nRet;
}

int MLadderGameStrategy::GetNewGroupID(MMatchObject* pLeaderObject, MMatchObject** ppMemberObjects, int nMemberCount)
{
	int nTeamID = 0;
#ifdef LIMIT_ACTIONLEAGUE	// Team4의 Sub Team 지원
	if (false == MMatchServer::GetInstance()->GetDBMgr()->GetLadderTeamMemberByCID(pLeaderObject->GetCharInfo()->m_nCID, 
		&nTeamID, NULL, 0)) 
	{
		MMatchServer::GetInstance()->Announce(pLeaderObject, "^1액션리그를 신청한 캐릭터가 아닙니다.");
		return 0;
	}
#else
	int nCIDs[MAX_LADDER_TEAM_MEMBER];
	for (int i = 0; i < nMemberCount; i++)
	{
		nCIDs[i] = ppMemberObjects[i]->GetCharInfo()->m_nCID;
	}
	nTeamID = MMatchServer::GetInstance()->GetLadderTeamIDFromDB(nMemberCount, nCIDs, nMemberCount);

	int nRet = ValidateChallenge(ppMemberObjects, nMemberCount);
	if (nRet != MOK)
	{
		MMatchServer::GetInstance()->RouteResponseToListener(pLeaderObject, MC_MATCH_LADDER_RESPONSE_CHALLENGE, nRet);
		return 0;
	}
#endif

	return nTeamID;
}

void MLadderGameStrategy::SetStageLadderInfo(MMatchLadderTeamInfo* poutRedLadderInfo, MMatchLadderTeamInfo* poutBlueLadderInfo,
								MLadderGroup* pRedGroup, MLadderGroup* pBlueGroup)
{
	poutRedLadderInfo->nTID = pRedGroup->GetID();
	poutBlueLadderInfo->nTID = pBlueGroup->GetID();

	poutRedLadderInfo->nFirstMemberCount = (int)pRedGroup->GetLadderType();
	poutBlueLadderInfo->nFirstMemberCount = (int)pBlueGroup->GetLadderType();

	poutRedLadderInfo->nCLID = 0;
	poutBlueLadderInfo->nCLID = 0;
	poutRedLadderInfo->nCharLevel = 0;
	poutBlueLadderInfo->nCharLevel = 0;
	poutRedLadderInfo->nContPoint = 0;
	poutBlueLadderInfo->nContPoint = 0;

}

void MLadderGameStrategy::SavePointOnFinishGame(MMatchStage* pStage, MMatchTeam nWinnerTeam, bool bIsDrawGame,
		                               MMatchLadderTeamInfo* pRedLadderInfo, MMatchLadderTeamInfo* pBlueLadderInfo)
{
	int nWinnerTID = 0, nLoserTID = 0;

	// red팀 승리
	if (bIsDrawGame == true)
	{
		nWinnerTID = pRedLadderInfo->nTID;
		nLoserTID = pBlueLadderInfo->nTID;
	}
	else if (nWinnerTeam == MMT_RED)
	{
		nWinnerTID = pRedLadderInfo->nTID;
		nLoserTID = pBlueLadderInfo->nTID;
	}
	else if (nWinnerTeam == MMT_BLUE)
	{
		nWinnerTID = pBlueLadderInfo->nTID;
		nLoserTID = pRedLadderInfo->nTID;
	}
	else
	{
		_ASSERT(0);
	}

	if ((nWinnerTID == 0) || (nLoserTID == 0)) return;

	int nTeamMemberCount = pBlueLadderInfo->nFirstMemberCount;

#ifdef LIMIT_ACTIONLEAGUE
	MMatchServer::GetInstance()->SaveLadderTeamPointToDB(nTeamMemberCount, 
														nWinnerTID, nLoserTID, bIsDrawGame);
#endif
}

int MLadderGameStrategy::GetRandomMap(int nTeamMember)
{
	// Game Setting - 맵, TIC 클랜전은 다르게 해야한다. 월요일날 하장
	MMatchConfig* pConfig = MGetServerConfig();

	// Random 하게 맵을 고른다
	list<int> mapList;
	for (int i=0; i<MMATCH_MAP_MAX; i++) {
		if (pConfig->IsEnableMap(MMATCH_MAP(i)))
			mapList.push_back(i);
	}
	MTime time;
	int nRandomMapIndex = time.MakeNumber(0, (int)mapList.size()-1);
	int nRandomMap=0;

	list<int>::iterator mapItor = mapList.begin();
	for (int i = 0; i < nRandomMapIndex; i++) mapItor++;

	if (mapItor != mapList.end()) nRandomMap = (*mapItor);
	else nRandomMap = *mapList.begin();

	return nRandomMap;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////
void InsertLadderRandomMap(vector<int>& vec, int nNum, int nCount)
{
	for (int i = 0; i < nCount; i++)
		vec.push_back(nNum);
}

MClanGameStrategy::MClanGameStrategy()
{
	for (int i = MLADDERTYPE_NORMAL_2VS2; i <= MLADDERTYPE_NORMAL_3VS3; i++)
	{
		InsertLadderRandomMap(m_RandomMapVec[i], MMATCH_MAP_MANSION,		10);
		InsertLadderRandomMap(m_RandomMapVec[i], MMATCH_MAP_PRISON_II,		5);
		InsertLadderRandomMap(m_RandomMapVec[i], MMATCH_MAP_BATTLE_ARENA,	10);
		InsertLadderRandomMap(m_RandomMapVec[i], MMATCH_MAP_TOWN,			10);
		InsertLadderRandomMap(m_RandomMapVec[i], MMATCH_MAP_DUNGEON,		2);
		InsertLadderRandomMap(m_RandomMapVec[i], MMATCH_MAP_PORT,			10);
		InsertLadderRandomMap(m_RandomMapVec[i], MMATCH_MAP_CASTLE,			5);
		InsertLadderRandomMap(m_RandomMapVec[i], MMATCH_MAP_ISLAND,			5);
		InsertLadderRandomMap(m_RandomMapVec[i], MMATCH_MAP_GARDEN,			10);
	}

	for (int i = MLADDERTYPE_NORMAL_4VS4; i < MLADDERTYPE_MAX; i++)
	{
		InsertLadderRandomMap(m_RandomMapVec[i], MMATCH_MAP_MANSION,		10);
		InsertLadderRandomMap(m_RandomMapVec[i], MMATCH_MAP_PRISON_II,		5);
		InsertLadderRandomMap(m_RandomMapVec[i], MMATCH_MAP_BATTLE_ARENA,	10);
		InsertLadderRandomMap(m_RandomMapVec[i], MMATCH_MAP_TOWN,			10);
		InsertLadderRandomMap(m_RandomMapVec[i], MMATCH_MAP_DUNGEON,		10);
		InsertLadderRandomMap(m_RandomMapVec[i], MMATCH_MAP_PORT,			10);
		InsertLadderRandomMap(m_RandomMapVec[i], MMATCH_MAP_ISLAND,			5);
		InsertLadderRandomMap(m_RandomMapVec[i], MMATCH_MAP_GARDEN,			10);
		InsertLadderRandomMap(m_RandomMapVec[i], MMATCH_MAP_CASTLE,			10);
	}
}

int MClanGameStrategy::ValidateChallenge(MMatchObject** ppMemberObject, int nMemberCount)
{
	if ((nMemberCount<0) || (nMemberCount > MAX_CLANBATTLE_TEAM_MEMBER)) return MERR_CB_NO_TEAM_MEMBER;

	// 팀원수 체크
	bool bFit = false;
	for (int i = 0; i < MLADDERTYPE_MAX; i++)
	{
		if (nMemberCount == MMatchServer::GetInstance()->GetLadderMgr()->GetNeedMemberCount(MLADDERTYPE(i)))
		{
			bFit = true;
			break;
		}
	}
	if (!bFit) return MERR_CB_NO_TEAM_MEMBER;	// 적당한 인원수가 아님

	bool bAllSameClan = true;
	int nCLIDs[MAX_CLANBATTLE_TEAM_MEMBER] = {0, };

	for (int i = 0; i < nMemberCount; i++)
	{
		if (! IsEnabledObject(ppMemberObject[i])) return MERR_CB_NO_TEAM_MEMBER;
		if (!ppMemberObject[i]->GetCharInfo()->m_ClanInfo.IsJoined()) return MERR_CB_WRONG_TEAM_MEMBER;
		if (ppMemberObject[i]->IsLadderChallenging() == true) return MERR_CB_EXIST_CANNOT_CHALLENGE_MEMBER;

		nCLIDs[i] = ppMemberObject[i]->GetCharInfo()->m_ClanInfo.m_nClanID;
	}

	// 같은 클랜인지 체크
	for (int i = 0; i < nMemberCount-1; i++)
	{
		for (int j = i+1; j < nMemberCount; j++)
		{
			if (nCLIDs[i] != nCLIDs[j]) return MERR_CB_WRONG_TEAM_MEMBER;			
		}
	}

	// 모두 같은 채널에 있는지 체크
	MUID uidLastChannel = ppMemberObject[0]->GetChannelUID();
	for (int i = 1; i < nMemberCount; i++)
	{
		if (ppMemberObject[i]->GetChannelUID() != uidLastChannel)
		{
			return MERR_LADDER_EXIST_CANNOT_CHALLENGE_MEMBER;
		}
		uidLastChannel = ppMemberObject[i]->GetChannelUID();
	}



	return MOK;
}

int MClanGameStrategy::ValidateRequestInviteProposal(MMatchObject* pProposerObject, MMatchObject** ppReplierObjects,
													   const int nReplierCount)
{
	int nRet = MERR_UNKNOWN;

	MMatchObject* ppTeamMemberObjects[MAX_REPLIER];
	int nTeamMemberCount = nReplierCount+1;	// 제안자까지..
	if (nTeamMemberCount <= MAX_CLANBATTLE_TEAM_MEMBER)
	{
		ppTeamMemberObjects[0] = pProposerObject;
		for (int i = 0; i < nReplierCount; i++)
		{
			ppTeamMemberObjects[i+1] = ppReplierObjects[i];
		}
		nRet = ValidateChallenge(ppTeamMemberObjects, nTeamMemberCount);
	}
	else 
	{
		nRet = MERR_CB_WRONG_TEAM_MEMBER;
	}

	return nRet;
}

int MClanGameStrategy::GetNewGroupID(MMatchObject* pLeaderObject, MMatchObject** ppMemberObjects, int nMemberCount)
{
	return MMatchServer::GetInstance()->GetLadderMgr()->GenerateID();
}


void MClanGameStrategy::SetLadderGroup(MLadderGroup* pGroup, MMatchObject** ppMemberObjects, int nMemberCount)
{
	if (nMemberCount > 0)
	{
		pGroup->SetCLID(ppMemberObjects[0]->GetCharInfo()->m_ClanInfo.m_nClanID);
	}
}

void MClanGameStrategy::SetStageLadderInfo(MMatchLadderTeamInfo* poutRedLadderInfo, MMatchLadderTeamInfo* poutBlueLadderInfo,
								MLadderGroup* pRedGroup, MLadderGroup* pBlueGroup)
{
	poutRedLadderInfo->nTID = pRedGroup->GetID();
	poutBlueLadderInfo->nTID = pBlueGroup->GetID();

	poutRedLadderInfo->nCLID = pRedGroup->GetCLID();
	poutBlueLadderInfo->nCLID = pBlueGroup->GetCLID();

	poutRedLadderInfo->nFirstMemberCount = (int)pRedGroup->GetLadderType();
	poutBlueLadderInfo->nFirstMemberCount = (int)pBlueGroup->GetLadderType();

	poutRedLadderInfo->nCharLevel = pRedGroup->GetCharLevel();
	poutBlueLadderInfo->nCharLevel = pBlueGroup->GetCharLevel();

	poutRedLadderInfo->nContPoint = pRedGroup->GetContPoint();
	poutBlueLadderInfo->nContPoint = pBlueGroup->GetContPoint();
}


void MClanGameStrategy::SavePointOnFinishGame(MMatchStage* pStage, MMatchTeam nWinnerTeam, bool bIsDrawGame,
		                               MMatchLadderTeamInfo* pRedLadderInfo, MMatchLadderTeamInfo* pBlueLadderInfo)
{
	int nWinnerCLID = 0, nLoserCLID = 0;

	MMatchTeam nLoserTeam = (nWinnerTeam == MMT_RED) ? MMT_BLUE : MMT_RED;

	// red팀 승리
	if (bIsDrawGame == true)
	{
		nWinnerCLID = pRedLadderInfo->nCLID;
		nLoserCLID = pBlueLadderInfo->nCLID;
	}
	else if (nWinnerTeam == MMT_RED)
	{
		nWinnerCLID = pRedLadderInfo->nCLID;
		nLoserCLID = pBlueLadderInfo->nCLID;
	}
	else if (nWinnerTeam == MMT_BLUE)
	{
		nWinnerCLID = pBlueLadderInfo->nCLID;
		nLoserCLID = pRedLadderInfo->nCLID;
	}
	else
	{
		_ASSERT(0);
	}

	if ((nWinnerCLID == 0) || (nLoserCLID == 0)) return;

	MMatchClan* pWinnerClan = MMatchServer::GetInstance()->GetClanMap()->GetClan(nWinnerCLID);
	MMatchClan* pLoserClan = MMatchServer::GetInstance()->GetClanMap()->GetClan(nLoserCLID);

	if ((!pWinnerClan) || (!pLoserClan)) return;

	int nFirstMemberCount = pRedLadderInfo->nFirstMemberCount;


	char szWinnerMembers[512] = "";
	char szLoserMembers[512] = "";
	list<MUID>		WinnerObjUIDs;

	for (auto itor=pStage->GetObjBegin(); itor!=pStage->GetObjEnd(); itor++) 
	{
		MMatchObject* pObj = (MMatchObject*)(*itor).second;
		if (IsEnabledObject(pObj))
		{
			if (pObj->GetTeam() == nWinnerTeam)
			{
				WinnerObjUIDs.push_back(pObj->GetUID());
				strcat_safe(szWinnerMembers, pObj->GetCharInfo()->m_szName);
				strcat_safe(szWinnerMembers, " ");
			}
			else
			{
				strcat_safe(szLoserMembers, pObj->GetCharInfo()->m_szName);
				strcat_safe(szLoserMembers, " ");
			}
		}
	}

	int nMapID = pStage->GetStageSetting()->GetMapIndex();
	int nGameType = (int)pStage->GetStageType();
	int nRoundWins = pStage->GetTeamScore(nWinnerTeam);
	int nRoundLosses = pStage->GetTeamScore(nLoserTeam);

	int nLoserSeriesOfVictories = pLoserClan->GetSeriesOfVictories();
	float fPointRatio = 1.0f;


	if (!bIsDrawGame)
	{
		// MatchMakingSystem 통계 입력
		MLadderStatistics* pLS = MMatchServer::GetInstance()->GetLadderMgr()->GetStatistics();
		pLS->InsertLevelRecord(pRedLadderInfo->nCharLevel, pBlueLadderInfo->nCharLevel, nWinnerTeam);
		pLS->InsertContPointRecord(pRedLadderInfo->nContPoint, pBlueLadderInfo->nContPoint, nWinnerTeam);
		pLS->InsertClanPointRecord(pWinnerClan->GetClanInfoEx()->nPoint, pLoserClan->GetClanInfoEx()->nPoint, MMT_RED);


		// 방송
		int nWinnerSeriesOfVictories = pWinnerClan->GetSeriesOfVictories();

		if (nLoserSeriesOfVictories >= 10)
		{
			MMatchServer::GetInstance()->BroadCastClanInterruptVictories(pWinnerClan->GetName(), pLoserClan->GetName(), 
				nLoserSeriesOfVictories+1);

			fPointRatio = 2.0f;
		}
		else if ((nWinnerSeriesOfVictories == 3) || (nWinnerSeriesOfVictories == 5) || 
			(nWinnerSeriesOfVictories == 7) || (nWinnerSeriesOfVictories >= 10))
		{
			MMatchServer::GetInstance()->BroadCastClanRenewVictories(pWinnerClan->GetName(), pLoserClan->GetName(), 
				nWinnerSeriesOfVictories);
		}
	}

	MMatchServer::GetInstance()->SaveClanPoint(pWinnerClan, pLoserClan, bIsDrawGame,
												nRoundWins, nRoundLosses, nMapID, nGameType,
												nFirstMemberCount, WinnerObjUIDs,
												szWinnerMembers, szLoserMembers, fPointRatio);

}

int MClanGameStrategy::GetRandomMap(int nTeamMember)
{
	int nVecIndex = 0;
	int nMaxSize = 0;
	switch (nTeamMember)
	{
	case 2:
		nVecIndex = MLADDERTYPE_NORMAL_2VS2;
		break;
	case 3:
		nVecIndex = MLADDERTYPE_NORMAL_3VS3;
		break;
	case 4:
		nVecIndex = MLADDERTYPE_NORMAL_4VS4;
		break;
/*	case 8:
		nVecIndex = MLADDERTYPE_NORMAL_8VS8;
		break;*/
	};

	nMaxSize = (int)m_RandomMapVec[nVecIndex].size();
	
	int nRandomMapIndex = 0;
	int nRandomMap=0;

	if (nMaxSize!=0) {
		nRandomMapIndex = rand() % nMaxSize;
		nRandomMap = m_RandomMapVec[nVecIndex][nRandomMapIndex];
	}

	return nRandomMap;
}