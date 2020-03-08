#include "stdafx.h"
#include "MLocatorStatistics.h"
#include "MDebug.h"
#include "MUtil.h"
#include "MLocatorConfig.h"
#include "MTime.h"

MLocatorStatistics::MLocatorStatistics() = default;
MLocatorStatistics::~MLocatorStatistics() = default;


void MLocatorStatistics::Reset()
{
	DumpSelfLog();

	ResetCountryStatistics();
	ResetDeadServerStatistics();
	ResetCountryCodeCacheHitMissCount();
	ResetInvalidIPCount();
	ResetCountryCodeCheckCount();
	ResetBlockCount();
	ResetBlockCountryCodeHitCount();
}


void MLocatorStatistics::IncreaseCountryStatistics( const std::string& strCountryCode3, const int nCount )
{
	if( strCountryCode3.empty() || (3 < strCountryCode3.length()) ) 
		return;

	CountryStatisticsMap::iterator it = m_CountryStatistics.find( strCountryCode3 );
	if( m_CountryStatistics.end() != it )
	{
		it->second += nCount;
	}
	else
	{
		m_CountryStatistics.insert( CountryStatisticsMap::value_type(strCountryCode3, nCount) );
		mlog( "locator statistics add new country code3(%s)\n", strCountryCode3.c_str() );
	}

	IncreaseCountryCodeCheckCount();
}


void MLocatorStatistics::IncreaseDeadServerStatistics( const int nServerID )
{
	if( 0 > nServerID ) 
		return;

	DeadServerStatisticsMap::iterator it = m_DeadServerStatistics.find( nServerID );
	if( m_DeadServerStatistics.end() != it )
	{
		++it->second;
	}
	else
	{
		m_DeadServerStatistics.insert( DeadServerStatisticsMap::value_type(nServerID, 1) );
		mlog( "locator statistics add new server id(%d)\n", nServerID );
	}
}


void MLocatorStatistics::ResetCountryStatistics()
{
	CountryStatisticsMap::iterator it, end;
	end = m_CountryStatistics.end();
	for( it = m_CountryStatistics.begin(); it != end; ++it )
		it->second = 0;
}


void MLocatorStatistics::ResetDeadServerStatistics()
{
	DeadServerStatisticsMap::iterator it, end;
	end = m_DeadServerStatistics.end();
	for( it = m_DeadServerStatistics.begin(); it != end; ++it )
		it->second = 0;
}


void MLocatorStatistics::DumpSelfLog()
{
	mlog( "\n===========================Loator self dump(%s)=========================\n",
		MGetStrLocalTime().c_str() );
	mlog( " - Server dead count list- \n" );
	mlog( "LiveServerCount(%u) : DeadServerCount(%u)\n",
		GetLiveServerCount(), GetDeadServerCount() );

	DeadServerStatisticsMap::iterator it, end;
	end = m_DeadServerStatistics.end();
	for( it = m_DeadServerStatistics.begin(); it != end; ++it )
		mlog( "ServerID:%d, DeadCount:%d\n", it->first, it->second );

	mlog( "\n" );

	mlog( "Country code3 cache hit miss count : %u\n", GetCountryCodeCacheHitMissCount() );
	mlog( "Invalid IP count : %d\n", GetInvalidIPCount() );
	mlog( "Block Count : %d\n", GetBlockCount() );
	mlog( "Country code check count : %u\n", GetCountryCodeCheckCount() );
	mlog( "Block country code hit count : %u\n", GetBlockCountryCodeHitCount() );
	mlog( "========================================================================================\n\n" );
}


void MLocatorStatistics::InitInsertCountryCode( const std::string& strCountryCode3 )
{
	if( strCountryCode3.empty() || (3 < strCountryCode3.length()) ) 
		return;

	m_CountryStatistics.insert( CountryStatisticsMap::value_type(strCountryCode3, 0) );
}


MLocatorStatistics& GetLocatorStatistics()
{
	return MLocatorStatistics::GetInstance();
}