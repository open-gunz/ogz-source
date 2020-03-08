#pragma once

/////////////////////////////////////////////////////////////
//	SafeUDP.h	- SafeUDP 1.9.2
//								 Programmed by Kim Young-Ho 
//								    LastUpdate : 2000/07/25
/////////////////////////////////////////////////////////////

#include <map>
#include <list>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "MTime.h"
#include "MSync.h"
#include "MThread.h"
#include "MBasePacket.h"
#include "MTrafficLog.h"
#include "MInetUtil.h"
#include <memory>

#ifdef WIN32
#pragma comment(lib, "ws2_32.lib")
#endif

class MSafeUDP;

// INNER CLASS //////////////////////////////////////////////////////////////////////////
struct MSendQueueItem {
	u32 dwIP;
	u16 wRawPort;
	MBasePacket* pPacket;
	u32 dwPacketSize;
};

struct MACKQueueItem {
	u32 dwIP;
	u16 wRawPort;
	u8 nSafeIndex;
};

struct MACKWaitItem {
	std::unique_ptr<MSafePacket> pPacket;
	u32 dwPacketSize;
	MTime::timeval tvFirstSent;
	MTime::timeval tvLastSent;
	u8 nSendCount;
};

// OUTER CLASS //////////////////////////////////////////////////////////////////////////
class MNetLink {
public:
	enum LINKSTATE {
		LINKSTATE_CLOSED,
		LINKSTATE_ESTABLISHED,
		LINKSTATE_SYN_SENT,
		LINKSTATE_SYN_RCVD,
		LINKSTATE_FIN_SENT,
		LINKSTATE_FIN_RCVD
	};

	typedef std::list<MACKWaitItem*>		ACKWaitList;
	typedef ACKWaitList::iterator	ACKWaitListItor;

private:
	MSafeUDP*		m_pSafeUDP{};
	bool			m_bConnected{};
	LINKSTATE		m_nLinkState = LINKSTATE_CLOSED;
	MSocket::sockaddr_in m_Address{}; // IP and Port
	u8				m_nNextReadIndex{};
	u8				m_nNextWriteIndex{};

	u32				m_dwAuthKey{};
	void*			m_pUserData{};

public:
	MTime::timeval			m_tvConnectedTime{};
	MTime::timeval			m_tvLastPacketRecvTime{};

public:
	ACKWaitList		m_ACKWaitQueue;	// Safe Sent queue, Wait for ACK

private:
	void Setconnected(bool bConnected)	{ m_bConnected = bConnected; }
	void CreateAuthKey() { 	
		m_dwAuthKey = rand() * rand();
	}
	u8 GetNextReadIndex() { return m_nNextReadIndex++; }
	u8 GetNextWriteIndex() { return m_nNextWriteIndex++; }

public:
	MNetLink();
	~MNetLink();

	bool SendControl(MControlPacket::CONTROL nControl);
	bool OnRecvControl(MControlPacket* pPacket);

	bool SetACKWait(MSafePacket* pPacket, u32 dwPacketSize);
	bool ClearACKWait(u8 nSafeIndex);

	void SetSafeUDP(MSafeUDP* pSafeUDP)	{ m_pSafeUDP = pSafeUDP; }
	MSafeUDP* GetSafeUDP()				{ return m_pSafeUDP; }
	bool IsConnected()					{ return m_bConnected; }
	MTime::timeval GetConnectedTime()			{ return m_tvConnectedTime; }
	void SetLinkState(MNetLink::LINKSTATE nState);
	MNetLink::LINKSTATE GetLinkState()	{ return m_nLinkState; }
	static bool MakeSockAddr(const char* pszIP, int nPort, MSocket::sockaddr_in* pSockAddr);
	bool SetAddress(char* pszIP, int nPort);
	std::string GetIPString() const		{ return GetIPv4String(m_Address.sin_addr); }
	u32 GetIP() const					{ return m_Address.sin_addr.S_un.S_addr; }
	u16 GetRawPort()					{ return m_Address.sin_port; }
	int GetPort();
	MSocket::sockaddr_in* GetSockAddr()			{ return &m_Address; }
	i64 GetMapKey();
	static i64 GetMapKey(MSocket::sockaddr_in* pSockAddr);
	MTime::timeval GetLastPacketRecvTime()		{ return m_tvLastPacketRecvTime; }

	u32 GetAuthKey() const			{ return m_dwAuthKey; }
	void SetUserData(void* pUserData)	{ m_pUserData = pUserData; }
	void* GetUserData() const			{ return m_pUserData; }
};

typedef std::map<i64, MNetLink*>	NetLinkMap;
typedef NetLinkMap::value_type	NetLinkType;
typedef NetLinkMap::iterator	NetLinkItor;


// INNER CLASS //////////////////////////////////////////////////////////////////////////
typedef void(MNETLINKSTATECALLBACK)(MNetLink* pNetLink, MNetLink::LINKSTATE nState);
typedef bool(MCUSTOMRECVCALLBACK)(u32 dwIP, u16 wRawPort, char* pPacket, u32 dwSize);	// Real UDP Packet
typedef void(MLIGHTRECVCALLBACK)(u32 dwIP, u16 wRawPort, MLightPacket* pPacket, u32 dwSize);
typedef void(MGENERICRECVCALLBACK)(MNetLink* pNetLink, MBasePacket* pPacket, u32 dwSize);

class MSafeUDP;
class MSocketThread : public MThread
{
public:
	typedef std::list<MACKQueueItem*>	ACKSendList;
	typedef ACKSendList::iterator	ACKSendListItor;
	typedef std::list<MSendQueueItem*>	SendList;
	typedef SendList::iterator		SendListItor;

	MCUSTOMRECVCALLBACK*	m_fnCustomRecvCallback{};
	MLIGHTRECVCALLBACK*		m_fnLightRecvCallback{};
	MGENERICRECVCALLBACK*	m_fnGenericRecvCallback{};

	// Shadows MThread::Create.
	void Create();

	// Shadows MThread::Destroy.
	void Destroy();

	MSafeUDP* GetSafeUDP() const { return m_pSafeUDP; }
	void SetSafeUDP(MSafeUDP* pSafeUDP)	{ m_pSafeUDP = pSafeUDP; }

	bool PushSend(MNetLink* pNetLink, MBasePacket* pPacket, u32 dwpPacketSize, bool bRetransmit);	
	bool PushSend(const char* pszIP, int nPort, char* pPacket, u32 dwPacketSize);
	bool PushSend(u32 dwIP, int nPort, char* pPacket, u32 dwPacketSize );

	int GetSendTraffic() const { return m_SendTrafficLog.GetTrafficSpeed(); }
	int GetRecvTraffic() const { return m_RecvTrafficLog.GetTrafficSpeed(); }

	virtual void Run() override;

private:
	void LockACK() { m_csACKLock.lock(); }
	void UnlockACK() { m_csACKLock.unlock(); }
	void LockSend() { m_csSendLock.lock(); }
	void UnlockSend() { m_csSendLock.unlock(); }

	bool PushACK(MNetLink* pNetLink, MSafePacket* pPacket);
	bool FlushACK();
	bool FlushSend();

	bool SafeSendManage();

	bool Recv();
	bool OnCustomRecv(u32 dwIP, u16 wRawPort, char* pPacket, u32 dwSize);
	bool OnControlRecv(u32 dwIP, u16 wRawPort, MBasePacket* pPacket, u32 dwSize);
	bool OnLightRecv(u32 dwIP, u16 wRawPort, MLightPacket* pPacket, u32 dwSize);
	bool OnACKRecv(u32 dwIP, u16 wRawPort, MACKPacket* pPacket);
	bool OnGenericRecv(u32 dwIP, u16 wRawPort, MBasePacket* pPacket, u32 dwSize);

	template <typename T>
	bool SendPacket(const T& DestAddr, const void* Data, size_t DataSize);

	MSafeUDP*				m_pSafeUDP{};
	MSignalEvent			m_ACKEvent;
	MSignalEvent			m_SendEvent;
	MSignalEvent			m_KillEvent;

	ACKSendList				m_ACKSendList;		// Sending priority High
	ACKSendList				m_TempACKSendList;	// Temporary ACK List for Sync
	MCriticalSection		m_csACKLock;

	SendList				m_SendList;			// Sending priority Low	(Safe|Normal) Packet
	SendList				m_TempSendList;		// Temporary Send List for Sync
	MCriticalSection		m_csSendLock;

	u32						m_nTotalSend{};
	u32						m_nTotalRecv{};
	MTrafficLog				m_SendTrafficLog;
	MTrafficLog				m_RecvTrafficLog;
};

class MSafeUDP
{
public:
	bool Create(bool bBindWinsockDLL, int nPort, bool bReuse = true);
	void Destroy();

	void SetNetLinkStateCallback(MNETLINKSTATECALLBACK pCallback) { m_fnNetLinkStateCallback = pCallback; }
	void SetLightRecvCallback(MLIGHTRECVCALLBACK pCallback) { m_SocketThread.m_fnLightRecvCallback = pCallback; }
	void SetGenericRecvCallback(MGENERICRECVCALLBACK pCallback) { m_SocketThread.m_fnGenericRecvCallback = pCallback; }
	void SetCustomRecvCallback(MCUSTOMRECVCALLBACK pCallback) { m_SocketThread.m_fnCustomRecvCallback = pCallback; }

	MNetLink* OpenNetLink(char* szIP, int nPort);
	bool CloseNetLink(MNetLink* pNetLink);

	MNetLink* Connect(char* szIP, int nPort);
	void Reconnect(MNetLink* pNetLink);
	bool Disconnect(MNetLink* pNetLink);
	int DisconnectAll();

	bool Send(MNetLink* pNetLink, MBasePacket* pPacket, u32 dwSize);
	bool Send(const char* pszIP, int nPort, char* pPacket, u32 dwSize);
	bool Send(u32 dwIP, int nPort, char* pPacket, u32 dwSize );

	SOCKET GetLocalSocket() const { return m_Socket; }
	std::string GetLocalIPString() const { return GetIPv4String(m_LocalAddress.sin_addr); }
	u32 GetLocalIP() const { return m_LocalAddress.sin_addr.S_un.S_addr; }
	u16 GetLocalPort() const { return m_LocalAddress.sin_port; }

	MNetLink* FindNetLink(u32 dwIP, u16 wRawPort);
	MNetLink* FindNetLink(i64 nMapKey);

	void GetTraffic(int* nSendTraffic, int* nRecvTraffic) const {
		*nSendTraffic = m_SocketThread.GetSendTraffic();
		*nRecvTraffic = m_SocketThread.GetRecvTraffic();
	}

	void LockNetLink() { m_csNetLink.lock(); }
	void UnlockNetLink() { m_csNetLink.unlock(); }

	MCriticalSection			m_csNetLink;

	NetLinkMap					m_NetLinkMap;
	MNETLINKSTATECALLBACK*		m_fnNetLinkStateCallback;

private:
	bool OpenSocket(int nPort, bool bReuse = true);
	void CloseSocket();
	void OnConnect(MNetLink* pNetLink);
	void OnDisconnect(MNetLink* pNetLink);

	bool						m_bBindWinsockDLL;	// Socket DLL Load
	SOCKET						m_Socket;			// My Socket
	MSocket::sockaddr_in		m_LocalAddress;		// My IP and Port

	MSocketThread				m_SocketThread;
};