#pragma once
#include "afxwin.h"


// CMtrlEditDlg 대화 상자입니다.
class RMtrl;
class RMtrlMgr;
class CMtrlEditDlg : public CDialog
{
	DECLARE_DYNAMIC(CMtrlEditDlg)

public:
	CMtrlEditDlg(CWnd* pParent = NULL);   // 표준 생성자입니다.
	virtual ~CMtrlEditDlg();

// 대화 상자 데이터입니다.
	enum { IDD = IDD_MTRLEDITDLG };

	RMtrlMgr* GetMtrlMgr();
	RMtrl* GetMtrl(int index);
	void UpdateMtrl(RMtrl* pMtrl);
	void UpdateName();

	void ClearListBox();

	void Begin();
	void End();

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 지원입니다.

	DECLARE_MESSAGE_MAP()
public:
	CListBox m_mtrl_list_box;
	CString m_texture_name;
	CString m_alpha_texture_name;
	CButton m_SaveButton;
	afx_msg void OnBnClickedOksave();
	afx_msg void OnBnClickedCancel();
	afx_msg void OnLbnSelchangeMtrllist();
	BYTE m_nAlphaRefValue;
};
