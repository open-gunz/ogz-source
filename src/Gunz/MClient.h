#pragma once

#include "MCommandCommunicator.h"
#include "MPacketCrypter.h"
#include "MTCPSocket.h"

class MCommand;

enum MZMOMType{
	MZMDM_GENERAL,
	MZMOM_USERCOMMAND,
	MZMOM_ERROR,
	MZMOM_LOCALREPLY,
	MZMOM_SERVERREPLY,
};

class MClient : public MCommandCommunicator {
private:
	static MClient*	m_pInstance;
protected:
	MUID				m_Server;

	MClientSocket		m_ClientSocket;
	MCriticalSection	m_csRecvLock;

	MCommandBuilder*	m_pCommandBuilder;
	MPacketCrypter		m_ServerPacketCrypter;

protected:
	void LockRecv() { m_csRecvLock.lock(); }
	void UnlockRecv() { m_csRecvLock.unlock(); }

	virtual void OnRegisterCommand(MCommandManager* pCommandManager);
	virtual bool OnCommand(MCommand* pCommand);

	virtual void OutputLocalInfo() = 0;
	virtual void OutputMessage(const char* szMessage, MZMOMType nType=MZMDM_GENERAL) = 0;

	MCommandBuilder* GetCommandBuilder() { return m_pCommandBuilder; }
	virtual void SendCommand(MCommand* pCommand);
	virtual MCommand* GetCommandSafe();

	virtual int OnConnected(MUID* pTargetUID, MUID* pAllocUID, unsigned int nTimeStamp,
		MCommObject* pCommObj);
	virtual int OnConnected(SOCKET sock, MUID* pTargetUID, MUID* pAllocUID, unsigned int nTimeStamp);
	int MakeCmdPacket(char* pOutPacket, int iMaxPacketSize, MPacketCrypter* pPacketCrypter,
		MCommand* pCommand);
	

	// Socket Event
	virtual bool OnSockConnect(SOCKET sock);
	virtual bool OnSockDisconnect(SOCKET sock);
	virtual bool OnSockRecv(SOCKET sock, char* pPacket, u32 dwSize);
	virtual void OnSockError(SOCKET sock, SOCKET_ERROR_EVENT ErrorEvent, int &ErrorCode);

public:
	MClient();
	virtual ~MClient();

	static MClient* GetInstance();

	MUID GetServerUID(){ return m_Server; }
	virtual MUID GetSenderUIDBySocket(SOCKET socket);
	MClientSocket* GetClientSocket()						{ return &m_ClientSocket; }
	void GetTraffic(int* nSendTraffic, int* nRecvTraffic)	{ return m_ClientSocket.GetTraffic(nSendTraffic, nRecvTraffic); }

	virtual bool Post(MCommand* pCommand);
	virtual bool Post(char* szErrMsg, int nErrMsgCount, const char* szCommand);

	virtual int Connect(MCommObject* pCommObj);
	virtual int Connect(SOCKET* pSocket, char* szIP, int nPort);
	virtual void Disconnect(MUID uid);
	virtual void Log(const char* szLog){}

	void OutputMessage(MZMOMType nType, const char *pFormat,...);

	static bool SocketRecvEvent(void* pCallbackContext, SOCKET sock, char* pPacket, u32 dwSize);
	static bool SocketConnectEvent(void* pCallbackContext, SOCKET sock);
	static bool SocketDisconnectEvent(void* pCallbackContext, SOCKET sock);
	static void SocketErrorEvent(void* pCallbackContext, SOCKET sock, SOCKET_ERROR_EVENT ErrorEvent, int &ErrorCode);

	bool IsConnected() { return m_ClientSocket.IsActive(); }
};

void SplitIAddress(char* szIP, int maxlen, int* pPort, const char* szAddress);