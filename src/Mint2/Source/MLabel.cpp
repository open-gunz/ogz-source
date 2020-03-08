#include "stdafx.h"
#include "MLabel.h"
#include "MColorTable.h"

#define MLABEL_DEFAULT_ALIGNMENT_MODE	MAM_LEFT


IMPLEMENT_LOOK(MLabel, MLabelLook)

MLabel::MLabel(const char* szName, MWidget* pParent, MListener* pListener) : MWidget(szName, pParent, pListener)
{
	m_TextColor		= DEFCOLOR_MLABEL_TEXT;
	m_AlignmentMode	= MLABEL_DEFAULT_ALIGNMENT_MODE;

	MFont* pFont = GetFont();
	int w = 100;
	if(szName!=NULL) w = pFont->GetWidth(szName);
	int h = pFont->GetHeight();
	SetSize(w, h);
}

void MLabel::SetTextColor(MCOLOR color)
{
	m_TextColor = color;
}

MCOLOR MLabel::GetTextColor()
{
	return m_TextColor;
}

MAlignmentMode MLabel::GetAlignment()
{
	return m_AlignmentMode;
}

MAlignmentMode MLabel::SetAlignment(MAlignmentMode am)
{
	MAlignmentMode temp = m_AlignmentMode;
	m_AlignmentMode = am;
	return temp;
}

void MLabelLook::OnDraw(MLabel* pLabel, MDrawContext* pDC)
{
	pDC->SetColor(pLabel->GetTextColor());
	MCOLOR PrevHCol = pDC->SetHighlightColor(MCOLOR(DEFCOLOR_PEN_HIGHLIGHT));
	MRECT r = pLabel->GetClientRect();
	pDC->TextWithHighlight(r, pLabel->m_szName, pLabel->GetAlignment());
	pDC->SetHighlightColor(PrevHCol);
}

MRECT MLabelLook::GetClientRect(MLabel* pLabel, const MRECT& r)
{
	MRECT t = r;
	t.EnLarge(-1);
	return t;
}

