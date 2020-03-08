#pragma once

#include <list>
#include <vector>
#include <deque>
#include <algorithm>
#include "MSocket.h"
#include "MSync.h"
#include "MThread.h"
#include "MPacket.h"
#include "MCommand.h"
#include "MTrafficLog.h"

class MServerSocket;
class MClientSocket;

struct MTCPSendQueueItem
{
	char*		pPacket;
	u32			dwPacketSize;
};

using TCPSendList = std::list<MTCPSendQueueItem*>;

struct MSocketObj
{
	SOCKET				sock;
	MSignalEvent		event;
	TCPSendList			sendlist;
};

using SocketList = std::list<MSocketObj*>;

enum SOCKET_ERROR_EVENT { eeGeneral, eeSend, eeReceive, eeConnect, eeDisconnect, eeAccept };

// general callback
typedef void(MSOCKETERRORCALLBACK)(void* pCallbackContext, SOCKET sock, SOCKET_ERROR_EVENT ErrorEvent,
	int &ErrorCode);
// client callback
typedef bool(MCLIENTRECVCALLBACK)(void* pCallbackContext, SOCKET socket, char* pPacket, u32 dwSize);
typedef bool(MCONNECTCALLBACK)(void* pCallbackContext, SOCKET sock);
typedef bool(MDISCONNECTCALLBACK)(void* pCallbackContext, SOCKET sock);
// server callback
typedef bool(MSERVERRECVCALLBACK)(MSocketObj* pSocketObj, char* pPacket, u32 dwSize);
typedef bool(MACCEPTCALLBACK)(MSocketObj* pSocketObj);
typedef bool(MDISCONNECTCLIENTCALLBACK)(MSocketObj* pSocketObj);

class MTCPSocketThread : public MThread 
{
public:
	int GetSendTraffic() const { return m_SendTrafficLog.GetTrafficSpeed(); }
	int GetRecvTraffic() const { return m_RecvTrafficLog.GetTrafficSpeed(); }

	bool IsActive() const { return m_bActive; }

	// Shadows MThread::Create
	void Create();

	// Shadows MThread::Destroy
	void Destroy();

	void*						m_pCallbackContext{};
	MSOCKETERRORCALLBACK*		m_fnSocketErrorCallback{};

protected:
	void OnSocketError(SOCKET sock, SOCKET_ERROR_EVENT ErrorEvent, int &ErrorCode) {
		if (m_fnSocketErrorCallback)
			m_fnSocketErrorCallback(m_pCallbackContext, sock, ErrorEvent, ErrorCode);
	}

	MSignalEvent			m_SendEvent;
	MSignalEvent			m_KillEvent;
	MCriticalSection		m_csSendLock;
	bool					m_bActive{};

	u32						m_nTotalSend{};
	u32						m_nTotalRecv{};
	MTrafficLog				m_SendTrafficLog;
	MTrafficLog				m_RecvTrafficLog;
};

class MClientSocketThread : public MTCPSocketThread 
{
public:
	MClientSocketThread(MClientSocket* Socket) : m_pTCPSocket{ Socket } {}

	bool PushSend(char* pPacket, u32 dwPacketSize);

	bool OnDisconnect(SOCKET sock);
	int GetSendItemCount()	{ return (int)m_SendList.size(); }

	virtual void Run() override;

	MClientSocket* m_pTCPSocket{};

	MCONNECTCALLBACK*		m_fnConnectCallback{};
	MCLIENTRECVCALLBACK*	m_fnRecvCallback{};
	MDISCONNECTCALLBACK*	m_fnDisconnectCallback{};

protected:
	TCPSendList				m_SendList;			// Sending priority Low	(Safe|Normal) Packet
	TCPSendList				m_TempSendList;		// Temporary Send List for Sync

	size_t GetSendWaitQueueCount() { return m_TempSendList.size(); }

	bool OnConnect(SOCKET sock);
	bool OnRecv(SOCKET socket, char* pPacket, u32 dwSize);
	bool FlushSend();
	bool Recv();
	void ClearSendList();

	void HandleSocketEvent(MSignalEvent& hFDEvent);
};

class MServerSocketThread : public MTCPSocketThread 
{
public:
	MServerSocketThread(MServerSocket* Socket) : m_pTCPSocket{ Socket } {}

	void Disconnect(MSocketObj* pSocketObj);
	bool PushSend(MSocketObj* pSocketObj, char *pPacket, u32 dwPacketSize);

	virtual void Run() override;

	MServerSocket* m_pTCPSocket{};

	MACCEPTCALLBACK*			m_fnAcceptCallback{};
	MSERVERRECVCALLBACK*		m_fnRecvCallback{};
	MDISCONNECTCLIENTCALLBACK*	m_fnDisconnectCallback{};

private:
	bool OnRecv(MSocketObj* pSocketObj, char* pPacket, u32 dwPacketSize);
	bool OnAccept(MSocketObj* pSocketObj);
	bool OnDisconnectClient(MSocketObj* pSocketObj);

	bool FlushSend();
	bool Recv(MSocketObj* pSocketObj);
	void FreeSocketObj(MSocketObj* pSocketObj);
	SocketList::iterator RemoveSocketObj(SocketList::iterator itor);
	void RenumberEventArray();
	MSocketObj* InsertSocketObj(SOCKET sock);

	void HandleAcceptEvent();
	void HandleClientSockets();

	MCriticalSection m_csSocketLock;
	SocketList m_SocketList;

	// 0 = Accept event (from m_pTCPSocket)
	// 1 = Send event (m_SendEvent)
	// 2 = Kill event (m_KillEvent)
	// 3-63 = Client socket events
	MSignalEvent* m_EventArray[64];
	static constexpr auto ClientSocketsStartIndex = 3;
};

template <typename T>
class MTCPSocket
{
public:
	MTCPSocket() {
		MSocket::Startup();
	}
	~MTCPSocket() {
		MSocket::Cleanup();
	}

	SOCKET GetSocket() const { return m_Socket; }
	int GetPort() const { return m_nPort; }
	bool IsActive() const { return Derived().Thread.IsActive(); }

	void GetTraffic(int* nSendTraffic, int* nRecvTraffic) {
		*nSendTraffic = Derived().Thread.GetSendTraffic();
		*nRecvTraffic = Derived().Thread.GetRecvTraffic();
	}

	void SetSocketErrorCallback(MSOCKETERRORCALLBACK pCallback) {
		Derived().Thread.m_fnSocketErrorCallback = pCallback;
	}
	void SetCallbackContext(void* pCallbackContext) {
		Derived().Thread.m_pCallbackContext = pCallbackContext;
	}
	template <typename U>
	void SetRecvCallback(U pCallback) {
		Derived().Thread.m_fnRecvCallback = pCallback;
	}
	template <typename U>
	void SetDisconnectCallback(U pCallback) {
		Derived().Thread.m_fnDisconnectCallback = pCallback;
	}

protected:
	bool OpenSocket();
	void CloseSocket();
	
	int							m_nPort{};
	SOCKET						m_Socket = MSocket::InvalidSocket;

	T& Derived() { return *static_cast<T*>(this); }
	const T& Derived() const { return *static_cast<const T*>(this); }
};

class MServerSocket : public MTCPSocket<MServerSocket>
{
public:
	MServerSocket() : Thread{ this } {}
	~MServerSocket() = default;

	bool Listen(int nPort);
	bool Close();
	bool Disconnect(MSocketObj* pSocketObj);

	bool Send(MSocketObj* pSocketObj, char* pPacket, u32 dwPacketSize);

	void SetAcceptCallback(MACCEPTCALLBACK pCallback) { 
		Thread.m_fnAcceptCallback = pCallback;
	}

	u32 GetLocalIP() const { return m_LocalAddress.sin_addr.S_un.S_addr; }

	bool OpenSocket(int nPort);

	MServerSocketThread Thread;

	MSocket::sockaddr_in m_LocalAddress;
};

class MClientSocket : public MTCPSocket<MClientSocket>
{
public:
	MClientSocket() : Thread{ this } {}
	~MClientSocket() { CloseSocket(); }

	bool Connect(SOCKET* pSocket, const char* szIP, int nPort);
	bool Disconnect();
	bool Send(char *pPacket, u32 dwPacketSize);

	void SetConnectCallback(MCONNECTCALLBACK pCallback) {
		Thread.m_fnConnectCallback = pCallback;
	}

	const char* GetHost() const { return m_szHost; }

	MClientSocketThread Thread;

	char m_szHost[255];
};