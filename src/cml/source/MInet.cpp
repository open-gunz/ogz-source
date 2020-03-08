#include "stdafx.h"
#include "MInet.h"
#include <stdio.h> 
#include "MDebug.h"
#include <mutex>

#ifdef WIN32
#include <windows.h> 
#include <wininet.h> 
#endif

bool MHTTP_Get(const char *szUrl, char *out, int nOutLen)
{
#ifdef WIN32
    HINTERNET h, h2; 
    unsigned long len, i = 0; 

    h = InternetOpen("Microsoft Internet Explorer", INTERNET_OPEN_TYPE_DIRECT, NULL, NULL, NULL); 
    h2 = InternetOpenUrl(h, szUrl, NULL, 0, INTERNET_FLAG_DONT_CACHE|INTERNET_FLAG_NO_CACHE_WRITE, 0); 

    if (h2) 
	{
        do 
		{
			InternetReadFile(h2, out+i, nOutLen-1-i, &len); 
            i += len; 
        } while(len > 0 && i<len-1); 
        out[i] = '\0'; 

        InternetCloseHandle(h2); 
		return true;
    } 

    InternetCloseHandle(h); 
	return false;
#else
	return false;
#endif
}

void MHttpThread::Run()
{
	m_bActive = true;

	MSignalEvent* EventArray[]{
		&m_QueryEvent,
		&m_KillEvent,
	};

	bool bEnd = false;
	while (!bEnd)
	{
		auto WaitResult = WaitForMultipleEvents(EventArray, MSync::Infinite);
		if (WaitResult == MSync::WaitFailed ||
			WaitResult == MSync::WaitTimeout) {
			continue;
		}

		switch (WaitResult)
		{
		case 0: //  Query Event
			FlushQuery();
			m_QueryEvent.ResetEvent();
			break;

		case 1: // Kill Event
			bEnd = true;
			m_KillEvent.ResetEvent();
			break;

		default:
			MLog("MHttpThread::Run -- Exceptional case %d\n", WaitResult);
			bEnd = true;
			break;
		}	// switch

	}	// while

	m_bActive = false;
}

void MHttpThread::Create()
{
	MThread::Create();
}

void MHttpThread::Destroy()
{
	m_QueryList.clear();
	m_KillEvent.SetEvent(); 

	MThread::Destroy();
}

void MHttpThread::Query(const char* szQuery)
{
	if (!m_bActive) return;

	{
		std::lock_guard<MCriticalSection> lock{ m_csQueryLock };
		m_TempQueryList.push_back(szQuery);
	}

	m_QueryEvent.SetEvent();
}

void MHttpThread::FlushQuery()
{
	if (!m_bActive) return;

	{
		std::lock_guard<MCriticalSection> lock{ m_csQueryLock };
		while (!m_TempQueryList.empty())
		{
			auto itor = m_TempQueryList.begin();
			m_QueryList.push_back(*itor);
			m_TempQueryList.erase(itor);
		}
	}

	while (!m_QueryList.empty())
	{
		auto itor = m_QueryList.begin();
		const char* szQuery = (*itor).c_str();

		MHTTP_Get(szQuery, m_szRecvBuf, HTTP_BUFSIZE);

		if (m_fnRecvCallback)
		{
			m_fnRecvCallback(m_pCallbackContext, m_szRecvBuf);
		}

		m_QueryList.erase(m_QueryList.begin());
	}
}

void MHttpThread::ClearQuery()
{
	std::lock_guard<MCriticalSection> lock{ m_csQueryLock };
	m_TempQueryList.clear();
	m_QueryList.clear();
}

MHttp::MHttp() = default;
MHttp::~MHttp() { Destroy(); }

bool MHttp::Create()
{
	m_HttpThread.Create();

	return true;
}

void MHttp::Destroy()
{
	m_HttpThread.Destroy();
}

void MHttp::Query(const char* szQuery)
{
	m_HttpThread.Query(szQuery);
}

void MHttp::Clear()
{
	m_HttpThread.m_pCallbackContext = NULL;
	m_HttpThread.m_fnRecvCallback = NULL;

	m_HttpThread.ClearQuery();
}

void MHttp::ReplaceBlank(char* szOut, char* szSrc)
{
	int nLen = (int)strlen(szSrc);
	int nOutIdx = 0;
	for(int i = 0; i < nLen; i++)
	{
		if (szSrc[i] == ' ')
		{
			szOut[nOutIdx++] = '%';
			szOut[nOutIdx++] = '2';
			szOut[nOutIdx++] = '0';
		}
		else
		{
			szOut[nOutIdx++] = szSrc[i];
		}
	}
	szOut[nOutIdx] = '\0';
}