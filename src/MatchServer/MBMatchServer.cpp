#include "stdafx.h"
#include "MBMatchServer.h"
#ifdef MFC
#include "MatchServerDoc.h"
#include "OutputView.h"
#include "MatchServer.h"
#include "CommandLogView.h"
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
#include "MMatchEventFactory.h"
#include "MMatchLocale.h"
#pragma comment(lib, "comsupp.lib")

#if defined(_DEBUG) && defined(MFC)
#define new DEBUG_NEW
#endif

bool MBMatchServer::OnCreate(void)
{
	if( !MMatchServer::OnCreate() )
		return false;

	if( !m_ConfigReloader.Create() )
		return false;

	WriteServerInfoLog();

	InitConsoleCommands();

	return true;
}

void MBMatchServer::OnDestroy(void)
{
}

void MBMatchServer::OnPrepareCommand(MCommand* pCommand)
{
#if defined(_DEBUG) && defined(MFC)
#ifndef _DEBUG_PUBLISH
	// 커맨드 로그 남기기
	if(m_pCmdLogView==NULL) return;

	CMatchServerApp* pApp = (CMatchServerApp*)AfxGetApp();
	if (pApp->CheckOutputLog() == false) return;


	CCommandLogView::CCommandType t;
	if(pCommand->m_pCommandDesc->IsFlag(MCDT_LOCAL)==true) t = CCommandLogView::CCT_LOCAL;
	else if(pCommand->m_Sender==m_This) t = CCommandLogView::CCT_SEND;
	else if(pCommand->m_Receiver==m_This) t = CCommandLogView::CCT_RECEIVE;
	else _ASSERT(FALSE);
	
	m_pCmdLogView->AddCommand(GetGlobalClockCount(), t, pCommand);
#endif
#endif
}



MBMatchServer::MBMatchServer(COutputView* pView)
{
#ifdef MFC
	m_pView = pView;
	m_pCmdLogView = NULL;
#endif
	
	SetKeeperUID( MUID(0, 0) );
}

void MBMatchServer::Shutdown()
{
	MMatchServer::Shutdown();
#ifdef MFC
	AfxGetMainWnd()->PostMessage(WM_DESTROY);
#endif
}

static void MBMatchServerLog(unsigned int LogLevel, const char* Msg, bool Newline)
{
	static std::mutex LogMutex;
	std::lock_guard<std::mutex> Lock(LogMutex);

	char Str[1024 * 16];
	ArrayView<char> Remaining = Str;
	auto Consumed = strftime(Str, sizeof(Str), "%FT%T%z", localtime(&unmove(time(0))));
	if (!Consumed)
	{
		assert(false);
		return;
	}
	Remaining.remove_prefix(Consumed);
	auto Append = [&](const char* a) {
		auto Zero = strcpy_safe(Remaining, a);
		Remaining.remove_prefix(Zero - Remaining.data());
	};
	Append(" | ");
	Append(Msg);
	Append(Newline ? "\n" : "");

	fputs(Str, stdout);
	fflush(stdout);

	if (LogLevel & MMatchServer::LOG_FILE)
		MLogFile(Str);

#ifdef _DEBUG
	if (LogLevel & MMatchServer::LOG_DEBUG)
	{
		OutputDebugString(Msg);
		if (Newline)
			OutputDebugString("\n");
	}
#endif

}

void MBMatchServer::Log(unsigned int LogLevel, const char* Msg)
{
	MBMatchServerLog(LogLevel, Msg, true);
}

bool MBMatchServer::InitSubTaskSchedule()
{
	if( ( MSM_CLAN == MGetServerConfig()->GetServerMode() ) || MSM_TEST == MGetServerConfig()->GetServerMode() ){
		if( !AddClanServerSwitchDownSchedule() )
			return false;

		if( !AddClanServerAnnounceSchedule() )
		return false;
	}

	return true;
}

bool MBMatchServer::AddClanServerSwitchDownSchedule()
{
	int a = 0;

	MCommand* pCmd = CreateCommand( MC_MATCH_SCHEDULE_CLAN_SERVER_SWITCH_DOWN, MUID(0, 0) );
	if( 0 == pCmd )
		return false;

	tm t;
	MMatchGetLocalTime(&t);

	MMatchScheduleData* pScheduleData = m_pScheduler->MakeOnceScheduleData( t.tm_year - 100, t.tm_mon + 1, GetMaxDay(), 23, 50, pCmd );
	if( 0 == pScheduleData ){
		delete pCmd;
		return false;
	}

	if( !m_pScheduler->AddDynamicSchedule(pScheduleData) ){
		delete pCmd;
		delete pScheduleData;
		return false;
	}

	mlog( "MBMatchServer::AddClanServerSwitchDownSchedule - 클랜서버다운 커맨드 생성 성공. 다음실행시간:%d년%d월%d일 %d시%d분\n",
		pScheduleData->GetYear(), pScheduleData->GetMonth(), pScheduleData->GetDay(),
		pScheduleData->GetHour(), pScheduleData->GetMin() );

	return true;
}


bool MBMatchServer::AddClanServerSwitchUpSchedule()
{
	MCommand* pCmd = CreateCommand( MC_MATCH_SCHEDULE_CLAN_SERVER_SWITCH_ON, MUID(0, 0) );
	if( 0 == pCmd )
	{
		mlog( "MBMatchServer::AddClanServerSwitchUpSchedule - 클랜섭 활성화 커맨드 생성 실패.\n" );
		return false;
	}

	tm t;
	MMatchGetLocalTime(&t);

	unsigned char cYear;
	unsigned char cMon;

	if( 12 >= (t.tm_mon + 2) )
	{
		cYear = t.tm_year - 100;
		cMon  = t.tm_mon + 2;
	}
	else
	{
		cYear = t.tm_year - 99;
		cMon  = 1;
	}

	MMatchScheduleData* pScheduleData = m_pScheduler->MakeOnceScheduleData( cYear, cMon, 1, 9, 0, pCmd );
	if( 0 == pScheduleData )
	{
		delete pCmd;
		return false;
	}

	if( !m_pScheduler->AddDynamicSchedule(pScheduleData) )
	{
		delete pCmd;
		delete pScheduleData;
		return false;
	}

	mlog( "MBMatchServer::AddClanServerSwitchUpSchedule - 클랜서버업 커맨드 생성 성공. 다음실행시간:%d년%d월%d일 %d시%d분\n",
		pScheduleData->GetYear(), pScheduleData->GetMonth(), pScheduleData->GetDay(),
		pScheduleData->GetHour(), pScheduleData->GetMin() );

	return true;
}

bool MBMatchServer::AddClanServerAnnounceSchedule()
{
	char szTest[] = "클랜전 게임을 11시 50분에 닫도록 하겠습니다.";

	MCommand* pCmd = CreateCommand( MC_MATCH_SCHEDULE_ANNOUNCE_MAKE, MUID(0, 0) );
	if( 0 == pCmd )
		return false;

	MCommandParameterString* pCmdPrmStr = new MCommandParameterString( szTest );
	if( 0 == pCmdPrmStr ){
		delete pCmd;
		return false;
	}
	
	if( !pCmd->AddParameter(pCmdPrmStr) ){
		delete pCmd;
		return false;
	}

	tm t;
	MMatchGetLocalTime(&t);

	MMatchScheduleData* pScheduleData = m_pScheduler->MakeOnceScheduleData( t.tm_year - 100, t.tm_mon + 1, GetMaxDay(), 23, 40, pCmd );
	if( 0 == pScheduleData ){
		delete pCmd;
		return false;
	}

	if( !m_pScheduler->AddDynamicSchedule(pScheduleData) ){
		pScheduleData->Release();
		delete pScheduleData;
		return false;
	}
	
	return true;
}



char log_buffer[65535];

void AddStr(const char* pFormat,...)
{
	va_list args;
	char temp[1024];

	va_start(args, pFormat);
	vsprintf_safe(temp, pFormat, args);

	strcat_safe(log_buffer, temp);
	va_end(args);
}

void MBMatchServer::OnViewServerStatus()
{
	MGetServerStatusSingleton()->SaveToLogFile();
}

void MBMatchServer::InitLocator()
{
	if (!MGetServerConfig()->IsMasterServer())
		return;

	Locator = std::make_unique<MLocator>();
	if (!Locator->Create())
	{
		Log(LOG_ALL, "Failed to create locator");
		Locator.reset();
		return;
	}
	else
		Log(LOG_ALL, "Locator created!");
}

bool MBMatchServer::IsKeeper( const MUID& uidKeeper )
{
	MMatchObject* pObj = GetObject( uidKeeper );
	if( 0 == pObj )
		return false;

	if( !MGetServerConfig()->IsKeeperIP(pObj->GetIPString()) )
	{
		mlog( "Keeper hacking. " );
		if( 0 != pObj->GetIPString() )
			mlog( "IP:%s, ", pObj->GetIPString() );

		if( (0 != pObj->GetCharInfo()) && (0 != pObj->GetCharInfo()->m_szName) )
			mlog( "Name:%s", pObj->GetCharInfo()->m_szName );

		mlog( "\n" );

		return false;
	}

	return true;
}


void MBMatchServer::WriteServerInfoLog()
{
	mlog( "\n" );
	mlog( "================================== Server configure info ==================================\n" );

	char szTemp[256];
	sprintf_safe(szTemp, "Release Date : %s", __DATE__);
	Log(LOG_ALL, szTemp);
	
	if( MC_KOREA == MGetLocale()->GetCountry() )
		LOG( LOG_ALL, "Server Country : KOREA" );
	else if( MC_US == MGetLocale()->GetCountry() )
		LOG( LOG_ALL, "Server Country : US" );
	else if( MC_JAPAN == MGetLocale()->GetCountry() )
		LOG( LOG_ALL, "Server Country : JAPAN" );
	else if( MC_BRAZIL == MGetLocale()->GetCountry() )
		LOG( LOG_ALL, "Server Country : BRAZIL" );
	else if( MC_INDIA == MGetLocale()->GetCountry() )
		LOG( LOG_ALL, "Server Country : INDIA" );
	else
	{
		ASSERT( 0 && "국가 설정을 해주세요." );
		LOG( LOG_ALL, "!!!!!!!!!Not setted country code!!!!!!!!!!!!!" );
	}
	
	LOG( LOG_ALL, "Command version : (%u)", MCOMMAND_VERSION );
	LOG( LOG_ALL, "Event usable state : (%s)", MGetServerConfig()->IsUseEvent() ? "true" : "false" );
	LOG( LOG_ALL, "Load event size : (%u)", MMatchEventFactoryManager::GetInstance().GetLoadEventSize() );
	LOG( LOG_ALL, "FileCRCCheckSum usable state : (%s)", MGetServerConfig()->IsUseFileCrc() ? "true" : "false" );
	LOG( LOG_ALL, "FileCRC size : (%u)", MMatchAntiHack::GetFielCRCSize() );
	LOG( LOG_ALL, "Country Code Filter usalbe state : (%s)", MGetServerConfig()->IsUseFilter() ? "true" : "false" );
	LOG( LOG_ALL, "Accept Invalied IP state : (%s)", MGetServerConfig()->IsAcceptInvalidIP() ? "true" : "false" );
	LOG( LOG_ALL, "Keeper IP : (%s)", MGetServerConfig()->GetKeeperIP().c_str() );

	mlog( "===========================================================================================\n" );
	mlog( "\n" );
}

void MBMatchServer::OnRun()
{
	if (Locator)
		Locator->Run();

	MMatchServer::OnRun();
}

void MatchServerCustomLog(const char* Msg)
{
	MBMatchServerLog(MMatchServer::LOG_ALL & ~MMatchServer::LOG_FILE, Msg, false);
}