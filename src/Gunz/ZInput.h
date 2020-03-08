#pragma once

#include "ZActionDef.h"
#include <map>
#include "Mint.h"

using namespace std;

class ZDirectInput;

const int Z_DIRECTINPUTKEY_MESSAGE = 0x0f01;	// 참조하지 않는다. 별로 중요하진 않다.


typedef int ZVIRTUALKEY;

/*
ZVIRTUALKEY 으로 다음을 통합한다

키보드 스캔코드  0 ~ 255
마우스 버튼		 256 ~ 511
조이스틱 버튼	 512 ~
*/



/// first button, second ActionID
typedef map<ZVIRTUALKEY, int>	ZACTIONKEYMAP;

const int MAX_JOY_POV_COUNT		=	4;
const int MAX_JOY_BUTTON_COUNT	=	32;
const int MAX_JOY_VIRTUAL_BUTTON = MAX_JOY_POV_COUNT*4 + MAX_JOY_BUTTON_COUNT;

class ZInput
{
	/// Action Map
	ZACTIONKEYMAP	m_ActionKeyMap;

	bool			m_ActionKeyPressedTable[ZACTION_COUNT];
	bool			m_ActionKeyPressedLastTable[ZACTION_COUNT];
	
	bool			m_JoyPressedTable[MAX_JOY_VIRTUAL_BUTTON];

	ZDirectInput*	m_pDirectInput;

	MGLOBALEVENTCALLBACK* m_pEventListener;
	MGLOBALEVENTCALLBACK* m_pExclusiveListener;

	void Event(ZVIRTUALKEY key, bool bPressed);

	float			m_fRotationDeltaX;	// 누적된 회전값 움직임
	float			m_fRotationDeltaY;

public:
	ZInput(ZDirectInput* pDI);
	virtual ~ZInput(void);
	
	static const char* GetUndefinedKeyName();

	void Update();

	void GetRotation(float* pfX,float* pfY);

	bool GetInputKeyName(ZVIRTUALKEY key, char* pBuffer, int nBuffer);

	void ExclusiveEvent(ZVIRTUALKEY key);

	void OnActionKey(int nActionID,bool bPressed);

	/// 액션키 추가
	bool RegisterActionKey(int nActionID, ZVIRTUALKEY nKey);
	/// 액션키 제거
//	bool UnregisterActionKey(int nActionID);

	/// 액션키 클리어
	void ClearActionKey();

	// 이벤트 리스너 설정
	void SetEventListener(MGLOBALEVENTCALLBACK pEventCallback);

	// 모든 입력을 받을수 있는 독점 리스너 설정
	void SetExclusiveListener(MGLOBALEVENTCALLBACK pEventCallback);

	bool IsActionKeyDown(int nActionID)
	{
		if(nActionID<0 || nActionID>=ZACTION_COUNT){
			_ASSERT(FALSE);	// 0 ~ ACTIONKEYMAP_IDCOUNT-1 사이값이여야 한다.
			return false;
		}
		return m_ActionKeyPressedTable[nActionID];
	}

	bool WasActionKeyDownLast(int nActionID)
	{
		if (nActionID<0 || nActionID >= ZACTION_COUNT) {
			_ASSERT(FALSE);	// 0 ~ ACTIONKEYMAP_IDCOUNT-1 사이값이여야 한다.
			return false;
		}
		return m_ActionKeyPressedLastTable[nActionID];
	}

	// 입력은 0~1 사이, 0 = 진동없음. 1 = 진동 최대
	bool SetDeviceForcesXY(float fXForce, float fYForce);

	void ResetRotation()
	{
		m_fRotationDeltaX = m_fRotationDeltaY = 0;
	}
};