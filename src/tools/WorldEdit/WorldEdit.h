// WorldEdit.h : main header file for the WORLDEDIT application
//

#if !defined(AFX_WORLDEDIT_H__B0DB957A_4F7B_4FF8_BA08_F0EA3E0B3879__INCLUDED_)
#define AFX_WORLDEDIT_H__B0DB957A_4F7B_4FF8_BA08_F0EA3E0B3879__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"       // main symbols

/////////////////////////////////////////////////////////////////////////////
// CWorldEditApp:
// See WorldEdit.cpp for the implementation of this class
//

class CWorldEditApp : public CWinApp
{
public:
	CWorldEditApp();
	virtual ~CWorldEditApp();

// Overrides
	void OnFileOpen();
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CWorldEditApp)
	public:
	virtual BOOL InitInstance();
	//}}AFX_VIRTUAL

// Implementation
	//{{AFX_MSG(CWorldEditApp)
	afx_msg void OnAppAbout();
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
	virtual BOOL OnIdle(LONG lCount);
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_WORLDEDIT_H__B0DB957A_4F7B_4FF8_BA08_F0EA3E0B3879__INCLUDED_)
