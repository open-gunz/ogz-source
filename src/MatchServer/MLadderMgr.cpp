#include "stdafx.h"
#include "MMatchServer.h"

#include "MLadderMgr.h"
#include "MLadderPicker.h"
#include "MMatchGlobal.h"
#include "MThread.h"
#include "MSharedCommandTable.h"
#include "MMatchUtil.h"


MLadderGroupMap* MLadderMgr::GetWaitGroupContainer(MLADDERTYPE nLadderType)
{
	if ((nLadderType < 0) || (nLadderType >= MLADDERTYPE_MAX))
	{
		_ASSERT("UNKNOWN LADDERMAP");
		return NULL;
	}

	return &m_WaitingMaps[(int)nLadderType];
}

MLadderGroup* MLadderMgr::CreateLadderGroup()
{
	return new MLadderGroup(MMatchServer::GetInstance()->GetTickTime());
}

MLadderGroup* MLadderMgr::FindLadderGroup(int nGroupID)
{
	MLadderGroup* pGroup = NULL;

	for (int i = 0; i < MLADDERTYPE_MAX; i++)
	{
		if (pGroup=m_WaitingMaps[i].Find(nGroupID))
			return pGroup;
	}
	return NULL;
}

////////////////////////////////////////////////////////////////////////////////////
bool MLadderMgr::Init()
{
	m_Stat.Init();

	return true;
}

void MLadderMgr::AddGroup(MLADDERTYPE nLadderType, MLadderGroup* pGroup)
{
	pGroup->SetLadderType(nLadderType);

	MLadderGroupMap* pGroupMap = GetWaitGroupContainer(nLadderType);
	if (pGroupMap == NULL) return;

	pGroupMap->Add(pGroup);
	m_GroupList.push_back(pGroup);

	// Ladder 상대 찾는중 알림(for UI)
	for (list<MUID>::iterator i=pGroup->GetPlayerListBegin(); i!= pGroup->GetPlayerListEnd(); i++)
	{
		MUID uidPlayer = (*i);
		MCommand* pCmd = MMatchServer::GetInstance()->CreateCommand(MC_MATCH_LADDER_SEARCH_RIVAL, uidPlayer);
		
		MMatchObject* pObj = MMatchServer::GetInstance()->GetObject(uidPlayer);
		if (!IsEnabledObject(pObj))
		{
			delete pCmd;
			continue;
		}

		MMatchServer::GetInstance()->RouteToListener(pObj, pCmd);
	}	
}

bool MLadderMgr::Challenge(MLadderGroup* pGroup)
{
	int nPlayerCount = (int)pGroup->GetPlayerCount();

	if (nPlayerCount > 0)
	{
		for (int i = 0; i < MLADDERTYPE_MAX; i++)
		{
			if (nPlayerCount == GetNeedMemberCount(MLADDERTYPE(i)))
			{
				AddGroup(MLADDERTYPE(i), pGroup);
			}
		}

		//AddGroup(MLADDERTYPE(nPlayerCount-1), pGroup);
		return true;
	}

	_ASSERT("UNKNOWN LADDERTYPE");

	return false;
}

void MLadderMgr::RemoveFromGroupList(MLadderGroup* pGroup)
{
	if (pGroup)
	{
		m_GroupList.remove(pGroup);
	}
}

void MLadderMgr::CancelChallenge(int nGroupID, const char* pszCancelName)
{
	MLadderGroup* pGroup = FindLadderGroup(nGroupID);
	if (pGroup == NULL) return;
	MLadderGroupMap* pGroupMap = GetWaitGroupContainer((MLADDERTYPE)pGroup->GetLadderType());
	if (pGroupMap == NULL) return;

	for (list<MUID>::iterator i=pGroup->GetPlayerListBegin(); i!= pGroup->GetPlayerListEnd(); i++)
	{
		MUID uidMember = (*i);

		MMatchObject* pMemberObject = MMatchServer::GetInstance()->GetObject(uidMember);
		if (!IsEnabledObject(pMemberObject)) continue;
		pMemberObject->SetLadderChallenging(false);
		pMemberObject->SetLadderGroupID(0);

		MCommand* pCmd = MMatchServer::GetInstance()->CreateCommand(MC_MATCH_LADDER_CANCEL_CHALLENGE, uidMember);
		pCmd->AddParameter(new MCmdParamStr(pszCancelName));

		MMatchObject* pObj = MMatchServer::GetInstance()->GetObject(uidMember);
		if (!IsEnabledObject(pObj))
		{
			delete pCmd;
			continue;
		}

		MMatchServer::GetInstance()->RouteToListener(pObj, pCmd);
	}
	pGroupMap->Remove(pGroup->GetID());
	RemoveFromGroupList(pGroup);
	delete pGroup;
}

int MLadderMgr::MakeMatch(MLADDERTYPE nLadderType)
{
	MLadderGroupMap* pWaitGroupMap = GetWaitGroupContainer(nLadderType);
	if (pWaitGroupMap == NULL) return 0;

	MTime time;
	MLadderPicker	ladderPicker;

	for (MLadderGroupMap::iterator i=pWaitGroupMap->begin();
		 i!=pWaitGroupMap->end(); i++) 
	{
		MLadderGroup* pGroup = (*i).second;
		pGroup->UpdateTick();

/*
#ifdef _DEBUG
		const unsigned int MIN_ADDTICKET_TICK = 3000;
		if (MGetTimeDistance(MMatchServer::GetInstance()->GetTickTime(), pGroup->GetRegTime()) < MIN_ADDTICKET_TICK) continue;
#endif
*/

		//ladderPicker.AddTicket( pGroup, time.MakeNumber(0,pGroup->GetScore()) );
		
		int nClanPoint = DEFAULT_CLAN_POINT;
		MMatchClan* pClan = MMatchServer::GetInstance()->GetClanMap()->GetClan(pGroup->GetCLID());
		if (pClan)
		{
			nClanPoint = pClan->GetClanInfoEx()->nPoint;
		}

		const int MAX_RANDOM_ARG = 100;

		ladderPicker.AddTicket(pGroup, nClanPoint, pGroup->GetTickCount(), time.MakeNumber(0, MAX_RANDOM_ARG));
	}

	ladderPicker.Shuffle();

	int nLaunchCount = 0;
	while(true) {
		int nGroupA = 0;
		int nGroupB = 0;
		if (ladderPicker.PickMatch(&nGroupA, &nGroupB) == false)
			break;
		LaunchLadder(nLadderType, nGroupA, nGroupB);
		nLaunchCount++;
	}
	return nLaunchCount;
}

void MLadderMgr::CleaningGarbages()
{
	for (int i = 0; i < MLADDERTYPE_MAX; i++)
	{
		MLADDERTYPE nLadderType = MLADDERTYPE(i);

		MLadderGroupMap* pWaitGroupMap = GetWaitGroupContainer(nLadderType);
		if (pWaitGroupMap == NULL) continue;

		list<int>		CancelGroupIDList;

		for (MLadderGroupMap::iterator itorGroup = pWaitGroupMap->begin(); itorGroup!=pWaitGroupMap->end(); itorGroup++) 
		{
			MLadderGroup* pGroup = (*itorGroup).second;

			// 한명이라도 접속을 끊었으면 Cancel한다.
			bool bExistCannotPlayer = false;
			for (list<MUID>::iterator itorPlayerUID=pGroup->GetPlayerListBegin(); itorPlayerUID!= pGroup->GetPlayerListEnd(); 
				itorPlayerUID++)
			{
				MUID uidMember = (*itorPlayerUID);

				MMatchObject* pMemberObject = MMatchServer::GetInstance()->GetObject(uidMember);
				if (!IsEnabledObject(pMemberObject))
				{
					bExistCannotPlayer = true;
					break;
				}
			}

			if (bExistCannotPlayer)
			{
				CancelGroupIDList.push_back(pGroup->GetID());
			}
		}

		// 상대팀 대기중인 플레이어의 접속이 끊어져서 Cancel해야 되는 그룹은 Cancel해준다.
		for (list<int>::iterator itorGroupID = CancelGroupIDList.begin(); itorGroupID != CancelGroupIDList.end(); 
			++itorGroupID)
		{
			CancelChallenge((*itorGroupID), "");
		}
	}
}

void MLadderMgr::LaunchLadder(MLADDERTYPE nLadderType, int nGroupA, int nGroupB)
{
#ifdef _DEBUG
	//char szLog[128];
	//sprintf_safe(szLog, "Team(%d) vs Group(%d) LADDER LAUNCHED \n", nGroupA, nGroupB);
	//OutputDebugString(szLog);
#endif 

	MLadderGroupMap* pGroupMap = GetWaitGroupContainer(nLadderType);
	if (pGroupMap == NULL) return;

	MLadderGroup* pGroupA = pGroupMap->Find(nGroupA);
	MLadderGroup* pGroupB = pGroupMap->Find(nGroupB);

	// 만약 같은 클랜이거나 같은 그룹이면 런치가 안된다.
	if ((pGroupA != NULL) && (pGroupB != NULL) && (pGroupA->IsSameGroup(pGroupB))) return;

	pGroupMap->Remove(nGroupA);
	pGroupMap->Remove(nGroupB);

	RemoveFromGroupList(pGroupA);
	RemoveFromGroupList(pGroupB);

	if ((pGroupA == NULL) || (pGroupB == NULL)) {
		DMLog("LADDER 불발 \n");
		return;
	}

	MMatchServer* pServer = MMatchServer::GetInstance();
	pServer->LadderGameLaunch(pGroupA, pGroupB);
}

#define MTIME_LADDER_DEFAULT_TICKINTERVAL		10000			// 기본틱 10초

u32 MLadderMgr::GetTickInterval()
{
	u32 nDefaultTickInterval = MTIME_LADDER_DEFAULT_TICKINTERVAL;

	// 동접에 따라 틱시간을 가변으로 한다.
	int nObjSize = (int)MMatchServer::GetInstance()->GetObjects()->size();
	
	if (nObjSize < 50)
	{
		nDefaultTickInterval = 5000;		// 50명미만은 5초
	}
	else if ((nObjSize>=50) && (nObjSize < 150))
	{
		nDefaultTickInterval = 7000;		// 150명 미만은 7초
	}
	else if ((nObjSize>=150) && (nObjSize < 300))
	{
		nDefaultTickInterval = 9000;		// 300명 미만은 9초
	}
	return nDefaultTickInterval;
}


void MLadderMgr::Tick(u64 nTick)
{
	if (nTick - GetLastTick() < GetTickInterval())
		return;
	else
		SetLastTick(nTick);

	CleaningGarbages();

	for (int i = 0; i < MLADDERTYPE_MAX; i++)
	{
		MakeMatch(MLADDERTYPE(i));
	}


	m_Stat.Tick(nTick);
}

int MLadderMgr::GetNeedMemberCount(MLADDERTYPE nLadderType)
{
	if ((nLadderType >= 0) && (nLadderType < MLADDERTYPE_MAX))
	{
		return g_nNeedLadderMemberCount[(int)nLadderType];
	}

	return -1;
}

int MLadderMgr::GetTotalGroupCount()
{
	int ret = 0;
	for (int i = 0; i < MLADDERTYPE_MAX; i++)
	{
		ret += (int)m_WaitingMaps[i].size();
	}
	return ret;
}

u32 MLadderMgr::GetChecksum(int nFirstIndex, int nGroupCount)
{
	u32 nGroupListChecksum = 0;

	list<MLadderGroup*>::iterator itorGroup = m_GroupList.begin();
	for (int i = 0; i < nFirstIndex; i++, itorGroup++)
	{
		if (itorGroup == m_GroupList.end()) return 0;
	}
	
	for (int i = 0; i < nGroupCount; i++)
	{
		if (itorGroup == m_GroupList.end()) return nGroupListChecksum;

		MLadderGroup* pGroup = (*itorGroup);
		nGroupListChecksum += pGroup->GetChecksum();
		itorGroup++;
	}

	return nGroupListChecksum;

}

void MLadderMgr::DebugTest()
{
#ifdef _DEBUG
	DMLog("Ahehheh \n");

	MMatchServer* pServer = MMatchServer::GetInstance();

	MMatchObject* pPlayer1 = pServer->GetPlayerByName("라온마이");
	if (pPlayer1 == NULL) return;
	MMatchObject* pPlayer2 = pServer->GetPlayerByName("라온하제2");
	if (pPlayer2 == NULL) return;


	// Ensure All Player Not in LadderGroup
	MLadderGroup* pGroupA = CreateLadderGroup();
	pGroupA->AddPlayer(pPlayer1);
	MLadderGroup* pGroupB = CreateLadderGroup();
	pGroupB->AddPlayer(pPlayer2);

//	if (ValidateLadderGroup(pGroup))
	Challenge(pGroupA);
	Challenge(pGroupB);

#endif
}
