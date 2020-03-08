#ifndef _MLADDERPICKER_H
#define _MLADDERPICKER_H

#pragma once

#include <list>
#include <queue>
#include <algorithm>
using namespace std;



class MLadderTicket {
private: 
	int		m_nGroupID;
	int		m_nScore;
	int		m_nRandomArg;

	// 추가
	int		m_nCharLevel;		// 캐릭터 레벨 평균
	int		m_nClanPoint;		// 클랜 포인트
	int		m_nContPoint;		// 클랜 기여도 평균

	int		m_nTickCount;
public:
	// 예전 액션리그 시절의 유물
	MLadderTicket(int nGroupID, int nScore, int nRandomArg) 
	{
		m_nGroupID = nGroupID;
		m_nScore = nScore;
		m_nRandomArg = nRandomArg;

		m_nCharLevel = 0;
		m_nClanPoint = 0;
		m_nContPoint = 0;

		m_nTickCount = 0;
	}

	MLadderTicket(int nGroupID, int nCharLevel, int nContPoint, int nClanPoint, int nTickCount, int nRandomArg)
	{
		m_nGroupID = nGroupID;
		m_nCharLevel = nCharLevel;
		m_nContPoint = nContPoint;
		m_nClanPoint = nClanPoint;
		m_nTickCount = nTickCount;

		m_nScore = 0;
		m_nRandomArg = nRandomArg;
	}
	int GetGroupID()	{ return m_nGroupID; }
	int GetEvaluation()	{ return m_nScore + m_nRandomArg; }

	int GetCharLevel()	{ return m_nCharLevel; }
	int GetClanPoint()	{ return m_nClanPoint; }
	int GetContPoint()	{ return m_nContPoint; }
	int GetTickCount()	{ return m_nTickCount; }
};

class MLadderPicker {
protected:
	list<MLadderTicket*>		m_LadderMatchList;

	bool Evaluate(MLadderTicket* pTicket, list<MLadderTicket*>::iterator* poutItorMatchedTicket);
	bool EvaluateTicket(MLadderTicket* pTicketA, MLadderTicket* pTicketB, float* poutTotalRate);
public:
	void AddTicket(MLadderGroup* pGroup, int nRandomArg);	// 예전 액션리그 시절의 유물
	void AddTicket(MLadderGroup* pGroup, int nClanPoint, int nTickCount, int nRandomArg);

	bool PickMatch(int* pGroupA, int* pGroupB);
	void Shuffle();
};


#endif