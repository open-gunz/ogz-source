//////////////////////////////////////////////////////////////////
// Class:	MRealCPNet class (2001/10/25)
// File:	RealCPNet.cpp
// Author:	Kim young ho (moanus@maiet.net)
//
// Implements Scalable Network Module with I/O Competion Port.
// Code based on MS iocpserver/iocpclient example
//////////////////////////////////////////////////////////////////
#define REALCPNET_LINK_SOCKET_LIBS

#ifdef WIN32
#include <WinSock2.h>
#include "MWindows.h"
#include <MSWSock.h>
#pragma comment(lib, "Mswsock.lib")

#include "MSocket.h"
#include "RealCPNet.h"
#include <time.h>
#include "MCrashDump.h"
#include "MInetUtil.h"
#include "MFile.h"

static RCPLOGFUNC*	g_RCPLog = NULL;
void SetupRCPLog(RCPLOGFUNC* pFunc) { g_RCPLog = pFunc; }
#define RCPLOG if(g_RCPLog) g_RCPLog

static int g_LogSessionCreated = 0;
static int g_LogSessionDestroyed = 0;

class RCPOverlapped : public WSAOVERLAPPED {
protected:
	RCP_IO_OPERATION	m_IOOperation;
public:
	RCPOverlapped(RCP_IO_OPERATION nIO) {
		ZeroMemory(this, sizeof(RCPOverlapped));
		m_IOOperation = nIO;
	}
	RCP_IO_OPERATION GetIOOperation() { return m_IOOperation; }
};

class RCPOverlappedSend : public RCPOverlapped {
protected:
	int		m_nTotalBytes;
	int		m_nTransBytes;
	char*	m_pData;
public:
	RCPOverlappedSend() : RCPOverlapped(RCP_IO_WRITE) {
		m_nTotalBytes = 0;
		m_nTransBytes = 0;
		m_pData = NULL;
	}
	virtual ~RCPOverlappedSend() {
		free(m_pData);	m_pData = NULL;
	}
	void SetData(char* pData, int nDataLen) {
		m_pData = pData;
		m_nTotalBytes = nDataLen;
	}
	int GetTotalBytes() { return m_nTotalBytes; }
	int GetTransBytes() { return m_nTransBytes; }
	void AddTransBytes(int nBytes) { m_nTransBytes += nBytes; }
	char* GetData() { return m_pData; }
};

class RCPOverlappedRecv : public RCPOverlapped {
protected:
	char*	m_pBuffer;
	int		m_nBufferSize;
public:
	RCPOverlappedRecv() : RCPOverlapped(RCP_IO_READ) {
		m_pBuffer = 0;
		m_nBufferSize = 0;
	}
	void SetBuffer(char* pBuffer, int nBufferSize) {
		m_pBuffer = pBuffer;
		m_nBufferSize = nBufferSize;
	}
	char* GetBuffer() { return m_pBuffer; }
	int GetBufferSize() { return m_nBufferSize; }
};

class RCPOverlappedAccept : public RCPOverlapped {
protected:
	SOCKET	m_Socket;
	char*	m_pBuffer;
	int		m_nBufferSize;
public:
	RCPOverlappedAccept() : RCPOverlapped(RCP_IO_ACCEPT) {
		m_Socket = MSocket::InvalidSocket;
		m_pBuffer = 0;
		m_nBufferSize = 0;
	}
	void SetSocket(SOCKET sd) { m_Socket = sd; }
	SOCKET GetSocket() { return m_Socket; }
	void SetBuffer(char* pBuffer, int nBufferSize) {
		m_pBuffer = pBuffer;
		m_nBufferSize = nBufferSize;
	}
	char* GetBuffer() { return m_pBuffer; }
	int GetBufferSize() { return m_nBufferSize; }
};

MRealCPNet::MRealCPNet() = default;
MRealCPNet::~MRealCPNet() = default;

bool MRealCPNet::Create(int nPort, const bool bReuse )
{
#ifdef WIN32
	SYSTEM_INFO         systemInfo;
	GetSystemInfo(&systemInfo);
	m_dwThreadCount = systemInfo.dwNumberOfProcessors * 2;
#else
	m_dwThreadCount = 8;
#endif

	m_hCleanupEvent = MSocket::CreateEvent();

	if (!MSocket::Startup())
	{
		MLog("MSocket::Startup failed\n");
		return false;
	}

	if (nPort != 0)
		m_nPort = nPort;

	m_bRestart = FALSE;
	m_bEndServer = FALSE;

	// notice that we will create more worker threads (dwThreadCount) than
	// the thread concurrency limit on the IOCP.
	m_hIOCP = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0);
	if (NULL == m_hIOCP) {
		RCPLOG("RealCPNet> CreateIoCompletionPort failed to create I/O completion port: %d\n", GetLastError());
        return false;
	}

	for (DWORD dwCPU=0; dwCPU < m_dwThreadCount; dwCPU++) {
		// Create worker threads to service the overlapped I/O requests.  The decision
		// to create 2 worker threads per CPU in the system is a heuristic.  Also,
		// note that thread handles are closed right away, because we will not need them
		// and the worker threads will continue to execute.
		std::thread{[this] {
			WorkerThread();
		}}.detach();
	}

	if (!CreateListenSocket(bReuse))
		return false;

    if (!CreateAcceptSocket(TRUE))
		return false;

	return true;
}

void MRealCPNet::Destroy()
{
	mlog("RealCPNet> SessionCreated=%d, SessionDestroyed=%d \n",
			g_LogSessionCreated, g_LogSessionDestroyed);

	RCPLOG("RealCPNet> Begin Destroy \n");

	m_bEndServer = TRUE;
	// Cause worker threads to exit
    if (m_hIOCP) {
		for (DWORD i = 0; i < m_dwThreadCount; i++)
			PostQueuedCompletionStatus(m_hIOCP, 0, 0, NULL);
	}

	RCPLOG("RealCPNet> Destroy Process 1/4 \n");

	RCPLOG("RealCPNet> Destroy Process 2/4 \n");

	if (m_sdListen != MSocket::InvalidSocket) {
		MSocket::closesocket(m_sdListen);
		m_sdListen = MSocket::InvalidSocket;
	}

	if (m_pListenSession) {
		if (m_pListenSession->GetSocket() != MSocket::InvalidSocket)
			MSocket::closesocket(m_pListenSession->GetSocket());
		m_pListenSession->SetSocket(MSocket::InvalidSocket);

		if (m_pListenSession)
			delete m_pListenSession;
		m_pListenSession = NULL;
	}

	RCPLOG("RealCPNet> Destroy Process 3/4 \n");

	DeleteAllSession();

	RCPLOG("RealCPNet> Destroy Process 4/4 \n");

	if (m_hIOCP) {
		CloseHandle(m_hIOCP);
		m_hIOCP = NULL;
	}

	MSocket::Cleanup();

	RCPLOG("RealCPNet> Destroy End \n");
}

// Create a socket with all the socket options we need, namely disable buffering
// and set linger.
SOCKET MRealCPNet::CreateSocket()
{
	int         nRet;
	int         nZero = 0;
	linger      lingerStruct;
	SOCKET      sdSocket = MSocket::InvalidSocket;


	sdSocket = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_IP, NULL, 0, WSA_FLAG_OVERLAPPED);
	if (INVALID_SOCKET == sdSocket) {
		RCPLOG("RealCPNet> WSASocket(sdSocket): %d\n", WSAGetLastError());
		return(sdSocket);
	}

	bool val = TRUE;
	nRet = setsockopt(sdSocket, IPPROTO_TCP, TCP_NODELAY, (const char FAR *)&val, sizeof(bool) );
	if (SOCKET_ERROR == nRet) {
		RCPLOG("RealCPNet> setsockopt(TCP_NODELAY): %d\n", WSAGetLastError());
		return(sdSocket);
	}

	lingerStruct.l_onoff = 1;
	lingerStruct.l_linger = 0;
	nRet = setsockopt(sdSocket, SOL_SOCKET, SO_LINGER, (char *)&lingerStruct, sizeof(lingerStruct));
	if (SOCKET_ERROR == nRet) {
		RCPLOG("RealCPNet> setsockopt(SO_LINGER): %d\n", WSAGetLastError());
		return(sdSocket);
	}

	return(sdSocket);
}

// Create a listening socket, bind, and set up its listening backlog.
bool MRealCPNet::CreateListenSocket(bool bReuse)
{
	SOCKADDR_IN	si_addrlocal;
	int			nRet;
	LINGER		lingerStruct;

	lingerStruct.l_onoff = 1;
	lingerStruct.l_linger = 0;


	m_sdListen = CreateSocket();
	if (INVALID_SOCKET == m_sdListen) {
		return(FALSE);
	}

	if (bReuse) {
		int opt = 1;
		if (setsockopt(m_sdListen, SOL_SOCKET, SO_REUSEADDR, (char*)&opt, sizeof(opt)) == SOCKET_ERROR)
			return FALSE;
	}

	si_addrlocal.sin_family = AF_INET;
	si_addrlocal.sin_port = htons(m_nPort);
	si_addrlocal.sin_addr.s_addr = htonl(INADDR_ANY);
	nRet = ::bind(m_sdListen, (struct sockaddr *)&si_addrlocal, sizeof(si_addrlocal));
	if (SOCKET_ERROR == nRet) {
		RCPLOG("RealCPNet> bind: %d\n", WSAGetLastError());
		return(FALSE);
	}

	nRet = listen(m_sdListen, 16);
	if (SOCKET_ERROR == nRet) {
		RCPLOG("RealCPNet> listen: %d\n", WSAGetLastError());
		return(FALSE);
	}

	return(TRUE);
}

// Create a socket and invoke AcceptEx.  Only the original call to to this
// function needs to be added to the IOCP.
//
// If the expected behaviour of connecting client applications is to NOT
// send data right away, then only posting one AcceptEx can cause connection
// attempts to be refused if a client connects without sending some initial
// data (notice that the associated iocpclient does not operate this way
// but instead makes a connection and starts sending data write away).
// This is because the IOCP packet does not get delivered without the initial
// data (as implemented in this sample) thus preventing the worker thread
// from posting another AcceptEx and eventually the backlog value set in
// listen() will be exceeded if clients continue to try to connect.
//
// One technique to address this situation is to simply cause AcceptEx
// to return right away upon accepting a connection without returning any
// data.  This can be done by setting dwReceiveDataLength=0 when calling AcceptEx.
//
// Another technique to address this situation is to post multiple calls
// to AcceptEx.  Posting multiple calls to AcceptEx is similar in concept to
// increasing the backlog value in listen(), though posting AcceptEx is
// dynamic (i.e. during the course of running your application you can adjust
// the number of AcceptEx calls you post).  It is important however to keep
// your backlog value in listen() high in your server to ensure that the
// stack can accept connections even if your application does not get enough
// CPU cycles to repost another AcceptEx under stress conditions.
//
// This sample implements neither of these techniques and is therefore
// susceptible to the behaviour described above.
//
bool MRealCPNet::CreateAcceptSocket(bool fUpdateIOCP)
{
	int		nRet;
	DWORD	dwRecvNumBytes = 0;

	//The context for listening socket uses the SockAccept member to store the
	//socket for client connection.
	if (fUpdateIOCP) {
		m_pListenSession = UpdateCompletionPort(m_sdListen, RCP_IO_ACCEPT, FALSE);
		if (m_pListenSession == NULL) {
			RCPLOG("failed to update listen socket to IOCP\n");
			return(FALSE);
		}
	}

	SOCKET sdAccept = CreateSocket();
	if (INVALID_SOCKET == sdAccept) {
		RCPLOG("failed to create new accept socket\n");
		delete m_pListenSession;
		return(FALSE);
	}

	RCPOverlappedAccept* pRCPAccept = new RCPOverlappedAccept();
	pRCPAccept->SetBuffer(m_pListenSession->m_RecvBuffer, MAX_BUFF_SIZE);
	pRCPAccept->SetSocket(sdAccept);

	// pay close attention to these parameters and buffer lengths
	nRet = AcceptEx(m_sdListen,
					pRCPAccept->GetSocket(),
					(LPVOID)pRCPAccept->GetBuffer(),
					0,
					sizeof(SOCKADDR_IN) + 16,
					sizeof(SOCKADDR_IN) + 16,
					&dwRecvNumBytes,
					(LPOVERLAPPED)pRCPAccept);
	if (nRet == SOCKET_ERROR && (ERROR_IO_PENDING != WSAGetLastError())) {
		RCPLOG("AcceptEx Failed: %d\n", WSAGetLastError());
		return(FALSE);
	}

	return(TRUE);
}

//  Allocate a context structures for the socket and add the socket to the IOCP.
//  Additionally, add the context structure to the global list of context structures.
MRealSession* MRealCPNet::UpdateCompletionPort(SOCKET sd, RCP_IO_OPERATION nOperation, bool bAddToList)
{
	MRealSession* pSession;

	pSession = CreateSession(sd, nOperation);
	if (pSession == NULL)
		return(NULL);

	m_hIOCP = CreateIoCompletionPort((HANDLE)sd, m_hIOCP, (DWORD_PTR)pSession, 0);
	if (NULL == m_hIOCP) {
		RCPLOG("CreateIoCompletionPort: %d\n", GetLastError());

		delete pSession;
		return(NULL);
	}

	//The listening socket context (bAddToList is FALSE) is not added to the list.
	//All other socket contexts are added to the list.
	if (bAddToList)
		m_SessionMap.Add(pSession);

	if (m_bVerbose)
		RCPLOG("UpdateCompletionPort: Socket(%d) added to IOCP\n", pSession->GetSocket());

	return pSession;
}

bool MRealCPNet::MakeSockAddr(const char* pszIP, int nPort, MSocket::sockaddr_in* pSockAddr)
{
	sockaddr_in RemoteAddr{};
	memset((char*)&RemoteAddr, 0, sizeof(sockaddr_in));

	// Set Dest IP and Port
	RemoteAddr.sin_family = AF_INET;
	RemoteAddr.sin_port = htons(nPort);
	auto dwAddr = inet_addr(pszIP);
	if (dwAddr != INADDR_NONE) {
		memcpy(&(RemoteAddr.sin_addr), &dwAddr, 4);
	} else {
		auto pHost = MSocket::gethostbyname(pszIP);
		if (pHost == NULL) { // error
			OutputDebugString("<REALCP_ERROR> Can't resolve hostname </REALCP_ERROR>\n");
			return false;
		}
		memcpy((char FAR *)&(RemoteAddr.sin_addr), pHost->h_addr, pHost->h_length);
	}
	memcpy(pSockAddr, &RemoteAddr, sizeof(MSocket::sockaddr_in));

	return true;
}

bool MRealCPNet::CheckIPFloodAttack(MSocket::sockaddr_in* pRemoteAddr, int* poutIPCount)
{
	std::lock_guard<MCriticalSection> lock{ m_SessionMap.m_csLock };

	auto Count = std::count_if(std::begin(m_SessionMap), std::end(m_SessionMap), [&](auto&& Pair) {
		return Pair.second->GetSockAddr()->sin_addr.S_un.S_addr == pRemoteAddr->sin_addr.S_un.S_addr;
	});

	if (poutIPCount)
		*poutIPCount = Count;

	return Count > 32;
}

bool MRealCPNet::Connect(SOCKET* pSocket, const char* pszAddress, int nPort)
{
	SOCKET sdConnect = CreateSocket();
	if (INVALID_SOCKET == sdConnect) {
		RCPLOG("Can't Create Socket \n");
		*pSocket = INVALID_SOCKET;
		return false;
	}

	MSocket::sockaddr_in	ConnectAddr;
	if (MakeSockAddr(pszAddress, nPort, &ConnectAddr) == false) {
		RCPLOG("Can't resolve Address %s:%n", pszAddress, nPort);
		closesocket(sdConnect);
		*pSocket = INVALID_SOCKET;
		return false;
	}

	MRealSession* pContextConnect = UpdateCompletionPort(sdConnect, RCP_IO_CONNECT, TRUE);
	if (pContextConnect == NULL) {
		RCPLOG("failed to update listen socket to IOCP\n");
		closesocket(sdConnect);
		*pSocket = INVALID_SOCKET;
		return false;
	}
	pContextConnect->SetSockAddr(&ConnectAddr, sizeof(SOCKADDR_IN));

	int	nRet;
	nRet = connect(sdConnect, (LPSOCKADDR)&ConnectAddr, sizeof(ConnectAddr));
	if (SOCKET_ERROR == nRet) {
		RCPLOG("RCPLOG> Failed to Connect (%s:%d)\n", pszAddress, nPort);
		CloseSession(pContextConnect, TRUE);
		*pSocket = INVALID_SOCKET;
		return false;
	} else {
		*pSocket = sdConnect;
		PostIORecv(pContextConnect->GetSocket());

		if (m_fnCallback)
			m_fnCallback(m_pCallbackContext, RCP_IO_CONNECT, (DWORD)sdConnect, NULL, 0);

		return true;
	}
}

void MRealCPNet::Disconnect(SOCKET sd)
{
	MRealSession* pSession = m_SessionMap.GetSession(sd);
	if (pSession)
		CloseSession(pSession, FALSE);
}

//  Close down a connection with a client.  This involves closing the socket (when
//  initiated as a result of a CTRL-C the socket closure is not graceful).  Additionally,
//  any context data associated with that socket is free'd.
void MRealCPNet::CloseSession(MRealSession* pSession, bool bGraceful)
{
	std::lock_guard<MCriticalSection> lock{ m_SessionMap.m_csLock };

	if (m_SessionMap.IsExistUnsafe(pSession)) {
		SOCKET sd = pSession->GetSocket();
		if (m_bVerbose)
			RCPLOG("CloseSession: Socket(%d) connection closing (graceful=%s)\n",
					sd, (bGraceful?"TRUE":"FALSE"));
		if (sd != INVALID_SOCKET) {
			if (!bGraceful) {
				// force the subsequent closesocket to be abortative.
				LINGER  lingerStruct;
				lingerStruct.l_onoff = 1;
				lingerStruct.l_linger = 0;
				setsockopt(sd, SOL_SOCKET, SO_LINGER, (char *)&lingerStruct, sizeof(lingerStruct));
			}

			closesocket(sd);
			pSession->SetSocket(INVALID_SOCKET);
			pSession->SetSessionState(MRealSession::SESSIONSTATE_DEAD);	// Set Dead Mark
		}

		if (m_fnCallback)
			m_fnCallback(m_pCallbackContext, RCP_IO_DISCONNECT, (DWORD)sd, NULL, 0);
		pSession->SetUserContext(NULL);
		m_SessionMap.RemoveUnsafe(sd);
		g_LogSessionDestroyed++;
	}
}

// Free all context structure in the global list of context structures.
void MRealCPNet::DeleteAllSession()
{
	m_SessionMap.RemoveAll();
	return;
}

bool MRealCPNet::GetAddress(SOCKET sd, char* pszAddress, size_t maxlen, int* pPort)
{
	std::lock_guard<MCriticalSection> lock{ m_SessionMap.m_csLock };

	MRealSession* pSession = m_SessionMap.GetSessionUnsafe(sd);
	if (!pSession)
		return false;

	MSocket::in_addr addr;
	addr.S_un.S_addr = pSession->GetIP();

	char ip[16];
	GetIPv4String(addr, ip);

	strcpy_safe(pszAddress, maxlen, ip);
	*pPort = pSession->GetPort();

	return true;
}

void* MRealCPNet::GetUserContext(SOCKET sd)
{
	std::lock_guard<MCriticalSection> lock{ m_SessionMap.m_csLock };

	MRealSession* pSession = m_SessionMap.GetSessionUnsafe(sd);
	if (!pSession)
		return nullptr;

	return pSession->GetUserContext();
}

void MRealCPNet::SetUserContext(SOCKET sd, void* pContext)
{
	std::lock_guard<MCriticalSection> lock{ m_SessionMap.m_csLock };

	MRealSession* pSession = m_SessionMap.GetSessionUnsafe(sd);
	if (pSession) {
		pSession->SetUserContext(pContext);
	}
}

bool MRealCPNet::Send(SOCKET sd, MPacketHeader* pPacket, int nSize)
{
	assert(nSize > 0);

	std::lock_guard<MCriticalSection> lock{ m_SessionMap.m_csLock };

	MRealSession* pSession = m_SessionMap.GetSessionUnsafe(sd);
	if (pSession) {
		PostIOSend(sd, (char*)pPacket, nSize);
	} else {
		free(pPacket);
	}

	return true;
}

bool MRealCPNet::PostIOSend(SOCKET sd, char* pBuf, u32 nBufLen)
{
	RCPOverlappedSend* pRCPSend = new RCPOverlappedSend();
	pRCPSend->SetData(pBuf, nBufLen);

	WSABUF wsaBuf;
	wsaBuf.buf = pBuf;
	wsaBuf.len = nBufLen;

	DWORD dwSendNumBytes = 0;
	DWORD dwFlags = 0;

	int nRet = WSASend(sd,
					&wsaBuf, 1, &dwSendNumBytes,
					dwFlags,
					pRCPSend, NULL);
	if (SOCKET_ERROR == nRet && (ERROR_IO_PENDING != WSAGetLastError())) {
		DWORD dwError = WSAGetLastError();
		RCPLOG("WSASend: %d\n", dwError);
		delete pRCPSend;
	}

	return true;
}

void MRealCPNet::PostIORecv(SOCKET sd)
{
	int nRet = 0;

	{
		std::lock_guard<MCriticalSection> lock{ m_SessionMap.m_csLock };

		MRealSession* pSession = m_SessionMap.GetSessionUnsafe(sd);
		if (pSession) {
			RCPOverlappedRecv* pRecv = new RCPOverlappedRecv();
			pRecv->SetBuffer(pSession->m_RecvBuffer, MAX_BUFF_SIZE);

			WSABUF	buffRecv;
			buffRecv.buf = pRecv->GetBuffer();
			buffRecv.len = pRecv->GetBufferSize();

			DWORD dwRecvNumBytes = 0;
			DWORD dwFlags = 0;

			nRet = WSARecv(sd, &buffRecv, 1, &dwRecvNumBytes, &dwFlags,
				pRecv, NULL);
		}
	}

	if (SOCKET_ERROR == nRet && (ERROR_IO_PENDING != WSAGetLastError())) {
		DWORD dwError = WSAGetLastError();
		RCPLOG("PostIORecv->WSARecv: %d\n", dwError);
	}
}

// Allocate a socket context for the new connection.
MRealSession* MRealCPNet::CreateSession(SOCKET sd, RCP_IO_OPERATION ClientIO)
{
	MRealSession* pSession;
	pSession = new MRealSession;
	if (pSession) {
		pSession->SetSocket(sd);
	} else {
		RCPLOG("new MRealSession Failed: %d\n", GetLastError());
		return NULL;
	}
	g_LogSessionCreated++;
	return pSession;
}

// Worker thread that handles all I/O requests on any socket handle added to the IOCP.
void MRealCPNet::WorkerThread()
{
	bool bSuccess = false;
	int nRet;

	OVERLAPPED* lpOverlapped = nullptr;
	MRealSession* pSession = nullptr;
	MRealSession* lpAcceptSession = nullptr;

	u32 dwRecvNumBytes = 0;
	u32 dwSendNumBytes = 0;
	u32 dwFlags = 0;
	unsigned long dwIoSize;

    while (true) {
		// continually loop to service io completion packets
		bSuccess = GetQueuedCompletionStatus(
						m_hIOCP,
						&dwIoSize,
						(PDWORD_PTR)&pSession,
						&lpOverlapped,
						INFINITE) != FALSE;
		if (!bSuccess) {
//			continue;	// 2005-08-01 RaonHaje
		}

		if (pSession == NULL) {
			// CTRL-C handler used PostQueuedCompletionStatus to post an I/O packet with
			// a NULL CompletionKey (or if we get one for any reason).  It is time to exit.
			return;
		}

		if (m_bEndServer) {
			// main thread will do all cleanup needed - see finally block
			return;
		}

		RCPOverlapped* pRCPOverlapped = (RCPOverlapped*)lpOverlapped;

		//We should never skip the loop and not post another AcceptEx if the current
		//completion packet is for previous AcceptEx
		if (pRCPOverlapped->GetIOOperation() != RCP_IO_ACCEPT) {
			if (!bSuccess ||
				(bSuccess && (0 == dwIoSize))) {
				// client connection dropped, continue to service remaining (and possibly
				// new) client connections
				DMLog("%u:RCP_CLOSE_SESSION(%d)\n",
					GetTickCount(), pSession->GetSocket());

				CloseSession(pSession, FALSE);
				continue;
			}
		}

		// determine what type of IO packet has completed by checking the PER_IO_CONTEXT
		// associated with this socket.  This will determine what action to take.
		switch (pRCPOverlapped->GetIOOperation()) {
		case RCP_IO_ACCEPT:
			{
				// When the AcceptEx function returns, the socket sAcceptSocket is
				// in the default state for a connected socket. The socket sAcceptSocket
				// does not inherit the properties of the socket associated with
				// sListenSocket parameter until SO_UPDATE_ACCEPT_CONTEXT is set on
				// the socket. Use the setsockopt function to set the SO_UPDATE_ACCEPT_CONTEXT
				// option, specifying sAcceptSocket as the socket handle and sListenSocket
				// as the option value.

				RCPOverlappedAccept* pRCPAccept= (RCPOverlappedAccept*)lpOverlapped;

				if (pRCPAccept->GetSocket() == INVALID_SOCKET) {
					static int nInvalidAccept = 0;
					char szLog[64]; sprintf_safe(szLog, "Accept with INVALID_SOCKET (Count=%d) \n", nInvalidAccept++);
					OutputDebugString(szLog);
					delete pRCPAccept;
					CreateAcceptSocket(FALSE);
					continue;
				}

				// Get Address First //
				int locallen;
				int remotelen;
				MSocket::sockaddr_in *plocal = nullptr;
				MSocket::sockaddr_in *premote = nullptr;
				MSocket::sockaddr_in LocalAddr{};
				MSocket::sockaddr_in RemoteAddr{};

				GetAcceptExSockaddrs((LPVOID)(pRCPAccept->GetBuffer()),
					0,
					sizeof(SOCKADDR_IN) + 16,
					sizeof(SOCKADDR_IN) + 16,
					(sockaddr **)&plocal,
					&locallen,
					(sockaddr **)&premote,
					&remotelen);

				memcpy(&LocalAddr, plocal, sizeof(sockaddr_in));
				memcpy(&RemoteAddr, premote, sizeof(sockaddr_in));
				// Get Address End //

				// Check Connection Flood Attack
				int nIPCount = 0;
				if (CheckIPFloodAttack(&RemoteAddr, &nIPCount) == true) {
					char ip_string_buffer[128]; ip_string_buffer[0] = 0;
					MSocket::inet_ntop(MSocket::AF::INET, &RemoteAddr.sin_addr,
						ip_string_buffer, std::size(ip_string_buffer));
					RCPLOG("Accept Detected CONNECTION FLOOD ATTACK (IP=%s , Count=%d) \n",
						ip_string_buffer, nIPCount);
					closesocket(pRCPAccept->GetSocket());
					CreateAcceptSocket(FALSE);
					delete pRCPAccept;
					continue;
				}

				nRet = setsockopt(
							pRCPAccept->GetSocket(),
							SOL_SOCKET,
							SO_UPDATE_ACCEPT_CONTEXT,
							(char*)&m_sdListen,
							sizeof(m_sdListen)
						);

				if (nRet == SOCKET_ERROR) {
					int nError = WSAGetLastError();
					//just warn user here.
					RCPLOG("setsockopt(SO_UPDATE_ACCEPT_CONTEXT) failed to update accept socket. (Err:%d) \n",
						nError);

					closesocket(pRCPAccept->GetSocket());
					CreateAcceptSocket(FALSE);
					delete pRCPAccept;
					continue;
				}

				lpAcceptSession = UpdateCompletionPort(
												pRCPAccept->GetSocket(),
												RCP_IO_ACCEPT, TRUE);
				if (lpAcceptSession == NULL) {
					//just warn user here.
					RCPLOG("failed to update accept socket to IOCP\n");
					m_hCleanupEvent.SetEvent();
					delete pRCPAccept;
					return;
				}

				lpAcceptSession->SetSockAddr(&RemoteAddr, remotelen);

				//Time to post another outstanding AcceptEx
				if (!CreateAcceptSocket(FALSE)) {
					RCPLOG("Please shut down and reboot the server.\n");
					m_hCleanupEvent.SetEvent();
					delete pRCPAccept;
					return;
				}

				if (m_fnCallback)
					m_fnCallback(m_pCallbackContext, RCP_IO_ACCEPT,
											(DWORD)lpAcceptSession->GetSocket(),
											(MPacketHeader*)lpAcceptSession->m_RecvBuffer, (DWORD)dwIoSize);

				PostIORecv(lpAcceptSession->GetSocket());

				delete pRCPAccept;
			}
			break;
		case RCP_IO_READ:
			{
				RCPOverlappedRecv* pRCPRecv= (RCPOverlappedRecv*)lpOverlapped;
				delete pRCPRecv;

				SOCKET sdRecv = INVALID_SOCKET;

				m_SessionMap.Lock();
					if (m_SessionMap.IsExistUnsafe(pSession)) {	// Ensure exist Session
						if (pSession->GetSessionState() != MRealSession::SESSIONSTATE_DEAD) {
							sdRecv = pSession->GetSocket();
							if (m_fnCallback)
								m_fnCallback(m_pCallbackContext, RCP_IO_READ,
									(DWORD)pSession->GetSocket(),
									(MPacketHeader*)pSession->m_RecvBuffer, (DWORD)dwIoSize);
						}
					}
				m_SessionMap.Unlock();

				if (INVALID_SOCKET != sdRecv)
					PostIORecv(sdRecv);
			}
			break;

		case RCP_IO_WRITE:
			{
				RCPOverlappedSend* pRCPSend = (RCPOverlappedSend*)lpOverlapped;
				pRCPSend->AddTransBytes(dwIoSize);
				_ASSERT(pRCPSend->GetTransBytes() == pRCPSend->GetTotalBytes());
				delete pRCPSend;
			}
			break;
		} //switch
	} //while
}
#endif
