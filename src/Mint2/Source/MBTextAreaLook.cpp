#include "stdafx.h"
#include "MBTextAreaLook.h"
#include "MBitmapDrawer.h"

void MBTextAreaLook::OnFrameDraw(MTextArea* pTextArea, MDrawContext* pDC)
{
	/*
	MRECT r = pTextArea->GetInitialClientRect();
	DrawBitmapFrame9(pDC, r, m_pFrameBitmaps);
	*/

	MRECT r = pTextArea->GetInitialClientRect();
	pDC->SetColor(19,19,19,255);
	pDC->FillRectangle(r);
	pDC->SetColor(128,128,128,255);
	pDC->Rectangle(r);

}

MBTextAreaLook::MBTextAreaLook(void)
{
	m_pFont = NULL;
	for(int i=0; i<9; i++){
		m_pFrameBitmaps[i] = NULL;
	}
}

MRECT MBTextAreaLook::GetClientRect(MTextArea* pTextArea, MRECT& r)
{
	int al = GETWIDTH(m_pFrameBitmaps[3]);
	int au = GETHEIGHT(m_pFrameBitmaps[7]);
	int ar = GETWIDTH(m_pFrameBitmaps[5]);
	int ab = GETHEIGHT(m_pFrameBitmaps[1]);

	int nWidth = r.w-(al+ar);
	if(pTextArea->IsScrollBarVisible()) 
		nWidth-=pTextArea->GetScrollBarWidth();

	const int nMargin = 2;

	return MRECT(r.x+al + nMargin, r.y+au + nMargin, nWidth -nMargin*2, r.h-(au+ab) -nMargin*2);
}
