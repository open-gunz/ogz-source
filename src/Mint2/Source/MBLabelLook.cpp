#include "stdafx.h"
#include "MBLabelLook.h"
#include "MBitmapDrawer.h"

void MBLabelLook::OnDraw(MLabel* pLabel, MDrawContext* pDC)
{
	MRECT r = pLabel->GetInitialClientRect();
	if(pLabel->GetFont() != NULL ) pDC->SetFont( pLabel->GetFont() );
	else if(m_pFont!=NULL) pDC->SetFont(m_pFont);
	//pDC->SetColor(m_FontColor);
	pDC->SetColor( pLabel->GetTextColor() );
	pDC->Text(r, pLabel->m_szName, pLabel->GetAlignment());
}

MBLabelLook::MBLabelLook(void)
{
	m_FontColor = MCOLOR(255, 255, 255);
}

MRECT MBLabelLook::GetClientRect(MLabel* pLabel, MRECT& r)
{
	return r;
}
