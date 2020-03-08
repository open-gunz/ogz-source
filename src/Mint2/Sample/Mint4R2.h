#ifndef MINT4R2_H
#define MINT4R2_H

#include "MDrawContext.h"
#include "RFont.h"
#include <D3dx8.h>

#define MINT_R2_CLASS_TYPE	0x1130		// RTTI와 같은 기능을 _DEBUG모드에서 구현하기 위한 ID

class MDrawContextR2 : public MDrawContext{
protected:
	LPDIRECT3DDEVICE8	m_pd3dDevice;
public:
	MDrawContextR2(LPDIRECT3DDEVICE8 pd3dDevice);
	virtual ~MDrawContextR2(void);

	// Basic Drawing Functions
	virtual void SetPixel(int x, int y, MCOLOR& color);
	virtual void HLine(int x, int y, int len);
	virtual void VLine(int x, int y, int len);
	virtual void Line(int sx, int sy, int ex, int ey);
	virtual void Rectangle(int x, int y, int cx, int cy);
	virtual void FillRectangle(int x, int y, int cx, int cy);

	// Bitmap Drawing
	virtual void Draw(int x, int y, bool bSprite, unsigned char nOpacity);
	virtual void Draw(int x, int y, int w, int h, unsigned char nOpacity);

	// Text
	virtual int Text(int x, int y, const char* szText);

	virtual void SetClipRect(MRECT& r);
};


// Lost 문제 해결해야 함
class MBitmapR2 : public MBitmap{
public:
	LPDIRECT3DDEVICE8		m_pd3dDevice;		// Local Copy
	LPD3DXSPRITE			m_pSprite;
	LPDIRECT3DTEXTURE8		m_pTexture;
	int						m_nWidth;
	int						m_nHeight;
	D3DXIMAGE_INFO			m_Info;

public:
	MBitmapR2(void);
	virtual ~MBitmapR2(void);

	virtual bool Create(const char* szAliasName, LPDIRECT3DDEVICE8 pd3dDevice, const char* szFileName);
	virtual bool Create(const char* szAliasName, LPDIRECT3DDEVICE8 pd3dDevice, void* pData, int nSize);
	virtual void Destroy(void);

	virtual int GetWidth(void);
	virtual int GetHeight(void);

	void OnLostDevice(void);
	void OnResetDevice(void);
};

class MFontR2 : public MFont{
public:
	float		m_fScale;
	_RS2::RFont	m_Font;
public:
	MFontR2(void);
	virtual ~MFontR2(void);

	virtual bool Create(const char* szAliasName, LPDIRECT3DDEVICE8 pd3dDevice, const char* szFontName, int nHeight, float fScale=1.0f, bool bBold=false, bool bItalic=false, int nCacheSize=-1);
	virtual void Destroy(void);

	virtual int GetHeight(void);
	virtual int GetWidth(const char* szText);
};

#endif