#include "stdafx.h"
#include "Mcv.h"

#include "MainFrm.h"
#include "McvDoc.h"
#include "McvView.h"

#include <mmsystem.h>

#include "MDebug.h"
#include "RealSpace2.h"

#include "RMtrl.h"

#include "RMesh.h"
#include "RMeshMgr.h"

#include "RBspObject.h"
#include "RMaterialList.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

BEGIN_MESSAGE_MAP(CMcvApp, CWinApp)
	//{{AFX_MSG_MAP(CMcvApp)
	ON_COMMAND(ID_APP_ABOUT, OnAppAbout)
	ON_COMMAND(ID_FILE_NEW, CWinApp::OnFileNew)
	ON_COMMAND(ID_FILE_OPEN, CWinApp::OnFileOpen)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

CMcvApp::CMcvApp()
{
}

CMcvApp theApp;

BOOL CMcvApp::InitInstance()
{
	CoInitialize(NULL);

	AfxEnableControlContainer();

#ifdef _AFXDLL
	Enable3dControls();			
#else
	Enable3dControlsStatic();	
#endif
	
	SetRegistryKey(_T("Local AppWizard-Generated Applications"));

	LoadStdProfileSettings(0);  

	CSingleDocTemplate* pDocTemplate;
	pDocTemplate = new CSingleDocTemplate( IDR_MAINFRAME, RUNTIME_CLASS(CMcvDoc), RUNTIME_CLASS(CMainFrame), RUNTIME_CLASS(CMcvView));
	AddDocTemplate(pDocTemplate);

	CCommandLineInfo cmdInfo;
	ParseCommandLine(cmdInfo);

	if (!ProcessShellCommand(cmdInfo))	return FALSE;

	m_pMainWnd->ShowWindow(SW_SHOW);
	m_pMainWnd->UpdateWindow();

//	m_pView = (CMcvView*)m_pMainWnd->GetActiveWindow();

//	if(!m_pView->Init()) return false;

	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// CAboutDlg dialog used for App About

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

	//{{AFX_DATA(CAboutDlg)
	enum { IDD = IDD_ABOUTBOX };
	//}}AFX_DATA

	//{{AFX_VIRTUAL(CAboutDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

protected:
	//{{AFX_MSG(CAboutDlg)
		// No message handlers
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
	//{{AFX_DATA_INIT(CAboutDlg)
	//}}AFX_DATA_INIT
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAboutDlg)
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
	//{{AFX_MSG_MAP(CAboutDlg)
		// No message handlers
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

void CMcvApp::OnAppAbout()
{
	CAboutDlg aboutDlg;
	aboutDlg.DoModal();
}

CMcvView* CMcvApp::GetView()
{
	CMainFrame* pFrame = (CMainFrame*)AfxGetMainWnd();
	if (pFrame) {
		CMcvView* pView = (CMcvView*)pFrame->GetActiveView();
		if (pView)
			return pView;
	}
	return NULL;
}

/////////////////////////////////////////////////////////////////////////////
// CMcvApp message handlers

int CMcvApp::Run()
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

		HeartBeat();
	}

	return 0;
}

void CMcvApp::HeartBeat()
{
	::Sleep(10);

	CMainFrame* pFrame = (CMainFrame*)AfxGetMainWnd();
	if (pFrame) {
		CMcvView* pView = (CMcvView*)pFrame->GetActiveView();
		pView->Idle();
	}
}
/*
BOOL CMcvApp::OnIdle(LONG lCount) 
{
	if(m_pView) {
		if( ((CMainFrame*)m_pMainWnd)->m_bActivate ) {
			::Sleep(10);
			m_pView->Idle();
		}
		else {
			m_pView->Idle();
			::Sleep(20);
		}
	}

	CWinApp::OnIdle(1);

	return 1;
}
*/
int CMcvApp::ExitInstance() 
{
	CoUninitialize();
	return CWinApp::ExitInstance();
}

BOOL CAboutDlg::PreCreateWindow(CREATESTRUCT& cs)
{
	return CDialog::PreCreateWindow(cs);
}
