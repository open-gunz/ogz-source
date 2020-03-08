#pragma once

#include <map>
#include <string>

typedef std::map<std::string, u32>	CountryStatisticsMap;
typedef std::map<int, u32>		DeadServerStatisticsMap;


class MLocatorStatistics
{
public:
	~MLocatorStatistics();

	void Reset();

	void InitInsertCountryCode( const std::string& strCountryCode3 );

	void IncreaseCountryStatistics( const std::string& strCountryCode3, const int nCount = 1 );
	void IncreaseDeadServerStatistics( const int nServerID );
	void IncreaseCountryCodeCacheHitMissCount() { ++m_dwCountryCodeCacheHitMissCount; }
	void IncreaseInvalidIPCount()				{ ++m_dwInvalidIPCount; }
	void IncreaseBlockCount()					{ ++m_dwBlockCount; }
	void IncreaseBlockCountryCodeHitCount()		{ ++m_dwBlockCountryCodeHitCount; }
	
	const CountryStatisticsMap& GetCountryStatistics() const	{ return m_CountryStatistics; }
	auto GetLastUpdatedTime()							{ return m_dwLastUpdatedTime; }
	
	void SetLastUpdatedTime( u64 dwTime )	{ m_dwLastUpdatedTime = dwTime; }
	void SetDeadServerCount( const u32 dwCount )	{ m_dwDeadServerCount = dwCount; }
	void SetLiveServerCount( const u32 dwCount )	{ m_dwLiveServerCount = dwCount; }

	static MLocatorStatistics& GetInstance() 
	{
		static MLocatorStatistics LocatorStatistics;
		return LocatorStatistics;
	}

private:
	MLocatorStatistics();

	void IncreaseCountryCodeCheckCount() { ++m_dwCountryCodeCheckCount; }

	const u32 GetCountryCodeCacheHitMissCount() const { return m_dwCountryCodeCacheHitMissCount; }
	const u32 GetInvalidIPCount() const				{ return m_dwInvalidIPCount; }
	const u32 GetCountryCodeCheckCount() const		{ return m_dwCountryCodeCheckCount; }
	const u32 GetBlockCount() const					{ return m_dwBlockCount; }
	const u32 GetDeadServerCount() const				{ return m_dwDeadServerCount; }
	const u32 GetLiveServerCount() const				{ return m_dwLiveServerCount; }
	const u32 GetBlockCountryCodeHitCount() const		{ return m_dwBlockCountryCodeHitCount; }

	void ResetCountryStatistics();
	void ResetDeadServerStatistics();
	void ResetCountryCodeCacheHitMissCount()	{ m_dwCountryCodeCacheHitMissCount = 0; }
	void ResetInvalidIPCount()					{ m_dwInvalidIPCount = 0; }
	void ResetCountryCodeCheckCount()			{ m_dwCountryCodeCheckCount = 0; }
	void ResetBlockCount()						{ m_dwBlockCount = 0; }
	void ResetBlockCountryCodeHitCount()		{ m_dwBlockCountryCodeHitCount = 0; }

	void DumpSelfLog();
	
private :
	CountryStatisticsMap	m_CountryStatistics;
	DeadServerStatisticsMap m_DeadServerStatistics;
	u32					m_dwCountryCodeCacheHitMissCount{};
	u32					m_dwInvalidIPCount{};
	u32					m_dwCountryCodeCheckCount{};
	u32					m_dwBlockCount{};
	u32					m_dwLiveServerCount{};
	u32					m_dwDeadServerCount{};
	u32					m_dwBlockCountryCodeHitCount{};

	u64					m_dwLastUpdatedTime{};
};

MLocatorStatistics& GetLocatorStatistics();