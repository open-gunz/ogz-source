// MatchServer.cpp : 응용 프로그램에 대한 클래스 동작을 정의합니다.
//

#include "stdafx.h"
#ifdef MFC
#include "MatchServer.h"
#ifdef MFC
#include "MainFrm.h"
#endif

#include <shlwapi.h>

#include "ChildFrm.h"
#include "MatchServerDoc.h"
#include "MatchServerView.h"
#include "OutputView.h"
#include "CommandLogView.h"
#include "MRegistry.h"
#include "matchserver.h"
#include "MBMatchServer.h"
#include "MDebug.h"
#include "MSync.h"
#include "MMatchConfig.h"
#include "MTraceMemory.h"

#ifdef _DEBUG
#define new DEBUG_NEW

// 이것은 테스트용..bird
#define _FETCH_112

#endif


#ifdef _FETCH_112
	#include "MInet.h"
	MHttp g_Http;
#endif

#define APPLICATION_NAME	"MatchServer"

// 유일한 CMatchServerApp 개체입니다.
CMatchServerApp			theApp;
MSingleRunController	g_SingleRunController("MatchServer"); 


// CMatchServerApp

BEGIN_MESSAGE_MAP(CMatchServerApp, CWinApp)
	ON_COMMAND(ID_APP_ABOUT, OnAppAbout)
	// 표준 파일을 기초로 하는 문서 명령입니다.
	ON_COMMAND(ID_FILE_NEW, CWinApp::OnFileNew)
	ON_COMMAND(ID_FILE_OPEN, CWinApp::OnFileOpen)
	// 표준 인쇄 설정 명령입니다.
	ON_COMMAND(ID_FILE_PRINT_SETUP, CWinApp::OnFilePrintSetup)
	ON_COMMAND(ID_ViewServerStatus, OnViewServerStatus)
	ON_COMMAND(ID_MESSAGE_EXIT, OnMessageExit)
	ON_COMMAND(ID_SHOW_CMD_LOG, OnShowCmdLog)
	ON_COMMAND(ID_USE_COUNTRY_FILTER, OnSetUseCountryFilter)
	ON_COMMAND(ID_ACCEPT_INVAILD_IP, OnSetAccetpInvalidIP)
	ON_COMMAND(ID_UPDATE_IPtoCOUNTRY, OnUpdateIPtoCountry)
	ON_COMMAND(ID_UPDATE_BLOCK_COUNTRY_CODE, OnUpdateBlockCountryCode)
	ON_COMMAND(ID_UPDATE_CUSTOM_IP, OnUpdateCustomIP)
	ON_UPDATE_COMMAND_UI(ID_SHOW_CMD_LOG, OnUpdateShowCmdLog)
	ON_UPDATE_COMMAND_UI(ID_USE_COUNTRY_FILTER, OnUseCountryFilter)
	ON_UPDATE_COMMAND_UI(ID_ACCEPT_INVAILD_IP, OnAcceptInvalidIP)
	ON_UPDATE_COMMAND_UI(ID_UPDATE_IPtoCOUNTRY, OnEnableUpdateIPtoCountry)
	ON_UPDATE_COMMAND_UI(ID_UPDATE_BLOCK_COUNTRY_CODE, OnEnableUpdateBlockCountryCode)
	ON_UPDATE_COMMAND_UI(ID_UPDATE_CUSTOM_IP, OnEnableUpdateCustomIP)
END_MESSAGE_MAP()



// CMatchServerApp 생성
CMatchServerApp::CMatchServerApp()
{
	// TODO: 여기에 생성 코드를 추가합니다.
	// InitInstance에 모든 중요한 초기화 작업을 배치합니다.
}

CMatchServerApp::~CMatchServerApp()
{
#ifdef _MTRACEMEMORY
	MShutdownTraceMemory();
#endif

	if (m_pDocTemplateCmdLogView)
	{
		delete m_pDocTemplateCmdLogView;
	}
}

// CMatchServerApp 초기화
BOOL CMatchServerApp::InitInstance()
{
#ifdef _MTRACEMEMORY
	MInitTraceMemory();
#endif
//	_CrtSetBreakAlloc(55307);

	m_bOutputLog = 0;

	// Current Directory를 맞춘다.
	char szModuleFileName[_MAX_DIR] = {0,};
	GetModuleFileName(NULL, szModuleFileName, _MAX_DIR);
	PathRemoveFileSpec(szModuleFileName);
	SetCurrentDirectory(szModuleFileName);


//	MNewMemories::Init();

#ifdef _FETCH_112
	g_Http.Create();
#endif


	if (g_SingleRunController.Create(true) == false)
		return FALSE;


	MRegistry::szApplicationName=APPLICATION_NAME;

	if(m_ZFS.Create(".")==false){
		AfxMessageBox("MAIET Zip File System Initialize Error");
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
	CMultiDocTemplate* pDocTemplate;
	pDocTemplate = new CMultiDocTemplate(IDR_MatchServerTYPE,
		RUNTIME_CLASS(CMatchServerDoc),
		RUNTIME_CLASS(CChildFrame), // 사용자 지정 MDI 자식 프레임입니다.
		RUNTIME_CLASS(COutputView));
	AddDocTemplate(pDocTemplate);

	m_pDocTemplateOutput = pDocTemplate;

	// Template
	m_pDocTemplateOutput = pDocTemplate;
	m_pDocTemplateCmdLogView = new CMultiDocTemplate(IDR_MatchServerTYPE,
		RUNTIME_CLASS(CMatchServerDoc),
		RUNTIME_CLASS(CChildFrame), // custom MDI child frame
		RUNTIME_CLASS(CCommandLogView));


	// 주 MDI 프레임 창을 만듭니다.
	CMainFrame* pMainFrame = new CMainFrame;
	if (!pMainFrame->LoadFrame(IDR_MAINFRAME))
		return FALSE;
	m_pMainWnd = pMainFrame;
	// 접미사가 있을 경우에만 DragAcceptFiles를 호출합니다.
	// MDI 응용 프로그램에서는 m_pMainWnd를 설정한 후 바로 이러한 호출이 발생해야 합니다.
	// 표준 셸 명령, DDE, 파일 열기에 대한 명령줄을 구문 분석합니다.
	CCommandLineInfo cmdInfo;
	ParseCommandLine(cmdInfo);
	// 명령줄에 지정된 명령을 디스패치합니다. 응용 프로그램이 /RegServer, /Register, /Unregserver 또는 /Unregister로 시작된 경우 FALSE를 반환합니다.
	if (!ProcessShellCommand(cmdInfo))
		return FALSE;
	// 주 창이 초기화되었으므로 이를 표시하고 업데이트합니다.
	pMainFrame->ShowWindow(m_nCmdShow);
	pMainFrame->UpdateWindow();

	pMainFrame->m_wndConsoleBar.GetDlgItem(IDC_COMBO_COMMAND)->SetFocus();
	pMainFrame->m_wndConsoleBar.ShowWindow(SW_HIDE);


	// 디버그 모드일때는 뜨는 창이 귀찮으므로 안보이게 한다.
#ifdef _DEBUG
	m_pMainWnd->ShowWindow(SW_HIDE);
#endif

	return TRUE;
}

// CMatchServerApp 메시지 처리기
int CMatchServerApp::ExitInstance()
{
#ifdef _FETCH_112
	g_Http.Destroy();
#endif

	// TODO: Add your specialized code here and/or call the base class
	return CWinApp::ExitInstance();
}

void CMatchServerApp::HeartBeat()
{
	POSITION p = GetFirstDocTemplatePosition(); 
	CDocTemplate* pTemplate = GetNextDocTemplate(p); 
	p = pTemplate->GetFirstDocPosition(); 
	CMatchServerDoc* pDoc = (CMatchServerDoc*)pTemplate->GetNextDoc(p); 
	if(pDoc!=NULL) pDoc->Run();
	Sleep(1);


#ifdef _FETCH_112
	u32 nNowTime=timeGetTime();
	static u32 nLastTime = 0;
	//if ((nNowTime - nLastTime) >= (1000 * 60 * 5))		// 5분마다 한번씩 fetch
	if ((nNowTime - nLastTime) >= (1000 * 60  * 1))		// 5분마다 한번씩 fetch
	{
		g_Http.Query("http://192.168.0.31:8080/112.html?mode=fetch");
		nLastTime = nNowTime;
	}
#endif

}


int CMatchServerApp::Run()
{

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

		if (m_bShutdown == false)
			HeartBeat();
		else
			break;

	}
	return 0;
}

#include "MMatchStatus.h"
#include ".\matchserver.h"

void CMatchServerApp::OnViewServerStatus()
{
	// TODO: Add your command handler code here
	MMatchServer* pServer = MMatchServer::GetInstance();
	if (pServer) pServer->Log(MCommandCommunicator::LOG_ALL, "서버상태보기");

	POSITION p = GetFirstDocTemplatePosition(); 
	CDocTemplate* pTemplate = GetNextDocTemplate(p); 
	p = pTemplate->GetFirstDocPosition(); 
	CMatchServerDoc* pDoc = (CMatchServerDoc*)pTemplate->GetNextDoc(p); 

	if(pDoc!=NULL) 
	{
		pDoc->m_pMatchServer->OnViewServerStatus();
//		MNewMemories::Dump();
		MGetServerStatusSingleton()->Dump();

#ifdef _CMD_PROFILE
		pDoc->m_pMatchServer->m_CommandProfiler.Analysis();
#endif
	}
	
}

BOOL CMatchServerApp::PreTranslateMessage(MSG* pMsg)
{
	if(GetKeyState(17)<0)
	{
		if(pMsg->message==WM_KEYDOWN && pMsg->wParam=='P')
		{
			OnViewServerStatus();
			return TRUE;
		}
		if(pMsg->message==WM_KEYDOWN && pMsg->wParam=='C')	// For Crash Test
		{
			return TRUE;
		}
		if(pMsg->message==WM_KEYDOWN && pMsg->wParam=='D') // For UI Debug
		{
			MBMatchServer* pServer = (MBMatchServer*)MMatchServer::GetInstance();
			CRichEditCtrl& c = pServer->m_pView->GetRichEditCtrl();

			return TRUE;
		}
	}

	return CWinApp::PreTranslateMessage(pMsg);
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
void CMatchServerApp::OnAppAbout()
{
	CAboutDlg aboutDlg;
	aboutDlg.DoModal();
}
void CMatchServerApp::OnMessageExit()
{
	// TODO: 여기에 명령 처리기 코드를 추가합니다.
	MMatchServer* pServer = MMatchServer::GetInstance();
	if(pServer)	{
		pServer->OnAdminServerHalt();
		pServer->Log(MCommandCommunicator::LOG_ALL, "서버종료 카운트다운");
	}
}

void CMatchServerApp::OnShowCmdLog()
{
	// TODO: 여기에 명령 처리기 코드를 추가합니다.
	m_bOutputLog = !m_bOutputLog;
}

void CMatchServerApp::OnUpdateShowCmdLog(CCmdUI *pCmdUI)
{
	// TODO: 여기에 명령 업데이트 UI 처리기 코드를 추가합니다.
	pCmdUI->SetCheck(m_bOutputLog);
}

void CMatchServerApp::OnUpdateIPtoCountry()
{
	MMatchServer* pServer = MMatchServer::GetInstance();
	if(pServer)	
		pServer->UpdateIPtoCountryList();
}


void CMatchServerApp::OnUpdateBlockCountryCode()
{
	MMatchServer* pServer = MMatchServer::GetInstance();
	if(pServer)	
		pServer->UpdateBlockCountryCodeLsit();
}


void CMatchServerApp::OnUpdateCustomIP()
{
	MMatchServer* pServer = MMatchServer::GetInstance();
	if(pServer)	
		pServer->UpdateCustomIPList();
}


void CMatchServerApp::OnUseCountryFilter(CCmdUI *pCmdUI)
{
	pCmdUI->SetCheck( MGetServerConfig()->IsUseFilter() );
}


void CMatchServerApp::OnSetUseCountryFilter()
{
	MMatchServer* pServer = MMatchServer::GetInstance();
	if(pServer)	
		pServer->SetUseCountryFilter();
}


void CMatchServerApp::OnSetAccetpInvalidIP()
{
	MMatchServer* pServer = MMatchServer::GetInstance();
	if(pServer)	
		pServer->SetAccetpInvalidIP();
}


void CMatchServerApp::OnAcceptInvalidIP(CCmdUI* pCmdUI )
{
	// pCmdUI->Enable( MGetServerConfig()->IsUseFilter() );
	pCmdUI->SetCheck( MGetServerConfig()->IsAcceptInvalidIP() );
}


void CMatchServerApp::OnEnableUpdateIPtoCountry( CCmdUI* pCmdUI )
{
	pCmdUI->Enable( MGetServerConfig()->IsUseFilter() );
}


void CMatchServerApp::OnEnableUpdateBlockCountryCode( CCmdUI* pCmdUI )
{
	pCmdUI->Enable( MGetServerConfig()->IsUseFilter() );
}


void CMatchServerApp::OnEnableUpdateCustomIP( CCmdUI* pCmdUI )
{
	pCmdUI->Enable( MGetServerConfig()->IsUseFilter() );
}
#endif