#include "stdafx.h"
#include "MMatchConfig.h"
#include "MMatchMap.h"
#include "MLex.h"
#include "MZFileSystem.h"
#include "MErrorTable.h"
#include "MMatchServer.h"
#include "rapidxml.hpp"
#include <fstream>
#include "RGVersion.h"
#include "IniParser.h"

MMatchConfig::MMatchConfig()
{
	m_nMaxUser					= 0;
	m_szDB_DNS[0]				= '\0';
	m_szDB_UserName[0]			= '\0';
	m_szDB_Password[0]			= '\0';
	m_nServerID					= 0;
	m_szServerName[0]			= '\0';
	m_nServerMode				= MSM_NORMAL_;
	m_bRestrictionMap			= false;
	m_bCheckPremiumIP			= false;
	m_bUseFilter				= false;
	m_bAcceptInvalidIP			= false;
	m_bIsDebugServer			= false;
	m_bEnabledCreateLadderGame	= true;
	m_bIsComplete				= false;

	Version.Major = RGUNZ_VERSION_MAJOR;
	Version.Minor = RGUNZ_VERSION_MINOR;
	Version.Patch = RGUNZ_VERSION_PATCH;
	Version.Revision = RGUNZ_VERSION_REVISION;
}

template <typename T>
static bool SetEnum(const IniParser& ini, T& Dest, const char* Section, const char* Name,
	T Max = T::Max)
{
	auto Value = ini.GetString(Section, Name, ToString(T(0)));
	auto Index = [&] {
		for (int i = 0; i < int(Max); ++i)
			if (iequals(ToString(T(i)), Value))
				return i;
		return -1;
	}();
	if (Index == -1)
	{
		MLog("Invalid value for config option [%s] %s = %.*s\n",
			Section, Name, Value.size(), Value.data());
		return false;
	}
	Dest = T(Index);
	return true;
}


bool MMatchConfig::Create()
{
	IniParser ini;
	if (!ini.Parse(SERVER_CONFIG_FILENAME))
		return false;

	m_nMaxUser = ini.GetInt("SERVER", "MAXUSER", 1500);
	m_nServerID = ini.GetInt("SERVER", "SERVERID", 0);
	strcpy_safe(m_szServerName, ini.GetString("SERVER", "SERVERNAME", "matchserver"));

	if (!SetEnum(ini, m_nServerMode, "SERVER", "MODE", MSM_MAX))
		return false;

	auto AddIPs = [&](auto& Container, const char* Name) {
		Split(ini.GetString("SERVER", Name, ""), " ", [&](StringView Str) {
			Container.push_back(Str.str());
		});
	};

	AddIPs(m_FreeLoginIPList, "FREELOGINIP");
	AddIPs(m_DebugLoginIPList, "DEBUGIP");

	m_strKeeperIP = ini.GetString("SERVER", "KEEPERIP", "").str();

	m_bIsDebugServer = bool(ini.GetInt("SERVER", "DEBUG", SERVER_CONFIG_DEBUG_DEFAULT));
	m_bCheckPremiumIP = bool(ini.GetInt("SERVER", "CheckPremiumIP", 0));

	auto DefaultCountry = "BRZ";
	m_strCountry = ini.GetString("SERVER", "COUNTRY", DefaultCountry).str();
	m_strLanguage = ini.GetString("SERVER", "LANGUAGE", DefaultCountry).str();

	int nMapCount = 0;

	Split(ini.GetString("SERVER", "EnableMap", ""), ";", [&](StringView token) {
		token = trim(token);

		auto it = std::find_if(std::begin(g_MapDesc), std::end(g_MapDesc), [&](auto& x) {
			return iequals(token, x.szMapName);
		});
		if (it != std::end(g_MapDesc))
		{
			nMapCount++;
			m_EnableMaps.insert(set<int>::value_type(it - std::begin(g_MapDesc)));
		}
	});

	if (nMapCount <= 0)
	{
		for (int i = 0; i < MMATCH_MAP_MAX; i++) m_EnableMaps.insert(set<int>::value_type(i));
		m_bRestrictionMap = false;
	}
	else
	{
		m_bRestrictionMap = true;
	}

	GameDirectory = ini.GetString("SERVER", "game_dir", "").str();
	bIsMasterServer = ini.GetInt<bool>("SERVER", "is_master_server", true);

	if (!SetEnum(ini, DBType, "DB", "database_type"))
		return false;

	strcpy_safe(m_NJ_szDBAgentIP, ini.GetString("LOCALE", "DBAgentIP",
		SERVER_CONFIG_DEFAULT_NJ_DBAGENT_IP));
	m_NJ_nDBAgentPort = ini.GetInt("LOCALE", "DBAgentPort", SERVER_CONFIG_DEFAULT_NJ_DBAGENT_PORT);
	m_NJ_nGameCode = ini.GetInt("LOCALE", "GameCode", SERVER_CONFIG_DEFAULT_NJ_DBAGENT_GAMECODE);

	SetUseFilterState(ini.GetInt<bool>("FILTER", "USE", 0));
	SetAcceptInvalidIPState(ini.GetInt<bool>("FILTER", "ACCEPT_INVALID_IP", 1));
	m_bIsUseEvent = ini.GetInt<bool>("ENVIRONMENT", "USE_EVENT", SERVER_CONFIG_DEFAULT_USE_EVENT);
	m_bIsUseFileCrc = ini.GetInt<bool>("ENVIRONMENT", "USE_FILECRC",
		SERVER_CONFIG_DEFAULT_USE_FILECRC);

	m_bIsComplete = true;
	return m_bIsComplete;
}
void MMatchConfig::Destroy()
{

}

void MMatchConfig::AddFreeLoginIP(const char* pszIP)
{
	m_FreeLoginIPList.push_back(pszIP);
}


void MMatchConfig::AddDebugLoginIP( const char* szIP )
{
	m_DebugLoginIPList.push_back( szIP );
}


bool MMatchConfig::CheckFreeLoginIPList(const char* pszIP)
{
	list<string>::iterator end = m_FreeLoginIPList.end();
	for (list<string>::iterator i = m_FreeLoginIPList.begin(); i!= end; i++) {
		const char* pszFreeIP = (*i).c_str();
		if (strncmp(pszIP, pszFreeIP, strlen(pszFreeIP)) == 0) {
			return true;
		}
	}
	return false;
}


bool MMatchConfig::IsDebugLoginIPList( const char* pszIP )
{
	list< string >::iterator it, end;
	end = m_DebugLoginIPList.end();
	for( it = m_DebugLoginIPList.begin(); it != end; ++it )
	{
		const char* pszFreeIP = (*it).c_str();
		if (strncmp(pszIP, pszFreeIP, strlen(pszFreeIP)) == 0) {
			return true;
		}
	}
	return false;
}


void MMatchConfig::TrimStr(const char* szSrcStr, char* outStr, int maxlen)
{
	char szInputMapName[256] = "";

	// 왼쪽 공백제거
	int nSrcStrLen = (int)strlen(szSrcStr);
	for (int i = 0; i < nSrcStrLen; i++)
	{
		if (!isspace(szSrcStr[i]))
		{
			strcpy_safe(szInputMapName, &szSrcStr[i]);
			break;
		}
	}
	// 오른쪽 공백제거
	int nLen = (int)strlen(szInputMapName);
	for (int i = nLen-1; i >= 0; i--)
	{
		if (isspace(szInputMapName[i]))
		{
			szInputMapName[i] = '\0';
		}
		else
		{
			break;
		}
	}

	strcpy_safe(outStr, maxlen, szInputMapName);
}

void MMatchConfig::Clear()
{
	memset(m_szDB_DNS, 0, 64 );
	memset(m_szDB_UserName, 0, 64 );
	memset(m_szDB_Password, 0, 64 );		///< DB Password

	m_nMaxUser = 0;					///< 최대 접속자
	m_nServerID = 0;
	memset(m_szServerName, 0, 256 );		///< 서버이름

	// m_nServerMode;				///< 서버모드
	m_bRestrictionMap = false;			///< 맵제한이 있는지 여부 - default : false
	m_EnableMaps.clear();				///< 맵제한이 있을경우 가능한 맵
	m_FreeLoginIPList.clear();			///< 접속인원 무시 IP
	m_bCheckPremiumIP = true;			///< 프리미엄 IP 체크

	// enabled 씨리즈 - ini에서 관리하지 않는다.
	m_bEnabledCreateLadderGame = false;	///< 클랜전 생성가능한지 여부

	// -- 일본 넷마블 전용
	memset( m_NJ_szDBAgentIP, 0, 64 );
	m_NJ_nDBAgentPort = 0;
	m_NJ_nGameCode = 0;

	// filter 사용 설정.
	m_bUseFilter = false;				/// 필터 사용을 설정함(0:사용않함. 1:사용.)
	m_bAcceptInvalidIP = true;			/// 리스트에 없는 IP허용 여부.(0:허용하지 않음. 1:허용.)
}
