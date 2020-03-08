// Locator.cpp : 응용 프로그램에 대한 클래스 동작을 정의합니다.
//

#include "stdafx.h"
#ifdef MFC
#include "Locator.h"
#include "MainFrm.h"

#include "LocatorDoc.h"
#include "LocatorView.h"
#include ".\locator.h"
#include "MDebug.h"
#include "MLocatorConfig.h"
#include "MLocatorView.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CLocatorApp

BEGIN_MESSAGE_MAP(CLocatorApp, CWinApp)
	ON_COMMAND(ID_APP_ABOUT, OnAppAbout)
	// 표준 파일을 기초로 하는 문서 명령입니다.
	ON_COMMAND(ID_FILE_NEW, CWinApp::OnFileNew)
	ON_COMMAND(ID_FILE_OPEN, CWinApp::OnFileOpen)
	// 표준 인쇄 설정 명령입니다.
	ON_COMMAND(ID_FILE_PRINT_SETUP, CWinApp::OnFilePrintSetup)
END_MESSAGE_MAP()


// CLocatorApp 생성

CLocatorApp::CLocatorApp() : m_dwViewUpdatedTime( timeGetTime() ) 
{
	// TODO: 여기에 생성 코드를 추가합니다.
	// InitInstance에 모든 중요한 초기화 작업을 배치합니다.
}


// 유일한 CLocatorApp 개체입니다.

CLocatorApp theApp;

// CLocatorApp 초기화

BOOL CLocatorApp::InitInstance()
{
	char szName[ 128 ] = {0,};
	SYSTEMTIME st;
	// GetSystemTime(&st);
	GetLocalTime( &st );
	_snprintf( szName, 127, "./Log/LocatorLog_%u-%u-%u_%u-%u.txt",
		st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute );
	InitLog( MLOGSTYLE_DEBUGSTRING | MLOGSTYLE_FILE, szName );

	// Locator의 환경 설정 정보파일을 로드.
	// 반드시 Locator가 생성되기전에 해줘야 함.
	if( !GetLocatorConfig()->LoadConfig() )
	{
		AfxMessageBox( "Lcator config load fail" );
		return FALSE;
	}

	// 응용 프로그램 매니페스트가 ComCtl32.dll 버전 6 이상을 사용하여 비주얼 스타일을
	// 사용하도록 지정하는 경우, Windows XP 상에서 반드시 InitCommonControls()가 필요합니다. 
	// InitCommonControls()를 사용하지 않으면 창을 만들 수 없습니다.
	InitCommonControls();

	CWinApp::InitInstance();

	// OLE 라이브러리를 초기화합니다.
	if (!AfxOleInit())
	{
		AfxMessageBox(IDP_OLE_INIT_FAILED);
		return FALSE;
	}
	AfxEnableControlContainer();
	// 표준 초기화
	// 이들 기능을 사용하지 않고 최종 실행 파일의 크기를 줄이려면
	// 아래에서 필요 없는 특정 초기화 루틴을 제거해야 합니다.
	// 해당 설정이 저장된 레지스트리 키를 변경하십시오.
	// TODO: 이 문자열을 회사 또는 조직의 이름과 같은
	// 적절한 내용으로 수정해야 합니다.
	SetRegistryKey(_T("로컬 응용 프로그램 마법사에서 생성된 응용 프로그램"));
	LoadStdProfileSettings(4);  // MRU를 포함하여 표준 INI 파일 옵션을 로드합니다.
	// 응용 프로그램의 문서 템플릿을 등록합니다. 문서 템플릿은
	// 문서, 프레임 창 및 뷰 사이의 연결 역할을 합니다.
	CSingleDocTemplate* pDocTemplate;
	pDocTemplate = new CSingleDocTemplate(
		IDR_MAINFRAME,
		RUNTIME_CLASS(CLocatorDoc),
		RUNTIME_CLASS(CMainFrame),       // 주 SDI 프레임 창입니다.
		// RUNTIME_CLASS(CLocatorView));
		RUNTIME_CLASS(MLocatorView));
	if (!pDocTemplate)
		return FALSE;
	AddDocTemplate(pDocTemplate);
	// 표준 셸 명령, DDE, 파일 열기에 대한 명령줄을 구문 분석합니다.
	CCommandLineInfo cmdInfo;
	ParseCommandLine(cmdInfo);
	// 명령줄에 지정된 명령을 디스패치합니다. 응용 프로그램이 /RegServer, /Register, /Unregserver 또는 /Unregister로 시작된 경우 FALSE를 반환합니다.
	if (!ProcessShellCommand(cmdInfo))
		return FALSE;
	// 창 하나만 초기화되었으므로 이를 표시하고 업데이트합니다.
	m_pMainWnd->SetWindowPos( &CWnd::wndTop, 200, 200, 520, 250, SWP_SHOWWINDOW );
	m_pMainWnd->ShowWindow(SW_SHOW);
	m_pMainWnd->UpdateWindow();
	// 접미사가 있을 경우에만 DragAcceptFiles를 호출합니다.
	// SDI 응용 프로그램에서는 ProcessShellCommand 후에 이러한 호출이 발생해야 합니다.

	return TRUE;
}



// 응용 프로그램 정보에 사용되는 CAboutDlg 대화 상자입니다.

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// 대화 상자 데이터
	enum { IDD = IDD_ABOUTBOX };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 지원

// 구현
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
END_MESSAGE_MAP()

// 대화 상자를 실행하기 위한 응용 프로그램 명령입니다.
void CLocatorApp::OnAppAbout()
{
	CAboutDlg aboutDlg;
	aboutDlg.DoModal();
}


// CLocatorApp 메시지 처리기


void CLocatorApp::HeartBeat()
{
	POSITION p = GetFirstDocTemplatePosition(); 
	CDocTemplate* pTemplate = GetNextDocTemplate(p); 
	p = pTemplate->GetFirstDocPosition(); 
	CLocatorDoc* pDoc = (CLocatorDoc*)pTemplate->GetNextDoc(p); 
	if(pDoc!=NULL) 
	{
		pDoc->Run();
		if( GetLocatorConfig()->GetMaxElapsedUpdateServerStatusTime() < (timeGetTime() - m_dwViewUpdatedTime) )
		{
			MLocatorView* pLocatorView = GetLocatorView();
			if( 0 != pLocatorView )
			{
				pLocatorView->GetListCtrl().DeleteAllItems();
				pLocatorView->UpdateView();

				m_dwViewUpdatedTime = timeGetTime();
			}
		}
	}
	Sleep( 1 );
}


int CLocatorApp::Run()
{
	// TODO: 여기에 특수화된 코드를 추가 및/또는 기본 클래스를 호출합니다.

	ASSERT_VALID(this);
	_AFX_THREAD_STATE* pState = AfxGetThreadState();

	// acquire and dispatch messages until a WM_QUIT message is received.
	for (;;)
	{
		// phase1: check to see if we can do idle work
		if (::PeekMessage(&(pState->m_msgCur), NULL, NULL, NULL, PM_NOREMOVE))
		{
			if (!PumpMessage())
				return ExitInstance();
		}

		HeartBeat();

		/*
		if( m_bShutdown == false )
			HeartBeat();
			
		else
			break;
			*/
	}

	return 0;
}
#endif