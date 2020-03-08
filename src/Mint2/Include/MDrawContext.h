#pragma once

#include "CMList.h"
#include "MTypes.h"
#include "MBitmap.h"
#include "MFont.h"
#include "MCursor.h"
#include "MResourceManager.h"

struct MRECT;
struct MPOINT;

#define MINT_BASE_CLASS_TYPE	0x1030
#define MAM_NOTALIGN	0
#define MAM_LEFT		1
#define MAM_RIGHT		2
#define MAM_HCENTER		4
#define MAM_EDIT		8
#define MAM_TOP			16
#define MAM_BOTTOM		32
#define MAM_VCENTER		64

typedef int MAlignmentMode;

enum MDrawEffect
{
	MDE_NORMAL = 0,
	MDE_ADD,
	MDE_MULTIPLY,

	MDE_MAX
};

class MDrawContext {
protected:
#ifdef _DEBUG
	int		m_nTypeID;
#endif
	MCOLOR			m_Color;
	MCOLOR			m_HighlightColor;
	MCOLOR			m_ColorKey;
	MCOLOR			m_BitmapColor;
	MBitmap*		m_pBitmap;
	MFont*			m_pFont;
	MRECT			m_Clip;
	MPOINT			m_Origin;
	MDrawEffect		m_Effect;
	u8				m_nOpacity;

public:
	MDrawContext();
	virtual ~MDrawContext();

	// Color Setting
	MCOLOR SetBitmapColor(const MCOLOR& color);
	MCOLOR SetBitmapColor(unsigned char r, unsigned char g, unsigned char b, unsigned char a = 255);
	MCOLOR GetBitmapColor(void);
	MCOLOR SetColor(const MCOLOR& color);
	MCOLOR SetColor(unsigned char r, unsigned char g, unsigned char b, unsigned char a = 255);
	MCOLOR GetColor(void);
	MCOLOR SetHighlightColor(const MCOLOR& color);
	MCOLOR SetHighlightColor(unsigned char r, unsigned char g, unsigned char b, unsigned char a = 255);
	MCOLOR GetHighlightColor(void);
	MCOLOR SetColorKey(const MCOLOR& color);
	MCOLOR SetColorKey(unsigned char r, unsigned char g, unsigned char b, unsigned char a = 255);
	MCOLOR GetColorKey(void);

	// Font Setting
	MFont* SetFont(MFont* pFont);
	MFont* GetFont() { return m_pFont; }

	// Bitmap Setting
	MBitmap* SetBitmap(MBitmap* pBitmap);
	MBitmap* GetBitmap() { return m_pBitmap; }

	// Clipping ( Screen Coordinate )
	virtual void SetClipRect(MRECT& r);
	void SetClipRect(int x, int y, int w, int h);
	MRECT GetClipRect(void);

	// Logical Origin
	void SetOrigin(int x, int y);
	void SetOrigin(MPOINT& p);
	MPOINT GetOrigin(void);

	// Draw Effect
	void SetEffect(MDrawEffect effect);
	MDrawEffect GetEffect() { return m_Effect; }

	// Basic Drawing Functions
	virtual void SetPixel(int x, int y, MCOLOR& color) = 0;
	virtual void HLine(int x, int y, int len) = 0;
	virtual void VLine(int x, int y, int len) = 0;
	virtual void Line(int sx, int sy, int ex, int ey) = 0;
	virtual void Rectangle(int x, int y, int cx, int cy);
	void Rectangle(const MRECT& r);
	virtual void FillRectangle(int x, int y, int cx, int cy) = 0;
	void FillRectangle(const MRECT& r);

	virtual void DrawRelative(float x, float y, float w, float h) = 0;
	virtual void DrawRelative(float x, float y, int w, int h) = 0;
	virtual int TextRelative(float x, float y, const char* szText) = 0;
	virtual void DrawRelative(float x, float y, float w, float h, int sx, int sy, int sw, int sh) = 0;
	virtual void FillRectangleRelative(float x, float y, float cx, float cy) = 0;

	// Bitmap Drawing
	void Draw(int x, int y);
	void Draw(int x, int y, int w, int h);
	void Draw(int x, int y, int sx, int sy, int sw, int sh);
	void Draw(MPOINT &p);
	void Draw(MRECT& r);
	void Draw(int x, int y, MRECT& s);
	virtual void Draw(const MRECT& d, const MRECT& s);
	virtual void DrawInverse(int x, int y, int w, int h);
	virtual void DrawInverse(int x, int y, int w, int h, int sx, int sy, int sw, int sh) {};
	virtual void DrawEx(int tx1, int ty1, int tx2, int ty2,
		int tx3, int ty3, int tx4, int ty4);
	virtual void Draw(int x, int y, int w, int h, int sx, int sy, int sw, int sh);

	// Text
	virtual bool BeginFont() = 0;
	virtual bool EndFont() = 0;
	virtual int Text(int x, int y, const char* szText) = 0;
	int Text(MPOINT& p, const char* szText) { return Text(p.x, p.y, szText); }
	virtual int TextMultiLine(MRECT& r, const char* szText, int nLineGap = 0,
		bool bAutoNextLine = true, int nIndentation = 0, int nSkipLine = 0, MPOINT* pPositions = NULL);
	virtual int TextMultiLine2(MRECT& r, const char* szText, int nLineGap = 0,
		bool bAutoNextLine = true, MAlignmentMode am = (MAM_HCENTER | MAM_VCENTER));

	int TextWithHighlight(int x, int y, const char* szText);
	void GetPositionOfAlignment(MPOINT* p, const MRECT& r, const char* szText, MAlignmentMode am, bool bAndInclude = true);
	int Text(const MRECT& r, const char* szText,
		MAlignmentMode am = (MAM_HCENTER | MAM_VCENTER));
	int TextWithHighlight(const MRECT& r, const char* szText,
		MAlignmentMode am = (MAM_HCENTER | MAM_VCENTER));

	void TextMC(int x, int y, const char* szText);
	void TextMC(MRECT& r, const char* szText, MAlignmentMode am);

	static char* GetPureText(const char *szText);

	unsigned char SetOpacity(unsigned char nOpacity);
	unsigned char GetOpacity();

private:
	virtual void Draw(MBitmap *pBitmap, int x, int y, int w, int h,
		int sx, int sy, int sw, int sh) = 0;
};

int MMGetWidth(MFont *pFont, const char* szText, int nSize, bool bColorSupport = false);
int MMGetNextLinePos(MFont *pFont, const char* szText, int nWidth,
	bool bAutoNextLine = true, bool bColorSupport = false);
int MMGetLinePos(MFont *pFont, const char* szText, int nWidth,
	bool bAutoNextLine = true, bool bColorSupport = false, int nLine = 1, int nIndentation = 0);
int MMGetLineCount(MFont *pFont, const char* szText, int nWidth,
	bool bAutoNextLine = true, bool bColorSupport = false, int nIndentation = 0);

extern u32 MMColorSet[10];