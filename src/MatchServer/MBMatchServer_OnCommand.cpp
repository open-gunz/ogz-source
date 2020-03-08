#include "stdafx.h"
#include "MBMatchServer.h"
#ifdef MFC
#include "CommandLogView.h"
#include "MatchServerDoc.h"
#include "OutputView.h"
#include "MatchServer.h"
#include <atltime.h>
#endif
#include "MErrorTable.h"
#include "MDebug.h"
#include "MMatchRule.h"
#include "MDebug.h"
#include "MMatchStatus.h"
#include "MMatchSchedule.h"
#include "MSharedCommandTable.h"
#include "MMatchConfig.h"
#include "MBlobArray.h"
#include "MUtil.h"

bool MBMatchServer::OnCommand(MCommand* pCommand)
{
	if( MMatchServer::OnCommand(pCommand) )
		return true;

	switch( pCommand->GetID() )
	{
	case MC_MATCH_SCHEDULE_ANNOUNCE_MAKE :
		{
			char szAnnounce[ 512 ];

			pCommand->GetParameter( szAnnounce, 0, MPT_STR, 512 );

			OnScheduleAnnounce( szAnnounce );
		}
		break;

	case MC_MATCH_SCHEDULE_CLAN_SERVER_SWITCH_DOWN : 
			OnScheduleClanServerSwitchDown();		
		break;

	case MC_MATCH_SCHEDULE_CLAN_SERVER_SWITCH_ON :
			OnScheduleClanServerSwitchUp();
		break;

	case MC_REQUEST_KEEPER_CONNECT_MATCHSERVER :
			OnRequestConnectMatchServer( pCommand->GetSenderUID() );
		break;

	case MC_REQUEST_MATCHSERVER_STATUS :
			OnResponseServerStatus(pCommand->GetSenderUID());
		break;

	case MC_REQUEST_SERVER_HEARBEAT :
			OnRequestServerHearbeat( pCommand->GetSenderUID() );
		break;

	case MC_REQUEST_KEEPER_ANNOUNCE :
		{
			char szAnnounce[ 256 ];

			pCommand->GetParameter( szAnnounce, 0, MPT_STR, 256 );

			OnRequestKeeperAnnounce( pCommand->GetSenderUID(), szAnnounce );
		}
		break;

	case MC_REQUEST_ANNOUNCE_STOP_SERVER :
			OnRequestStopServerWithAnnounce( pCommand->GetSenderUID() );
		break;

	case MC_REQUEST_KEEPER_MANAGER_SCHEDULE :
		{
			int nType;
			int nYear;
			int nMonth;
			int nDay;
			int nHour;
			int nMin;
			int nCount;
			int nCommand;
			char szAnnounce[ 256 ] = {0,};

			pCommand->GetParameter( &nType, 0, MPT_INT );
			pCommand->GetParameter( &nYear, 1, MPT_INT );
			pCommand->GetParameter( &nMonth, 2, MPT_INT );
			pCommand->GetParameter( &nDay, 3, MPT_INT );
			pCommand->GetParameter( &nHour, 4, MPT_INT );
			pCommand->GetParameter( &nMin, 5, MPT_INT );
			pCommand->GetParameter( &nCount, 6, MPT_INT );
			pCommand->GetParameter( &nCommand, 7, MPT_INT );
			pCommand->GetParameter( szAnnounce, 8, MPT_STR, 255 );

			OnRequestSchedule( pCommand->GetSenderUID(), 
				nType, 
				nYear, 
				nMonth, 
				nDay, 
				nHour, 
				nMin, 
				nCount, 
				nCommand, 
				szAnnounce );
		}
		break;

	case MC_MATCH_SCHEDULE_STOP_SERVER :
		{
			char szAnnounce[ 256 ] = { 0, };

			pCommand->GetParameter( szAnnounce, 0, MPT_STR, 255 );

			OnRequestKeeperStopServerSchedule( pCommand->GetSenderUID(), szAnnounce );
		}
		break;

	case MC_REQUEST_RELOAD_CONFIG:
	{
		char szFileList[1024] = { 0, };

		pCommand->GetParameter(szFileList, 0, MPT_STR, 1024);

		OnRequestReloadServerConfig(pCommand->GetSenderUID(), szFileList);
	}
	break;

	case MC_REQUEST_ADD_HASHMAP:
		break;

	default:
		return false;
	}

	return true;
}
