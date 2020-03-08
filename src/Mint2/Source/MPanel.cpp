#include "stdafx.h"
#include "MPanel.h"
#include "MColorTable.h"

IMPLEMENT_LOOK(MPanel, MPanelLook)

MPanel::MPanel(const char* szName, MWidget* pParent, MListener* pListener) : MWidget(szName, pParent, pListener)
{
	m_BorderColor = MCOLOR(DEFCOLOR_MPANEL_BORDER);
	m_BackgroundColor = MCOLOR(DEFCOLOR_MPANEL_PLANE);
	m_nBorderStyle = MBS_SINGLE;
}


void MPanel::SetBorderColor(MCOLOR color)
{	
	m_BorderColor = color;
}


MCOLOR MPanel::GetBorderColor(void)
{
	return m_BorderColor;
}

void MPanel::SetBorderStyle(MBorderStyle style)
{
	m_nBorderStyle = style;
}

MBorderStyle MPanel::GetBorderStyle()
{
	return m_nBorderStyle;
}

void MPanel::SetBackgroundColor(MCOLOR color)
{
	m_BackgroundColor = color;
}

MCOLOR MPanel::GetBackgroundColor(void)
{
	return m_BackgroundColor;
}

void MPanelLook::OnDraw(MPanel* pPanel, MDrawContext* pDC)
{
	if (pPanel->GetBorderStyle() == MBS_SINGLE) OnFrameDraw(pPanel, pDC);
}

MRECT MPanelLook::GetClientRect(MPanel* pLabel, const MRECT& r)
{
	return r;
}

void MPanelLook::OnFrameDraw(MPanel* pPanel, MDrawContext* pDC)
{
	MRECT r = pPanel->GetInitialClientRect();
	if (pPanel->GetBackgroundColor().a != 0)
	{
		pDC->SetColor(pPanel->GetBackgroundColor());
		pDC->FillRectangle(r);
	}
	pDC->SetColor(pPanel->GetBorderColor());
	pDC->Rectangle(r);
}

