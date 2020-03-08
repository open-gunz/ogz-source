#include "stdafx.h"
#ifdef MFC
#include "MDebug.h"
#include "Shlwapi.h"
#include "MCrashDump.h"
#include "MMatchStatus.h"
#include "MMatchServer.h"

#ifndef _DEBUG
#define SUPPORT_EXCEPTIONHANDLING
#endif

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


static bool GetRecommandLogFileName(char* pszBuf)
{
	if (PathIsDirectory("Log") == FALSE)
		CreateDirectory("Log", NULL);

	time_t		tClock;
	struct tm*	ptmTime;

	time(&tClock);
	ptmTime = localtime(&tClock);

	char szFileName[_MAX_DIR];

	int nFooter = 1;
	while(TRUE) {
		sprintf(szFileName, "Log/Locator_%02d-%02d-%02d-%d.txt", 
			ptmTime->tm_year+1900, ptmTime->tm_mon+1, ptmTime->tm_mday, nFooter);

		if (PathFileExists(szFileName) == FALSE)
			break;

		nFooter++;
		if (nFooter > 100) return false;
	}
	strcpy(pszBuf, szFileName);
	return true;
}


#ifdef LOCATOR_STANDALONE
int AFXAPI AfxWinMain(HINSTANCE hInstance, 
					  HINSTANCE hPrevInstance,
					  LPTSTR lpCmdLine, 
					  int nCmdShow)
{
	char szLogFileName[_MAX_DIR];
	if (GetRecommandLogFileName(szLogFileName) == false) 
		return FALSE;

	InitLog(MLOGSTYLE_DEBUGSTRING|MLOGSTYLE_FILE, szLogFileName);

	// Wrap WinMain in a structured exception handler (different from C++
	// exception handling) in order to make sure that all access violations
	// and other exceptions are displayed - regardless of when they happen.
	// This should be done for each thread, if at all possible, so that exceptions
	// will be reliably caught, even inside the debugger.
#ifdef SUPPORT_EXCEPTIONHANDLING
	char szDumpFileName[_MAX_DIR]; 
	strcpy(szDumpFileName, szLogFileName);
	strcat(szDumpFileName, ".dmp");
	__try {
#endif
		// The code inside the __try block is the MFC version of AfxWinMain(),
		// copied verbatim from the MFC source code.
		ASSERT(hPrevInstance == NULL);

		int nReturnCode = -1;
		CWinApp* pApp = AfxGetApp();

		// AFX internal initialization
		if (!AfxWinInit(hInstance, hPrevInstance, lpCmdLine, nCmdShow))
			goto InitFailure;

		// App global initializations (rare)
		ASSERT_VALID(pApp);
		if (!pApp->InitApplication())
			goto InitFailure;
		ASSERT_VALID(pApp);

		// Perform specific initializations
		if (!pApp->InitInstance())
		{
			if (pApp->m_pMainWnd != NULL)
			{
				TRACE(_T("Warning: Destroying non-NULL m_pMainWnd\n"));
				pApp->m_pMainWnd->DestroyWindow();
			}
			nReturnCode = pApp->ExitInstance();
			goto InitFailure;
		}
		ASSERT_VALID(pApp);

		nReturnCode = pApp->Run();
		ASSERT_VALID(pApp);

InitFailure:
#ifdef _DEBUG
		// Check for missing AfxLockTempMap calls
		if (AfxGetModuleThreadState()->m_nTempMapLock != 0)
		{
			TRACE(_T("Warning: Temp map lock count non-zero (%ld).\n"),
				AfxGetModuleThreadState()->m_nTempMapLock);
		}
		AfxLockTempMaps();
		AfxUnlockTempMaps(-1);
#endif

		AfxWinTerm();
		return nReturnCode;

#ifdef SUPPORT_EXCEPTIONHANDLING
	}
//	__except(MFilterException(GetExceptionInformation())){
	__except(CrashExceptionDump(GetExceptionInformation(), szDumpFileName)){
		
//		char szFileName[_MAX_DIR];
//		GetModuleFileName(NULL, szFileName, _MAX_DIR);
//		WinExec(szFileName, SW_SHOW);	// Launch again
		//MMatchServer::GetInstance()->CheckMemoryTest();
		//MGetServerStatusSingleton()->Dump();
	}
#endif
	return 0;
}
#else
void* silence_LNK4221_warning;
#endif
#endif