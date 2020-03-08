// CommandLogView.cpp : 구현 파일입니다.
//

#include "stdafx.h"
#ifdef MFC
#include "CommandLogView.h"
#include "MCommand.h"
#include "MMatchGlobal.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CCommandLogView

IMPLEMENT_DYNCREATE(CCommandLogView, CListView)

CCommandLogView::CCommandLogView()
{
}

CCommandLogView::~CCommandLogView()
{
}

BEGIN_MESSAGE_MAP(CCommandLogView, CListView)
END_MESSAGE_MAP()


// CCommandLogView 진단입니다.

#ifdef _DEBUG
void CCommandLogView::AssertValid() const
{
	CListView::AssertValid();
}

void CCommandLogView::Dump(CDumpContext& dc) const
{
	CListView::Dump(dc);
}
#endif //_DEBUG


// CCommandLogView 메시지 처리기입니다.

void CCommandLogView::OnInitialUpdate()
{
	CListView::OnInitialUpdate();

	CListView::OnInitialUpdate();

	// TODO: GetListCtrl()을 호출하여 해당 list 컨트롤을 직접 액세스함으로써
	// ListView를 항목으로 채울 수 있습니다.

	// this code only works for a report-mode list view
	ASSERT(GetStyle() & LVS_REPORT);

	// Gain a reference to the list control itself
	CListCtrl& theCtrl = GetListCtrl();

	// Insert a column. This override is the most convenient.
	theCtrl.InsertColumn(0, _T("No"), LVCFMT_LEFT);
	theCtrl.InsertColumn(1, _T("Time"), LVCFMT_LEFT);
	theCtrl.InsertColumn(2, _T("Type"), LVCFMT_LEFT);
	theCtrl.InsertColumn(3, _T("Sender"), LVCFMT_LEFT);
	theCtrl.InsertColumn(4, _T("Receiver"), LVCFMT_LEFT);
	theCtrl.InsertColumn(5, _T("Command"), LVCFMT_LEFT);

	#ifdef LEAK_TEST
		theCtrl.InsertColumn(6, _T("Memory"), LVCFMT_LEFT);
	#endif

	// Set reasonable widths for our columns
	theCtrl.SetColumnWidth(0, 50);
	theCtrl.SetColumnWidth(1, 80);
	theCtrl.SetColumnWidth(2, 50);
	theCtrl.SetColumnWidth(3, 80);
	theCtrl.SetColumnWidth(4, 80);
	theCtrl.SetColumnWidth(5, LVSCW_AUTOSIZE_USEHEADER);
	#ifdef LEAK_TEST
		theCtrl.SetColumnWidth(6, 80);
	#endif

}

BOOL CCommandLogView::PreCreateWindow(CREATESTRUCT& cs)
{
	cs.style |= LVS_REPORT;

	return CListView::PreCreateWindow(cs);
}


void CCommandLogView::AddCommand(u32 nGlobalClock, CCommandType t, MCommand* pCmd)
{
	static u32 m_stLogCount = 0;
	m_stLogCount++;

	char temp[4096]="";
	char szParam[1024]="";
	sprintf_safe(temp, "%s", pCmd->m_pCommandDesc->GetName());
	for(int i=0; i<pCmd->GetParameterCount(); i++){
		pCmd->GetParameter(i)->GetString(szParam);
		sprintf_safe(temp, "%s %s(%s)", temp, pCmd->GetParameter(i)->GetClassName(), szParam);
		
	}

	CListCtrl& theCtrl = GetListCtrl();

	char szNO[32];
	sprintf_safe(szNO, "%d", m_stLogCount);
	int nItemCount = theCtrl.GetItemCount();

	theCtrl.InsertItem(LVIF_TEXT|LVIF_STATE, nItemCount, szNO, 0, LVIS_SELECTED, 0, 0);

	char szGlobalClock[128];
	sprintf_safe(szGlobalClock, "%d", nGlobalClock);
	theCtrl.SetItemText(nItemCount, 1, szGlobalClock);
	char szType[64] = "Unknown";
	if(t==CCT_LOCAL) strcpy_literal(szType, "Local");
	else if(t==CCT_SEND) strcpy_literal(szType, "Send");
	else if(t==CCT_RECEIVE) strcpy_literal(szType, "Receive");
	char szSenderUID[128];
	sprintf_safe(szSenderUID, "%u:%u", pCmd->m_Sender.High, pCmd->m_Sender.Low);
	char szReceiverUID[128];
	sprintf_safe(szReceiverUID, "%u:%u", pCmd->m_Receiver.High, pCmd->m_Receiver.Low);
	theCtrl.SetItemText(nItemCount, 2, szType);
	theCtrl.SetItemText(nItemCount, 3, szSenderUID);
	theCtrl.SetItemText(nItemCount, 4, szReceiverUID);
	theCtrl.SetItemText(nItemCount, 5, temp);

#ifdef LEAK_TEST
	sprintf(temp, "%6.2f", (float)MGetCurrProcessMemory() / 1024.0f);
	theCtrl.SetItemText(nItemCount, 6, temp);
#endif

	//theCtrl.SetScrollPos(SB_VERT, );
	int nCount = theCtrl.GetItemCount();
	if (nCount > 0)	
		theCtrl.EnsureVisible(nCount-1, TRUE);

#define MAX_LOG_COUNT	1000
	if (nCount > MAX_LOG_COUNT)
		theCtrl.DeleteItem(0);
}
#endif