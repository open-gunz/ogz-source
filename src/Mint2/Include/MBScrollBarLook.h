#ifndef MBScrollBarLook_H
#define MBScrollBarLook_H

#include "MScrollBar.h"

class MBArrowLook : public MArrowLook{
public:
	MBitmap*	m_pArrowBitmaps[8];
protected:
	virtual void OnDrawUpArrow(MDrawContext* pDC, MRECT& r, bool bPressed);
	virtual void OnDrawDownArrow(MDrawContext* pDC, MRECT& r, bool bPressed);
	virtual void OnDrawLeftArrow(MDrawContext* pDC, MRECT& r, bool bPressed);
	virtual void OnDrawRightArrow(MDrawContext* pDC, MRECT& r, bool bPressed);
public:
	MBArrowLook(void);

	virtual MSIZE GetDefaultSize(MArrow* pThumb);
};

class MBThumbLook : public MThumbLook{
public:
	MBitmap*	m_pHBitmaps[3];
	MBitmap*	m_pHPressedBitmaps[3];
	MBitmap*	m_pVBitmaps[3];
	MBitmap*	m_pVPressedBitmaps[3];
public:
	virtual void OnDraw(MThumb* pThumb, MDrawContext* pDC);

	MBThumbLook(void);
};

class MBScrollBarLook : public MScrollBarLook{
public:
	MBitmap*	m_pHFrameBitmaps[3];
	MBitmap*	m_pVFrameBitmaps[3];

public:
	MBScrollBarLook(void);

	virtual void OnDraw(MScrollBar* pScrollBar, MDrawContext* pDC);
	virtual MRECT GetClientRect(MScrollBar* pScrollBar, const MRECT& r);
};

#endif