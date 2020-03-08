#define _WINSOCKAPI_
#include "stdafx.h"
#include "MLocator.h"
#ifdef MFC
#include "MLocatorDBMgr.h"
#endif
#include "CustomDefine.h"
#include "MSafeUDP.h"
#include "MSharedCommandTable.h"
#include "MBlobArray.h"
#include "MLocatorConfig.h"
#include "MLocatorUDP.h"
#include "MCommandCommunicator.h"
#include "MErrorTable.h"
#include "MCommandBuilder.h"
#include "MUtil.h"
#include "MLocatorStatistics.h"

#include "MServerStatus.h"
#include "MLogManager.h"
#ifndef LOCATOR_FREESTANDING
#include "MMatchServer.h"
#include "MMatchConfig.h"
#endif

#ifdef SetPort
#undef SetPort
#endif

MLocator* g_pMainLocator = NULL;
void SetMainLocator( MLocator* pLocator ) { g_pMainLocator = pLocator; }
MLocator* GetMainLocator() { return g_pMainLocator; }


MLocator::MLocator()
{
	auto time = GetGlobalTimeMS();
	m_dwLastServerStatusUpdatedTime = time;
	m_dwLastUDPManagerUpdateTime = time;
	m_dwLastLocatorStatusUpdatedTime = time;

	m_This = GetLocatorConfig()->GetLocatorUID();
}

MLocator::~MLocator()
{
	Destroy();
}


bool MLocator::Create()
{
	SetMainLocator( this );
	OnRegisterCommand( &m_CommandManager );

	if( !GetLocatorConfig()->IsInitCompleted() )
	{
		if (!GetLocatorConfig()->LoadConfig())
		{
			mlog("MLocator::Create - Failed to load config\n");
			return false;
		}
	}
	
#ifdef LOCATOR_FREESTANDING
	if( !InitDBMgr() )
	{
		mlog( "MLocator::Create - DB초기화 실패.\n" );
		return false;
	}
#endif

	if( !InitServerStatusMgr() )
	{
		mlog( "MLocator::Create - ServerStatusMgr 멤버 초기화 실패.\n" );
		return false;
	}

	if( !InitUDPManager() )
	{
		mlog( "MLocator::Create - UDP Manager 멤버 초기화 실패.\n" );
		return false;
	}

#ifdef LOCATOR_FREESTANDING
	if( !InitCountryCodeFilter() )
	{
		mlog( "MLocator::Create - 접속 허용 국가 코드 리스트 초기화 실패.\n" );
		return false;
	}
#endif

	if( !InitSafeUDP() )
	{
		mlog( "MLocator::Create - SafeUDP초기화 실패.\n" );
		return false;
	}

#ifdef _DEBUG
	InitDebug();
#endif

	return true;
}


bool MLocator::InitDBMgr()
{
#ifdef MFC
	if( 0 != m_pDBMgr )
		ReleaseDBMgr();

	m_pDBMgr = new MLocatorDBMgr;
	if( 0 != m_pDBMgr )
	{
		const CString strDSNString = m_pDBMgr->BuildDSNString( GetLocatorConfig()->GetDBDSN(),
															   GetLocatorConfig()->GetDBUserName(), 
															   GetLocatorConfig()->GetDBPassword() );
		const bool bConnect = m_pDBMgr->Connect( strDSNString );
		if( bConnect )
		{
			GetDBServerStatus( timeGetTime(), true );

			return m_pDBMgr->StartUpLocaterStauts( GetLocatorConfig()->GetLocatorID(), 
				GetLocatorConfig()->GetLocatorIP(), 
				GetLocatorConfig()->GetLocatorPort(),
				GetLocatorConfig()->GetMaxElapsedUpdateServerStatusTime() );
		}
	}

	return false;
#else
	return true;
#endif
}


bool MLocator::InitSafeUDP()
{
	if( 0 == m_pSafeUDP )
	{
		m_pSafeUDP = new MSafeUDP;
		if( 0 != m_pSafeUDP )
		{
			if( m_pSafeUDP->Create(true, GetLocatorConfig()->GetLocatorPort()) )
			{
				m_pSafeUDP->SetCustomRecvCallback( UDPSocketRecvEvent );
				return true;
			}
		}
	}

	return false;
}


bool MLocator::InitServerStatusMgr()
{
	if( 0 == m_pServerStatusMgr )
	{
		m_pServerStatusMgr = new MServerStatusMgr;
		if( 0 == m_pServerStatusMgr ) 
			return false;

		if( 0 != GetLocatorDBMgr() )
			GetDBServerStatus( 0, true );
#ifdef LOCATOR_FREESTANDING
		else
			ASSERT( 0 && "시작시에 DB의 정보를 가져오는것이 좋음." );
#endif
		
		return true;
	}

	return false;
}

bool MLocator::InitUDPManager()
{
	bool bRet = true;
	m_pRecvUDPManager = new MUDPManager;
	if( (0 == m_pRecvUDPManager) && (bRet) ) bRet = false;
	m_pSendUDPManager = new MUDPManager;
	if( (0 == m_pSendUDPManager) && (bRet) ) bRet = false;
	m_pBlockUDPManager = new MUDPManager;
	if( (0 == m_pBlockUDPManager) && (bRet) ) bRet = false;

	if( !bRet )
	{
		delete m_pRecvUDPManager;
		delete m_pSendUDPManager;
		delete m_pBlockUDPManager;

		m_pRecvUDPManager = 0;
		m_pSendUDPManager = 0;
		m_pBlockUDPManager = 0;

		return false;
	}

	InitMemPool( MLocatorUDPInfo );

	return true;
}

void MLocator::Destroy()
{
#ifdef LOCATOR_FREESTANDING
	ReleaseDBMgr();
#endif
	ReleaseSafeUDP();
	ReleaseUDPManager();
	ReleaseServerStatusMgr();
	ReleaseServerStatusInfoBlob();
}


void MLocator::ReleaseDBMgr()
{
#ifdef MFC
	if( 0 != m_pDBMgr )
	{
		m_pDBMgr->Disconnect();
		delete m_pDBMgr;
		m_pDBMgr = 0;
	}
#endif
}


void MLocator::ReleaseSafeUDP()
{
	if( 0 != m_pSafeUDP )
	{
		m_pSafeUDP->Destroy();
		delete m_pSafeUDP;
		m_pSafeUDP = 0;
	}
}


void MLocator::ReleaseServerStatusMgr()
{
	if( 0 != m_pServerStatusMgr )
	{
		delete m_pServerStatusMgr;
		m_pServerStatusMgr = 0;
	}
}


void MLocator::ReleaseServerStatusInfoBlob()
{
	if( 0 != m_vpServerStatusInfoBlob )
	{
		MEraseBlobArray( m_vpServerStatusInfoBlob );
		m_vpServerStatusInfoBlob = 0;
	}
}


void MLocator::ReleaseUDPManager()
{
	if( 0 != m_pRecvUDPManager )
	{
		m_pRecvUDPManager->SafeDestroy();
		delete m_pRecvUDPManager;
		m_pRecvUDPManager = 0;
	}

	if( 0 != m_pSendUDPManager )
	{
		m_pSendUDPManager->SafeDestroy();
		delete m_pSendUDPManager;
		m_pSendUDPManager = 0;
	}

	if( 0 != m_pBlockUDPManager )
	{
		m_pBlockUDPManager->SafeDestroy();
		delete m_pBlockUDPManager;
		m_pBlockUDPManager = 0;
	}

	ReleaseMemPool( MLocatorUDPInfo );

	UninitMemPool( MLocatorUDPInfo );
}

void MLocator::ReleaseCommand()
{
	while (MCommand* pCmd = GetCommandSafe())
		delete pCmd;
}

bool MLocator::GetServerStatus()
{
#ifdef LOCATOR_FREESTANDING
	return GetLocatorDBMgr()->GetServerStatus(GetServerStatusMgr());
#else
	auto& ServerStatusMgr = *GetServerStatusMgr();
	auto& MatchServer = *MGetMatchServer();
	auto& Config = *MGetServerConfig();

	if (ServerStatusMgr.GetSize() < 1)
	{
		ServerStatusMgr.Insert(MServerStatus());

		auto& Status = ServerStatusMgr[0];

		Status.SetID(Config.GetServerID());
		Status.SetType(4);
		Status.SetMaxPlayer(Config.GetMaxUser());
		Status.SetLastUpdatedTime("right meow");
		Status.SetIPString("");
		Status.SetIP(0);
		Status.SetPort(Config.GetPort());
		Status.SetServerName(Config.GetServerName());
		Status.SetOpenState(true);
		Status.SetLiveStatus(true);
	}

	auto& Status = ServerStatusMgr[0];
	Status.SetCurPlayer(MatchServer.GetObjects()->size());

	return true;
#endif
}

void MLocator::GetDBServerStatus(u64 dwEventTime, const bool bIsWithoutDelayUpdate)
{
	if (!(IsElapedServerStatusUpdatedTime(dwEventTime) || bIsWithoutDelayUpdate))
		return;

#ifdef LOCATOR_FREESTANDING
	if (!GetLocatorDBMgr())
		return;
#endif

	if (!m_pServerStatusMgr)
		return;

	if (GetServerStatus())
	{
		m_pServerStatusMgr->CheckDeadServerByLastUpdatedTime(GetLocatorConfig()->GetMarginOfErrorMin(),
			m_pServerStatusMgr->CalcuMaxCmpCustomizeMin());

		if (m_nLastGetServerStatusCount != m_pServerStatusMgr->GetSize())
		{
			MEraseBlobArray(m_vpServerStatusInfoBlob);

			m_nLastGetServerStatusCount = m_pServerStatusMgr->GetSize();
			m_vpServerStatusInfoBlob = MMakeBlobArray(MTD_SERVER_STATUS_INFO_SIZE, m_nLastGetServerStatusCount);
			m_nServerStatusInfoBlobSize = MGetBlobArraySize(m_vpServerStatusInfoBlob);
		}

		if (0 != m_vpServerStatusInfoBlob)
		{
			MTD_ServerStatusInfo* pMTDss;
			for (int i = 0; i < m_nLastGetServerStatusCount; ++i)
			{
				pMTDss = (MTD_ServerStatusInfo*)MGetBlobArrayElement(m_vpServerStatusInfoBlob, i);

				pMTDss->m_dwIP = (*m_pServerStatusMgr)[i].GetIP();
				pMTDss->m_nPort = (*m_pServerStatusMgr)[i].GetPort();
				pMTDss->m_nServerID = static_cast<unsigned char>((*m_pServerStatusMgr)[i].GetID());
				pMTDss->m_nCurPlayer = (*m_pServerStatusMgr)[i].GetCurPlayer();
				pMTDss->m_nMaxPlayer = (*m_pServerStatusMgr)[i].GetMaxPlayer();
				pMTDss->m_nType = (*m_pServerStatusMgr)[i].GetType();
				pMTDss->m_bIsLive = (*m_pServerStatusMgr)[i].IsLive();
			}

			UpdateLastServerStatusUpdatedTime(dwEventTime);
		}
		else
		{
			m_nLastGetServerStatusCount = -1;
		}
	}
	else
	{
		mlog("Fail to GetServerStatus\n");
		ASSERT(0 && "GetServerStatus실패.");
	}
}


bool MLocator::IsElapedServerStatusUpdatedTime(u64 dwEventTime)
{
	return (GetLocatorConfig()->GetMaxElapsedUpdateServerStatusTime() < (dwEventTime - GetUpdatedServerStatusTime()));
}


bool MLocator::IsLiveBlockUDP(const MLocatorUDPInfo* pBlkRecvUDPInfo, u64 dwEventTime)
{
	if( 0 == pBlkRecvUDPInfo ) return false;

	if( GetLocatorConfig()->GetBlockTime() > (dwEventTime - pBlkRecvUDPInfo->GetUseStartTime()) )
		return true;

	return false;
}


bool MLocator::IsBlocker(u32 dwIPKey, u64 dwEventTime)
{
	MUDPManager& rfBlkUDPMgr = GetBlockUDPManager();

	rfBlkUDPMgr.Lock();

	MLocatorUDPInfo* pBlkRecvUDPInfo = rfBlkUDPMgr.Find( dwIPKey );
	if( 0 != pBlkRecvUDPInfo )
	{
		if (!IsLiveBlockUDP(pBlkRecvUDPInfo, dwEventTime))
		{
			rfBlkUDPMgr.Delete( dwIPKey );
			rfBlkUDPMgr.Unlock();
			return false;
		}

		pBlkRecvUDPInfo->IncreaseUseCount();

		if( IsOverflowedNormalUseCount(pBlkRecvUDPInfo) )
		{
			pBlkRecvUDPInfo->SetUseStartTime( dwEventTime );
			pBlkRecvUDPInfo->SetUseCount( 1 );
		}

#ifdef _DEBUG
		mlog( "MLocator::IsBlocker - Block! time. dwIP:%u, UseCount:%d, LimitUseCount:%d, DbgInfo:%s\n",
			dwIPKey, 
			pBlkRecvUDPInfo->GetUseCount(), 
			GetLocatorConfig()->GetMaxFreeUseCountPerLiveTime(), 
			rfBlkUDPMgr.m_strExDbgInfo.c_str() );
#endif

		rfBlkUDPMgr.Unlock();
		return true;
	}

	rfBlkUDPMgr.Unlock();

	return false;
}// IsBlocker


bool MLocator::IsDuplicatedUDP(u32 dwIPKey, MUDPManager& rfCheckUDPManager, u64 dwEventTime)
{
	rfCheckUDPManager.Lock();

	MLocatorUDPInfo* pRecvUDPInfo = rfCheckUDPManager.Find( dwIPKey );
	if( 0 != pRecvUDPInfo )
	{
		pRecvUDPInfo->IncreaseUseCount();
		rfCheckUDPManager.Unlock();
		return false;
	}

	rfCheckUDPManager.Unlock();

	return true;
}


bool MLocator::UDPSocketRecvEvent(u32 dwIP, u16 wRawPort, char* pPacket, u32 dwSize)
{
	if( NULL == GetMainLocator() ) return false;
	if( sizeof(MPacketHeader) > dwSize ) return false;
	
	const auto dwEventTime = GetGlobalTimeMS();

	MLocator* pLocator = GetMainLocator();
	
	if( pLocator->IsBlocker(dwIP, dwEventTime ) ) 
		return false;
	
#ifdef _LOCATOR_TEST
	if( 400 < GetMainLocator()->GetRecvCount() ) 
		return true;
#endif
	
	if( !pLocator->IsDuplicatedUDP(dwIP, pLocator->GetRecvUDPManager(), dwEventTime) ) 
	{
		GetMainLocator()->IncreaseDuplicatedCount();
		return false;
	}

	MPacketHeader* pPacketHeader = (MPacketHeader*)pPacket;
	
	if ((dwSize != pPacketHeader->nSize) || 
		((pPacketHeader->nMsg != MSGID_COMMAND) && (pPacketHeader->nMsg != MSGID_RAWCOMMAND)) ) return false;
	
	unsigned int nPort = MSocket::ntohs(wRawPort);
	GetMainLocator()->ParseUDPPacket( &pPacket[MPACKET_HEADER_SIZE], pPacketHeader, dwIP, nPort );

	GetMainLocator()->IncreaseRecvCount();

	return true;
}// UDPSocketRecvEvent


void MLocator::ParseUDPPacket( char* pData, MPacketHeader* pPacketHeader, u32 dwIP, unsigned int nPort )
{
	switch (pPacketHeader->nMsg)
	{
	case MSGID_RAWCOMMAND:
		{
			unsigned short nCheckSum = MBuildCheckSum(pPacketHeader, pPacketHeader->nSize);
			if (pPacketHeader->nCheckSum != nCheckSum) {
				static int nLogCount = 0;
				if (nLogCount++ < 100) {	// Log Flooding
					mlog("MMatchClient::ParseUDPPacket() -> CHECKSUM ERROR(R=%u/C=%u)\n", 
						pPacketHeader->nCheckSum, nCheckSum);
				}
				return;
			} else {
				MCommand* pCmd = new MCommand();
				if (!pCmd->SetData(pData, &m_CommandManager))
				{
					char szLog[ 128 ] = {0,};
					sprintf_safe( szLog, "MLocator::ParseUDPPacket -> SetData Error\n" );
					GetLogManager().SafeInsertLog( szLog );
					
					delete pCmd;
					return;
				}

				if( MC_REQUEST_SERVER_LIST_INFO == pCmd->GetID() )
				{
					if( !GetRecvUDPManager().SafeInsert(dwIP, nPort, GetGlobalTimeMS()) )
					{
						char szLog[ 1024 ] = {0,};
						sprintf_safe(szLog, "fail to insert new IP(%u,%d) Time:%s\n",
							dwIP, nPort, MGetStrLocalTime().c_str() );
						GetLogManager().SafeInsertLog( szLog );
					}
				}
				else
				{
					ASSERT( 0 && "현제 추가정의된 처리커맨드가 없음." );

					char szLog[ 1024 ] = {0,};
					sprintf_safe(szLog, "invalide command(%u) Time:%s, dwIP:%u\n",
						pCmd->GetID(), MGetStrLocalTime().c_str(), dwIP );
					GetLogManager().SafeInsertLog( szLog );

					GetBlockUDPManager().SafeInsert( dwIP, nPort, GetGlobalTimeMS() );
					GetLocatorStatistics().IncreaseBlockCount();
				}

				delete pCmd;
			}
		}
		break;
	case MSGID_COMMAND:
		{
			ASSERT( 0 && "암호화 패킷 처리도 필요함." );
			char szLog[ 1024 ] = {0,};
			sprintf_safe(szLog, "encpypted command. Time:%s, dwIP:%u\n", 
				MGetStrLocalTime().c_str(), dwIP );
			GetLogManager().SafeInsertLog( szLog );

			GetBlockUDPManager().SafeInsert( dwIP, nPort, GetGlobalTimeMS() );
			GetLocatorStatistics().IncreaseBlockCount();

			unsigned short nCheckSum = MBuildCheckSum(pPacketHeader, pPacketHeader->nSize);
			if (pPacketHeader->nCheckSum != nCheckSum) {
				static int nLogCount = 0;
				if (nLogCount++ < 100) {	// Log Flooding
					mlog("MMatchClient::ParseUDPPacket() -> CHECKSUM ERROR(R=%u/C=%u)\n", 
						pPacketHeader->nCheckSum, nCheckSum);
				}
				return;
			} 
			else {
			}
		}
		break;
	default:
		{
			ASSERT( 0 && "Unrecognized packet" );
		}
		break;
	}
}// MLocator::ParseUDPPacket


void MLocator::Run()
{
	const auto dwEventTime = GetGlobalTimeMS();
	GetDBServerStatus( dwEventTime );
	FlushRecvQueue( dwEventTime );
	UpdateUDPManager( dwEventTime );
#ifdef LOCATOR_FREESTANDING
	UpdateLocatorStatus( dwEventTime );
	UpdateLocatorLog( dwEventTime );
#endif
	UpdateLogManager();
}


void MLocator::ResponseServerStatusInfoList( u32 dwIP, int nPort )
{
	if( 0 < m_nLastGetServerStatusCount )
	{
		MCommand* pCmd = CreateCommand( MC_RESPONSE_SERVER_LIST_INFO, MUID(0, 0) );
		if( 0 != pCmd )
		{
			pCmd->AddParameter( new MCommandParameterBlob(m_vpServerStatusInfoBlob, m_nServerStatusInfoBlobSize) );
			SendCommandByUDP( dwIP, nPort, pCmd );
			delete pCmd;
		}
	}
}


void MLocator::ResponseBlockCountryCodeIP( u32 dwIP, int nPort, const string& strCountryCode, const string& strRoutingURL )
{
	MCommand* pCmd = CreateCommand( MC_RESPONSE_BLOCK_COUNTRY_CODE_IP, MUID(0, 0) );
	if( 0 != pCmd )
	{
		pCmd->AddParameter( new MCommandParameterString(strCountryCode.c_str()) );
		pCmd->AddParameter( new MCommandParameterString(strRoutingURL.c_str()) );
		SendCommandByUDP( dwIP, nPort, pCmd );
		delete pCmd;
	}
}


bool MLocator::IsLiveUDP( const MLocatorUDPInfo* pRecvUDPInfo, u64 dwEventTime )
{
	if( 0 == pRecvUDPInfo ) return false;

	if( GetLocatorConfig()->GetUDPLiveTime() > (dwEventTime - pRecvUDPInfo->GetUseStartTime()) )
		return true;

	return false;
}


bool MLocator::IsOverflowedNormalUseCount( const MLocatorUDPInfo* pRecvUDPInfo )
{
	if( 0 == pRecvUDPInfo ) return false;

	if( GetLocatorConfig()->GetMaxFreeUseCountPerLiveTime() > pRecvUDPInfo->GetTotalUseCount() )
		return false;
	
	return true;
}


const int MLocator::MakeCmdPacket( char* pOutPacket, const int nMaxSize, MCommand* pCmd )
{
	if( (0 == pOutPacket) || (0 > nMaxSize) || (0 == pCmd) ) 
		return -1;

	MCommandMsg* pMsg = reinterpret_cast< MCommandMsg* >( pOutPacket );

	const auto nCmdSize = nMaxSize - MPACKET_HEADER_SIZE;

	pMsg->Buffer[ 0 ] = 0;
	pMsg->nCheckSum = 0;

	if( pCmd->m_pCommandDesc->IsFlag(MCCT_NON_ENCRYPTED) )
	{
		pMsg->nMsg = MSGID_RAWCOMMAND;

		const int nGetCmdSize = pCmd->GetData( pMsg->Buffer, nCmdSize );
		if( nGetCmdSize != nCmdSize )
			return -1;

		pMsg->nSize		= static_cast< unsigned int >( MPACKET_HEADER_SIZE ) + nGetCmdSize;
		pMsg->nCheckSum = MBuildCheckSum(pMsg, pMsg->nSize);
	}
	else
	{
		ASSERT( 0 && "암호화된 커맨드 처리는 없음.\n" );
		return -1;
	}

	return pMsg->nSize;
}


void MLocator::SendCommandByUDP( u32 dwIP, int nPort, MCommand* pCmd )
{
	const int nPacketSize = CalcPacketSize( pCmd );
	char* pszPacketBuf = new char[ nPacketSize ];
	if( 0 != pszPacketBuf ) 
	{
		const int nMakePacketSize = MakeCmdPacket( pszPacketBuf, nPacketSize, pCmd );
		if( nPacketSize == nMakePacketSize )
		{
			if( !m_pSafeUDP->Send(dwIP, nPort, pszPacketBuf, nMakePacketSize) )
			{
				delete [] pszPacketBuf;
				// mlog( "MLocator::SendCommandByUDP - fail:%u.\n", dwIP );
			}
		}
		else
		{
			delete [] pszPacketBuf;
			ASSERT( 0 && "Packet을 만드는데 문제가 있음." );
		}
	}
}


void MLocator::OnRegisterCommand(MCommandManager* pCommandManager)
{
	if( 0 != pCommandManager )
		MAddSharedCommandTable( pCommandManager, static_cast<MSharedCommandType::Type>(0) );
}


void MLocator::UpdateUDPManager(u64 dwEventTime )
{
	if( GetLocatorConfig()->GetUpdateUDPManagerElapsedTime() < (dwEventTime - GetLastUDPManagerUpdateTime()) )
	{
		GetSendUDPManager().SafeClearElapsedLiveTimeUDP( GetLocatorConfig()->GetUpdateUDPManagerElapsedTime(), dwEventTime );
		GetBlockUDPManager().SafeClearElapsedLiveTimeUDP( GetLocatorConfig()->GetBlockTime(), dwEventTime );

		UpdateLastUDPManagerUpdateTime( dwEventTime );
	}
}


void MLocator::UpdateLocatorLog(u64 dwEventTime )
{
	if( GetLocatorConfig()->GetElapsedTimeUpdateLocatorLog() < (dwEventTime - GetLocatorStatistics().GetLastUpdatedTime()) )
	{
#ifdef MFC
		GetLocatorDBMgr()->InsertLocatorLog( GetLocatorConfig()->GetLocatorID(),
			GetLocatorStatistics().GetCountryStatistics() );
#endif
		if( 0 != GetServerStatusMgr() )
		{
			GetLocatorStatistics().SetDeadServerCount( GetServerStatusMgr()->GetDeadServerCount() );
			GetLocatorStatistics().SetLiveServerCount( GetServerStatusMgr()->GetLiveServerCount() );
		}
		else
		{
			GetLocatorStatistics().SetDeadServerCount( 0 );
			GetLocatorStatistics().SetLiveServerCount( 0 );
		}
		GetLocatorStatistics().SetLastUpdatedTime( dwEventTime );
		GetLocatorStatistics().Reset();
	}
}


void MLocator::UpdateCountryCodeFilter(u64 dwEventTime) {}

void MLocator::UpdateLogManager()
{
	GetLogManager().SafeWriteMLog();
	GetLogManager().SafeReset();
}


void MLocator::FlushRecvQueue(u64 dwEventTime)
{
	MUDPManager& RecvUDPMgr = GetRecvUDPManager();
	MUDPManager& SendUDPMgr = GetSendUDPManager();

	MLocatorUDPInfo* pRecvUDPInfo;
	MLocatorUDPInfo* pSendUDPInfo;

	string	strCountryCode;
	string	strRoutingURL;
	string	strComment;

    RecvUDPMgr.Lock();
	while( 0 != (pRecvUDPInfo = RecvUDPMgr.SafePopFirst()) )
	{
		if( !SendUDPMgr.Insert(pRecvUDPInfo->GetIP(), pRecvUDPInfo) )
		{
			pSendUDPInfo = SendUDPMgr.Find( pRecvUDPInfo->GetIP() );
			if( 0 != pSendUDPInfo )
				pSendUDPInfo->IncreaseUseCount( pRecvUDPInfo->GetUseCount() );
			delete pRecvUDPInfo;
		}
	}
	RecvUDPMgr.Unlock();

	for( SendUDPMgr.SetBegin(); 0 != (pSendUDPInfo = SendUDPMgr.GetCurPosUDP()); )
	{
		if( 0 < pSendUDPInfo->GetUseCount() )
		{
			if( IsOverflowedNormalUseCount(pSendUDPInfo) )
			{
				if( !IsBlocker(pSendUDPInfo->GetIP(), dwEventTime) )
				{
					pSendUDPInfo->SetUseCount( 0 );

					if( !GetBlockUDPManager().SafeInsert(pSendUDPInfo->GetIP(), pSendUDPInfo->GetPort(), dwEventTime) )
					{
						mlog( "fail to block udp(%s) time:%s\n", 
							pSendUDPInfo->GetStrIP().c_str(), MGetStrLocalTime().c_str() );
					}

					GetLocatorStatistics().IncreaseBlockCount();
					mlog( "Block. IP(%s), time:%s\n", pSendUDPInfo->GetStrIP().c_str(), MGetStrLocalTime().c_str() );
				}
				SendUDPMgr.MoveNext();
				continue;
			}

			ResponseServerStatusInfoList(pSendUDPInfo->GetIP(), pSendUDPInfo->GetPort());

			pSendUDPInfo->IncreaseUsedCount( pSendUDPInfo->GetUseCount() );
			pSendUDPInfo->SetUseCount( 0 );
			IncreaseSendCount();

			
		}
		SendUDPMgr.MoveNext();
	}
}

MCommand* MLocator::CreateCommand(int nCmdID, const MUID& TargetUID)
{
	return new MCommand(m_CommandManager.GetCommandDescByID(nCmdID), TargetUID, m_This);
}

void MLocator::DumpLocatorStatusInfo()
{
	mlog( "\n======================================================\n" );
	mlog( "Locator Status Info.\n" );

	mlog( "Recv UDP Manager Status Info\n" );
	GetRecvUDPManager().Lock();
	GetRecvUDPManager().DumpStatusInfo();
	GetRecvUDPManager().Unlock();

	mlog( "Send UDP Manager Status Info\n" );
	GetSendUDPManager().Lock();
	GetSendUDPManager().DumpStatusInfo();
	GetSendUDPManager().Unlock();

	mlog( "Block UDP Manager Status Info\n" );
	GetBlockUDPManager().Lock();
	GetBlockUDPManager().DumpStatusInfo();
	GetBlockUDPManager().DumpUDPInfo();
	GetBlockUDPManager().Unlock();
	mlog( "======================================================\n\n" );
}


void MLocator::UpdateLocatorStatus(u64 dwEventTime)
{
	if( GetLocatorConfig()->GetMaxElapsedUpdateServerStatusTime() < 
		(dwEventTime - GetLastLocatorStatusUpdatedTime()) )
	{
#ifdef MFC
		if( 0 == m_pDBMgr )
			return;

		if( !m_pDBMgr->UpdateLocaterStatus( GetLocatorConfig()->GetLocatorID(), 
			GetRecvCount(), 
			GetSendCount(), 
			static_cast<u32>(GetBlockUDPManager().size()), 
			GetDuplicatedCount() ) )
		{
			mlog( "fail to update locator status.\n" );
		}
#endif

		ResetRecvCount();
		ResetSendCount();
		ResetDuplicatedCount();

		UpdateLastLocatorStatusUpdatedTime( dwEventTime );
	}
}

#ifdef _DEBUG

void MLocator::InitDebug()
{
	GetRecvUDPManager().m_strExDbgInfo = "Name:RecvUDPManager";
	GetSendUDPManager().m_strExDbgInfo = "Name:SendUDPManager";
	GetBlockUDPManager().m_strExDbgInfo = "Name:BlockUDPManager";
}


void MLocator::TestDo()
{
	if( 0 != m_pDBMgr )
	{
		if( IsElapedServerStatusUpdatedTime(timeGetTime()) )
		{
		}
	}
}

void MLocator::DebugOutput( void* vp )
{
	return; 

	MServerStatusMgr ssm = *( reinterpret_cast<MServerStatusMgr*>(vp) );

	int nSize = ssm.GetSize();

	char szBuf[ 1024 ];

	DMLog( "\nStart Debug Output-------------------------------------------------\n" );

	for( int i = 0; i < nSize; ++i )
	{
		sprintf_safe( szBuf, "dwIP:%u, Port:%d, ServerID:%d, Time:%s, Live:%d (%d/%d)\n",
			ssm[i].GetIP(), ssm[i].GetPort(), ssm[i].GetID(), ssm[i].GetLastUpdatedTime().c_str(), 
			ssm[i].IsLive(), 
			ssm[i].GetCurPlayer(), ssm[i].GetMaxPlayer() );

		DMLog( szBuf );

		char szVal[ 3 ];
		strcpy_safe( szVal, &ssm[i].GetLastUpdatedTime()[11] );
		const int nHour = atoi( szVal );
		strcpy_safe( szVal, &ssm[i].GetLastUpdatedTime()[14] );
		const int nMin = atoi( szVal );

		sprintf_safe( szBuf,  "%d:%d\n", nHour, nMin );

		DMLog( szBuf );
	}

	DMLog( "End Debug Output---------------------------------------------------\n\n" );
}
#endif