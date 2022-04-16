#include <imgui.h>
#include "imgui_impl_dx9.h"

#include "MUtil.h"
#include "defer.h"

#include "GUI.h"
#include "Log.h"
#include "LauncherConfig.h"
#include "Patch.h"

#include "SafeString.h"
#include "FileInfo.h"
#include "FileInfo.h"
#include "MFile.h"



#define WIN32_LEAN_AND_MEAN
#include <d3d9.h>
#define DIRECTINPUT_VERSION 0x0800
#include <dinput.h>
#include <tchar.h>

namespace launcher_main
{
static LPDIRECT3DDEVICE9 g_pd3dDevice;
static D3DPRESENT_PARAMETERS g_d3dpp;
}

LRESULT ImGui_ImplDX9_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	if (ImGui_ImplDX9_WndProcHandler(hWnd, msg, wParam, lParam))
		return true;

	switch (msg)
	{
	case WM_SIZE:
		if (launcher_main::g_pd3dDevice != NULL && wParam != SIZE_MINIMIZED)
		{
			ImGui_ImplDX9_InvalidateDeviceObjects();
			launcher_main::g_d3dpp.BackBufferWidth = LOWORD(lParam);
			launcher_main::g_d3dpp.BackBufferHeight = HIWORD(lParam);
			HRESULT hr = launcher_main::g_pd3dDevice->Reset(&launcher_main::g_d3dpp);
			if (hr == D3DERR_INVALIDCALL)
				IM_ASSERT(0);
			ImGui_ImplDX9_CreateDeviceObjects();
		}
		return 0;
	case WM_SYSCOMMAND:
		if ((wParam & 0xfff0) == SC_KEYMENU) // Disable ALT application menu
			return 0;
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
	}
	return DefWindowProc(hWnd, msg, wParam, lParam);
}

// TODO: Fix during source organization
#define GUNZ_FOLDER			"/Open GunZ"
#define LAUNCHER_LOG		"/launcher_log.txt"


int PASCAL WinMain(HINSTANCE this_inst, HINSTANCE prev_inst, LPSTR cmdline, int cmdshow)
{
	auto&& AppName = LauncherConfig::ApplicationName;
	auto MsgBox = [&](const char* Msg) {
		MessageBoxA(0, Msg, AppName, 0);
	};

	std::string szLogPath = GetMyDocumentsPath();
	szLogPath += GUNZ_FOLDER;
	szLogPath += LAUNCHER_LOG;
	MakePath(szLogPath.c_str());
	if (!Log.Init(szLogPath.c_str()))
	{
		auto&& Msg = "Failed to open log file launcher_log.txt. Logging will be disabled.";
		MsgBox(Msg);
		fputs(Msg, stderr);
	}

	auto Fatal = [&](const char* Msg) {
		Log.Fatal("%s\n", Msg);
		MsgBox(Msg);
	};

	struct {
		auto size() const { return __argc; }
		auto operator[](size_t i) const { return StringView{__argv[i]}; }
	} Args;
	Options Opt;
	auto ArgRes = HandleArguments(Opt, Args);
	if (!ArgRes.Success)
	{
		Fatal(("Invalid command line arguments: " + ArgRes.ErrorMessage).c_str());
		return 1;
	}

	WNDCLASSEX wc = {sizeof(WNDCLASSEX), CS_CLASSDC, WndProc, 0L, 0L, GetModuleHandle(NULL), NULL,
		LoadCursor(NULL, IDC_ARROW), NULL, NULL, AppName, NULL};
	RegisterClassEx(&wc);
	HWND hwnd = CreateWindow(AppName, AppName, WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT,
		600, 150, NULL, NULL, wc.hInstance, NULL);

	DEFER([&] { UnregisterClass(AppName, wc.hInstance); });

	if (!hwnd)
	{
		Fatal("Failed to create window!");
		return 1;
	}

	auto d3d9_dll = LoadLibrary("d3d9.dll");

	if (!d3d9_dll)
	{
		Fatal("Couldn't load d3d9.dll");
		return 1;
	}

	DEFER([&] { FreeLibrary(d3d9_dll); });

	using D3DCreateType = IDirect3D9* (__stdcall *)(UINT);
	auto D3DCreatePtr = GetProcAddress(d3d9_dll, "Direct3DCreate9");
	auto D3DCreateFunc = reinterpret_cast<D3DCreateType>(D3DCreatePtr);

	if (!D3DCreateFunc)
	{
		Fatal("Couldn't get address of Direct3DCreate9");
		return 1;
	}

	auto D3D = D3DPtr<IDirect3D9>{D3DCreateFunc(D3D_SDK_VERSION)};

	if (!D3D)
	{
		Fatal("Direct3DCreate9 failed");
		return 1;
	}

	auto& Device = launcher_main::g_pd3dDevice;
	auto& pp = launcher_main::g_d3dpp;

	pp.Windowed = TRUE;
	pp.SwapEffect = D3DSWAPEFFECT_DISCARD;
	pp.BackBufferFormat = D3DFMT_UNKNOWN;
	pp.EnableAutoDepthStencil = TRUE;
	pp.AutoDepthStencilFormat = D3DFMT_D16;
	pp.PresentationInterval = D3DPRESENT_INTERVAL_ONE;

	if (D3D->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, hwnd,
		D3DCREATE_HARDWARE_VERTEXPROCESSING, &pp, &Device) < 0)
	{
		Fatal("Failed to create D3D device");
		return 1;
	}

	DEFER([&] { SafeRelease(Device); });

	// Setup ImGui binding
	ImGui_ImplDX9_Init(hwnd, Device);

	ImGui::GetIO().Fonts->AddFontFromFileTTF(R"(c:\Windows\Fonts\consola.ttf)", 18.0f, NULL);
	ImGui::GetIO().Fonts->AddFontFromFileTTF(R"(c:\Windows\Fonts\arial.ttf)", 18.0f, NULL);

	ImGui::GetIO().IniFilename = nullptr;

	PatchInternalState PatchState;
	std::thread{[&] { Patch(PatchState, Opt); }}.detach();

	GUIState State;

	// Main loop
	MSG msg{};
	ShowWindow(hwnd, SW_SHOWDEFAULT);
	UpdateWindow(hwnd);
	while (msg.message != WM_QUIT)
	{
		if (PeekMessage(&msg, NULL, 0U, 0U, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
			continue;
		}
		ImGui_ImplDX9_NewFrame();

		RECT Rect;
		GetClientRect(hwnd, &Rect);
		ImVec2 WindowSize{float(Rect.right - Rect.left),
			float(Rect.bottom - Rect.top)};
		auto Res = Render(State, PatchState.Load(), WindowSize);
		if (Res.ShouldExit)
		{
			break;
		}

		// Rendering
		Device->SetRenderState(D3DRS_ZENABLE, false);
		Device->SetRenderState(D3DRS_ALPHABLENDENABLE, false);
		Device->SetRenderState(D3DRS_SCISSORTESTENABLE, false);
		ImVec4 clear_col = ImColor(114, 144, 154);
		D3DCOLOR clear_col_dx = D3DCOLOR_RGBA(
			int(clear_col.x*255.0f),
			int(clear_col.y*255.0f),
			int(clear_col.z*255.0f),
			int(clear_col.w*255.0f)
		);
		Device->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, clear_col_dx, 1.0f, 0);
		if (Device->BeginScene() >= 0)
		{
			ImGui::Render();
			Device->EndScene();
		}
		Device->Present(NULL, NULL, NULL, NULL);
	}

	ImGui_ImplDX9_Shutdown();

	if (msg.message == WM_QUIT)
	{
		Log.Info("Exiting due to WM_QUIT message\n");
	}
	else
	{
		Log.Info("Exiting due to Render request\n");
	}

	return 0;
}
