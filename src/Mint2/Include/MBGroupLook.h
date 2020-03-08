#ifndef MBGROUPLOOK_H
#define MBGROUPLOOK_H

#include "MGroup.h"

#define FRAME_BITMAP_COUNT	9
class MBGroupLook : public MGroupLook{
public:
	MBitmap*	m_pFrameBitmaps[FRAME_BITMAP_COUNT];
	MFont*		m_pFont;
	MCOLOR		m_FontColor;
	MPOINT		m_TitlePosition;
	bool		m_bStretch;

protected:
	virtual void OnDraw(MGroup* pGroup, MDrawContext* pDC);

public:
	MBGroupLook(void);

	virtual MRECT GetClientRect(MGroup* pGroup, MRECT& r);
};

#endif