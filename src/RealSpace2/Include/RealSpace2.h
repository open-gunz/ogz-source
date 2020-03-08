#pragma once

#include "GlobalTypes.h"
#include "RMath.h"
#include "RError.h"
#include "MZFileSystem.h"
#include "RNameSpace.h"
#ifdef _WIN32
#include <d3d9.h>
#endif
_NAMESPACE_REALSPACE2_BEGIN

extern int g_nFrameCount, g_nLastFrameCount;
extern float g_fFPS;
#ifdef _WIN32
extern HWND	g_hWnd;
#endif
extern MZFileSystem *g_pFileSystem;
extern rvector RCameraPosition, RCameraDirection, RCameraUp;
extern rmatrix RView, RProjection, RViewProjection, RViewport, RViewProjectionViewport;
extern rfrustum RViewFrustum;

enum class GraphicsAPI
{
	D3D9,
	Vulkan,
};

enum RRENDER_FLAGS
{
	RRENDER_CLEAR_BACKBUFFER = 0x000001,
};

enum RFUNCTIONTYPE {
	RF_CREATE = 0,
	RF_DESTROY,
	RF_RENDER,
	RF_UPDATE,
	RF_INVALIDATE,
	RF_RESTORE,
	RF_ACTIVATE,
	RF_DEACTIVATE,
	RF_ERROR,
	RF_PREPRESENT,

	RF_ENDOFRFUNCTIONTYPE
};

enum class ScreenshotFormatType
{
	BMP,
	JPG,
	PNG,
};

inline const char* GetScreenshotFormatExtension(ScreenshotFormatType Format)
{
	switch (Format)
	{
	case ScreenshotFormatType::BMP:
		return "bmp";
	case ScreenshotFormatType::JPG:
		return "jpg";
	case ScreenshotFormatType::PNG:
		return "png";
	}

	return nullptr;
}

using RFFUNCTION = RRESULT(*)(void *Params);

bool	RIsActive();
bool	RIsQuery();
void	RSetQuery(bool b);
bool	RIsFullscreen();
FullscreenType	RGetFullscreenMode();
bool	RIsHardwareTNL();
bool	RIsSupportVS();
bool	RIsAvailUserClipPlane();
bool	RIsTrilinear();
int		RGetApproxVMem();
int		RGetScreenWidth();
int		RGetScreenHeight();
inline float RGetAspect() { return float(RGetScreenWidth()) / RGetScreenHeight(); }
int		RGetPicmip();
RPIXELFORMAT RGetPixelFormat();
void SetClearColor(u32 c);
int		RGetVideoMemory();
void	RSetWBuffer(bool bEnable);
bool	RIsStencilBuffer();
bool IsDynamicResourceLoad();
bool SupportsVertexShaderVersion(int Major, int Minor);
bool SupportsPixelShaderVersion(int Major, int Minor);
int MaxStreamsSupported();
void SetVSync(bool b);
GraphicsAPI GetGraphicsAPI();

#ifdef _WIN32
D3DADAPTER_IDENTIFIER9*	RGetAdapterID();
D3DFORMAT GetDepthStencilFormat();
const D3DCAPS9& GetDeviceCaps();

rmatrix RGetTransform(D3DTRANSFORMSTATETYPE ts);
int RGetAdapterModeCount(D3DFORMAT Format, UINT Adapter = D3DADAPTER_DEFAULT);
bool REnumAdapterMode(UINT Adapter, D3DFORMAT Format, UINT Mode, D3DDISPLAYMODE* pMode);
void RSetTransform(D3DTRANSFORMSTATETYPE ts, const rmatrix& mat);
#endif


class RParticleSystem *RGetParticleSystem();

#ifdef _WIN32
bool RInitDisplay(HWND hWnd, HINSTANCE inst, const RMODEPARAMS *params, GraphicsAPI API);
bool RCloseDisplay();
#endif
void RSetFileSystem(MZFileSystem *pFileSystem);
void RAdjustWindow(const RMODEPARAMS* ModeParams);
#ifdef GetWindowStyle
#undef GetWindowStyle
#endif
u32 GetWindowStyle(const RMODEPARAMS& ModeParams);

#ifdef _WIN32
LPDIRECT3DDEVICE9 RGetDevice();
#endif

void RResetDevice(const RMODEPARAMS *params);
RRESULT RIsReadyToRender();
void RFlip();
void RClear();
bool REndScene();
bool RBeginScene();

void RSetRenderFlags(u32 nFlags);
u32 RGetRenderFlags();

//Fog
void RSetFog(bool bFog, float fNear = 0, float fFar = 0, u32 dwColor = 0xFFFFFFFF);
bool RGetFog();
float RGetFogNear();
float RGetFogFar();
u32 RGetFogColor();

void RSetCamera(const rvector &from, const rvector &at, const rvector &up);
void RUpdateCamera();

void RSetProjection(float fFov, float fNearZ, float fFarZ);
void RSetProjection(float fFov, float fAspect, float fNearZ, float fFarZ);

inline rfrustum& RGetViewFrustum() { return RViewFrustum; }
void RSetViewport(int x1, int y1, int x2, int y2);
#ifdef _WIN32
D3DVIEWPORT9 *RGetViewport();
#endif

void RResetTransform();

bool RGetScreenLine(int sx, int sy, rvector *pos, rvector *dir);

rvector RGetIntersection(int x, int y, rplane &plane);
bool RGetIntersection(rvector& a, rvector& b, rplane &plane, rvector* pIntersection);

rvector RGetTransformCoord(const rvector &coord);

void RDrawLine(const rvector &v1, const rvector &v2, u32 dwColor);
void RDrawCylinder(rvector origin, float fRadius, float fHeight, int nSegment);
void RDrawCorn(rvector center, rvector pole, float fRadius, int nSegment);
void RDrawSphere(rvector origin, float fRadius, int nSegment, u32 Color = 0xFFFF0000);
void RDrawAxis(rvector origin, float fSize);
void RDrawCircle(rvector origin, float fRadius, int nSegment);
void RDrawArc(rvector origin, float fRadius, float fAngle1, float fAngle2, int nSegment, u32 color);

bool RSaveAsBmp(int x, int y, void *data, const char *szFilename);
bool RScreenShot(int x, int y, void *data, const char *szFilename, ScreenshotFormatType Format);

void RSetGammaRamp(unsigned short nGammaValue = 255);
void RSetWBuffer(bool bEnable);

void RSetFunction(RFUNCTIONTYPE ft, RFFUNCTION pfunc);
#ifdef _WIN32
int RMain(const char *AppName, HINSTANCE this_inst, HINSTANCE prev_inst, LPSTR cmdline,
	int cmdshow, RMODEPARAMS *pModeParams, WNDPROC winproc, WORD nIconResID,
	GraphicsAPI API);
#endif

_NAMESPACE_REALSPACE2_END