#pragma once

#include "MDrawContext.h"
#include "RFont.h"
#include "RBaseTexture.h"
#include "d3dx9.h"

#define MINT_R2_CLASS_TYPE	0x1130 // RTTI

class MDrawContextR2 : public MDrawContext{
protected:
	LPDIRECT3DDEVICE9	m_pd3dDevice;
public:
	MDrawContextR2(LPDIRECT3DDEVICE9 pd3dDevice);
	virtual ~MDrawContextR2(void);

	// Basic Drawing Functions
	virtual void SetPixel(int x, int y, MCOLOR& color) override;
	virtual void HLine(int x, int y, int len) override;
	virtual void VLine(int x, int y, int len) override;
	virtual void Line(int sx, int sy, int ex, int ey) override;
	virtual void Rectangle(int x, int y, int cx, int cy) override;
	virtual void FillRectangle(int x, int y, int cx, int cy) override;

	// Bitmap Drawing
	virtual void DrawEx(int tx1, int ty1, int tx2, int ty2, int tx3, int ty3, int tx4, int ty4) override;

	// Text
	virtual bool BeginFont() override;
	virtual bool EndFont() override;

	virtual int Text(int x, int y, const char* szText) override;

	virtual void SetClipRect(MRECT& r) override;

	// Functions to draw relative to 4:3 aspect ratio
	virtual void DrawRelative(float x, float y, float w, float h) override;
	virtual void DrawRelative(float x, float y, int w, int h) override;
	virtual int TextRelative(float x, float y, const char* szText) override;
	virtual void DrawRelative(float x, float y, float w, float h, int sx, int sy, int sw, int sh) override;
	virtual void FillRectangleRelative(float x, float y, float cx, float cy) override;

private:
	virtual void Draw(MBitmap *pBitmap, int x, int y, int w, int h, int sx, int sy, int sw, int sh) override;
};

class MBitmapR2 : public MBitmap{
public:
	LPDIRECT3DDEVICE9		m_pd3dDevice;
	RealSpace2::RBaseTexture *m_pTexture;

	static	u32			m_dwStateBlock;

protected:
	void BeginState(MDrawEffect effect);
	void EndState(void);

public:
	MBitmapR2();
	virtual ~MBitmapR2();

	virtual bool Create(const StringView& szAliasName, LPDIRECT3DDEVICE9 pd3dDevice,
		const StringView& szFileName, bool bUseFileSystem = true);
	virtual void Destroy() override;

	virtual int GetWidth() override;
	virtual int GetHeight() override;

	void CheckDrawMode(float* fuv);

	void Draw(float x, float y, float w, float h, float sx, float sy, float sw, float sh, 
		      DWORD dwColor, MDrawEffect effect=MDE_NORMAL);
	void DrawEx(float tx1, float ty1, float tx2, float ty2, 
		        float tx3, float ty3, float tx4, float ty4, DWORD dwColor, MDrawEffect effect=MDE_NORMAL);

	void OnLostDevice();
	void OnResetDevice();
};

class MFontR2 : public MFont {
public:
	bool		m_bAntiAlias;
	float		m_fScale;
	_RS2::RFont	m_Font;
public:
	MFontR2();
	virtual ~MFontR2();

	virtual bool Create(const char* szAliasName, const char* szFontName, int nHeight,
		float fScale = 1.0f, bool bBold = false, bool bItalic = false,
		int nOutlineStyle = 0, int nCacheSize = -1, bool bAntiAlias = false,
		u32 nColorArg1 = 0, u32 nColorArg2 = 0);
	virtual void Destroy() override;

	virtual int GetHeight() override;
	virtual int GetWidth(const char* szText, int nSize = -1) override;
	int GetWidth(const wchar_t* szText, int nSize = -1) {
		return int(m_Font.GetTextWidth(szText, nSize)*m_fScale);
	}
};