#pragma once
#include "afxcmn.h"
#include "afxwin.h"

// CControlDialog dialog

class CControlDialog : public CDialog
{
	DECLARE_DYNAMIC(CControlDialog)

public:
	CControlDialog(CWnd* pParent = NULL);   // standard constructor
	virtual ~CControlDialog();

// Dialog Data
	enum { IDD = IDD_DIALOG_CONTROL };

protected:
	DECLARE_MESSAGE_MAP()

public:
	afx_msg void OnStnClickedMinimap();
	afx_msg void OnBnClickedButtonResetcamera();
};
