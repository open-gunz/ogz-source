#if !defined(AFX_OBJECTDIALOG_H__B3451C4F_9CCA_4114_8E09_8BDCC7030388__INCLUDED_)
#define AFX_OBJECTDIALOG_H__B3451C4F_9CCA_4114_8E09_8BDCC7030388__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ObjectDialog.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CObjectDialog dialog

class CObjectDialog : public CDialog
{
// Construction
public:
	CObjectDialog(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CObjectDialog)
	enum { IDD = IDD_DIALOG_OBJECT };
	CListBox	m_ObjectList;
	//}}AFX_DATA

	void Initilize();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CObjectDialog)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CObjectDialog)
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
//	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
public:
//	afx_msg void OnBnClickedRadio1();
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_OBJECTDIALOG_H__B3451C4F_9CCA_4114_8E09_8BDCC7030388__INCLUDED_)
