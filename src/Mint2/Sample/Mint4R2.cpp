#include "Mint4R2.h"
#include "MWidget.h"
//#include <dxutil.h>

static int Floorer2PowerSize(int v)
{
	if(v<=2) return 2;
	else if(v<=4) return 4;
	else if(v<=8) return 8;
	else if(v<=16) return 16;
	else if(v<=32) return 32;
	else if(v<=64) return 64;
	else if(v<=128) return 128;
	else if(v<=256) return 256;
	else if(v<=512) return 512;
	else if(v<=1024) return 1024;

	_ASSERT(FALSE);	// Too Big!

	return 2;
}

MDrawContextR2::MDrawContextR2(LPDIRECT3DDEVICE8 pd3dDevice)
{
	m_pd3dDevice = pd3dDevice;
#ifdef _DEBUG
	m_nTypeID = MINT_R2_CLASS_TYPE;
#endif
}

MDrawContextR2::~MDrawContextR2(void)
{
}

struct PIXEL2DVERTEX { D3DXVECTOR4 p;   DWORD color; };
#define D3DFVF_PIXEL2DVERTEX (D3DFVF_XYZRHW|D3DFVF_DIFFUSE)

void MDrawContextR2::SetPixel(int x, int y, MCOLOR& color)
{
	m_pd3dDevice->SetVertexShader(D3DFVF_PIXEL2DVERTEX);
	m_pd3dDevice->SetPixelShader(NULL);

	static PIXEL2DVERTEX p;
	p.p.x = float(x);
	p.p.y = float(y);
	p.p.z = 0;
	p.p.w = 1;
	p.color = color.GetARGB();

	m_pd3dDevice->DrawPrimitiveUP(D3DPT_POINTLIST, 1, &p, sizeof(PIXEL2DVERTEX));
}

void MDrawContextR2::HLine(int x, int y, int len)
{
	Line(x, y, x+len, y);
}

void MDrawContextR2::VLine(int x, int y, int len)
{
	Line(x, y, x, y+len);
}

void MDrawContextR2::Line(int sx, int sy, int ex, int ey)
{
	m_pd3dDevice->SetVertexShader(D3DFVF_PIXEL2DVERTEX);
	m_pd3dDevice->SetPixelShader(NULL);

	sx += m_Origin.x;
	sy += m_Origin.y;
	ex += m_Origin.x;
	ey += m_Origin.y;

	DWORD argb = m_Color.GetARGB();
	static PIXEL2DVERTEX p[2];
	p[0].p.x = float(sx);
	p[0].p.y = float(sy);
	p[0].p.z = 0;
	p[0].p.w = 1;
	p[0].color = argb;
	p[1].p.x = float(ex);
	p[1].p.y = float(ey);
	p[1].p.z = 0;
	p[1].p.w = 1;
	p[1].color = argb;

    m_pd3dDevice->SetRenderState( D3DRS_ALPHABLENDENABLE, TRUE);
    m_pd3dDevice->SetRenderState( D3DRS_SRCBLEND,   D3DBLEND_SRCALPHA );
    m_pd3dDevice->SetRenderState( D3DRS_DESTBLEND,  D3DBLEND_INVSRCALPHA );
    m_pd3dDevice->SetRenderState( D3DRS_ALPHATESTENABLE,  FALSE );

	m_pd3dDevice->SetTexture(0, NULL);

	m_pd3dDevice->DrawPrimitiveUP(D3DPT_LINELIST, 1, p, sizeof(PIXEL2DVERTEX));
}

void MDrawContextR2::Rectangle(int x, int y, int cx, int cy)
{
	x += m_Origin.x;
	y += m_Origin.y;

	DWORD argb = m_Color.GetARGB();
	static PIXEL2DVERTEX p[5];
	p[0].p.x = float(x);
	p[0].p.y = float(y);
	p[0].p.z = 0;
	p[0].p.w = 1;
	p[0].color = argb;
	p[1].p.x = float(x+cx);
	p[1].p.y = float(y);
	p[1].p.z = 0;
	p[1].p.w = 1;
	p[1].color = argb;
	p[2].p.x = float(x+cx);
	p[2].p.y = float(y+cy);
	p[2].p.z = 0;
	p[2].p.w = 1;
	p[2].color = argb;
	p[3].p.x = float(x);
	p[3].p.y = float(y+cy);
	p[3].p.z = 0;
	p[3].p.w = 1;
	p[3].color = argb;
	p[4].p.x = float(x);
	p[4].p.y = float(y);
	p[4].p.z = 0;
	p[4].p.w = 1;
	p[4].color = argb;

	m_pd3dDevice->SetVertexShader(D3DFVF_PIXEL2DVERTEX);
	m_pd3dDevice->SetPixelShader(NULL);

    m_pd3dDevice->SetRenderState( D3DRS_ALPHABLENDENABLE, TRUE);
    m_pd3dDevice->SetRenderState( D3DRS_SRCBLEND,   D3DBLEND_SRCALPHA );
    m_pd3dDevice->SetRenderState( D3DRS_DESTBLEND,  D3DBLEND_INVSRCALPHA );
    m_pd3dDevice->SetRenderState( D3DRS_ALPHATESTENABLE,  FALSE );

	m_pd3dDevice->SetTexture(0, NULL);

	m_pd3dDevice->DrawPrimitiveUP(D3DPT_LINESTRIP, 4, p, sizeof(PIXEL2DVERTEX));

}

void MDrawContextR2::FillRectangle(int x, int y, int cx, int cy)
{
	m_pd3dDevice->SetVertexShader(D3DFVF_PIXEL2DVERTEX);
	m_pd3dDevice->SetPixelShader(NULL);

	x += m_Origin.x;
	y += m_Origin.y;

	DWORD argb = m_Color.GetARGB();
	static PIXEL2DVERTEX p[4];
	p[0].p.x = float(x);
	p[0].p.y = float(y);
	p[0].p.z = 0;
	p[0].p.w = 1;
	p[0].color = argb;

	p[1].p.x = float(x+cx);
	p[1].p.y = float(y);
	p[1].p.z = 0;
	p[1].p.w = 1;
	p[1].color = argb;

	p[2].p.x = float(x+cx);
	p[2].p.y = float(y+cy);
	p[2].p.z = 0;
	p[2].p.w = 1;
	p[2].color = argb;

	p[3].p.x = float(x);
	p[3].p.y = float(y+cy);
	p[3].p.z = 0;
	p[3].p.w = 1;
	p[3].color = argb;

    m_pd3dDevice->SetRenderState( D3DRS_ALPHABLENDENABLE, TRUE);
    m_pd3dDevice->SetRenderState( D3DRS_SRCBLEND,   D3DBLEND_SRCALPHA );
    m_pd3dDevice->SetRenderState( D3DRS_DESTBLEND,  D3DBLEND_INVSRCALPHA );
    m_pd3dDevice->SetRenderState( D3DRS_ALPHATESTENABLE,  FALSE );

	m_pd3dDevice->SetTexture(0, NULL);

	m_pd3dDevice->DrawPrimitiveUP(D3DPT_TRIANGLEFAN, 2, p, sizeof(PIXEL2DVERTEX));
}

void MDrawContextR2::Draw(int x, int y, bool bSprite, unsigned char nOpacity)
{
	MBitmapR2* pBitmap = (MBitmapR2*)m_pBitmap;
	if(pBitmap==NULL) return;

	_ASSERT(pBitmap->m_nTypeID==MINT_R2_CLASS_TYPE);

	// Sprite는 현재 뷰포트의 시작점을 0, 0으로 인식
	/*
	x += m_Origin.x;
	y += m_Origin.y;
	*/

	MCOLOR color(0xFF, 0xFF, 0xFF, nOpacity);
	// 텍스쳐가 2^n형태로 만들어지므로, 이에 맞게 스케일 조정을 해줘야 한다.
	float fws = pBitmap->GetWidth()/(float)Floorer2PowerSize(pBitmap->GetWidth());
	float fhs = pBitmap->GetHeight()/(float)Floorer2PowerSize(pBitmap->GetHeight());
	D3DXVECTOR2 Trans((float)x, (float)y);
	D3DXVECTOR2 Scaling(fws, fhs);
	pBitmap->m_pSprite->Begin();
	pBitmap->m_pSprite->Draw(pBitmap->m_pTexture, NULL, &Scaling, NULL, 0, &Trans, color.GetARGB());
	//pBitmap->m_pSprite->Draw(pBitmap->m_pTexture, NULL, NULL, NULL, 0, &Trans, 0xFFFFFFFF);
	pBitmap->m_pSprite->End();
}

void MDrawContextR2::Draw(int x, int y, int w, int h, unsigned char nOpacity)
{
	MBitmapR2* pBitmap = (MBitmapR2*)m_pBitmap;
	if(pBitmap==NULL) return;

	_ASSERT(pBitmap->m_nTypeID==MINT_R2_CLASS_TYPE);

	// Sprite는 현재 뷰포트의 시작점을 0, 0으로 인식
	/*
	x += m_Origin.x;
	y += m_Origin.y;
	*/

	MCOLOR color(0xFF, 0xFF, 0xFF, nOpacity);
	// 텍스쳐가 2^n형태로 만들어지므로, 이에 맞게 스케일 조정을 해줘야 한다.
	float fws = pBitmap->GetWidth()/(float)Floorer2PowerSize(pBitmap->GetWidth());
	float fhs = pBitmap->GetHeight()/(float)Floorer2PowerSize(pBitmap->GetHeight());
	D3DXVECTOR2 Trans((float)x, (float)y);
	D3DXVECTOR2 Scaling(fws*w/(float)pBitmap->GetWidth(), fhs*h/(float)pBitmap->GetHeight());
	pBitmap->m_pSprite->Begin();
	pBitmap->m_pSprite->Draw(pBitmap->m_pTexture, NULL, &Scaling, NULL, 0, &Trans, color.GetARGB());
	pBitmap->m_pSprite->End();
}

int MDrawContextR2::Text(int x, int y, const char* szText)
{
	MFontR2* pFont = (MFontR2*)m_pFont;
	if(m_pFont==NULL) pFont = (MFontR2*)MFontManager::Get(NULL);

	_ASSERT(pFont->m_nTypeID==MINT_R2_CLASS_TYPE);

	x += m_Origin.x;
	y += m_Origin.y;

	pFont->m_Font.DrawText((float)x, (float)y, szText, m_Color.GetARGB(), pFont->m_fScale);

	return 0;
}

void MDrawContextR2::SetClipRect(MRECT& r)
{
	MDrawContext::SetClipRect(r);

	// DX Clipping이 한픽셀 삑사리가 나서 보정
	r.w+=1;
	r.h+=1;

	if(r.x<0){
		r.w += r.x;
		r.x = 0;
	}
	if(r.y<0){
		r.h += r.y;
		r.y = 0;
	}
	if(r.x+r.w>=MGetWorkspaceWidth()) r.w = MGetWorkspaceWidth() - r.x;
	if(r.y+r.h>=MGetWorkspaceHeight()) r.h = MGetWorkspaceHeight() - r.y;

	D3DVIEWPORT8 vp;
	vp.X = r.x;
	vp.Y = r.y;
	vp.Width = r.w;
	vp.Height = r.h;
	vp.MaxZ = 1;
	vp.MinZ = 0;

	m_pd3dDevice->SetViewport(&vp);
}


MBitmapR2::MBitmapR2(void)
{
#ifdef _DEBUG
	m_nTypeID = MINT_R2_CLASS_TYPE;
#endif
	m_nWidth = m_nHeight = 0;
	m_pd3dDevice = NULL;
	m_pSprite = NULL;
	m_pTexture = NULL;
}

MBitmapR2::~MBitmapR2(void)
{
	Destroy();
}

bool MBitmapR2::Create(const char* szAliasName, LPDIRECT3DDEVICE8 pd3dDevice, const char* szFileName)
{
	MBitmap::Create(szAliasName);

	if( FAILED(D3DXCreateTextureFromFileEx(pd3dDevice, szFileName, D3DX_DEFAULT, D3DX_DEFAULT, D3DX_DEFAULT, 0, 
				D3DFMT_UNKNOWN, D3DPOOL_MANAGED, D3DX_FILTER_TRIANGLE|D3DX_FILTER_MIRROR, D3DX_FILTER_TRIANGLE|D3DX_FILTER_MIRROR, 
				0, &m_Info, NULL, &m_pTexture))) return false;

	if(FAILED(D3DXCreateSprite(pd3dDevice, &m_pSprite))) return false;

	m_pd3dDevice = pd3dDevice;

	return true;
}

bool MBitmapR2::Create(const char* szAliasName, LPDIRECT3DDEVICE8 pd3dDevice, void* pData, int nSize)
{
	MBitmap::Create(szAliasName);

	if( FAILED(D3DXCreateTextureFromFileInMemoryEx(pd3dDevice,(void*)pData, nSize, D3DX_DEFAULT, D3DX_DEFAULT, D3DX_DEFAULT, 0, 
				D3DFMT_UNKNOWN, D3DPOOL_MANAGED, D3DX_FILTER_TRIANGLE|D3DX_FILTER_MIRROR, D3DX_FILTER_TRIANGLE|D3DX_FILTER_MIRROR, 
				0, &m_Info, NULL, &m_pTexture ))) return false;

	if(FAILED(D3DXCreateSprite(pd3dDevice, &m_pSprite))) return false;

	m_pd3dDevice = pd3dDevice;

	return true;
}

void MBitmapR2::Destroy(void)
{
	SAFE_RELEASE(m_pSprite);
	SAFE_RELEASE(m_pTexture);
}

int MBitmapR2::GetWidth(void)
{
	return m_Info.Width;
}

int MBitmapR2::GetHeight(void)
{
	return m_Info.Height;
}

void MBitmapR2::OnLostDevice(void)
{
	if(m_pSprite!=NULL) m_pSprite->OnLostDevice();
}

void MBitmapR2::OnResetDevice(void)
{
	if(m_pSprite!=NULL) m_pSprite->OnResetDevice();
}

MFontR2::MFontR2(void)
{
#ifdef _DEBUG
	m_nTypeID = MINT_R2_CLASS_TYPE;
#endif
	m_fScale = 1.0f;
}

MFontR2::~MFontR2(void)
{
}

bool MFontR2::Create(const char* szAliasName, LPDIRECT3DDEVICE8 pd3dDevice, const char* szFontName, int nHeight, float fScale, bool bBold, bool bItalic, int nCacheSize)
{
	m_fScale = fScale;
	MFont::Create(szAliasName);
	return m_Font.Create(pd3dDevice, szFontName, nHeight, bBold, bItalic, nCacheSize);
}

void MFontR2::Destroy(void)
{
	m_Font.Destroy();
}

int MFontR2::GetHeight(void)
{
	return int(m_Font.GetHeight()*m_fScale);
}

int MFontR2::GetWidth(const char* szText)
{
	return int(m_Font.GetTextWidth(szText)*m_fScale);
}
