#pragma once

#include <list>
#include <map>
#include "MXml.h"

class ZGameTypeConfigData
{
public:
	int		m_nValue;
	char	m_szString[16];
};

typedef std::list< ZGameTypeConfigData* >	MGAMETYPECFGDATA;

class ZGameTypeConfig final
{
public:
	// Round
	MGAMETYPECFGDATA					m_Round;
	int									m_nDefaultRound;

	// Limit time
	MGAMETYPECFGDATA					m_LimitTime;
	int									m_nDefaultLimitTime;

	// Max players
	MGAMETYPECFGDATA					m_MaxPlayers;
	int									m_nDefaultMaxPlayers;



	ZGameTypeConfig();
	~ZGameTypeConfig();

	MGAMETYPECFGDATA GetRoundList()			{ return m_Round; }
	const int GetSizeOfRoundList()			{ return (int)m_Round.size(); }
	const int GetDefaultRound()				{ return m_nDefaultRound; }

	MGAMETYPECFGDATA GetLimitTimeList()		{ return m_LimitTime; }
	const int GetSizeOfLimitTimeList()		{ return (int)m_LimitTime.size(); }
	const int GetDefaultLimitTime()			{ return m_nDefaultLimitTime; }

	MGAMETYPECFGDATA GetMaxPlayersList()	{ return m_MaxPlayers; }
	const int GetSizeOfMaxPlayers()			{ return (int)m_MaxPlayers.size(); }
	const int GetDefaultMaxPlayers()		{ return m_nDefaultMaxPlayers; }
};

typedef std::map< int, ZGameTypeConfig* > MGAMETYPECFG;

class ZGameTypeList final
{
private:
	MGAMETYPECFG	m_GameTypeCfg;

public:
	ZGameTypeList();
	~ZGameTypeList();

	void Clear();

	bool ParseGameTypeList( int nGameTypeNum, MXmlElement& element);

	ZGameTypeConfig* GetGameTypeCfg( int nGameTypeNum);

private:
	bool ParseRound( ZGameTypeConfig* pConfig, MXmlElement& element);
	bool ParseLimitTime( ZGameTypeConfig* pConfig, MXmlElement& element);
	bool ParseMaxPlayers( ZGameTypeConfig* pConfig, MXmlElement& element);
};
