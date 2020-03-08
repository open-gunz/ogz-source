#include "stdafx.h"

#include "Mint4R2.h"
#include "MWidget.h"
#include "Mint.h"

_USING_NAMESPACE_REALSPACE2

MDrawContextR2::MDrawContextR2(LPDIRECT3DDEVICE9 pd3dDevice)
{
	m_pd3dDevice = pd3dDevice;
#ifdef _DEBUG
	m_nTypeID = MINT_R2_CLASS_TYPE;
#endif
}

MDrawContextR2::~MDrawContextR2()
{
}

struct PIXEL2DVERTEX { v4 p;   DWORD color; };
#define D3DFVF_PIXEL2DVERTEX (D3DFVF_XYZRHW|D3DFVF_DIFFUSE)

void MDrawContextR2::SetPixel(int x, int y, MCOLOR& color)
{
	m_pd3dDevice->SetFVF(D3DFVF_PIXEL2DVERTEX);
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
	m_pd3dDevice->SetFVF(D3DFVF_PIXEL2DVERTEX);
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
	m_pd3dDevice->SetRenderState( D3DRS_ZWRITEENABLE,     FALSE );

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

	m_pd3dDevice->SetFVF(D3DFVF_PIXEL2DVERTEX);
	m_pd3dDevice->SetPixelShader(NULL);

    m_pd3dDevice->SetRenderState( D3DRS_ALPHABLENDENABLE, TRUE);
    m_pd3dDevice->SetRenderState( D3DRS_SRCBLEND,   D3DBLEND_SRCALPHA );
    m_pd3dDevice->SetRenderState( D3DRS_DESTBLEND,  D3DBLEND_INVSRCALPHA );
    m_pd3dDevice->SetRenderState( D3DRS_ALPHATESTENABLE,  FALSE );
	m_pd3dDevice->SetRenderState( D3DRS_ZWRITEENABLE,     FALSE );

	m_pd3dDevice->SetTexture(0, NULL);

	m_pd3dDevice->DrawPrimitiveUP(D3DPT_LINESTRIP, 4, p, sizeof(PIXEL2DVERTEX));

}

void MDrawContextR2::FillRectangle(int x, int y, int cx, int cy)
{
	m_pd3dDevice->SetFVF(D3DFVF_PIXEL2DVERTEX);
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
	m_pd3dDevice->SetRenderState( D3DRS_ZWRITEENABLE,     FALSE );
	m_pd3dDevice->SetRenderState( D3DRS_FOGENABLE, FALSE );
	m_pd3dDevice->SetRenderState( D3DRS_CULLMODE, D3DCULL_NONE );

	RGetDevice()->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_SELECTARG1);
	RGetDevice()->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_DIFFUSE);
	RGetDevice()->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);
	RGetDevice()->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_DIFFUSE);
	
	m_pd3dDevice->SetTexture(0, NULL);
	 
	HRESULT hr = m_pd3dDevice->DrawPrimitiveUP(D3DPT_TRIANGLEFAN, 2, p, sizeof(PIXEL2DVERTEX));
	_ASSERT(hr==D3D_OK);
}

void MDrawContextR2::Draw(MBitmap *pBitmap, int x, int y, int w, int h, int sx, int sy, int sw, int sh)
{
	if(pBitmap==NULL) return;

	_ASSERT(pBitmap->m_nTypeID==MINT_R2_CLASS_TYPE);

	if(x+m_Origin.x>m_Clip.x+m_Clip.w || y+m_Origin.y>m_Clip.y+m_Clip.h || x+w+m_Origin.x<m_Clip.x || y+h+m_Origin.y<m_Clip.y) return;

	MCOLOR color(m_BitmapColor.r,m_BitmapColor.g,m_BitmapColor.b,m_nOpacity);

	((MBitmapR2*)pBitmap)->Draw((float)x+m_Origin.x, (float)y+m_Origin.y, (float)w, (float)h, (float)sx, (float)sy, (float)sw, (float)sh, color.GetARGB(), m_Effect);
}


void MDrawContextR2::DrawEx(int tx1, int ty1, int tx2, int ty2, int tx3, int ty3, int tx4, int ty4)
{
	MBitmapR2* pBitmap = (MBitmapR2*)m_pBitmap;
	if(pBitmap==NULL) return;
	_ASSERT(pBitmap->m_nTypeID==MINT_R2_CLASS_TYPE);
	MCOLOR color(0xFF, 0xFF, 0xFF, m_nOpacity);

	pBitmap->DrawEx((float)tx1+m_Origin.x, (float)ty1+m_Origin.y,
		            (float)tx2+m_Origin.x, (float)ty2+m_Origin.y,
					(float)tx3+m_Origin.x, (float)ty3+m_Origin.y,
					(float)tx4+m_Origin.x, (float)ty4+m_Origin.y, color.GetARGB(), m_Effect);
}

bool MDrawContextR2::BeginFont()
{
	MFontR2* pFont = (MFontR2*)m_pFont;

	if(m_pFont==NULL)
		pFont = (MFontR2*)MFontManager::Get(NULL);

	_ASSERT(pFont->m_nTypeID==MINT_R2_CLASS_TYPE);

	return pFont->m_Font.BeginFont();
}

bool MDrawContextR2::EndFont()
{
	MFontR2* pFont = (MFontR2*)m_pFont;

	if(m_pFont==NULL) 
		pFont = (MFontR2*)MFontManager::Get(NULL);

	_ASSERT(pFont->m_nTypeID==MINT_R2_CLASS_TYPE);

	return pFont->m_Font.EndFont();
}

int MDrawContextR2::Text(int x, int y, const char* szText)
{
	MFontR2* pFont = (MFontR2*)m_pFont;

	if(m_pFont==NULL) 
		pFont = (MFontR2*)MFontManager::Get(NULL);

	_ASSERT(pFont->m_nTypeID==MINT_R2_CLASS_TYPE);

	x += m_Origin.x;
	y += m_Origin.y;

	DWORD dwColor = m_Color.GetARGB();
	if(pFont->m_nOutlineStyle==1)
		dwColor = 0xffffffff;

	pFont->m_Font.DrawText((float)x, (float)y, szText, dwColor , pFont->m_fScale);
	return 0;
}

void MDrawContextR2::SetClipRect(MRECT& r)
{
	MDrawContext::SetClipRect(r);

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

	D3DVIEWPORT9 vp;
	vp.X = r.x;
	vp.Y = r.y;
	vp.Width = r.w;
	vp.Height = r.h;
	vp.MaxZ = 1;
	vp.MinZ = 0;

	HRESULT hr = m_pd3dDevice->SetViewport(&vp);
	_ASSERT(hr==D3D_OK);
}

#define RELATIVE_VARS() \
	int correct_w = MGetCorrectedWorkspaceWidth(); \
	int correct_h = MGetWorkspaceHeight(); \
	int start = (MGetWorkspaceWidth() - correct_w) / 2

#define RELATIVE_VARS_M() RELATIVE_VARS(); float m = correct_w / 800.0f;

void MDrawContextR2::DrawRelative(float x, float y, float w, float h)
{
	RELATIVE_VARS_M();
	MDrawContext::Draw(x * correct_w + start, y * correct_h, w * m, h * m);
}
void MDrawContextR2::DrawRelative(float x, float y, int w, int h)
{
	RELATIVE_VARS();
	MDrawContext::Draw(x * correct_w + start, y * correct_h, w, h);
}
int MDrawContextR2::TextRelative(float x, float y, const char* szText)
{
	RELATIVE_VARS();
	return Text(x * correct_w + start, y * correct_h, szText);
}
void MDrawContextR2::DrawRelative(float x, float y, float w, float h, int sx, int sy, int sw, int sh)
{
	RELATIVE_VARS_M();
	MDrawContext::Draw(x * correct_w + start, y * correct_h, w * m, h * m, sx, sy, sw, sh);
}
void MDrawContextR2::FillRectangleRelative(float x, float y, float cx, float cy)
{
	RELATIVE_VARS();
	FillRectangle(x * correct_w + start, y * correct_h, cx * correct_w, cy * correct_h);
}

MBitmapR2::MBitmapR2()
{
#ifdef _DEBUG
	m_nTypeID = MINT_R2_CLASS_TYPE;
#endif
	m_pd3dDevice = NULL;
	m_pTexture = NULL;

	m_dwStateBlock = NULL;
}

MBitmapR2::~MBitmapR2()
{
	Destroy();
}

bool MBitmapR2::Create(const StringView& szAliasName, LPDIRECT3DDEVICE9 pd3dDevice,
	const StringView& szFileName, bool bUseFileSystem)
{
	MBitmap::Create(szAliasName);

	m_pTexture = RCreateBaseTexture(szFileName, RTextureType::Etc, false, bUseFileSystem);
	if(!m_pTexture) return false;

	m_pd3dDevice = pd3dDevice;

	return true;
}

void MBitmapR2::Destroy()
{
	if(m_pTexture)
	{
		RDestroyBaseTexture(m_pTexture);
		m_pTexture = NULL;
	}
}

int MBitmapR2::GetWidth()
{
	if(!m_pTexture) 
		return 0;
	return m_pTexture->GetWidth();
}

int MBitmapR2::GetHeight()
{
	if(!m_pTexture) 
		return 0;

	return m_pTexture->GetHeight();
}

u32 MBitmapR2::m_dwStateBlock;

void MBitmapR2::BeginState(MDrawEffect effect)
{
	m_pd3dDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);

	switch (effect)
	{
	case MDE_NORMAL:
	{
		m_pd3dDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
		m_pd3dDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
	}
	break;
	case MDE_ADD:
	{
		m_pd3dDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
		m_pd3dDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_ONE);
	}
	break;
	case MDE_MULTIPLY:
	{
		m_pd3dDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_ZERO);
		m_pd3dDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_SRCCOLOR);
	}
	break;

	}
	m_pd3dDevice->SetRenderState(D3DRS_ALPHATESTENABLE, FALSE);
	m_pd3dDevice->SetRenderState(D3DRS_ALPHAREF, 0x08);
	m_pd3dDevice->SetRenderState(D3DRS_ALPHAFUNC, D3DCMP_GREATEREQUAL);

	const bool bFiltering = false;

	m_pd3dDevice->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
	m_pd3dDevice->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
	m_pd3dDevice->SetSamplerState(0, D3DSAMP_MIPFILTER, D3DTEXF_LINEAR);

	m_pd3dDevice->SetRenderState(D3DRS_LIGHTING, FALSE);
	m_pd3dDevice->SetRenderState(D3DRS_FILLMODE, D3DFILL_SOLID);
	m_pd3dDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_CCW);


	m_pd3dDevice->SetRenderState(D3DRS_STENCILENABLE, FALSE);
	m_pd3dDevice->SetRenderState(D3DRS_CLIPPING, TRUE);
	m_pd3dDevice->SetRenderState(D3DRS_CLIPPLANEENABLE, FALSE);
	m_pd3dDevice->SetRenderState(D3DRS_VERTEXBLEND, FALSE);
	m_pd3dDevice->SetRenderState(D3DRS_INDEXEDVERTEXBLENDENABLE, FALSE);
	m_pd3dDevice->SetRenderState(D3DRS_FOGENABLE, FALSE);
	m_pd3dDevice->SetRenderState(D3DRS_ZWRITEENABLE, FALSE);


	m_pd3dDevice->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
	m_pd3dDevice->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
	m_pd3dDevice->SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);
	m_pd3dDevice->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_MODULATE);
	m_pd3dDevice->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
	m_pd3dDevice->SetTextureStageState(0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE);
	m_pd3dDevice->SetTextureStageState(0, D3DTSS_TEXCOORDINDEX, 0);
	m_pd3dDevice->SetTextureStageState(0, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_DISABLE);

}
void MBitmapR2::EndState()
{
}

inline void _swap(float& a,float& b) {
	static float temp;
	temp = a;
	a = b;
	b = temp;
}

void MBitmapR2::CheckDrawMode(float* fuv)
{
	if(m_DrawMode) {

		float temp[2];

		if(m_DrawMode & MBM_FlipLR)	{
			_swap(fuv[0],fuv[2]);
			_swap(fuv[1],fuv[3]);
			_swap(fuv[4],fuv[6]);
			_swap(fuv[5],fuv[7]);
		}
		if(m_DrawMode & MBM_FlipUD) {
			_swap(fuv[0],fuv[6]);
			_swap(fuv[1],fuv[7]);
			_swap(fuv[2],fuv[4]);
			_swap(fuv[3],fuv[5]);
		}
		if(m_DrawMode & MBM_RotL90) {
			temp[0] = fuv[4];temp[1] = fuv[5];
			fuv[4]  = fuv[6];fuv[5]  = fuv[7];
			fuv[6]  = fuv[0];fuv[7]  = fuv[1];
			fuv[0]  = fuv[2];fuv[1]  = fuv[3];
			fuv[2]  = temp[0];fuv[3]  = temp[1];
		}
		if(m_DrawMode & MBM_RotR90) {
			temp[0] = fuv[6];temp[1] = fuv[7];
			fuv[6]  = fuv[4];fuv[7]  = fuv[5];
			fuv[4]  = fuv[2];fuv[5]  = fuv[3];
			fuv[2]  = fuv[0];fuv[3]  = fuv[1];
			fuv[0]  = temp[0];fuv[1]  = temp[1];
		}
	}
}

struct BitmapVertex {
    FLOAT	x, y, z, rhw;
    u32		color;
    FLOAT	tu, tv;
};

constexpr u32 BitmapFVF = D3DFVF_XYZRHW | D3DFVF_DIFFUSE | D3DFVF_TEX1;

void MBitmapR2::Draw(float x, float y, float w, float h, float sx, float sy, float sw, float sh, 
					 DWORD dwColor, MDrawEffect effect)
{
	if(m_pTexture==NULL) return;

	float ftw = (float)m_pTexture->GetWidth();
	float fth = (float)m_pTexture->GetHeight();

	float fuv[8] = {
		(sx)/ftw	,(sy)/fth,
		(sx+sw)/ftw	,(sy)/fth,
		(sx+sw)/ftw	,(sy+sh)/fth,
		(sx)/ftw	,(sy+sh)/fth
	};

	CheckDrawMode(fuv);

	BitmapVertex Sprite[4] = {
#define ADJUST_SIZE		0.5f
#define ADJUST_SIZE2	0.0f
		{x-ADJUST_SIZE,    y-ADJUST_SIZE,    0, 1.0f, dwColor, fuv[0], fuv[1]},
		{x+w-ADJUST_SIZE2, y-ADJUST_SIZE,    0, 1.0f, dwColor, fuv[2], fuv[3]},
		{x+w-ADJUST_SIZE2, y+h-ADJUST_SIZE2, 0, 1.0f, dwColor, fuv[4], fuv[5]},
		{x-ADJUST_SIZE,    y+h-ADJUST_SIZE2, 0, 1.0f, dwColor, fuv[6], fuv[7]},
	};
	m_pd3dDevice->SetFVF(BitmapFVF);
	m_pd3dDevice->SetPixelShader(NULL);

	m_pd3dDevice->SetTexture(0, m_pTexture->GetTexture());

	BeginState(effect);

	HRESULT hr = m_pd3dDevice->DrawPrimitiveUP(D3DPT_TRIANGLEFAN, 2, Sprite, sizeof(BitmapVertex));
	_ASSERT(hr==D3D_OK);

	EndState();
}

void MBitmapR2::DrawEx(float tx1, float ty1, float tx2, float ty2, 
			float tx3, float ty3, float tx4, float ty4, DWORD dwColor, MDrawEffect effect)
{
	float ftw = (float)m_pTexture->GetWidth();
	float fth = (float)m_pTexture->GetHeight();

	float fuv[8] = {
		0.f,0.f,
		1.f,0.f,
		1.f,1.f,
		0.f,1.f
	};

	CheckDrawMode(fuv);

	BitmapVertex Sprite[4] = {
#define ADJUST_SIZE		0.5f
#define ADJUST_SIZE2	0.0f
		{tx1-ADJUST_SIZE,  ty1-ADJUST_SIZE,  0, 1.0f, dwColor, fuv[0], fuv[1]},
		{tx2-ADJUST_SIZE2, ty2-ADJUST_SIZE,  0, 1.0f, dwColor, fuv[2], fuv[3]},
		{tx4-ADJUST_SIZE2, ty4-ADJUST_SIZE2, 0, 1.0f, dwColor, fuv[4], fuv[5]},
		{tx3-ADJUST_SIZE,  ty3-ADJUST_SIZE2, 0, 1.0f, dwColor, fuv[6], fuv[7]},
	};
	m_pd3dDevice->SetFVF(BitmapFVF);
	m_pd3dDevice->SetPixelShader(NULL);

	m_pd3dDevice->SetTexture(0, m_pTexture->GetTexture());

	BeginState(effect);

	HRESULT hr = m_pd3dDevice->DrawPrimitiveUP(D3DPT_TRIANGLEFAN, 2, Sprite, sizeof(BitmapVertex));
	_ASSERT(hr==D3D_OK);

	EndState();
}

void MBitmapR2::OnLostDevice()
{
}

void MBitmapR2::OnResetDevice()
{
}

MFontR2::MFontR2()
{
#ifdef _DEBUG
	m_nTypeID = MINT_R2_CLASS_TYPE;
#endif
	m_fScale = 1.0f;
}

MFontR2::~MFontR2()
{
	Destroy();
}

bool MFontR2::Create(const char* szAliasName, const char* szFontName, int nHeight,
	float fScale, bool bBold, bool bItalic, int nOutlineStyle,
	int nCacheSize, bool bAntiAlias, u32 nColorArg1, u32 nColorArg2)
{
	m_fScale = fScale;

	m_nOutlineStyle = nOutlineStyle;
	m_ColorArg1 = nColorArg1;
	m_ColorArg2 = nColorArg2;
	m_bAntiAlias = bAntiAlias;

	MFont::Create(szAliasName);
	return m_Font.Create(szFontName, nHeight, bBold, bItalic, nOutlineStyle, nCacheSize, bAntiAlias, nColorArg1, nColorArg2);
}

void MFontR2::Destroy()
{
	m_Font.Destroy();
}

int MFontR2::GetHeight()
{
	return int(m_Font.GetHeight()*m_fScale);
}

int MFontR2::GetWidth(const char* szText, int nSize)
{
	return int(m_Font.GetTextWidth(szText, nSize)*m_fScale);
}
