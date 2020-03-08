#ifndef MBListBoxLook_H
#define MBListBoxLook_H

#include "MListBox.h"

class MBListBoxLook : public MListBoxLook{
public:
	MBitmap*	m_pFrameBitmaps[9];
	MFont*		m_pFont;

protected:
	virtual void OnFrameDraw(MListBox* pListBox, MDrawContext* pDC);

public:
	MBListBoxLook(void);

	virtual MRECT GetClientRect(MListBox* pListBox, MRECT& r);
	virtual void OnDraw(MListBox* pListBox, MDrawContext* pDC);
};

#endif