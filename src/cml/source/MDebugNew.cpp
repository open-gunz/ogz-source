#include "stdafx.h"
#include "MDebugNew.h"

#ifdef _MTRACEMEMORY

#define MAX_CALL_STACK	4

struct MNEWINFO {
	DWORD callStack[MAX_CALL_STACK];	// 콜스택 4개만 기록한다
};

/////////////////////////////////////////////////////////////////////////

MNEWLIST		MNewMemories::m_List;
MNEWPOINTERMAP	MNewMemories::m_Pointers;

char szNewPosition[512];


#include <stdio.h>
#include <imagehlp.h>
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

static fn_sym_SetOption					g_pfnSymSetOptions;
static fn_sym_initialize				g_pfnSymInitialize;
static fn_sym_Cleanup					g_pfnSymCleanup;
static fn_sym_GetSymFromAddr			g_pfnSymGetSymFromAddr;
static fn_sym_GetLineFromAddr			g_pfnSymGetLineFromAddr;
static fn_sym_EnumerateLoadModules		g_pfnEnumerateLoadedModules;
static fn_sym_LoadModule				g_pfnSymLoadModule;
static fn_sym_GetModuleInfo				g_pfnSymGetModuleInfo;
static fn_sym_StackWalk					g_pfnStackWalk;


HINSTANCE	MNewMemories::m_hModule;
bool		MNewMemories::m_bInitialized = false;

bool MNewMemories::Init()
{
	m_hModule = LoadLibrary("dbghelp.dll");

	if (m_hModule)
	{
		g_pfnSymSetOptions			= (fn_sym_SetOption)				GetProcAddress(m_hModule, "SymSetOptions");
		g_pfnSymInitialize			= (fn_sym_initialize)				GetProcAddress(m_hModule, "SymInitialize");
		g_pfnSymCleanup				= (fn_sym_Cleanup)					GetProcAddress(m_hModule, "SymCleanup");

		g_pfnSymGetSymFromAddr		= (fn_sym_GetSymFromAddr)			GetProcAddress(m_hModule, "SymGetSymFromAddr");
		g_pfnSymGetLineFromAddr		= (fn_sym_GetLineFromAddr)			GetProcAddress(m_hModule, "SymGetLineFromAddr");

		g_pfnEnumerateLoadedModules = (fn_sym_EnumerateLoadModules)		GetProcAddress(m_hModule, "EnumerateLoadedModules");
		g_pfnSymGetModuleInfo		= (fn_sym_GetModuleInfo)			GetProcAddress(m_hModule, "SymGetModuleInfo");
		g_pfnSymLoadModule			= (fn_sym_LoadModule)				GetProcAddress(m_hModule, "SymLoadModule");
		g_pfnStackWalk				= (fn_sym_StackWalk)				GetProcAddress(m_hModule, "StackWalk");

		if (!g_pfnSymSetOptions || !g_pfnSymInitialize || !g_pfnSymCleanup || !g_pfnSymGetSymFromAddr || !g_pfnEnumerateLoadedModules || !g_pfnSymGetModuleInfo || !g_pfnSymLoadModule || !g_pfnStackWalk)
		{
			FreeLibrary(m_hModule);
			return false;
		}
	}
	m_bInitialized = true;
	return true;
}

void MNewMemories::Shutdown()
{
	Dump();
	FreeLibrary(m_hModule);
	m_bInitialized = false;
}

static BOOL CALLBACK EnumLoadedModulesCallback(LPSTR pModuleName, ULONG ulModuleBase,  ULONG ulModuleSize,  PVOID pUserContext)
{
    if (!g_pfnSymLoadModule((HANDLE)pUserContext, 0, pModuleName, 0, ulModuleBase, ulModuleSize))
    {
//		::MessageBox(NULL,"SymLoadModule failed","error",MB_OK);
		return false;
    }
    return TRUE;
}

static void LoadModuleSymbols(DWORD dwProcessId, HANDLE hProcess)
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

void MNewMemories::OnNew(MNEWINFO *allocator,void* pPointer)
{
	if(!m_bInitialized) return;

	MNEWLIST::iterator itr = m_List.insert(m_List.end(),allocator);
	m_Pointers.insert(MNEWPOINTERMAP::value_type(pPointer,itr));

	/*
	MNEWLIST::iterator itr = m_Positions.find(dwAllocator);
	if(itr==m_Positions.end())
	{
		MMALLOCINFO info;
		m_Positions.insert( MNEWLIST::value_type(dwAllocator,1));
		itr = m_Positions.find(dwAllocator);
	}else
		itr->second = itr->second +1;

	m_Pointers.insert(MNEWPOINTERMAP::value_type(pPointer,dwAllocator));
	*/
}

bool bDontCheck = false;

bool MNewMemories::OnDelete(void* pPointer)
{
	if(!m_bInitialized || bDontCheck) return false;
	if(pPointer==NULL) return false;

	bDontCheck=true;

	MNEWPOINTERMAP::iterator itr = m_Pointers.find(pPointer);
	if(itr==m_Pointers.end()) {
//		assert(0);
		bDontCheck=false;
		return false;		// 여기서 할당된 메모리가 아니다
	}else{

		m_List.erase(itr->second);

		/*
		DWORD dwAllocator = itr->second;
		MNEWLIST::iterator pitr = m_Positions.find(dwAllocator);

		int nCount = pitr->second;
		assert(nCount>0);
	    
		pitr->second--;
		if(pitr->second==0)
			m_Positions.erase(pitr);
		*/
	}
	m_Pointers.erase(itr);
	bDontCheck=false;
	return true;
}

class MNewInfoCmp {
public:
	bool operator() (const MNEWINFO& n1,const MNEWINFO& n2) const {
		for(int i=0;i<MAX_CALL_STACK;i++) {
			if(n1.callStack[i]<n2.callStack[i]) return true;
			if(n1.callStack[i]>n2.callStack[i]) return false;
		}
		return false;	// 같다
	}
};

void MNewMemories::Dump()
{
	bDontCheck = true;

	class MSortedMemories : public map <MNEWINFO,int,MNewInfoCmp> {
	public:
		void Add(MNEWINFO *pNewInfo) {
			iterator itr = find(*pNewInfo);
			if(itr==end()) {
				insert(value_type(*pNewInfo,1));
			}else
				itr->second = itr->second+1;
		}
	} sortedMemories;
	


	DWORD             dwDisplacement = 0;
	LPSTR             szSymName;
//	IMAGEHLP_MODULE   mi;
	HANDLE            hProcess, hThread;
	IMAGEHLP_LINE     imageLine;
	BOOL              fDisplayCurrentStackFrame = TRUE;
	char              buffer[1024];
	PIMAGEHLP_SYMBOL  pSymbol;

	hProcess = GetCurrentProcess();
	hThread = GetCurrentThread();

	if(!g_pfnSymSetOptions) return;
	g_pfnSymSetOptions(SYMOPT_UNDNAME|SYMOPT_LOAD_LINES);

	if (!g_pfnSymInitialize(hProcess, NULL, FALSE)) return;

	LoadModuleSymbols(GetCurrentProcessId(), hProcess);

	memset(buffer, 0, sizeof(buffer));

	pSymbol = (PIMAGEHLP_SYMBOL)buffer;
	pSymbol->SizeOfStruct = sizeof(IMAGEHLP_SYMBOL);
	pSymbol->MaxNameLength = sizeof(buffer) - sizeof(IMAGEHLP_SYMBOL) + 1;

	char temp[1024];

	FILE *file=fopen("memorydump.txt","w+");

	sprintf(temp,"----- current memory usage -----\n");
	OutputDebugString(temp);
	fprintf(file,temp);

	for(MNEWLIST::iterator itr=m_List.begin();itr!=m_List.end();itr++) {
		MNEWINFO *pNewInfo= *itr;
		sortedMemories.Add(pNewInfo);
	}

	for(MSortedMemories::iterator itr=sortedMemories.begin();itr!=sortedMemories.end();itr++) {

		sprintf(temp,"--- count (%d) \n",itr->second);
		fprintf(file,temp);
		OutputDebugString(temp);

		MNEWINFO newInfo = itr->first;
		for(int i=0;i<MAX_CALL_STACK;i++)
		{
			DWORD address = newInfo.callStack[i];
			if(!address) break;

			pSymbol->Address = address;
			dwDisplacement = 0;

			if (g_pfnSymGetSymFromAddr(hProcess, address, &dwDisplacement, pSymbol))
				szSymName = pSymbol->Name;
			else
				szSymName = "<nosymbols>";

			/*
			mi.SizeOfStruct = sizeof(mi);
			g_pfnSymGetModuleInfo(hProcess, pSymbol->Address, &mi);
			*/

			ZeroMemory(&imageLine,sizeof(imageLine));
			imageLine.SizeOfStruct = sizeof(imageLine);

			g_pfnSymGetLineFromAddr( hProcess, address, &dwDisplacement, &imageLine);

			char szIndent[MAX_CALL_STACK+1] = {0,};
			for(int j=0;j<i;j++)
			{
				szIndent[j]=' ';
			}

			sprintf(temp,"%s%s(%d) : function(%s) addr(%08x) \n",szIndent,imageLine.FileName,imageLine.LineNumber,szSymName,address);
			fprintf(file,temp);
			OutputDebugString(temp);
		}
	}

	sprintf(temp,"\n\n");
	OutputDebugString(temp);

	fclose(file);

	bDontCheck = false;
}

void* operator new(size_t _size)
{
	void * ptr = malloc(_size);
	if (ptr) {

		STACKFRAME stk;
		ZeroMemory(&stk, sizeof(stk));

		_asm {
			call next							// 이렇게 하면 eip 를 stack 에 push 한다
			next:
			pop eax								// push된 eip의 내용을 eax 로 pop 한다
			mov stk.AddrPC.Offset , eax
			mov stk.AddrStack.Offset , esp
			mov stk.AddrFrame.Offset , ebp
		}

		stk.AddrPC.Mode			= AddrModeFlat;
		stk.AddrStack.Mode		= AddrModeFlat;
		stk.AddrFrame.Mode		= AddrModeFlat;

		HANDLE	hProcess, hThread;
		hProcess = GetCurrentProcess();
		hThread = GetCurrentThread();

		if(g_pfnStackWalk && !bDontCheck)
		{
			bDontCheck=true;

			MNEWINFO *pInfo = new MNEWINFO;
			memset(pInfo, 0, sizeof(pInfo));

			int nDepth = 0;
			while(g_pfnStackWalk(IMAGE_FILE_MACHINE_I386, hProcess, hThread, &stk, NULL, 0, 0, 0, 0))
			{
				pInfo->callStack[nDepth++] = stk.AddrReturn.Offset;
				if (!stk.AddrReturn.Offset || nDepth>=MAX_CALL_STACK) break;
			}

			MNewMemories::OnNew(pInfo, ptr);
			bDontCheck=false;
		}
		return ptr;
	}

	return NULL;
}

void * operator new[]( size_t _size )
{
	// 쓸데없이 stack 한칸 차지하는걸 없애려면 copy & paste 하자 (inline은 실패했다)
	return operator new(_size);			
}

void operator delete(void* addr)
{
	if(MNewMemories::OnDelete(addr))
		free(addr);
}

void operator delete[](void* addr)
{
	if(MNewMemories::OnDelete(addr))
		free(addr);
}

#else
bool MNewMemories::Init()		{ return true; }
void MNewMemories::Shutdown()	{}
void MNewMemories::Dump()		{}
#endif
