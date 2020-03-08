#pragma once

#include "RTypes.h"
#include <map>
#include <list>
#include <string>
#include "RealSpace2.h"
#include "MemPool.h"

_NAMESPACE_REALSPACE2_BEGIN

struct RCHARINFO : public CMemPoolSm<RCHARINFO> {
	int nWidth;
	int nFontTextureID;
	int nFontTextureIndex;
};

using RCHARINFOMAP = std::map<WORD,RCHARINFO*>;

struct RFONTTEXTURECELLINFO;

using RFONTTEXTURECELLINFOLIST = std::list<RFONTTEXTURECELLINFO*>;

struct RFONTTEXTURECELLINFO {
	int nID;
	int nIndex;
	RFONTTEXTURECELLINFOLIST::iterator itr;
};


class RFontTexture
{
	HDC		m_hDC;
	u32	*m_pBitmapBits;
	HBITMAP m_hbmBitmap;
	HBITMAP m_hPrevBitmap;


	LPDIRECT3DTEXTURE9		m_pTexture;
	int m_nWidth;
	int	m_nHeight;
	int m_nX,m_nY;
	int m_nCell;
	int m_LastUsedID;

	RFONTTEXTURECELLINFO	*m_CellInfo;
	RFONTTEXTURECELLINFOLIST m_PriorityQueue;

	bool UploadTexture(RCHARINFO *pCharInfo,u32* pBitmapBits,int w,int h);

public:
	RFontTexture();
	~RFontTexture();

	bool Create();
	void Destroy();

	LPDIRECT3DTEXTURE9 GetTexture() { return m_pTexture; }

	int GetCharWidth(HFONT hFont, const char* szChar);
	int GetCharWidth(HFONT hFont, const wchar_t* szChar);
	bool MakeFontBitmap(HFONT hFont, RCHARINFO *pInfo, const char* szText,
		int nOutlineStyle, u32 nColorArg1, u32 nColorArg2);
	bool MakeFontBitmap(HFONT hFont, RCHARINFO *pInfo, const wchar_t* szText,
		int nOutlineStyle, u32 nColorArg1, u32 nColorArg2);
	bool IsNeedUpdate(int nIndex, int nID);
	
	int GetWidth() { return m_nWidth; }
	int GetHeight() { return m_nHeight; }
	int GetCellCountX() { return m_nX; }
	int GetCellCountY() { return m_nY; }
};

class RFont {
	template <typename CharT>
	void DrawTextImpl(float x, float y, const BasicStringView<CharT>& Text,
		u32 dwColor = 0xFFFFFFFF, float fScale = 1.0f);
	template <typename CharT>
	int GetTextWidthImpl(const CharT* szText, int nSize = -1);

	HFONT	m_hFont;
	int		m_nHeight;
	int		m_nOutlineStyle;
	bool	m_bAntiAlias;

	u32	m_ColorArg1;
	u32	m_ColorArg2;

	RCHARINFOMAP m_CharInfoMap;
	RFontTexture *m_pFontTexture;

	static bool m_bInFont;

public:
	RFont();
	virtual ~RFont();

	bool Create(const TCHAR* szFontName, int nHeight,
		bool bBold = false, bool bItalic = false,
		int nOutlineStyle = 0, int nCacheSize = -1,
		bool bAntiAlias = false,
		u32 nColorArg1 = 0, u32 nColorArg2 = 0);
	void Destroy();

	bool BeginFont();
	bool EndFont();

	// Draws an extended ASCII string in the current codepage.
	void DrawText(float x, float y, const StringView& Text, u32 Color = 0xFFFFFFFF, float Scale = 1.0f);

	// Draws a UTF-16 string.
	// Doesn't support astral plane characters.
	void DrawText(float x, float y, const WStringView& Text, u32 Color = 0xFFFFFFFF, float Scale = 1.0f);

	int GetHeight() const { return m_nHeight; }
	int GetTextWidth(const char* szText, int nSize = -1);
	int GetTextWidth(const wchar_t* szText, int nSize = -1);
};

// debug
bool RFontCreate();
void RFontDestroy();

_NAMESPACE_REALSPACE2_END
