#pragma once

#ifdef _MSC_VER

#include <type_traits>
#include "MFile.h"

/*
200 OK 
201 Created 
202 Accepted 
204 No Content 
301 Moved Permanently 
302 Moved Temporarily 
304 Not Modified 
400 Bad Request 
401 Unauthorized 
403 Forbidden 
404 Not Found 
500 Internal Server Error 
501 Not Implemented 
502 Bad Gateway 
503 Service Unavailable 
*/

class MAsyncHttp;
class MAsyncHttpContext {
public:
	enum MAHC_TYPE {
		MAHC_TYPE_UNKNOWN,
		MAHC_TYPE_CONNECT,
		MAHC_TYPE_REQUEST
	};

protected:
	MAHC_TYPE		m_ContextType;
	MAsyncHttp*		m_pAsyncHttp;
public:
	MAsyncHttpContext()
	{
		m_ContextType = MAHC_TYPE_UNKNOWN;
		m_pAsyncHttp = NULL;
	}
	MAsyncHttpContext(MAHC_TYPE nType, MAsyncHttp* pAsyncHttp)
	{
		m_ContextType = nType;
		m_pAsyncHttp = pAsyncHttp;
	}
	MAHC_TYPE GetContextType()	{ return m_ContextType; }
	MAsyncHttp* GetAsyncHttp()	{ return m_pAsyncHttp; }
};

using HANDLE = void*;
using HINTERNET = HANDLE;
using WIN_DWORD_PTR = std::conditional_t<sizeof(void*) == 4, unsigned long, unsigned long long>;

class MAsyncHttp {
protected:
	HINTERNET	m_hInstance;
	HINTERNET	m_hConnect;
	HINTERNET	m_hRequest;

	HANDLE		m_hConnectedEvent;
	HANDLE		m_hRequestOpenedEvent;
	HANDLE		m_hRequestCompleteEvent;

	char		m_szBasePath[MFile::MaxPath];

	int			m_nLastHttpStatusCode;
	bool		m_bTransferFinished;
	bool		m_bVerbose;

	MAsyncHttpContext	m_ConnectContext;
	MAsyncHttpContext	m_RequestContext;

	int GetLastHttpStatusCode()		{ return m_nLastHttpStatusCode; }
	void SetLastHttpStatusCode(int nCode)	{ m_nLastHttpStatusCode = nCode; }
	bool IsTransferFinished()		{ return m_bTransferFinished; }
	void SetTransferFinished(bool bVal)	{ m_bTransferFinished = bVal; }
	bool IsVerbose()				{ return m_bVerbose; }
	void SetVerbose(bool bVal)		{ m_bVerbose = bVal; }

protected:
	static void __stdcall StatusCallback(HINTERNET hInternet,
		WIN_DWORD_PTR dwContext, unsigned long dwInternetStatus,
		void* lpStatusInfo, unsigned long dwStatusInfoLen);
public:
	MAsyncHttp();
	virtual ~MAsyncHttp();

	const char* GetBasePath()				{ return m_szBasePath; }
	void SetBasePath(const char* pszPath)	{ strcpy_safe(m_szBasePath, pszPath); }

	bool Get(const char* pszURL);
};

#endif
