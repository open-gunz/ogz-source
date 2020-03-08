#include "stdafx.h"

#ifdef WIN32

#include "MAsyncHttp.h"
#include <crtdbg.h>
#include <shlwapi.h>
#include <WinInet.h>

#pragma comment(lib, "shlwapi.lib")
#pragma comment(lib, "Wininet.lib")

#define MSEC_SENDCOMPLETE_TIMEOUT	5000

MAsyncHttp::MAsyncHttp() 
{
	m_hInstance = NULL;
	m_hConnect = NULL;
	m_hRequest = NULL;

	m_hConnectedEvent = CreateEventA(NULL, FALSE, FALSE, NULL);
	m_hRequestOpenedEvent = CreateEventA(NULL, FALSE, FALSE, NULL);
	m_hRequestCompleteEvent = CreateEventA(NULL, FALSE, FALSE, NULL);

	m_ConnectContext = MAsyncHttpContext(MAsyncHttpContext::MAHC_TYPE_CONNECT, this);
	m_RequestContext = MAsyncHttpContext(MAsyncHttpContext::MAHC_TYPE_REQUEST, this);

	m_szBasePath[0]=NULL;

	SetTransferFinished(false);
	SetVerbose(false);
}

MAsyncHttp::~MAsyncHttp()
{
	OutputDebugString("Closing AsyncHttp \n");

	InternetSetStatusCallback(m_hInstance, (INTERNET_STATUS_CALLBACK)NULL);	// Release Callback

	CloseHandle(m_hRequestCompleteEvent);	m_hRequestCompleteEvent = NULL;
	CloseHandle(m_hRequestOpenedEvent);		m_hRequestOpenedEvent = NULL;
	CloseHandle(m_hConnectedEvent);			m_hConnectedEvent = NULL;

	InternetCloseHandle(m_hRequest);		m_hRequest = NULL;
	InternetCloseHandle(m_hConnect);		m_hConnect = NULL;
	InternetCloseHandle(m_hInstance);		m_hInstance = NULL;
}

bool MAsyncHttp::Get(const char* pszURL)
{
	char szLog[1024]="";

	// Initialize 
	SetTransferFinished(false);

	//// Parse URL //////////////////
	#define DOMAIN_LEN	256
	char szDomain[DOMAIN_LEN] = "";

	#define URLPATH_LEN	256
	char szURLPath[URLPATH_LEN] = "";

	URL_COMPONENTS uc;
	ZeroMemory(&uc, sizeof uc);

	uc.dwStructSize = sizeof uc;
	uc.lpszHostName = szDomain;
	uc.dwHostNameLength = DOMAIN_LEN;
	uc.lpszUrlPath = szURLPath;
	uc.dwUrlPathLength = URLPATH_LEN;

	if (!InternetCrackUrl(pszURL, lstrlen(pszURL), ICU_DECODE, &uc)) {
		// GetLastError()
		return false;
	}
	/////////////////////////////////

	m_hInstance = InternetOpen("MAsyncHttp", 
							INTERNET_OPEN_TYPE_DIRECT,
							NULL,
							NULL,
							INTERNET_FLAG_ASYNC); // ASYNC Flag

	if (m_hInstance == NULL)
	{
		// GetLastError()
		return false;
	}

	// Setup callback function
	if (InternetSetStatusCallback(m_hInstance, StatusCallback) == INTERNET_INVALID_STATUS_CALLBACK)
	{
		// GetLastError()
		return false;
	}

	// First call that will actually complete asynchronously even
	// though there is no network traffic
	m_hConnect = InternetConnect(m_hInstance, 
								szDomain, 
								INTERNET_DEFAULT_HTTP_PORT,
								NULL,
								NULL,
								INTERNET_SERVICE_HTTP,
								0,
								(DWORD_PTR)&m_ConnectContext); // Connection handle's Context
	if (m_hConnect == NULL)
	{
		if (GetLastError() != ERROR_IO_PENDING)
		{
			//  GetLastError()
			return false;
		}
		// Wait until we get the connection handle
		WaitForSingleObject(m_hConnectedEvent, INFINITE);
	}

	// Open the request
	m_hRequest = HttpOpenRequest(m_hConnect, 
								"GET", 
								szURLPath,
								NULL,
								NULL,
								NULL,
								INTERNET_FLAG_RELOAD | INTERNET_FLAG_NO_CACHE_WRITE,
								(DWORD_PTR)&m_RequestContext);  // Request handle's context 
	if (m_hRequest == NULL)
	{
		if (GetLastError() != ERROR_IO_PENDING)
		{
			//  GetLastError();
			return false;
		}
		// Wait until we get the request handle
		WaitForSingleObject(m_hRequestOpenedEvent, INFINITE);
	}

	if (!HttpSendRequest(m_hRequest, 
							NULL, 
							0, 
							NULL,
							0))
	{
		if (GetLastError() != ERROR_IO_PENDING)
		{
			// GetLastError();
			return false;
		}
	}

	// Raon Debug - Deadlock on Alt-F4
	if (WAIT_TIMEOUT == WaitForSingleObject(m_hRequestCompleteEvent, MSEC_SENDCOMPLETE_TIMEOUT)) {
		OutputDebugString("SENDCOMPLETE TIMEOUT \n");
		return false;
	}

	//// HTTP_QUERY_STATUS_CODE ////////////////////////////////////////
	DWORD dwStatusCode;
	DWORD dwStatusLength = sizeof(dwStatusCode);
	if (!HttpQueryInfo(m_hRequest, HTTP_QUERY_STATUS_CODE | HTTP_QUERY_FLAG_NUMBER,
						&dwStatusCode, &dwStatusLength, NULL))
	{
		// GetLastError()
		SetLastHttpStatusCode(503); // Service Unavailable
		return false;
	} else {
		SetLastHttpStatusCode(dwStatusCode);
		if (dwStatusCode != 200)
			return false;
	}
	//// HTTP_QUERY_CONTENT_LENGTH ////////////////////////////////////////
	DWORD dwFileSize;
	DWORD dwSizeLength = sizeof(dwFileSize);
	if (!HttpQueryInfo(m_hRequest, HTTP_QUERY_CONTENT_LENGTH | HTTP_QUERY_FLAG_NUMBER,
						&dwFileSize, &dwSizeLength, NULL))
	{
		// GetLastError()
		return false;
	}
	//// HTTP_QUERY_DATE ////////////////////////////////////////	제대로 안넘어옴
	FILETIME ftDate; ZeroMemory(&ftDate, sizeof(ftDate));
	SYSTEMTIME tmDate;
	DWORD dwDateLength = sizeof(tmDate);
	if (!HttpQueryInfo(m_hRequest, HTTP_QUERY_LAST_MODIFIED | HTTP_QUERY_FLAG_SYSTEMTIME,
						&tmDate, &dwDateLength, NULL))
	{
		// GetLastError()
		return false;
	} else {		
		if (!SystemTimeToFileTime(&tmDate, &ftDate))  // converts to file time format
			OutputDebugString("SystemTimeToFileTime Failed \n");
	}
	////////////////////////////////////////////////////////////////////

	// Get FileName From URL
	char szFileName[_MAX_DIR];
	strcpy_safe(szFileName, szURLPath);
	PathStripPath(szFileName);

	char szFullPath[_MAX_DIR];
	strcpy_safe(szFullPath, GetBasePath());
	strcat_safe(szFullPath, "/");
	strcat_safe(szFullPath, szFileName);

	sprintf_safe(szLog, "MAsyncHttp> Download Begin : %s \n", szFileName);
	OutputDebugString(szLog);

	// Create File
	HANDLE hFile = CreateFileA(TEXT(szFullPath),// Open Two.txt.
						GENERIC_WRITE,			// Open for writing
						0,						// Do not share
						NULL,					// No security
						OPEN_ALWAYS,			// Open or create
						FILE_ATTRIBUTE_NORMAL,	// Normal file
						NULL);					// No template file
	if (hFile == INVALID_HANDLE_VALUE)
	{
		OutputDebugString("MAsyncHttp::Get> CreateFile FAILED \n");
		return false;
	}

	// Change FileTime
	if (!SetFileTime(hFile, (LPFILETIME)NULL, (LPFILETIME)NULL, &ftDate)) {
		// GetLastError()
		OutputDebugString("Filetime change failed \n");
	}

	//------------------- Read the response -------------------
	char pReadBuff[256];
	do
	{
		INTERNET_BUFFERS InetBuff;
		FillMemory(&InetBuff, sizeof(InetBuff), 0);
		InetBuff.dwStructSize = sizeof(InetBuff);
		InetBuff.lpvBuffer = pReadBuff;
		InetBuff.dwBufferLength = sizeof(pReadBuff) - 1;
	    
		if (!InternetReadFileEx(m_hRequest, &InetBuff, 0, 2))
		{
			if (GetLastError() == ERROR_IO_PENDING)
			{
				// Waiting for InternetReadFileEx to complete" << endl;
				WaitForSingleObject(m_hRequestCompleteEvent, INFINITE);
			} else {
				//  GetLastError()
				return false;
			}
		}

//		pReadBuff[InetBuff.dwBufferLength] = 0;
//		OutputDebugString(pReadBuff);

		DWORD dwWritten = 0;
		WriteFile (hFile, InetBuff.lpvBuffer, InetBuff.dwBufferLength, &dwWritten, NULL);

		if (InetBuff.dwBufferLength == 0) 
			SetTransferFinished(true);
	} while (IsTransferFinished() == false);

	CloseHandle(hFile);

	sprintf_safe(szLog, "MAsyncHttp> Download End : %s \n", szFileName);
	OutputDebugString(szLog);

	//------------------- Request Complete ----------------
	return true;
}

void CALLBACK MAsyncHttp::StatusCallback(HINTERNET hInternet,
	DWORD_PTR dwContext, DWORD dwInternetStatus,
	LPVOID pStatusInfo, DWORD dwStatusInfoLen)
{
	MAsyncHttpContext* pMAHContext = (MAsyncHttpContext*)dwContext;
	MAsyncHttp* pAsyncHttp = pMAHContext->GetAsyncHttp();
	if (pAsyncHttp == NULL) {
		_ASSERT("pAsyncHttp == NULL");
		return;
	}

	switch(pMAHContext->GetContextType()) {
	case MAsyncHttpContext::MAHC_TYPE_CONNECT: // Connection handle
		if (dwInternetStatus == INTERNET_STATUS_HANDLE_CREATED)
		{
			INTERNET_ASYNC_RESULT *pRes = (INTERNET_ASYNC_RESULT *)pStatusInfo;
			pAsyncHttp->m_hConnect = (HINTERNET)pRes->dwResult;
			if (pAsyncHttp->IsVerbose())
			{
				OutputDebugString("MAsyncHttp::StatusCallback> Connect handle created \n");
			}
			SetEvent(pAsyncHttp->m_hConnectedEvent);
		}
		break;
	case MAsyncHttpContext::MAHC_TYPE_REQUEST: // Request handle
		{
			switch(dwInternetStatus)
			{
			case INTERNET_STATUS_HANDLE_CREATED:
				{
					INTERNET_ASYNC_RESULT *pRes = (INTERNET_ASYNC_RESULT *)pStatusInfo;
					pAsyncHttp->m_hRequest = (HINTERNET)pRes->dwResult;
					if (pAsyncHttp->IsVerbose())
					{
						OutputDebugString("MAsyncHttp::StatusCallback> Request handle created \n");
					}
					SetEvent(pAsyncHttp->m_hRequestOpenedEvent);
				}
				break;
			case INTERNET_STATUS_REQUEST_SENT:
				{
					DWORD* pBytesSent = (DWORD*)pStatusInfo;
					if (pAsyncHttp->IsVerbose())
					{
						OutputDebugString("MAsyncHttp::StatusCallback> Request Sent \n");
					}
				}
				break;
			case INTERNET_STATUS_REQUEST_COMPLETE:
				{
					INTERNET_ASYNC_RESULT *pAsyncRes = (INTERNET_ASYNC_RESULT *)pStatusInfo;
					if (pAsyncHttp->IsVerbose())
					{
						char szLog[256];
						sprintf_safe(szLog, "MAsyncHttp::StatusCallback> RequestComplete(Result=%u, Error=%u) \n", 
								(u32)pAsyncRes->dwResult, pAsyncRes->dwError);
						OutputDebugString(szLog);
					}
					SetEvent(pAsyncHttp->m_hRequestCompleteEvent);
				}
				break;
			case INTERNET_STATUS_RECEIVING_RESPONSE:
				if (pAsyncHttp->IsVerbose())
				{
					OutputDebugString("MAsyncHttp::StatusCallback> Receiving Response \n");
				}
				break;
			case INTERNET_STATUS_RESPONSE_RECEIVED:
				{
					DWORD *dwBytesReceived = (DWORD*)pStatusInfo;
					if (*dwBytesReceived == 0)
						pAsyncHttp->SetTransferFinished(true);
					if (pAsyncHttp->IsVerbose())
					{
						OutputDebugString("MAsyncHttp::StatusCallback> ResponseReceived \n");
					}
				}
				break;
			}; // switch(dwInternetStatus)
		} // case 2
		break;
	default:
		_ASSERT("Unknown MASYNCHTTP_CONTEXT_TYPE");
		break;
	}; // switch(dwContext)
}

#endif