#include "stdafx.h"

#ifndef _DEBUG

void MInitTraceMemory()		{ }
void MShutdownTraceMemory()	{ }

#else

#include "Windows.h"
#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable: 4091)
#endif
#include <imagehlp.h>
#ifdef _MSC_VER
#pragma warning(pop)
#endif
#include "crtdbg.h"
#include "Tlhelp32.h"
#include <vector>

using namespace std;

const int NMAXCALLSTACK = 100;
#define MTMFILENAME	"memory.mtm"

static FILE *file;
static DWORD callStack[NMAXCALLSTACK];
static bool bShutdown = false;
static char	szCurrentDir[_MAX_PATH];
//CRITICAL_SECTION csNewMemoryDumpLock;

int __cdecl MyAllocHook( 
            int nAllocType, 
            void * pvData, 
            size_t nSize, 
            int nBlockUse, 
            long lRequest, 
            const unsigned char * szFileName, 
            int nLine 
            ) 
{ 
	if(bShutdown) return TRUE;

	if ( nBlockUse == _CRT_BLOCK ) // Ignore internal C runtime library allocations 
	return( TRUE ); 

//	if(nAllocType==1) // allocating
	{
//		EnterCriticalSection(&csNewMemoryDumpLock);

		int nDepth = 0;
		DWORD *stack;
		_asm {
			mov stack, ebp
		}
		DWORD retAddress;
		
		try {
			while((retAddress = *(stack+1)) && nDepth<NMAXCALLSTACK) {
				stack = (DWORD*)(void*)stack[0];
				callStack[nDepth] = retAddress;
				nDepth++;
			}
		} catch (std::exception) {
			int a= 0;
		}
		catch ( ... )
		{
			int a= 0;
		}


		fwrite(&nDepth,sizeof(int),1,file);
		fwrite(&lRequest,sizeof(long),1,file);
		fwrite(callStack,sizeof(DWORD),nDepth,file);

//		LeaveCriticalSection(&csNewMemoryDumpLock);
	}
	return TRUE;
}

struct MCallStackInfo
{
	long lRequest;
	int nCount;
	DWORD *pCallStack;
};

namespace MTraceMemory
{

static BOOL CALLBACK EnumLoadedModulesCallback(LPSTR pModuleName, ULONG ulModuleBase, ULONG ulModuleSize, PVOID pUserContext)
{
	if (!SymLoadModule((HANDLE)pUserContext, 0, pModuleName, 0, ulModuleBase, ulModuleSize))
	{
		//::MessageBox(NULL,"SymLoadModule failed","error",MB_OK);
		return false;
	}
	return TRUE;
}

}

// Disable warning about GetVersionEx deprecation
#pragma warning(disable:4996)
bool InitializeSymbols()
{
	DWORD dwProcessId = GetCurrentProcessId();
	HANDLE hProcess = GetCurrentProcess();
	if(!SymInitialize(hProcess,NULL,false)) return false;
//	if(!SymLoadModule64(hProcess ,NULL, "MatchServerD.exe",NULL,NULL,NULL)) return false;

	OSVERSIONINFO   osver;
	osver.dwOSVersionInfoSize = sizeof(osver);
	if (!GetVersionEx(&osver)) return false;

	if (osver.dwPlatformId == VER_PLATFORM_WIN32_NT)
	{
		if (!EnumerateLoadedModules(hProcess, (PENUMLOADED_MODULES_CALLBACK)MTraceMemory::EnumLoadedModulesCallback, (PVOID)hProcess))
		{
//			::MessageBox(NULL,"EnumerateLoadedModules failed","error",MB_OK);
		}
	}else
	if (osver.dwPlatformId == VER_PLATFORM_WIN32_WINDOWS)
	{
		HANDLE          hSnapShot;
		hSnapShot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, dwProcessId);
		if (hSnapShot == INVALID_HANDLE_VALUE) return false;

		MODULEENTRY32   module;
		BOOL            fFlag;
		module.dwSize = sizeof(module) ;
		fFlag = Module32First(hSnapShot, &module);

		while (fFlag)
		{
			if (!SymLoadModule(hProcess, 0, module.szExePath, 0, 0, 0))
			{
//				::MessageBox(NULL,"SymLoadModule failed","error",MB_OK);
			}
			fFlag = Module32Next(hSnapShot, &module);
		}
	}
	return true;
}
#pragma warning(default:4996)

void MDumpCallStack(int lRequest);

int MyReportHook( int reportType, char *message, int *returnValue )
{
//	mlog("test : %s\n",message);

	// {1234} 와 같은형식의 memory leak 리포트를 가로채서 콜스택을 덤프한다
	if(message[0]=='{' && message[strlen(message)-2]=='}' && message[strlen(message)-1]==' ') {
		MDumpCallStack(atoi(message+1));
	}
	return 0;
}

void MInitTraceMemory()
{
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);

	file = nullptr;
	fopen_s(&file, MTMFILENAME, "wb+");
//	InitializeCriticalSection(&csNewMemoryDumpLock);

   _CrtSetAllocHook( MyAllocHook );

   GetCurrentDirectory(sizeof(szCurrentDir),szCurrentDir);

}

static vector<MCallStackInfo*> *memories;

void MShutdownTraceMemory()
{
	_CrtSetDbgFlag(0);
	bShutdown = true;
//	DeleteCriticalSection(&csNewMemoryDumpLock);
	fclose(file);

	// 전역변수로 두면 지워질수 있다. 의도적으로 leak을 낸다
	memories = new vector<MCallStackInfo*>;

	char szFileName[_MAX_PATH];
	sprintf_safe(szFileName,"%s/%s",szCurrentDir,MTMFILENAME);
	FILE *file = nullptr;
	fopen_s(&file, szFileName, "rb");

	int nCount = 0;
	while(fread(&nCount,sizeof(int),1,file)) {
		MCallStackInfo *callstack = new MCallStackInfo;
		callstack->nCount = nCount;
		callstack->pCallStack = new DWORD[nCount];
		fread(&callstack->lRequest,sizeof(long),1,file);
		size_t nRead = fread(callstack->pCallStack,sizeof(DWORD),nCount,file);
		_ASSERT(nRead == (size_t)nCount);

		int index = memories->size();
		if(index>=1300 && index<=1400)
		{
			printf("%d : %d depth\n",index,nCount);
		}
		

		memories->push_back(callstack);
	}
	fclose(file);

	InitializeSymbols();

   _CrtSetReportHook( MyReportHook );
//   _CrtDumpMemoryLeaks();
}

int find(vector<MCallStackInfo*> &memories,long lRequest)
{
	for(size_t i=0;i<memories.size();i++)
	{
		if(memories[i]->lRequest == lRequest)
			return i;
	}
	return -1;
}


void MDumpCallStack(int lRequest)
{
	char buffer[2048];

	int index = find(*memories,lRequest);
	if(index != -1)
	{
		for(int i=0;i<(*memories)[index]->nCount;i++) {

			DWORD address = (*memories)[index]->pCallStack[i];

			PIMAGEHLP_SYMBOL  pSymbol;
			char              szSymBuffer[1024];
			LPSTR             szSymName;

			memset(buffer, 0, sizeof(buffer));

			pSymbol = (PIMAGEHLP_SYMBOL)szSymBuffer;
			pSymbol->SizeOfStruct = sizeof(IMAGEHLP_SYMBOL);
			pSymbol->MaxNameLength = sizeof(buffer) - sizeof(IMAGEHLP_SYMBOL) + 1;

			DWORD             dwDisplacement = 0;

			if (SymGetSymFromAddr(GetCurrentProcess(), address, &dwDisplacement, pSymbol))
				szSymName = pSymbol->Name;
			else
				szSymName = "<nosymbols>";

			IMAGEHLP_LINE     imageLine;
			ZeroMemory(&imageLine,sizeof(imageLine));
			imageLine.SizeOfStruct = sizeof(imageLine);

			SymGetLineFromAddr( GetCurrentProcess(), address, &dwDisplacement, &imageLine);

//			sprintf_safe(buffer,"   %s(%d) \n",imageLine.FileName, imageLine.LineNumber );
			sprintf_safe(buffer,"   %s(%d) : %s \n",imageLine.FileName, imageLine.LineNumber, szSymName);
			OutputDebugString(buffer);
		}
	}else
	{
		sprintf_safe(buffer,"%d request not found\n",lRequest);
		OutputDebugString(buffer);
	}
}

#endif