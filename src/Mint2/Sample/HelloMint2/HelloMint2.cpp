#include "stdafx.h"
#include "MXml.h"
#include "RealSpace2.h"
#include "RTypes.h"
#include "RFrameWork.h"
#include "Mint4R2.h"
#include "MWidget.h"
#include "Mint.h"
#include "MIDLResource.h"

#ifdef _DEBUG
#pragma comment(lib,"../../../realspace2/lib/realspace2d.lib")
#pragma comment(lib,"../../../cml/lib/cmld.lib")
#else
#pragma comment(lib,"../../../realspace2/lib/realspace2.lib")
#pragma comment(lib,"../../../cml/lib/cml.lib")
#endif

_USING_NAMESPACE_REALSPACE2

RMODEPARAMS	g_ModeParams={800,600,false,D3DFMT_R5G6B5};

#define APPLICATION_NAME		"HELLOMINT2"

RRESULT RenderScene(void *pParam);
void Reload();

class Mint4Gunz : public Mint{
public:
	virtual MBitmap* OpenBitmap(const char* szName){

		char aliasname[256];
		char drive[_MAX_DRIVE],dir[_MAX_DIR],fname[_MAX_FNAME],ext[_MAX_EXT];
		_splitpath(szName,drive,dir,fname,ext);
		sprintf(aliasname,"%s%s",fname,ext);

		MBitmapR2* pNew = new MBitmapR2;
		bool bRet = pNew->Create(aliasname, RGetDevice(), szName);
		if(bRet==false){
			delete pNew;
			return NULL;
		}
		return pNew;
	}
	virtual MFont* OpenFont(const char* szName, int nHeight){
		MFontR2* pNew = new MFontR2;
		pNew->Create(szName, szName, nHeight);
		return pNew;
	}
	/*
	virtual MWidget* NewWidget(const char* szClass, const char* szName, MWidget* pParent, MListener* pListener){
		if(strcmp(szClass, MINT_BUTTON)==0) return new ZButton(szName, pParent, pListener);
		else if( strcmp(szClass, MINT_BMBUTTON)==0) return new ZBmButton(szName, pParent, pListener);
		else if( strcmp(szClass, MINT_MSGBOX)==0) return new ZMsgBox(szName, pParent, pListener);
			return Mint::NewWidget(szClass, szName, pParent, pListener);
	}

	virtual const char* GetActionKeyName(unsigned long int nKey){
		return g_DInput.GetKeyName(nKey);
	}
	*/

} g_Mint;
MDrawContextR2* g_pDC = NULL;
MFontR2*		g_pDefFont = NULL;
MIDLResource	g_IDLResource;

RRESULT OnCreate(void *pParam)
{
//	RSetGammaRamp(Z_VIDEO_GAMMA_VALUE);
	RSetRenderFlags(RRENDER_CLEAR_BACKBUFFER);

	g_pDefFont = new MFontR2;

	if( !g_pDefFont->Create("Default", "±¼¸²", 9, 1.0f) )
		//	if( !g_pDefFont->Create("Default", RGetDevice(), "µ¸¿ò", 9, 1.0f, true, false) )
		//	if( !g_pDefFont->Create("Default", RGetDevice(), "µ¸¿ò", 14, 1.0f, true, false) )
	{
		g_pDefFont->Destroy();
		SAFE_DELETE( g_pDefFont );
		g_pDefFont	= NULL;
	}

	g_pDC = new MDrawContextR2(RGetDevice());

	g_Mint.Initialize(800, 600, g_pDC, g_pDefFont);

	g_Mint.SetWorkspaceSize(g_ModeParams.nWidth, g_ModeParams.nHeight);
	g_Mint.GetMainFrame()->SetSize(g_ModeParams.nWidth, g_ModeParams.nHeight);
	g_Mint.SetHWND(RealSpace2::g_hWnd);

	Reload();
//	g_IDLResource.LoadFromFile("main.xml");

	SetCursor(LoadCursor(NULL, IDC_ARROW));
	ShowCursor(TRUE);

	return R_OK;
}

RRESULT OnDestroy(void *pParam)
{
	SAFE_DELETE(g_pDefFont);

	g_Mint.Finalize();

	delete g_pDC;

	MFontManager::Destroy();
	MBitmapManager::Destroy();
	MBitmapManager::DestroyAniBitmap();

	return R_OK;
}

RRESULT OnUpdate(void* pParam)
{
	return R_OK;
}

RRESULT OnRender(void *pParam)
{
	if( !RIsActive() && RIsFullScreen() )
	{
		return R_NOTREADY;
	}

	g_Mint.Draw();

	return R_OK;
}

RRESULT OnInvalidate(void *pParam)
{
	return R_OK;
}

RRESULT OnRestore(void *pParam)
{
	return R_OK;
}

void Reload()
{
	MBitmapManager::Destroy();
	MBitmapManager::DestroyAniBitmap();
	MBitmapManager::Add(g_Mint.OpenBitmap("buttonframe.tga"));
	MBitmapManager::Add(g_Mint.OpenBitmap("windowframe.tga"));

	g_IDLResource.Clear();
	g_IDLResource.LoadFromFile("main.xml");

}

LRESULT FAR PASCAL WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{

		/*
	case WM_SETCURSOR:
		if(ZApplication::GetGameInterface())
			ZApplication::GetGameInterface()->ResetCursor();
		return TRUE; // prevent Windows from setting cursor to window class cursor
		*/

	case WM_KEYDOWN:
		{
			switch(wParam){
				case VK_RETURN: {
					Reload();
					return TRUE;
			   }
			}
		}
	}

	if(Mint::GetInstance()->ProcessEvent(hWnd, message, wParam, lParam)==true)
		return 0;

	return DefWindowProc(hWnd, message, wParam, lParam);
}

int PASCAL WinMain(HINSTANCE this_inst, HINSTANCE prev_inst, LPSTR cmdline, int cmdshow)
{
	RSetFunction(RF_CREATE	,	OnCreate);
	RSetFunction(RF_RENDER	,	OnRender);
	RSetFunction(RF_UPDATE	,	OnUpdate);
	RSetFunction(RF_DESTROY ,	OnDestroy);
	RSetFunction(RF_INVALIDATE,	OnInvalidate);
	RSetFunction(RF_RESTORE,	OnRestore);

	return RMain(APPLICATION_NAME,this_inst,prev_inst,cmdline,cmdshow,&g_ModeParams,WndProc,NULL);
}