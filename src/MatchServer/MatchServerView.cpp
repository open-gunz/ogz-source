// MatchServerView.cpp : CMatchServerView 클래스의 구현
//

#include "stdafx.h"
#ifdef MFC
#include "MatchServer.h"

#include "MatchServerDoc.h"
#include "MatchServerView.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CMatchServerView

IMPLEMENT_DYNCREATE(CMatchServerView, CView)

BEGIN_MESSAGE_MAP(CMatchServerView, CView)
	// 표준 인쇄 명령입니다.
	ON_COMMAND(ID_FILE_PRINT, CView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_DIRECT, CView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_PREVIEW, CView::OnFilePrintPreview)
	ON_WM_SETFOCUS()
END_MESSAGE_MAP()

// CMatchServerView 생성/소멸

CMatchServerView::CMatchServerView()
{
	// TODO: 여기에 생성 코드를 추가합니다.

}

CMatchServerView::~CMatchServerView()
{
}

BOOL CMatchServerView::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: CREATESTRUCT cs를 수정하여 여기에서
	// Window 클래스 또는 스타일을 수정합니다.

	return CView::PreCreateWindow(cs);
}

// CMatchServerView 그리기

void CMatchServerView::OnDraw(CDC* /*pDC*/)
{
	CMatchServerDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);

	// TODO: 여기에 원시 데이터에 대한 그리기 코드를 추가합니다.
}


// CMatchServerView 인쇄

BOOL CMatchServerView::OnPreparePrinting(CPrintInfo* pInfo)
{
	// 기본적인 준비
	return DoPreparePrinting(pInfo);
}

void CMatchServerView::OnBeginPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO: 인쇄하기 전에 추가 초기화 작업을 추가합니다.
}

void CMatchServerView::OnEndPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO: 인쇄 후 정리 작업을 추가합니다.
}


// CMatchServerView 진단

#ifdef _DEBUG
void CMatchServerView::AssertValid() const
{
	CView::AssertValid();
}

void CMatchServerView::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}

CMatchServerDoc* CMatchServerView::GetDocument() const // 디버그되지 않은 버전은 인라인으로 지정됩니다.
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CMatchServerDoc)));
	return (CMatchServerDoc*)m_pDocument;
}
#endif //_DEBUG


// CMatchServerView 메시지 처리기

void CMatchServerView::OnSetFocus(CWnd* pOldWnd)
{
	CView::OnSetFocus(pOldWnd);

	// TODO: Add your message handler code here
	GetDocument()->SetTitle("MatchServerView");
}
#endif