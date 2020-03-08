#pragma once

#include "MTCPSocket.h"
#include "MSync.h"

class MCustomClient
{
private:

protected:
	MClientSocket		m_ClientSocket;
	MCriticalSection	m_csRecvLock;
protected:
	void LockRecv() { m_csRecvLock.lock(); }
	void UnlockRecv() { m_csRecvLock.unlock(); }

	// Socket Event
	virtual bool OnSockConnect(SOCKET sock);
	virtual bool OnSockDisconnect(SOCKET sock);
	virtual bool OnSockRecv(SOCKET sock, char* pPacket, u32 dwSize);
	virtual void OnSockError(SOCKET sock, SOCKET_ERROR_EVENT ErrorEvent, int &ErrorCode);
public:
	MCustomClient();
	virtual ~MCustomClient();
	MClientSocket* GetClientSocket()						{ return &m_ClientSocket; }

	virtual int Connect(char* szIP, int nPort);
	void Send(char* pBuf, u32 nSize);

	static bool SocketRecvEvent(void* pCallbackContext, SOCKET sock, char* pPacket, u32 dwSize);
	static bool SocketConnectEvent(void* pCallbackContext, SOCKET sock);
	static bool SocketDisconnectEvent(void* pCallbackContext, SOCKET sock);
	static void SocketErrorEvent(void* pCallbackContext, SOCKET sock, SOCKET_ERROR_EVENT ErrorEvent,
		int &ErrorCode);

	bool IsConnected() { return m_ClientSocket.IsActive(); }
	MClientSocket* GetSock() { return &m_ClientSocket; }
};