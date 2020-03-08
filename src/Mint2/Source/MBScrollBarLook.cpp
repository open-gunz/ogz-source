#include "stdafx.h"
#include "MBScrollBarLook.h"
#include "MBitmapDrawer.h"

void MBArrowLook::OnDrawUpArrow(MDrawContext* pDC, MRECT& r, bool bPressed)
{
	if(bPressed==false) pDC->SetBitmap(m_pArrowBitmaps[4]);
	else pDC->SetBitmap(m_pArrowBitmaps[6]);
	pDC->Draw(r.x, r.y);
}  

void MBArrowLook::OnDrawDownArrow(MDrawContext* pDC, MRECT& r, bool bPressed)
{
	if(bPressed==false) pDC->SetBitmap(m_pArrowBitmaps[5]);
	else pDC->SetBitmap(m_pArrowBitmaps[7]);
	pDC->Draw(r.x, r.y);
}

void MBArrowLook::OnDrawLeftArrow(MDrawContext* pDC, MRECT& r, bool bPressed)
{
	if(bPressed==false) pDC->SetBitmap(m_pArrowBitmaps[0]);
	else pDC->SetBitmap(m_pArrowBitmaps[2]);
	pDC->Draw(r.x, r.y);
}

void MBArrowLook::OnDrawRightArrow(MDrawContext* pDC, MRECT& r, bool bPressed)
{
	if(bPressed==false) pDC->SetBitmap(m_pArrowBitmaps[1]);
	else pDC->SetBitmap(m_pArrowBitmaps[3]);
	pDC->Draw(r.x, r.y);
}

MBArrowLook::MBArrowLook(void)
{
	for(int i=0; i<8; i++){
		m_pArrowBitmaps[i] = NULL;
	}
}

MSIZE MBArrowLook::GetDefaultSize(MArrow* pThumb)
{
	if(pThumb->m_nDirection==0){
		if(m_pArrowBitmaps[4]==NULL) return MArrowLook::GetDefaultSize(pThumb);
		return MSIZE(m_pArrowBitmaps[4]->GetWidth(), m_pArrowBitmaps[4]->GetHeight());
	}
	else{
		if(m_pArrowBitmaps[0]==NULL) return MArrowLook::GetDefaultSize(pThumb);
		return MSIZE(m_pArrowBitmaps[0]->GetWidth(), m_pArrowBitmaps[0]->GetHeight());
	}
}


#define IMAGEVGAP		3
void MBThumbLook::OnDraw(MThumb* pThumb, MDrawContext* pDC)
{
	MRECT r = pThumb->GetInitialClientRect();

//	if(pThumb->m_nDirection==0){
//		DrawBitmapFrameH3(pDC, r, m_pHBitmaps);
//	}
//	else{
//		DrawBitmapFrameV3(pDC, r, m_pVBitmaps);
//	}

	MBitmap *pBmp = m_pHBitmaps[0];
	pDC->SetBitmap( pBmp);

	pDC->Draw( r.x, r.y, r.w, r.h/2,
		       0, IMAGEVGAP, pBmp->GetWidth(), IMAGEVGAP);
	pDC->Draw( r.x, r.y+r.h/2, r.w, r.h/2,
		       0, pBmp->GetWidth() - IMAGEVGAP*2, pBmp->GetWidth(), IMAGEVGAP);
	pDC->Draw( r.x, r.y, r.w, IMAGEVGAP,
		       0, 0, pBmp->GetWidth(), IMAGEVGAP);
	pDC->Draw( r.x, r.y+r.h-IMAGEVGAP, r.w, IMAGEVGAP,
		       0, pBmp->GetHeight() - IMAGEVGAP, pBmp->GetWidth(), IMAGEVGAP);
	pDC->Draw( r.x, r.y + r.h/2 - (pBmp->GetHeight()-IMAGEVGAP*4)/2, r.w, pBmp->GetHeight() - IMAGEVGAP*4,
		       0, IMAGEVGAP*2, pBmp->GetWidth(), pBmp->GetHeight() - IMAGEVGAP*4);
}

MBThumbLook::MBThumbLook(void)
{
	for(int i=0; i<3; i++){
		m_pHBitmaps[i] = NULL;
		m_pHPressedBitmaps[i] = NULL;
		m_pVBitmaps[i] = NULL;
		m_pVPressedBitmaps[i] = NULL;
	}
}



MBScrollBarLook::MBScrollBarLook(void)
{
	for(int i=0; i<3; i++){
		m_pVFrameBitmaps[i] = NULL;
		m_pHFrameBitmaps[i] = NULL;
	}
}

void MBScrollBarLook::OnDraw(MScrollBar* pScrollBar, MDrawContext* pDC)
{
	MRECT r = pScrollBar->GetInitialClientRect();
	//if(pScrollBar->GetType()==MSBT_VERTICAL)
	//	DrawBitmapFrameV3(pDC, r, m_pVFrameBitmaps);
	//else 
	//	DrawBitmapFrameH3(pDC, r, m_pHFrameBitmaps);
	MRECT rtemp  = pDC->GetClipRect();
	pDC->SetClipRect(rtemp.x, rtemp.y, 22, rtemp.h );
	pDC->SetColor(MCOLOR(0xFF000000));
	pDC->FillRectangle( r.x, r.y, r.w, r.h );
	pDC->SetClipRect(rtemp);
}

MRECT MBScrollBarLook::GetClientRect(MScrollBar* pScrollBar, const MRECT& r)
{
	return r;
	/*
	if(IsNull(m_pVFrameBitmaps, 3)==true) return r;
	if(IsNull(m_pHFrameBitmaps, 3)==true) return r;

	if(pScrollBar->GetType()==MSBT_VERTICAL){
		int lw = m_pVFrameBitmaps[0]->GetWidth();
		int rw = m_pVFrameBitmaps[2]->GetWidth();
		return MRECT(r.x, r.y+lw, r.w, r.h-(lw+rw));
	}
	else{
		int lw = m_pHFrameBitmaps[0]->GetWidth();
		int rw = m_pHFrameBitmaps[2]->GetWidth();
		return MRECT(r.x+lw, r.y, r.w-(lw+rw), r.h);
	}
	*/
}
