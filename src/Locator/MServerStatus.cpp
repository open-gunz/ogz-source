#include "stdafx.h"
#include "MLocatorConfig.h"
#include "MLocatorStatistics.h"
#include "MServerStatus.h"
#include <ctime>
#include <algorithm>

const int MakeCustomizeMin( const std::string& strTime )
{
	if (strTime.length() < 16)
		return -1;

	int nHourConvMin;
	int nMin;
	char szVal[ 3 ];

	memset( szVal, 0, 3 );
	strncpy_safe( szVal, &strTime[11], 2 );
	nHourConvMin = atoi( szVal ) * 60;

	memset( szVal, 0, 3 );
	strncpy_safe( szVal, &strTime[14], 2 );
	nMin = atoi( szVal );

	return nHourConvMin + nMin;
}


const float GetPlayerRate( const float fCurPlayer, const float fMaxPlayer )
{
	return fCurPlayer / fMaxPlayer * 100.0f;
}

void MServerStatus::SetLiveStatus( const bool bLiveState )
{
	if( m_bIsLive && !bLiveState )
		GetLocatorStatistics().IncreaseDeadServerStatistics( GetID() );
	
	m_bIsLive = bLiveState;
}

void MServerStatusMgr::Insert( const MServerStatus& ss )
{
	m_ServerStatusVec.push_back( ss );
}


void MServerStatusMgr::CheckDeadServerByLastUpdatedTime( const int nMarginOfErrMin, const int nCmpCustomizeMin )
{
#ifdef LOCATOR_FREESTANDING
	SetLiveServerCount( 0 );
	SetDeadServerCount( 0 );
	ClearDeadServerIDList();

	for_each( m_ServerStatusVec.begin(),
			  m_ServerStatusVec.end(),
			  MDeadTimeServerChecker<MServerStatus>(nMarginOfErrMin, nCmpCustomizeMin, this) );
#endif
}


const int MServerStatusMgr::CalcuMaxCmpCustomizeMin()
{
	auto TM = *localtime(&unmove(time(0)));

	return static_cast< int >( (TM.tm_hour * 60) + TM.tm_min );
}