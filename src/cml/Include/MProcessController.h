#pragma once

using HANDLE = void*;
using PROCESSENTRY32 = struct tagPROCESSENTRY32;

class MProcessController {
public:
	static bool FindProcessByName(const char* pszProcessName, PROCESSENTRY32* pOutPE32);
	static HANDLE OpenProcessHandleByFilePath(const char* pszFilePath);
	static HANDLE OpenProcessHandleByName(const char* pszFilePath);
	static bool StartProcess(const char* pszProcessPath, const bool bInheritHandles = true);
	static bool StopProcess(HANDLE hProcess);
};

int MGetCurrProcessMemory();