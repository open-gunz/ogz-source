#pragma once

#include <string>
#include <list>
#include <set>

#include "MMatchMap.h"
#include "MMatchGlobal.h"
#include "IDatabase.h"

class MMatchConfig
{
private:
	char Server[256];
	char Database[256];
	char				m_szDB_DNS[64];
	char				m_szDB_UserName[64];
	char				m_szDB_Password[64];

	int					m_nMaxUser;
	int					m_nServerID;
	char				m_szServerName[256];

	MMatchServerMode	m_nServerMode;
	bool				m_bRestrictionMap;
	std::set<int>			m_EnableMaps;
	std::list<std::string>		m_FreeLoginIPList;
	bool				m_bCheckPremiumIP;
	std::string				m_strCountry;
	std::string				m_strLanguage;

	bool				m_bEnabledCreateLadderGame;

	char				m_NJ_szDBAgentIP[64];
	int					m_NJ_nDBAgentPort;
	int					m_NJ_nGameCode;

	// filter
	bool				m_bUseFilter;
	bool				m_bAcceptInvalidIP;

	// environment.
	bool				m_bIsUseEvent;
	bool				m_bIsUseFileCrc;

	// debug.
	std::list<std::string>		m_DebugLoginIPList;
	bool				m_bIsDebugServer;

	// keeper ip.
	std::string				m_strKeeperIP;

	std::string GameDirectory = "";
	bool bIsMasterServer = true;
	DatabaseType DBType = DatabaseType::SQLite;

	bool				m_bIsComplete;

	void AddFreeLoginIP(const char* szIP);
	void AddDebugLoginIP( const char* szIP );
	void TrimStr(const char* szSrcStr, char* outStr, int maxlen);

public:
	MMatchConfig();
	bool Create();
	void Destroy();
	void Clear();

	const int GetMaxUser()							{ return m_nMaxUser; }
	const int GetServerID()							{ return m_nServerID; }
	const char* GetServerName()						{ return m_szServerName; }
	const MMatchServerMode		GetServerMode()		{ return m_nServerMode; }
	bool IsResMap()									{ return m_bRestrictionMap; }
	bool IsEnableMap(const MMATCH_MAP nMap)
	{
		if (!m_bRestrictionMap) return true;
		if (m_EnableMaps.find(nMap) != m_EnableMaps.end()) return true;
		return false;
	}
	const bool IsDebugServer()							{ return m_bIsDebugServer; }
	bool CheckFreeLoginIPList(const char* pszIP);
	bool IsDebugLoginIPList( const char* pszIP );
	bool CheckPremiumIP()							{ return m_bCheckPremiumIP; }

	bool IsEnabledCreateLadderGame()				{ return m_bEnabledCreateLadderGame; }
	void SetEnabledCreateLadderGame(bool bVal)		{ m_bEnabledCreateLadderGame = bVal; }

	const char* GetNJDBAgentIP()					{ return m_NJ_szDBAgentIP; }
	int GetNJDBAgentPort()							{ return m_NJ_nDBAgentPort; }
	int GetNJDBAgentGameCode()						{ return m_NJ_nGameCode; }

	const bool IsUseFilter() const						{ return m_bUseFilter; }
	const bool IsAcceptInvalidIP() const				{ return m_bAcceptInvalidIP; }
	void SetUseFilterState( const bool bUse )			{ m_bUseFilter = bUse; }
	void SetAcceptInvalidIPState( const bool bAccept )	{ m_bAcceptInvalidIP = bAccept; }

	const bool IsUseEvent() const	{ return m_bIsUseEvent; }
	const bool IsUseFileCrc() const { return m_bIsUseFileCrc; }

	bool IsKeeperIP( const std::string& strIP )				{ return m_strKeeperIP == strIP; }

	const std::string& GetKeeperIP() { return m_strKeeperIP; }

	const std::string& GetCountry()	{ return m_strCountry; }
	const std::string& GetLanguage() { return m_strLanguage; }

	const bool IsComplete() { return m_bIsComplete; }

	const char* GetGameDirectory() const { return GameDirectory.c_str(); }
	bool HasGameData() const { return !GameDirectory.empty(); }

	bool IsMasterServer() const { return bIsMasterServer; }
	auto GetPort() const { return 6000; }
	auto GetDatabaseType() const { return DBType; }

	struct VersionType {
		u32 Major, Minor, Patch, Revision;
	} Version;

	bool VersionChecking = true;
};

inline MMatchConfig* MGetServerConfig()
{
	static MMatchConfig m_MatchConfig;
	return &m_MatchConfig;
}

inline bool QuestTestServer() { return (MGetServerConfig()->GetServerMode() == MSM_TEST); }


#define SERVER_CONFIG_FILENAME			"./server.ini"

#define SERVER_CONFIG_DEFAULT_NJ_DBAGENT_IP			"210.174.197.180"
#define SERVER_CONFIG_DEFAULT_NJ_DBAGENT_PORT		7500
#define SERVER_CONFIG_DEFAULT_NJ_DBAGENT_GAMECODE	1013

#define SERVER_CONFIG_DEFAULT_USE_EVENT		1
#define SERVER_CONFIG_DEFAULT_USE_FILECRC	0

#define SERVER_CONFIG_DEBUG_DEFAULT			0
