#pragma once

#include "MThread.h"
#include "MSync.h"
#include <list>
#include <string>

#ifdef WIN32
#pragma comment(lib, "Wininet.lib")
#endif

bool MHTTP_Get(const char *szUrl, char *out, int nOutLen);

typedef bool(MHttpRecvCallback)(void* pCallbackContext, char* pRecvData);

#define HTTP_BUFSIZE 65536

class MHttpThread : public MThread
{
private:
protected:
	void FlushQuery();

	MSignalEvent			m_QueryEvent;
	MSignalEvent			m_KillEvent;
	MCriticalSection		m_csQueryLock;
	std::list<std::string>	m_QueryList;
	std::list<std::string>	m_TempQueryList;		// Temporary for Sync
	char					m_szRecvBuf[HTTP_BUFSIZE]{};
	bool					m_bActive{};

public:
	virtual void Run();
	virtual void Create();
	virtual void Destroy();
	void ClearQuery();
	void Query(const char* szQuery);
	void LockQuery() { m_csQueryLock.lock(); }
	void UnlockQuery() { m_csQueryLock.unlock(); }

	void*					m_pCallbackContext{};
	MHttpRecvCallback*		m_fnRecvCallback{};
};

class MHttp
{
private:
protected:
	MHttpThread		m_HttpThread;
	void ReplaceBlank(char* szOut, char* szSrc);
public:
	MHttp();
	virtual ~MHttp();
	
	bool Create();
	void Destroy();
	void Query(const char* szQuery);

	void SetRecvCallback(void* pCallbackContext, MHttpRecvCallback pCallback)
	{
		m_HttpThread.m_pCallbackContext = pCallbackContext;
		m_HttpThread.m_fnRecvCallback = pCallback;
	}
	void Clear();
};