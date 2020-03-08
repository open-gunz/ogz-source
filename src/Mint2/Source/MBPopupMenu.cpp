#include "stdafx.h"
#include "MBPopupMenu.h"
#include "MBitmapDrawer.h"



MBPopupMenuLook::MBPopupMenuLook()
{
	for(int i=0; i<9; i++){
		m_pFrameBitmaps[i] = NULL;
	}
	m_pFont=NULL;
	m_FontColor = MCOLOR(255, 255, 255);
}

MRECT MBPopupMenuLook::GetClientRect(MPopupMenu* pMenu, MRECT& r)
{
	return MRECT(r.x+3, r.y+3, r.w-(6), r.h-(6));
}

void MBPopupMenuLook::OnFrameDraw(MPopupMenu* pMenu, MDrawContext* pDC)
{
	MRECT r = pMenu->GetInitialClientRect();
//	DrawBitmapFrame9(pDC, r, m_pFrameBitmaps);
	pDC->SetColor(19,19,19,255);
	pDC->FillRectangle(r);
	pDC->SetColor(128,128,128,255);
	pDC->Rectangle(r);
}

void MBPopupMenuLook::OnDraw(MPopupMenu* pMenu, MDrawContext* pDC)
{
	if(m_pFont!=NULL) pDC->SetFont(m_pFont);
	MPopupMenuLook::OnDraw(pMenu, pDC);
}
