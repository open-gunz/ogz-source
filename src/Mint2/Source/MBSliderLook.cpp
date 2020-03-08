#include "stdafx.h"
#include "MBSliderLook.h"
#include "MBitmapDrawer.h"

void MBSliderThumbLook::OnDraw(MSliderThumb* pThumb, MDrawContext* pDC)
{
	MRECT r = pThumb->GetInitialClientRect();
	if (m_pBitmap == NULL) {
		pDC->SetColor(MCOLOR(m_ThumbColor));
		pDC->FillRectangle(r);
	}  else {
		pDC->SetBitmap(m_pBitmap);
		pDC->Draw(r.x, r.y);
	}
}

MBSliderThumbLook::MBSliderThumbLook(void)
{
	m_pBitmap = NULL;
	m_pPressedBitmap = NULL;
}

MSIZE MBSliderThumbLook::GetDefaultSize(MSliderThumb* pThumb)
{
	if(m_pBitmap==NULL) return MSliderThumbLook::GetDefaultSize(pThumb);
	return MSIZE(m_pBitmap->GetWidth(), m_pBitmap->GetWidth());
}
