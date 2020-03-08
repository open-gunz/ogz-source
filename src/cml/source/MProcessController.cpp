#include "stdafx.h"
#include "MProcessController.h"
#ifdef _WIN32
#include "shlwapi.h"
#endif

#ifdef _MPROCESS_CONTROLLER

#include "psapi.h"


#pragma comment(lib, "Psapi.lib")
#pragma comment(lib, "shlwapi.lib")

#define TIMEOUT_PROCESS_STOP	5000


bool MProcessController::FindProcessByName(const char* pszProcessName, PROCESSENTRY32* pOutPE32)
{
	BOOL          bRet			= FALSE; 
	BOOL          bFound		= FALSE; 
	HANDLE        hProcessSnap	= NULL; 

	PROCESSENTRY32 pe32			= {0}; 
	pe32.dwSize = sizeof(MODULEENTRY32); 

	hProcessSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0); 
	if (hProcessSnap == INVALID_HANDLE_VALUE) 
		return false; 

	bool bResult = false;
	if (Process32First(hProcessSnap, &pe32)) 
	{ 
		do { 
			if (StrStrI(pe32.szExeFile, pszProcessName) != 0) {
				CopyMemory(pOutPE32, &pe32, sizeof(PROCESSENTRY32));
				bResult = true;
				break;
			}
		} while (Process32Next(hProcessSnap, &pe32)); 
	}
	CloseHandle (hProcessSnap);

	return bResult;
}

HANDLE MProcessController::OpenProcessHandleByFilePath(const char* pszFilePath)
{
	DWORD ProcessIDList[1024], cbNeeded;

	if ( !EnumProcesses(ProcessIDList, sizeof(ProcessIDList), &cbNeeded) )
		return NULL;

	// Calculate how many process identifiers were returned.
	int nProcessCount = cbNeeded / sizeof(DWORD);

	for (int i=0; i<nProcessCount; i++) {
		DWORD nProcessID = ProcessIDList[i];

		char szProcessName[MAX_PATH] = "unknown";
		char szProcessPath[MAX_PATH] = "unknown";

		// Get a handle to the process.
		//HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, nProcessID);
		//HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ| SYNCHRONIZE | PROCESS_TERMINATE, TRUE, nProcessID);
		HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, TRUE, nProcessID);
		if (NULL != hProcess)	// Get the process name.
		{
			HMODULE hMod;
			DWORD cbNeeded;

			if (EnumProcessModules(hProcess, &hMod, sizeof(hMod), &cbNeeded))
			{
				GetModuleBaseName( hProcess, hMod, szProcessName, sizeof(szProcessName) );
				GetModuleFileNameEx( hProcess, hMod, szProcessPath, sizeof(szProcessPath) );
				//TRACE("ProcessName=%s (Process ID: %u), ProcessPath=%s \n",szProcessName,nProcessID,szProcessPath);
				if (_stricmp(szProcessPath, pszFilePath) == 0)
					return hProcess;
			}

			CloseHandle(hProcess);
		}
	} // For
	return NULL;
}

bool MProcessController::StartProcess(const char* pszProcessPath, const BOOL bInheritHandles)
{
	BOOL bResult = FALSE;
	char szRunDir[MAX_PATH];
	strcpy_safe(szRunDir, pszProcessPath);
	PathRemoveFileSpec(szRunDir);

	STARTUPINFO stInfo;
	ZeroMemory( &stInfo, sizeof(stInfo) );

	PROCESS_INFORMATION prInfo;
	stInfo.cb = sizeof(stInfo);
	stInfo.dwFlags = STARTF_USESHOWWINDOW;
	stInfo.wShowWindow = SW_SHOW;

	bResult = CreateProcess(NULL, (LPSTR)(LPCSTR)pszProcessPath, 
							NULL, NULL, 
							bInheritHandles, CREATE_NEW_CONSOLE | NORMAL_PRIORITY_CLASS,
							NULL,
							(LPCSTR)szRunDir,
							&stInfo, &prInfo);

	CloseHandle(prInfo.hThread); 
	CloseHandle(prInfo.hProcess);

	if (!bResult) return false;
	return true;
}

bool MProcessController::StopProcess(HANDLE hProcess)
{
	BOOL bResult = TerminateProcess(hProcess, 0);
	WaitForSingleObject(hProcess, TIMEOUT_PROCESS_STOP);
	CloseHandle(hProcess);

	if (bResult == FALSE) {
		DWORD dwError = GetLastError();
	}
	if (bResult)
		return true;
	else
		return false;
}

int MGetCurrProcessMemory()
{
	PROCESS_MEMORY_COUNTERS pmCount;
	HANDLE pid = GetCurrentProcess();
	GetProcessMemoryInfo(pid,&pmCount,sizeof(pmCount));
	return (int)pmCount.WorkingSetSize;
}

#endif