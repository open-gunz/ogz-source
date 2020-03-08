// Locator.h : Locator 응용 프로그램에 대한 주 헤더 파일
//
#pragma once

#ifdef MFC
#ifndef __AFXWIN_H__
	#error PCH에서 이 파일을 포함하기 전에 'stdafx.h'를 포함하십시오.
#endif

#include "resource.h"       // 주 기호


// CLocatorApp:
// 이 클래스의 구현에 대해서는 Locator.cpp을 참조하십시오.
//

class CLocatorApp : public CWinApp
{
public:
	CLocatorApp();

private :
	void HeartBeat();

private :
	DWORD m_dwViewUpdatedTime;

// 재정의
public:
	virtual BOOL InitInstance();

// 구현
	afx_msg void OnAppAbout();
	DECLARE_MESSAGE_MAP()
	virtual int Run();
};

extern CLocatorApp theApp;
#endif