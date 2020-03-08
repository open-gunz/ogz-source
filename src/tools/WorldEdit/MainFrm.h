// MainFrm.h : interface of the CMainFrame class
//
/////////////////////////////////////////////////////////////////////////////

#if !defined(AFX_MAINFRM_H__DBADBDED_9592_4CE1_B0ED_BC527B18E675__INCLUDED_)
#define AFX_MAINFRM_H__DBADBDED_9592_4CE1_B0ED_BC527B18E675__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "ObjectDialog.h"
#include "PathDialog.h"
#include "ControlDialog.h"

class CMainFrame : public CFrameWnd
{
protected:
	CMainFrame();
	DECLARE_DYNCREATE(CMainFrame)

// Operations
public:
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);

// Implementation
public:
	virtual ~CMainFrame();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:  // control bar embedded members
	CStatusBar  m_wndStatusBar;
	CToolBar    m_wndToolBar;
	
	RECT		m_LastPos;
	CObjectDialog		m_ObjectDialog;
	CPathDialog			m_PathDialog;
	CControlDialog		m_ControlDialog;

	void HideDialogs();
	void PlaceControlDialog();
	void PlaceDialog(CDialog *pDlg);
	void MoveDialog(CDialog *pDlg,CPoint diff);
	void Resize(CSize size);

	bool ConfirmOverwrite(const char *filename);

protected:
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnMenuitemModeobject();
	afx_msg void OnMenuitemModepath();
	afx_msg void OnMenuitemGenerateLightmap();
	DECLARE_MESSAGE_MAP()
public:

	void Initialize();

	afx_msg void OnMove(int x, int y);
	afx_msg void OnMoving(UINT fwSide, LPRECT pRect);
	afx_msg void OnScreensize800();
	afx_msg void OnScreensize1280();
	afx_msg void OnScreensize1024();
	afx_msg void OnStnClickedMinimap();
	afx_msg void OnMenuitemGeneratePathdata();
	afx_msg void OnViewinfo();
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MAINFRM_H__DBADBDED_9592_4CE1_B0ED_BC527B18E675__INCLUDED_)
