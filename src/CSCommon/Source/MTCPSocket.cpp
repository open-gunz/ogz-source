#include "MTCPSocket.h"
#include "MDebug.h"
#include "MInetUtil.h"
#include "defer.h"
#include "reinterpret.h"

#define MAX_RECVBUF_LEN						4096
#define TCPSOCKET_MAX_SENDQUEUE_LEN			5120
#define MAX_CLIENTSOCKET_LEN				60

void MTCPSocketThread::Create()
{
	if (IsActive())
		Destroy();

	MThread::Create();

	m_bActive = true;
}

void MTCPSocketThread::Destroy() {
	DMLog("Set kill event %p\n", this);

	m_bActive = false;

	m_KillEvent.SetEvent();

	MThread::Destroy();
}

bool MClientSocketThread::OnConnect(SOCKET sock)
{
	m_bActive = true;

	if (m_fnConnectCallback)
		return m_fnConnectCallback(m_pCallbackContext, sock);

	return false;
}

bool MClientSocketThread::OnDisconnect(SOCKET sock)
{
	m_bActive = false;

	if (m_fnDisconnectCallback)
		return m_fnDisconnectCallback(m_pCallbackContext, sock);

	return false;
}

void MClientSocketThread::ClearSendList()
{
	std::lock_guard<MCriticalSection> lock{ m_csSendLock };

	for (auto itor = m_SendList.begin(); itor != m_SendList.end(); )
	{
		MTCPSendQueueItem*		pItem = (*itor);
		delete [] pItem->pPacket;
		delete pItem;
		++itor;
	}
	m_SendList.clear();

	for (auto itor = m_TempSendList.begin(); itor != m_TempSendList.end(); )
	{
		MTCPSendQueueItem*		pItem = (*itor);
		delete [] pItem->pPacket;
		delete pItem;
		++itor;
	}
	m_TempSendList.clear();
}

bool MClientSocketThread::FlushSend()
{
	{
		std::lock_guard<MCriticalSection> lock{ m_csSendLock };
		while (!m_TempSendList.empty())
		{
			auto itor = m_TempSendList.begin();
			m_SendList.push_back(*itor);
			m_TempSendList.erase(itor);
		}
	}

	while (!m_SendList.empty())
	{
		auto SendItor = m_SendList.begin();
		MTCPSendQueueItem* pSendItem = (MTCPSendQueueItem*)(*SendItor);

		unsigned int nTransBytes = 0;
		while (true)
		{
			int nSent = MSocket::send(m_pTCPSocket->GetSocket(),
				(char*)pSendItem->pPacket + nTransBytes,
				pSendItem->dwPacketSize - nTransBytes,
				0);

			if (nSent == MSocket::SocketError) {
				int ErrorCode = MSocket::GetLastError();

				using namespace MSocket::Error;

				if (ErrorCode == WouldBlock)
					continue;

				MLog("FLUSHSEND> FlushSend Error(%d)\n", ErrorCode);

				if (ErrorCode == ConnectionAborted ||
					ErrorCode == ConnectionReset ||
					ErrorCode == TimedOut)
				{
					 MLog("FLUSHSEND> Connection ERROR Closed!!!!!!!!!! \n");
					 return false;
				}
			} else {
				nTransBytes += nSent;
				if (nTransBytes >= pSendItem->dwPacketSize)
					break;
			}
		}

		m_nTotalSend += nTransBytes;
		m_SendTrafficLog.Record(m_nTotalSend);

		{
			std::lock_guard<MCriticalSection> lock{ m_csSendLock };
			if (pSendItem != NULL)
			{
				delete[] pSendItem->pPacket;
				delete pSendItem;
			}
			m_SendList.erase(SendItor);
		}
	}

	return true;
}

void MClientSocketThread::Run()
{
	DMLog("MClientSocketThread::Run -- Entering\n");

	auto SocketEvent = MSocket::CreateEvent();

	{
		using namespace MSocket::FD;
		const auto EventFlags = CONNECT | READ | WRITE | CLOSE;
		auto sel_ret = MSocket::EventSelect(m_pTCPSocket->GetSocket(), SocketEvent, EventFlags);
		if (sel_ret != 0)
		{
			LOG_SOCKET_ERROR("EventSelect", sel_ret);
			return;
		}
	}

	MSignalEvent* EventArray[] = {
		&SocketEvent,
		&m_SendEvent,
		&m_KillEvent,
	};

	while (true)
	{
		const auto Timeout = 100;
		u32 WaitResult = WaitForMultipleEvents(EventArray, Timeout);
		if (WaitResult == MSync::WaitFailed)
		{
			LOG_SOCKET_ERROR("WaitForMultipleEvents", WaitResult);
			return;
		}

		switch (WaitResult)
		{
			// Socket Event
		case 0:
			HandleSocketEvent(SocketEvent);
			break;

			// Send Event
		case 1:
			if (m_bActive)
				FlushSend();
			break;

			// Kill Event
		case 2:
			DMLog("MClientSocketThread::Run -- Received kill event\n");
			goto end_thread;
			break;

			// Timeout
		case MSync::WaitTimeout:
			if (GetSendWaitQueueCount() > 0)
				FlushSend();
			break;

		default:
			LOG_SOCKET_ERROR("WaitForMultipleEvents", WaitResult);
			break;
		}
	}

end_thread:

	ClearSendList();

	m_bActive = false;
	
	DMLog("MClientSocketThread::Run -- Leaving\n");
}

void MClientSocketThread::HandleSocketEvent(MSignalEvent& hFDEvent)
{
	MSocket::NetworkEvents NetEvent;
	auto ene_ret = MSocket::EnumNetworkEvents(m_pTCPSocket->GetSocket(), hFDEvent, &NetEvent);
	if (ene_ret != 0)
	{
		LOG_SOCKET_ERROR("EnumNetworkEvents", ene_ret);
		return;
	}

	DMLog("Got socket event, %u\n", NetEvent.ErrorCode);

	if (MSocket::IsNetworkEventSet(NetEvent, MSocket::FD::CONNECT))
	{
		auto Value = MSocket::GetNetworkEventValue(NetEvent, MSocket::FD::CONNECT);
		if (Value == 0)
		{
			OnConnect(m_pTCPSocket->GetSocket());
		}
		else
		{
			OnSocketError(m_pTCPSocket->GetSocket(), eeConnect, Value);
			m_KillEvent.SetEvent();
		}
	}

	if (MSocket::IsNetworkEventSet(NetEvent, MSocket::FD::READ))
	{
		auto Value = MSocket::GetNetworkEventValue(NetEvent, MSocket::FD::READ);
		if (Value == 0)
		{
			if (m_bActive)
				Recv();
		}
		else
		{
			OnSocketError(m_pTCPSocket->GetSocket(), eeReceive, Value);
		}

	}

	if (MSocket::IsNetworkEventSet(NetEvent, MSocket::FD::WRITE))
	{
		auto Value = MSocket::GetNetworkEventValue(NetEvent, MSocket::FD::WRITE);
		if (Value == 0)
		{
			if (m_bActive)
				FlushSend();
		}
		else
		{
			OnSocketError(m_pTCPSocket->GetSocket(), eeSend, Value);
		}
	}

	if (MSocket::IsNetworkEventSet(NetEvent, MSocket::FD::CLOSE))
	{
		OnDisconnect(m_pTCPSocket->GetSocket());

		auto Value = MSocket::GetNetworkEventValue(NetEvent, MSocket::FD::CLOSE);
		if (Value != 0)
		{
			OnSocketError(m_pTCPSocket->GetSocket(), eeDisconnect, Value);
		}
		m_KillEvent.SetEvent();
	}
}

bool MClientSocketThread::Recv()
{
	char			RecvBuf[MAX_RECVBUF_LEN];
	int				nRecv = 0;

	while (true) 
	{
		nRecv = MSocket::recv(m_pTCPSocket->GetSocket(), RecvBuf, MAX_RECVBUF_LEN, 0);
		if (nRecv != MSocket::SocketError) {
			m_nTotalRecv += nRecv;
			m_RecvTrafficLog.Record(m_nTotalRecv);
		}

		if (nRecv <= 0) break;

		if (m_fnRecvCallback && OnRecv(m_pTCPSocket->GetSocket(), RecvBuf, nRecv) == true)
		{
			continue;
		}
	}

	return true;
}

bool MClientSocketThread::OnRecv(SOCKET socket, char *pPacket, u32 dwSize)
{
	if (m_fnRecvCallback) return m_fnRecvCallback(m_pCallbackContext, socket, pPacket, dwSize);

	return false;
}

bool MClientSocketThread::PushSend(char *pPacket, u32 dwPacketSize)
{
	if (!m_bActive || m_SendList.size() > TCPSOCKET_MAX_SENDQUEUE_LEN)
		return false;

	_ASSERT(dwPacketSize > 0);

	MTCPSendQueueItem* pSendItem = new MTCPSendQueueItem;
	_ASSERT(pSendItem != NULL);

	pSendItem->dwPacketSize = 0;

	pSendItem->pPacket = pPacket;
	pSendItem->dwPacketSize = dwPacketSize;

	{
		std::lock_guard<MCriticalSection> lock{ m_csSendLock };
		m_TempSendList.push_back(pSendItem);
	}

	m_SendEvent.SetEvent();

	return true;
}

bool MServerSocketThread::FlushSend()
{
	return false;

	int result = 0;

	for (auto SocketItor = m_SocketList.begin(); SocketItor != m_SocketList.end(); ++SocketItor)		
	{
		auto pSocketObj = *SocketItor;

		while(pSocketObj->sendlist.size() > 0) 
		{
			auto SendItor = pSocketObj->sendlist.begin();
			auto* pSendItem = *SendItor;

			result = MSocket::send(pSocketObj->sock, (char*)pSendItem->pPacket,
							pSendItem->dwPacketSize, 0);

			if (result == MSocket::SocketError) {
				return false;
			} else {
				m_nTotalSend += result;
				m_SendTrafficLog.Record(m_nTotalSend);
			}
			
			delete [] pSendItem->pPacket;
			delete pSendItem;

			pSocketObj->sendlist.erase(SendItor);
		}
	}

	return true;
}

bool MServerSocketThread::Recv(MSocketObj* pSocketObj)
{
	char			RecvBuf[MAX_RECVBUF_LEN];
	int				nRecv = 0;

	while(pSocketObj->sock != MSocket::InvalidSocket)
	{
		nRecv = MSocket::recv(pSocketObj->sock, RecvBuf, MAX_RECVBUF_LEN, 0);
		if (nRecv != MSocket::SocketError) {
			m_nTotalRecv += nRecv;
			m_RecvTrafficLog.Record(m_nTotalRecv);
		}

		if (nRecv <= 0) break;
		if (m_fnRecvCallback && OnRecv(pSocketObj, RecvBuf, nRecv) == true)
		{
			continue;
		}

	}

	return true;
}

void MServerSocketThread::Run()
{
	auto SocketEvent = MSignalEvent(true);
	{
		using namespace MSocket::FD;
		const auto Flags = ACCEPT | CLOSE;
		auto result = MSocket::EventSelect(m_pTCPSocket->GetSocket(), SocketEvent, Flags);

		if (result == MSocket::SocketError)
		{
			LOG_SOCKET_ERROR("EventSelect", result);
			return;
		}
	}

	m_EventArray[0] = &SocketEvent; // Accept
	m_EventArray[1] = &m_SendEvent;	// Send
	m_EventArray[2] = &m_KillEvent;	// Kill

	while (true)
	{
		auto WaitResult = WaitForMultipleEvents(m_SocketList.size() + 3, m_EventArray, MSync::Infinite);

        if (WaitResult == MSync::WaitFailed)
        {
			LOG_SOCKET_ERROR("WaitForMultipleEvents", WaitResult);
        }

		switch (WaitResult)
		{
			case 0: // Accept Event
				HandleAcceptEvent();
				break;

			case 1: // Send Event
				FlushSend();
				m_SendEvent.ResetEvent();
				break;

			case 2: // Kill Event
				return;

			case MSync::WaitTimeout:
				FlushSend();
				break;
		}

		HandleClientSockets();
	}
}

void MServerSocketThread::HandleAcceptEvent()
{
	MSocket::NetworkEvents NetEvent;
	auto ret = MSocket::EnumNetworkEvents(m_pTCPSocket->GetSocket(), *m_EventArray[0], &NetEvent);
	if (ret != 0)
	{
		LOG_SOCKET_ERROR("EnumNetworkEvents", ret);
		return;
	}

	if (MSocket::IsNetworkEventSet(NetEvent, MSocket::FD::ACCEPT))
	{
		auto Value = MSocket::GetNetworkEventValue(NetEvent, MSocket::FD::ACCEPT);
		if (Value == 0)
		{
			if (m_SocketList.size() >= MAX_CLIENTSOCKET_LEN)
			{
				return;
			}

			auto AcceptSocket = MSocket::accept(m_pTCPSocket->GetSocket(), NULL, NULL);

			InsertSocketObj(AcceptSocket);
		}
		else
		{
			OnSocketError(m_pTCPSocket->GetSocket(), eeDisconnect, Value);
		}
	}
}

void MServerSocketThread::HandleClientSockets()
{
	// Client Socket Event
	for (auto itor = m_SocketList.begin(); itor != m_SocketList.end();)
	{
		auto pSocketObj = *itor;

		bool DontIncrement = false;

		DEFER([&] { if (!DontIncrement) {
			++itor;
		}});

		{
			auto WaitResult = pSocketObj->event.Await(0);
			if (WaitResult == MSync::WaitFailed)
			{
				LOG_SOCKET_ERROR("pSocketObj->event.Await", WaitResult);
				return;
			}
			else if (WaitResult == MSync::WaitTimeout)
			{
				continue;
			}
		}

		MSocket::NetworkEvents NetEvent;
		MSocket::EnumNetworkEvents(pSocketObj->sock, pSocketObj->event, &NetEvent);

		if (MSocket::IsNetworkEventSet(NetEvent, MSocket::FD::READ))
		{
			auto Value = MSocket::GetNetworkEventValue(NetEvent, MSocket::FD::READ);
			if (Value == 0)
			{
				Recv(pSocketObj);
			}
			else
			{
				OnSocketError(pSocketObj->sock, eeReceive, Value);
				Disconnect(pSocketObj);

				MLog("<SOCKETTHREAD_ERROR> FD_READ error </SOCKETTHREAD_ERROR>\n");
			}
		}
		if (MSocket::IsNetworkEventSet(NetEvent, MSocket::FD::WRITE))
		{
			auto Value = MSocket::GetNetworkEventValue(NetEvent, MSocket::FD::WRITE);
			if (Value == 0)
			{
				FlushSend();
			}
			else
			{
				OnSocketError(pSocketObj->sock, eeSend, Value);
				Disconnect(pSocketObj);

				MLog("<SOCKETTHREAD_ERROR> FD_WRITE error </SOCKETTHREAD_ERROR>\n");
			}
		}
		if (MSocket::IsNetworkEventSet(NetEvent, MSocket::FD::CLOSE))
		{
			Disconnect(pSocketObj);

			auto Value = MSocket::GetNetworkEventValue(NetEvent, MSocket::FD::CLOSE);
			if (Value != 0)
			{
				OnSocketError(pSocketObj->sock, eeDisconnect, Value);
			}
		}

		if (pSocketObj->sock == MSocket::InvalidSocket)
		{
			FreeSocketObj(pSocketObj);
			itor = RemoveSocketObj(itor);
			RenumberEventArray();
			DontIncrement = true;
		}
	}
}

bool MServerSocketThread::OnAccept(MSocketObj *pSocketObj)
{
	if (m_fnAcceptCallback)
		return m_fnAcceptCallback(pSocketObj);

	return false;
}

bool MServerSocketThread::OnDisconnectClient(MSocketObj* pSocketObj)
{
	if (m_fnDisconnectCallback)
		return m_fnDisconnectCallback(pSocketObj);
	else 
		return false;
}

bool MServerSocketThread::OnRecv(MSocketObj* pSocketObj, char* pPacket, u32 dwPacketSize)
{
	if (m_fnRecvCallback)
		return m_fnRecvCallback(pSocketObj, pPacket, dwPacketSize);

	return false;
}

MSocketObj* MServerSocketThread::InsertSocketObj(SOCKET sock)
{
	MSocketObj* pSocketObj = new MSocketObj;

	pSocketObj->sock = sock;
	pSocketObj->event = MSocket::CreateEvent();

	{
		using namespace MSocket::FD;
		const auto Flags = READ | WRITE | CLOSE;
		MSocket::EventSelect(pSocketObj->sock, pSocketObj->event, Flags);
	}

	{
		std::lock_guard<MCriticalSection> lock{ m_csSocketLock };
		m_SocketList.push_back(pSocketObj);
		m_EventArray[m_SocketList.size() + 2] = &pSocketObj->event;
	}

	OnAccept(pSocketObj);

	return pSocketObj;
}

void MServerSocketThread::RenumberEventArray()
{
	int EventIndex = ClientSocketsStartIndex;

	for (auto* Socket : m_SocketList)
	{
		m_EventArray[EventIndex] = &Socket->event;
		++EventIndex;
	}
}

void MServerSocketThread::FreeSocketObj(MSocketObj *pSocketObj)
{
	while(pSocketObj->sendlist.size() > 0) 
	{
		auto SendItor = pSocketObj->sendlist.begin();
		auto* pSendItem = *SendItor;

		delete pSendItem->pPacket;
		delete pSendItem;

		pSocketObj->sendlist.erase(SendItor);
	}

	if (pSocketObj->sock != MSocket::InvalidSocket)
	{
		MSocket::closesocket(pSocketObj->sock);
	}

	delete pSocketObj;
	pSocketObj = NULL;
}

SocketList::iterator MServerSocketThread::RemoveSocketObj(SocketList::iterator itor)
{
	std::lock_guard<MCriticalSection> lock{ m_csSocketLock };
	return m_SocketList.erase(itor);
}

bool MServerSocketThread::PushSend(MSocketObj *pSocketObj, char *pPacket, u32 dwPacketSize)
{
	if (pSocketObj->sendlist.size() > TCPSOCKET_MAX_SENDQUEUE_LEN) return false;

	MTCPSendQueueItem* pSendItem = new MTCPSendQueueItem;
	pSendItem->pPacket = new char[dwPacketSize];
	memcpy(pSendItem->pPacket, pPacket, dwPacketSize);
	pSendItem->dwPacketSize = dwPacketSize;

	{
		std::lock_guard<MCriticalSection> lock{ m_csSendLock };
		pSocketObj->sendlist.push_back(pSendItem);
	}

	m_SendEvent.SetEvent();

	return true;
}

void MServerSocketThread::Disconnect(MSocketObj *pSocketObj)
{
	MSocket::closesocket(pSocketObj->sock);

	OnDisconnectClient(pSocketObj);

	pSocketObj->sock = MSocket::InvalidSocket;
}

bool MServerSocket::Disconnect(MSocketObj *pSocketObj)
{
	Thread.Disconnect(pSocketObj);
	return true;
}

bool MServerSocket::Send(MSocketObj *pSocketObj, char *pPacket, u32 dwPacketSize)
{
	return Thread.PushSend(pSocketObj, pPacket, dwPacketSize);
}

bool MServerSocket::Listen(int nPort)
{
	if (!OpenSocket(nPort)) return false;

	MSocket::listen(m_Socket, 5);

	return true;
}

bool MServerSocket::Close()
{
	CloseSocket();

	return true;
}

bool MServerSocket::OpenSocket(int nPort)
{
	if (!MTCPSocket::OpenSocket()) return false;

	Thread.Create();

	MSocket::sockaddr_in LocalAddress;
	LocalAddress.sin_family			= MSocket::AF::INET;
	LocalAddress.sin_addr.s_addr	= MSocket::htonl(MSocket::in_addr::Any);
	LocalAddress.sin_port			= MSocket::htons(nPort);

	const auto LocalAddress_sa = reinterpret<MSocket::sockaddr>(LocalAddress);

	const auto BindResult = MSocket::bind(m_Socket, &LocalAddress_sa, sizeof(LocalAddress_sa));

	if (BindResult == MSocket::SocketError)
	{
		MSocket::closesocket(m_Socket);
		return false;
	}
	m_LocalAddress = LocalAddress;

	return true;
}

template <typename T>
bool MTCPSocket<T>::OpenSocket()
{
	DMLog("MTCPSocket::OpenSocket entering\n");
	SOCKET sockfd = MSocket::socket(MSocket::AF::INET, MSocket::SOCK::STREAM, 0);
	if (sockfd == MSocket::InvalidSocket)
	{
		return false;
	}

	m_Socket = sockfd;

	u32 option = 1;
	int result = MSocket::ioctlsocket(sockfd, MSocket::FIO::NBIO, &option);
	if (result == MSocket::SocketError)
	{
		MLog("<TCPSOCKET_ERROR> ioctl fail </TCPSOCKET_ERROR>\n");
		return false;
	}

	int val = 1;
	result = MSocket::setsockopt(sockfd,
		MSocket::IPPROTO::TCP, MSocket::TCP::NODELAY,
		reinterpret_cast<const char*>(&val), sizeof(int));
	if (result == MSocket::SocketError) {
		MLog("<TCPSOCKET_ERROR> setsockopt(TCP_NODELAY) \n");
		return false;
	}

	Derived().Thread.Create();

	DMLog("MTCPSocket::OpenSocket leaving\n");

	return true;
}

template <typename T>
void MTCPSocket<T>::CloseSocket()
{
	DMLog("MTCPSocket::CloseSocket, active %d, thread %p\n", IsActive(), &Derived().Thread);
	if (IsActive())
	{
		Derived().Thread.Destroy();
		MSocket::closesocket(m_Socket);
	}
}

bool MClientSocket::Connect(SOCKET* pSocket, const char *szIP, int nPort)
{
	if (GetSocket() != MSocket::InvalidSocket)
		CloseSocket();

	strcpy_safe(m_szHost, szIP);
	m_nPort = nPort;

	MSocket::sockaddr_in RemoteAddr;
	memset((char*)&RemoteAddr, 0, sizeof(MSocket::sockaddr_in));
	u32 dwAddr = GetIPv4Number(szIP);
	if (dwAddr != MSocket::in_addr::None) {
		memcpy(&(RemoteAddr.sin_addr), &dwAddr, 4);
	} else {
		MSocket::hostent* pHost = MSocket::gethostbyname(szIP);
		if (pHost == NULL) {
			MLog("<TCPSOCKET_ERROR> Can't resolve hostname </TCPSOCKET_ERROR>");
			return false;
		}
		memcpy((char *)&(RemoteAddr.sin_addr), pHost->h_addr, pHost->h_length);
	}

	auto addr = RemoteAddr;

	addr.sin_family = MSocket::AF::INET;
	addr.sin_port = MSocket::htons(nPort);

	OpenSocket();
	if (pSocket) 
		*pSocket = m_Socket;

	u32 result = MSocket::connect(m_Socket, (MSocket::sockaddr*)&addr, sizeof(MSocket::sockaddr_in));
	if (MSocket::SocketError == result)	{
		auto Error = MSocket::GetLastError();
		if (Error != MSocket::Error::WouldBlock) {
			LOG_SOCKET_ERROR("connect", result);
			return false;
		}
		else
		{
			DMLog("MSocket::connect returned EWOULDBLOCK\n");
		}
	}

	DMLog("MClientSocket::Connect -- Connect succeeded\n");

	return true;
}

bool MClientSocket::Disconnect()
{
	if (!IsActive())
		return false;

	CloseSocket();

	return Thread.OnDisconnect(m_Socket);
}

bool MClientSocket::Send(char *pPacket, u32 dwPacketSize)
{
	return Thread.PushSend(pPacket, dwPacketSize);
}