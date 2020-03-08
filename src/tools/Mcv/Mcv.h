// Mcv.h : main header file for the MCV application
//

#if !defined(AFX_MCV_H__DE0E8537_7A70_4308_B16E_3BA4EB7EED81__INCLUDED_)
#define AFX_MCV_H__DE0E8537_7A70_4308_B16E_3BA4EB7EED81__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"       // main symbols

/////////////////////////////////////////////////////////////////////////////
// CMcvApp:
// See Mcv.cpp for the implementation of this class
//
class CMcvView;

class CMcvApp : public CWinApp
{
public:
	CMcvApp();
	virtual int Run();
	void HeartBeat();
	CMcvView* GetView();

	//{{AFX_VIRTUAL(CMcvApp)
	public:
	virtual BOOL InitInstance();
//	virtual BOOL OnIdle(LONG lCount);
	virtual int	ExitInstance();
	//}}AFX_VIRTUAL

	//{{AFX_MSG(CMcvApp)
	afx_msg void OnAppAbout();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MCV_H__DE0E8537_7A70_4308_B16E_3BA4EB7EED81__INCLUDED_)
