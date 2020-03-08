#pragma once

#include "MWidget.h"
#include "GlobalTypes.h"

class MPicture : public MWidget{
protected:
	MBitmap* m_pBitmap;
	int		m_iStretch;
	int		m_iAnimType;
	float	m_fAnimTime;
	u64		m_CurrentTime;
	bool	m_bAnim;
	u32		m_DrawMode;

	MCOLOR	m_BitmapColor;
	MCOLOR	m_BitmapReseveColor;
	bool	m_bSwaped;

protected:
	virtual void OnDraw(MDrawContext* pDC) override;
	void OnAnimDraw(MDrawContext* pDC, int x, int y, int w, int h, int bx, int by, int bw, int bh );

public:
	MPicture(const char* szName = nullptr, MWidget* pParent = nullptr, MListener* pListener = nullptr);

	void SetBitmap(MBitmap* pBitmap);
	MBitmap* GetBitmap();
	int		GetStretch() { return m_iStretch; }
	void SetStretch(int i) { m_iStretch = i; }

	void SetAnimation( int animType, float fAnimTime);

	void SetBitmapColor( MCOLOR color );	
	MCOLOR GetBitmapColor() const {
		return m_BitmapColor;
	}
	MCOLOR GetReservedBitmapColor() const{
		return m_BitmapReseveColor;	
	}

	void SetDrawMode( u32 mode ) {
		m_DrawMode = mode;
	}
	u32 GetDrawMode( ) const {
		return m_DrawMode;
	}

	bool IsAnim() const { return m_bAnim;}

#define MINT_PICTURE	"Picture"
	virtual const char* GetClassName() override { return MINT_PICTURE; }
};
