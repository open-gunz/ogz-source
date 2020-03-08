#include "stdafx.h"

#include "ZCanvas.h"

void ZCanvas::OnDraw(MDrawContext* pDC)
{
	if (m_pOnDrawFunc) 
	{
		m_pOnDrawFunc(this, pDC);
	}
}

ZCanvas::ZCanvas(const char* szName, MWidget* pParent, MListener* pListener) : MWidget(szName, pParent, pListener)
{
	m_pOnDrawFunc = NULL;
}

ZCanvas::~ZCanvas()
{


}
