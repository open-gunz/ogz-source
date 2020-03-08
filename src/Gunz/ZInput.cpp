#include "StdAfx.h"
#include "ZInput.h"
#include "Mint4Gunz.h"
#include "ZConfiguration.h"

//////// ZInput 
// ZDirectInput 으로부터 키보드/마우스/조이스틱의 입력을 받아서
// 게임의 액션입력으로 변환하여주고 이벤트를 발생시키는 클래스입니다.

/*
ZVIRTUALKEY 으로 다음을 통합한다

키보드 스캔코드  0 ~ 255
마우스 버튼		 256 ~ 511
조이스틱 버튼	 512 ~
*/

const ZVIRTUALKEY MOUSE_BASE		= 256;
const ZVIRTUALKEY MOUSE_WHEEL		= MOUSE_BASE;
const ZVIRTUALKEY MOUSE_LBUTTON		= MOUSE_BASE+2;
const ZVIRTUALKEY JOY_BASE			= 512;
const ZVIRTUALKEY JOY_BUTTON_BASE	= 512+16;	// pov 최대4개(*4방향)

extern ZDirectInput	g_DInput;
extern Mint4Gunz	g_Mint;

ZInput::ZInput(ZDirectInput* pDI) : m_pDirectInput(pDI), m_pEventListener(NULL), m_pExclusiveListener(NULL)
{
	memset(m_ActionKeyPressedTable, 0, sizeof(bool)*ZACTION_COUNT);
	memset(m_JoyPressedTable, 0, sizeof(m_JoyPressedTable));
	m_fRotationDeltaX = 0;
	m_fRotationDeltaY = 0;
}

ZInput::~ZInput(void)
{
}

void ZInput::OnActionKey(int nActionID, bool bPressed)
{
	if(bPressed==m_ActionKeyPressedTable[nActionID]) return;	// 이미 같은상황
	m_ActionKeyPressedTable[nActionID] = bPressed;
	if(m_pEventListener==NULL) return;

	MEvent e;
	e.nMessage = bPressed ? MWM_ACTIONKEYDOWN : MWM_ACTIONKEYUP;
	e.nKey = nActionID;
	m_pEventListener(&e);

	e.nMessage = bPressed ? MWM_ACTIONPRESSED : MWM_ACTIONRELEASED;
	m_pEventListener(&e);
}

void ZInput::ExclusiveEvent(ZVIRTUALKEY key)
{
	if(m_pExclusiveListener)
	{
		MEvent e;
		e.nMessage = Z_DIRECTINPUTKEY_MESSAGE;
		e.nKey = key;
		m_pExclusiveListener(&e);
	}
}

const char* ZInput::GetUndefinedKeyName()
{
	return "N/A";
}

bool ZInput::GetInputKeyName(ZVIRTUALKEY key, char* pBuffer, int nBuffer)
{
	if(key>=512)	// joystick
	{
		if(key>=JOY_BASE && key<JOY_BASE+(int)m_pDirectInput->GetJoystickPovCount()*4)
		{
			int nPov = (key-JOY_BASE)/4;
			int nButton = (key-JOY_BASE)%4;
			const char *buttonName[] = { "Up", "Right", "Down", "Left" };
			sprintf_safe(pBuffer, nBuffer, "Joystick Pov%d %s",nPov,buttonName[nButton]);
			return true;
		}else
		if(key>=JOY_BUTTON_BASE && key<=JOY_BUTTON_BASE+(int)m_pDirectInput->GetJoystickButtonCount())
		{
			sprintf_safe(pBuffer, nBuffer, "Joystick button %d",key-JOY_BUTTON_BASE);
			return true;
		}

	}else
	if(key>=MOUSE_BASE && key<MOUSE_BASE+10)	// mouse
	{
		const char* mouseButtonNames[] = { "Wheel Up","Wheel Down","Left Button","Right Button","Middle Button","M4","M5","M6","M7","M8"};
		strcpy_safe(pBuffer, nBuffer, mouseButtonNames[key-MOUSE_BASE]);
		return true;
	}
	else	// keyboard
	{
		const char* szKeyName = m_pDirectInput->GetKeyName(key);
		if (szKeyName)
		{
			_ASSERT((int)strlen(szKeyName)<nBuffer);
			strcpy_safe(pBuffer, nBuffer, szKeyName);
			return true;
		}
	}

	strcpy_safe(pBuffer,nBuffer, GetUndefinedKeyName());
	return false;
}

void ZInput::GetRotation(float* pfX,float* pfY)
{
	*pfX = m_fRotationDeltaX;
	*pfY = m_fRotationDeltaY;

	m_fRotationDeltaX = 0;
	m_fRotationDeltaY = 0;
}


void ZInput::Update()
{
	if (!m_pDirectInput->IsInitialized()) return;

	memcpy(m_ActionKeyPressedLastTable, m_ActionKeyPressedTable, sizeof(m_ActionKeyPressedTable));

	////////////////////////////////////////////////////////
	// 키보드 입력
	static ZDIBUFFER keyBuffer[256];
	int nCount = m_pDirectInput->GetKeyboardBufferedData(keyBuffer,sizeof(keyBuffer)/sizeof(keyBuffer[0]));
	for(int i=0; i<nCount; i++){
		int nScanCode = keyBuffer[i].nKey;
		bool bPressed = keyBuffer[i].bPressed;
		if(bPressed)
			ExclusiveEvent(nScanCode);
		ZACTIONKEYMAP::iterator itr = m_ActionKeyMap.find(nScanCode);
		if(itr!=m_ActionKeyMap.end())
			OnActionKey(itr->second,bPressed);
	}

	static ZDIBUFFER mouseBuffer[256];
	int iDeltaX, iDeltaY;
	nCount = m_pDirectInput->GetMouseBufferedData(&iDeltaX,&iDeltaY,mouseBuffer,256);
	for(int i=0; i<nCount; i++){
		int nKey = mouseBuffer[i].nKey + MOUSE_BASE;
		bool bPressed = mouseBuffer[i].bPressed;
		if(bPressed)
			ExclusiveEvent(nKey);
		ZACTIONKEYMAP::iterator itr = m_ActionKeyMap.find(nKey);
		if(itr!=m_ActionKeyMap.end())
			OnActionKey(itr->second,bPressed);
	}

	if (Z_MOUSE_INVERT)
		iDeltaY = -iDeltaY;

	float fRotateStep = 0.005f * Z_MOUSE_SENSITIVITY;
	m_fRotationDeltaX += (iDeltaX * fRotateStep);
	m_fRotationDeltaY += (iDeltaY * fRotateStep);

	DIJOYSTATE2 js;
	if(m_pDirectInput->GetJoystickData(&js))
	{
		int nX = js.lX;
		int nY = js.lY;

		nX+= js.lZ;		// zaxis
		nY+= js.lRz;	// zrotation

		const int JOY_IGNORE = 100;
		nX = (nX<-JOY_IGNORE) ? nX+JOY_IGNORE :
				(nX>JOY_IGNORE) ? nX-JOY_IGNORE : 0;
		nY = (nY<-JOY_IGNORE) ? nY+JOY_IGNORE :
				(nY>JOY_IGNORE) ? nY-JOY_IGNORE : 0;
		
		float fJoyRotateStep = 0.0001f * ZGetConfiguration()->GetJoystick()->fSensitivity;
		m_fRotationDeltaX += nX * fJoyRotateStep;
		m_fRotationDeltaY += nY * fJoyRotateStep;

		for( unsigned int i=0; i< m_pDirectInput->GetJoystickPovCount(); i++)
		{
			const bool dir2buttons[9][4] = {
				{ true,false,false,false },
				{ true,true,false,false },
				{ false,true,false,false },
				{ false,true,true,false },
				{ false,false,true,false },
				{ false,false,true,true},
				{ false,false,false,true},
				{ true,false,false,true},
				{ false,false,false,false}};

			const bool *joyPOVTable;
			DWORD dwPOV = js.rgdwPOV[i];
			if(dwPOV==-1)
				joyPOVTable = dir2buttons[8];
			else
			{
				int nDirection = dwPOV / 4500;
				_ASSERT(nDirection>=0 && nDirection<=7);
				nDirection = min(7,max(0,nDirection));
				joyPOVTable = dir2buttons[nDirection];
			}

			for(int j=0;j<4;j++)
			{
				int nJoyVirtualButtonIndex = i*4+j;
				ZVIRTUALKEY nKey = JOY_BASE + nJoyVirtualButtonIndex;
				bool bPressed = joyPOVTable[j];
				if(bPressed)
					ExclusiveEvent(nKey);

				ZACTIONKEYMAP::iterator itr = m_ActionKeyMap.find(nKey);
				if(itr!=m_ActionKeyMap.end())
				{
					if(bPressed!=m_JoyPressedTable[nJoyVirtualButtonIndex])
					{
						m_JoyPressedTable[nJoyVirtualButtonIndex] = bPressed;
						OnActionKey(itr->second,bPressed);
					}
				}
			}
		}

		// 버튼 입력
		for( unsigned int i = 0; i < m_pDirectInput->GetJoystickButtonCount(); i++ )
		{
			int nJoyVirtualButtonIndex = MAX_JOY_POV_COUNT*4+i;

			ZVIRTUALKEY nKey = JOY_BASE + nJoyVirtualButtonIndex;

			bool bPressed = ((js.rgbButtons[i] & 0x80)!=0);

			if(bPressed)
				ExclusiveEvent(nKey);

			ZACTIONKEYMAP::iterator itr = m_ActionKeyMap.find(nKey);
			if(itr!=m_ActionKeyMap.end())
			{
				if(bPressed!=m_JoyPressedTable[nJoyVirtualButtonIndex])
				{
					m_JoyPressedTable[nJoyVirtualButtonIndex] = bPressed;
					OnActionKey(itr->second,bPressed);
				}
			}
		}
	}
}

bool ZInput::RegisterActionKey(int nActionID, ZVIRTUALKEY nKey)
{
	if(nActionID<0 || nActionID>=ZACTION_COUNT){
		_ASSERT(FALSE);
		return false;
	}

	m_ActionKeyMap.insert(ZACTIONKEYMAP::value_type(nKey, nActionID));
	return true;
}

/*
bool ZInput::UnregisterActionKey(int nActionID)
{
	if(nActionID<0 || nActionID>=ZACTION_COUNT){
		_ASSERT(FALSE);	// 0 ~ ZACTION_COUNT-1 사이값이여야 한다.
		return false;
	}

	for(ZACTIONKEYMAP::iterator i=m_ActionKeyMap.begin(); i!=m_ActionKeyMap.end(); i++){
		if((*i).second==nActionID){
			m_ActionKeyMap.erase(i);
			return true;
		}
	}
	return false;
}
*/

void ZInput::ClearActionKey()
{
	m_ActionKeyMap.clear();
}

void ZInput::SetEventListener(MGLOBALEVENTCALLBACK pEventCallback)
{
	m_pEventListener = pEventCallback;
}

void ZInput::SetExclusiveListener(MGLOBALEVENTCALLBACK pEventCallback)
{
	m_pExclusiveListener = pEventCallback;
}

bool ZInput::SetDeviceForcesXY(float fXForce, float fYForce)
{ 
	return m_pDirectInput->SetDeviceForcesXY(DI_FFNOMINALMAX*fXForce,DI_FFNOMINALMAX*fYForce);
}