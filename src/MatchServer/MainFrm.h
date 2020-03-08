// MainFrm.h : CMainFrame 클래스의 인터페이스
//
#include "ConsoleBar.h"


#pragma once

#include "TrayIcon.h"

#define WM_ICON_NOTIFY		(WM_USER+10)

class CMainFrame : public CMDIFrameWnd
{
	DECLARE_DYNAMIC(CMainFrame)
public:
	CMainFrame();

protected:
	void ReadPosition();
	void WritePosition();
	void UpdateServerStatusBar();

// 작업
public:

// 재정의
public:
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);

// 구현
public:
	virtual ~CMainFrame();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:  // 컨트롤 모음이 포함된 멤버입니다.
	CStatusBar  m_wndStatusBar;
	CToolBar    m_wndToolBar;

public:
	CConsoleBar		m_wndConsoleBar;
	CTrayIcon		m_TrayIcon;

// 메시지 맵 함수를 생성했습니다.
protected:
	LRESULT OnTrayNotification(WPARAM wParam, LPARAM lParam);
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnActivate(UINT nState, CWnd* pWndOther, BOOL bMinimized);
	afx_msg void OnClose();
	afx_msg void OnMenuTerminate();
	afx_msg void OnMenuShow();
	afx_msg void OnMenuHide();
	afx_msg void OnDestroy();
	afx_msg void OnTimer(UINT nIDEvent);
};


