#ifndef MBPOPUPMENULOOK_H
#define MBPOPUPMENULOOK_H

#include "MPopupMenu.h"

class MBPopupMenuLook : public MPopupMenuLook {
public:
	MFont*		m_pFont;
	MCOLOR		m_FontColor;
	MBitmap*	m_pFrameBitmaps[9];

protected:
	virtual void OnFrameDraw(MPopupMenu* pPopupMenu, MDrawContext* pDC);

public:
	MBPopupMenuLook();
	virtual MRECT GetClientRect(MPopupMenu* pPopupMenu, MRECT& r);
	virtual void OnDraw(MPopupMenu* pPopupMenu, MDrawContext* pDC);
};

#endif
