#ifndef MBTABCTRLLOOK_H
#define MBTABCTRLLOOK_H

#include "MTabCtrl.h"

class MBTabCtrlLook : public MTabCtrlLook {
public:
	MBitmap*	m_pFrameBitmaps[9];

public:
	MBTabCtrlLook(void);

//	virtual void OnFrameDraw(MTabCtrl* pTabCtrl, MDrawContext* pDC);
	virtual MRECT GetClientRect(MTabCtrl* pTabCtrl, MRECT& r);

protected:
	virtual void	OnDraw(MTabCtrl* pTabCtrl, MDrawContext* pDC);
};

#endif