#include "stdafx.h"
#ifdef _WIN32
#include "MPdb.h"
#include <stdio.h>
#include <Windows.h>
#pragma warning(push)
#pragma warning(disable: 4091)
#include <imagehlp.h>
#pragma warning(pop)
#include <tlhelp32.h>

typedef DWORD	(WINAPI *fn_sym_SetOption)				( DWORD dwSymOptions);
typedef BOOL	(WINAPI *fn_sym_GetLineFromAddr)		( HANDLE hProcess, DWORD dwAddr, PDWORD pDwDisplacement, PIMAGEHLP_LINE pImageLine);
typedef BOOL	(WINAPI *fn_sym_GetSymFromAddr)			( HANDLE hProcess, DWORD dwAddr, PDWORD pDwDisplacement, PIMAGEHLP_SYMBOL pSymbol);
typedef BOOL	(WINAPI *fn_sym_initialize)				( HANDLE hProcess, LPSTR pUserSearchPath, BOOL fInvadeProcess);
typedef DWORD	(WINAPI *fn_sym_LoadModule)				( HANDLE hProcess, HANDLE hFile, LPSTR pImageName, LPSTR pModuleName, DWORD dwBaseOfDll, DWORD dwSizeOfDll);
typedef BOOL	(WINAPI *fn_sym_GetModuleInfo)			( HANDLE hProcess, DWORD dwAddr, PIMAGEHLP_MODULE pModuleInfo);
typedef BOOL	(WINAPI *fn_sym_EnumerateLoadModules)	( HANDLE hProcess, PENUMLOADED_MODULES_CALLBACK pEnumLoadedModulesCallback, PVOID pUserContext);
typedef BOOL	(WINAPI *fn_sym_Cleanup)				( HANDLE hProcess);
typedef BOOL	(WINAPI *fn_sym_StackWalk)				( DWORD dwMachineType, HANDLE hProcess, HANDLE hThread, LPSTACKFRAME pStk, PVOID pContextRecord, PREAD_PROCESS_MEMORY_ROUTINE pReadMemoryRoutine, PFUNCTION_TABLE_ACCESS_ROUTINE pFunctionTableAccessRoutine, PGET_MODULE_BASE_ROUTINE pGetModuleBaseRoutine, PTRANSLATE_ADDRESS_ROUTINE pTranslateAddress);

fn_sym_SetOption				g_pfnSymSetOptions;
fn_sym_initialize				g_pfnSymInitialize;
fn_sym_Cleanup					g_pfnSymCleanup;
fn_sym_GetSymFromAddr			g_pfnSymGetSymFromAddr;
fn_sym_GetLineFromAddr			g_pfnSymGetLineFromAddr;
fn_sym_EnumerateLoadModules		g_pfnEnumerateLoadedModules;
fn_sym_LoadModule				g_pfnSymLoadModule;
fn_sym_GetModuleInfo			g_pfnSymGetModuleInfo;
fn_sym_StackWalk				g_pfnStackWalk;

PFUNCTION_TABLE_ACCESS_ROUTINE	g_pfnFunctionTableAccessRoutine;
PGET_MODULE_BASE_ROUTINE		g_pfnGetModuleBaseRoutine;

#include "MDebug.h"

namespace MPdb
{

BOOL CALLBACK EnumLoadedModulesCallback(LPSTR pModuleName, ULONG ulModuleBase, ULONG ulModuleSize, PVOID pUserContext)
{
	if (!g_pfnSymLoadModule((HANDLE)pUserContext, 0, pModuleName, 0, ulModuleBase, ulModuleSize))
	{
		//		::MessageBox(NULL,"SymLoadModule failed","error",MB_OK);
		mlog("SymLoadModule failed %d ( module = %s ) \n", GetLastError(), pModuleName);
		return false;
	}
	return TRUE;
}

}

// Disable warning about GetVersionEx deprecation
#pragma warning(disable: 4996)
void LoadModuleSymbols(DWORD dwProcessId, HANDLE hProcess)
{
	OSVERSIONINFO   osver;
	HINSTANCE       hInstLib;
	HANDLE          hSnapShot;
	MODULEENTRY32   module;
	BOOL            fFlag;

	HANDLE (WINAPI *lpfCreateToolhelp32Snapshot)(DWORD,DWORD);
	BOOL (WINAPI *lpfModule32First)(HANDLE,LPMODULEENTRY32);
	BOOL (WINAPI *lpfModule32Next)(HANDLE,LPMODULEENTRY32);

	osver.dwOSVersionInfoSize = sizeof(osver);
	if (!GetVersionEx(&osver))
	{
//		::MessageBox(NULL,"GetVersionEx failed","error",MB_OK);
		return;
	}
    
	if (osver.dwPlatformId == VER_PLATFORM_WIN32_NT)
	{
//		mlog("LoadModuleSymbols,VER_PLATFORM_WIN32_NT\n");
		if (!g_pfnEnumerateLoadedModules(hProcess, (PENUMLOADED_MODULES_CALLBACK)MPdb::EnumLoadedModulesCallback, (PVOID)hProcess))
		{
//			::MessageBox(NULL,"EnumerateLoadedModules failed","error",MB_OK);
//			mlog("LoadModuleSymbols,EnumerateLoadedModules failed\n");
		}
		return;
	}

	if (osver.dwPlatformId == VER_PLATFORM_WIN32_WINDOWS)
	{
//		mlog("LoadModuleSymbols,VER_PLATFORM_WIN32_WINDOWS\n");
		hInstLib = LoadLibraryA("Kernel32.DLL");

		if (hInstLib == NULL)          return;

		lpfCreateToolhelp32Snapshot=(HANDLE(WINAPI *)(DWORD,DWORD)) GetProcAddress(hInstLib, "CreateToolhelp32Snapshot");

		lpfModule32First=(BOOL(WINAPI *)(HANDLE,LPMODULEENTRY32)) GetProcAddress(hInstLib, "Module32First");

		lpfModule32Next=(BOOL(WINAPI *)(HANDLE,LPMODULEENTRY32)) GetProcAddress(hInstLib, "Module32Next");

		if (lpfModule32Next == NULL || lpfModule32First == NULL || lpfCreateToolhelp32Snapshot == NULL)
		{
//			mlog("LoadModuleSymbols,NULL\n");
			FreeLibrary(hInstLib);
			return;
		}

		hSnapShot = lpfCreateToolhelp32Snapshot(TH32CS_SNAPMODULE, dwProcessId);

		if (hSnapShot == INVALID_HANDLE_VALUE)
		{
//			mlog("LoadModuleSymbols,INVALID_HANDLE_VALUE\n");
			FreeLibrary(hInstLib);
			return;
		}

		module.dwSize = sizeof(module) ;
		fFlag = lpfModule32First(hSnapShot, &module);

		while (fFlag)
		{
			// windows98 에서 d3d9.dll 심볼을 로드하면 exe파일 심볼을 로드 못하는 경우가 생겨서 막아놓았다.
			if(_strnicmp(module.szModule,"d3d9",4)!=0) {
				if (!g_pfnSymLoadModule(hProcess, 0, module.szExePath, 0, 0, 0))
				{
//					::MessageBox(NULL,"SymLoadModule failed","error",MB_OK);
					mlog("SymLoadModule failed %d ( module = %s , exe = %s ) \n",GetLastError(),module.szModule,module.szExePath);
				}else {
//					mlog("LoadModuleSymbols,SymLoadModule ok ( module = %s , exe = %s ) \n",module.szModule,module.szExePath);
				}
			}else{
//				mlog("skip d3d9\n");
			}

			fFlag = lpfModule32Next(hSnapShot, &module);
		}

		FreeLibrary(hInstLib) ;
	}   
}
#pragma warning(default: 4996)

DWORD GetCrashInfo(LPEXCEPTION_POINTERS exceptionInfo, std::string& str)
{
	auto Append = [&](const char *Format, ...)
	{
		char buf[512];

		va_list args;

		va_start(args, Format);
		int ret = vsprintf_safe(buf, Format, args);
		va_end(args);

		str += buf;
	};

	char LastError[256];

	auto GetLastErrorString = [&]() -> const char*
	{
		auto ret = FormatMessageA(FORMAT_MESSAGE_FROM_SYSTEM, nullptr, GetLastError(),
			MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), LastError, sizeof(LastError), nullptr);

		if (!ret)
			return "Failed to retrieve error string";

		return LastError;
	};

	auto PrintError = [&](const char* FunctionName)
	{
		auto LastError = GetLastError();

		if (LastError == ERROR_INVALID_ADDRESS)
			return;

		Append("     %s failed with error code %d: %s\n", FunctionName, LastError, GetLastErrorString());
	};

	DWORD             dwDisplacement = 0;
	DWORD             dwMachType;
	int               nframes = 0;
	LPSTR             szSymName;
	IMAGEHLP_MODULE   mi;
	STACKFRAME        stk;
	CONTEXT           context;
	LPCONTEXT         lpContext = 0;
	HANDLE            hProcess, hThread;
	IMAGEHLP_LINE     imageLine;
	BOOL              fDisplayCurrentStackFrame = TRUE;
	char              buffer[1024];
	PIMAGEHLP_SYMBOL  pSymbol;
	HINSTANCE         hLibrary;

	hLibrary = LoadLibrary("dbghelp.dll");

	if (hLibrary)
	{
		g_pfnSymSetOptions			= (fn_sym_SetOption)				GetProcAddress(hLibrary, "SymSetOptions");
		g_pfnSymInitialize			= (fn_sym_initialize)				GetProcAddress(hLibrary, "SymInitialize");
		g_pfnSymCleanup				= (fn_sym_Cleanup)					GetProcAddress(hLibrary, "SymCleanup");

		g_pfnSymGetSymFromAddr		= (fn_sym_GetSymFromAddr)			GetProcAddress(hLibrary, "SymGetSymFromAddr");
		g_pfnSymGetLineFromAddr		= (fn_sym_GetLineFromAddr)			GetProcAddress(hLibrary, "SymGetLineFromAddr");

		g_pfnEnumerateLoadedModules = (fn_sym_EnumerateLoadModules)		GetProcAddress(hLibrary, "EnumerateLoadedModules");
		g_pfnSymGetModuleInfo		= (fn_sym_GetModuleInfo)			GetProcAddress(hLibrary, "SymGetModuleInfo");
		g_pfnSymLoadModule			= (fn_sym_LoadModule)				GetProcAddress(hLibrary, "SymLoadModule");
		g_pfnStackWalk				= (fn_sym_StackWalk)				GetProcAddress(hLibrary, "StackWalk");

		g_pfnFunctionTableAccessRoutine = (PFUNCTION_TABLE_ACCESS_ROUTINE) GetProcAddress(hLibrary, "SymFunctionTableAccess");
		g_pfnGetModuleBaseRoutine = (PGET_MODULE_BASE_ROUTINE)		GetProcAddress(hLibrary, "SymGetModuleBase");

		if (!g_pfnSymSetOptions || !g_pfnSymInitialize || !g_pfnSymCleanup || !g_pfnSymGetSymFromAddr || !g_pfnEnumerateLoadedModules || !g_pfnSymGetModuleInfo || !g_pfnSymLoadModule || !g_pfnStackWalk)
		{
			FreeLibrary(hLibrary);
			return EXCEPTION_EXECUTE_HANDLER;
		}
	}
	else
	{
		return EXCEPTION_EXECUTE_HANDLER;
	}

	hProcess = GetCurrentProcess();
	hThread = GetCurrentThread();

	if (!exceptionInfo)
	{
		char* ptr = 0;
		__try
		{
			*ptr = 0;
		}
		__except(CopyMemory(&context, (GetExceptionInformation())->ContextRecord, sizeof(context)), EXCEPTION_EXECUTE_HANDLER)
		{
		}
		lpContext = &context;
		fDisplayCurrentStackFrame = FALSE;
	}
	else
	{
		CopyMemory(&context, exceptionInfo->ContextRecord, sizeof(context));
		lpContext = &context;
	}

	g_pfnSymSetOptions(SYMOPT_UNDNAME|SYMOPT_LOAD_LINES);

	if (!g_pfnSymInitialize(hProcess, NULL, FALSE))
	{
		return EXCEPTION_EXECUTE_HANDLER;
	}

	LoadModuleSymbols(GetCurrentProcessId(), hProcess);

	ZeroMemory(&stk, sizeof(stk));

	dwMachType				= IMAGE_FILE_MACHINE_I386;
#ifndef _WIN64
	stk.AddrPC.Offset		= lpContext->Eip;
	stk.AddrStack.Offset	= lpContext->Esp;
	stk.AddrFrame.Offset	= lpContext->Ebp;
#else
	stk.AddrPC.Offset = lpContext->Rip;
	stk.AddrStack.Offset = lpContext->Rsp;
	stk.AddrFrame.Offset = lpContext->Rbp;
#endif
	stk.AddrPC.Mode			= AddrModeFlat;
	stk.AddrStack.Mode		= AddrModeFlat;
	stk.AddrFrame.Mode		= AddrModeFlat;

	memset(buffer, 0, sizeof(buffer));

	pSymbol = (PIMAGEHLP_SYMBOL)buffer;
	pSymbol->SizeOfStruct = sizeof(IMAGEHLP_SYMBOL);
	pSymbol->MaxNameLength = sizeof(buffer) - sizeof(IMAGEHLP_SYMBOL) + 1;

	while (g_pfnStackWalk(dwMachType, hProcess, hThread, &stk, lpContext, 0, g_pfnFunctionTableAccessRoutine, g_pfnGetModuleBaseRoutine, 0))
	{
		if (!fDisplayCurrentStackFrame)
		{
			fDisplayCurrentStackFrame = TRUE;
			continue;
		}

		pSymbol->Address = stk.AddrPC.Offset;
		dwDisplacement = 0;

		// 주소 -100 부터 찾아주기 넣기 ?? 추가 - 버젼 구분해서 win98 이라면

		if (g_pfnSymGetSymFromAddr(hProcess, stk.AddrPC.Offset, &dwDisplacement, pSymbol))
		{
			szSymName = pSymbol->Name;
		}
		else
		{
			szSymName = "<nosymbols>";
//			::MessageBox(NULL,"SymGetSymFromAddr failed","error",MB_OK);

			PrintError("SymGetSymFromAddr");
		}

		nframes++;

		mi.SizeOfStruct = sizeof(mi);

		char Offset[32];
		bool bGotModule = false;

		if (g_pfnSymGetModuleInfo(hProcess, stk.AddrPC.Offset, &mi))
		{
			if (_itoa_s(stk.AddrPC.Offset - mi.BaseOfImage, Offset, 16))
				strcpy_safe(Offset, "???");
			bGotModule = true;
		}
		else
		{
			strcpy_safe(Offset, "???");
		}

		Append("Frame %02d: Address: %08X (base + %s), return address: %08X\n", nframes, stk.AddrPC.Offset, Offset, stk.AddrReturn.Offset);

		if (bGotModule)
		{
			Append("     ModuleName : %s\n", mi.ModuleName);
		}
		else
		{
			PrintError("SymGetModuleInfo");
		}

		Append("     Param[0] : %08x\n", stk.Params[0]);
		Append("     Param[1] : %08x\n", stk.Params[1]);
		Append("     Param[2] : %08x\n", stk.Params[2]);
		Append("     Param[3] : %08x\n", stk.Params[3]);

		dwDisplacement = 0;

		imageLine.SizeOfStruct = sizeof(imageLine);

		if (g_pfnSymGetLineFromAddr)
		{
			if (g_pfnSymGetLineFromAddr( hProcess, stk.AddrPC.Offset, &dwDisplacement, &imageLine))
			{
				Append("     File Name : %s\n", imageLine.FileName);
				Append("     Line Number : %d\n", imageLine.LineNumber);
			}else {
				PrintError("SymGetLineFromAddr");
			}
		}

		Append("     Function Name : %s\n", szSymName);

		Append("\n\n");

		if (!stk.AddrReturn.Offset)
		{
			break;
		}
	}

	g_pfnSymCleanup(hProcess);

	FreeLibrary(hLibrary);

	return EXCEPTION_EXECUTE_HANDLER;
}

#endif
