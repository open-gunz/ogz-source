#include "stdafx.h"
#include "RealSpace2.h"
#include "MFile.h"
#include "MDebug.h"
#ifdef _WIN32
#include <DxErr.h>
#include "RParticleSystem.h"
#include "RFrameWork.h"
#include "RBaseTexture.h"
#include "RMeshUtil.h"
#include "RFont.h"
#include "RS2.h"
using std::min;
using std::max;
#ifdef _USE_GDIPLUS
#include "unknwn.h"
#include "gdiplus.h"
using namespace Gdiplus;
#endif
#endif

_NAMESPACE_REALSPACE2_BEGIN

static bool g_bHardwareTNL;
static bool g_bSupportVS;
static bool g_bAvailUserClipPlane;
static FullscreenType g_FullscreenMode = FullscreenType::Fullscreen;
static int g_nScreenWidth;
static int g_nScreenHeight;
static int g_nPicmip;
static RPIXELFORMAT g_PixelFormat;
MZFileSystem* g_pFileSystem;
bool DynamicResourceLoading = true;
#ifdef _WIN32
static RParticleSystem g_ParticleSystem;
HWND g_hWnd;
static HMODULE g_hD3DLibrary;
static bool g_bStencilBuffer;
static LPDIRECT3D9 g_pD3D;
static LPDIRECT3DDEVICE9 g_pd3dDevice;
static D3DADAPTER_IDENTIFIER9 g_DeviceID;
static D3DPRESENT_PARAMETERS g_d3dpp;
static D3DCAPS9 g_d3dcaps;
static D3DDISPLAYMODE g_d3ddm;
static GraphicsAPI g_GraphicsAPI;

//Fog
static float g_fFogNear;
static float g_fFogFar;
static u32 g_dwFogColor;
static bool g_bFog;

static int g_nVideoMemory;

int g_nFrameCount;
int g_nLastFrameCount;
float g_fFPS;
static auto g_dwLastFPSTime = GetGlobalTimeMS();

static bool g_bTrilinear = true;
constexpr bool bTripleBuffer = false;
static bool g_bQuery = false;

static D3DMULTISAMPLE_TYPE g_MultiSample = D3DMULTISAMPLE_NONE;

alignas(RS2) char rs2buf[sizeof(RS2)];

void RS2::OnInvalidate()
{
	Render.OnInvalidate();
}

void RS2::OnRestore()
{
	Render.OnRestore();
}

RS2& RS2::Get() { return reinterpret_cast<RS2&>(*rs2buf); }

bool IsDynamicResourceLoad() {
	return DynamicResourceLoading;
}
bool SupportsVertexShaderVersion(int Major, int Minor) {
	return g_d3dcaps.VertexShaderVersion >= D3DVS_VERSION(Major, Minor);
}
bool SupportsPixelShaderVersion(int Major, int Minor) {
	return g_d3dcaps.PixelShaderVersion >= D3DVS_VERSION(Major, Minor);
}
int MaxStreamsSupported() {
	return g_d3dcaps.MaxStreams;
}

void SetVSync(bool b)
{
	if (b)
	{
		g_d3dpp.PresentationInterval = D3DPRESENT_INTERVAL_ONE;
		g_d3dpp.FullScreen_RefreshRateInHz = D3DPRESENT_RATE_DEFAULT;
	}
	else
	{
		g_d3dpp.PresentationInterval = D3DPRESENT_INTERVAL_IMMEDIATE;
		g_d3dpp.FullScreen_RefreshRateInHz = 0;
	}
}

void RSetQuery(bool b) { g_bQuery = b; }
bool RIsQuery() { return g_bQuery; }
bool RIsFullscreen() { return g_FullscreenMode == FullscreenType::Fullscreen; }
FullscreenType	RGetFullscreenMode() { return g_FullscreenMode; }
bool RIsHardwareTNL() { return g_bHardwareTNL; }
bool RIsSupportVS() { return g_bSupportVS; }
bool RIsTrilinear() { return g_bTrilinear; }
bool RIsAvailUserClipPlane() { return g_bAvailUserClipPlane; }
int RGetScreenWidth() { return g_nScreenWidth; }
int RGetScreenHeight() { return g_nScreenHeight; }
int RGetPicmip() { return g_nPicmip; }
RPIXELFORMAT RGetPixelFormat() { return g_PixelFormat; }
LPDIRECT3DDEVICE9	RGetDevice() { return g_pd3dDevice; }
bool RIsStencilBuffer() { return g_bStencilBuffer; }
bool CheckVideoAdapterSupported();
D3DFORMAT GetDepthStencilFormat() { return g_d3dpp.AutoDepthStencilFormat; }
const D3DCAPS9 & GetDeviceCaps() { return g_d3dcaps; }
GraphicsAPI GetGraphicsAPI()
{
	return g_GraphicsAPI;
}
rmatrix RGetTransform(D3DTRANSFORMSTATETYPE ts)
{
	rmatrix mat;
	RGetDevice()->GetTransform(ts, static_cast<D3DMATRIX*>(mat));
	return mat;
}
void RSetTransform(D3DTRANSFORMSTATETYPE ts, const rmatrix & mat)
{
	RGetDevice()->SetTransform(ts, static_cast<const D3DMATRIX*>(mat));
}
static u32 g_rsnRenderFlags = RRENDER_CLEAR_BACKBUFFER;
void RSetRenderFlags(u32 nFlags) { g_rsnRenderFlags = nFlags; }
u32 RGetRenderFlags() { return g_rsnRenderFlags; }
int RGetVideoMemory() { return g_nVideoMemory; }

int RGetApproxVMem() {
	if (!g_pd3dDevice)
		return 0;
	return int(g_pd3dDevice->GetAvailableTextureMem() / 2);
}

HRESULT RError(int nErrCode)
{
	RSetError(nErrCode);
	return RFrame_Error();
}

bool CreateDirect3D9()
{
	if (!g_pD3D)
	{
		g_hD3DLibrary = LoadLibrary("d3d9.dll");

		if (!g_hD3DLibrary)
		{
			mlog("Error: Couldn't load d3d9.dll\n");
			return false;
		}

		using D3DCREATETYPE = IDirect3D9 * (__stdcall *)(UINT);
		auto d3dCreate = reinterpret_cast<D3DCREATETYPE>(GetProcAddress(g_hD3DLibrary, "Direct3DCreate9"));

		if (!d3dCreate)
		{
			mlog("Error: Couldn't get address of Direct3DCreate9\n");
			return false;
		}

		g_pD3D = d3dCreate(D3D_SDK_VERSION);

		if (!g_pD3D)
		{
			mlog("Error: Direct3DCreate9 returned null\n");
			return false;
		}
	}

	g_pD3D->GetAdapterIdentifier(0, 0, &g_DeviceID);

	return true;
}

D3DADAPTER_IDENTIFIER9*	RGetAdapterID()
{
	if (!g_pD3D) assert(false);
	return &g_DeviceID;
}

BOOL IsCompressedTextureFormatOk(D3DFORMAT TextureFormat)
{
	HRESULT hr = g_pD3D->CheckDeviceFormat(D3DADAPTER_DEFAULT,
		D3DDEVTYPE_HAL,
		g_PixelFormat,
		0,
		D3DRTYPE_TEXTURE,
		TextureFormat);

	return SUCCEEDED(hr);
}

void RSetFileSystem(MZFileSystem *pFileSystem) { g_pFileSystem = pFileSystem; }

RParticleSystem *RGetParticleSystem() { return &g_ParticleSystem; }

static void InitDevice()
{
	g_pd3dDevice->SetRenderState(D3DRS_NORMALIZENORMALS, TRUE);
	g_pd3dDevice->SetRenderState(D3DRS_LIGHTING, FALSE);
	g_pd3dDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
	g_pd3dDevice->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
	g_pd3dDevice->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
	g_pd3dDevice->SetSamplerState(0, D3DSAMP_MIPFILTER, g_bTrilinear ? D3DTEXF_LINEAR : D3DTEXF_NONE);
	g_pd3dDevice->SetSamplerState(1, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
	g_pd3dDevice->SetSamplerState(1, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
	g_pd3dDevice->SetSamplerState(1, D3DSAMP_MIPFILTER, g_bTrilinear ? D3DTEXF_LINEAR : D3DTEXF_NONE);

	g_pd3dDevice->SetRenderState(D3DRS_ALPHAFUNC, D3DCMP_NOTEQUAL);

	RSetWBuffer(true);

	float fMaxBias = -1.0f;
	g_pd3dDevice->SetSamplerState(0, D3DSAMP_MIPMAPLODBIAS, *(unsigned long*)&fMaxBias);
	g_pd3dDevice->SetSamplerState(1, D3DSAMP_MIPMAPLODBIAS, *(unsigned long*)&fMaxBias);

	g_nVideoMemory = g_pd3dDevice->GetAvailableTextureMem() / 2;

	if (D3DERR_NOTAVAILABLE == g_pd3dDevice->CreateQuery(D3DQUERYTYPE_OCCLUSION, NULL))
		g_bQuery = false;
	else
		g_bQuery = true;

	RBeginScene();
}

void CheckMipFilter()
{
	auto pd3dDevice = RGetDevice();

	pd3dDevice->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
	pd3dDevice->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
	pd3dDevice->SetSamplerState(0, D3DSAMP_MIPFILTER, D3DTEXF_LINEAR);
	pd3dDevice->SetSamplerState(1, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
	pd3dDevice->SetSamplerState(1, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
	pd3dDevice->SetSamplerState(1, D3DSAMP_MIPFILTER, D3DTEXF_LINEAR);

	pd3dDevice->SetTextureStageState(1, D3DTSS_COLORARG1, D3DTA_TEXTURE);
	pd3dDevice->SetTextureStageState(1, D3DTSS_COLOROP, D3DTOP_SELECTARG1);
	pd3dDevice->SetTextureStageState(1, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
	pd3dDevice->SetTextureStageState(1, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);

	D3DPtr<IDirect3DVertexDeclaration9> DefaultVertexDeclaration;

	D3DVERTEXELEMENT9 Elements[] = {
		{ 0, 0, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0 },
		{ 0, sizeof(float) * 3, D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 0 },
		D3DDECL_END()
	};

	RGetDevice()->CreateVertexDeclaration(Elements, MakeWriteProxy(DefaultVertexDeclaration));
	RGetDevice()->SetVertexDeclaration(DefaultVertexDeclaration.get());

	unsigned long dwNumPasses;
	HRESULT hr = pd3dDevice->ValidateDevice(&dwNumPasses);
	if (hr == D3DERR_UNSUPPORTEDTEXTUREFILTER) {
		g_bTrilinear = false;
	}
}

static void EnumAdapters()
{
	for (u32 i{}; i < g_pD3D->GetAdapterCount(); ++i)
	{
		D3DADAPTER_IDENTIFIER9 Identifier;
		g_pD3D->GetAdapterIdentifier(i, 0, &Identifier);
		MLog("Adapter %d: %s\n", i, Identifier.Description);
	}
}

static bool InitD3D9(HWND hWnd, const RMODEPARAMS* params)
{
	if (CreateDirect3D9() == false)
	{
		RError(RERROR_CANNOT_CREATE_D3D);
		return false;
	}

	if (CheckVideoAdapterSupported() == false)
	{
		if (RError(RERROR_INVALID_DEVICE) != R_OK)
			return false;
	}

	g_pD3D->GetDeviceCaps(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, &g_d3dcaps);

	auto& d3dcaps = g_d3dcaps;
	g_bHardwareTNL = (d3dcaps.DevCaps & D3DDEVCAPS_HWTRANSFORMANDLIGHT) != 0;

	g_bAvailUserClipPlane = (d3dcaps.MaxUserClipPlanes > 0) ? true : false;
	if (d3dcaps.RasterCaps & D3DPRASTERCAPS_WFOG) mlog("WFog Enabled Device.\n");

	g_bSupportVS = d3dcaps.VertexShaderVersion >= D3DVS_VERSION(1, 1);

	if (!g_bSupportVS)
	{
		mlog("Vertex shader isn't supported (expected at least 1_1, got %d_%d)\n",
			D3DSHADER_VERSION_MAJOR(g_d3dcaps.VertexShaderVersion),
			D3DSHADER_VERSION_MINOR(g_d3dcaps.VertexShaderVersion));
	}
	else
	{
		if (d3dcaps.MaxVertexShaderConst < 60)
		{
			mlog("Too small Constant Number to use Hardware Skinning so Disable Vertex Shader\n");
			g_bSupportVS = false;
		}
	}

	// Get screen information
	HRESULT hr;
	hr = g_pD3D->GetAdapterDisplayMode(D3DADAPTER_DEFAULT, &g_d3ddm);

	g_FullscreenMode = params->FullscreenMode;
	g_nScreenWidth = params->nWidth;
	g_nScreenHeight = params->nHeight;
	g_PixelFormat = g_d3ddm.Format;

	ZeroMemory(&g_d3dpp, sizeof(g_d3dpp));
	g_d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
	g_d3dpp.BackBufferWidth = params->nWidth;
	g_d3dpp.BackBufferHeight = params->nHeight;
	g_d3dpp.BackBufferCount = bTripleBuffer ? 2 : 1;
	g_d3dpp.Windowed = params->FullscreenMode != FullscreenType::Fullscreen;
	g_d3dpp.BackBufferFormat = g_PixelFormat;
	g_d3dpp.EnableAutoDepthStencil = TRUE;
	g_d3dpp.MultiSampleType = g_MultiSample;

	g_bStencilBuffer = true;
	if (FAILED(g_pD3D->CheckDeviceFormat(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, D3DFMT_X8R8G8B8, D3DUSAGE_DEPTHSTENCIL, D3DRTYPE_SURFACE, D3DFMT_D24S8)) ||
		FAILED(g_pD3D->CheckDeviceFormat(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, D3DFMT_R5G6B5, D3DUSAGE_DEPTHSTENCIL, D3DRTYPE_SURFACE, D3DFMT_D24S8)))
	{
		mlog("Does not provide D24S8 DepthStencil Buffer Format\n");
		g_bStencilBuffer = false;
	}
	else if (FAILED(g_pD3D->CheckDepthStencilMatch(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, g_PixelFormat, g_PixelFormat, D3DFMT_D24S8)))
	{
		mlog("D24S8 DepthStencil Buffer Format doesn't match for current display mode\n");
		g_bStencilBuffer = false;
	}
	if (g_bStencilBuffer)
		g_d3dpp.AutoDepthStencilFormat = D3DFMT_D24S8;
	else
		g_d3dpp.AutoDepthStencilFormat = D3DFMT_D16;


	g_d3dpp.Flags = 0;
	g_d3dpp.PresentationInterval = D3DPRESENT_INTERVAL_IMMEDIATE;

	u32 BehaviorFlags = D3DCREATE_FPU_PRESERVE |
		(g_bHardwareTNL ? D3DCREATE_HARDWARE_VERTEXPROCESSING : D3DCREATE_SOFTWARE_VERTEXPROCESSING);

	if (IsDynamicResourceLoad())
		BehaviorFlags |= D3DCREATE_MULTITHREADED;

#ifndef _NVPERFHUD
	if (FAILED(g_pD3D->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, hWnd, BehaviorFlags, &g_d3dpp, &g_pd3dDevice)))
	{
		SAFE_RELEASE(g_pD3D);
		mlog("Failed to create Direct3D9 device\n");
		return false;
	}
#else
	UINT AdapterToUse = D3DADAPTER_DEFAULT;
	D3DDEVTYPE DeviceType = D3DDEVTYPE_HAL;

	for (UINT Adapter = 0; Adapter < g_pD3D->GetAdapterCount(); Adapter++)
	{
		D3DADAPTER_IDENTIFIER9 Identifier;
		HRESULT Res = g_pD3D->GetAdapterIdentifier(Adapter, 0, &Identifier);
		if (strcmp(Identifier.Description, "NVIDIA NVPerfHUD") == 0)
		{
			AdapterToUse = Adapter;
			DeviceType = D3DDEVTYPE_REF;
			break;
		}
	}
	if (FAILED(g_pD3D->CreateDevice(AdapterToUse, DeviceType, hWnd, D3DCREATE_HARDWARE_VERTEXPROCESSING, &g_d3dpp, &g_pd3dDevice)))
	{
		mlog("can't create device\n");
		return false;
	}

#endif

	mlog("Device created\n");

	RSetViewport(0, 0, g_nScreenWidth, g_nScreenHeight);
	g_pd3dDevice->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, 0x000000, 1.0f, 0);

	CheckMipFilter();
	InitDevice();
	RFrame_Restore();
	RBaseTexture_Create();
	if (!RFontCreate())
	{
		RCloseDisplay();
		mlog("Failed to create font\n");
		return false;
	}

	RGetDevice()->Clear(0, NULL, D3DCLEAR_TARGET, 0x00000000, 1.0f, 0);
	RFlip();

	return true;
}

bool RInitDisplay(HWND hWnd, HINSTANCE inst, const RMODEPARAMS *params, GraphicsAPI API)
{
	g_GraphicsAPI = API;

	if (API == GraphicsAPI::D3D9)
	{
		InitD3D9(hWnd, params);
		new (rs2buf) RS2{ D3D9Tag{} };
	}
	else if (API == GraphicsAPI::Vulkan)
	{
		g_FullscreenMode = params->FullscreenMode;
		g_nScreenWidth = params->nWidth;
		g_nScreenHeight = params->nHeight;
		g_PixelFormat = g_d3ddm.Format;
		g_bHardwareTNL = true;
		new (rs2buf) RS2{ VulkanTag{} };
		GetRS2().CreateStatic<RS2Vulkan>(hWnd, inst);
	}
	else
	{
		MLog("Unrecognized graphics API identifier %d\n", static_cast<int>(API));
		assert(false);
		return false;
	}

	return true;
}

bool RCloseDisplay()
{
	RFontDestroy();
	RBaseTexture_Destroy();
	RFrame_Invalidate();
	RBaseTexture_Invalidate();

	if (g_pd3dDevice)
	{
		g_pd3dDevice->EndScene();
		SAFE_RELEASE(g_pd3dDevice);
	}

	SAFE_RELEASE(g_pD3D);
	// Commented out to prevent crash when in fullscreen.
	// The crash happens when the window proc code tries to call something inside the
	// now-unloaded d3d9.dll. Don't know why it doesn't unhook the window proc when the DLL
	// is unloaded.
	/*FreeLibrary(g_hD3DLibrary);
	g_hD3DLibrary = NULL;*/

	return true;
}

void RAdjustWindow(const RMODEPARAMS* ModeParams)
{
	if ((GetWindowLong(g_hWnd, GWL_STYLE) & WS_CHILD) != 0)
		return;

	SetWindowLong(g_hWnd, GWL_STYLE, GetWindowStyle(*ModeParams));

	if (ModeParams->FullscreenMode != FullscreenType::Fullscreen)
	{
		RECT rt = { 0, 0, ModeParams->nWidth, ModeParams->nHeight };
		AdjustWindowRect(&rt, GetWindowLong(g_hWnd, GWL_STYLE), FALSE);
		SetWindowPos(g_hWnd, HWND_NOTOPMOST, 0, 0, rt.right - rt.left, rt.bottom - rt.top, SWP_SHOWWINDOW);
	}
}

u32 GetWindowStyle(const RMODEPARAMS & ModeParams)
{
	switch (ModeParams.FullscreenMode)
	{
	default:
	case FullscreenType::Fullscreen:
		return WS_POPUP | WS_SYSMENU;
	case FullscreenType::Borderless:
		return WS_POPUP;
	case FullscreenType::Windowed:
		return WS_POPUP | WS_CAPTION | WS_SYSMENU;
	}
}

void RResetDevice(const RMODEPARAMS *params)
{
	mlog("Resetting device...\n");

	RFrame_Invalidate();
	RBaseTexture_Invalidate();
	RS2::Get().OnInvalidate();

	g_FullscreenMode = params->FullscreenMode;
	g_nScreenWidth = params->nWidth;
	g_nScreenHeight = params->nHeight;
	g_PixelFormat = g_d3ddm.Format;

	bool Fullscreen = params->FullscreenMode == FullscreenType::Fullscreen;
	g_d3dpp.Windowed = !Fullscreen;
	g_d3dpp.BackBufferWidth = g_nScreenWidth;
	g_d3dpp.BackBufferHeight = g_nScreenHeight;
	g_d3dpp.BackBufferFormat = g_PixelFormat;
	g_d3dpp.MultiSampleType = g_MultiSample;

	auto hr = g_pd3dDevice->Reset(&g_d3dpp);

	if (hr != D3D_OK) {
		char errstr[512];
		sprintf_safe(errstr, "Device reset failed: %s", DXGetErrorString(hr));
		mlog("%s\n", errstr);
		assert(false);
		throw std::runtime_error{ errstr };
	}

	InitDevice();

	RAdjustWindow(params);

	ShowWindow(g_hWnd, SW_SHOW);
	UpdateWindow(g_hWnd);

	RSetViewport(0, 0, g_nScreenWidth, g_nScreenHeight);

	RUpdateCamera();

	RResetTransform();

	RS2::Get().OnRestore();
	RBaseTexture_Restore();
	RFrame_Restore();

	MLog("... SUCCESS!\n");
}

RRESULT RIsReadyToRender()
{
	if (!g_pd3dDevice) return R_NOTREADY;

	HRESULT hr;
	if (FAILED(hr = g_pd3dDevice->TestCooperativeLevel()))
	{
		// If the device was lost, do not render until we get it back
		if (D3DERR_DEVICELOST == hr)
			return R_NOTREADY;

		// Check if the device needs to be resized.
		if (D3DERR_DEVICENOTRESET == hr)
			return R_RESTORED;
	}
	return R_OK;
}

#define FPS_INTERVAL 1000

static u32 g_clear_color = 0x00000000;

void SetClearColor(u32 c) {
	g_clear_color = c;
}

bool REndScene()
{
	g_pd3dDevice->EndScene();
	return true;
}

bool RBeginScene()
{
	g_pd3dDevice->BeginScene();
	return true;
}

void RFlip()
{
	REndScene();

	RFrame_PrePresent();

	g_pd3dDevice->Present(NULL, NULL, NULL, NULL);

	RClear();

	RBeginScene();

	g_nFrameCount++;
	auto currentTime = GetGlobalTimeMS();
	if (g_dwLastFPSTime + FPS_INTERVAL < currentTime)
	{
		g_fFPS = (g_nFrameCount - g_nLastFrameCount)*FPS_INTERVAL /
			((float)(currentTime - g_dwLastFPSTime) *
			(FPS_INTERVAL / 1000));
		g_dwLastFPSTime = currentTime;
		g_nLastFrameCount = g_nFrameCount;
	}
}

void RClear()
{
	if (g_rsnRenderFlags && RRENDER_CLEAR_BACKBUFFER)
		g_pd3dDevice->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, g_clear_color, 1.0f, 0L);
	else
		g_pd3dDevice->Clear(0, NULL, D3DCLEAR_ZBUFFER, g_clear_color, 1.0f, 0);
}

void RDrawLine(const rvector &v1, const rvector &v2, u32 dwColor)
{
	struct LVERTEX {
		float x, y, z;
		u32 color;
	};

	LVERTEX ver[2] = { {v1.x,v1.y,v1.z,dwColor}, {v2.x,v2.y,v2.z,dwColor} };

	HRESULT hr = RGetDevice()->DrawPrimitiveUP(D3DPT_LINELIST, 1, ver, sizeof(LVERTEX));
}

rvector RGetTransformCoord(const rvector &coord)
{
	return TransformCoord(coord, RViewProjectionViewport);
}

bool SaveMemoryBmp(int x, int y, void *data, void **retmemory, int *nsize)
{
	unsigned char *memory = NULL, *dest = NULL;

	if (!data) return false;

	int nBytesPerLine = (x * 3 + 3) / 4 * 4; // 4 byte align
	int nMemoryNeed = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + nBytesPerLine*y;
	memory = new unsigned char[nMemoryNeed];
	if (!memory) return false;

	*nsize = nMemoryNeed;
	*retmemory = memory;

	dest = memory;
	BITMAPFILEHEADER *pbmfh = (BITMAPFILEHEADER*)dest;
	dest += sizeof(BITMAPFILEHEADER);
	BITMAPINFOHEADER *pbmih = (BITMAPINFOHEADER*)dest;
	dest += sizeof(BITMAPINFOHEADER);

	// SET FILE HEADER : 14 byte
	pbmfh->bfType = 0x4D42;
	pbmfh->bfSize = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + x*y * 3;
	pbmfh->bfReserved1 = 0;
	pbmfh->bfReserved2 = 0;
	pbmfh->bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);

	// SET INFO HEADER : 40 byte
	ZeroMemory(pbmih, sizeof(BITMAPINFOHEADER));
	pbmih->biSize = sizeof(BITMAPINFOHEADER);
	pbmih->biWidth = x;
	pbmih->biHeight = y;
	pbmih->biPlanes = 1;
	pbmih->biBitCount = 24;
	pbmih->biCompression = BI_RGB;
	pbmih->biSizeImage = 0;
	pbmih->biXPelsPerMeter = 0;
	pbmih->biYPelsPerMeter = 0;
	pbmih->biClrUsed = 0;
	pbmih->biClrImportant = 0;

	int i, j;

	u32 *p = (u32*)data + (y - 1)*x;

	for (i = y - 1; i >= 0; i--)
	{
		for (j = 0; j < x; j++)
		{
			*dest++ = *((u8*)p + j * 4 + 0);
			*dest++ = *((u8*)p + j * 4 + 1);
			*dest++ = *((u8*)p + j * 4 + 2);
		}
		if (x * 3 % 4 != 0)
		{
			unsigned char zero[] = { 0,0,0,0 };
			memcpy(dest, zero, 4 - x * 3 % 4);
			dest += 4 - x * 3 % 4;
		}
		p -= x;
	}

	return true;
}


#ifdef _USE_GDIPLUS
int GetCodecClsid(const WCHAR* format, CLSID* pClsid)
{
	UINT  num = 0;          // number of image encoders
	UINT  size = 0;         // size of the image encoder array in bytes

	ImageCodecInfo* pImageCodecInfo = NULL;

	GetImageEncodersSize(&num, &size);
	if (size == 0)
		return -1;  // Failure

	pImageCodecInfo = (ImageCodecInfo*)(malloc(size));
	if (pImageCodecInfo == NULL)
		return -1;  // Failure

	GetImageEncoders(num, size, pImageCodecInfo);

	for (UINT j = 0; j < num; ++j)
	{
		if (wcscmp(pImageCodecInfo[j].MimeType, format) == 0)
		{
			*pClsid = pImageCodecInfo[j].Clsid;
			free(pImageCodecInfo);
			return j;  // Success
		}
	} // for

	free(pImageCodecInfo);
	return -1;  // Failure
} // GetCodecClsid

bool RSaveAsGeneric(int x, int y, void *data, const char *szFilename, bool JPG)
{
	// Setting up RAW Data
	BitmapData bitmapData;
	bitmapData.Width = x;
	bitmapData.Height = y;
	bitmapData.Stride = 4 * bitmapData.Width;
	bitmapData.PixelFormat = PixelFormat32bppARGB;
	bitmapData.Scan0 = data;
	bitmapData.Reserved = NULL;

	// Write to Bitmap
	Rect rect(0, 0, x, y);
	Bitmap bitmap(x, y, 4 * bitmapData.Width, PixelFormat32bppARGB, (u8*)data);
	bitmap.LockBits(&rect, ImageLockModeWrite | ImageLockModeUserInputBuf, PixelFormat32bppARGB, &bitmapData);
	bitmap.UnlockBits(&bitmapData);

	// Make WFileName
	WCHAR wstrName[512];
	int nNameLen = strlen(szFilename) + 1;
	MultiByteToWideChar(CP_ACP, 0, szFilename, -1, wstrName, nNameLen - 1);
	wstrName[nNameLen - 1] = 0;

	// Save Bitmap
	CLSID Clsid;
	int ret = GetCodecClsid(JPG ? L"image/jpeg" : L"image/png", &Clsid);
	if (ret == -1)
		return false;

	return bitmap.Save(wstrName, &Clsid, NULL) == Ok;
}

bool RSaveAsJpeg(int x, int y, void *data, const char *szFilename)
{
	return RSaveAsGeneric(x, y, data, szFilename, true);
}

bool RSaveAsPng(int x, int y, void *data, const char *szFilename)
{
	auto* ByteData = static_cast<u8*>(data);
	for (int i = 0; i < x * y; ++i)
	{
		ByteData[i * 4 + 3] = 0xFF;
	}

	return RSaveAsGeneric(x, y, data, szFilename, false);
}
#endif	// _USE_GDIPLUS

bool RSaveAsBmp(int x, int y, void *data, const char *szFilename)
{
	void *memory;
	int nSize;

	if (!SaveMemoryBmp(x, y, data, &memory, &nSize))
		return false;

	FILE *file{};
	auto ret = fopen_s(&file, szFilename, "wb+");
	if (ret != 0 || !file) return false;

	fwrite(memory, nSize, 1, file);
	fclose(file);

	delete memory;

	return true;
}

bool RScreenShot(int x, int y, void *data, const char *szFilename, ScreenshotFormatType Format)
{
	auto* Extension = GetScreenshotFormatExtension(Format);
	if (!Extension)
		return false;

	char FullFilename[MFile::MaxPath];
	sprintf_safe(FullFilename, "%s.%s", szFilename, Extension);

	switch (Format)
	{
	case ScreenshotFormatType::BMP:
		return RSaveAsBmp(x, y, data, FullFilename);
#ifdef _USE_GDIPLUS
	case ScreenshotFormatType::JPG:
		return RSaveAsJpeg(x, y, data, FullFilename);
	case ScreenshotFormatType::PNG:
		return RSaveAsPng(x, y, data, FullFilename);
#endif
	}

	return false;
}

bool RGetScreenLine(int sx, int sy, rvector *pos, rvector *dir)
{
	rvector scrpoint = rvector((float)sx, (float)sy, 0.1f);

	rmatrix inv;
	inv = Inverse(RViewProjectionViewport);

	rvector worldpoint = TransformCoord(scrpoint, inv);

	*pos = RCameraPosition;
	*dir = Normalized(worldpoint - RCameraPosition);

	return true;
}

rvector RGetIntersection(int x, int y, rplane &plane)
{
	rvector scrpoint = rvector((float)x, (float)y, 0.1f);

	rmatrix inv = Inverse(RViewProjectionViewport);

	rvector worldpoint = TransformCoord(scrpoint, inv);

	rvector ret;
	IntersectLineSegmentPlane(&ret, plane, worldpoint, RCameraPosition);

	return ret;
}

bool RGetIntersection(rvector& a, rvector& b, rplane &plane, rvector* pIntersection)
{
	rvector d = b - a;

	float t = -(plane.a * d.x + plane.b * d.y + plane.c * d.z + plane.d)
		/ (plane.a * a.x + plane.b * a.y + plane.c * a.z);

	if (t < 0.0f || t > 1.0f)
	{
		return false;
	}
	else
	{
		*pIntersection = a + t*d;
	}
	return true;
}

void RSetGammaRamp(unsigned short nGammaValue)
{
	D3DCAPS9 caps;
	RGetDevice()->GetDeviceCaps(&caps);
	if (!(caps.Caps2 & D3DCAPS2_FULLSCREENGAMMA)) return;

	D3DGAMMARAMP gramp;

	RGetDevice()->GetGammaRamp(0, &gramp);

	for (int i = 0; i < 256; i++)
	{
		gramp.red[i] = ((i * nGammaValue) > 65535 ? 65535 : (i * nGammaValue));
		gramp.green[i] = ((i * nGammaValue) > 65535 ? 65535 : (i * nGammaValue));
		gramp.blue[i] = ((i * nGammaValue) > 65535 ? 65535 : (i * nGammaValue));
	}
	RGetDevice()->SetGammaRamp(0, D3DSGR_CALIBRATE, &gramp);
}

void RDrawCylinder(rvector origin, float fRadius, float fHeight, int nSegment)
{
	RGetDevice()->SetTexture(0, NULL);
	RGetDevice()->SetTexture(1, NULL);
	RGetDevice()->SetFVF(D3DFVF_XYZ | D3DFVF_DIFFUSE);
	for (int i = 0; i < nSegment; i++)
	{
		float fAngle = i * 2 * PI_FLOAT / (float)nSegment;
		float fAngle2 = (i + 1) * 2 * PI_FLOAT / (float)nSegment;

		rvector a = fRadius*rvector(cos(fAngle), sin(fAngle), 0) + origin;
		rvector b = fRadius*rvector(cos(fAngle2), sin(fAngle2), 0) + origin;

		RDrawLine(a + rvector(0, 0, fHeight), b + rvector(0, 0, fHeight), 0xffff0000);
		RDrawLine(a - rvector(0, 0, fHeight), b - rvector(0, 0, fHeight), 0xffff0000);

		RDrawLine(a + rvector(0, 0, fHeight), a - rvector(0, 0, fHeight), 0xffff0000);
	}
}

void RDrawCorn(rvector center, rvector pole, float fRadius, int nSegment)
{
	RGetDevice()->SetTexture(0, NULL);
	RGetDevice()->SetTexture(1, NULL);
	RGetDevice()->SetFVF(D3DFVF_XYZ | D3DFVF_DIFFUSE);

	rvector dir = pole - center;
	Normalize(dir);

	rvector up = DotProduct(dir, rvector(0, 0, 1)) > DotProduct(dir, rvector(0, 1, 0)) ?
		rvector(0, 1, 0) : rvector(0, 0, 1);

	rvector x, y;
	CrossProduct(&x, up, dir);
	Normalize(x);
	CrossProduct(&y, x, dir);
	Normalize(y);

	for (int i = 0; i < nSegment; i++)
	{
		float fAngle = i * 2 * PI_FLOAT / (float)nSegment;
		float fAngle2 = (i + 1) * 2 * PI_FLOAT / (float)nSegment;

		rvector a = fRadius*(x*cos(fAngle) + y*sin(fAngle)) + center;
		rvector b = fRadius*(x*cos(fAngle2) + y*sin(fAngle2)) + center;

		RDrawLine(a, pole, 0xffff0000);
		RDrawLine(a, b, 0xffff0000);
	}
}

void RDrawSphere(rvector origin, float fRadius, int nSegment, u32 Color)
{
	RGetDevice()->SetTexture(0, NULL);
	RGetDevice()->SetTexture(1, NULL);
	RGetDevice()->SetFVF(D3DFVF_XYZ | D3DFVF_DIFFUSE);

	for (int i = 0; i < nSegment; i++)
	{
		float fAngleZ = i * 2 * PI_FLOAT / (float)nSegment;
		float fAngleZ2 = (i + 1) * 2 * PI_FLOAT / (float)nSegment;
		for (int j = 0; j < nSegment; j++)
		{
			float fAngle = j * 2 * PI_FLOAT / (float)nSegment;
			float fAngle2 = (j + 1) * 2 * PI_FLOAT / (float)nSegment;

			rvector a = fRadius * rvector(cos(fAngle) * cos(fAngleZ),
				sin(fAngle) * cos(fAngleZ),
				sin(fAngleZ)) + origin;

			rvector b = fRadius * rvector(cos(fAngle2) * cos(fAngleZ),
				sin(fAngle2) * cos(fAngleZ),
				sin(fAngleZ)) + origin;

			RDrawLine(a, b, Color);

			b = fRadius * rvector(cos(fAngle) * cos(fAngleZ2),
				sin(fAngle) * cos(fAngleZ2),
				sin(fAngleZ2)) + origin;

			RDrawLine(a, b, Color);
		}
	}
}

void RDrawAxis(rvector origin, float fSize)
{
	RGetDevice()->SetTexture(0, NULL);
	RGetDevice()->SetTexture(1, NULL);
	RGetDevice()->SetFVF(D3DFVF_XYZ | D3DFVF_DIFFUSE);

	RDrawLine(origin, origin + rvector(fSize, 0, 0), 0xffff0000);
	RDrawLine(origin, origin + rvector(0, fSize, 0), 0xff00ff00);
	RDrawLine(origin, origin + rvector(0, 0, fSize), 0xff0000ff);
}

void RDrawCircle(rvector origin, float fRadius, int nSegment)
{
	RGetDevice()->SetTexture(0, NULL);
	RGetDevice()->SetTexture(1, NULL);
	RGetDevice()->SetFVF(D3DFVF_XYZ | D3DFVF_DIFFUSE);
	for (int i = 0; i < nSegment; i++)
	{
		float fAngle = i * 2 * PI_FLOAT / (float)nSegment;
		float fAngle2 = (i + 1) * 2 * PI_FLOAT / (float)nSegment;

		rvector a = fRadius*rvector(cos(fAngle), sin(fAngle), 0) + origin;
		rvector b = fRadius*rvector(cos(fAngle2), sin(fAngle2), 0) + origin;

		RDrawLine(a, b, 0xffff0000);
	}
}

void RDrawArc(rvector origin, float fRadius, float fAngle1, float fAngle2, int nSegment, u32 color)
{
	float fAngle = fAngle2 - fAngle1;
	for (int i = 0; i < nSegment; i++)
	{
		float fAngleA = fAngle1 + (i * fAngle / (float)nSegment);
		float fAngleB = fAngle1 + ((i + 1) * fAngle / (float)nSegment);

		rvector a = fRadius*rvector(cos(fAngleA), sin(fAngleA), 0) + origin;
		rvector b = fRadius*rvector(cos(fAngleB), sin(fAngleB), 0) + origin;

		RDrawLine(a, b, color);
	}

}

void RSetWBuffer(bool bEnable)
{
	if (bEnable) {
		if (g_d3dcaps.RasterCaps & D3DPRASTERCAPS_WBUFFER) {
			RGetDevice()->SetRenderState(D3DRS_ZENABLE, D3DZB_USEW);
		}
		else {
			RGetDevice()->SetRenderState(D3DRS_ZENABLE, D3DZB_TRUE);
		}
	}
	else {
		RGetDevice()->SetRenderState(D3DRS_ZENABLE, FALSE);
	}
}

int RGetAdapterModeCount(D3DFORMAT Format, UINT Adapter)
{
	return g_pD3D->GetAdapterModeCount(Adapter, Format);
}

bool REnumAdapterMode(UINT Adapter, D3DFORMAT Format, UINT Mode, D3DDISPLAYMODE* pMode)
{
	if (pMode == 0)
		return false;
	g_pD3D->EnumAdapterModes(Adapter, Format, Mode, pMode);
	return true;
}

void RSetFog(bool bFog, float fNear, float fFar, u32 dwColor)
{
	g_bFog = bFog;
	g_pd3dDevice->SetRenderState(D3DRS_FOGENABLE, g_bFog);
	if (bFog)
	{
		g_fFogNear = fNear;
		g_fFogFar = fFar;
		g_dwFogColor = dwColor;
		g_pd3dDevice->SetRenderState(D3DRS_FOGCOLOR, dwColor);
		g_pd3dDevice->SetRenderState(D3DRS_FOGTABLEMODE, D3DFOG_LINEAR);
		g_pd3dDevice->SetRenderState(D3DRS_FOGSTART, *(u32 *)(&g_fFogNear));
		g_pd3dDevice->SetRenderState(D3DRS_FOGEND, *(u32 *)(&g_fFogFar));
	}
}

bool RGetFog() { return g_bFog; }
float RGetFogNear() { return g_fFogNear; }
float RGetFogFar() { return g_fFogFar; }
u32 RGetFogColor() { return g_dwFogColor; }


bool CheckVideoAdapterSupported()
{
	bool bSupported = true;

	D3DADAPTER_IDENTIFIER9 *ai = RGetAdapterID();
	if (ai == NULL) return false;


	if (ai->VendorId == 0x8086)
	{
		if (ai->DeviceId == 0x7125)	// 82810E
			bSupported = false;
	}

	if (ai->VendorId == 0x121a) {	// 3dfx
		bSupported = false;
	}


	return bSupported;
}
#endif

_NAMESPACE_REALSPACE2_END
