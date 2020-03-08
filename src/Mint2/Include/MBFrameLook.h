#ifndef MBFrameLOOK_H
#define MBFrameLOOK_H

#include "MFrame.h"
#include "MScalableLook.h"

#define FRAME_BITMAP_COUNT			9
#define FRAME_BUTTON_BITMAP_COUNT	3

class MBFrameLook : public MFrameLook , public MScalableLook {
public:
	char		m_szDefaultTitle[256];
	MBitmap*		m_pFrameBitmaps[FRAME_BITMAP_COUNT];
	MFont*			m_pFont;
	MCOLOR		m_FontColor;
	MCOLOR		m_BGColor;
	MPOINT		m_TitlePosition;
	bool				m_bStretch;
	MBitmap*		m_pCloseButtonBitmaps[FRAME_BUTTON_BITMAP_COUNT];	// 0 - up, 1 - down, 2 - disable
	MBitmap*		m_pMinimizeButtonBitmaps[FRAME_BUTTON_BITMAP_COUNT];
protected:
	virtual void	OnDraw(MFrame* pFrame, MDrawContext* pDC);
	int					m_iCustomLook;

public:
	MBFrameLook(void);
	void SetCustomLook( int i) {
		m_iCustomLook = i;
	}
	int GetCustomLook() const{
		return m_iCustomLook;
	}

	virtual MRECT GetClientRect(MFrame* pFrame, const MRECT& r) override;
};

#endif