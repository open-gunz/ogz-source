/*
	RealSound.h
	------

	RealSound is Wave Generating Engine 2.0 Version ( WaGe ).
	All copyright 1997 (c), MAIET entertainment software
*/
#ifndef __RealSound_HEADER__
#define __RealSound_HEADER__

#include "MUtil.h"
#include <math.h>
#include <dsound.h>

#pragma comment( lib, "dsound.lib" )

class SEObject;

struct ENUMDEVICEINFO{
	LPGUID	lpGUID;
	char	szDescription[256];
};

#define MAX_ENUM_DEVICE_COUNT	10
class RealSound {
protected:
	struct HWND__* m_hOwnerWnd;
	LPDIRECTSOUND8				m_lpDS;					// Direct Sound
	LPDIRECTSOUND3DLISTENER8	m_lpDSListener;			// 3D Listener

	static ENUMDEVICEINFO	m_EnumDeviceInfos[MAX_ENUM_DEVICE_COUNT];
	static int				m_nEnumDeviceCount;

protected:
	static void InitEnumInfo(void);
	static int STDCALL EnumProc(LPGUID lpGUID, LPCTSTR lpszDesc,
		LPCTSTR lpszDrvName, LPVOID lpContext);

public:
	RealSound();
	virtual ~RealSound();

	static bool Enumerate(void);
	static int GetEnumDeviceCount(void);
	static GUID* RealSound::GetEnumDeviceGUID(int i);
	static char* RealSound::GetEnumDeviceDescription(int i);

	bool Create( HWND hWnd, LPGUID lpGUID=NULL );
	void Destroy();

	LPDIRECTSOUND GetDS() { return m_lpDS; }

	bool IsValid() { return m_lpDS != nullptr; }

	void SetListenerPosition(float x, float y, float z);
	void SetListenerOrientation(float dirx, float diry, float dirz, float upx, float upy, float upz);
	void CommitDeferredSettings();

	void SetRolloffFactor(float t);
	void SetDistanceFactor(float t);
	void SetDopplerFactor(float t);
};

#define DSBVOLUME_MIN               -10000
#define DSBVOLUME_MAX               0

static int LinearToLogVol(double fLevel)
{
	// Clamp the value
	if(fLevel <= 0.0f)
		return DSBVOLUME_MIN;
	else if(fLevel >= 1.0f)
		return 0;
    return (long) (-2000.0 * log10(1.0f / fLevel));
}

static float LogToLinearVol(int iLevel)
{
	// Clamp the value
	if(iLevel <= -9600)
		return 0.0f;
	else if(iLevel >= 0)
		return 1.0f;
    return (float)pow(double(10), double(iLevel + 2000) / 2000.0f) / 10.0f;
}

// use a linear scale from 0.0 (silence) to 1.0 (full volume)
static int VolumeToDecibels(float vol) 
{
	if (vol>=1.0F) 
		return 0;
	if (vol<=0.0F) 
		return DSBVOLUME_MIN;
	static const float adj=3321.928094887F;  // 1000/log10(2)
	return int(float(log10(vol)) * adj);
}

#endif