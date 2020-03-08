// MatchServerView.h : iCMatchServerView 클래스의 인터페이스
//


#pragma once


class CMatchServerDoc;


class CMatchServerView : public CView
{
protected: // serialization에서만 만들어집니다.
	CMatchServerView();
	DECLARE_DYNCREATE(CMatchServerView)

// 특성
public:
	CMatchServerDoc* GetDocument() const;

// 작업
public:

// 재정의
	public:
	virtual void OnDraw(CDC* pDC);  // 이 뷰를 그리기 위해 재정의되었습니다.
virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
protected:
	virtual BOOL OnPreparePrinting(CPrintInfo* pInfo);
	virtual void OnBeginPrinting(CDC* pDC, CPrintInfo* pInfo);
	virtual void OnEndPrinting(CDC* pDC, CPrintInfo* pInfo);

// 구현
public:
	virtual ~CMatchServerView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// 메시지 맵 함수를 생성했습니다.
protected:
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnSetFocus(CWnd* pOldWnd);
};

#ifndef _DEBUG  // MatchServerView.cpp의 디버그 버전
inline CMatchServerDoc* CMatchServerView::GetDocument() const
   { return reinterpret_cast<CMatchServerDoc*>(m_pDocument); }
#endif

