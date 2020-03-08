#pragma once

#include "MWidget.h"
#include "MToolTip.h"

#define MAX_TOOLTIP_LINE_STRING 40

class ZToolTip : public MToolTip
{
public:
	ZToolTip(const char* szName, MWidget* pParent);
	virtual void OnDraw(MDrawContext* pDC);
	virtual void SetBounds(void);

	MBitmap* m_pBitmap1;
	MBitmap* m_pBitmap2;
};