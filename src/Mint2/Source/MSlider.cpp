#include "stdafx.h"
#include "MSlider.h"
#include "MColorTable.h"


void MSliderThumbLook::OnDraw(MSliderThumb* pThumb, MDrawContext* pDC)
{
	pDC->SetColor(MCOLOR(DEFCOLOR_MBUTTON_LIGHTPLANE));
	pDC->FillRectangle(pThumb->GetClientRect());
}

MRECT MSliderThumbLook::GetClientRect(MSliderThumb* pThumb, const MRECT& r)
{
	return r;
}

MSIZE MSliderThumbLook::GetDefaultSize(MSliderThumb* pThumb)
{
	return MSIZE(MSCROLLBAR_DEFAULT_WIDTH, MSCROLLBAR_DEFAULT_WIDTH);
}


IMPLEMENT_LOOK(MSliderThumb, MSliderThumbLook)

MSliderThumb::MSliderThumb(const char* szName, MWidget* pParent, MListener* pListener)
: MThumb(szName, pParent, pListener)
{
}

MSIZE MSliderThumb::GetDefaultSize()
{
	if(GetLook()!=NULL) return GetLook()->GetDefaultSize(this);
	return MSIZE(MSCROLLBAR_DEFAULT_WIDTH, MSCROLLBAR_DEFAULT_WIDTH);
}


IMPLEMENT_LOOK(MSlider, MScrollBarLook)

int MSlider::GetThumbSize()
{
	MSIZE s = ((MSliderThumb*)m_pThumb)->GetDefaultSize();
	return s.w;
}

void MSlider::Initialize()
{
	delete m_pThumb;
	m_pThumb = new MSliderThumb(NULL, this, this);
	MSIZE s = ((MSliderThumb*)m_pThumb)->GetDefaultSize();
	m_pThumb->SetSize(s.w, s.h);
	m_pThumb->m_nDirection = MSBT_HORIZONTAL;
}

MSlider::MSlider(const char* szName, MWidget* pParent, MListener* pListener)
: MScrollBar(szName, pParent, pListener, MSBT_HORIZONTAL)
{
	Initialize();
}

MSlider::MSlider(MWidget* pParent, MListener* pListener)
: MScrollBar(pParent, pListener, MSBT_HORIZONTAL)
{
	Initialize();
}

MSlider::~MSlider()
{
}
