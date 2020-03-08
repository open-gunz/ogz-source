#include "stdafx.h"
#include "MLocatorConfig.h"
#include "IniParser.h"
#include "MUtil.h"
#include "StringView.h"
#include "ArrayView.h"
#include <unordered_map>

MLocatorConfig* GetLocatorConfig()
{
	static MLocatorConfig LocatorConfig;
	return &LocatorConfig;
}

bool MLocatorConfig::LoadConfig()
{
	IniParser ini;
	if (!ini.Parse(LOCATOR_CONFIG))
		return false;
	bool DBLoadSuccess = LoadDBConfig(ini);
#ifdef LOCATOR_FREESTANDING
	if( !DBLoadSuccess )	return false;
#endif
	if( !LoadNetConfig(ini) )	return false;
	if( !LoadEnvConfig(ini) )	return false;

	m_bIsInitCompleted = true;

	return true;
}

bool MLocatorConfig::LoadDBConfig(const IniParser& ini)
{
	if (auto Value = ini.GetString("DB", "DNS"))
		m_strDBDSN = Value->str();
	else
		return false;

	if (auto Value = ini.GetString("DB", "USERNAME"))
		m_strDBUserName = Value->str();
	else
		return false;

	if (auto Value = ini.GetString("DB", "PASSWORD"))
		m_strDBPassword = Value->str();
	else
		return false;

	return true;
}

bool MLocatorConfig::LoadNetConfig(const IniParser& ini)
{
	m_strLocatorIP = ini.GetString("NETWORK", "IP", "127.0.0.1").str();
	m_nLocatorPort = ini.GetInt("NETWORK", "PORT", 8900);

	return true;
}

bool MLocatorConfig::LoadEnvConfig(const IniParser& ini)
{
	{
		auto High = ini.GetInt("ENV", "LOCATOR_UID_HIGH", 5);
		auto Low = ini.GetInt("ENV", "LOCATOR_UID_LOW", 0);
		m_uidLocatorUID = MUID(High, Low);
	}
	m_dwID = ini.GetInt("ENV", "ID", 1);
	m_dwMaxElapsedUpdateServerStatusTime = ini.GetInt("ENV",
		"MAX_ELAPSED_UPDATE_SERVER_STATUS_TIME", 1000);
	m_dwUDPLiveTime = ini.GetInt("ENV", "UDP_LIVE_TIME", 10000000);
	m_dwMaxFreeUseCountPerLiveTime = ini.GetInt("ENV", "MAX_FREE_RECV_COUNT_PER_LIVE_TIME", 9);
	m_dwBlockTime = ini.GetInt("ENV", "BLOCK_TIME", 0);
	m_dwUpdateUDPManagerElapsedTime = ini.GetInt("ENV", "UPDATE_UDP_MANAGER_ELAPSED_TIME", 3);
	m_dwMarginOfErrorMin = ini.GetInt("ENV", "MARGIN_OF_ERROR_MIN", 500000);
	m_dwElapsedTimeUpdateLocatorLog = ini.GetInt("ENV", "ELAPSED_TIME_UPDATE_LOCATOR_LOG", 10000);
	m_bIsUseCountryCodeFilter = ini.GetInt<bool>("ENV", "USE_COUNTRY_CODE_FILTER", 0);
	m_bIsAcceptInvaildIP = ini.GetInt<bool>("ENV", "ACCEPT_INVALID_IP", 1);
	m_bIsTestServerOnly = ini.GetInt<bool>("ENV", "TEST_SERVER", 0);

	return true;
}