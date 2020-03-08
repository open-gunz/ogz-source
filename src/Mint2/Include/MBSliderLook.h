#ifndef MBSliderLook_H
#define MBSliderLook_H

#include "MSlider.h"

class MBSliderThumbLook : public MSliderThumbLook
{
public:
	MBitmap*	m_pBitmap;
	MBitmap*	m_pPressedBitmap;
	MCOLOR		m_ThumbColor;

public:
	virtual void OnDraw(MSliderThumb* pThumb, MDrawContext* pDC);

	MBSliderThumbLook(void);

	virtual MSIZE GetDefaultSize(MSliderThumb* pThumb);
};

#endif