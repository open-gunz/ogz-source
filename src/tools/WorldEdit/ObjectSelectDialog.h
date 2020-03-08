#if !defined(AFX_OBJECTSELECTDIALOG_H__B3451C4F_9CCA_4114_8E09_8BDCC7030388__INCLUDED_)
#define AFX_OBJECTSELECTDIALOG_H__B3451C4F_9CCA_4114_8E09_8BDCC7030388__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ObjectSelectDialog.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CObjectSelectDialog dialog

class CObjectSelectDialog : public CDialog
{
// Construction
public:
	CObjectSelectDialog(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CObjectSelectDialog)
	enum { IDD = IDD_DIALOG_OBJECTSELECT };
	CListBox	m_ObjectList;
	//}}AFX_DATA

	void Initilize();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CObjectSelectDialog)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CObjectSelectDialog)
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_OBJECTSELECTDIALOG_H__B3451C4F_9CCA_4114_8E09_8BDCC7030388__INCLUDED_)
