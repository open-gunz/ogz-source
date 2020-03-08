#pragma once

#define DIRECTINPUT_VERSION 0x0800

#include <dinput.h>

struct ZDIBUFFER {
	BYTE nKey;
	bool bPressed;
};

class ZDirectInput {
protected:
	HMODULE					m_hD3DLibrary;
	BOOL					m_bInitialized;
	LPDIRECTINPUT8			m_pDI;				///< The DirectInput object         
	LPDIRECTINPUTDEVICE8	m_pKeyboard;		///< The keyboard device 
	BOOL					m_bImmediateMode;

	LPDIRECTINPUTDEVICE8	m_pMouse;			///< mouse device
	unsigned int			m_nMouseButtons;	///> mouse buttons
	bool					m_bMouseButtonStates[8]{};

	LPDIRECTINPUTDEVICE8	m_pJoystick;		///< joystick device
	unsigned int			m_nJoyButtons;		///< joystick buttons
	unsigned int			m_nJoyPovs;			///< joystick povs
	unsigned int			m_nFFAxis;			///< force feedback axis
	bool					m_bForceFeedback;
	LPDIRECTINPUTEFFECT		m_pFFEffect;		///< force feedback effect

#define KEYNAMETABLE_COUNT	256
	char*					m_szKeyNameTable[KEYNAMETABLE_COUNT];

	static BOOL CALLBACK EnumJoysticksCallback( const DIDEVICEINSTANCE* pdidInstance, VOID* pContext );
	static BOOL CALLBACK EnumJoyObjectsCallback( const DIDEVICEOBJECTINSTANCE* pdidoi, VOID* pContext );

public:
	ZDirectInput();
	virtual ~ZDirectInput();

	bool CreateDirectInput();
	bool Create(HWND hWnd, BOOL bExclusive=TRUE, BOOL bImmediateMode=TRUE);
	void Destroy(void);

	void OnActivate(bool bActive);

	BOOL IsInitialized()	{ return m_bInitialized; }

	DWORD GetKeyboardBufferedData(ZDIBUFFER* pBuffer,unsigned int nBuffer);
	const char* GetKeyName(u32 nKey);


	unsigned int GetMouseButtonCount()		{ return m_nMouseButtons; }
	DWORD GetMouseBufferedData(int* pSumX,int* pSumY, ZDIBUFFER* pBuffer,unsigned int nBuffer);
	bool GetImmediateData(DIMOUSESTATE2 *pdims2);

	unsigned int GetJoystickPovCount()		{ return m_nJoyPovs; }
	unsigned int GetJoystickButtonCount()	{ return m_nJoyButtons; }
	bool GetJoystickData(DIJOYSTATE2* pjs);

	bool SetDeviceForcesXY(int nXForce, int nYForce);

};

#define ISKEYDOWN(_ScanCodeTable, _ScanCode)	((_ScanCodeTable[_ScanCode]&0x80)?true:false)