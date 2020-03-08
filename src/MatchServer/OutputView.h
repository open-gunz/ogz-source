#pragma once


// COutputView view

class COutputView : public CRichEditView
{
	DECLARE_DYNCREATE(COutputView)

protected:
	COutputView();           // protected constructor used by dynamic creation
	virtual ~COutputView();

public:
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	void AddString(const char* szString, COLORREF color=RGB(0,0,0), bool bCarrigeReturn=true);

protected:
	DECLARE_MESSAGE_MAP()
public:
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
//	virtual void OnInitialUpdate();
//	afx_msg void OnSetFocus(CWnd* pOldWnd);
//	afx_msg void OnActivate(UINT nState, CWnd* pWndOther, BOOL bMinimized);
//	afx_msg void OnSetFocus(CWnd* pOldWnd);
};


