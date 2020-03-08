#pragma once
#include "afxwin.h"


// CModelInfoDlg 대화 상자입니다.

class CModelInfoDlg : public CDialog
{
	DECLARE_DYNAMIC(CModelInfoDlg)

public:
	CModelInfoDlg(CWnd* pParent = NULL);   // 표준 생성자입니다.
	virtual ~CModelInfoDlg();

// 대화 상자 데이터입니다.
	enum { IDD = IDD_MODEL_INFO };

	void ClearListBox();

	void Begin();
	void End();

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 지원입니다.

	DECLARE_MESSAGE_MAP()
public:
	CListBox m_ListMeshNode;
	CListBox m_ListMtrlNode;
	afx_msg void OnBnClickedButtonMeshnodeColor();
	afx_msg void OnBnClickedButtonMtrlnodeColor();
};
