// MatchServer.h : MatchServer 응용 프로그램에 대한 주 헤더 파일
//
#pragma once

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"       // 주 기호
#include "MZFileSystem.h"


// CMatchServerApp:
// 이 클래스의 구현에 대해서는 MatchServer.cpp을 참조하십시오.
//

class CMatchServerApp : public CWinApp
{
private:
	bool				m_bShutdown;
	bool				m_bOutputLog;

public:
	MZFileSystem		m_ZFS;

	CMultiDocTemplate* m_pDocTemplateOutput;
	//CMultiDocTemplate* m_pDocTemplateMapView;
	CMultiDocTemplate* m_pDocTemplateCmdLogView;

	BOOL PreTranslateMessage(MSG* pMsg);
public:
	CMatchServerApp();
	virtual ~CMatchServerApp();
	
	bool CheckOutputLog()	{ return m_bOutputLog; }

	bool IsShutdown()	{ return m_bShutdown; }
	void Shutdown()		{ m_bShutdown = true; }
	void HeartBeat();

// 재정의
public:
	virtual BOOL InitInstance();

// 구현
	afx_msg void OnAppAbout();
	DECLARE_MESSAGE_MAP()
	virtual int ExitInstance();
	virtual int Run();
	afx_msg void OnViewServerStatus();
	afx_msg void OnMessageExit();
	afx_msg void OnShowCmdLog();
	afx_msg void OnSetUseCountryFilter();
	afx_msg void OnSetAccetpInvalidIP();
	afx_msg void OnUpdateShowCmdLog(CCmdUI *pCmdUI);
	afx_msg void OnUseCountryFilter(CCmdUI *pCmdUI);
	afx_msg void OnAcceptInvalidIP(CCmdUI* pCmdUI );
	afx_msg void OnUpdateIPtoCountry();
	afx_msg void OnUpdateBlockCountryCode();
	afx_msg void OnUpdateCustomIP();
	afx_msg void OnEnableUpdateIPtoCountry( CCmdUI* pCmdUI );
	afx_msg void OnEnableUpdateBlockCountryCode( CCmdUI* pCmdUI );
	afx_msg void OnEnableUpdateCustomIP( CCmdUI* pCmdUI );
};

extern CMatchServerApp theApp;
