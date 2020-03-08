#ifndef _ZSCORELISTBOX_H
#define _ZSCORELISTBOX_H

#include <stdio.h>
#include "MWidget.h"
#include "MListBox.h"
#include "MFrame.h"

class ZListItemScore;

class ZScoreListBox : public MListBox
{
private:
protected:
	virtual bool OnEvent(MEvent* pEvent, MListener* pListener)
	{
		return false;
	}
public:
	ZScoreListBox(const char* szName, MWidget* pParent=NULL, MListener* pListener=NULL);
	virtual ~ZScoreListBox() { }
};


class ZListItemScore : public MListItem
{
protected:
	char	m_szTeam[256];
	char	m_szID[256];
	char	m_szState[256];
	int		m_nScore;
	int		m_nKills;
	int		m_nDeaths;
	int		m_nPing;
public:
	ZListItemScore(const char* szTeam, const char* szID, const char* szState, 
		int nScore, int nKills, int nDeaths, int nPing)
	{
		strcpy_safe(m_szTeam, szTeam);
		strcpy_safe(m_szID, szID);
		strcpy_safe(m_szState, szState);
		m_nScore = nScore;
		m_nKills = nKills;
		m_nDeaths = nDeaths;
		m_nPing = nPing;
	}
	virtual const char* GetString() { return m_szID; }
	virtual const char* GetString(int i) 
	{
		static char szTemp[256];
		switch (i)
		{
		case 0:
			return m_szTeam;
			break;
		case 1:
			return m_szID;
			break;
		case 2:
			return m_szState;
			break;
		case 3:
			sprintf_safe(szTemp, "%d", m_nScore);
			return szTemp;
			break;
		case 4:
			sprintf_safe(szTemp, "%d", m_nKills);
			return szTemp;
			break;
		case 5:
			sprintf_safe(szTemp, "%d", m_nDeaths);
			return szTemp;
			break;
		case 6:
			sprintf_safe(szTemp, "%d", m_nPing);
			return szTemp;
			break;
		}

		return NULL;
	}
};

class ZScoreBoardFrame : public MFrame
{
private:
protected:
	virtual bool OnEvent(MEvent* pEvent, MListener* pListener)
	{
		return false;
	}
public:
	ZScoreBoardFrame(const char* szName, MWidget* pParent=NULL, MListener* pListener=NULL)
		: MFrame(szName, pParent, pListener)
	{

	}
	virtual ~ZScoreBoardFrame() { }
};
#endif