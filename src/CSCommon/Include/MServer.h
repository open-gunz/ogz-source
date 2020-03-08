#pragma once

#include "MCommandCommunicator.h"
#include "RealCPNet.h"
#include "MDebug.h"
#include <list>
#include "NetIO.h"

class MCommand;

class MServer : public MCommandCommunicator {
protected:
	NetIO Net;

	std::list<MCommObject*>			m_AcceptWaitQueue;
	MCriticalSection				m_csAcceptWaitQueue;

	void LockAcceptWaitQueue() { m_csAcceptWaitQueue.lock(); }
	void UnlockAcceptWaitQueue() { m_csAcceptWaitQueue.unlock(); }

	MUIDRefCache<MCommObject>	m_CommRefCache;
	MCriticalSection			m_csCommList;

	void LockCommList() { m_csCommList.lock(); }
	void UnlockCommList() { m_csCommList.unlock(); }

	MCommandList				m_SafeCmdQueue;
	MCriticalSection			m_csSafeCmdQueue;
	void LockSafeCmdQueue() { m_csSafeCmdQueue.lock(); }
	void UnlockSafeCmdQueue() { m_csSafeCmdQueue.unlock(); }

	virtual MUID UseUID() = 0;

	void AddCommObject(const MUID& uid, MCommObject* pCommObj);
	void RemoveCommObject(const MUID& uid);
	void InitCryptCommObject(MCommObject* pCommObj, unsigned int nTimeStamp);

	void PostSafeQueue(MCommand* pNew);

	void SendCommand(MCommand* pCommand);
	void ParsePacket(MCommObject* pCommObj, MPacketHeader* pPacket);

	virtual void OnPrepareRun();
	virtual void OnRun();
	virtual bool OnCommand(MCommand* pCommand);

	virtual void OnNetClear(const MUID& CommUID);
	virtual void OnNetPong(const MUID& CommUID, unsigned int nTimeStamp);

	bool SendMsgReplyConnect(MUID* pHostUID, MUID* pAllocUID, unsigned int nTimeStamp,
		MCommObject* pCommObj);
	bool SendMsgCommand(uintptr_t nClientKey, char* pBuf, int nSize,
		unsigned short nMsgHeaderID, MPacketCrypterKey* pCrypterKey);

	static void RCPCallback(void* pCallbackContext, NetIO::IOOperation Op,
		NetIO::ConnectionHandle Handle, const void* Data);	// Thread not safe

public:	// For Debugging
	char m_szName[128];
	void SetName(const char* pszName) { strcpy_safe(m_szName, pszName); }
	void DebugLog(char* pszLog) {
		char szLog[128];
		sprintf_safe(szLog, "[%s] %s\n", m_szName, pszLog);
		DMLog(szLog);
	}

public:
	MServer();
	~MServer();

	bool Create(int nPort, const bool bReuse = false );
	void Destroy();
	int GetCommObjCount();

	virtual int Connect(MCommObject* pCommObj);
	int ReplyConnect(MUID* pTargetUID, MUID* pAllocUID, unsigned int nTimeStamp, MCommObject* pCommObj);
	virtual int OnAccept(MCommObject* pCommObj);
	virtual void OnLocalLogin(MUID CommUID, MUID PlayerUID);
	virtual void Disconnect(MUID uid);
	virtual int OnDisconnect(const MUID& uid);	// Thread not safe

	virtual void Log(unsigned int nLogLevel, const char* szLog) = 0;

	void LogF(unsigned int Level, const char* Format, ...);
};
