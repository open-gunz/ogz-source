#pragma once
#include "afxwin.h"


// CRCFrameInfoDlg 대화 상자입니다.

class CRCFrameInfoDlg : public CDHtmlDialog
{
	DECLARE_DYNCREATE(CRCFrameInfoDlg)

public:
	CRCFrameInfoDlg(CWnd* pParent = NULL);   // 표준 생성자입니다.
	virtual ~CRCFrameInfoDlg();
// 재정의
	HRESULT OnButtonOK(IHTMLElement *pElement);
	HRESULT OnButtonCancel(IHTMLElement *pElement);

// 대화 상자 데이터입니다.
	enum { IDD = IDD_FRAME_INFO, IDH = IDR_HTML_RCFRAMEINFODLG };

	void ClearListBox();
	void Begin();
	void End();

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 지원입니다.
	virtual BOOL OnInitDialog();

	DECLARE_MESSAGE_MAP()
	DECLARE_DHTML_EVENT_MAP()
public:
	CListBox m_ListBox;
	afx_msg void OnLbnSelchangeNodeListbox();
};
