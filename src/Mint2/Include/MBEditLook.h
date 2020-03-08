#ifndef MBEDITLOOK_H
#define MBEDITLOOK_H

#include "MEdit.h"

class MBEditLook : public MEditLook{
public:
	MBitmap*	m_pFrameBitmaps[9];
	MFont*		m_pFont;
	

public:
	MBEditLook(void);

	virtual void OnFrameDraw(MEdit* pEdit, MDrawContext* pDC);
	virtual MRECT GetClientRect(MEdit* pEdit, MRECT& r);
	virtual void OnDraw(MEdit* pEdit, MDrawContext* pDC);
};

#endif