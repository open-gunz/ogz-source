/*
	RealSound.cpp
	--------

	RealSound Main Module
	RealSound Object는 RealSound Class Library의 최상위 객체로서
	Manager역할을 담당하게 된다.

	Programming by Chojoongpil
	All copyright 1997 (c), MAIET entertainment sotware
*/
#include "stdafx.h"
#include <crtdbg.h>
#include "RealSound.h"
#include "MDebug.h"
#include "MWindows.h"
#include <dsound.h>

#ifdef _DEBUG
	#define _D		::OutputDebugString
#else 
	#define _D		
#endif

/////////////////////////////////////////
// RealSound Class Implement

/////////////////////////////////////////
// Constructor & Destructor


ENUMDEVICEINFO RealSound::m_EnumDeviceInfos[MAX_ENUM_DEVICE_COUNT];
int RealSound::m_nEnumDeviceCount = 0;

RealSound::RealSound()
{
	m_lpDS = NULL;
	m_lpDSListener = NULL;
	m_hOwnerWnd = NULL;
}

RealSound::~RealSound()
{
	Destroy();
}

/////////////////////////////////////////
// Member Function Implement

void RealSound::InitEnumInfo(void)
{
	for(int i=0; i<MAX_ENUM_DEVICE_COUNT; i++){
		m_EnumDeviceInfos[i].lpGUID = NULL;
		m_EnumDeviceInfos[i].szDescription[0] = 0;
	}
	m_nEnumDeviceCount = 0;
}


BOOL CALLBACK RealSound::EnumProc(LPGUID lpGUID, LPCTSTR lpszDesc, LPCTSTR lpszDrvName, LPVOID lpContext)
{
 
	m_EnumDeviceInfos[m_nEnumDeviceCount].lpGUID = lpGUID;
	strcpy_safe(m_EnumDeviceInfos[m_nEnumDeviceCount].szDescription, lpszDesc);
	m_nEnumDeviceCount++;

    return TRUE;
}

bool RealSound::Enumerate(void)
{
	if FAILED(DirectSoundEnumerate((LPDSENUMCALLBACK)EnumProc, NULL))
		return false;
	return true;
}

int RealSound::GetEnumDeviceCount(void)
{
	return m_nEnumDeviceCount;
}

GUID* RealSound::GetEnumDeviceGUID(int i)
{
	if(i>=m_nEnumDeviceCount || i<0) return NULL;
	return m_EnumDeviceInfos[i].lpGUID;
}

char* RealSound::GetEnumDeviceDescription(int i)
{
	if(i>=m_nEnumDeviceCount || i<0) return NULL;
	return m_EnumDeviceInfos[i].szDescription;
}

void RealSound::Destroy()
{
	if( m_lpDSListener ){
		m_lpDSListener->Release();
		m_lpDSListener = NULL;
	}
	if( m_lpDS ){
		m_lpDS->Release();
		m_lpDS = NULL;
	}
}

#define DEFAULT_ROLLOFF_FACTOR			10.0f


bool RealSound::Create( HWND hWnd, LPGUID lpGUID )
{	
	if( !hWnd ){
		_D("RealSound::Create error : hWnd is NULL\n");
		return FALSE;
	}
	if( m_lpDS ){
		_D("RealSound::Create error : DirectSound can initialize once.\n");
		return FALSE;
	}
	
	m_hOwnerWnd = hWnd;

	if( DirectSoundCreate8( lpGUID, &m_lpDS, NULL ) != DS_OK ){
		_D("RealSound::Create error : DirectSoundCreate Error");
		return false;
	}
	
	if( m_lpDS->SetCooperativeLevel( m_hOwnerWnd, DSSCL_PRIORITY ) != DS_OK )
	{
		if( m_lpDS->SetCooperativeLevel( m_hOwnerWnd, DSSCL_NORMAL ) != DS_OK )
		{
			_D("RealSound::Create error : SetCooperativeLevel Error");
			return false;	
		}
	}
	
	LPDIRECTSOUNDBUFFER pDSBPrimary;
	DSBUFFERDESC dsbdesc;
	ZeroMemory(&dsbdesc, sizeof(DSBUFFERDESC));
	dsbdesc.dwSize = sizeof(DSBUFFERDESC);
	// 믹싱할때 소리 왜곡이 생겨서 DSBCAPS_LOCSOFTWARE로 우선 처리
	dsbdesc.dwFlags = DSBCAPS_CTRL3D | DSBCAPS_CTRLVOLUME |  DSBCAPS_PRIMARYBUFFER | DSBCAPS_LOCSOFTWARE;// | DSBCAPS_CTRLFX;
	if( FAILED( m_lpDS->CreateSoundBuffer( &dsbdesc, &pDSBPrimary, NULL ) ) )
	{
		mlog( "RealSound Primary Buffer Fail..\n");
		return false;
	}
	

	// Default Setting 16Bit, 44KHz
	WAVEFORMATEX wfm;
	memset(&wfm, 0, sizeof(WAVEFORMATEX));
	wfm.wFormatTag = WAVE_FORMAT_PCM;
	wfm.nChannels = 2;
	wfm.nSamplesPerSec = 44100;
	wfm.wBitsPerSample = 16;
	wfm.nBlockAlign = wfm.wBitsPerSample / 8 * wfm.nChannels;
	wfm.nAvgBytesPerSec = wfm.nSamplesPerSec * wfm.nBlockAlign;
	wfm.cbSize = 0;
	pDSBPrimary->SetFormat(&wfm);

	if( FAILED( pDSBPrimary->QueryInterface( IID_IDirectSound3DListener8, (VOID**)&m_lpDSListener ) ) ){
		return false;
	}

	pDSBPrimary->Play(0, 0, DSBPLAY_LOOPING);

	pDSBPrimary->Release();
	return true;
}


void RealSound::SetListenerPosition(float x, float y, float z)
{
	HRESULT hr = m_lpDSListener->SetPosition(x, y, z, DS3D_IMMEDIATE);
}

void RealSound::SetListenerOrientation(float dirx, float diry, float dirz, float upx, float upy, float upz)
{
	m_lpDSListener->SetOrientation(dirx, diry, dirz, upx, upy, upz, DS3D_IMMEDIATE);
}

void RealSound::SetRolloffFactor(float t)
{
	if( t < DS3D_MINROLLOFFFACTOR )
		t = DS3D_MINROLLOFFFACTOR;
	else if( t > DS3D_MAXROLLOFFFACTOR )
		t = DS3D_MAXROLLOFFFACTOR;
	m_lpDSListener->SetRolloffFactor(t, DS3D_IMMEDIATE);
}

void RealSound::SetDistanceFactor(float t)
{
	m_lpDSListener->SetDistanceFactor(t, DS3D_IMMEDIATE);
}

void RealSound::SetDopplerFactor(float t)
{
	m_lpDSListener->SetDopplerFactor(t, DS3D_IMMEDIATE);
}

void RealSound::CommitDeferredSettings()	
{
	if(FAILED(m_lpDSListener->CommitDeferredSettings()))
		mlog("Failed to Calculate Listener's Position\n");
}	
