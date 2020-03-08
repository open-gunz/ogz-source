// OutputView.cpp : implementation file
//

#include "stdafx.h"
#ifdef MFC
#include "MatchServer.h"
#include "OutputView.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// COutputView

IMPLEMENT_DYNCREATE(COutputView, CRichEditView)

COutputView::COutputView()
{
}

COutputView::~COutputView()
{
}

BEGIN_MESSAGE_MAP(COutputView, CRichEditView)
	ON_WM_CREATE()
//	ON_WM_SETFOCUS()
//	ON_WM_ACTIVATE()
//ON_WM_SETFOCUS()
END_MESSAGE_MAP()


// COutputView diagnostics

#ifdef _DEBUG
void COutputView::AssertValid() const
{
	CRichEditView::AssertValid();
}

void COutputView::Dump(CDumpContext& dc) const
{
	CRichEditView::Dump(dc);
}
#endif //_DEBUG

void COutputView::AddString(const char* szString, COLORREF color, bool bCarrigeReturn)
{
	CRichEditCtrl& c = GetRichEditCtrl();

	// 최대출력 라인수 유지 //////////////////////////////////
	#define MAX_OUTPUT_COUNT	1000
	c.SetReadOnly(FALSE);
		int nCount;
		while( (nCount=c.GetLineCount()) > MAX_OUTPUT_COUNT) {
			int nBegin, nEnd;
			if (nCount > MAX_OUTPUT_COUNT) {
				if ( (nBegin=c.LineIndex(0)) != -1) {
					nEnd = nBegin+c.LineLength(0);
					c.SetSel(nBegin, nEnd+1);
					c.Clear();
				}
			}
		}
	c.SetReadOnly(TRUE);
	////////////////////////////////// 최대출력 라인수 유지 //

	// Determine AutoScroll ////////////////////////////
	bool bScroll = false;
	int nScrollMax = c.GetScrollLimit(SB_VERT);
	int nScrollCurrent = c.GetScrollPos(SB_VERT);
	if (nScrollCurrent+1 >= nScrollMax)
		bScroll = true;
	int nOldLineCount = c.GetLineCount();

	// Add String //////////////////////////
	c.SetSel(-1, -1);
	CHARFORMAT cf;
	cf.dwMask = CFM_FACE|CFM_SIZE|CFM_COLOR;
	strcpy_safe(cf.szFaceName, "Tahoma");
	cf.yHeight = 9*1440*1/72;	// Twip = 1/1440 inch, Font Dialog Size = 1/72 inch
	cf.crTextColor = color;
	cf.dwEffects = 0;
	c.SetSelectionCharFormat(cf);
	c.ReplaceSel(szString);
	if(bCarrigeReturn==true){
		c.SetSel(-1, -1);
		c.ReplaceSel("\n");
	}

	// AutoScroll //
	if (bScroll) {
		int nLines = c.GetLineCount()-nOldLineCount;
		c.LineScroll(nLines);
	}
}


// COutputView message handlers

int COutputView::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CRichEditView::OnCreate(lpCreateStruct) == -1)
		return -1;

	CRichEditCtrl& c = GetRichEditCtrl();
	c.SetReadOnly(TRUE);
	c.SetUndoLimit(0);
	c.SetOptions(ECOOP_OR, ECO_AUTOVSCROLL);
	c.SetOptions(ECOOP_XOR, ECO_AUTOVSCROLL);

	return 0;
}

//void COutputView::OnInitialUpdate()
//{
//	CRichEditView::OnInitialUpdate();
//
//}

//void COutputView::OnSetFocus(CWnd* pOldWnd)
//{
//	CRichEditView::OnSetFocus(pOldWnd);
//
//	GetParentFrame()->SetWindowText("Output");
//}

//void COutputView::OnActivate(UINT nState, CWnd* pWndOther, BOOL bMinimized)
//{
//	CRichEditView::OnActivate(nState, pWndOther, bMinimized);
//
//	GetParentFrame()->SetWindowText("Output");
//}

//void COutputView::OnSetFocus(CWnd* pOldWnd)
//{
//	CRichEditView::OnSetFocus(pOldWnd);
//
//	CDocument* pDoc = CView::GetDocument();
//	if(pDoc!=NULL) pDoc->SetTitle("Output");
//}
#endif