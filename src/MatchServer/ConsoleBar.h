#pragma once


class MMonitor;
class MMonitorCommandProcessor;

// CConsoleBar dialog

class CConsoleBar : public CDialogBar
{
	DECLARE_DYNAMIC(CConsoleBar)

protected:
	CString	m_szPrevCommandEdit;
public:
	CConsoleBar(CWnd* pParent = NULL);   // standard constructor
	virtual ~CConsoleBar();

	MMonitor* GetMonitor(void);

	void GetCurrentCommandString(CString& szCommand);
	void PostCommand(void);

// Dialog Data
	enum { IDD = IDD_DIALOG_CONSOLE };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnSize(UINT nType, int cx, int cy);
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	afx_msg void OnCbnEditchangeComboCommand();
	afx_msg void OnCbnSelchangeComboCommand();
	afx_msg void OnCbnSelendokComboCommand();
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnCbnDropdownComboCommand();
	afx_msg void OnCbnEditupdateComboCommand();
};
