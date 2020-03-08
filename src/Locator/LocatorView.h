// LocatorView.h : iCLocatorView 클래스의 인터페이스
//


#pragma once


class CLocatorView : public CView
{
protected: // serialization에서만 만들어집니다.
	CLocatorView();
	DECLARE_DYNCREATE(CLocatorView)

// 특성
public:
	CLocatorDoc* GetDocument() const;

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
	virtual ~CLocatorView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// 메시지 맵 함수를 생성했습니다.
protected:
	DECLARE_MESSAGE_MAP()
public:
};

#ifndef _DEBUG  // LocatorView.cpp의 디버그 버전
inline CLocatorDoc* CLocatorView::GetDocument() const
   { return reinterpret_cast<CLocatorDoc*>(m_pDocument); }
#endif

