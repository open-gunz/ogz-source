#ifndef MBTEXTAREALOOK_H
#define MBTEXTAREALOOK_H

#include "MTextArea.h"

class MBTextAreaLook : public MTextAreaLook{
public:
	MFont*		m_pFont;
	MBitmap*	m_pFrameBitmaps[9];

public:
	MBTextAreaLook(void);

	virtual void OnFrameDraw(MTextArea* pTextArea, MDrawContext* pDC);
	virtual MRECT GetClientRect(MTextArea* pTextArea, MRECT& r);
};

#endif