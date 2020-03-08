// MainFrm.cpp : CMainFrame 클래스의 구현
//

#include "stdafx.h"
#ifdef MFC
#include "MatchServer.h"

#include "MainFrm.h"
#include "MRegistry.h"
#include "mainfrm.h"
#include "resource.h"

#include "MMatchServer.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CMainFrame
IMPLEMENT_DYNAMIC(CMainFrame, CMDIFrameWnd)

BEGIN_MESSAGE_MAP(CMainFrame, CMDIFrameWnd)
	ON_WM_CREATE()
	ON_WM_SIZE()
	ON_WM_ACTIVATE()
	ON_WM_CLOSE()
	ON_MESSAGE(WM_ICON_NOTIFY, OnTrayNotification)
	ON_COMMAND(ID_MENU_TERMINATE, OnMenuTerminate)
	ON_COMMAND(ID_MENU_SHOW, OnMenuShow)
	ON_COMMAND(ID_MENU_HIDE, OnMenuHide)
	ON_WM_DESTROY()
	ON_WM_TIMER()
END_MESSAGE_MAP()

static UINT indicators[] =
{
	ID_SEPARATOR,           // 상태 줄 표시기
	ID_INDICATOR_SERVERSTATUS,
	ID_INDICATOR_CAPS,
	ID_INDICATOR_NUM,
	ID_INDICATOR_SCRL,
};

#define TIMERID_REFRESH_STATUSBAR	101


// CMainFrame 생성/소멸

CMainFrame::CMainFrame()
{
	// TODO: 여기에 멤버 초기화 코드를 추가합니다.
}

CMainFrame::~CMainFrame()
{
	m_TrayIcon.RemoveIcon();
}


int CMainFrame::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CMDIFrameWnd::OnCreate(lpCreateStruct) == -1)
		return -1;
	
	if (!m_wndToolBar.CreateEx(this, TBSTYLE_FLAT, WS_CHILD | WS_VISIBLE | CBRS_TOP
		| CBRS_GRIPPER | CBRS_TOOLTIPS | CBRS_FLYBY | CBRS_SIZE_DYNAMIC) ||
		!m_wndToolBar.LoadToolBar(IDR_MAINFRAME))
	{
		TRACE0("도구 모음을 만들지 못했습니다.\n");
		return -1;      // 만들지 못했습니다.
	}

	if (!m_wndStatusBar.Create(this) ||
		!m_wndStatusBar.SetIndicators(indicators,
		  sizeof(indicators)/sizeof(UINT)))
	{
		TRACE0("상태 표시줄을 만들지 못했습니다.\n");
		return -1;      // 만들지 못했습니다.
	}

	// TODO: 도구 모음을 도킹할 수 없게 하려면 이 세 줄을 삭제하십시오.
	m_wndToolBar.EnableDocking(CBRS_ALIGN_ANY);
	EnableDocking(CBRS_ALIGN_ANY);
	DockControlBar(&m_wndToolBar);

	if(!m_wndConsoleBar.Create(this, IDD_DIALOG_CONSOLE, CBRS_BOTTOM|CBRS_TOOLTIPS|CBRS_FLYBY, IDD_DIALOG_CONSOLE)){
		TRACE0("Failed to create ConsoleBar\n");
		return -1;
	}
	m_wndConsoleBar.ShowWindow(SW_HIDE);

	ReadPosition();

	// 트레이 아이콘 등록
	if (!m_TrayIcon.Create(this, WM_ICON_NOTIFY, _T("MatchServer"), NULL, IDR_MAINFRAME))
	{
		TRACE0("트레이 아이콘을 만들지 못했습니다.\n");
		return -1;
	}
	// 아이콘 설정
	HICON icon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	m_TrayIcon.SetIcon(icon);

	// 서버상태바 타이머
	this->SetTimer(TIMERID_REFRESH_STATUSBAR, 5000, NULL);

	return 0;
}

BOOL CMainFrame::PreCreateWindow(CREATESTRUCT& cs)
{
	if( !CMDIFrameWnd::PreCreateWindow(cs) )
		return FALSE;
	// TODO: CREATESTRUCT cs를 수정하여 여기에서
	// Window 클래스 또는 스타일을 수정합니다.

	return TRUE;
}


// CMainFrame 진단

#ifdef _DEBUG
void CMainFrame::AssertValid() const
{
	CMDIFrameWnd::AssertValid();
}

void CMainFrame::Dump(CDumpContext& dc) const
{
	CMDIFrameWnd::Dump(dc);
}

#endif //_DEBUG


// CMainFrame 메시지 처리기


void CMainFrame::OnSize(UINT nType, int cx, int cy)
{
	// Console Resize
	RECT cr;
	GetClientRect(&cr);
	m_wndConsoleBar.m_sizeDefault.cx = cr.right-cr.left;
	//RecalcLayout();

	CMDIFrameWnd::OnSize(nType, cx, cy);
	// TODO: Add your message handler code here
}

void CMainFrame::OnActivate(UINT nState, CWnd* pWndOther, BOOL bMinimized)
{
	CMDIFrameWnd::OnActivate(nState, pWndOther, bMinimized);

	CMatchServerApp* pApp = (CMatchServerApp*)AfxGetApp();
	if (pApp->IsShutdown() == true) return;

	if(nState==WA_ACTIVE) m_wndConsoleBar.GetDlgItem(IDC_COMBO_COMMAND)->SetFocus();
}

void CMainFrame::OnClose()
{
	if (AfxMessageBox("정말로 서버 끌꺼에요?", MB_YESNO | MB_ICONQUESTION) != IDYES)
	{
		return;
	}

	// TODO: Add your message handler code here and/or call default
   
	CMDIFrameWnd::OnClose();
}

void CMainFrame::OnDestroy()
{
	CMDIFrameWnd::OnDestroy();

	// TODO: Add your message handler code here
	TRACE("CMainFrame::OnDestroy() \n");

	WritePosition();
	CMatchServerApp* pApp = (CMatchServerApp*)AfxGetApp();
	pApp->Shutdown();
}

void CMainFrame::ReadPosition()
{
	RECT rt;
	DWORD dwSize = sizeof(rt);
	if (MRegistry::ReadBinary(HKEY_CURRENT_USER, "MainPosition", (char*)&rt, &dwSize) == true)
		MoveWindow(&rt);
		
}

void CMainFrame::WritePosition()
{
	RECT rt;
	GetWindowRect(&rt);
	if (rt.left < 0 || rt.top < 0 || rt.right - rt.left < 0 || rt.bottom - rt.top < 0)
		return;

	MRegistry::WriteBinary(HKEY_CURRENT_USER, "MainPosition", (char*)&rt, sizeof(rt));
}


LRESULT CMainFrame::OnTrayNotification(WPARAM wParam, LPARAM lParam)
{
	if (LOWORD(lParam) == WM_RBUTTONUP)
	{
		CMenu menu, *pSubMenu;

		if (!menu.LoadMenu(IDR_POPUPMENU)) return 0;
		if (!(pSubMenu = menu.GetSubMenu(0))) return 0;

		CPoint pos;
		GetCursorPos(&pos);
		SetForegroundWindow();

		pSubMenu->TrackPopupMenu(TPM_RIGHTALIGN, pos.x, pos.y, this);
		menu.DestroyMenu();
	}
	else if (LOWORD(lParam) == WM_LBUTTONDBLCLK)
	{
		if (IsWindowVisible()) ShowWindow(SW_HIDE);
		else ShowWindow(SW_SHOW);
	}
	return 1;
}
void CMainFrame::OnMenuTerminate()
{
	SendMessage(WM_CLOSE, 0, 0);
}

void CMainFrame::OnMenuShow()
{
	ShowWindow(SW_SHOW);
}

void CMainFrame::OnMenuHide()
{
	ShowWindow(SW_HIDE);
}


void CMainFrame::OnTimer(UINT nIDEvent)
{
	// TODO: 여기에 메시지 처리기 코드를 추가 및/또는 기본값을 호출합니다.
	if (nIDEvent == TIMERID_REFRESH_STATUSBAR) {
		UpdateServerStatusBar();
	}

	CMDIFrameWnd::OnTimer(nIDEvent);
}

void CMainFrame::UpdateServerStatusBar()
{
	MMatchServer* pServer = MMatchServer::GetInstance();
	if (!pServer->IsCreated()) return;

	CString strStatus;
	strStatus.Format("C(%d),N(%d),A(%d)", 
		pServer->GetClientCount(), pServer->GetCommObjCount(), pServer->GetAgentCount());

	int nIndex = m_wndStatusBar.CommandToIndex(ID_INDICATOR_SERVERSTATUS);
	m_wndStatusBar.SetPaneInfo(nIndex, ID_INDICATOR_SERVERSTATUS, SBPS_STRETCH, 200);
	m_wndStatusBar.SetPaneText(nIndex, strStatus);
}
#endif