#include "stdafx.h"
#include "MMatchSchedule.h"
#include "MBMatchServer.h"
#include "MSharedCommandTable.h"
#include "MErrorTable.h"


void MBMatchServer::OnResponseServerStatus( const MUID& uidSender )
{
	if( !IsKeeper(uidSender) )
		return;

	MCommand* pCmd = CreateCommand( MC_RESPONSE_MATCHSERVER_STATUS, uidSender );
	if( 0 == pCmd )
		return;

	string strServerVer( __DATE__ );

	pCmd->AddParameter( new MCmdParamInt(GetDBMgr()->IsOpen() ? SES_NO : SES_ERR_DB) );
	pCmd->AddParameter( new MCmdParamStr(strServerVer.c_str()) );
	pCmd->AddParameter( new MCmdParamUChar(static_cast<unsigned char>(GetAgentCount())) );
	
	Post( pCmd );
}


void MBMatchServer::OnRequestServerHearbeat( const MUID& uidSender )
{
	if( !IsKeeper(uidSender) )
		return;

	OnResponseServerHeartbeat( uidSender );
}


void MBMatchServer::OnResponseServerHeartbeat( const MUID& uidSender )
{
	if( !IsKeeper(uidSender) )
		return;

	MCommand* pCmd = CreateCommand( MC_RESPONSE_SERVER_HEARHEAT, uidSender );
	if( 0 == pCmd )
		return;

	Post( pCmd );
}


void MBMatchServer::OnRequestConnectMatchServer( const MUID& uidSender )
{
	MCommObject* pCommObj = (MCommObject*)m_CommRefCache.GetRef( uidSender );
	if (pCommObj == NULL) 
		return;

	if( MOK == ObjectAdd(uidSender) )
	{
		MMatchObject* pObj = GetObject( uidSender );
		if( 0 == pObj ) 
			return;

		pObj->AddCommListener( uidSender );
		pObj->SetObjectType(MOT_PC);
		pObj->SetFreeLoginIP( true );
		
		if (pCommObj != NULL)
		{
			pObj->SetPeerAddr(pCommObj->GetIP(), pCommObj->GetIPString(), pCommObj->GetPort());
		}

		OnResponseConnectMatchServer( uidSender );
	}
	else
	{
		mlog( "MMatchServer::OnRequestConnectMatchServer - Keeper connect ½ÇÆÐ.\n" );
		return;
	}
}


void MBMatchServer::OnResponseConnectMatchServer( const MUID& uidSender )
{
	MCommand* pCmd = CreateCommand( MC_RESPONSE_KEEPER_CONNECT_MATCHSERVER, uidSender );
	if( 0 != pCmd )
	{
		pCmd->AddParameter( new MCmdParamUID(uidSender) );

		Post( pCmd );

		OnResponseServerStatus( uidSender );

		SetKeeperUID( uidSender );
	}
}


void MBMatchServer::OnRequestKeeperAnnounce( const MUID& uidSender, const char* pszAnnounce )
{
	if( !IsKeeper(uidSender) )
		return;

	MCommand* pCmd = CreateCommand( MC_MATCH_ANNOUNCE, MUID(0,0) );
	pCmd->AddParameter( new MCmdParamUInt(0) );
	pCmd->AddParameter( new MCmdParamStr(pszAnnounce) );

	RouteToAllClient( pCmd );
}


void MBMatchServer::OnRequestStopServerWithAnnounce( const MUID& uidSender )
{
	if( !IsKeeper(uidSender) )
		return;
	OnResponseStopServerWithAnnounce( uidSender );
}


void MBMatchServer::OnResponseStopServerWithAnnounce( const MUID& uidSender )
{
	if( !IsKeeper(uidSender) )
		return;

	OnAdminServerHalt();

	MCommand* pCmd = CreateCommand( MC_RESPONSE_ANNOUNCE_STOP_SERVER, uidSender );
	if( 0 != pCmd )
		Post( pCmd );
}


void MBMatchServer::OnRequestSchedule( const MUID& uidSender, 
									   const int nType, 
									   const int nYear, 
									   const int nMonth, 
									   const int nDay, 
									   const int nHour, 
									   const int nMin,
									   const int nCount,
									   const int nCommand,
									   const char* pszAnnounce )
{
	if( !IsKeeper(uidSender) )
		return;
	
	OnResponseSchedule( uidSender, nType, nYear, nMonth, nDay, nHour, nMin, nCount, nCommand, pszAnnounce );
}


void MBMatchServer::OnResponseSchedule( const MUID& uidSender,
									    const int nType, 
									    const int nYear, 
									    const int nMonth, 
									    const int nDay, 
									    const int nHour, 
									    const int nMin,
									    const int nCount,
									    const int nCommand,
									    const char* pszAnnounce )
{
	if( !IsKeeper(uidSender) )
		return;

	MCommand* pCmd = CreateCommand( MC_RESPONSE_KEEPER_MANAGER_SCHEDULE, uidSender );
	if( 0 == pCmd )
		return;

	pCmd->AddParameter( new MCmdParamInt(nCommand) );

	switch( nCommand )
	{
	case KMSC_ANNOUNCE :
		{
			MCommand* pCmdSchedule = CreateCommand( MC_MATCH_SCHEDULE_ANNOUNCE_MAKE, MUID(0, 0) );
			if( 0 != pCmdSchedule )
			{
				pCmdSchedule->AddParameter( new MCmdParamStr(pszAnnounce) );
				const bool bIsOk = AddDynamicSchedule( m_pScheduler, nType - 1, pCmdSchedule, nYear, nMonth, nDay, nHour, nMin, nCount );
				if( !bIsOk )
					delete pCmdSchedule;
                
				pCmd->AddParameter( new MCmdParamChar(bIsOk) );
			}
			else
			{
				pCmd->AddParameter( new MCmdParamChar(false) );
			}
		}
		break;

	case KMSC_STOP_SERVER :
		{
			MCommand* pCmdSchedule = CreateCommand( MC_MATCH_SCHEDULE_STOP_SERVER, MUID(0, 0) );
			if( 0 != pCmdSchedule )
			{
				pCmdSchedule->AddParameter( new MCmdParamStr(pszAnnounce) );
				const bool bIsOk = AddDynamicSchedule( m_pScheduler, nType - 1, pCmdSchedule, nYear, nMonth, nDay, nHour, nMin, nCount );
				if( !bIsOk )
					delete pCmdSchedule;

				pCmd->AddParameter( new MCmdParamChar(bIsOk) );
			}
			else
			{
				pCmd->AddParameter( new MCmdParamChar(false) );
			}
		}
		break;

	default :
		{
			ASSERT( 0 );
		}
		break;
	}

	Post( pCmd );
}


void MBMatchServer::OnRequestKeeperStopServerSchedule( const MUID& uidSender, const char* pszAnnounce )
{
	if( m_This == uidSender )
	{
		OnResponseKeeperStopServerSchedule( uidSender, pszAnnounce );
	}
}


void MBMatchServer::OnResponseKeeperStopServerSchedule( const MUID& uidSender, const char* pszAnnounce )
{
	OnAdminServerHalt();

	if( (0 != pszAnnounce) && (0 < strlen(pszAnnounce)) )
	{
		Shout(pszAnnounce);
	}
}


void MBMatchServer::OnRequestReloadServerConfig( const MUID& uidSender, const string& strFileList )
{
	if( !IsKeeper(uidSender) )
		return;

	OnResponseReloadServerConfig( uidSender, strFileList );
}


void MBMatchServer::OnResponseReloadServerConfig( const MUID& uidSender, const string& strFileList )
{
	if( !IsKeeper(uidSender) )
		return;

	bool bReloadOK = false;
	
	if( !strFileList.empty() )
	{
		bool bContinue = true;
		ReloadFileListVec FileList;
		// split file list.
		char szFileName[ 64 ] = {0,};
		string::size_type start, end;
		start = end = 0;
		while( bContinue )
		{
			end = strFileList.find( ",", start );
			if( string::npos != end )
				strncpy_safe( szFileName, strFileList.c_str() + start, end - start );
			else
			{
				strncpy_safe( szFileName, strFileList.c_str() + start, strFileList.length() - start );
				bContinue = false;
			}

			FileList.push_back( szFileName );

#ifdef _DEBUG
			mlog( "%s\n", szFileName );
#endif
			memset( szFileName, 0, 64 );
			start = ++end;
		}

		bReloadOK = m_ConfigReloader.Reload( FileList );
	}
	else
	{
		bReloadOK = m_ConfigReloader.Reload( RELOAD_LIST );
	}
}
