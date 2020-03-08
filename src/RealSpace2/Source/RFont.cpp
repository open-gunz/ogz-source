#include "stdafx.h"
#include "RFont.h"
#include <crtdbg.h>
#include <mbstring.h>
#include <tchar.h>
#include "mDebug.h"
#include "mprofiler.h"
#include "VertexTypes.h"

#ifdef _USE_GDIPLUS
#include "unknwn.h"
#include "gdiplus.h"
	 using namespace Gdiplus;
#pragma comment(lib, "gdiplus.lib")
#pragma comment(lib, "Gdi32.lib")
#endif

_NAMESPACE_REALSPACE2_BEGIN

constexpr auto RFONT_TEXTURE_SIZE = 512;
constexpr auto RFONT_CELL_SIZE = 32;

RFontTexture::RFontTexture() 
{ 
	m_pTexture = NULL;
	m_CellInfo = NULL;
}

RFontTexture::~RFontTexture()
{
	Destroy();
}

bool RFontTexture::Create() 
{
	m_nWidth = RFONT_TEXTURE_SIZE;
	m_nHeight = RFONT_TEXTURE_SIZE;
	HRESULT hr = RGetDevice()->CreateTexture(m_nWidth, m_nHeight, 1, 0, D3DFMT_A8R8G8B8, D3DPOOL_MANAGED, &m_pTexture,NULL);
	if(hr!=D3D_OK) return false;

	m_nX = m_nWidth / RFONT_CELL_SIZE;
	m_nY = m_nHeight / RFONT_CELL_SIZE;

	m_nCell = m_nX * m_nY;
	m_CellInfo = new RFONTTEXTURECELLINFO[m_nCell];
	for(int i=0;i<m_nCell;i++) {
		m_CellInfo[i].nID = 0;
		m_CellInfo[i].nIndex = i;
		m_PriorityQueue.push_back(&m_CellInfo[i]);
	}

	m_LastUsedID = 0;

	m_hDC = CreateCompatibleDC(NULL);

	////////////////////////////////////////////////--------------------------------------------------------
	// Bitmap Creation
	BITMAPINFO bmi;
	ZeroMemory(&bmi.bmiHeader, sizeof(BITMAPINFOHEADER));
	bmi.bmiHeader.biSize        = sizeof(BITMAPINFOHEADER);
	bmi.bmiHeader.biWidth       =  (int)RFONT_CELL_SIZE;
	bmi.bmiHeader.biHeight      = -(int)RFONT_CELL_SIZE;
	bmi.bmiHeader.biPlanes      = 1;
	bmi.bmiHeader.biCompression = BI_RGB;
	bmi.bmiHeader.biBitCount    = 32;

	m_hbmBitmap = CreateDIBSection(m_hDC, &bmi, DIB_RGB_COLORS, (VOID**)&m_pBitmapBits, NULL, 0);
	if( m_pBitmapBits == NULL )
	{
		DeleteDC(m_hDC);
		return false;
	}

	HBITMAP m_hPrevBitmap = (HBITMAP)SelectObject(m_hDC, m_hbmBitmap);

	SetMapMode(m_hDC, MM_TEXT);

	return true;
}

void RFontTexture::Destroy() {
	
	if(m_hDC) {
		SelectObject(m_hDC, m_hPrevBitmap);
		if(m_hbmBitmap)
		{
			DeleteObject(m_hbmBitmap);
			m_hbmBitmap = NULL;
		}
		DeleteDC(m_hDC);
		m_hDC = NULL;
	}
	SAFE_RELEASE(m_pTexture);
	SAFE_DELETE_ARRAY(m_CellInfo);
}

void BlitRect(BYTE* pDest, int x1,int y1,int x2,int y2,int w, int h, u32* pBitmapBits, INT Pitch)
{
	if(pDest==NULL) return;

	u32* pDestTemp = NULL;

	int by=0;
	int bx=0;

	for(int y=y1; y<y2; y++) {
		pDestTemp = (u32*)(pDest+(y*Pitch));
		bx = 0;
		for(int x=x1; x<x2; x++) {

			u32 dwPixel = pBitmapBits[(w * by) + (bx)];
			if ( dwPixel & 0x00ffffff)
				dwPixel |= 0xff000000;

			*(pDestTemp+x) = dwPixel;

			bx++;
		}
		by++;
	}
}


bool RFontTexture::UploadTexture(RCHARINFO *pCharInfo,u32* pBitmapBits,int w,int h)
{
	D3DLOCKED_RECT d3dlr;
	HRESULT hr = m_pTexture->LockRect(0,&d3dlr,NULL,NULL);

	if(hr == D3D_OK)
	{
		m_LastUsedID++;

		RFONTTEXTURECELLINFO *pInfo = *(m_PriorityQueue.begin());
		pInfo->nID = m_LastUsedID;
		pInfo->itr = m_PriorityQueue.begin();

		int x = pInfo->nIndex % GetCellCountX();
		int y = pInfo->nIndex / GetCellCountX();

		int _x = x*RFONT_CELL_SIZE;
		int _y = y*RFONT_CELL_SIZE;

		BlitRect((BYTE*)d3dlr.pBits, _x, _y, _x+w, _y+h, RFONT_CELL_SIZE, RFONT_CELL_SIZE, pBitmapBits, d3dlr.Pitch);

		hr = m_pTexture->UnlockRect(0);

		m_PriorityQueue.splice(m_PriorityQueue.end(),m_PriorityQueue,m_PriorityQueue.begin());

		pCharInfo->nFontTextureID = pInfo->nID;
		pCharInfo->nFontTextureIndex = pInfo->nIndex;
		pCharInfo->nWidth = w;

		return true;
	}

	return false;
}

bool RFontTexture::IsNeedUpdate(int nIndex, int nID)
{
	if(nIndex==-1) return true;
	if(m_CellInfo[nIndex].nID == nID) {
		m_PriorityQueue.splice(m_PriorityQueue.end(),m_PriorityQueue,m_CellInfo[nIndex].itr);
		return false;
	}
	return true;
}

bool RFontTexture::MakeFontBitmap(HFONT hFont, RCHARINFO *pInfo, const wchar_t* szText,
	int nOutlineStyle, u32 nColorArg1, u32 nColorArg2)
{
	HFONT hPrevFont = (HFONT)SelectObject(m_hDC, hFont);

	SIZE size;
	GetTextExtentPoint32W(m_hDC, szText, wcslen(szText), &size);

	int nWidth = min(int(size.cx), RFONT_CELL_SIZE);

#ifdef _USE_GDIPLUS
	Graphics graphics(m_hDC);
	Gdiplus::Font font(m_hDC, hFont);

	graphics.Clear(Color(0,0,0,0));

	const StringFormat* pTypoFormat = StringFormat::GenericTypographic();

	if (nOutlineStyle == 0)
	{
		SolidBrush  solidBrush(Color(255, 255, 255, 255));
		graphics.DrawString(szText, -1, &font, PointF(0.0f, 0.0f), pTypoFormat, &solidBrush);
	}
	else if (nOutlineStyle == 1)
	{
		GraphicsPath path;
		FontFamily fontFamily;
		font.GetFamily(&fontFamily);

		TEXTMETRIC tm;
		GetTextMetrics(m_hDC, &tm);

		int nHeight;
		nHeight = min( (int)tm.tmHeight, (int)RFONT_CELL_SIZE-2);

		path.AddString(szText, -1, &fontFamily, FontStyleBold, Gdiplus::REAL(nHeight), PointF(-1.0f, -1.0f), pTypoFormat);

		graphics.SetSmoothingMode(SmoothingModeAntiAlias);

		Pen pen(Color(nColorArg2), 2.0f);
		graphics.DrawPath(&pen, &path);

		SolidBrush brush(Color((Gdiplus::ARGB)nColorArg1));
		graphics.FillPath((Brush*)&brush, &path);
	}
	else if (nOutlineStyle == 2)
	{
		SolidBrush  solidBrush2(Color((Gdiplus::ARGB)nColorArg2));
		graphics.DrawString(szText, -1, &font, PointF(1.0f, 1.0f), pTypoFormat, &solidBrush2);

		SolidBrush  solidBrush1(Color((Gdiplus::ARGB)nColorArg1));

		graphics.DrawString(szText, -1, &font, PointF(0.0f, 0.0f), pTypoFormat, &solidBrush1);

		char chChar = (char)szText[0];
		if ( (chChar >= '0') && (chChar <= '9'))
			nWidth++;
	}
#else	

	SetTextColor(m_hDC, RGB(255,255,255));
	SetBkColor(m_hDC, 0x00000000);
	SetTextAlign(m_hDC, TA_TOP);
	ExtTextOut(m_hDC, 0, 0, ETO_OPAQUE, NULL, szText, _tcslen(szText), NULL);

#endif

	bool bRet = UploadTexture(pInfo,m_pBitmapBits,nWidth,RFONT_CELL_SIZE);
	
	SelectObject(m_hDC, hPrevFont);

	return bRet;

}

int RFontTexture::GetCharWidth(HFONT hFont, const char* szChar)
{
	SIZE size;
	HFONT hPrevFont = (HFONT)SelectObject(m_hDC, hFont);
	GetTextExtentPoint32A(m_hDC, szChar, strlen(szChar), &size);
	SelectObject(m_hDC, hPrevFont);
	return size.cx;
}

int RFontTexture::GetCharWidth(HFONT hFont, const wchar_t* szChar)
{
	SIZE size;
	HFONT hPrevFont = (HFONT)SelectObject(m_hDC, hFont);
	GetTextExtentPoint32W(m_hDC, szChar, wcslen(szChar), &size);
	SelectObject(m_hDC, hPrevFont);
	return size.cx;
}

bool RFontTexture::MakeFontBitmap(HFONT hFont, RCHARINFO * pInfo, const char * szText,
	int nOutlineStyle, u32 nColorArg1, u32 nColorArg2)
{
	wchar_t WideText[256];
	auto len = MultiByteToWideChar(CP_ACP, 0, szText, -1, WideText, std::size(WideText));
	assert(len != 0);
	return MakeFontBitmap(hFont, pInfo, WideText, nOutlineStyle, nColorArg1, nColorArg2);
}

RFontTexture g_FontTexture;

bool RFontCreate()
{
	return g_FontTexture.Create();
}

void RFontDestroy()
{
	g_FontTexture.Destroy();
	RCHARINFO::Release();
}

constexpr auto RFONT_VERTEXCOUNT = 4;
using FONT2DVERTEX = ScreenSpaceColorTexVertex;
constexpr auto D3DFVF_FONT2DVERTEX = ScreenSpaceColorTexFVF;

constexpr auto MAX_FONT_BUFFER = 4000;

static	int				g_nFontCount = 0;
static	FONT2DVERTEX	g_FontVertexBuffer[4 * MAX_FONT_BUFFER];
static	u16				g_FontIndexBuffer[6 * MAX_FONT_BUFFER];

RFont::RFont()
{
}

RFont::~RFont()
{
	Destroy();
}

bool RFont::Create(const TCHAR* szFontName, int nHeight, bool bBold, bool bItalic,
	int nOutlineStyle, int nCacheSize, bool bAntiAlias, u32 nColorArg1, u32 nColorArg2)
{
	m_nOutlineStyle = nOutlineStyle;
	HDC hDC = GetDC(g_hWnd);
	SetMapMode(hDC, MM_TEXT);
	nHeight *= GetDeviceCaps(hDC, LOGPIXELSY);
	nHeight /= 72;
	
	m_nHeight = nHeight;

	m_ColorArg1 = nColorArg1;
	m_ColorArg2 = nColorArg2;
	m_bAntiAlias = bAntiAlias;

	const auto Weight = bBold ? FW_BOLD : FW_NORMAL;
	const auto Italic = static_cast<BOOL>(bItalic);
	m_hFont = CreateFont(
		-nHeight,            // Height
		0,                   // Width
		0,                   // Escapement
		0,                   // Orientation
		Weight,              // Weight
		Italic,              // Italic
		FALSE,               // Underline
		FALSE,               // Strike out
		DEFAULT_CHARSET,     // Char set
		OUT_DEFAULT_PRECIS,  // Out precision
		CLIP_DEFAULT_PRECIS, // Clip precision
		DEFAULT_QUALITY,     // Quality
		DEFAULT_PITCH,       // Pitch and family
		szFontName);         // Face name

	if (m_hFont == NULL)
		return false;
	ReleaseDC(g_hWnd, hDC);

	m_pFontTexture = &g_FontTexture;
	if (m_pFontTexture->GetTexture() == NULL)
		m_pFontTexture->Create();

	ASSERT(nHeight <= RFONT_CELL_SIZE);

	return true;
}

void RFont::Destroy()
{
	DeleteObject(m_hFont);
	m_hFont = NULL;

	while (m_CharInfoMap.size())
	{
		RCHARINFO* pCharInfo = m_CharInfoMap.begin()->second;
		delete pCharInfo;
		m_CharInfoMap.erase(m_CharInfoMap.begin());
	}
}

bool RFont::m_bInFont = false;

bool RFont::BeginFont()
{
	if(m_bInFont) return true;

	auto pDevice = RGetDevice();

	pDevice->SetRenderState( D3DRS_ALPHABLENDENABLE, TRUE);
	pDevice->SetRenderState( D3DRS_SRCBLEND,   D3DBLEND_SRCALPHA );
	pDevice->SetRenderState( D3DRS_DESTBLEND,  D3DBLEND_INVSRCALPHA );
	pDevice->SetRenderState( D3DRS_ALPHATESTENABLE,  FALSE );
	pDevice->SetRenderState( D3DRS_ALPHAREF,         0x08 );
	pDevice->SetRenderState( D3DRS_ALPHAFUNC,  D3DCMP_GREATEREQUAL );

	constexpr bool bFiltering = true;

	if (bFiltering) {
		pDevice->SetSamplerState( 0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR );
		pDevice->SetSamplerState( 0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR );
	}
	else{
		pDevice->SetSamplerState( 0, D3DSAMP_MINFILTER, D3DTEXF_POINT );
		pDevice->SetSamplerState( 0, D3DSAMP_MAGFILTER, D3DTEXF_POINT );
	}

	pDevice->SetRenderState( D3DRS_LIGHTING,   FALSE );
	pDevice->SetRenderState( D3DRS_SPECULARENABLE,   FALSE );
	pDevice->SetRenderState( D3DRS_FILLMODE,   D3DFILL_SOLID );
	pDevice->SetRenderState( D3DRS_CULLMODE,   D3DCULL_NONE );
	pDevice->SetRenderState( D3DRS_STENCILENABLE,    FALSE );
	pDevice->SetRenderState( D3DRS_CLIPPING,         TRUE );
	pDevice->SetRenderState( D3DRS_CLIPPLANEENABLE,  FALSE );
	pDevice->SetRenderState( D3DRS_VERTEXBLEND,      FALSE );
	pDevice->SetRenderState( D3DRS_INDEXEDVERTEXBLENDENABLE, FALSE );
	pDevice->SetRenderState( D3DRS_FOGENABLE,        FALSE );

	pDevice->SetTextureStageState( 0, D3DTSS_COLOROP,   D3DTOP_MODULATE );
	pDevice->SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_TEXTURE );
	pDevice->SetTextureStageState( 0, D3DTSS_COLORARG2, D3DTA_DIFFUSE );

	pDevice->SetTextureStageState( 0, D3DTSS_ALPHAOP,   D3DTOP_MODULATE );
	pDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE );
	pDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE );

	pDevice->SetSamplerState( 0, D3DSAMP_MIPFILTER, D3DTEXF_NONE );

	pDevice->SetTextureStageState( 0, D3DTSS_TEXCOORDINDEX, 0 );
	pDevice->SetTextureStageState( 0, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_DISABLE );
	pDevice->SetTextureStageState( 1, D3DTSS_COLOROP,   D3DTOP_DISABLE );
	pDevice->SetTextureStageState( 1, D3DTSS_ALPHAOP,   D3DTOP_DISABLE );

	RGetDevice()->SetFVF(D3DFVF_FONT2DVERTEX);
	RGetDevice()->SetTexture(0,g_FontTexture.GetTexture());

	g_nFontCount = 0;

	m_bInFont = true;

	return true;
}

bool RFont::EndFont()
{
	if(!m_bInFont) return true;

	if(g_nFontCount)
	{
		auto hr = RGetDevice()->DrawIndexedPrimitiveUP(D3DPT_TRIANGLELIST,0,g_nFontCount*4,g_nFontCount*2,
			g_FontIndexBuffer,D3DFMT_INDEX16,g_FontVertexBuffer,sizeof(FONT2DVERTEX));

		ASSERT(hr==D3D_OK);

		g_nFontCount = 0;
	}

	
	RGetDevice()->SetSamplerState( 0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR );
	RGetDevice()->SetSamplerState( 0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR );

	m_bInFont = false;
	return true;
}

static const char* inc(const char* c) {
	return (const char*)_mbsinc((const unsigned char*)c);
}

static const wchar_t* inc(const wchar_t* c) {
	return c + 1;
}

static size_t len(const char* str) {
	return strlen(str);
}

static size_t len(const wchar_t* str) {
	return wcslen(str);
}

template <typename CharT>
void RFont::DrawTextImpl(float x, float y, const BasicStringView<CharT>& Text,
	u32 dwColor, float fScale)
{
	if (Text.empty())
		return;

	bool bPrevInFont = false;

	BeginFont();

	auto p = Text.data();
	CharT szChar[4];

	while (true)
	{
		auto pp = inc(p);
		if (pp - p == 1) {
			szChar[0] = *p;
			szChar[1] = 0;
		}
		else {
			szChar[0] = *p;
			szChar[1] = *(p + 1);
			szChar[2] = 0;
		}
		_ASSERT(pp - p == 2 || pp - p == 1);

		WORD key = *(WORD*)szChar;

		auto i = m_CharInfoMap.find(key);

		RCHARINFO *pInfo = NULL;
		if (i == m_CharInfoMap.end()) {
			pInfo = new RCHARINFO;
			bool bRet = m_pFontTexture->MakeFontBitmap(m_hFont, pInfo, szChar,
				m_nOutlineStyle, m_ColorArg1, m_ColorArg2);
			if (bRet)
				m_CharInfoMap.insert(RCHARINFOMAP::value_type(key, pInfo));
			else {
				SAFE_DELETE(pInfo);
			}
		}
		else
		{
			pInfo = i->second;
			if (m_pFontTexture->IsNeedUpdate(pInfo->nFontTextureIndex, pInfo->nFontTextureID)) {
				m_pFontTexture->MakeFontBitmap(m_hFont, pInfo, szChar,
					m_nOutlineStyle, m_ColorArg1, m_ColorArg2);
			}
		}

		if (pInfo != NULL) {
			static FONT2DVERTEX vertices[RFONT_VERTEXCOUNT];
			u16 indices[6] = { 3, 0, 2, 0, 1, 2 };
			// 0 3
			// 1 2

			int nWidth = min(RFONT_CELL_SIZE, pInfo->nWidth);
			int w = int(nWidth * fScale);
			int h = int(RFONT_CELL_SIZE * fScale);

			if (x + w > RGetViewport()->X && x < RGetViewport()->X + RGetViewport()->Width &&
				y + h > RGetViewport()->Y && y < RGetViewport()->Y + RGetViewport()->Height)
			{
				vertices[0].pos = { x, y, 0, 1 };
				vertices[1].pos = { x, y + h, 0, 1 };
				vertices[2].pos = { x + w, y + h, 0, 1 };
				vertices[3].pos = { x + w, y, 0, 1 };

				int nCellX = pInfo->nFontTextureIndex % m_pFontTexture->GetCellCountX();
				int nCellY = pInfo->nFontTextureIndex / m_pFontTexture->GetCellCountX();

				float fMinX = (float)(.5f + nCellX*RFONT_CELL_SIZE) / (float)m_pFontTexture->GetWidth();
				float fMaxX = (float)(.5f + nCellX*RFONT_CELL_SIZE + nWidth) / (float)m_pFontTexture->GetWidth();
				float fMinY = (float)(.5f + nCellY*RFONT_CELL_SIZE) / (float)m_pFontTexture->GetHeight();
				float fMaxY = (float)(.5f + (nCellY + 1)*RFONT_CELL_SIZE) / (float)m_pFontTexture->GetHeight();

				vertices[0].tu = fMinX; vertices[0].tv = fMinY;
				vertices[1].tu = fMinX; vertices[1].tv = fMaxY;
				vertices[2].tu = fMaxX; vertices[2].tv = fMaxY;
				vertices[3].tu = fMaxX; vertices[3].tv = fMinY;

				vertices[0].color = vertices[1].color = vertices[2].color = vertices[3].color = dwColor;

				for (int i = 0; i<6; i++) {
					indices[i] += g_nFontCount * 4;
				}

				memcpy(g_FontVertexBuffer + g_nFontCount * 4, vertices, sizeof(vertices));
				memcpy(g_FontIndexBuffer + g_nFontCount * 6, indices, sizeof(indices));
				g_nFontCount++;
			}

			if (m_nOutlineStyle == 1)
				x += min(int(pInfo->nWidth*fScale + 1), RFONT_CELL_SIZE);
			else
				x += (pInfo->nWidth*fScale);

		}

		p = pp;

		if (p - Text.data() >= int(Text.size()))
			break;
	}

	if (!bPrevInFont)
		EndFont();
}

void RFont::DrawText(float x, float y, const StringView& Text, u32 Color, float Scale) {
	return DrawTextImpl(x, y, Text, Color, Scale);
}
void RFont::DrawText(float x, float y, const WStringView& Text, u32 Color, float Scale) {
	return DrawTextImpl(x, y, Text, Color, Scale);
}

template <typename CharT>
int RFont::GetTextWidthImpl(const CharT* szText, int nSize)
{
	if (!nSize || !szText || !szText[0]) return 0;
	if (nSize == -1) nSize = len(szText);

	int nWidth = 0;

	auto p = szText;
	CharT szChar[4];

	while (true)
	{
		auto pp = inc(p);

		if (pp - p == 1) {
			szChar[0] = *p;
			szChar[1] = 0;
		}
		else{
			szChar[0] = *p;
			szChar[1] = *(p+1);
			szChar[2] = 0;
		}
		_ASSERT(pp - p == 2 || pp - p == 1);

		WORD key = *(WORD*)szChar;

		auto i = m_CharInfoMap.find(key);

		RCHARINFO *pInfo = NULL;

		int nCurWidth;
		if (i == m_CharInfoMap.end()) {
			nCurWidth = m_pFontTexture->GetCharWidth(m_hFont, szChar);
			pInfo = new RCHARINFO;
			pInfo->nWidth = nCurWidth;
			pInfo->nFontTextureIndex = -1;
			m_CharInfoMap.emplace(key, pInfo);
		}
		else
		{
			nCurWidth = (*i->second).nWidth;
		}

		nWidth += nCurWidth;

		p = pp;

		if (p - szText >= nSize)
			break;
	}

	return nWidth;
}

int RFont::GetTextWidth(const char * szText, int nSize)
{
	return GetTextWidthImpl(szText, nSize);
}

int RFont::GetTextWidth(const wchar_t * szText, int nSize)
{
	return GetTextWidthImpl(szText, nSize);
}

_NAMESPACE_REALSPACE2_END
