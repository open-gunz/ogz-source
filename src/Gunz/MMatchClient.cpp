#include "stdafx.h"
#include "MMatchClient.h"
#include "MErrorTable.h"
#include "MSharedCommandTable.h"
#include "MDebug.h"
#include "MBlobArray.h"
#include "MMatchUtil.h"
#include "MMatchNotify.h"

MMatchClient* g_pMatchClient = NULL;
MMatchClient* GetMainMatchClient() { return g_pMatchClient; }

void MakeTCPCommandSerialNumber(MCommand* pCmd)
{
	static unsigned char nSerial = 0;
	nSerial++;
	pCmd->m_nSerialNumber = nSerial;
}


void MakeUDPCommandSerialNumber(MCommand* pCmd)
{
	static unsigned char nSerial = 0;
	nSerial++;
	pCmd->m_nSerialNumber = nSerial;
}

#define MAX_PING_TRY_COUNT		3

int MMatchPeerInfo::GetPing(unsigned int nCurrTime)
{
	if ((int)m_nLastPongTime - (int)m_nLastPingTime < 0)
	{
		int nDelay = nCurrTime - m_nLastPingTime;
		if (nDelay >= MAX_PING && m_nPingTryCount >= MAX_PING_TRY_COUNT)
		{
			return MAX_PING;
		}
	}

	return m_nPing;
}

void MMatchPeerInfo::UpdatePing(unsigned int nTime, int nPing)
{
	m_nLastPongTime = nTime;
	m_nPingTryCount = 0;
	m_nPing = nPing;
}

void MMatchPeerInfo::SetLastPingTime(unsigned int nTime)
{
	if ((int)m_nLastPongTime - (int)m_nLastPingTime >= 0)
		m_nLastPingTime = nTime;

	m_nPingTryCount++;
}

MMatchPeerInfoList::MMatchPeerInfoList() = default;

MMatchPeerInfoList::~MMatchPeerInfoList()
{
	Clear();
	mlog("PeerInfoList Released\n");
}

bool MMatchPeerInfoList::Delete(MMatchPeerInfo* pPeerInfo)
{
	std::lock_guard<MCriticalSection> lock{ m_csLock };

	if (pPeerInfo == nullptr)
		return false;

	auto it = MUIDMap.find(pPeerInfo->uidChar);
	if (it == MUIDMap.end())
		return false;

	IPnPortMap.erase(IPnPort{ pPeerInfo });
	MUIDMap.erase(it);
	delete pPeerInfo;

	return true;
}

void MMatchPeerInfoList::Clear()
{
	std::lock_guard<MCriticalSection> lock{ m_csLock };

	for (auto* PeerInfo : MakePairValueAdapter(MUIDMap))
		delete PeerInfo;

	MUIDMap.clear();
	IPnPortMap.clear();
}

void MMatchPeerInfoList::Add(MMatchPeerInfo* pPeerInfo)
{
	std::lock_guard<MCriticalSection> lock{ m_csLock };

	MUIDMap.emplace(pPeerInfo->uidChar, pPeerInfo);
	IPnPortMap.emplace(IPnPort{ pPeerInfo }, pPeerInfo);
}

MMatchPeerInfo* MMatchPeerInfoList::Find(const MUID& uidChar)
{
	std::lock_guard<MCriticalSection> lock{ m_csLock };

	auto it = MUIDMap.find(uidChar);
	if (it == MUIDMap.end())
		return nullptr;

	return it->second;
}

MUID MMatchPeerInfoList::FindUID(u32 dwIP, int nPort)
{
	std::lock_guard<MCriticalSection> lock{ m_csLock };

	auto it = IPnPortMap.find(IPnPort{ dwIP, static_cast<u16>(nPort) });
	if (it == IPnPortMap.end())
		return{ 0, 0 };

	return it->second->uidChar;
}

MMatchClient::MMatchClient()
{
	g_pMatchClient = this;
	SetServerAddr("", 6000);
	SetServerPeerPort(7777);

	m_uidAgentServer = MUID(0,0);
	m_uidAgentClient = MUID(0,0);

	SetAgentAddr("", 6000);
	SetAgentPeerPort(7776);

	m_szServerName[0] = 0;
	m_nServerMode = MSM_NORMAL_;
}

MMatchClient::~MMatchClient()
{
	m_SafeUDP.SetCustomRecvCallback(NULL);
	m_SafeUDP.Destroy();
	ClearObjCaches();
}

MMatchClient::CreationResult MMatchClient::Create(u16 nUDPPort)
{
	if (MCommandCommunicator::Create() == false)
	{
		return{ -1, "MCommandCommunicator::Create failed" };
	}

	if (m_SafeUDP.Create(true, nUDPPort, false) == false)
	{
		auto ErrorCode = MSocket::GetLastError();
		std::string ErrorMessage;
		if (ErrorCode == MSocket::Error::AddressInUse)
		{
			NotifyMessage(MATCHNOTIFY_NETWORK_PORTINUSE, &ErrorMessage);
		}
		else
		{
			MSocket::GetErrorString(ErrorCode, ErrorMessage);
		}
		return{ int(ErrorCode), ErrorMessage };
	}

	m_SafeUDP.SetCustomRecvCallback(UDPSocketRecvEvent);
	SetUDPTestProcess(false);

	m_AgentSocket.SetCallbackContext(this);
	m_AgentSocket.SetConnectCallback(SocketConnectEvent);
	m_AgentSocket.SetDisconnectCallback(SocketDisconnectEvent);
	m_AgentSocket.SetRecvCallback(SocketRecvEvent);
	m_AgentSocket.SetSocketErrorCallback(SocketErrorEvent);

	return{ 0, "" };
}

MUID MMatchClient::GetSenderUIDBySocket(SOCKET socket)
{
	if (m_ClientSocket.GetSocket() == socket)
		return m_Server;
	else if (m_AgentSocket.GetSocket() == socket)
		return GetAgentServerUID();
	else
		return MUID(0,0);
}

bool MMatchClient::OnSockConnect(SOCKET sock)
{
	MClient::OnSockConnect(sock);

	MLog("%s %d\n", m_ClientSocket.GetHost(), m_ClientSocket.GetPort());
	SetServerAddr(m_ClientSocket.GetHost(), m_ClientSocket.GetPort());

	return true;
}
bool MMatchClient::OnSockDisconnect(SOCKET sock)
{
	MClient::OnSockDisconnect(sock);
	OutputMessage("TCP Socket disconnected.", MZMOM_LOCALREPLY);

	return true;
}
bool MMatchClient::OnSockRecv(SOCKET sock, char* pPacket, u32 dwSize)
{
	MClient::OnSockRecv(sock, pPacket, dwSize);

	return true;
}
void MMatchClient::OnSockError(SOCKET sock, SOCKET_ERROR_EVENT ErrorEvent, int &ErrorCode)
{
	MClient::OnSockError(sock, ErrorEvent, ErrorCode);
	MLog("Socket Error: %d\n", ErrorCode);
	if (ErrorCode == WSAECONNABORTED)
		OutputMessage(MZMOM_LOCALREPLY, "Disconnected", ErrorCode);
	else
		OutputMessage(MZMOM_LOCALREPLY, "TCP Socket Error(Code =  %d)", ErrorCode);
}

bool MMatchClient::OnCommand(MCommand* pCommand)
{
	bool ret = MClient::OnCommand(pCommand);

	if ( (pCommand->m_pCommandDesc->IsFlag(MCDT_PEER2PEER)==true) )
	{
		if (pCommand->GetSenderUID() == GetUID())
		{
			pCommand->SetSenderUID(GetPlayerUID());
		}
		else
		{
			MMatchPeerInfo* pPeer = FindPeer(pCommand->GetSenderUID());
			if (pPeer)
			{
				if (!pPeer->CheckCommandValidate(pCommand))
				{
					if (pCommand->m_pCommandDesc->IsFlag(MCCT_NON_ENCRYPTED) == false)
					{
						return false;
					}
				}
			}
		}
	}


	switch(pCommand->GetID())
	{
	case MC_MATCH_P2P_COMMAND:
	{
		/*if (PeerToPeer)
			break;*/

		MUID Sender;
		if (pCommand->GetParameter(&Sender, 0, MPT_UID) == false) break;

		MCommandParameter* pParam = pCommand->GetParameter(1);
		if (pParam->GetType() != MPT_BLOB) break;
		void* Blob = pParam->GetPointer();
		auto Size = ((MCmdParamBlob*)pParam)->GetPayloadSize();

		MCommand* pCmd = MakeCmdFromSaneTunnelingBlob(Sender, m_This, Blob, Size);
		if (pCmd == nullptr) break;

		LockRecv();
		m_CommandManager.Post(pCmd);
		UnlockRecv();

		if (pCmd->GetID() != MC_PEER_BASICINFO_RG)
			DMLog("Received tunnelled P2P command ID %x\n", pCmd->GetID());
	}
	break;
		case MC_MATCH_RESPONSE_LOGIN:
			{
				int nResult;
				char nServerMode;
				unsigned char nUGradeID, nPGradeID;
				MUID uidPlayer;
				char szServerName[256], szAccountID[256];
				char szRandomValue[256];

				pCommand->GetParameter(&nResult,		0, MPT_INT);
				pCommand->GetParameter(szServerName,	1, MPT_STR, sizeof(szServerName) );
				pCommand->GetParameter(&nServerMode,	2, MPT_CHAR);
				pCommand->GetParameter(szAccountID,		3, MPT_STR, sizeof(szAccountID) );
				pCommand->GetParameter(&nUGradeID,		4, MPT_UCHAR);
				pCommand->GetParameter(&nPGradeID,		5, MPT_UCHAR);
				pCommand->GetParameter(&uidPlayer,		6, MPT_UID);
				pCommand->GetParameter(szRandomValue,	7, MPT_STR, sizeof(szRandomValue) );

				OnResponseMatchLogin(
					pCommand->GetSenderUID(),
					nResult, szServerName,
					MMatchServerMode(nServerMode),
					szAccountID, MMatchUserGradeID(nUGradeID),
					MMatchPremiumGradeID(nPGradeID), uidPlayer,
					szRandomValue);
			}
			break;
		case MC_MATCH_OBJECT_CACHE:
			{
				unsigned char nType;
				pCommand->GetParameter(&nType, 0, MPT_UCHAR);
				MCommandParameter* pParam = pCommand->GetParameter(1);
				if(pParam->GetType()!=MPT_BLOB) break;
				void* pBlob = pParam->GetPointer();
				int nCount = MGetBlobArrayCount(pBlob);
				OnObjectCache((unsigned int)nType, pBlob, nCount);
			}
			break;
		case MC_AGENT_RESPONSE_LOGIN:
			{
				OnResponseAgentLogin();
			}
			break;
		case MC_AGENT_LOCATETO_CLIENT:
			{
				MUID uidAgent;
				char szIP[64];
				int nPort, nUDPPort;

				if (pCommand->GetParameter(&uidAgent, 0, MPT_UID) == false) break;
				if (pCommand->GetParameter(szIP, 1, MPT_STR, sizeof(szIP) ) == false) break;
				if (pCommand->GetParameter(&nPort, 2, MPT_INT) == false) break;
				if (pCommand->GetParameter(&nUDPPort, 3, MPT_INT) == false) break;

				OnLocateAgentToClient(uidAgent, szIP, nPort, nUDPPort);
			}
			break;
		case MC_AGENT_TUNNELING_TCP:
			{
				MUID uidSender, uidReceiver;
				if (pCommand->GetParameter(&uidSender, 0, MPT_UID)==false) break;
				if (pCommand->GetParameter(&uidReceiver, 1, MPT_UID)==false) break;

				MCommandParameter* pParam = pCommand->GetParameter(2);
				if (pParam->GetType()!=MPT_BLOB) break;
				void* pBlob = pParam->GetPointer();
				int nCount = MGetBlobArrayCount(pBlob);

				OnTunnelingTCP(uidSender, pBlob, nCount);
			}
			break;
		case MC_AGENT_TUNNELING_UDP:
			{
				MUID uidSender, uidReceiver;
				if (pCommand->GetParameter(&uidSender, 0, MPT_UID)==false) break;
				if (pCommand->GetParameter(&uidReceiver, 1, MPT_UID)==false) break;

				MCommandParameter* pParam = pCommand->GetParameter(2);
				if (pParam->GetType()!=MPT_BLOB) break;
				void* pBlob = pParam->GetPointer();
				int nCount = MGetBlobArrayCount(pBlob);

				OnTunnelingUDP(uidSender, pBlob, nCount);
			}
			break;
		case MC_AGENT_ALLOW_TUNNELING_TCP:
			{
				OnAllowTunnelingTCP();
			}
			break;
		case MC_AGENT_ALLOW_TUNNELING_UDP:
			{
				OnAllowTunnelingUDP();
			}
			break;
		case MC_AGENT_ERROR:
			{
				int nError;
				if (pCommand->GetParameter(&nError, 0, MPT_INT) == false) break;

				OnAgentError(nError);
			}
			break;

		case MC_VERSION:
			OutputMessage("MAIET MatchClient Version", MZMOM_LOCALREPLY);
			break;
		case MC_NET_ENUM:
			break;
		case MC_NET_RESPONSE_INFO:
			break;
		case MC_PEER_UDPTEST:
			{
				OnUDPTest(pCommand->GetSenderUID());
			}
			break;
		case MC_PEER_UDPTEST_REPLY:
			{
				OnUDPTestReply(pCommand->GetSenderUID());
			}
			break;
		case MC_AGENT_DEBUGTEST:
			{

			}
			break;
		default:
			if (!ret)
			{
				return false;
			}
	}
	return true;
}

void MMatchClient::OnRegisterCommand(MCommandManager* pCommandManager)
{
	MClient::OnRegisterCommand(pCommandManager);
	MAddSharedCommandTable(pCommandManager, MSharedCommandType::Client);
}

void MMatchClient::OutputLocalInfo(void)
{
	OutputMessage("MAIET Match Client", MZMOM_LOCALREPLY);
	OutputMessage(MZMOM_LOCALREPLY, "UID : %u:%u", m_This.High, m_This.Low);
	OutputMessage(MZMOM_LOCALREPLY, "Connected Communicator : %u:%u", m_Server.High, m_Server.Low);
}

int MMatchClient::OnConnected(SOCKET sock, MUID* pTargetUID, MUID* pAllocUID, unsigned int nTimeStamp)
{
#ifdef _DEBUG
	// UDP Socket Log
	SOCKADDR_IN SockAddr;	int nErrorCode=0;
	int nAddrLen = sizeof(SOCKADDR_IN);
	if (getsockname(m_SafeUDP.GetLocalSocket(), (SOCKADDR*)&SockAddr, &nAddrLen) == SOCKET_ERROR)
		nErrorCode = WSAGetLastError();
	auto IP = GetIPv4String(SockAddr.sin_addr);
	unsigned int nPort = ntohs(SockAddr.sin_port);
	mlog("UDP Address = %s:%d \n", IP.c_str(), nPort);
#endif

	if (sock == m_ClientSocket.GetSocket()) {
		int ret = MClient::OnConnected(sock, pTargetUID, pAllocUID, nTimeStamp);

		return ret;
	} else if (sock == m_AgentSocket.GetSocket()) {
		OnAgentConnected(*pTargetUID, *pAllocUID);
		return MOK;
	} else {
		return MERR_UNKNOWN;
	}
}

void MMatchClient::OnAgentConnected(const MUID& uidAgentServer, const MUID& uidAlloc)
{
	m_uidAgentServer = uidAgentServer;
	m_uidAgentClient = uidAlloc;

	SetAllowTunneling(false);

	MPacketCrypterKey key;
	MMakeSeedKey(&key, uidAgentServer, uidAlloc, 0);
	m_AgentPacketCrypter.InitKey(&key);
}

int MMatchClient::OnResponseMatchLogin(const MUID& uidServer, int nResult, const char* szServerName,
									   const MMatchServerMode nServerMode,
									   const char* szAccountID,
									   const MMatchUserGradeID nUGradeID,
									   const MMatchPremiumGradeID nPGradeID,
									   const MUID& uidPlayer,
									   const char* szRandomValue)
{
	m_uidServer = uidServer;
	m_uidPlayer = uidPlayer;

	strcpy_safe(m_szServerName, szServerName);
	m_nServerMode = nServerMode;

	return MOK;
}


void MMatchClient::OnObjectCache(unsigned int nType, void* pBlob, int nCount)
{
	if (nType == MATCHCACHEMODE_REPLACE) {
		for(int i=0; i<nCount; i++){
			MMatchObjCache* pCache = (MMatchObjCache*)MGetBlobArrayElement(pBlob, i);
			ReplaceObjCache(pCache);
		}
	} else {
		if (nType == MATCHCACHEMODE_UPDATE)
			ClearObjCaches();

		for(int i=0; i<nCount; i++){
			MMatchObjCache* pCache = (MMatchObjCache*)MGetBlobArrayElement(pBlob, i);
			if (nType==MATCHCACHEMODE_ADD || nType==MATCHCACHEMODE_UPDATE)
				UpdateObjCache(pCache);
			else if (nType == MATCHCACHEMODE_REMOVE)
				RemoveObjCache(pCache->GetUID());
		}
	}
}

void MMatchClient::CastStageBridgePeer(const MUID& uidChar, const MUID& uidStage)
{
	auto* Desc = m_CommandManager.GetCommandDescByID(MC_MATCH_BRIDGEPEER);
	auto* pCmd = new MCommand(Desc, GetServerUID(), m_This);
	pCmd->AddParameter(new MCommandParameterUID(uidChar));
	pCmd->AddParameter(new MCommandParameterUInt(0)); // IP
	pCmd->AddParameter(new MCommandParameterUInt(0)); // Port

	SendCommandByUDP(pCmd, GetServerIP(), GetServerPeerPort());

	delete pCmd;
}

void MMatchClient::OnUDPTest(const MUID& uidChar)
{
	auto* pPeer = FindPeer(uidChar);
	if (!pPeer)
		return;

	auto* pCmd = CreateCommand(MC_PEER_UDPTEST_REPLY, uidChar);
	SendCommandByUDP(pCmd, pPeer->szIP, pPeer->nPort);
	delete pCmd;
}

void MMatchClient::OnUDPTestReply(const MUID& uidChar)
{
	DMLog("[%d:%d] UDP_TEST_REPLY: from (%d:%d) \n",
		GetPlayerUID().High, GetPlayerUID().Low, uidChar.High, uidChar.Low);

	MMatchPeerInfo* pPeer = FindPeer(uidChar);
	if (pPeer) {
		pPeer->SetUDPTestResult(true);
		pPeer->StopUDPTest();
	}
}

void MMatchClient::UpdateUDPTestProcess()
{
	int nProcessCount = 0;
	for (auto* pPeer : MakePairValueAdapter(m_Peers.MUIDMap))
	{
		if (pPeer->GetProcess()) {
			pPeer->UseTestCount();
			if (pPeer->GetTestCount() <= 0) {
				pPeer->StopUDPTest();

				OnStopUDPTest(pPeer->uidChar);

#ifdef MATCHAGENT
				MCommand* pCmd = CreateCommand(MC_MATCH_REQUEST_PEER_RELAY, GetServerUID());
				pCmd->AddParameter(new MCmdParamUID(GetPlayerUID()));
				pCmd->AddParameter(new MCmdParamUID(pPeer->uidChar));
				Post(pCmd);
#endif
			} else {
				nProcessCount++;
			}
		}
	}
	if (nProcessCount <= 0)
		SetUDPTestProcess(false);
}

void MMatchClient::OnResponseAgentLogin()
{
	MCommand* pCmd = CreateCommand(MC_AGENT_PEER_BINDTCP, GetAgentServerUID());
	pCmd->AddParameter(new MCmdParamUID(GetPlayerUID()));
	Post(pCmd);

	mlog("Logged in Agent, Bind TCP \n");
}

void MMatchClient::OnLocateAgentToClient(const MUID& uidAgent, char* szIP, int nPort, int nUDPPort)
{
	SetAgentAddr(szIP, nPort);
	SetAgentPeerPort(nUDPPort);

	if (GetBridgePeerFlag() == false) {
		AgentConnect(NULL, szIP, nPort);
		mlog("Connect to Agent by TCP (%s:%d) \n", szIP, nPort);
	} else {
		StartAgentPeerConnect();
		mlog("Connect to Agent by UDP (%s:%d) \n", szIP, nPort);
	}
}

MCommand* MMatchClient::MakeCmdFromTunnelingBlob(const MUID& uidSender, void* pBlob, int nBlobArrayCount)
{
	if (nBlobArrayCount != 1)
	{
		mlog("MakeCmdFromTunnelingBlob: BlobArrayCount is not 1\n");
		return NULL;
	}

	char* pPacket = (char*)MGetBlobArrayElement(pBlob, 0);

	int nSize = MGetBlobArraySize(pBlob) - (sizeof(int) * 2);
	if ((nSize <= 0) || (nSize >= MAX_BLOB_SIZE))
	{
		mlog("MakeCmdFromTunnelingBlob: Blob Size Error(size = %d)\n", nSize);
		return NULL;
	}

	char* pData = new char[nSize];

	if (!m_PeerPacketCrypter.Decrypt(pPacket, nSize, pData, nSize))
	{
		delete[] pData;
		return NULL;
	}


	MCommand* pCmd = new MCommand();
	if (!pCmd->SetData(pData, &m_CommandManager))
	{
		delete[] pData;
		delete pCmd;
		return NULL;
	}

	delete[] pData;

	pCmd->m_Sender = uidSender;
	pCmd->m_Receiver = m_This;

	MMatchPeerInfo* pPeer = FindPeer(uidSender);
	if (pPeer == NULL)
	{
		delete pCmd;
		return NULL;
	}

	return pCmd;
}

void MMatchClient::OnTunnelingTCP(const MUID& uidSender, void* pBlob, int nCount)
{
	MCommand* pCmd = MakeCmdFromTunnelingBlob(uidSender, pBlob, nCount);
	if (pCmd == NULL) return;

	LockRecv();
	m_CommandManager.Post(pCmd);
	UnlockRecv();
}

void MMatchClient::OnTunnelingUDP(const MUID& uidSender, void* pBlob, int nCount)
{
	MCommand* pCmd = MakeCmdFromTunnelingBlob(uidSender, pBlob, nCount);
	if (pCmd == NULL) return;

	LockRecv();
	m_CommandManager.Post(pCmd);
	UnlockRecv();
}

void MMatchClient::OnAllowTunnelingTCP()
{
	SetAllowTunneling(true);
}

void MMatchClient::OnAllowTunnelingUDP()
{
	SetAllowTunneling(true);
	SetAgentPeerFlag(true);
	mlog("TUNNELING_UDP_ALLOWED \n");
}

void MMatchClient::OnAgentError(int nError)
{
}

void MMatchClient::SendCommand(MCommand* pCommand)
{
	if (pCommand->m_pCommandDesc->IsFlag(MCDT_PEER2PEER)==true)
	{
		MakeUDPCommandSerialNumber(pCommand);

		if (GetBridgePeerFlag() == false) {
			SendCommandByTunneling(pCommand);
		} else {
			if (!PeerToPeer)
			{
				SendCommandByMatchServerTunneling(pCommand);
			}
			else
			{
				if (pCommand->GetReceiverUID() == MUID(0, 0)) {	// BroadCasting
					int nTunnelingCount = 0;

					for (auto* pPeerInfo : MakePairValueAdapter(m_Peers.MUIDMap))
					{
						if (pPeerInfo->uidChar == MUID(0, 0) ||
							pPeerInfo->uidChar != GetPlayerUID())
						{
#ifdef MATCHAGENT
							if ((pPeerInfo->GetProcess() == false) &&
								(pPeerInfo->GetUDPTestResult() == false))
								nTunnelingCount++;
							else
								SendCommandByUDP(pCommand, pPeerInfo->szIP, pPeerInfo->nPort);
#else
							if (!pPeerInfo->GetUDPTestResult())
								SendCommandByMatchServerTunneling(pCommand, pPeerInfo->uidChar);
							else
								SendCommandByUDP(pCommand, pPeerInfo->szIP, pPeerInfo->nPort);
#endif
						}
					}

#ifdef MATCHAGENT
					if (nTunnelingCount > 0) {
						SendCommandByTunneling(pCommand);
					}
#endif
				}
				else {
					MMatchPeerInfo* pPeerInfo = FindPeer(pCommand->GetReceiverUID());
					if (pPeerInfo) {
#ifdef MATCHAGENT
						if ((pPeerInfo->GetProcess() == false) &&
							(pPeerInfo->GetUDPTestResult() == false))
							SendCommandByTunneling(pCommand);
#else
						if (pCommand->GetID() != MC_PEER_BASICINFO_RG)
							DMLog("Sending direct command ID %d from %d to %d\n",
								pCommand->GetID(), pCommand->GetSenderUID().Low,
								pCommand->GetReceiverUID().Low);
						if (!pPeerInfo->GetUDPTestResult())
							SendCommandByMatchServerTunneling(pCommand, pPeerInfo->uidChar);
#endif
						else
							SendCommandByUDP(pCommand, pPeerInfo->szIP, pPeerInfo->nPort);
					}
				}
			}
		}
	}
	else
	{
		if ( (pCommand->GetReceiverUID() != MUID(0,0)) &&
			 (pCommand->GetReceiverUID() == GetAgentServerUID()) )
		{
			bool bResult = SendCommandToAgent(pCommand);

			if (pCommand->GetID() == MC_AGENT_PEER_BINDTCP) {
				if (bResult)
					OutputDebugString("SendCommand(AGENT_PEER_BINDTCP) \n");
				else
					OutputDebugString("SendCommand(AGENT_PEER_FAILED) \n");
			}
		} else {
			MakeTCPCommandSerialNumber(pCommand);
			MClient::SendCommand(pCommand);
		}
	}
}

bool MMatchClient::SendCommandToAgent(MCommand* pCommand)
{
	int nPacketSize = CalcPacketSize(pCommand);
	char* pSendBuf = new char[nPacketSize];

	int size = MakeCmdPacket(pSendBuf, nPacketSize, &m_AgentPacketCrypter, pCommand);

	if (size > 0)
	{
		return m_AgentSocket.Send(pSendBuf, size);
	}
	else
	{
		delete [] pSendBuf;
		return false;
	}
}

void MMatchClient::SendCommandByUDP(MCommand* pCommand, const char* szIP, int nPort)
{
	int nPacketSize = CalcPacketSize(pCommand);
	char* pSendBuf = new char[nPacketSize];

	int nSize = MakeCmdPacket(pSendBuf, nPacketSize, &m_PeerPacketCrypter, pCommand);

	_ASSERT(nPacketSize > 0 && nPacketSize == nSize);

	if (nSize > 0)
	{
		m_SafeUDP.Send(szIP, nPort, pSendBuf, nSize);
	}
	else
	{
		_ASSERT(0);
		delete [] pSendBuf;
	}
}

bool MMatchClient::MakeTunnelingCommandBlob(MCommand* pWrappingCmd, MCommand* pSrcCmd)
{
	int nCmdSize = pSrcCmd->GetSize();
	if (nCmdSize == 0)
	{
		return false;
	}

	char* pCmdData = new char[nCmdSize];
	int nSize = pSrcCmd->GetData(pCmdData, nCmdSize);
	if (nSize != nCmdSize)
	{
		delete [] pCmdData; return false;
	}

	if (!m_PeerPacketCrypter.Encrypt(pCmdData, nSize))
	{
		delete [] pCmdData; return false;
	}

	void* pBlob = MMakeBlobArray(nSize, 1);
	char* pCmdBlock = (char*)MGetBlobArrayElement(pBlob, 0);
	CopyMemory(pCmdBlock, pCmdData, nSize);

	pWrappingCmd->AddParameter(new MCmdParamBlob(pBlob, MGetBlobArraySize(pBlob)));

	MEraseBlobArray(pBlob);
	delete [] pCmdData;

	return true;
}

void MMatchClient::SendCommandByTunneling(MCommand* pCommand)
{
	if (!GetAllowTunneling())
		return;

	if (!GetBridgePeerFlag()) {
		MCommand* pCmd = CreateCommand(MC_AGENT_TUNNELING_TCP, GetAgentServerUID());
		pCmd->AddParameter(new MCmdParamUID(GetPlayerUID()));
		pCmd->AddParameter(new MCmdParamUID(pCommand->GetReceiverUID()));

		if (!MakeTunnelingCommandBlob(pCmd, pCommand))
		{
			delete pCmd; pCmd = NULL; return;
		}

		SendCommandToAgent(pCmd);
		delete pCmd;
	}
	else {
		MCommand* pCmd = CreateCommand(MC_AGENT_TUNNELING_UDP, GetAgentServerUID());
		pCmd->AddParameter(new MCmdParamUID(GetPlayerUID()));
		pCmd->AddParameter(new MCmdParamUID(pCommand->GetReceiverUID()));

		if (!MakeTunnelingCommandBlob(pCmd, pCommand))
		{
			delete pCmd; pCmd = NULL; return;
		}

		SendCommandByUDP(pCmd, GetAgentIP(), GetAgentPeerPort());

		delete pCmd;
	}
}

void MMatchClient::SendCommandByMatchServerTunneling(MCommand * pCommand, const MUID & Receiver)
{
	MCommand* pCmd = CreateCommand(MC_MATCH_P2P_COMMAND, GetServerUID());
	pCmd->AddParameter(new MCmdParamUID(Receiver));

	if (!MakeSaneTunnelingCommandBlob(pCmd, pCommand))
	{
		delete pCmd; pCmd = NULL; return;
	}

	Post(pCmd);
}

void MMatchClient::SendCommandByMatchServerTunneling(MCommand* pCommand)
{
	SendCommandByMatchServerTunneling(pCommand, pCommand->GetReceiverUID());
}

bool MMatchClient::UDPSocketRecvEvent(u32 dwIP, WORD wRawPort, char* pPacket, u32 dwSize)
{
	if (GetMainMatchClient() == NULL) return false;
	if (dwSize < sizeof(MPacketHeader)) return false;

	MPacketHeader*	pPacketHeader;
	pPacketHeader = (MPacketHeader*)pPacket;
	int nPacketSize = pPacketHeader->CalcPacketSize(&GetMainMatchClient()->m_PeerPacketCrypter);

	if ((dwSize != nPacketSize) ||
		((pPacketHeader->nMsg != MSGID_COMMAND) && (pPacketHeader->nMsg != MSGID_RAWCOMMAND)) ) return false;

	unsigned int nPort = ntohs(wRawPort);
	GetMainMatchClient()->ParseUDPPacket(&pPacket[sizeof(MPacketHeader)], pPacketHeader, dwIP, nPort);
	return true;
}


bool MMatchClient::DeletePeer(const MUID uid)
{
	MMatchPeerInfo* pPeer = FindPeer(uid);
	if (pPeer != NULL)
	{
		return m_Peers.Delete(pPeer);
	}
	return false;
}

void MMatchClient::AddPeer(MMatchPeerInfo* pPeerInfo)
{
	m_Peers.Add(pPeerInfo);
}

MUID MMatchClient::FindPeerUID(const u32 dwIP, const int nPort)
{
	return m_Peers.FindUID(dwIP, nPort);
}

MMatchPeerInfo* MMatchClient::FindPeer(const MUID& uidChar)
{
	return m_Peers.Find(uidChar);
}

void MMatchClient::ClearPeers()
{
	m_Peers.Clear();
}

void MMatchClient::SetUDPPort(int nPort)
{
	m_SafeUDP.Destroy();
	m_SafeUDP.Create(true, nPort);
}

string MMatchClient::GetObjName(const MUID& uid)
{
	MMatchObjCache* pCache = FindObjCache(uid);
	char szName[32];
	if (pCache && strlen(pCache->GetName()) > 2)
		sprintf_safe(szName, "%s", pCache->GetName());
	else
		sprintf_safe(szName, "%d%d", uid.High, uid.Low);
	string name = szName;
	return name;
}

MMatchObjCache* MMatchClient::FindObjCache(const MUID& uid)
{
	MMatchObjCacheMap::iterator i = m_ObjCacheMap.find(uid);
	if (i == m_ObjCacheMap.end()) return NULL;
	return (*i).second;
}

void MMatchClient::ReplaceObjCache(MMatchObjCache* pCache)
{
	MMatchObjCache* pScanCache = FindObjCache(pCache->GetUID());
	if (pScanCache == NULL) return;

	pScanCache->SetInfo(pCache->GetUID(), pCache->GetName(), pCache->GetClanName(), pCache->GetLevel(),
						pCache->GetUGrade(), pCache->GetPGrade());
	pScanCache->SetCLID(pCache->GetCLID());
	pScanCache->SetEmblemChecksum(pCache->GetEmblemChecksum());
	pScanCache->AssignCostume(pCache->GetCostume());
	pScanCache->SetFlags(pCache->GetFlags());
}

void MMatchClient::UpdateObjCache(MMatchObjCache* pCache)
{
	MMatchObjCache* pScanCache = FindObjCache(pCache->GetUID());
	if (pScanCache != NULL) return;

	MMatchObjCache* pNewCache = new MMatchObjCache;

	pNewCache->SetInfo(pCache->GetUID(), pCache->GetName(), pCache->GetClanName(), pCache->GetLevel(),
					   pCache->GetUGrade(), pCache->GetPGrade());
	pNewCache->SetCLID(pCache->GetCLID());
	pNewCache->SetEmblemChecksum(pCache->GetEmblemChecksum());
	pNewCache->AssignCostume(pCache->GetCostume());
	pNewCache->SetFlags(pCache->GetFlags());

	_ASSERT(m_ObjCacheMap.find(pNewCache->GetUID())==m_ObjCacheMap.end());
	m_ObjCacheMap.Insert(pNewCache->GetUID(), pNewCache);
}

void MMatchClient::RemoveObjCache(const MUID& uid)
{
	MMatchObjCacheMap::iterator i = m_ObjCacheMap.find(uid);
	if (i==m_ObjCacheMap.end()) return;

	MMatchObjCache* pCache = i->second;
	delete pCache;
	m_ObjCacheMap.erase(i); // Channel Cache
}

void MMatchClient::ClearObjCaches()
{
	while( m_ObjCacheMap.begin() != m_ObjCacheMap.end()) {
		MMatchObjCache* pCache = (*m_ObjCacheMap.begin()).second;
		delete pCache;
		m_ObjCacheMap.erase(m_ObjCacheMap.begin());
	}
}

void MMatchClient::ParseUDPPacket(char* pData, MPacketHeader* pPacketHeader, u32 dwIP, unsigned int nPort)
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
					mlog("MMatchClient::ParseUDPPacket() -> SetData Error\n");

					delete pCmd;
					return;
				}

				MUID uidPeer = FindPeerUID(dwIP, nPort);
				if (uidPeer != MUID(0,0))
				{
					pCmd->m_Sender = uidPeer;
				} else {
					sockaddr_in Addr;
					Addr.sin_addr.S_un.S_addr = dwIP;
					Addr.sin_port = nPort;
					auto IP = GetIPv4String(Addr.sin_addr);

					if (IP == GetAgentIP())
					{
						pCmd->m_Sender = GetAgentServerUID();
					}
					else if( (MC_RESPONSE_SERVER_LIST_INFO == pCmd->GetID()) ||
						(MC_RESPONSE_BLOCK_COUNTRY_CODE_IP == pCmd->GetID()) )
					{
					}
					else
					{
						delete pCmd; pCmd = NULL;
						return;
					}
				}

				pCmd->m_Receiver = m_This;

				LockRecv();
				m_CommandManager.Post(pCmd);
				UnlockRecv();
			}
		}
		break;
	case MSGID_COMMAND:
		{
			int nPacketSize = pPacketHeader->CalcPacketSize(&m_PeerPacketCrypter);
			unsigned short nCheckSum = MBuildCheckSum(pPacketHeader, nPacketSize);

			if (pPacketHeader->nCheckSum != nCheckSum) {
				static int nLogCount = 0;
				if (nLogCount++ < 100) {	// Log Flooding
					mlog("MMatchClient::ParseUDPPacket() -> CHECKSUM ERROR(R=%u/C=%u)\n",
						pPacketHeader->nCheckSum, nCheckSum);
				}
				return;
			} else {
				MCommand* pCmd = new MCommand();

				int nCmdSize = nPacketSize - sizeof(MPacketHeader);

				if (!m_PeerPacketCrypter.Decrypt(pData, nCmdSize))
				{
					mlog("MMatchClient::ParseUDPPacket() -> Decrypt Error\n");

					delete pCmd; pCmd = NULL;
					return;
				}

				if (!pCmd->SetData(pData, &m_CommandManager))
				{
					sockaddr_in Addr;
					Addr.sin_addr.S_un.S_addr = dwIP;
					Addr.sin_port = nPort;
					auto IP = GetIPv4String(Addr.sin_addr);

					mlog("MMatchClient::ParseUDPPacket() -> MSGID_COMMAND SetData Error(%s:%d), size=%d\n",
						IP.c_str(), nPort, nCmdSize);

					delete pCmd; pCmd = NULL;
					return;
				}

				MUID uidPeer = FindPeerUID(dwIP, nPort);
				if (uidPeer != MUID(0,0))
				{
					pCmd->m_Sender = uidPeer;
				} else {
					delete pCmd;
					return;
				}

				pCmd->m_Receiver = m_This;

				LockRecv();
				m_CommandManager.Post(pCmd);
				UnlockRecv();
			}
		}
		break;
	default:
		{
			Log("MatchClient: Parse Packet Error");
		}
		break;
	}
}

int MMatchClient::AgentConnect(SOCKET* pSocket, char* szIP, int nPort)
{
	if (m_AgentSocket.Connect(pSocket, szIP, nPort))
		return MOK;
	else
		return MERR_UNKNOWN;
}

void MMatchClient::AgentDisconnect()
{
	m_AgentSocket.Disconnect();
}

void MMatchClient::StartAgentPeerConnect()
{
	SetAgentPeerFlag(false);
	SetAgentPeerCount(10);
}

void MMatchClient::CastAgentPeerConnect()
{
	MCommand* pCmd = new MCommand(m_CommandManager.GetCommandDescByID(MC_AGENT_PEER_BINDUDP),
		GetAgentServerUID(), m_This);
	pCmd->AddParameter(new MCommandParameterUID(GetPlayerUID()));
	pCmd->AddParameter(new MCommandParameterString("localhost"));
	pCmd->AddParameter(new MCommandParameterUInt(0));
	pCmd->AddParameter(new MCommandParameterString(""));
	pCmd->AddParameter(new MCommandParameterUInt(0));

	SendCommandByUDP(pCmd, GetAgentIP(), GetAgentPeerPort());
	delete pCmd;
}


void MMatchClient::StartUDPTest(const MUID& uidChar)
{
	SetUDPTestProcess(true);
	if (uidChar == GetPlayerUID())
	{
		for (auto* pPeer : MakePairValueAdapter(m_Peers.MUIDMap))
		{
			if (pPeer->GetUDPTestResult() == false)
				pPeer->StartUDPTest();
		}
	}
	else
	{
		auto* pPeer = FindPeer(uidChar);
		if (pPeer && pPeer->GetUDPTestResult() == false)
			pPeer->StartUDPTest();
	}
}

void MMatchClient::InitPeerCrypt(const MUID& uidStage, unsigned int nChecksum)
{
	MPacketCrypterKey key;
	MMakeSeedKey(&key, MUID(3465, nChecksum), uidStage, 9578234);

	m_PeerPacketCrypter.InitKey(&key);
}
