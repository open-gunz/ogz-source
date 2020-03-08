#pragma once

//////////////////////////////////////////////////////////////////
// Class:	MRealCPNet class (2001/10/25)
// File:	RealCPNet.cpp
// Author:	Kim young ho (moanus@maiet.net)
//
// Implements Scalable Network Module with I/O Competion Port.
// Code based on MS iocpserver/iocpclient example
////////////////////////////////////////////////////////////////// 

#include <list>
#include <algorithm>
#include "MPacket.h"
#include "MSocket.h"
#include "MSync.h"
#include "MUtil.h"

#ifdef REALCPNET_LINK_SOCKET_LIBS
#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "mswsock.lib")
#endif

// Constants
#define DEFAULT_PORT		5000
#define MAX_BUFF_SIZE		8192
#define MAX_WORKER_THREAD	16


typedef std::list<MPacketHeader*>		MPacketList;
typedef MPacketList::iterator		MPacketListItor;
class MRealCPNet;


enum RCP_IO_OPERATION {
	RCP_IO_NONE,
	RCP_IO_ACCEPT,
	RCP_IO_CONNECT,
	RCP_IO_DISCONNECT,
	RCP_IO_READ,
	RCP_IO_WRITE,
};

// For AcceptEx, the IOCP key is the MRealSession for the listening socket,
// so we need to another field SocketAccept in PER_IO_CONTEXT. When the outstanding
// AcceptEx completes, this field is our connection socket handle.
class MRealSession {
public:
	enum SESSIONSTATE { SESSIONSTATE_IDLE, SESSIONSTATE_ACTIVE, SESSIONSTATE_DEAD };

	MRealSession() = default;
	virtual ~MRealSession() = default;

	void SetSocket(SOCKET sd)	{ m_sdSocket = sd; }
	SOCKET	GetSocket()			{ return m_sdSocket; }

	void SetSockAddr(MSocket::sockaddr_in* pAddr, int nAddrLen) {
		memcpy(&m_SockAddr, pAddr, (std::min)(static_cast<int>(sizeof(MSocket::sockaddr_in)), nAddrLen)); }
	MSocket::sockaddr_in* GetSockAddr()	{ return &m_SockAddr; }
	u32 GetIP() const { return m_SockAddr.sin_addr.S_un.S_addr; }
	int GetPort() const { return MSocket::ntohs(m_SockAddr.sin_port); }
	u16 GetRawPort() const { return m_SockAddr.sin_port; }

	void SetSessionState(SESSIONSTATE nState) { m_nSessionState = nState; }
	SESSIONSTATE GetSessionState() const { return m_nSessionState; }
	void SetUserContext(void* pContext) { m_pUserContext = pContext; }
	void* GetUserContext() const { return m_pUserContext; }

	char m_RecvBuffer[MAX_BUFF_SIZE]{};

private:
	SOCKET						m_sdSocket = MSocket::InvalidSocket;
	MSocket::sockaddr_in		m_SockAddr{};
	SESSIONSTATE				m_nSessionState = SESSIONSTATE_IDLE;
	void*						m_pUserContext{};
};


class MSessionMap : public std::map<SOCKET, MRealSession*> {
public:
	MCriticalSection m_csLock;

	void Lock() { m_csLock.lock(); }
	void Unlock() { m_csLock.unlock(); }

	// Safe methods
	void Add(MRealSession* pSession) {
		std::lock_guard<MCriticalSection> lock{ m_csLock };

		_ASSERT(pSession->GetSocket() != MSocket::InvalidSocket);
		insert(MSessionMap::value_type(pSession->GetSocket(), pSession));
	}
	bool Remove(SOCKET sd, MSessionMap::iterator* pNextItor = nullptr) {
		std::lock_guard<MCriticalSection> lock{ m_csLock };
		return RemoveUnsafe(sd, pNextItor);
	}
	void RemoveAll() {
		std::lock_guard<MCriticalSection> lock{ m_csLock };

		for (auto* Session : MakePairValueAdapter(*this))
			delete Session;
		
		clear();
	}
	bool IsExist(SOCKET sd) {
		std::lock_guard<MCriticalSection> lock{ m_csLock };

		auto it = find(sd);
		return it != end();
	}
	MRealSession* GetSession(SOCKET sd) {
		std::lock_guard<MCriticalSection> lock{ m_csLock };

		auto it = find(sd);
		if (it == end())
			return nullptr;

		return it->second;
	}

	// Unsafe methods
	MRealSession* GetSessionUnsafe(SOCKET sd) {
		auto it = find(sd);
		if (it == end())
			return nullptr;

		return it->second;
	}
	bool IsExistUnsafe(MRealSession* pSession) {
		for (MSessionMap::iterator i=begin(); i!=end(); i++) {
			MRealSession* pItorSession = (*i).second;
			if (pItorSession == pSession)
				return true;
		}
		return false;
	}
	bool RemoveUnsafe(SOCKET sd, MSessionMap::iterator* pNextItor = nullptr) {
		auto it = find(sd);
		if (it == end())
			return false;

		delete it->second;
		auto next_it = erase(it);
		if (pNextItor)
			*pNextItor = next_it;

		return true;
	}
};

using RCPCALLBACK = void(void* pCallbackContext, RCP_IO_OPERATION nIO,
	u32 dwKey, MPacketHeader* pPacket, u32 dwPacketLen);

class MRealCPNet {
private:
	unsigned short		m_nPort = DEFAULT_PORT;
	bool				m_bEndServer{};
	bool				m_bRestart = true;
	bool				m_bVerbose{};
	HANDLE				m_hIOCP{};
	SOCKET				m_sdListen = MSocket::InvalidSocket;
	u32					m_dwThreadCount{};
	MSignalEvent		m_hCleanupEvent{};

	MRealSession*		m_pListenSession{};
	MSessionMap			m_SessionMap;

	MCriticalSection	m_csCrashDump;

	RCPCALLBACK*		m_fnCallback{};
	void*				m_pCallbackContext{};

protected:
	SOCKET CreateSocket();
	bool CreateListenSocket(bool bReuse);
	bool CreateAcceptSocket(bool fUpdateIOCP);

	MRealSession* UpdateCompletionPort(SOCKET sd, RCP_IO_OPERATION nOperation, bool bAddToList);
	// bAddToList is FALSE for listening socket, and TRUE for connection sockets.
	// As we maintain the context for listening socket in a global structure, we
	// don't need to add it to the list.

	bool PostIOSend(SOCKET sd, char* pBuf, u32 nBufLen);
	void PostIORecv(SOCKET sd);

	static bool MakeSockAddr(const char* pszIP, int nPort, MSocket::sockaddr_in* pSockAddr);
	bool CheckIPFloodAttack(MSocket::sockaddr_in* pRemoteAddr, int* poutIPCount);

	MRealSession* CreateSession(SOCKET sd, RCP_IO_OPERATION ClientIO);
	void DeleteAllSession();

	void WorkerThread();

public:
	MRealCPNet();
	~MRealCPNet();
	bool Create(int nPort, const bool bReuse = false );
	void Destroy();

	void SetLogLevel(int nLevel) { m_bVerbose = nLevel > 0; }
	void SetCallback(RCPCALLBACK* pCallback, void* pCallbackContext) {
		m_fnCallback = pCallback; m_pCallbackContext = pCallbackContext; }

	bool Connect(SOCKET* pSocket, const char* pszAddress, int nPort);
	void Disconnect(SOCKET sd);
	void CloseSession(MRealSession* pSession, bool bGraceful);

	template<size_t size> bool GetAddress(SOCKET sd, char(&pszAddress)[size], int *pPort) {
		return GetAddress(sd, pszAddress, size, pPort);
	}
	bool GetAddress(SOCKET sd, char* pszAddress, size_t maxlen, int* pPort);
	void* GetUserContext(SOCKET sd);
	void SetUserContext(SOCKET sd, void* pContext);

	bool Send(SOCKET sd, MPacketHeader* pPacket, int nSize);

friend RCPCALLBACK;
};


typedef void(RCPLOGFUNC)(const char *pFormat,...);
void SetupRCPLog(RCPLOGFUNC* pFunc);