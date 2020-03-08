#include "afxwin.h"
#include "afxcmn.h"
#if !defined(AFX_MCVVIEW_H__F6EB1E45_E7A7_4458_BE9F_B50A9E68AAA9__INCLUDED_)
#define AFX_MCVVIEW_H__F6EB1E45_E7A7_4458_BE9F_B50A9E68AAA9__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CMcvView : public CFormView
{
protected: 
	CMcvView();
	DECLARE_DYNCREATE(CMcvView)

public:
	//{{AFX_DATA(CMcvView)
	enum { IDD = IDD_MCV_FORM };

	CListBox	m_PartsListbox;
	CComboBox	m_PartsTypeCombo;
	CComboBox	m_WeaponTypeCombo;
	CListBox	m_WeaponListBox;
	CListBox	m_AnimationListBox;

	//}}AFX_DATA

	int		m_nSelectPartsType;
	int		m_nSelectWeaponType;
	int		m_nSelectWeaponType2;

	int		m_nWheel;

public:
	CMcvDoc* GetDocument();

public:
	bool Init();
	void Finish();

	void Update();
	void UpdateKey(float time);
	void UpdateLightKey(float time);
	void UpdateCameraKey(float time);

	void Idle();

	void FileOpen(char* FileName);

	void LoadFile(char* FileName);
	void LoadAniFile(char* FileName);
	void LoadXmlFile(char* FileName);

	void WFileOpen(char* FileName);

	void LoadWeaponFile(char* FileName);
	void LoadWeaponXmlFile(char* FileName);

	void ChangePartsListBox();
	void ChangeWeaponListBox();
	void ChangeAnimationListBox();

	void ChangeAnimationInfoString();

	//{{AFX_VIRTUAL(CMcvView)
	public:
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	virtual BOOL Create(LPCTSTR lpszClassName, LPCTSTR lpszWindowName, DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID, CCreateContext* pContext = NULL);
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual void OnInitialUpdate(); // called first time after construct
	//}}AFX_VIRTUAL
	
	CSliderCtrl* m_pSlider;

private:
	void UpdateAnimationSlider();
//	void ChangeAnimationInfoString();

public:
	virtual ~CMcvView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

protected:
	//{{AFX_MSG(CMcvView)
	afx_msg void OnDestroy();
	afx_msg void OnClose();
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnFileOpen();
	afx_msg void OnDropFiles(HDROP hDropInfo);
	afx_msg BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnTexOnOff();
	afx_msg void OnVertexNormal();
	afx_msg void OnUpdateVertexNormal(CCmdUI *pCmdUI);
	afx_msg void OnUpdateTexOnOff(CCmdUI *pCmdUI);
	afx_msg void OnCbnSelchangePartsType();
	afx_msg void OnLbnSelchangePartsList();
	afx_msg void OnLbnSelchangeWeaponList();
	afx_msg void OnLbnSelchangeAnimationList();
	afx_msg void OnBnClickedPlay();
	afx_msg void OnBnClickedStop();
	afx_msg void OnBnClickedPause();
	afx_msg void OnFileWopen();
	afx_msg void OnCameraReset();
	afx_msg void OnCbnSelchangeWeaponType();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
public:
	CComboBox m_WeaponType2Combo;
	afx_msg void OnCbnSelchangeWeaponType2();
	CSliderCtrl m_SliderAniSpeed;

	afx_msg void OnNMThemeChangedSlider1(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnNMReleasedcaptureSlider1(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnScaledlg();
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnBkcolor();
	afx_msg void OnMapObjPos();
	afx_msg void OnGridOnoff();
	afx_msg void OnBboxOnoff();
	afx_msg void OnUpdateBboxOnoff(CCmdUI *pCmdUI);
	afx_msg void OnUpdateGridOnoff(CCmdUI *pCmdUI);
	afx_msg void OnEffectRender();
	afx_msg void OnUpdateEffectRender(CCmdUI *pCmdUI);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnRButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnCharViewMode();
	afx_msg void OnUpdateCharViewMode(CCmdUI *pCmdUI);
	afx_msg void OnPartscolor();
	afx_msg void OnMtrleditdlg();
	afx_msg void OnPartscolorall();

	CString m_SelMeshNodeName;
	CString m_InfoString;
	afx_msg void OnAnimationinfo();
	afx_msg void OnBlendColor();
	afx_msg void OnCartoonLightOnoff();
	afx_msg void OnUpdateCartoonLightOnoff(CCmdUI *pCmdUI);
	afx_msg void OnModelinfo();
	afx_msg void OnStnClickedCurrentFrame();
	CStatic m_CurrentFrame;
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
};

#ifndef _DEBUG  
inline CMcvDoc* CMcvView::GetDocument()
   { return (CMcvDoc*)m_pDocument; }
#endif

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MCVVIEW_H__F6EB1E45_E7A7_4458_BE9F_B50A9E68AAA9__INCLUDED_)
