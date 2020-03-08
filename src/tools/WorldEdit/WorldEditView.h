// WorldEditView.h : interface of the CWorldEditView class
//
/////////////////////////////////////////////////////////////////////////////

#if !defined(AFX_WORLDEDITVIEW_H__36AF7C31_5DA9_4540_85E4_25B13C0CFF62__INCLUDED_)
#define AFX_WORLDEDITVIEW_H__36AF7C31_5DA9_4540_85E4_25B13C0CFF62__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "RTypes.h"
#include "RBspObject.h"
#include "GlobalTypes.h"
#include "MTime.h"

enum EDITMODE {
	EDITMODE_OBJECT,
	EDITMODE_PATH,
	EDITMODE_TRIGGER,
};

class CWorldEditView : public CView
{
protected:
	DECLARE_DYNCREATE(CWorldEditView)

public:
	CWorldEditDoc* GetDocument();
	EDITMODE	m_EditMode;

public:

	void Resize(CSize size);
	void OnResetCamera();

public:
	virtual void OnDraw(CDC* pDC);
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	virtual BOOL Create(LPCTSTR lpszClassName, LPCTSTR lpszWindowName, DWORD dwStyle,
		const RECT& rect, CWnd* pParentWnd, UINT nID, CCreateContext* pContext = NULL);

public:
	virtual ~CWorldEditView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	bool	m_bWireframe{};
	bool	m_bDrawBoundingBox{};
	bool	m_bDrawOcclusion{};
	bool	m_bShowLightmap{};

protected:
	bool m_bLastShiftState{}, m_bLastAltState{};
	v3 m_LastWorldPosition{ 0, 0, 0 };
	v3 m_LastCameraPosition{ 0, 0, 0 };
	rmatrix m_LastMatrix = GetIdentityMatrix();
	CPoint	m_LastCursorPosition;
	CPoint LastFrameCursorPosition;

	RSBspNode	*m_pSelectedNode;
	int			m_nSelectedIndex;
	int			m_nSelectedEdge;

	bool Activated{};
	u64 LastTime = GetGlobalTimeMS();

	void GetWorldCoordinate(rvector *ret,CPoint pt);

protected:
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);
	afx_msg void OnRuntimeboundingbox();
	afx_msg void OnWireframe();
	afx_msg void OnUpdateBoundingbox(CCmdUI *pCmdUI);
	afx_msg void OnBoundingbox();
	afx_msg void OnUpdateWireframe(CCmdUI *pCmdUI);
	afx_msg void OnOcclusion();
	afx_msg void OnUpdateOcclusion(CCmdUI *pCmdUI);
	afx_msg void OnShowlightmap();
	afx_msg void OnUpdateShowlightmap(CCmdUI *pCmdUI);
	afx_msg void OnDropFiles(HDROP hDropInfo);
};

#ifndef _DEBUG  // debug version in WorldEditView.cpp
inline CWorldEditDoc* CWorldEditView::GetDocument()
   { return (CWorldEditDoc*)m_pDocument; }
#endif

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_WORLDEDITVIEW_H__36AF7C31_5DA9_4540_85E4_25B13C0CFF62__INCLUDED_)
