#include "stdafx.h"

#include "MDebug.h"
#include "ZDirectInput.h"
#include <crtdbg.h>
#include "MUtil.h"

#pragma comment(lib,"dinput8.lib")

#define SAMPLE_BUFFER_SIZE 32  // arbitrary number of buffer elements

#undef _DONOTUSE_DINPUT_MOUSE

ZDirectInput::ZDirectInput(void) : m_nJoyButtons(0), m_nJoyPovs(0), m_bForceFeedback(false)
{
	m_bInitialized = FALSE;
	m_pDI = NULL;
	m_pKeyboard = NULL;
	m_pMouse = NULL;
	m_pJoystick = NULL;
	m_bImmediateMode = TRUE;

	for(int i=0; i<KEYNAMETABLE_COUNT; i++){
		m_szKeyNameTable[i] = NULL;
	}
	m_hD3DLibrary = NULL;
}

ZDirectInput::~ZDirectInput(void)
{
	Destroy();
}

// enum keyboard buttons
BOOL CALLBACK EnumDeviceObjectsCB( LPCDIDEVICEOBJECTINSTANCE lpddoi, LPVOID pvRef )
{
    // Extract the passed pointer
	char** szKeyNameTable = (char**)pvRef;

	int nKey = lpddoi->dwOfs;
	if(nKey<0 || nKey>=KEYNAMETABLE_COUNT) return DIENUM_STOP;

	szKeyNameTable[nKey] = new char[strlen(lpddoi->tszName)+2];
	strcpy_unsafe(szKeyNameTable[nKey], lpddoi->tszName);

	return DIENUM_CONTINUE;
}

// enum joystick objects
BOOL CALLBACK ZDirectInput::EnumJoyObjectsCallback( const DIDEVICEOBJECTINSTANCE* pdidoi, VOID* pContext )
{
	ZDirectInput* pDI = (ZDirectInput*)pContext;

	if( (pdidoi->dwFlags & DIDOI_FFACTUATOR) != 0 )
		pDI->m_nFFAxis++;

	if( pdidoi->dwType & DIDFT_AXIS )
	{
		DIPROPRANGE diprg; 
		diprg.diph.dwSize       = sizeof(DIPROPRANGE); 
		diprg.diph.dwHeaderSize = sizeof(DIPROPHEADER); 
		diprg.diph.dwHow        = DIPH_BYID; 
		diprg.diph.dwObj        = pdidoi->dwType; // Specify the enumerated axis
		diprg.lMin              = -1000; 
		diprg.lMax              = +1000; 

		// Set the range for the axis
		if( FAILED( pDI->m_pJoystick->SetProperty( DIPROP_RANGE, &diprg.diph ) ) ) 
			return DIENUM_STOP;

	}
	return DIENUM_CONTINUE;
}



BOOL CALLBACK ZDirectInput::EnumJoysticksCallback( const DIDEVICEINSTANCE* pdidInstance, VOID* pContext )
{
	ZDirectInput* pDInput = (ZDirectInput*)pContext;
	
	HRESULT hr;

	// Obtain an interface to the enumerated Joystick.
	hr = pDInput->m_pDI->CreateDevice( pdidInstance->guidInstance, &pDInput->m_pJoystick, NULL );

	// If it failed, then we can't use this Joystick. (Maybe the user unplugged
	// it while we were in the middle of enumerating it.)
	if( FAILED(hr) ) 
		return DIENUM_CONTINUE;

	DIDEVCAPS caps;
	caps.dwSize = sizeof(DIDEVCAPS);

	hr = pDInput->m_pJoystick->GetCapabilities(&caps);
	if(SUCCEEDED(hr))
	{
		pDInput->m_nJoyButtons	= caps.dwButtons;
		pDInput->m_nJoyPovs		= caps.dwPOVs;
		pDInput->m_bForceFeedback = (caps.dwFlags & DIDC_FORCEFEEDBACK)!=0;
	}

	// Stop enumeration. Note: we're just taking the first Joystick we get. You
	// could store all the enumerated Joysticks and let the user pick.
	return DIENUM_STOP;
}



bool ZDirectInput::CreateDirectInput()
{
	_ASSERT(!m_pDI);
	m_hD3DLibrary = LoadLibrary( "dinput8.dll" );

	if (!m_hD3DLibrary)
	{
		mlog("Error, could not load dinput8.dll");
		return false;
	}

	typedef HRESULT (__stdcall *DINPUTCREATETYPE)(HINSTANCE,DWORD,REFIID,LPVOID*,LPUNKNOWN);
	DINPUTCREATETYPE dinputCreate = (DINPUTCREATETYPE) GetProcAddress(m_hD3DLibrary, "DirectInput8Create");

	if (!dinputCreate)
	{
		mlog("Error, could not get proc adress of DirectInput8Create.");
		return false;
	}

	HRESULT hr = (*dinputCreate)(GetModuleHandle(NULL), DIRECTINPUT_VERSION, IID_IDirectInput8, (VOID**)&m_pDI, NULL ) ;

	if(FAILED(hr)) return false;

	return true;
}

bool ZDirectInput::Create(HWND hWnd, BOOL bExclusive, BOOL bImmediateMode)
{
	// Init /////////////
	m_bInitialized = FALSE;
	m_pDI = NULL;
	m_pKeyboard = NULL;
	m_pMouse = NULL;
	m_bImmediateMode = TRUE;

	for(int i=0; i<KEYNAMETABLE_COUNT; i++){
		m_szKeyNameTable[i] = NULL;
	}
	/////////////////////

	m_bImmediateMode = bImmediateMode;

    HRESULT hr;
    BOOL    bForeground = TRUE;
    BOOL    bDisableWindowsKey = FALSE;
    DWORD   dwCoopFlags;

    if( bExclusive ) dwCoopFlags = DISCL_EXCLUSIVE;
    else dwCoopFlags = DISCL_NONEXCLUSIVE;

    if( bForeground ) dwCoopFlags |= DISCL_FOREGROUND;
    else dwCoopFlags |= DISCL_BACKGROUND;

    if( bDisableWindowsKey && !bExclusive && bForeground ) dwCoopFlags |= DISCL_NOWINKEY;

#ifdef _PUBLISH
	dwCoopFlags |= DISCL_NOWINKEY ;
#endif

	if(!CreateDirectInput()) return false;
//    if( FAILED( hr = DirectInput8Create( GetModuleHandle(NULL), DIRECTINPUT_VERSION, IID_IDirectInput8, (VOID**)&m_pDI, NULL ) ) ) return false;


	// 키보드 device 를 만든다

    if( FAILED( hr = m_pDI->CreateDevice( GUID_SysKeyboard, &m_pKeyboard, NULL ) ) ) return false;
    
    if( FAILED( hr = m_pKeyboard->SetDataFormat( &c_dfDIKeyboard ) ) ) return false;

	hr = m_pKeyboard->SetCooperativeLevel( hWnd, dwCoopFlags );
    if( hr == DIERR_UNSUPPORTED && !bForeground && bExclusive )
    {
        Destroy();
		return false;
    }

    if( FAILED(hr) ) return false;

    if( !m_bImmediateMode )
    {
        DIPROPDWORD dipdw;

        dipdw.diph.dwSize       = sizeof(DIPROPDWORD);
        dipdw.diph.dwHeaderSize = sizeof(DIPROPHEADER);
        dipdw.diph.dwObj        = 0;
        dipdw.diph.dwHow        = DIPH_DEVICE;
        dipdw.dwData            = SAMPLE_BUFFER_SIZE; // Arbitary buffer size

        if( FAILED( hr = m_pKeyboard->SetProperty( DIPROP_BUFFERSIZE, &dipdw.diph ) ) ) return false;
    }

    m_pKeyboard->Acquire();

	// Enum Buttons
    hr = m_pKeyboard->EnumObjects( EnumDeviceObjectsCB, m_szKeyNameTable, DIDFT_BUTTON );
    if( FAILED(hr)) return false;


#ifndef _DONOTUSE_DINPUT_MOUSE

	// 마우스 device 를 만든다
	if( FAILED( hr = m_pDI->CreateDevice( GUID_SysMouse, &m_pMouse, NULL ) ) ) return false;

	if( FAILED( hr = m_pMouse->SetDataFormat( &c_dfDIMouse2 ) ) ) return false;

	hr = m_pMouse->SetCooperativeLevel( hWnd, dwCoopFlags );
	if( hr == DIERR_UNSUPPORTED && !bForeground && bExclusive )
	{
		Destroy();
		return false;
	}

	if( FAILED(hr) ) return false;

	const bool bMouseImmediate = true;

	if( !bMouseImmediate )
	{
		DIPROPDWORD dipdw;
		dipdw.diph.dwSize       = sizeof(DIPROPDWORD);
		dipdw.diph.dwHeaderSize = sizeof(DIPROPHEADER);
		dipdw.diph.dwObj        = 0;
		dipdw.diph.dwHow        = DIPH_DEVICE;
		dipdw.dwData            = SAMPLE_BUFFER_SIZE; // Arbitary buffer size

		if( FAILED( hr = m_pMouse->SetProperty( DIPROP_BUFFERSIZE, &dipdw.diph ) ) )
			return false;
	}

	m_pMouse->Acquire();
#endif

	// 조이스틱 디바이스
	// Look for a simple Joystick we can use for this sample program.
	if( FAILED( hr = m_pDI->EnumDevices( DI8DEVCLASS_GAMECTRL, 
		EnumJoysticksCallback,
		this, DIEDFL_ATTACHEDONLY ) ) )
		return false;

	if(m_pJoystick)
	{
		if( FAILED( hr = m_pJoystick->SetDataFormat( &c_dfDIJoystick2 ) ) )
			return false;

		// Set the cooperative level to let DInput know how this device should
		// interact with the system and with other DInput applications.
		if( FAILED( hr = m_pJoystick->SetCooperativeLevel( hWnd, DISCL_EXCLUSIVE | 
			DISCL_FOREGROUND ) ) )
			return false;

		if( FAILED( hr = m_pJoystick->EnumObjects( EnumJoyObjectsCallback, 
			(VOID*)this, DIDFT_ALL ) ) )
			return false;
	}

	if(m_nFFAxis>2)
		m_nFFAxis = 2;

	// 포스피드백 이펙트 초기화. 실패하면 disable 시키고 그대로 진행한다
	if(m_bForceFeedback && m_nFFAxis>0)
	{
		// This application needs only one effect: Applying raw forces.
		DWORD           rgdwAxes[2]     = { DIJOFS_X, DIJOFS_Y };
		LONG            rglDirection[2] = { 0, 0 };
		DICONSTANTFORCE cf              = { 0 };

		DIEFFECT eff;
		ZeroMemory( &eff, sizeof(eff) );
		eff.dwSize                  = sizeof(DIEFFECT);
		eff.dwFlags                 = DIEFF_CARTESIAN | DIEFF_OBJECTOFFSETS;
		eff.dwDuration              = INFINITE;
		eff.dwSamplePeriod          = 0;
		eff.dwGain                  = DI_FFNOMINALMAX;
		eff.dwTriggerButton         = DIEB_NOTRIGGER;
		eff.dwTriggerRepeatInterval = 0;
		eff.cAxes                   = m_nFFAxis;
		eff.rgdwAxes                = rgdwAxes;
		eff.rglDirection            = rglDirection;
		eff.lpEnvelope              = 0;
		eff.cbTypeSpecificParams    = sizeof(DICONSTANTFORCE);
		eff.lpvTypeSpecificParams   = &cf;
		eff.dwStartDelay            = 0;

		// Create the prepared effect
		if( FAILED( hr = m_pJoystick->CreateEffect( GUID_ConstantForce, 
			&eff, &m_pFFEffect, NULL ) ) )
		{
			m_bForceFeedback = false;
		}

		if(m_pFFEffect)
			m_pFFEffect->Start( 1, 0 ); // Start the effect
		else
			m_bForceFeedback = false;
	}

	m_bInitialized = true;

    return true;
}

void ZDirectInput::Destroy(void)
{
	m_bInitialized = false;

	if( m_pMouse ) m_pMouse->Unacquire();
	SAFE_RELEASE( m_pMouse );

    if( m_pKeyboard ) m_pKeyboard->Unacquire();
    SAFE_RELEASE( m_pKeyboard );

	if( m_pJoystick	) m_pJoystick->Unacquire();
	SAFE_RELEASE( m_pJoystick );
	SAFE_RELEASE( m_pFFEffect );

	SAFE_RELEASE( m_pDI );


	FreeLibrary(m_hD3DLibrary);
	m_hD3DLibrary = NULL;

	for(int i=0; i<KEYNAMETABLE_COUNT; i++){
		if(m_szKeyNameTable[i]!=NULL){
			delete[] m_szKeyNameTable[i];
			m_szKeyNameTable[i] = NULL;
		}
	}
}

// 사용하지 않는다
/*
bool ZDirectInput::GetImmediateData(BYTE ScanCode[256])
{
    HRESULT hr;

    if( NULL == m_pKeyboard ) return false;
	if(m_bImmediateMode==FALSE){
		_ASSERT(FALSE);	// Immediat Mode Only
		return false;
	}
    
    ZeroMemory( ScanCode, sizeof(ScanCode) );

	hr = m_pKeyboard->GetDeviceState( sizeof(ScanCode), ScanCode );
    if( FAILED(hr) ) {
        hr = m_pKeyboard->Acquire();
        while( hr == DIERR_INPUTLOST ) hr = m_pKeyboard->Acquire();
		return false;
    }
    
	return true;
}
*/

/*
bool ZDirectInput::GetImmediateData(DIMOUSESTATE2 *pdims2)
{
	if( NULL == m_pMouse ) return false;

	HRESULT hr;
	ZeroMemory( pdims2, sizeof(DIMOUSESTATE2) );
	hr = m_pMouse->GetDeviceState( sizeof(DIMOUSESTATE2), pdims2 );
	if( FAILED(hr) ) 
	{
		hr = m_pMouse->Acquire();
		while( hr == DIERR_INPUTLOST ) 
			hr = m_pMouse->Acquire();
		return false;
	}
	return true;
}
*/

DWORD ZDirectInput::GetKeyboardBufferedData(ZDIBUFFER* pBuffer,unsigned int nBuffer)
{
    DIDEVICEOBJECTDATA didod[SAMPLE_BUFFER_SIZE];  // Receives buffered data 
    DWORD              dwElements;
    DWORD              i;
    HRESULT            hr;

    if( NULL == m_pKeyboard ) return 0;
    
	if(m_bImmediateMode==TRUE){
		_ASSERT(FALSE);	// Buffered Mode Only
		return 0;
	}

	dwElements = SAMPLE_BUFFER_SIZE;
    hr = m_pKeyboard->GetDeviceData( sizeof(DIDEVICEOBJECTDATA), didod, &dwElements, 0 );
    if( hr != DI_OK ) {
        hr = m_pKeyboard->Acquire();
        while( hr == DIERR_INPUTLOST ) 
			hr = m_pKeyboard->Acquire();
		return 0;
    }

	for (i = 0; i < min(u32(dwElements), nBuffer); i++) {
		pBuffer[i].nKey = BYTE(didod[i].dwOfs & 0xFF);
		pBuffer[i].bPressed = (didod[i].dwData & 0x80)?true:false;
    }
    return dwElements;
}

DWORD ZDirectInput::GetMouseBufferedData(int* pSumX,int* pSumY, ZDIBUFFER* pBuffer,unsigned int nBuffer)
{
	*pSumX = 0;
	*pSumY = 0;

	if( NULL == m_pMouse ) return 0;

	DIMOUSESTATE2 dims2{};
	auto hr = m_pMouse->GetDeviceState(sizeof(dims2), &dims2);

	if (FAILED(hr))
	{
		hr = m_pMouse->Acquire();
		while (hr == DIERR_INPUTLOST)
			hr = m_pMouse->Acquire();
		return 0;
	}

	int nCount = 0;

	// wheel
	if (dims2.lZ != 0)
	{
		int nButton = (dims2.lZ) > 0 ? 0 : 1;
		pBuffer[nCount].bPressed = true;
		pBuffer[nCount].nKey = nButton;
		nCount++;
		pBuffer[nCount].bPressed = false;
		pBuffer[nCount].nKey = nButton;
		nCount++;
	}

	for (int i = 0; i<8; i++)
	{
		bool bPressed = (dims2.rgbButtons[i] & 0x80) ? true : false;
		if (m_bMouseButtonStates[i] != bPressed)
		{
			m_bMouseButtonStates[i] = bPressed;
			pBuffer[nCount].bPressed = bPressed;
			pBuffer[nCount].nKey = i + 2;
			nCount++;
		}
	}

	int nSumX = dims2.lX;
	int nSumY = dims2.lY;

	*pSumX = nSumX;
	*pSumY = nSumY;
	return nCount;
}

const char* ZDirectInput::GetKeyName(u32 nKey)
{
	if(nKey<0 || nKey>=KEYNAMETABLE_COUNT){
		static char* szUnknownKeyName = "N/A";
		return szUnknownKeyName;
	}
	return m_szKeyNameTable[nKey];
}


bool ZDirectInput::GetJoystickData(DIJOYSTATE2* pjs)
{
//	DIJOYSTATE2 js;           // DInput Joystick state 

	if( NULL == m_pJoystick ) return false;

	// Poll the device to read the current state
	HRESULT hr = m_pJoystick->Poll(); 
	if( FAILED(hr) )  
	{
		// DInput is telling us that the input stream has been
		// interrupted. We aren't tracking any state between polls, so
		// we don't have any special reset that needs to be done. We
		// just re-acquire and try again.
		hr = m_pJoystick->Acquire();
		while( hr == DIERR_INPUTLOST ) 
			hr = m_pJoystick->Acquire();	// 위험한디..

		// hr may be DIERR_OTHERAPPHASPRIO or other errors.  This
		// may occur when the app is minimized or in the process of 
		// switching, so just try again later 
		return false; 
	}

	// Get the input's device state
	if( FAILED( hr = m_pJoystick->GetDeviceState( sizeof(DIJOYSTATE2), pjs ) ) )
		return false; // The device should have been acquired during the Poll()

	return true;
}

void ZDirectInput::OnActivate(bool bActive)
{
	if(bActive)
	{
		m_pKeyboard->Acquire();
		m_pMouse->Acquire();
		m_pJoystick->Acquire();

		if( m_pFFEffect ) 
			m_pFFEffect->Start( 1, 0 ); // Start the effect
	}
}


bool ZDirectInput::SetDeviceForcesXY(int nXForce, int nYForce)
{
	if(!m_bForceFeedback || NULL==m_pFFEffect) return false;

	// Modifying an effect is basically the same as creating a new one, except
	// you need only specify the parameters you are modifying
	LONG rglDirection[2] = { 0, 0 };

	DICONSTANTFORCE cf;

	if( m_nFFAxis == 1 )
	{
		// If only one force feedback axis, then apply only one direction and 
		// keep the direction at zero
		cf.lMagnitude = nXForce;
		rglDirection[0] = 0;
	}
	else
	{
		// If two force feedback axis, then apply magnitude from both directions 
		rglDirection[0] = nXForce;
		rglDirection[1] = nYForce;
		cf.lMagnitude = (DWORD)sqrt( (double)nXForce * (double)nXForce +
			(double)nYForce * (double)nYForce );
	}

	DIEFFECT eff;
	ZeroMemory( &eff, sizeof(eff) );
	eff.dwSize                = sizeof(DIEFFECT);
	eff.dwFlags               = DIEFF_CARTESIAN | DIEFF_OBJECTOFFSETS;
	eff.cAxes                 = m_nFFAxis;
	eff.rglDirection          = rglDirection;
	eff.lpEnvelope            = 0;
	eff.cbTypeSpecificParams  = sizeof(DICONSTANTFORCE);
	eff.lpvTypeSpecificParams = &cf;
	eff.dwStartDelay            = 0;

	// Now set the new parameters and start the effect immediately.
	HRESULT hr = m_pFFEffect->SetParameters( &eff, DIEP_DIRECTION |
		DIEP_TYPESPECIFICPARAMS |
		DIEP_START );

	return SUCCEEDED(hr);
}
