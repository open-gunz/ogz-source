#include "stdafx.h"
#include "MSharedCommandTable.h"
#include "MMatchConfig.h"
#include "MBMatchServer.h"

// 공지사항.
void MBMatchServer::OnScheduleAnnounce( const char* pszAnnounce )
{
	if( 0 == pszAnnounce )
		return;

	MCommand* pNewCmd = CreateCommand( MC_MATCH_SCHEDULE_ANNOUNCE_SEND, MUID(0, 0) );
	if( 0 == pNewCmd )
		return;

	MCommandParameterString* pCmdPrmStr = new MCommandParameterString( pszAnnounce );
	if( 0 == pCmdPrmStr ){
		delete pNewCmd;
		return;
	}

	if( !pNewCmd->AddParameter(pCmdPrmStr) ){
		delete pNewCmd;
		return;
	}

	RouteToAllClient( pNewCmd );
}

// 클랜서버 Disable.
void MBMatchServer::OnScheduleClanServerSwitchDown()
{
	// 클랜 서버일 경우만 실행될수 있게.
	if( ( MSM_CLAN != MGetServerConfig()->GetServerMode() ) && ( MSM_TEST != MGetServerConfig()->GetServerMode() ) )
		return;		// 아니면 종료.

	MGetServerConfig()->SetEnabledCreateLadderGame( false );

	if( !AddClanServerSwitchUpSchedule() )
	{
		mlog( "MBMatchServer::OnScheduleClanServerSwitchDown - 클랜전 자동 활성화 커맨드 생성 작업 실패.\n" );
		return;
	}

	mlog( "MBMatchServer::OnScheduleClanServerSwitchDown - 클랜서버 클랜전 비활성화.\n" );
}


void MBMatchServer::OnScheduleClanServerSwitchUp()
{
	// 클랜 서버일 경우만 실행될수 있게.
	if ((MSM_CLAN != MGetServerConfig()->GetServerMode()) && (MSM_TEST != MGetServerConfig()->GetServerMode()))
		return;		// 아니면 종료.

	MGetServerConfig()->SetEnabledCreateLadderGame( true );

	// 클랜전 비활성화 커맨드 생성.
	if( AddClanServerAnnounceSchedule() )
	{
		if( !AddClanServerSwitchDownSchedule() )
			mlog( "MBMatchServer::OnScheduleClanServerSwitchUp - 클랜전 비활성화 스케쥴작업 실패.\n" );
	}
	else
	{
		mlog( "MBMatchServer::OnScheduleClanServerSwitchUp - 클랜전 비활성화 공지 커맨드 생성 스케쥴작업 실패.\n" );
		return;
	}	

	mlog( "MBMatchServer::OnScheduleClanServerSwitchDown - 클랜서버 클랜전 활성화.\n" );
}