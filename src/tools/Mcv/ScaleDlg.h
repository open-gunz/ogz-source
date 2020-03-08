#pragma once
#include "afxcmn.h"

class CScaleDlg : public CDialog
{
	DECLARE_DYNAMIC(CScaleDlg)

public:
	CScaleDlg(CWnd* pParent = NULL);   // 표준 생성자입니다.
	virtual ~CScaleDlg();

	enum { IDD = IDD_SCALE };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 지원입니다.

	DECLARE_MESSAGE_MAP()
public:
	CSliderCtrl m_sx;
	CSliderCtrl m_sy;
	CSliderCtrl m_sz;
	afx_msg void OnBnClickedOk();
	virtual BOOL OnInitDialog();
	afx_msg void OnNMThemeChangedSx(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnNMReleasedcaptureSx(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnNMReleasedcaptureSy(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnNMReleasedcaptureSz(NMHDR *pNMHDR, LRESULT *pResult);
};
