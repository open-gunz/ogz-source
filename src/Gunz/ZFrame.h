#ifndef ZFRAME_H
#define ZFRAME_H

#include "MFrame.h"
#include "MPopupMenu.h"
#include "MSlider.h"

class MFrame;

class ZFrame : public MFrame
{
protected:
	bool				m_bExclusive;
	u32	m_nShowTime;
	bool				m_bNextVisible;

	virtual void OnDraw(MDrawContext* pDC);

public:
	ZFrame(const char* szName=NULL, MWidget* pParent=NULL, MListener* pListener=NULL);
	virtual ~ZFrame(void);

	void Show(bool bVisible, bool bModal=false);
};


#endif