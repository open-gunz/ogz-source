#include "stdafx.h"
#include <stdio.h>
#include <signal.h>
#include "fileinfo.h"
#include "MDebug.h"
#include <string>
#include "MPdb.h"

using namespace std;

static char logfilename[256];
//static char szLoghistory[MLOG_DEFAULT_HISTORY_COUNT][1024];
//static int nHead=0,nTail=0;
static int g_nLogMethod=MLOGSTYLE_DEBUGSTRING;

void InitLog(int logmethodflags, const char* pszLogFileName)
{
//	int i;
	g_nLogMethod=logmethodflags;
/*
	nHead=0;nTail=0;
	for(i=0;i<MLOG_DEFAULT_HISTORY_COUNT;i++)
		szLoghistory[i][0]=0;
*/
	if(g_nLogMethod&MLOGSTYLE_FILE)
	{
		GetFullPath(logfilename, pszLogFileName);
		FILE *pFile;
		pFile=fopen(logfilename,"w+");
		if( !pFile ) return;
		fclose(pFile);
	}
}

// history 봉인 !

void __cdecl MLog(const char *pFormat,...)
{
//	char *temp=szLoghistory[nTail];

	char temp[16*1024];	// 16k

	va_list args;

	va_start(args,pFormat);
	vsprintf(temp,pFormat,args);
	va_end(args);


	if(g_nLogMethod&MLOGSTYLE_FILE)
	{
		FILE *pFile;
		pFile = fopen( logfilename, "a" );
		if( !pFile ) pFile=fopen(logfilename,"w");
		if( pFile==NULL ) return;
		fprintf(pFile,temp);
		fclose(pFile);
	}
	if(g_nLogMethod&MLOGSTYLE_DEBUGSTRING)
	{
		OutputDebugString(temp);
	}

	/*
	nTail=(nTail+1)%MLOG_DEFAULT_HISTORY_COUNT;
	if((nTail==(nHead+1)%MLOG_DEFAULT_HISTORY_COUNT)&&(szLoghistory[nTail][0]))
		nHead=(nHead+1)%MLOG_DEFAULT_HISTORY_COUNT;

	*/
}

/*
char *MGetLogHistory(int i)
{
	return NULL;
	MASSERT((i>=0)&&(i<MLOG_DEFAULT_HISTORY_COUNT));
	return szLoghistory[(i+nHead)%MLOG_DEFAULT_HISTORY_COUNT];
}

int MGetLogHistoryCount()
{
	return 0;
	return (nHead<nTail)?(nTail-nHead):MLOG_DEFAULT_HISTORY_COUNT;
}
*/

#ifdef _WIN32
#include <windows.h>
#include <crtdbg.h>

void __cdecl MMsg(const char *pFormat,...)
{
    char buff[256];

    wvsprintf(buff, pFormat, (char *)(&pFormat+1));
    lstrcat(buff, "\r\n");
    MessageBox( NULL, buff, "RealSpace Message", MB_OK );
	mlog(buff);mlog("\n");
}
#endif

////////////////////////////////////////////////////////////////////////////////
// exception handler

void MShowContextRecord(CONTEXT* p)
{
	mlog("[Context]\n");

	mlog("GS : %08x  FS : %08x  ES : %08x  DS : %08x\n", p->SegGs, p->SegFs, p->SegEs, p->SegDs);
	mlog("EDI: %08x  ESI: %08x  EBX: %08x  EDX: %08x\n", p->Edi, p->Esi, p->Ebx, p->Edx);
	mlog("ECX: %08x  EAX: %08x  EBP: %08x  EIP: %08x\n", p->Ecx, p->Eax, p->Ebp, p->Eip);
	mlog("CS : %08x  Flg: %08x  ESP: %08x  SS : %08x\n", p->SegCs, p->EFlags, p->Esp, p->SegSs);

	mlog("\n");
}

void MShowStack(DWORD* sp, int nSize)
{
	mlog("[Stack]");

	for(int i=0; i<nSize; i++){
		if((i%8)==0) mlog("\n");
		mlog("%08x ", *(sp+i));
	}

	mlog("\n");
}

DWORD MFilterException(LPEXCEPTION_POINTERS p)
{
	char tmpbuf[128];
	_strtime( tmpbuf );
	mlog("Crash ( %s )\n",tmpbuf);

	mlog("Build "__DATE__" "__TIME__"\n\n");

	mlog("\n[Exception]\n");
	mlog("Address	:	%08x\n", p->ExceptionRecord->ExceptionAddress);
	mlog("ExpCode	:	%08x\n", p->ExceptionRecord->ExceptionCode);
	mlog("Flags	:	%08x\n", p->ExceptionRecord->ExceptionFlags);
	mlog("#Param	:	%08x\n", p->ExceptionRecord->NumberParameters);
	mlog("other	:	%08x\n", p->ExceptionRecord->ExceptionRecord);
	mlog("\n");
	
	MShowContextRecord(p->ContextRecord);
	MShowStack((DWORD*)p->ContextRecord->Esp, 128);

	mlog("\n");

	string str;

	GetCrashInfo(p,str);

	mlog(str.c_str());
	mlog("\n");

	return EXCEPTION_EXECUTE_HANDLER;
}

void MSEHTranslator(UINT nSeCode, _EXCEPTION_POINTERS* pExcPointers)
{
	MFilterException(pExcPointers);

	raise(SIGABRT);     // raise abort signal 

	// We usually won't get here, but it's possible that
	// SIGABRT was ignored.  So hose the program anyway.
	_exit(3);
}

void MInstallSEH()	// Compile Option에 /EHa 있어야함
{
	#ifndef _DEBUG
		_set_se_translator(MSEHTranslator);
	#endif
}

#ifndef _PUBLISH

#pragma comment(lib, "winmm.lib")

#define MAX_PROFILE_COUNT	10000

struct MPROFILEITEM {
	char szName[256];
	DWORD dwStartTime;
	DWORD dwTotalTime;
	DWORD dwCalledCount;
};
MPROFILEITEM g_ProfileItems[MAX_PROFILE_COUNT];

DWORD g_dwEnableTime;

void MInitProfile()
{
	for(int i=0;i<MAX_PROFILE_COUNT;i++)
	{
		g_ProfileItems[i].szName[0]=0;
		g_ProfileItems[i].dwTotalTime=0;
		g_ProfileItems[i].dwCalledCount=0;
	}
	g_dwEnableTime=timeGetTime();
}

void MBeginProfile(int nIndex,const char *szName)
{
	if(g_ProfileItems[nIndex].szName[0]==0)
		strcpy(g_ProfileItems[nIndex].szName,szName);

	g_ProfileItems[nIndex].dwStartTime=timeGetTime();
	g_ProfileItems[nIndex].dwCalledCount++;
}

void MEndProfile(int nIndex)
{
	g_ProfileItems[nIndex].dwTotalTime+= 
		timeGetTime()-g_ProfileItems[nIndex].dwStartTime;
}

void MSaveProfile(const char *filename)
{
	DWORD dwTotalTime = timeGetTime()-g_dwEnableTime;

	FILE *file=fopen(filename,"w+");

	fprintf(file," total time = %6.3f seconds \n",(float)dwTotalTime*0.001f);

	fprintf(file,"id   (loop ms)  seconds     %%        calledcount   name \n");
	fprintf(file,"=========================================================\n");

	float cnt = (float)g_ProfileItems[0].dwCalledCount;

	for(int i=0;i<MAX_PROFILE_COUNT;i++)
	{
		if(g_ProfileItems[i].dwTotalTime>0)
		{
			fprintf(file,"(%05d) %8.3f %8.3f ( %6.3f %% , %6u) %s \n",i,((float)g_ProfileItems[i].dwTotalTime) / cnt,
				0.001f*(float)g_ProfileItems[i].dwTotalTime, 
				100.f*(float)g_ProfileItems[i].dwTotalTime/(float)dwTotalTime
				,g_ProfileItems[i].dwCalledCount
				,g_ProfileItems[i].szName);
		}
	}
	fclose(file);
}

#else
void MInitProfile() {}
void MBeginProfile(int nIndex,const char *szName) {}
void MEndProfile(int nIndex) {}
void MSaveProfile(const char *file) {}

#endif