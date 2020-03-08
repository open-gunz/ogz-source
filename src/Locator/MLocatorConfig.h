#pragma once

#include "GlobalTypes.h"
#include "MUID.h"

#define LOCATOR_CONFIG "./Locator.ini"

class MLocatorConfig
{
public:
	bool LoadConfig();

	// DB
	auto& GetDBDSN() const			{ return m_strDBDSN; }
	auto& GetDBUserName() const	{ return m_strDBUserName; }
	auto& GetDBPassword() const	{ return m_strDBPassword; }

	// Network
	auto& GetLocatorIP()	const { return m_strLocatorIP; }
	auto GetLocatorPort()		const { return m_nLocatorPort; }

	// Environment
	auto GetLocatorID() const							{ return m_dwID; }
	auto& GetLocatorUID() const							{ return m_uidLocatorUID; }
	auto GetMaxElapsedUpdateServerStatusTime() const	{ return m_dwMaxElapsedUpdateServerStatusTime; }
	auto GetUDPLiveTime() const							{ return m_dwUDPLiveTime; }
	auto GetMaxFreeUseCountPerLiveTime() const			{ return m_dwMaxFreeUseCountPerLiveTime; }
	auto GetBlockTime() const							{ return m_dwBlockTime; }
	auto GetUpdateUDPManagerElapsedTime() const			{ return m_dwUpdateUDPManagerElapsedTime; }
	auto GetMarginOfErrorMin() const					{ return m_dwMarginOfErrorMin; }
	auto GetElapsedTimeUpdateLocatorLog() const			{ return m_dwElapsedTimeUpdateLocatorLog; }

	auto IsUseCountryCodeFilter() const	{ return m_bIsUseCountryCodeFilter; }
	auto IsInitCompleted() const			{ return m_bIsInitCompleted; }
	auto IsAcceptInvalidIP() const			{ return m_bIsAcceptInvaildIP; }
	auto IsTestServerOnly() const			{ return m_bIsTestServerOnly; }

private:
	bool LoadDBConfig(const struct IniParser&);
	bool LoadNetConfig(const IniParser&);
	bool LoadEnvConfig(const IniParser&);
	
	// Network
	std::string	m_strLocatorIP;
	int m_nLocatorPort;

	// Environment
	u32	m_dwID;
	MUID m_uidLocatorUID;
	u32	m_dwMaxElapsedUpdateServerStatusTime;
	u32	m_dwUDPLiveTime;
	u32	m_dwMaxFreeUseCountPerLiveTime;
	u32	m_dwBlockTime;
	u32	m_dwUpdateUDPManagerElapsedTime;
	u32	m_dwMarginOfErrorMin;
	u32	m_dwGMTDiff;
	bool m_bIsUseCountryCodeFilter;
	u32	m_dwElapsedTimeUpdateLocatorLog;

	bool	m_bIsAcceptInvaildIP;
	bool	m_bIsTestServerOnly;
	
	// DB
	std::string m_strDBDSN;
	std::string m_strDBUserName;
	std::string m_strDBPassword;

	bool	m_bIsInitCompleted = false;
};


MLocatorConfig* GetLocatorConfig();