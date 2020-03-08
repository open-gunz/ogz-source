#include "stdafx.h"
#include "MMatchServer.h"

#include "MLadderMgr.h"
#include "MLadderPicker.h"


void MLadderPicker::AddTicket(MLadderGroup* pGroup, int nRandomArg)
{
	m_LadderMatchList.push_back( new MLadderTicket(pGroup->GetID(), pGroup->GetScore(), nRandomArg) );
}

void MLadderPicker::AddTicket(MLadderGroup* pGroup, int nClanPoint, int nTickCount, int nRandomArg)
{
	int nID = pGroup->GetID();
	int nCharLevel = pGroup->GetCharLevel();
	int nContPoint = pGroup->GetContPoint();
	m_LadderMatchList.push_back( new MLadderTicket(nID, nCharLevel, nContPoint, nClanPoint, nTickCount, nRandomArg) );
}


static bool CompareLadderTicket(MLadderTicket* left, MLadderTicket* right) 
{
	return left->GetEvaluation() < right->GetEvaluation();
}

void MLadderPicker::Shuffle()
{
	if (m_LadderMatchList.empty()) return;

	m_LadderMatchList.sort(CompareLadderTicket);
}

bool MLadderPicker::PickMatch(int* pGroupA, int* pGroupB)
{
	int nGroupA = 0, nGroupB = 0;

	int nCounter = 0;
	while (true)
	{
		// test
		if (nCounter++ >= 100)
		{
			_ASSERT(0);
			return false;
		}

		if (m_LadderMatchList.size() < 2) return false;

		MLadderTicket* pTicketA = (*m_LadderMatchList.begin());
		nGroupA = pTicketA->GetGroupID();
		m_LadderMatchList.erase(m_LadderMatchList.begin());

		list<MLadderTicket*>::iterator itorMatchedTicket;
		if (Evaluate(pTicketA, &itorMatchedTicket))
		{
			MLadderTicket* pTicketB = (*itorMatchedTicket);
			nGroupB = pTicketB->GetGroupID();
			m_LadderMatchList.erase(itorMatchedTicket);

			if (nGroupA != nGroupB)
			{
				*pGroupA = nGroupA;
				*pGroupB = nGroupB;
				return true;
			}
		}
	}

	return false;
}

#include "MDebug.h"

bool MLadderPicker::EvaluateTicket(MLadderTicket* pTicketA, MLadderTicket* pTicketB, float* poutTotalRate)
{
	int nLevelDiff = abs(pTicketA->GetCharLevel() - pTicketB->GetCharLevel());
	int nClanPointDiff = abs(pTicketA->GetClanPoint() - pTicketB->GetClanPoint());
	int nContPointDiff = abs(pTicketA->GetContPoint() - pTicketB->GetContPoint());

	// TickCount는 0부터 시작
	int nTickCount = min(pTicketA->GetTickCount(), pTicketB->GetTickCount()) - 1;
	
	MLadderStatistics* pLS = MMatchServer::GetInstance()->GetLadderMgr()->GetStatistics();
	float fLevelRate = pLS->GetLevelVictoriesRate(nLevelDiff);
	float fClanPointRate = pLS->GetClanPointVictoriesRate(nClanPointDiff);
	float fContPointRate = pLS->GetContPointVictoriesRate(nContPointDiff);

	float fTotalRate = fLevelRate * 0.6f + fClanPointRate * 0.3f + fContPointRate * 0.1f;
	*poutTotalRate = fTotalRate;

// 두그룹이 모두 5틱이 넘었으면 바로 매칭
#define MAX_LADDER_EVALUATION_TICK		5			

	if (nTickCount >= MAX_LADDER_EVALUATION_TICK) return true;
	

	const float LADDER_EVALUATION_TOTALPOINT_RATE[MAX_LADDER_EVALUATION_TICK] = 
				{ 0.60f, 0.65f, 0.70f, 0.75f, 0.8f };

	const int	LADDER_EVALUATION_LEVEL_DIFF[MAX_LADDER_EVALUATION_TICK] = 
				{ 5, 8, 15, 20, 28 };

	const int	LADDER_EVALUATION_CLANPOINT_DIFF[MAX_LADDER_EVALUATION_TICK] =
				{ 30, 60, 100, 150, 300 };
	
	const int	LADDER_EVALUATION_CONTPOINT_DIFF[MAX_LADDER_EVALUATION_TICK] =
				{ 30, 60, 100, 150, 200 };

	nTickCount = min(0, nTickCount);

	// 캐릭터 레벨이 적당하면 바로 매칭
	if (nLevelDiff <= LADDER_EVALUATION_LEVEL_DIFF[nTickCount]) 
		return true;

	// 토탈비율이 적당하면 매칭
	if (fTotalRate <= LADDER_EVALUATION_TOTALPOINT_RATE[nTickCount]) 
		return true;

	// 3틱부터는 클랜 포인트가 비슷해도 바로 매칭
	if ((nTickCount >= 2) && (nClanPointDiff <= LADDER_EVALUATION_CLANPOINT_DIFF[nTickCount]))
		return true;

	// 4틱부터는 클랜 기여도가 비슷해도 바로 매칭
	if ((nTickCount >= 3) && (nContPointDiff <= LADDER_EVALUATION_CONTPOINT_DIFF[nTickCount]))
		return true;


	return false;
}

bool MLadderPicker::Evaluate(MLadderTicket* pTicket, list<MLadderTicket*>::iterator* poutItorMatchedTicket)
{
	bool bExist = false;	

	float fRealTotalRate = -1.0f;
	MLadderTicket* pRealTarTicket = NULL;

	for (list<MLadderTicket*>::iterator itorTicket = m_LadderMatchList.begin();
			itorTicket != m_LadderMatchList.end(); ++itorTicket)
	{
		MLadderTicket* pTarTicket = *(itorTicket);

		float fTotalRate = 0.0f;
		if (EvaluateTicket(pTicket, pTarTicket, &fTotalRate))
		{
			if (fRealTotalRate <= fTotalRate)
			{
				bExist = true;
				pRealTarTicket = pTarTicket;

				fRealTotalRate = fTotalRate;
				*poutItorMatchedTicket = itorTicket;
			}
			

		}
	}

/*
	if ((bExist) && (pRealTarTicket))
	{
		// 로그를 쌓아서 확인해보장
		{
			int nLevelDiff = abs(pTicket->GetCharLevel() - pRealTarTicket->GetCharLevel());
			int nClanPointDiff = abs(pTicket->GetClanPoint() - pRealTarTicket->GetClanPoint());
			int nContPointDiff = abs(pTicket->GetContPoint() - pRealTarTicket->GetContPoint());

			mlog("Matched: LevelDiff = %2d(%2d, %2d), ClanPointDiff = %4d(%4d, %4d), ContPointDiff = %3d(%3d, %3d), TickCount = (%d, %d)\n",
				nLevelDiff, pTicket->GetCharLevel(), pRealTarTicket->GetCharLevel(),
				nClanPointDiff, pTicket->GetClanPoint(), pRealTarTicket->GetClanPoint(),
				nContPointDiff, pTicket->GetContPoint(), pRealTarTicket->GetContPoint(),
				pTicket->GetTickCount(), pRealTarTicket->GetTickCount());
		}
	}
*/

	return bExist;
}











