/*
	----------------
	Multimedia Timer
	----------------

	MMTimer.h

	"Multimedia Timer" handling class header file

	Programming by Chojoongpil
*/
#pragma once

#include "GlobalTypes.h"
#include "MUtil.h"

typedef bool (*MMTIMERCALLBACK)(uintptr_t);

// 하나의 클래스는 한개의 타이머를 갖는다.
// 여러개의 셋팅을 지원하지 않으므로 주의할 것.
class MMTimer
{
public:
    MMTimer();
    ~MMTimer();
	
	// Multimedia Timer한개를 생성한다.
    bool Create(u32 nPeriod, u32 nRes, u32 dwUser,  MMTIMERCALLBACK pfnCallback);
	void Destroy();

protected:
	static void STDCALL TimeProc(unsigned int uID, unsigned int uMsg,
		WIN_DWORD_PTR dwUser, WIN_DWORD_PTR dw1, WIN_DWORD_PTR dw2);

	// 현재 타이머가 분기할 콜백 함수
    MMTIMERCALLBACK m_pfnCallback;

    u32 m_dwUser;
    u32 m_nPeriod;
    u32 m_nRes;
    u32 m_nIDTimer;
};
