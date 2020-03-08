#include "StdAfx.h"
#include "MDebug.h"
#include "d3d9.h"

void MSysInfoLog_CPU()
{
#if defined(MSVC_VER) && defined(ENV32BIT)
	LARGE_INTEGER ulFreq, ulTicks, ulValue, ulStartCounter,
		ulEAX_EDX, ulResult;

	// it is number of ticks per seconds
	QueryPerformanceFrequency(&ulFreq);
	// current value of the performance counter

	QueryPerformanceCounter(&ulTicks);
	// calculate one-second interval
	ulValue.QuadPart = ulTicks.QuadPart + ulFreq.QuadPart;
	// read time stamp counter
	// this asm instruction load the highorder 32 bit of the
	// register into EDX and the lower order 32 bits into EAX
	_asm
	{
		rdtsc
		mov ulEAX_EDX.LowPart, EAX
		mov ulEAX_EDX.HighPart, EDX
	}

	// start number of ticks
	ulStartCounter.QuadPart = ulEAX_EDX.QuadPart;
	// loop for 1 second
	/*do
	{
		QueryPerformanceCounter(&ulTicks);
	} while (ulTicks.QuadPart <= ulValue.QuadPart);*/

	// get the actual number of ticks
	_asm
	{
		rdtsc
		mov ulEAX_EDX.LowPart, EAX
		mov ulEAX_EDX.HighPart, EDX
	}

	// calculate result
	ulResult.QuadPart = ulEAX_EDX.QuadPart
		- ulStartCounter.QuadPart;
	int nCPUClock = int(ulResult.QuadPart / 1000000);


	DWORD nCPUFamily,nCPUModel,nCPUStepping;
	static char pszCPUType[13];
	memset(pszCPUType, 0, 13);
	_asm
	{
		mov eax, 0
		cpuid

		// getting information from EBX
		mov pszCPUType[0], bl
		mov pszCPUType[1], bh
		ror ebx, 16
		mov pszCPUType[2], bl
		mov pszCPUType[3], bh

		// getting information from EDX
		mov pszCPUType[4], dl
		mov pszCPUType[5], dh
		ror edx, 16
		mov pszCPUType[6], dl
		mov pszCPUType[7], dh

		// getting information from ECX
		mov pszCPUType[8], cl
		mov pszCPUType[9], ch
		ror ecx, 16
		mov pszCPUType[10], cl
		mov pszCPUType[11], ch

		mov eax, 1
		cpuid

		mov nCPUFamily, eax
		mov nCPUModel, eax
		mov nCPUStepping, eax

	}
	pszCPUType[12] = '\0';

	nCPUFamily = (nCPUFamily >> 8);
	nCPUModel =  ((nCPUModel >> 4 ) & 0x0000000f);
	nCPUStepping = (nCPUStepping & 0x0000000f);

	char szDesc[512]="";
	sprintf_safe(szDesc, "CPU ID = %s ( family = %d , model = %d , stepping = %d ) @ %d MHz\n",
		pszCPUType,nCPUFamily,nCPUModel,nCPUStepping,nCPUClock);
	mlog(szDesc);
#endif
}

void MSysInfoLog_Display()
{
	HMODULE					hD3DLibrary=NULL;
	LPDIRECT3D9				pD3D=NULL;
	LPDIRECT3DDEVICE9		pd3dDevice=NULL;
	D3DADAPTER_IDENTIFIER9	deviceID;

	hD3DLibrary = LoadLibrary( "d3d9.dll" );

	if (!hD3DLibrary)
	{
		mlog("Error, could not load d3d9.dll");
		return;
	}

	typedef IDirect3D9 * (__stdcall *D3DCREATETYPE)(UINT);
	D3DCREATETYPE d3dCreate = (D3DCREATETYPE) GetProcAddress(hD3DLibrary, "Direct3DCreate9");

	if (!d3dCreate)
	{
		mlog("Error, could not get proc adress of Direct3DCreate9.");
		FreeLibrary(hD3DLibrary);
		return;
	}

	//just like pID3D = Direct3DCreate9(D3D_SDK_VERSION);
	pD3D = (*d3dCreate)(D3D_SDK_VERSION);
	if (!pD3D)
	{
		mlog("Error initializing D3D.");
		FreeLibrary(hD3DLibrary);
		return;
	}

	pD3D->GetAdapterIdentifier(0,0,&deviceID);
	pD3D->Release();
	FreeLibrary(hD3DLibrary);

#ifndef _PUBLISH
	mlog("D3D_SDK_VERSION = %d \n", D3D_SDK_VERSION);
#endif

	mlog("Display Device = %s ( vendor=%x device=%x subsys=%x revision=%x )\n", 
		deviceID.Description,deviceID.VendorId,deviceID.
		DeviceId,deviceID.SubSysId,deviceID.Revision);

	mlog("Display Driver Version = %d.%d.%04d.%04d\n",
		deviceID.DriverVersion.HighPart >> 16 , deviceID.DriverVersion.HighPart & 0xffff,
		deviceID.DriverVersion.LowPart >> 16 , deviceID.DriverVersion.LowPart & 0xffff );
}

void MSysInfoLog_OS()
{
	OSVERSIONINFO os;
	os.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
#pragma warning(push)
#pragma warning(disable: 4996)
	GetVersionEx(&os);
#pragma warning(pop)

	MEMORYSTATUS ms;
	ms.dwLength = sizeof(MEMORYSTATUS);
	::GlobalMemoryStatus(&ms);
	DWORD dwPhysicalMemory;
	dwPhysicalMemory=ms.dwTotalPhys;

	mlog("Windows = %d.%d Build %d , %s (%dKB) : ", os.dwMajorVersion, os.dwMinorVersion,
		os.dwBuildNumber, os.szCSDVersion, (int)(dwPhysicalMemory/1024));

	char szDesc[512];

	if(os.dwMajorVersion==5) {
		if(os.dwMinorVersion==0)		sprintf_safe(szDesc," Windows 2000..\n");
		else if(os.dwMinorVersion==1)	sprintf_safe(szDesc," Windows xp..\n");
		else if(os.dwMinorVersion==2)	sprintf_safe(szDesc," Windows 2003..\n");
		else							sprintf_safe(szDesc," ..\n");
	}
	else if(os.dwMajorVersion==4) {
		if(os.dwMinorVersion==0)		sprintf_safe(szDesc," Windows 95..\n");
		else if(os.dwMinorVersion==10)	sprintf_safe(szDesc," Windows 98..\n");
		else if(os.dwMinorVersion==90)	sprintf_safe(szDesc," Windows Me..\n");
		else							sprintf_safe(szDesc," ..\n");
	}
	else if(os.dwMajorVersion==3) {
		if(os.dwMinorVersion==51)		sprintf_safe(szDesc," Windows NT 3.51..\n");
		else							sprintf_safe(szDesc," ..\n");
	}
	else	{
		sprintf_safe(szDesc," ..\n");
	}
	mlog(szDesc);
}

void MSysInfoLog()
{
	MSysInfoLog_CPU();
	MSysInfoLog_Display();
	MSysInfoLog_OS();
}