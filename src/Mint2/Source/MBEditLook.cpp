#include "stdafx.h"
#include "MBEditLook.h"
#include "MBitmapDrawer.h"

void MBEditLook::OnFrameDraw(MEdit* pEdit, MDrawContext* pDC)
{
	MRECT r = pEdit->GetInitialClientRect();
 	if(GetCustomLook())
	{
		pDC->SetColor(MCOLOR(200,200,200,255));
		pDC->Rectangle(r);
		HLineBitmap( pDC, r.x+1, r.y+1, r.w-2, m_pFrameBitmaps[4], false );
		return;
	}
	DrawBitmapFrame9(pDC, r, m_pFrameBitmaps);
}

MBEditLook::MBEditLook(void)
{
	for(int i=0; i<9; i++){
		m_pFrameBitmaps[i] = NULL;
	}
	m_pFont=NULL;
}

MRECT MBEditLook::GetClientRect(MEdit* pEdit, MRECT& r)
{
	if( m_bCustomLook )
	{
		return MRECT(r.x+1, r.y+1, r.w-2, r.h-2);
	}
	int al = GETWIDTH(m_pFrameBitmaps[3]);
	int au = GETHEIGHT(m_pFrameBitmaps[7]);
	int ar = GETWIDTH(m_pFrameBitmaps[5]);
	int ab = GETHEIGHT(m_pFrameBitmaps[1]);
	return MRECT(r.x+al, r.y+au, r.w-(al+ar), r.h-(au+ab));
}

void MBEditLook::OnDraw(MEdit* pEdit, MDrawContext* pDC) 
{
	if(m_pFont!=NULL) pDC->SetFont(m_pFont);

    MEditLook::OnDraw(pEdit,pDC);
}
