#include "MIDLResource.h"
#include <windows.h>
#include <mmsystem.h>

#include "RFont.h"

#include "MDebug.h"
#include "RealSpace2.h"

#include "MWidget.h"
#include "Mint4R2.h"
#include "MFileDialog.h"

#include <tchar.h>
#include "Mint.h"

#include "RBaseTexture.h"

#include <io.h>
#include <Windows.h>

#include "MBButtonLook.h"

#include "MHotKey.h"
#include "MPopupMenu.h"

_USING_NAMESPACE_REALSPACE2

RMODEPARAMS mparams={ 640,480,false,RPIXELFORMAT_565 };

LPDIRECT3DDEVICE8 g_dev = NULL;

int		g_id;
HWND	g_hAppWnd = NULL;
BOOL	g_bActive;


RFont g_Font;
MEdit* g_pEdit = NULL;
MLabel* g_pLabel = NULL;
MIDLResource	g_IDLResource;
MBButtonLook			m_ButtonLook;	// test


void LoadBitmap(const char* szFilter)
{
	struct _finddata_t c_file;
	intptr_t hFile;

	if( (hFile = _findfirst(szFilter, &c_file )) != -1L ){
		do{
			MBitmapR2* pBitmap = new MBitmapR2;
			if(pBitmap->Create(c_file.name, g_dev, c_file.name)==true)
				MBitmapManager::Add(pBitmap);
			else
				delete pBitmap;
		}while( _findnext( hFile, &c_file ) == 0 );
		_findclose(hFile);
	}

}

void InitBitmapArray(MBitmap** pBms, char* szPrefix, int nCount)
{
	//button00-
	char szName[32];
	for(int i=0; i<nCount; i++){
		sprintf(szName, "%s%d.png", szPrefix, i);
		pBms[i] = MBitmapManager::Get(szName);
	}
}

BOOL InitScene(HWND hWnd)
{
	InitLog(MLOGSTYLE_DEBUGSTRING|MLOGSTYLE_FILE);
	
	RInitDisplay(hWnd,&mparams);
	
	RSetCamera(rvector(0.f,100.f,-200.f),rvector(0.f,0.f,0.f),rvector(0.f,1.f,0.f));
	RSetProjection(1.5,5,1000);

	g_dev = RGetDevice();

	if(g_id == -1) return false;

#define FONT_PHYSICAL_SIZE	(12)
#define FONT_DRAW_SIZE		12
	MFontR2* pDefFont = new MFontR2;
	pDefFont->Create("Default", g_dev, "Tahoma", FONT_PHYSICAL_SIZE, FONT_DRAW_SIZE/(float)FONT_PHYSICAL_SIZE);

	MLoadDesignerMode();
	MInitialize(640, 480, pDefFont);

/*
	g_pEdit = new MEdit("Hello MINT!", MGetMainFrame(), MGetMainFrame());
	g_pEdit->SetBounds(10, 10, 200, 20);
	g_pEdit->SetFocus();
*/

	LoadBitmap("*.png");
	LoadBitmap("*.bmp");
	LoadBitmap("*.tga");


//	MCreateSample();

//	g_pLabel = new MLabel("testtest", MGetMainFrame(), MGetMainFrame());
//	g_pLabel->SetBounds(10, 100, 200, 20);
	
	// test
	g_IDLResource.LoadFromFile("main.xml");

	MFrame* pFrame = (MFrame*)g_IDLResource.FindWidget("SampleFrame");
	if (pFrame != NULL) pFrame->Show(true);

	MPopupMenu* pMenu = (MPopupMenu*)g_IDLResource.FindWidget("SamplePopup");
	if (pMenu!=NULL) 
	{
		pMenu->Show(0,0,true);
		pMenu->Show(true);
	}
/*
	class MHotKeyTest : public MLabel{
	protected:
		virtual bool OnHotKey(int nID){
			char temp[64];
			sprintf(temp, "%d HotKey Pressed", nID);
			SetText(temp);
			return true;
		}
	public:
		MHotKeyTest(const char* szName=NULL, MWidget* pParent=NULL, MListener* pListener=NULL)
			: MLabel(szName, pParent, pListener){
			}
	};

	MHotKey* pHotKey = new MHotKey("", MGetMainFrame(), MGetMainFrame());
	pHotKey->SetBounds(300, 20, 100, 20);

	MSetHWND(hWnd);
	MRegisterHotKey(MMODIFIER_CTRL, VK_RETURN);
	MRegisterHotKey(MMODIFIER_SHIFT, VK_RETURN);
	//MRegisterHotKey(NULL, VK_TAB);
	//MRegisterHotKey(MMODIFIER_ALT, VK_RETURN);

	MHotKeyTest* pTest = new MHotKeyTest("N/A", MGetMainFrame(), MGetMainFrame());
	pTest->SetBounds(0, 200, 200, 20);
*/

/*
	InitBitmapArray(m_ButtonLook.m_pUpBitmaps, "button00-", 9);
	InitBitmapArray(m_ButtonLook.m_pDownBitmaps, "button00d-", 9);
	m_ButtonLook.m_pFont = MFontManager::Get("GasinamuB9");
	
	MButton* pButton = (MButton*)g_IDLResource.FindWidget("SampleButton2");
	pButton->ChangeCustomLook(&m_ButtonLook);
//	MButton::ChangeLook(&m_ButtonLook);
*/


//	g_IDLResource.SaveToFile("savetest.xml");
/*  
	for (MWidgetList::iterator itor = g_IDLResource.GetWidgetList()->begin(); 
		itor != g_IDLResource.GetWidgetList()->end(); ++itor)
	{
		MLabel* pLabel = (MLabel*)(*itor);
		(*itor)->Show(true);
	}
*/
	//MSetDesignerMode(true);
	

	return TRUE;
}

BOOL CloseScene()
{
/*
	if(g_pEdit!=NULL){
		delete g_pEdit;
		g_pEdit = NULL;
	}
*/

	MDestroySample();

	MFinalize();

	RCloseDisplay();
	return TRUE;
}

#define IsKeyDown(key) ((GetAsyncKeyState(key) & 0x8000)!=0)
#pragma comment(lib,"winmm.lib")

BOOL RenderScene()
{
	RRESULT isOK=RIsReadyToRender();
	if(isOK==R_NOTREADY)
		return false;
	else
	if(isOK==R_RESTORED)
	{
	} // restore device dependent objects

	MRun();
	MDrawContextR2 dc(g_dev);
	MDraw(&dc);

//	MPOINT p = MEvent::GetMousePos();
//	MCursorSystem::Draw(&dc, p.x, p.y);

	RFlip();
	return TRUE;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

/*
 * Register window class for the application, and do any other
 * application initialization
 */
static BOOL FirstInstance(HINSTANCE this_inst)
{
    WNDCLASS    wc;
    wc.style = CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS;
    wc.lpfnWndProc = WndProc;
    wc.cbClsExtra = 0;
    wc.cbWndExtra = sizeof(DWORD);
    wc.hInstance = this_inst;
    wc.hIcon = NULL;//LoadIcon(this_inst, "ViewerIcon");
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
    wc.lpszMenuName = NULL;
    wc.lpszClassName = _T("RealSpace2");
	if(!RegisterClass(&wc)) return FALSE;

    g_hAppWnd = CreateWindow( _T("RealSpace2"), _T("MINT for Realspace2"),
            WS_VISIBLE | WS_POPUP , CW_USEDEFAULT, CW_USEDEFAULT, 
			640, 480, NULL, NULL, this_inst , NULL );
	ShowWindow(g_hAppWnd,SW_SHOW);
//	ShowCursor(FALSE);	// test

    if (!g_hAppWnd) return FALSE;

    return TRUE;
}

void OnLostAndResetDevice(void)
{
	for(int i=0; i<MBitmapManager::GetCount(); i++){
		MBitmapR2* pBitmap = (MBitmapR2*)MBitmapManager::Get(i);
		pBitmap->OnLostDevice();
		//pBitmap->OnResetDevice();
	}
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	
    // Handle messages
    switch (message)
    {
        case WM_KEYDOWN:
        {
            switch (wParam)
            {
				case VK_F1:
					break;

				case VK_F2:
					MSetDesignerMode(!MIsDesignerMode());
					break;

                case VK_ESCAPE:	
					PostMessage(g_hAppWnd,WM_CLOSE,0,0);
					break;
			}
        }
        break;

		case WM_SYSCHAR:
			if(wParam==VK_RETURN)
			{
				mparams.bFullScreen=!mparams.bFullScreen;
				RResetDevice(&mparams);
				LONG WndStyle = GetWindowLong(hWnd, GWL_STYLE);
				if(mparams.bFullScreen==true){
					WndStyle&=~WS_CAPTION;
					WndStyle&=~WS_BORDER;
				}
				else{
					WndStyle|=WS_CAPTION;
					WndStyle|=WS_BORDER;
				}
				SetWindowLong(hWnd, GWL_STYLE, WndStyle);

				OnLostAndResetDevice();
			}return 0;
		case WM_SYSCOMMAND:
			{
				switch (wParam)
				{
					// Trap ALT so it doesn't pause the app
					case SC_KEYMENU :
					{
						return 0;
					}
					break;
				}
			}       
        
        case WM_ACTIVATEAPP:
        {
            // Determine whether app is being activated or not
            g_bActive = (BOOL)wParam ? TRUE : FALSE;
        }
        break;

		case WM_CLOSE:
		{
            PostQuitMessage(0);
			break;
		}

    }

	MProcessEvent(hWnd, message, wParam, lParam);

    return DefWindowProc(hWnd, message, wParam, lParam);
}

int PASCAL WinMain
    (HINSTANCE this_inst, HINSTANCE prev_inst, LPSTR cmdline, int cmdshow)
{
	if(!FirstInstance(this_inst)) return 1;
	if(!InitScene(g_hAppWnd)) return 1;

	// message loop
    // Now we're ready to recieve and process Windows messages.
    BOOL bGotMsg;
    MSG  msg;
    PeekMessage( &msg, NULL, 0U, 0U, PM_NOREMOVE );

    while( WM_QUIT != msg.message  )
    {
        // Use PeekMessage() if the app is active, so we can use idle time to
        // render the scene. Else, use GetMessage() to avoid eating CPU time.
        bGotMsg = PeekMessage( &msg, NULL, 0U, 0U, PM_REMOVE );
        if( bGotMsg )
        {
            TranslateMessage( &msg );
            DispatchMessage( &msg );
        }
		RenderScene();
    }

	CloseScene();
    return (INT)msg.wParam;
}

