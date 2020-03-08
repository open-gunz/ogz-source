#include "stdafx.h"
#include "MMatchRuleDuel.h"
#include "MMatchTransDataType.h"
#include "MBlobArray.h"
#include "MMatchServer.h"

#include <algorithm>
//////////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////////
// MMatchRuleDuel	   ///////////////////////////////////////////////////////////
MMatchRuleDuel::MMatchRuleDuel(MMatchStage* pStage) : MMatchRule(pStage)
{

}

void MMatchRuleDuel::OnBegin()
{
	uidChampion = MUID(0, 0);
	uidChallenger = MUID(0, 0);

	MMatchStage* pStage = GetStage();

	WaitQueue.clear();	// 대기 큐를 비우고

	if (pStage != NULL)
	{
		for(auto itor=pStage->GetObjBegin(); itor!=pStage->GetObjEnd(); itor++)
			WaitQueue.push_back(itor->first);			// 플레이어들 그냥 몽땅 대기 큐에 넣는다.

//		SpawnPlayers();
	}

	nVictory = 0;

	return;
}

void MMatchRuleDuel::OnEnd()
{
}


void MMatchRuleDuel::OnRoundBegin()
{
	isRoundEnd = false;
	isTimeover = true;

	SpawnPlayers();
	SendQueueInfo(true);
	// 왜그런지 몰라도 옵저버 해야 할 놈이 스폰된걸 봐서 -_- 죽여버린다. 서버에선 스폰안됐으니 더이상의 처리는 필요없을듯.
	// 이거 한 후에 스폰되는거면 낭팬데;
	for (list<MUID>::iterator i = WaitQueue.begin(); i!=WaitQueue.end();  ++i)
		MMatchServer::GetInstance()->OnDuelSetObserver(*i);							

}


void MMatchRuleDuel::OnRoundEnd()
{
	if (isTimeover)
	{	
		WaitQueue.push_back(uidChampion);
		WaitQueue.push_back(uidChallenger);
		uidChampion = uidChallenger = MUID(0, 0);
		nVictory = 0;
	}
	else
	{
		if (isChangeChampion || uidChampion == MUID(0, 0))				// 챔피온이 바뀌어야 하면 일단 챔피온과 도전자를 스왑
		{
			MUID uidTemp;
			uidTemp = uidChampion;
			uidChampion = uidChallenger;
			uidChallenger = uidTemp;
		}

		if (uidChallenger != MUID(0, 0))
		{
			WaitQueue.push_back(uidChallenger);	// 도전자는 큐의 맨 뒤로 밀어넣고
			uidChallenger = MUID(0, 0);			// 도전자의 id를 무효화
		}
	}

//	SpawnPlayers();
	LogInfo();
}

bool MMatchRuleDuel::RoundCount() 
{
	if (m_pStage == NULL) return false;

	int nTotalRound = m_pStage->GetStageSetting()->GetRoundMax();
	m_nRoundCount++;

	MMatchStage* pStage = GetStage();
	for (auto i=pStage->GetObjBegin(); i!=pStage->GetObjEnd(); i++)
	{
		MMatchObject* pObj = i->second;
		if (pObj->GetEnterBattle() == false) continue;

		if (pObj->GetAllRoundKillCount() >= (unsigned int)pStage->GetStageSetting()->GetRoundMax())
		{
			return false;
		}
	}

	return true;
}

bool MMatchRuleDuel::OnCheckRoundFinish()
{
	if (!isRoundEnd)
		return false;
	else
	{
		isRoundEnd = false;
		isTimeover = false;
		return true;	
	}
}

void MMatchRuleDuel::OnRoundTimeOut()
{
	SetRoundArg(MMATCH_ROUNDRESULT_DRAW);
}


void MMatchRuleDuel::OnGameKill(const MUID& uidAttacker, const MUID& uidVictim)
{
	isRoundEnd = true;

	MUID chanID = MMatchServer::GetInstance()->GetChannelMap()->Find(m_pStage->GetOwnerChannel())->GetUID();

	if (uidVictim == uidChallenger)		// 챔피온이 잡았으면 챔피온 유지
	{
		isChangeChampion = false;
		nVictory++;

		if (m_pStage == NULL) return;
		if (m_pStage->IsPrivate()) return;		// 비밀방이면 방송 패스

		if (nVictory % 10 != 0) return;			// 연승수가 10의 배수일때만

		MMatchObject* pChamp;
		pChamp = m_pStage->GetObj(uidChampion);
		if (pChamp == NULL) return;


		MMatchServer::GetInstance()->BroadCastDuelRenewVictories(
			chanID,
			pChamp->GetName(), 
			MMatchServer::GetInstance()->GetChannelMap()->Find(m_pStage->GetOwnerChannel())->GetName(), 
			m_pStage->GetIndex()+1,
			nVictory
			);
	}
	else
	{
		isChangeChampion = true;

		int nowVictory = nVictory;

		nVictory = 1;

		if (nowVictory < 10) return;				// 10연승 이상을 저지했을때만
		if (m_pStage == NULL) return;
		if (m_pStage->IsPrivate()) return;		// 비밀방이면 방송 패스
	
		MMatchObject* pChamp, *pChallenger;
		pChamp = m_pStage->GetObj(uidChampion);
		if (pChamp == NULL) return;
		pChallenger = m_pStage->GetObj(uidChallenger);
		if (pChallenger == NULL) return;

		if (strcmp(m_pStage->GetPassword(), "") != 0) return;

		MMatchServer::GetInstance()->BroadCastDuelInterruptVictories(
			chanID,
			pChamp->GetName(),
			pChallenger->GetName(),
			nowVictory
			);
	}


	LogInfo();

}

void MMatchRuleDuel::OnEnterBattle(MUID& uidChar)
{
	MMatchObject* pChar;
	pChar = m_pStage->GetObj(uidChar);
	if (pChar->GetTeam() == MMT_SPECTATOR)
		return;

	if ((uidChar != uidChampion) && (uidChar != uidChallenger) && (find(WaitQueue.begin(), WaitQueue.end(), uidChar) == WaitQueue.end()))
	{
		WaitQueue.push_back(uidChar);
		SpawnPlayers();
	}
	SendQueueInfo();
	LogInfo();
}

void MMatchRuleDuel::OnLeaveBattle(MUID& uidChar)
{
	if (uidChar == uidChampion)
	{
		isChangeChampion = true;
		isRoundEnd = true;
		uidChampion = MUID(0, 0);
		nVictory = 0;
	}
	else if (uidChar == uidChallenger)
	{
		isChangeChampion = false;
		isRoundEnd = true;
		uidChallenger = MUID(0, 0);
	}
	else
	{
		WaitQueue.remove(uidChar);
		SendQueueInfo();
		LogInfo();
	}
}

void MMatchRuleDuel::OnTeam(const MUID &uidPlayer, MMatchTeam nTeam)
{
	if (nTeam != MMT_SPECTATOR)
		return;

	auto it = std::find(WaitQueue.begin(), WaitQueue.end(), uidPlayer);

	if (it == WaitQueue.end())
		return;

	WaitQueue.remove(uidPlayer);
}

void MMatchRuleDuel::SpawnPlayers()
{
	if (uidChampion == MUID(0, 0))
	{
		if (!WaitQueue.empty())
		{
			uidChampion = WaitQueue.front();
			WaitQueue.pop_front();
		}
	}
	if (uidChallenger == MUID(0, 0))
	{
		if (!WaitQueue.empty())
		{
			uidChallenger = WaitQueue.front();
			WaitQueue.pop_front();
		}
	}

	for (MUID UID : {uidChampion, uidChallenger})
	{
		auto Object = MGetMatchServer()->GetObject(UID);
		if (!Object || !(Object->GetPlayerFlags() & MTD_PlayerFlags_Bot)) continue;
		Object->SetAlive(true);
	}
}

bool MMatchRuleDuel::OnCheckEnableBattleCondition()
{
	if (uidChampion == MUID(0, 0) || uidChallenger == MUID(0, 0))
	{
		if (WaitQueue.empty())
			return false;
		else
			isRoundEnd = true;
	}

	return true;
}

void MMatchRuleDuel::LogInfo()
{
	if (m_pStage == NULL) return;
	MMatchObject* pObj;
	char buf[250];
	sprintf_safe(buf, "Logging Que--------------------\n");
	DMLog(buf);

	pObj = m_pStage->GetObj(uidChampion);
	if (pObj != NULL)
	{
		sprintf_safe(buf, "Champion name : %s \n", pObj->GetName());
		DMLog(buf);
	}

	pObj = m_pStage->GetObj(uidChallenger);
	if (pObj != NULL)
	{
		sprintf_safe(buf, "Challenger name : %s \n", pObj->GetName());
		DMLog(buf);
	}

	int x = 0;
	for (list<MUID>::iterator i = WaitQueue.begin(); i!=WaitQueue.end();  ++i)
	{
		pObj = m_pStage->GetObj(*i);
		if (pObj != NULL)
		{
			sprintf_safe(buf, "Wait Queue #%d : %s \n", x, pObj->GetName());
			DMLog(buf);		
			x++;
		}
	}
}

void MMatchRuleDuel::SendQueueInfo(bool isRoundEnd)
{
	if (m_pStage == NULL) return;
	MTD_DuelQueueInfo QInfo;
	QInfo.m_uidChampion = uidChampion;
	QInfo.m_uidChallenger = uidChallenger;
	QInfo.m_nQueueLength = static_cast<char>(WaitQueue.size());
	QInfo.m_nVictory = nVictory;
	QInfo.m_bIsRoundEnd = isRoundEnd;

	int i=0;
	for (list<MUID>::iterator iter = WaitQueue.begin(); iter != WaitQueue.end(); ++iter, ++i)
		QInfo.m_WaitQueue[i] = *iter;

	MMatchServer::GetInstance()->OnDuelQueueInfo(m_pStage->GetUID(), QInfo);
}