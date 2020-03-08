#pragma once

#include "MCommandManager.h"
#include "MUID.h"
#include "MPacket.h"
#include "MPacketCrypter.h"
#include "SafeString.h"
#include "GlobalTypes.h"
#include "MInetUtil.h"

//#define _CMD_PROFILE

#ifdef _CMD_PROFILE
#include "MCommandProfiler.h"
#endif

class MCommandCommunicator;
class MCommandBuilder;

class MCommObject {
protected:
	MUID					m_uid;

	MCommandBuilder*		m_pCommandBuilder;
	MPacketCrypter			m_PacketCrypter;

	MCommandCommunicator*	m_pDirectConnection;
	uintptr_t				m_dwUserContext;

	char					m_szIP[128];
	int						m_nPort;
	u32						m_dwIP;
	bool					m_bAllowed;

public:
	MCommObject(MCommandCommunicator* pCommunicator);
	virtual ~MCommObject();

	auto& GetUID() const { return m_uid; }
	void SetUID(const MUID& uid) { m_uid = uid; }

	MCommandBuilder*	GetCommandBuilder()				{ return m_pCommandBuilder; }
	MPacketCrypter*		GetCrypter()					{ return &m_PacketCrypter; }

	MCommandCommunicator* GetDirectConnection()			{ return m_pDirectConnection; }
	void SetDirectConnection(MCommandCommunicator* pDC)	{ m_pDirectConnection = pDC; }
	uintptr_t GetUserContext() const					{ return m_dwUserContext; }
	void SetUserContext(uintptr_t dwContext)			{ m_dwUserContext = dwContext; }

	auto* GetIPString() const { return m_szIP; }
	u32 GetIP() const		{ return m_dwIP; }
	int GetPort() const		{ return m_nPort; }
	void SetAddress(const char* pszIP, int nPort) {
		strcpy_safe(m_szIP, pszIP);
		m_dwIP = GetIPv4Number(m_szIP);
		m_nPort = nPort;
	}
	void SetAllowed(bool bAllowed)	{ m_bAllowed = bAllowed; }
	bool IsAllowed() const			{ return m_bAllowed; }
};


class MPacketInfo {
public:
	MCommObject*		m_pCommObj;
	MPacketHeader*		m_pPacket;

	MPacketInfo(MCommObject* pCommObj, MPacketHeader* pPacket)
	{
		m_pCommObj = pCommObj;
		m_pPacket = pPacket;
	}
};

typedef std::list<MPacketInfo*>		MPacketInfoList;
typedef MPacketInfoList::iterator	MPacketInfoListItor;

class MCommandCommunicator{
public:
	MCommandCommunicator();
	virtual ~MCommandCommunicator();

	bool Create();
	void Destroy();

	virtual int Connect(MCommObject* pCommObj)=0;
	virtual int OnConnected(MUID* pTargetUID, MUID* pAllocUID,
		unsigned int nTimeStamp, MCommObject* pCommObj);
	virtual void Disconnect(MUID uid)=0;

	virtual bool Post(MCommand* pCommand);
	virtual bool Post(char* szErrMsg, int nErrMsgCount, const char* szCommand);

	virtual MCommand* GetCommandSafe();

	void Run();

	MCommandManager* GetCommandManager(void){
		return &m_CommandManager;
	}
	MCommand* CreateCommand(int nCmdID, const MUID& TargetUID);

	enum _LogLevel	{ LOG_DEBUG = 1, LOG_FILE = 2, LOG_PROG = 4, LOG_ALL = 7,  };

	virtual void Log(unsigned int nLogLevel, const char* szLog){}
	void LOG(unsigned int nLogLevel, const char *pFormat,...);

	MUID GetUID(void){ return m_This; }
	MCommand* MakeCmdFromSaneTunnelingBlob(const MUID& Sender, const MUID& Receiver,
		const void* pBlob, size_t Size);

#ifdef _CMD_PROFILE
	MCommandProfiler		m_CommandProfiler;
#endif

	MCommand* BlobToCommand(const void* Data, size_t Size);
	MCommand* BlobToCommand(MCmdParamBlob * Blob);

protected:
	virtual void SendCommand(MCommand* pCommand) = 0;
	virtual void ReceiveCommand(MCommand* pCommand);

	virtual void OnRegisterCommand(MCommandManager* pCommandManager);
	virtual bool OnCommand(MCommand* pCommand);
	virtual void OnPrepareRun();
	virtual void OnPrepareCommand(MCommand* pCommand);
	virtual void OnRun();

	void SetDefaultReceiver(MUID Receiver);

	MCommandManager	m_CommandManager;

	MUID			m_This;
	MUID			m_DefaultReceiver;
};

MCmdParamBlob* CommandToBlob(MCommand& Command);
int CalcPacketSize(MCommand* pCmd);

bool MakeSaneTunnelingCommandBlob(MCommand* pWrappingCmd, MCommand* pSrcCmd);
