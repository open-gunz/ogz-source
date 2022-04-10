#include "stdafx.h"

#ifdef WIN32

#include "MDebug.h"
#include "RealSpace2.h"
#include "RParticleSystem.h"
#include "RFont.h"
#include "RMeshUtil.h"
#include "RS2.h"
#include "RS2Vulkan.h"
#include "RBspObject.h"
#include <Windows.h>

#pragma comment(lib,"winmm.lib")

#ifndef _PUBLISH
#define __BP(i,n)	MBeginProfile(i,n);
#define __EP(i)		MEndProfile(i);
#else
#define __BP(i,n) ;
#define __EP(i) ;
#endif

#define UNICODE_WINDOW

#ifdef UNICODE_WINDOW
const auto RegisterClassX = RegisterClassW;
const auto PeekMessageX = PeekMessageW;
const auto DispatchMessageX = DispatchMessageW;
#define CreateWindowX CreateWindowW
using WNDCLASSX = WNDCLASSW;
#define TEXTX(expr) L##expr
#else
const auto RegisterClassX = RegisterClassA;
const auto PeekMessageX = PeekMessageA;
const auto DispatchMessageX = DispatchMessageA;
#define CreateWindowX CreateWindowA
using WNDCLASSX = WNDCLASSA;
#define TEXTX(expr) expr
#endif

#define RTOOLTIP_GAP 700
static u64 g_last_mouse_move_time;
static bool g_tool_tip;

bool IsToolTipEnable() {
	return g_tool_tip;
}

_NAMESPACE_REALSPACE2_BEGIN

extern HWND g_hWnd;

#ifdef GAME_FOCUS_CHECK
static bool g_bActiveg_bActive;
#endif

static RECT g_rcWindowBounds;
static WNDPROC g_WinProc;
static RFFUNCTION g_pFunctions[RF_ENDOFRFUNCTIONTYPE];

#ifdef _USE_GDIPLUS
#include "unknwn.h"
#include "gdiplus.h"

static Gdiplus::GdiplusStartupInput g_gdiplusStartupInput;
static ULONG_PTR g_gdiplusToken;
#endif

void RSetFunction(RFUNCTIONTYPE ft, RFFUNCTION pfunc) {
	g_pFunctions[ft] = pfunc;
}

bool RIsActive() {
	return GetActiveWindow() == g_hWnd;
}

RRESULT RFrame_Error()
{
	if (!g_pFunctions[RF_ERROR])
		return R_OK;
	return g_pFunctions[RF_ERROR](nullptr);
}

void RFrame_Create()
{
#ifdef _USE_GDIPLUS
	Gdiplus::GdiplusStartup(&g_gdiplusToken, &g_gdiplusStartupInput, NULL);
#endif
	GetWindowRect(g_hWnd, &g_rcWindowBounds);
}

void RFrame_Restore()
{
	RParticleSystem::Restore();
	if(g_pFunctions[RF_RESTORE])
		g_pFunctions[RF_RESTORE](NULL);
}

void RFrame_Destroy()
{
	if(g_pFunctions[RF_DESTROY])
		g_pFunctions[RF_DESTROY](NULL);
	RCloseDisplay();

#ifdef _USE_GDIPLUS
	Gdiplus::GdiplusShutdown(g_gdiplusToken);
#endif
}

void RFrame_Invalidate()
{
	RParticleSystem::Invalidate();
	if(g_pFunctions[RF_INVALIDATE])
		g_pFunctions[RF_INVALIDATE](NULL);
}

void RFrame_Update()
{
	if (g_pFunctions[RF_UPDATE])
		g_pFunctions[RF_UPDATE](NULL);
}

void RFrame_RenderD3D9()
{
	RRESULT isOK = RIsReadyToRender();
	if (isOK == R_NOTREADY)
		return;

	if (isOK == R_RESTORED)
	{
		RMODEPARAMS ModeParams = {
			RGetScreenWidth(),
			RGetScreenHeight(),
			RGetFullscreenMode(),
			RGetPixelFormat()
		};
		RResetDevice(&ModeParams);
	}

	if (GetGlobalTimeMS() > g_last_mouse_move_time + RTOOLTIP_GAP)
		g_tool_tip = true;

	RRESULT ret{};
	if (g_pFunctions[RF_RENDER])
		ret = g_pFunctions[RF_RENDER](nullptr);

	RGetDevice()->SetStreamSource(0, nullptr, 0, 0);
	RGetDevice()->SetIndices(0);
	RGetDevice()->SetTexture(0, nullptr);
	RGetDevice()->SetTexture(1, nullptr);

	__BP(5007, "RFlip");
	if (ret != R_NOFLIP)
		RFlip();
	__EP(5007);
}

void RFrame_RenderVulkan()
{
	GetRS2().DrawStatic<RS2Vulkan>();
}

void RFrame_Render()
{
	if (!RIsActive() && RIsFullscreen()) return;

	if (GetRS2().UsingD3D9())
		return RFrame_RenderD3D9();

	return RFrame_RenderVulkan();
}

void RFrame_PrePresent()
{
	if (g_pFunctions[RF_PREPRESENT])
		g_pFunctions[RF_PREPRESENT](nullptr);
}

LRESULT FAR PASCAL WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	// Handle messages
	switch (message)
	{
	case WM_SYSCOMMAND:
	{
		switch (wParam)
		{
			// Trap ALT so it doesn't pause the app
		case SC_PREVWINDOW:
		case SC_NEXTWINDOW:
		case SC_KEYMENU:
		{
			return 0;
		}
		break;
		}
	}
	break;

	case WM_ACTIVATEAPP:
	{
		if (wParam == TRUE) {
			if (g_pFunctions[RF_ACTIVATE])
				g_pFunctions[RF_ACTIVATE](NULL);
			#ifdef GAME_FOCUS_CHECK
			g_bActive = true;
			#endif
		}
		else {
			if (g_pFunctions[RF_DEACTIVATE])
				g_pFunctions[RF_DEACTIVATE](NULL);

			if (RIsFullscreen()) {
				ShowWindow(hWnd, SW_MINIMIZE);
				UpdateWindow(hWnd);
			}
			#ifdef GAME_FOCUS_CHECK
			g_bActive = false;
			#endif
		}
	}
	break;

	case WM_MOUSEMOVE:
	{
		g_last_mouse_move_time = GetGlobalTimeMS();
		g_tool_tip = false;
	}
	break;

	case WM_CLOSE:
	{
		mlog("Received WM_CLOSE, exiting\n");
		RFrame_Destroy();
		PostQuitMessage(0);
	}
	break;
	}
	return g_WinProc(hWnd, message, wParam, lParam);
}

static bool RegisterWindowClass(HINSTANCE this_inst, WORD IconResID, WNDPROC WindowProc)
{
	WNDCLASSX wc;
	wc.style = CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS;
	wc.lpfnWndProc = WindowProc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = sizeof(DWORD);
	wc.hInstance = this_inst;
	wc.hIcon = LoadIcon(this_inst, MAKEINTRESOURCE(IconResID));
	wc.hCursor = 0;
	wc.hbrBackground = NULL;
	wc.lpszMenuName = NULL;
	wc.lpszClassName = TEXTX("RealSpace2");
	return RegisterClassX(&wc) != 0;
}

static bool InitGraphicsAPI(const RMODEPARAMS* ModeParams, HINSTANCE inst, HWND hWnd, GraphicsAPI API)
{
	RFrame_Create();

	ShowWindow(g_hWnd, SW_SHOW);
	if (!RInitDisplay(g_hWnd, inst, ModeParams, API))
	{
		mlog("Fatal error: Failed to initialize display\n");
		return false;
	}

	if (g_pFunctions[RF_CREATE])
	{
		if (g_pFunctions[RF_CREATE](NULL) != R_OK)
		{
			RFrame_Destroy();
			return false;
		}
	}

	return true;
}

static int RenderLoop()
{
	MSG msg;

	do
	{
		auto GotMsg = PeekMessageX(&msg, NULL, 0U, 0U, PM_REMOVE);

		if (GotMsg)
		{
			TranslateMessage(&msg);
			DispatchMessageX(&msg);
		}
		else
		{
			__BP(5006, "RMain::Run");

			RFrame_Update();
			RFrame_Render();

			__EP(5006);

			MCheckProfileCount();
		}
		#ifdef GAME_FOCUS_CHECK
		if (!g_bActive)
			Sleep(10);
		#endif
	} while (WM_QUIT != msg.message);

	return static_cast<int>(msg.wParam);
}

static bool RCreateWindow(const char *AppName, HINSTANCE this_inst, HINSTANCE prev_inst,
	RMODEPARAMS *ModeParams, WORD IconResID)
{
#ifndef UNICODE_WINDOW
	auto AdjAppName = AppName;
#else
	wchar_t AdjAppName[256];

	// CreateWindowExW for some reason takes a wchar_t* but treats it as a char*, meaning that the
	// window title gets cut off when passed a proper wchar_t since the wide ASCII values have
	// null higher bytes.
	// To prevent this, we copy the char bytes into the wchar_t array instead of converting them.
	// No idea why this happens.
	memcpy(AdjAppName, AppName, min(std::size(AdjAppName), strlen(AppName) + 1));

	/*auto len = MultiByteToWideChar(CP_UTF8, 0, AppName, -1, AdjAppName, std::size(AdjAppName));
	if (len == 0)
		return false;*/
#endif

	// Make a window
	if (!RegisterWindowClass(this_inst, IconResID, WndProc))
		return false;

	auto Style = GetWindowStyle(*ModeParams);
	g_hWnd = CreateWindowX(TEXTX("RealSpace2"), AdjAppName, Style, CW_USEDEFAULT, CW_USEDEFAULT,
		ModeParams->nWidth, ModeParams->nHeight, NULL, NULL, this_inst, NULL);

	RAdjustWindow(ModeParams);

	return true;
}

int RMain(const char *AppName, HINSTANCE this_inst, HINSTANCE prev_inst, LPSTR cmdline,
	int cmdshow, RMODEPARAMS *ModeParams, WNDPROC winproc, WORD IconResID,
	GraphicsAPI API)
{
	g_WinProc = winproc ? winproc : DefWindowProc;

	if (!RCreateWindow(AppName, this_inst, prev_inst, ModeParams, IconResID))
		return 1;

	while (ShowCursor(FALSE) > 0)
		Sleep(10);

	if (!InitGraphicsAPI(ModeParams, this_inst, g_hWnd, API))
		return 1;

	return RenderLoop();
}

_NAMESPACE_REALSPACE2_END

#endif

#undef __BP
#undef __EP