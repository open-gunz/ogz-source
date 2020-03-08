#ifndef _ZVOTEINTERFACE_H
#define _ZVOTEINTERFACE_H

//#pragma once

#include <string>
#include <vector>
using namespace std;

class MDrawContext;


class ZVoteInterface {
protected:
	bool			m_bShowTargelist;
	char			m_szDiscuss[128];
	vector<string>	m_TargetList;

protected:
	const char* GetDiscuss()				{ return m_szDiscuss; }
	void SetDiscuss(const char* pszDiscuss)	{ strcpy_safe(m_szDiscuss, pszDiscuss); }

	int ConvKeyToIndex(char nChar);
	char ConvIndexToKey(int nIndex);
	
	bool OnVoteRun(int nTargetIndex);

public:
	ZVoteInterface()					{ Clear(); }
	~ZVoteInterface()					{ Clear(); }

	bool GetShowTargetList()		{ return m_bShowTargelist; }
	void ShowTargetList(bool bVal)	{ m_bShowTargelist = bVal; }

	void Clear();
	void CallVote(const char* pszDiscuss);

	void VoteInput(char nKey);
	void CancelVote();

	void DrawVoteTargetlist(MDrawContext* pDC);
	void DrawVoteMessage(MDrawContext* pDC);
};


#endif