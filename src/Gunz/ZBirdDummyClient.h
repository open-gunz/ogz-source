#ifndef _ZBIRDDUMMYCLIENT_H
#define _ZBIRDDUMMYCLIENT_H

// 여기서부터 테스트를 위한 코드 - Bird ////////////////////////////////////////////////
//#ifdef _BIRDTEST

#include "MCommandCommunicator.h"
#include "MClient.h"
#include "ZGameClient.h"
#include "MTCPSocket.h"

class MClientCommandProcessor;
class MCommand;
class ZBirdDummyClient;

typedef void (*ZBT_DummyONCommand)(ZBirdDummyClient*, MCommand* pCmd);

class ZBirdDummyClient : public MCommandCommunicator
{
protected:
	int					m_nDummyID;

	MClientSocket		m_ClientSocket;
	int					m_nPBufferTop;
	MCriticalSection	m_csRecvLock;
	MCommandBuilder*	m_pCommandBuilder;
	MUID				m_Server;

	ZBT_DummyONCommand	m_fnOnCommandCallBack;
protected:
	MUID				m_uidServer;
	MUID				m_uidPlayer;
	MUID				m_uidChannel;
	MUID				m_uidStage;
	char				m_szChannel[256];
	char				m_szStageName[256];
	char				m_szPlayerName[256];
protected:
	void LockRecv() { m_csRecvLock.lock(); }
	void UnlockRecv() { m_csRecvLock.unlock(); }

	virtual void OnRegisterCommand(MCommandManager* pCommandManager);
	virtual bool OnCommand(MCommand* pCommand);

	virtual void OutputLocalInfo(void);
	virtual void OutputMessage(const char* szMessage, MZMOMType nType=MZMDM_GENERAL);

	virtual void SendCommand(MCommand* pCommand);
	virtual MCommand* GetCommandSafe();
	virtual int Connect(MCommObject* pCommObj) { return 0; }
	int OnConnected(SOCKET sock, MUID* pTargetUID, MUID* pAllocUID);

	int MakeCmdPacket(char* pOutPacket, int iMaxPacketSize, MCommand* pCommand);
	MUID GetSenderUIDBySocket(SOCKET socket);

	// Socket Event
	bool OnSockConnect(SOCKET sock);
	bool OnSockDisconnect(SOCKET sock);
	bool OnSockRecv(SOCKET sock, char* pPacket, u32 dwSize);
	void OnSockError(SOCKET sock, SOCKET_ERROR_EVENT ErrorEvent, int &ErrorCode);
protected:
	int OnResponseMatchLogin(const MUID& uidServer, int nResult, const MUID& uidPlayer);
	void OnResponseRecommandedChannel(const MUID& uidChannel, char* szChannel);
	void OnStageJoin(const MUID& uidChar, const MUID& uidStage, char* szStageName);
	void OnStageLeave(const MUID& uidChar, const MUID& uidStage);
public:
	ZBirdDummyClient();
	virtual ~ZBirdDummyClient();
	void Create(int nID, ZBT_DummyONCommand pCallBack);
public:
	bool Post(MCommand* pCommand);
	int Connect(SOCKET* pSocket, char* szIP, int nPort);
	void Disconnect(MUID uid);

	static bool SocketRecvEvent(void* pCallbackContext, SOCKET sock, char* pPacket, u32 dwSize);
	static bool SocketConnectEvent(void* pCallbackContext, SOCKET sock);
	static bool SocketDisconnectEvent(void* pCallbackContext, SOCKET sock);
	static void SocketErrorEvent(void* pCallbackContext, SOCKET sock, SOCKET_ERROR_EVENT ErrorEvent, int &ErrorCode);

public:
	MUID GetServerUID() { return m_uidServer; }
	MUID GetPlayerUID()	{ return m_uidPlayer; }
	MUID GetChannelUID() { return m_uidChannel; }
	MUID GetStageUID() { return m_uidStage; }
	const char* GetChannelName() { return m_szChannel; }
	const char* GetStageName() { return m_szStageName; }
	void SetPlayerName(const char* szPlayerName) { strcpy_safe(m_szPlayerName, szPlayerName); }
	const char* GetPlayerName() { return m_szPlayerName; }

	int GetDummyID() { return m_nDummyID; }
};

bool ZBirdPostCommand(ZBirdDummyClient* pDummyClient, MCommand* pCmd);
MCommand* ZNewBirdCmd(int nID);

#define ZBIRDPOSTCMD0(_CLIENT, _ID)									{ MCommand* pC=ZNewCmd(_ID); ZBirdPostCommand(_CLIENT, pC); }
#define ZBIRDPOSTCMD1(_CLIENT, _ID, _P0)								{ MCommand* pC=ZNewCmd(_ID); pC->AddParameter(new _P0); ZBirdPostCommand(_CLIENT, pC); }
#define ZBIRDPOSTCMD2(_CLIENT, _ID, _P0, _P1)						{ MCommand* pC=ZNewCmd(_ID); pC->AddParameter(new _P0); pC->AddParameter(new _P1); ZBirdPostCommand(_CLIENT, pC); }
#define ZBIRDPOSTCMD3(_CLIENT, _ID, _P0, _P1, _P2)					{ MCommand* pC=ZNewCmd(_ID); pC->AddParameter(new _P0); pC->AddParameter(new _P1); pC->AddParameter(new _P2); ZBirdPostCommand(_CLIENT, pC); }
#define ZBIRDPOSTCMD4(_CLIENT, _ID, _P0, _P1, _P2, _P3)				{ MCommand* pC=ZNewCmd(_ID); pC->AddParameter(new _P0); pC->AddParameter(new _P1); pC->AddParameter(new _P2); pC->AddParameter(new _P3); ZBirdPostCommand(_CLIENT, pC); }
#define ZBIRDPOSTCMD5(_CLIENT, _ID, _P0, _P1, _P2, _P3, _P4)			{ MCommand* pC=ZNewCmd(_ID); pC->AddParameter(new _P0); pC->AddParameter(new _P1); pC->AddParameter(new _P2); pC->AddParameter(new _P3); pC->AddParameter(new _P4); ZBirdPostCommand(_CLIENT, pC); }
#define ZBIRDPOSTCMD6(_CLIENT, _ID, _P0, _P1, _P2, _P3, _P4, _P5)	{ MCommand* pC=ZNewCmd(_ID); pC->AddParameter(new _P0); pC->AddParameter(new _P1); pC->AddParameter(new _P2); pC->AddParameter(new _P3); pC->AddParameter(new _P4); pC->AddParameter(new _P5); ZBirdPostCommand(_CLIENT, pC); }
#define ZBIRDPOSTCMD7(_CLIENT, _ID, _P0, _P1, _P2, _P3, _P4, _P5, _P6)	{ MCommand* pC=ZNewCmd(_ID); pC->AddParameter(new _P0); pC->AddParameter(new _P1); pC->AddParameter(new _P2); pC->AddParameter(new _P3); pC->AddParameter(new _P4); pC->AddParameter(new _P5); pC->AddParameter(new _P6); ZBirdPostCommand(_CLIENT, pC); }

void AddToLogFrame(int nDummyID, const char* szStr);

#ifdef _BIRDTEST
bool OnCommonLogin(ZBirdDummyClient* pClient, MCommand* pCmd);
#endif
//#endif


#endif