#pragma once

#include <list>
#include <map>
#include "MMatchGlobal.h"
#include "MCommandCommunicator.h"
#include "MClient.h"
#include "MSafeUDP.h"
#include "MMatchObjCache.h"
#include "MMatchTransDataType.h"
#include "MMatchGlobal.h"
#include "MPacketCrypter.h"
#include "MTCPSocket.h"

#define MATCHCLIENT_DEFAULT_UDP_PORT	10000
#define MAX_PING						999

class MMatchPeerInfo
{
public:
	MUID	uidChar;
	char	szIP[64];
	u32		dwIP;
	int		nPort;
	MTD_CharInfo		CharInfo;
	MTD_ExtendInfo		ExtendInfo;
protected:
	bool				m_bUDPTestResult;
	bool				m_bUDPTestProcess;
	int					m_nUDPTestCount;

	bool				m_bOpened;
	int					m_nPing;
	int					m_nPingTryCount;
	unsigned int		m_nLastPingTime;
	unsigned int		m_nLastPongTime;
	MCommandSNChecker	m_CommandSNChecker;
public:
	MMatchPeerInfo() {
		uidChar = MUID(0,0);
		szIP[0] = NULL;
		dwIP = 0;
		nPort = 0;
		memset(&CharInfo, 0, sizeof(MTD_CharInfo));

		m_bUDPTestResult = false;
		m_bUDPTestProcess = false;
		m_nUDPTestCount = 0;

		m_bOpened = false;
		m_nPing = 0;
		m_nLastPingTime = 0;
		m_nLastPongTime = 0;
		m_nPingTryCount = 0;
	}
	virtual ~MMatchPeerInfo()			{}
	bool GetUDPTestResult()				{ return m_bUDPTestResult; }
	void SetUDPTestResult(bool bResult)	{ m_bUDPTestResult = bResult; }
	void StartUDPTest()					{ m_bUDPTestProcess = true; m_nUDPTestCount = 10; }
	void StopUDPTest()					{ m_bUDPTestProcess = false; m_nUDPTestCount = 0; }
	bool GetProcess()					{ return m_bUDPTestProcess; }
	int GetTestCount()					{ return m_nUDPTestCount; }
	void UseTestCount()					{ m_nUDPTestCount--; }

	bool IsOpened()						{ return m_bOpened; }
	void SetOpened(bool bVal)			{ m_bOpened = bVal; }
	int GetPing(unsigned int nCurrTime);
	void UpdatePing(unsigned int nTime, int nPing);
	void SetLastPingTime(unsigned int nTime);
	bool CheckCommandValidate(MCommand* pCmd)
	{
		return m_CommandSNChecker.CheckValidate(pCmd->m_nSerialNumber);
	}
};

struct IPnPort
{
	u32 IP;
	u16 Port;

	IPnPort() = default;
	IPnPort(u32 IP, u16 Port) : IP{ IP }, Port{ Port } {}
	IPnPort(MMatchPeerInfo* pPeerInfo) : IP{ pPeerInfo->dwIP }, Port{ static_cast<u16>(pPeerInfo->nPort) } {}

	u64 Packed() const { return (u64(IP) << 32) | Port; }

	bool operator==(const IPnPort& rhs) const { return Packed() == rhs.Packed(); }
};

struct IPnPortHasher {
	size_t operator()(const IPnPort& Value) const {
		auto Packed = Value.Packed();
		return std::hash<u64>{}(Packed);
	}
};

class MMatchPeerInfoList
{
public:
	MMatchPeerInfoList();
	~MMatchPeerInfoList();
	void Clear();
	void Add(MMatchPeerInfo* pPeerInfo);
	bool Delete(MMatchPeerInfo* pPeerInfo);
	MMatchPeerInfo* Find(const MUID& uidChar);
	MUID FindUID(u32 dwIP, int nPort);

	void Lock() { m_csLock.lock(); }
	void Unlock() { m_csLock.unlock(); }

	std::unordered_map<MUID, MMatchPeerInfo*> MUIDMap;
	std::unordered_map<IPnPort, MMatchPeerInfo*, IPnPortHasher> IPnPortMap;
	MCriticalSection m_csLock;
};

class MMatchClient : public MClient
{
protected:
	MUID				m_uidServer;
	// Note that this is equivalent to m_This
	MUID				m_uidPlayer;
	MUID				m_uidChannel;
	MUID				m_uidStage;

	char				m_szServerName[64];
	char				m_szServerIP[32];
	int					m_nServerPort;
	int					m_nServerPeerPort;
	MMatchServerMode	m_nServerMode;

protected:
	MMatchObjCacheMap	m_ObjCacheMap;
	MSafeUDP			m_SafeUDP;
	MMatchPeerInfoList	m_Peers;
	bool				m_bBridgePeerFlag;
	bool				m_bUDPTestProcess;
	MPacketCrypter		m_AgentPacketCrypter;
	MPacketCrypter		m_PeerPacketCrypter;
protected:
	MClientSocket		m_AgentSocket;

	MUID				m_uidAgentServer;
	MUID				m_uidAgentClient;

	char				m_szAgentIP[32];
	int					m_nAgentPort;
	int					m_nAgentPeerPort;

	bool				m_bAgentPeerFlag;
	int					m_nAgentPeerCount;

	bool				m_bAllowTunneling;

	bool PeerToPeer = true;

public:
	MCommand* MakeCmdFromTunnelingBlob(const MUID& uidSender, void* pBlob, int nBlobArrayCount);
	bool MakeTunnelingCommandBlob(MCommand* pWrappingCmd, MCommand* pSrcCmd);
protected:
	bool GetAgentPeerFlag()				{ return m_bAgentPeerFlag; }
	void SetAgentPeerFlag(bool bVal)	{ m_bAgentPeerFlag = bVal; }
	int GetAgentPeerCount()				{ return m_nAgentPeerCount; }
	void SetAgentPeerCount(int nCount)	{ m_nAgentPeerCount = nCount; }
	void StartAgentPeerConnect();
	void CastAgentPeerConnect();
	void StartUDPTest(const MUID& uidChar);
	void InitPeerCrypt(const MUID& uidStage, unsigned int nChecksum);
protected:
	// tcp socket event
	virtual bool OnSockConnect(SOCKET sock) override;
	virtual bool OnSockDisconnect(SOCKET sock) override;
	virtual bool OnSockRecv(SOCKET sock, char* pPacket, u32 dwSize) override;
	virtual void OnSockError(SOCKET sock, SOCKET_ERROR_EVENT ErrorEvent, int &ErrorCode) override;

	virtual int OnConnected(SOCKET sock, MUID* pTargetUID, MUID* pAllocUID, unsigned int nTimeStamp) override;
	virtual void OnRegisterCommand(MCommandManager* pCommandManager) override;
	virtual bool OnCommand(MCommand* pCommand) override;
	virtual void SendCommand(MCommand* pCommand) override;

	virtual int OnResponseMatchLogin(const MUID& uidServer, 
									 int nResult, 
									 const char* szServerName, 
		                             const MMatchServerMode nServerMode, 
									 const char* szAccountID, 
                                     const MMatchUserGradeID nUGradeID, 
                                     const MMatchPremiumGradeID nPGradeID,
									 const MUID& uidPlayer,
									 const char* szRandomValue);
	virtual void OnObjectCache(unsigned int nType, void* pBlob, int nCount);
	virtual void OnUDPTest(const MUID& uidChar);
	virtual void OnUDPTestReply(const MUID& uidChar);
	virtual void OnResponseAgentLogin();
	virtual void OnLocateAgentToClient(const MUID& uidAgent, char* szIP, int nPort, int nUDPPort);
	virtual void OnTunnelingTCP(const MUID& uidSender, void* pBlob, int nCount);
	virtual void OnTunnelingUDP(const MUID& uidSender, void* pBlob, int nCount);	
	virtual void OnAllowTunnelingTCP();
	virtual void OnAllowTunnelingUDP();	
	virtual void OnAgentConnected(const MUID& uidAgentServer, const MUID& uidAlloc);
	virtual void OnAgentError(int nError);

	void OutputLocalInfo();

	bool SendCommandToAgent(MCommand* pCommand);
	void SendCommandByTunneling(MCommand* pCommand);
	void SendCommandByMatchServerTunneling(MCommand* pCommand, const MUID& Receiver);
	void SendCommandByMatchServerTunneling(MCommand* pCommand);
	void ParseUDPPacket(char* pData,MPacketHeader* pPacketHeader,u32 dwIP,unsigned int nPort);
public:
	void SendCommandByUDP(MCommand* pCommand, const char* szIP, int nPort);

public:
	MMatchClient();
	virtual ~MMatchClient() override;

	struct CreationResult {
		int ErrorCode;
		std::string ErrorMessage;
	};
	CreationResult Create(u16 nUDPPort);
	
	bool GetBridgePeerFlag()			{ return m_bBridgePeerFlag; }
	void SetBridgePeerFlag(bool bFlag)	{ m_bBridgePeerFlag = bFlag; }
	void AddPeer(MMatchPeerInfo* pPeerInfo);
	bool DeletePeer(const MUID uid);
	MUID FindPeerUID(const u32 dwIP, const int nPort);
	MMatchPeerInfo* FindPeer(const MUID& uidChar);
	void ClearPeers();
	void CastStageBridgePeer(const MUID& uidChar, const MUID& uidStage);

	bool GetUDPTestProcess()			{ return m_bUDPTestProcess; }
	void SetUDPTestProcess(bool bVal)	{ m_bUDPTestProcess = bVal; }
	void UpdateUDPTestProcess();
	void GetUDPTraffic(int* nSendTraffic, int* nRecvTraffic) const {
		return m_SafeUDP.GetTraffic(nSendTraffic, nRecvTraffic);
	}

	void SetUDPPort(int nPort);
	auto& GetServerUID() const { return m_uidServer; }
	auto& GetPlayerUID() const { return m_uidPlayer; }
	auto& GetChannelUID() const { return m_uidChannel; }
	auto& GetStageUID() const { return m_uidStage; }
	virtual MUID GetSenderUIDBySocket(SOCKET socket);


	void SetServerAddr(const char* szIP, int nPort)	{ 
		strcpy_safe(m_szServerIP, szIP); m_nServerPort = nPort;
	}
	const char* GetServerIP() const { return m_szServerIP; }
	int GetServerPort() const { return m_nServerPort; }
	void SetServerPeerPort(int nPeerPort) { m_nServerPeerPort = nPeerPort; }
	int GetServerPeerPort() const { return m_nServerPeerPort; }

	MMatchPeerInfoList* GetPeers() { return &m_Peers; }	
	MSafeUDP* GetSafeUDP() { return &m_SafeUDP; }
	std::string GetObjName(const MUID& uid);
	MMatchObjCache* FindObjCache(const MUID& uid);
	void ReplaceObjCache(MMatchObjCache* pCache);
	void UpdateObjCache(MMatchObjCache* pCache);
	void RemoveObjCache(const MUID& uid);
	void ClearObjCaches();

	static bool UDPSocketRecvEvent(u32 dwIP, u16 wRawPort, char* pPacket, u32 dwSize);

public:
	void SetAgentAddr(const char* szIP, int nPort)	{ 
		strcpy_safe(m_szAgentIP,szIP), m_nAgentPort = nPort; 
	}
	char* GetAgentIP() { return m_szAgentIP; }
	int GetAgentPort() { return m_nAgentPort; }
	void SetAgentPeerPort(int nPeerPort) { m_nAgentPeerPort = nPeerPort; }
	int GetAgentPeerPort() { return m_nAgentPeerPort; }
	const MUID& GetAgentServerUID() { return m_uidAgentServer; }
	const MUID& GetAgentClientUID() { return m_uidAgentClient; }
	bool GetAllowTunneling() { return m_bAllowTunneling; }
	void SetAllowTunneling(bool bAllow) { m_bAllowTunneling = bAllow; }

	int AgentConnect(SOCKET* pSocket, char* szIP, int nPort);
	void AgentDisconnect();

	MMatchObjCacheMap* GetObjCacheMap() { return &m_ObjCacheMap; }
	MMatchServerMode GetServerMode()	{ return m_nServerMode; }
	const char* GetServerName()			{ return m_szServerName; }

protected:
	virtual void OnStopUDPTest(const MUID& uid) = 0;
};
