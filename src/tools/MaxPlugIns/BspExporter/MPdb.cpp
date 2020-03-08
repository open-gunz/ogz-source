#include "stdafx.h"
#include "stdafx.h"
#include "MPdb.h"
#include <stdio.h>
#include <imagehlp.h>
#include <tlhelp32.h>

/*
using namespace std;


struct CImageHlp_Module : public IMAGEHLP_MODULE
{
    CImageHlp_Module ( )
    {
        memset ( this , NULL , sizeof ( IMAGEHLP_MODULE ) ) ;
        SizeOfStruct = sizeof ( IMAGEHLP_MODULE ) ;
    }
};

struct CImageHlp_Line : public IMAGEHLP_LINE
{
    CImageHlp_Line ( )
    {
        memset ( this , NULL , sizeof ( IMAGEHLP_LINE ) ) ;
        SizeOfStruct = sizeof ( IMAGEHLP_LINE ) ;
    }
} ;

class MPdb
{
public :
    MPdb ( void )
    {
    }

    virtual ~MPdb ( void )
    {
    }

public :

    BOOL GetImageHlpVersion ( DWORD & dwMS , DWORD & dwLS )
    {
        return( GetInMemoryFileVersion ( _T ( "DBGHELP.DLL" ) , dwMS,dwLS ) );
    }

    BOOL GetDbgHelpVersion ( DWORD & dwMS , DWORD & dwLS )
    {
        return( GetInMemoryFileVersion ( _T ( "DBGHELP.DLL" ) , dwMS,dwLS ) );
    }

    BOOL GetPDBReaderVersion ( DWORD & dwMS , DWORD & dwLS )
    {
        if ( TRUE == GetInMemoryFileVersion ( _T ( "MSDBI.DLL" ) ,dwMS, dwLS ) )
        {
            return ( TRUE ) ;
        }
        else if ( TRUE == GetInMemoryFileVersion ( _T ( "MSPDB60.DLL" ),dwMS,dwLS ))
        {
            return ( TRUE ) ;
        }
        return ( GetInMemoryFileVersion ( _T ( "MSPDB50.DLL" ) ,dwMS,dwLS ) ) ;
    }

    BOOL GetInMemoryFileVersion ( LPCTSTR szFile ,DWORD & dwMS, DWORD & dwLS )
    {
        HMODULE hInstIH = GetModuleHandle ( szFile ) ;

        TCHAR szImageHlp[ MAX_PATH ] ;
        GetModuleFileName ( hInstIH , szImageHlp , MAX_PATH ) ;

        dwMS = 0 ;
        dwLS = 0 ;

        DWORD dwVerInfoHandle ;
        DWORD dwVerSize ;

        dwVerSize = GetFileVersionInfoSize ( szImageHlp , &dwVerInfoHandle  ) ;

        if ( 0 == dwVerSize )
        {
            return ( FALSE ) ;
        }

        LPVOID lpData = (LPVOID)new TCHAR [ dwVerSize ] ;
        if ( FALSE == GetFileVersionInfo ( szImageHlp , dwVerInfoHandle , dwVerSize ,lpData ) )
        {
            delete [] lpData ;
            return ( FALSE ) ;
        }

        VS_FIXEDFILEINFO * lpVerInfo;

        UINT uiLen;
        BOOL bRet = VerQueryValue ( lpData,_T ( "\\" ),(LPVOID*)&lpVerInfo,&uiLen );

        if ( TRUE == bRet )
        {
            dwMS = lpVerInfo->dwFileVersionMS ;
            dwLS = lpVerInfo->dwFileVersionLS ;
        }

        delete [] lpData ;

        return ( bRet ) ;
    }

public :

    BOOL SymInitialize ( IN HANDLE hProcess, IN LPSTR UserSearchPath ,IN BOOL fInvadeProcess)
    {
        m_hProcess = hProcess ;
        return ( ::SymInitialize ( hProcess , UserSearchPath , fInvadeProcess  ) ) ;
    }

#ifdef USE_BUGSLAYERUTIL
    BOOL BSUSymInitialize ( DWORD  dwPID , HANDLE hProcess ,PSTR UserSearchPath , BOOL fInvadeProcess )
    {
        m_hProcess = hProcess ;
        return ( ::BSUSymInitialize ( dwPID , hProcess , UserSearchPath , fInvadeProcess  ) ) ;
    }
#endif  // USE_BUGSLAYERUTIL

    BOOL SymCleanup ( void )
    {
        return ( ::SymCleanup ( m_hProcess ) ) ;
    }

public :

    BOOL SymEnumerateModules ( IN PSYM_ENUMMODULES_CALLBACK EnumModulesCallback,IN PVOID UserContext )
    {
        return ( ::SymEnumerateModules ( m_hProcess , EnumModulesCallback , UserContext ) ) ;
    }

    BOOL SymLoadModule ( IN  HANDLE hFile ,IN  PSTR   ImageName,IN  PSTR   ModuleName,IN  DWORD  BaseOfDll,IN  DWORD  SizeOfDll )
    {
        return ( ::SymLoadModule ( m_hProcess   , hFile ,ImageName , ModuleName , BaseOfDll , SizeOfDll ) ) ;
    }

    BOOL EnumerateLoadedModules ( IN PENUMLOADED_MODULES_CALLBACK EnumLoadedModulesCallback,IN PVOID UserContext)
    {
        return ( ::EnumerateLoadedModules ( m_hProcess , EnumLoadedModulesCallback , UserContext ));
    }

    BOOL SymUnloadModule ( IN  DWORD BaseOfDll )
    {
        return ( ::SymUnloadModule ( m_hProcess , BaseOfDll ) ) ;
    }

    BOOL SymGetModuleInfo ( IN  DWORD dwAddr, OUT PIMAGEHLP_MODULE ModuleInfo  )
    {
        return ( ::SymGetModuleInfo ( m_hProcess , dwAddr, ModuleInfo ) ) ;
    }

    DWORD SymGetModuleBase ( IN DWORD dwAddr )
    {
        return ( ::SymGetModuleBase ( m_hProcess , dwAddr ) ) ;
    }

public      :

    BOOL SymEnumerateSymbols (IN DWORD BaseOfDll, IN PSYM_ENUMSYMBOLS_CALLBACK EnumSymbolsCallback,IN PVOID UserContext )
    {
        return ( ::SymEnumerateSymbols ( m_hProcess , BaseOfDll , EnumSymbolsCallback , UserContext ) ) ;
    }

    BOOL SymGetSymFromAddr ( IN  DWORD dwAddr , OUT PDWORD pdwDisplacement , OUT PIMAGEHLP_SYMBOL Symbol)
    {
        return ( ::SymGetSymFromAddr ( m_hProcess , dwAddr , pdwDisplacement , Symbol ) ) ;
    }

    BOOL SymGetSymFromName ( IN  LPSTR Name, OUT PIMAGEHLP_SYMBOL Symbol)
    {
        return ( ::SymGetSymFromName ( m_hProcess , Name ,  Symbol ) ) ;
    }

    BOOL SymGetSymNext ( IN OUT PIMAGEHLP_SYMBOL Symbol )
    {
        return ( ::SymGetSymNext ( m_hProcess , Symbol ) ) ;
    }

    BOOL SymGetSymPrev ( IN OUT PIMAGEHLP_SYMBOL Symbol )
    {
        return ( ::SymGetSymPrev ( m_hProcess , Symbol ) ) ;
    }

public :

    BOOL SymGetLineFromAddr ( IN  DWORD dwAddr , OUT PDWORD pdwDisplacement , OUT PIMAGEHLP_LINE Line )
    {

#ifdef DO_NOT_WORK_AROUND_SRCLINE_BUG
        return ( ::SymGetLineFromAddr ( m_hProcess , dwAddr , pdwDisplacement , Line ) ) ;

#else

        DWORD dwTempDis = 0 ;

        while ( FALSE == ::SymGetLineFromAddr ( m_hProcess , dwAddr - dwTempDis , pdwDisplacement , Line ) )
        {
            dwTempDis += 1 ;
            if ( 100 == dwTempDis )
            {
                return ( FALSE ) ;
            }
        }

        if ( 0 != dwTempDis )
        {
            *pdwDisplacement = dwTempDis ;
        }
        return ( TRUE ) ;
#endif // DO_NOT_WORK_AROUND_SRCLINE_BUG
    }

	BOOL SymGetLineFromName ( IN LPSTR ModuleName,IN LPSTR FileName,IN DWORD dwLineNumber,OUT PLONG plDisplacement,IN OUT PIMAGEHLP_LINE Line)
    {
        return ( ::SymGetLineFromName ( m_hProcess , ModuleName , FileName , dwLineNumber , plDisplacement , Line ) ) ;
    }

    BOOL SymGetLineNext ( IN OUT PIMAGEHLP_LINE Line )
    {
        return ( ::SymGetLineNext ( m_hProcess , Line ) ) ;
    }

    BOOL SymGetLinePrev ( IN OUT PIMAGEHLP_LINE Line )
    {
        return ( ::SymGetLinePrev ( m_hProcess , Line ) ) ;
    }

    BOOL SymMatchFileName ( IN  LPSTR   FileName ,IN  LPSTR   Match, OUT LPSTR * FileNameStop, OUT LPSTR * MatchStop)
    {
        return ( ::SymMatchFileName ( FileName , Match , FileNameStop , MatchStop) ) ;
    }

public      :

    LPVOID SymFunctionTableAccess ( DWORD AddrBase )
    {
        return ( ::SymFunctionTableAccess ( m_hProcess , AddrBase ) ) ;
    }

    BOOL SymGetSearchPath ( OUT LPSTR SearchPath , IN  DWORD SearchPathLength )
    {
        return ( ::SymGetSearchPath ( m_hProcess , SearchPath , SearchPathLength  ) ) ;
    }

    BOOL SymSetSearchPath ( IN LPSTR SearchPath )
    {
        return ( ::SymSetSearchPath ( m_hProcess , SearchPath ) ) ;
    }

    BOOL SymRegisterCallback ( IN PSYMBOL_REGISTERED_CALLBACK CallbackFunction,IN PVOID UserContext )
    {
        return ( ::SymRegisterCallback ( m_hProcess , CallbackFunction , UserContext ) ) ;
    }

protected :

    HANDLE      m_hProcess ;
};

bool GetCrashInfo(char* FullPathFileName,DWORD CrashAddress,char* OutInfo)
{
	MPdb t_sym;

	srand ( 100 );
	HANDLE hRandHandle = (HANDLE)rand( );

	int bRet = t_sym.SymInitialize ( hRandHandle , NULL , FALSE );

	if ( FALSE == bRet )	return false;

    DWORD dwOpts = SymGetOptions( );
    SymSetOptions ( dwOpts | SYMOPT_LOAD_LINES | SYMOPT_OMAP_FIND_NEAREST  ) ;

//	::OutputDebugString("SymInitialize!!\n");

	string	mod_name;
	string	func_name;
	string	code_name;
	DWORD	line_num;

	LOADED_IMAGE stLI ;

	if ( ( FALSE == MapAndLoad ((LPTSTR)(LPCTSTR)FullPathFileName , NULL , &stLI, TRUE , TRUE ) ) || ( IMAGE_NT_SIGNATURE != stLI.FileHeader->Signature ) )
	{
		::OutputDebugString(FullPathFileName);
		::OutputDebugString(" MapAndLoad failed!!\n");
		return false;
	}

	DWORD base_address = stLI.FileHeader->OptionalHeader.ImageBase;

	UnMapAndLoad ( &stLI ) ;

	bRet =  t_sym.SymLoadModule( NULL,(LPTSTR)(LPCTSTR)FullPathFileName,NULL,base_address,0	);

	if( FALSE == bRet )
	{
		::OutputDebugString("m_cSymEng.SymLoadModule failed!!\n");
		return false;
	}

	CImageHlp_Line		stIHLine;
	DWORD				dwFnDispl;
	DWORD				dwSrcDispl;

    IMAGEHLP_SYMBOL* pstIHSym =
            (IMAGEHLP_SYMBOL *)new BYTE [ sizeof ( IMAGEHLP_SYMBOL ) + 256 ];
            memset ( pstIHSym , NULL , sizeof ( IMAGEHLP_SYMBOL ) + 256 );
            pstIHSym->SizeOfStruct = sizeof ( IMAGEHLP_SYMBOL ) + 256;
            pstIHSym->MaxNameLength = 256 ;

	BOOL bSymFound  = TRUE ;
	BOOL bLineFound = TRUE ;
	BOOL bModFound  = TRUE ;

	CImageHlp_Module cMod;

	bModFound = t_sym.SymGetModuleInfo ( CrashAddress , &cMod );

	if ( FALSE == bModFound )
	{
		bSymFound = FALSE;
		bLineFound = FALSE;
		::OutputDebugString("SymGetModuleInfo failed!!\n");
		return false;
	}
	else
	{
		mod_name = cMod.ModuleName;

		BOOL bRet = t_sym.SymGetSymFromAddr ( CrashAddress,&dwFnDispl ,pstIHSym);

		if ( FALSE == bRet )
		{
			bSymFound = FALSE;
			::OutputDebugString("SymGetSymFromAddr failed!!\n");
		}

		func_name = pstIHSym->Name;

		bRet = t_sym.SymGetLineFromAddr ( CrashAddress , &dwSrcDispl , &stIHLine ) ;
		if ( FALSE == bRet )
		{
			bLineFound = FALSE ;
			::OutputDebugString("SymGetLineFromAddr failed!!\n");
		}
		code_name = stIHLine.FileName;
		line_num  = stIHLine.LineNumber;
	}

	////////////////////////////////////
	//
	////////////////////////////////////

	t_sym.SymCleanup();

	char temp[256];

	sprintf(temp,		"Line Number	= %d line\n",line_num);

	string all_str =	"Module Name	= "+ mod_name +"\n"+ 
						"Function Name	= "+ func_name +"()\n"+ 
						"Source Name	= "+ code_name +"\n"+ temp;

	strcpy(OutInfo,all_str.c_str());

	delete[] pstIHSym;

	return true;
}
*/

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

BOOL CALLBACK EnumLoadedModulesCallback(LPSTR pModuleName, ULONG ulModuleBase,  ULONG ulModuleSize,  PVOID pUserContext)
{
    if (!g_pfnSymLoadModule((HANDLE)pUserContext, 0, pModuleName, 0, ulModuleBase, ulModuleSize))
    {
//		::MessageBox(NULL,"SymLoadModule failed","error",MB_OK);
		return false;
    }
    return TRUE;
}

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
		if (!g_pfnEnumerateLoadedModules(hProcess, EnumLoadedModulesCallback, (PVOID)hProcess))
		{
//			::MessageBox(NULL,"EnumerateLoadedModules failed","error",MB_OK);
		}
		return;
	}

	if (osver.dwPlatformId == VER_PLATFORM_WIN32_WINDOWS)
	{
		hInstLib = LoadLibraryA("Kernel32.DLL");

		if (hInstLib == NULL)          return;

		lpfCreateToolhelp32Snapshot=(HANDLE(WINAPI *)(DWORD,DWORD)) GetProcAddress(hInstLib, "CreateToolhelp32Snapshot");

		lpfModule32First=(BOOL(WINAPI *)(HANDLE,LPMODULEENTRY32)) GetProcAddress(hInstLib, "Module32First");

		lpfModule32Next=(BOOL(WINAPI *)(HANDLE,LPMODULEENTRY32)) GetProcAddress(hInstLib, "Module32Next");

		if (lpfModule32Next == NULL || lpfModule32First == NULL || lpfCreateToolhelp32Snapshot == NULL)
		{
			FreeLibrary(hInstLib);
			return;
		}

		hSnapShot = lpfCreateToolhelp32Snapshot(TH32CS_SNAPMODULE, dwProcessId);

		if (hSnapShot == INVALID_HANDLE_VALUE)
		{
			FreeLibrary(hInstLib);
			return;
		}

		module.dwSize = sizeof(module) ;
		fFlag = lpfModule32First(hSnapShot, &module);

		while (fFlag)
		{
			if (!g_pfnSymLoadModule(hProcess, 0, module.szExePath, 0, 0, 0))
			{
//				::MessageBox(NULL,"SymLoadModule failed","error",MB_OK);
			}
			fFlag = lpfModule32Next(hSnapShot, &module);
		}

		FreeLibrary(hInstLib) ;
	}   
}

DWORD GetCrashInfo(LPEXCEPTION_POINTERS exceptionInfo,string& str)
{
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
	stk.AddrPC.Offset		= lpContext->Eip;
	stk.AddrStack.Offset	= lpContext->Esp;
	stk.AddrFrame.Offset	= lpContext->Ebp;
	stk.AddrPC.Mode			= AddrModeFlat;
	stk.AddrStack.Mode		= AddrModeFlat;
	stk.AddrFrame.Mode		= AddrModeFlat;

	memset(buffer, 0, sizeof(buffer));

	pSymbol = (PIMAGEHLP_SYMBOL)buffer;
	pSymbol->SizeOfStruct = sizeof(IMAGEHLP_SYMBOL);
	pSymbol->MaxNameLength = sizeof(buffer) - sizeof(IMAGEHLP_SYMBOL) + 1;

	char t_temp[1024];

	while (g_pfnStackWalk(dwMachType, hProcess, hThread, &stk, lpContext, 0, 0, 0, 0))
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
		}

		nframes++;

		sprintf(t_temp,"frame : (%02d) : PC Address : %08x\n", nframes, stk.AddrPC.Offset);

		str += t_temp;

		sprintf(t_temp,"     Param[0] : %08x\n", stk.Params[0]);		str += t_temp;
		sprintf(t_temp,"     Param[1] : %08x\n", stk.Params[1]);		str += t_temp;
		sprintf(t_temp,"     Param[2] : %08x\n", stk.Params[2]);		str += t_temp;
		sprintf(t_temp,"     Param[3] : %08x\n", stk.Params[3]);		str += t_temp;

		mi.SizeOfStruct = sizeof(mi);

		if (g_pfnSymGetModuleInfo(hProcess, stk.AddrPC.Offset, &mi))
		{
			sprintf(t_temp,"     ModuleName : %s\n", mi.ModuleName);	str += t_temp;
		}

		sprintf(t_temp,"     Function Name : %s\n", szSymName);			str += t_temp;

		dwDisplacement = 0;

		imageLine.SizeOfStruct = sizeof(imageLine);

		if (g_pfnSymGetLineFromAddr)
		{
			if (g_pfnSymGetLineFromAddr( hProcess, stk.AddrPC.Offset, &dwDisplacement, &imageLine))
			{
				sprintf(t_temp,"     File Name : %s\n", imageLine.FileName);		str += t_temp;
				sprintf(t_temp,"     Line Number : %d\n", imageLine.LineNumber);	str += t_temp;
			}
		}

		sprintf(t_temp,"\n\n");

		if (!stk.AddrReturn.Offset)
		{
			break;
		}
	}

	g_pfnSymCleanup(hProcess);

	FreeLibrary(hLibrary);

	return EXCEPTION_EXECUTE_HANDLER;
}

