// MainFrm.cpp : implementation of the CMainFrame class
//

#include "stdafx.h"
#include "WorldEdit.h"
#include "WorldEditDoc.h"
#include "WorldEditView.h"

#include "FileInfo.h"
#include "RBspObject.h"

#include "MainFrm.h"
#include "GenerateLightmapDialog.h"
#include "GeneratePathDataDialog.h"
#include "ProgressDialog.h"
#include "InfoDialog.h"
#include ".\mainfrm.h"
#include "mmsystem.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CMainFrame

IMPLEMENT_DYNCREATE(CMainFrame, CFrameWnd)

BEGIN_MESSAGE_MAP(CMainFrame, CFrameWnd)
	//{{AFX_MSG_MAP(CMainFrame)
	ON_WM_CREATE()
	ON_COMMAND(ID_MENUITEM_MODEOBJECT, OnMenuitemModeobject)
	ON_COMMAND(ID_MENUITEM_MODEPATH, OnMenuitemModepath)
	ON_COMMAND(ID_MENUITEM_GENERATE_LIGHTMAP, OnMenuitemGenerateLightmap)
	//}}AFX_MSG_MAP
//	ON_WM_PAINT()
	ON_WM_MOVE()
	ON_WM_MOVING()
	ON_COMMAND(ID_SCREENSIZE_800, OnScreensize800)
	ON_COMMAND(ID_SCREENSIZE_1280, OnScreensize1280)
	ON_COMMAND(ID_SCREENSIZE_1024, OnScreensize1024)
	ON_STN_CLICKED(ID_MINIMAP, OnStnClickedMinimap)
//	ON_WM_SHOWWINDOW()
ON_COMMAND(ID_MENUITEM_GENERATE_PATHDATA, OnMenuitemGeneratePathdata)
ON_COMMAND(ID_VIEWINFO, OnViewinfo)
END_MESSAGE_MAP()

static UINT indicators[] =
{
	ID_SEPARATOR,           // status line indicator
	ID_INDICATOR_CAPS,
	ID_INDICATOR_NUM,
	ID_INDICATOR_SCRL,
};

/////////////////////////////////////////////////////////////////////////////
// CMainFrame construction/destruction

CMainFrame::CMainFrame()
{
	// TODO: add member initialization code here
}

CMainFrame::~CMainFrame()
{
}

int CMainFrame::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	SetTitle("WorldEdit ( build " __DATE__ " " __TIME__ " )");
	lpCreateStruct->dwExStyle |= WS_EX_ACCEPTFILES;

	if (CFrameWnd::OnCreate(lpCreateStruct) == -1)
		return -1;
	
	if (!m_wndToolBar.CreateEx(this, TBSTYLE_FLAT, WS_CHILD | WS_VISIBLE | CBRS_TOP
		| CBRS_GRIPPER | CBRS_TOOLTIPS | CBRS_FLYBY | CBRS_SIZE_DYNAMIC) ||
		!m_wndToolBar.LoadToolBar(IDR_MAINFRAME))
	{
		TRACE0("Failed to create toolbar\n");
		return -1;      // fail to create
	}

	if (!m_wndStatusBar.Create(this) ||
		!m_wndStatusBar.SetIndicators(indicators,
		  sizeof(indicators)/sizeof(UINT)))
	{
		TRACE0("Failed to create status bar\n");
		return -1;      // fail to create
	}

	// TODO: Delete these three lines if you don't want the toolbar to
	//  be dockable
	m_wndToolBar.EnableDocking(CBRS_ALIGN_ANY);
	EnableDocking(CBRS_ALIGN_ANY);
	DockControlBar(&m_wndToolBar);


	m_ObjectDialog.Create(IDD_DIALOG_OBJECTSELECT,this);
	m_ObjectDialog.Initilize();

	m_PathDialog.Create(IDD_DIALOG_PATH,this);

	m_ControlDialog.Create(IDD_DIALOG_CONTROL,this);
	PlaceControlDialog();
	m_ControlDialog.GetDlgItem(ID_MINIMAP)->EnableWindow();


	m_ControlDialog.ShowWindow(SW_SHOW);
	m_ControlDialog.UpdateWindow();


	/*
	m_ControlDialog.m_SliderCameraX.SetRange(1,100);
	m_ControlDialog.m_SliderCameraZ.SetRange(0,100);
	m_ControlDialog.m_SliderCameraZ.SetPos(50);
	*/
	
	SetPriorityClass(GetCurrentProcess(),BELOW_NORMAL_PRIORITY_CLASS);

	return 0;
}

BOOL CMainFrame::PreCreateWindow(CREATESTRUCT& cs)
{
	cs.style &= ~WS_THICKFRAME;
	if( !CFrameWnd::PreCreateWindow(cs) )
		return FALSE;
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs

	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// CMainFrame diagnostics

#ifdef _DEBUG
void CMainFrame::AssertValid() const
{
	CFrameWnd::AssertValid();
}

void CMainFrame::Dump(CDumpContext& dc) const
{
	CFrameWnd::Dump(dc);
}

#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CMainFrame message handlers


void CMainFrame::HideDialogs()
{
	m_PathDialog.ShowWindow(SW_HIDE);
	m_ObjectDialog.ShowWindow(SW_HIDE);
}

void CMainFrame::PlaceControlDialog()
{
	RECT rt;
	GetWindowRect(&rt);

	RECT dlgrt;
	m_ControlDialog.GetWindowRect(&dlgrt);

	RECT destrt;
	destrt.left=rt.right;
	destrt.top=rt.top;
	destrt.right=destrt.left+(dlgrt.right-dlgrt.left);
	destrt.bottom=destrt.top+(dlgrt.bottom-dlgrt.top);

	m_ControlDialog.MoveWindow(&destrt);
}

void CMainFrame::PlaceDialog(CDialog *pDlg)
{
	RECT rt;
	m_ControlDialog.GetWindowRect(&rt);

	RECT dlgrt;
	pDlg->GetWindowRect(&dlgrt);

	RECT destrt;
	destrt.left=rt.left;
	destrt.top=rt.bottom;
	destrt.right=destrt.left+(dlgrt.right-dlgrt.left);
	destrt.bottom=destrt.top+(dlgrt.bottom-dlgrt.top);

	pDlg->MoveWindow(&destrt);
}


void CMainFrame::OnMenuitemModeobject() 
{
	((CWorldEditView*)GetActiveView())->m_EditMode=EDITMODE_OBJECT;

	HideDialogs();
	PlaceDialog(&m_ObjectDialog);
	m_ObjectDialog.ShowWindow(SW_SHOW);
	m_ObjectDialog.UpdateWindow();
}



void CMainFrame::OnMenuitemModepath() 
{
	((CWorldEditView*)GetActiveView())->m_EditMode=EDITMODE_PATH;

	HideDialogs();
	PlaceDialog(&m_PathDialog);
	m_PathDialog.ShowWindow(SW_SHOW);
	m_PathDialog.UpdateWindow();
}

CProgressDialog *g_pDlg;
int	g_nProgressCount;
bool UpdateProgress(float fProgress)
{
	if(!g_bProgress)
		return false;

	g_nProgressCount++;
	if(g_nProgressCount%10==0)
	{
		g_pDlg->m_Progress.SetRange( 0, 1000 );
		g_pDlg->m_Progress.SetPos(int(fProgress*1000.f));
		g_pDlg->m_Progress.Invalidate();
	}
	
	return true;
}


//// thread Æã¼Ç¶«½Ã..
char lightmapfilename[256];
int nMaxSize;
int nMinSize;
int nSuperSample;
float fToler;
bool bSuccess;


UINT threadFunc( LPVOID pParam )
{
	auto* pDoc = static_cast<CWorldEditDoc*>(pParam);

	bSuccess = pDoc->m_pBspObject->GenerateLightmap(
		lightmapfilename, nMaxSize, nMinSize,
		nSuperSample, fToler, { 0, 0, 0 }, UpdateProgress);

	if(g_bProgress)
		g_pDlg->OK();

	return 0;
}

void CMainFrame::OnMenuitemGenerateLightmap() 
{
	CWorldEditDoc *pDoc=(CWorldEditDoc*)GetActiveView()->GetDocument();
	if(!pDoc) return;
	if(!pDoc->m_pBspObject) return;

	CGenerateLightmapDialog dialog;
	dialog.m_MaxSize="64";
	dialog.m_MinSize="8";
	dialog.m_SuperSample="2";
	
	rvector diff=pDoc->m_pBspObject->GetDimension();
	float fmax=max(max(diff.x,diff.y),diff.z);
	float fDefaultToler=fmax/250.f;
	dialog.m_Toler.Format("%5.1f",fDefaultToler);

	sprintf_safe(lightmapfilename,"%s.lm", static_cast<const char*>(pDoc->GetPathName()) );

	if(dialog.DoModal()==IDOK && ConfirmOverwrite(lightmapfilename))
	{
		CTime t1 = CTime::GetCurrentTime();


		nMaxSize=atoi(dialog.m_MaxSize);
		nMinSize=atoi(dialog.m_MinSize);
		nSuperSample=atoi(dialog.m_SuperSample);
		fToler=(float)atof(dialog.m_Toler);

		CProgressDialog pdlg;
		g_pDlg=&pdlg;

		CWinThread *thread=AfxBeginThread(threadFunc,pDoc);
		
		pdlg.DoModal();

		WaitForSingleObject(thread->m_hThread,INFINITE);

		sndPlaySound("done.wav",SND_ASYNC);

		if(bSuccess)
		{
			CTime t2 = CTime::GetCurrentTime();
			CTimeSpan ts = t2 - t1;
			CString s = ts.Format( "Total days: %D, hours: %H, mins: %M, secs: %S" );

			AfxMessageBox(s);
		}else
		{
			AfxMessageBox("Failed to generate lightmap, potentially check mlog.txt for info");
		}                        

		g_nProgressCount = 0;

		pDoc->ReOpen();
	}
}

void CMainFrame::OnMenuitemGeneratePathdata()
{
	CWorldEditDoc *pDoc=(CWorldEditDoc*)GetActiveView()->GetDocument();
	if(!pDoc) return;

	// TODO: Add your command handler code here
	CGeneratePathDataDialog dialog;
	dialog.m_WalkableAngle="45";
	dialog.m_Toler="0.01";

	char pathfilename[256];
	sprintf_safe(pathfilename,"%s.pat", static_cast<const char*>(pDoc->GetPathName()) );

	if(dialog.DoModal()==IDOK && ConfirmOverwrite(pathfilename))
	{
		float fAngle=(float)atof(dialog.m_WalkableAngle);
		fAngle=max(90.f-fAngle,0)/180.f*PI_FLOAT;
		float fToler=(float)atof(dialog.m_Toler);

		// TODO: Figure stuff out here
//		pDoc->m_pBspObject->GeneratePathData(pathfilename,fAngle,fToler);
	}
}

void CMainFrame::MoveDialog(CDialog *pDlg,CPoint diff)
{
	RECT dlgrt;

	pDlg->GetWindowRect(&dlgrt);
	dlgrt.left+=diff.x;dlgrt.right+=diff.x;
	dlgrt.top+=diff.y;dlgrt.bottom+=diff.y;
	pDlg->MoveWindow(&dlgrt);
}

void CMainFrame::OnMove(int x, int y)
{
	CFrameWnd::OnMove(x, y);

	if(!m_ObjectDialog.m_hWnd)
	{
		GetClientRect(&m_LastPos);
		ClientToScreen(&m_LastPos);
		return;
	}

	// TODO: Add your message handler code here
	
	CPoint diff(x-m_LastPos.left,y-m_LastPos.top);

	MoveDialog(&m_ObjectDialog,diff);
	MoveDialog(&m_PathDialog,diff);
	MoveDialog(&m_ControlDialog,diff);
}

void CMainFrame::OnMoving(UINT fwSide, LPRECT pRect)
{
	GetClientRect(&m_LastPos);
	ClientToScreen(&m_LastPos);
	CFrameWnd::OnMoving(fwSide, pRect);
	// TODO: Add your message handler code here
}

void CMainFrame::Resize(CSize size)
{
	CView *pView=GetActiveView();
	
	if(pView)
	{
		RECT crt;
		pView->GetWindowRect(&crt);
		CPoint Diff=CPoint(size.cx-(crt.right-crt.left),size.cy-(crt.bottom-crt.top));

		RECT rt;
		GetWindowRect(&rt);
		rt.right+=Diff.x;
		rt.bottom+=Diff.y;
		MoveWindow(&rt);

		PlaceControlDialog();
		((CWorldEditView*)GetActiveView())->Resize(size);
	}
}

void CMainFrame::OnScreensize800() { Resize(CSize(800,600)); }
void CMainFrame::OnScreensize1280() { Resize(CSize(1280,960)); }
void CMainFrame::OnScreensize1024() { Resize(CSize(1024,768)); }
void CMainFrame::OnStnClickedMinimap() {}

void CMainFrame::Initialize()
{
	OnScreensize1024();
	((CWorldEditView*)GetActiveView())->OnResetCamera();
}

bool CMainFrame::ConfirmOverwrite(const char *filename)
{
	if(IsExist(filename))
	{
		char buffer[256];
		sprintf(buffer,"Overwrite file : %s ?",filename);
		if(MessageBox(buffer,"Confirm",MB_YESNO | MB_ICONQUESTION ) != IDYES )
			return false;
	}

	return true;
}

void CMainFrame::OnViewinfo()
{
	CWorldEditDoc *pDoc=(CWorldEditDoc*)GetActiveView()->GetDocument();
	if(!pDoc) return;

	CInfoDialog Dlg;

	Dlg.DoModal();
}