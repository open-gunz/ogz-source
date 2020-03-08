#include "stdafx.h"
#include "Mint.h"
#include "MBFrameLook.h"
#include "MBitmapDrawer.h"


void MBFrameLook::OnDraw(MFrame* pFrame, MDrawContext* pDC)
{
	MRECT r = pFrame->GetInitialClientRect();

	// 땜.. 장비창의 tooltip frame -
	// DrawBitmapFrame2 로 2장의 이미지를 사용해서 그림..

	if(GetCustomLook()==3) { 

		if(pFrame->GetVisible()==false)
			return;

		MRECT rr = r; // 툴팁이라서 그리는 영역을 조금 보정..

		rr.x -= 10;
		rr.y -= 10;
		rr.w += 20;
		rr.h += 20;

		// 지금은 툴팁 대용으로 쓰기때문에 cliprect 를 무시..

		MRECT cliprect = MRECT(0,0,MGetWorkspaceWidth(),MGetWorkspaceHeight());

		DrawBitmapFrame2(pDC,rr,cliprect,m_pFrameBitmaps);

		return;
	}

	MCOLOR color = MCOLOR(0xFF, 0xFF, 0xFF, pFrame->GetOpacity());

	if( GetCustomLook() == 1 )
		DrawBitmapFrameCustom1(pDC, r, m_pFrameBitmaps, m_bStretch);
	else if( GetCustomLook() == 2)
		DrawBitmapFrameCustom2(pDC, r, m_pFrameBitmaps, m_BGColor, m_bStretch );
	else if( GetCustomLook() == 3 )
	{
		pDC->SetColor( 128, 128, 128 );
		//r = pFrame->GetRect();
		pDC->Rectangle( r );
	}
	else
		DrawBitmapFrame9(pDC, r, m_pFrameBitmaps, m_bStretch, GetScale());
	

	if(pFrame->m_bTitleBar==true){
		if (pFrame->GetCloseButton()->m_bVisible)
		{
			MBitmap* pBitmap;
			if (pFrame->GetCloseButton()->IsButtonDown() == false)
			{
				pBitmap = m_pCloseButtonBitmaps[0];
			}
			else
			{
				pBitmap = m_pCloseButtonBitmaps[1];
				if (pBitmap == NULL) pBitmap = m_pCloseButtonBitmaps[0];
			}

			if (pBitmap!=NULL)
			{
				int x, y;
				x = pFrame->GetCloseButton()->m_Rect.x;
				y = pFrame->GetCloseButton()->m_Rect.y;

				pDC->SetBitmap(pBitmap);
				pDC->Draw(x, y);
			}

		}
		if (pFrame->GetMinimizeButton()->m_bVisible)
		{
			MBitmap* pBitmap;
			if (pFrame->GetMinimizeButton()->IsButtonDown() == false)
			{
				pBitmap = m_pMinimizeButtonBitmaps[0];
			}
			else
			{
				pBitmap = m_pMinimizeButtonBitmaps[1];
				if (pBitmap == NULL) pBitmap = m_pMinimizeButtonBitmaps[0];
			}

			if (pBitmap!=NULL)
			{
				int x, y;
				x = pFrame->GetMinimizeButton()->m_Rect.x;
				y = pFrame->GetMinimizeButton()->m_Rect.y;

				pDC->SetBitmap(pBitmap);
				pDC->Draw(x, y);
			}

		}

		if(m_pFont!=NULL) pDC->SetFont(m_pFont);
		pDC->SetColor(MCOLOR(0x0));
		if( GetCustomLook() == 0 ) pDC->Text(r.x+16, r.y+12, pFrame->m_szName);
		/*
		pDC->Text(r.x+12, r.y+8, pFrame->m_szName);
		pDC->Text(r.x+16, r.y+8, pFrame->m_szName);
		pDC->Text(r.x+12, r.y+12, pFrame->m_szName);
		*/

		pDC->SetColor(m_FontColor);

		// 나중에 align고려해서 다시 수정해야 함
		
		int y = int(m_TitlePosition.y*GetScale());

		if(m_pFrameBitmaps[7])
		{
			int au = (int)(GetScale() * m_pFrameBitmaps[7]->GetHeight());
			y = (au - m_pFont->GetHeight())/2;
		}

		pDC->Text(int(r.x+m_TitlePosition.x*GetScale()), r.y+y, pFrame->m_szName);
//		pDC->Text(r.x+m_TitlePosition.x*GetScale(), r.y+m_TitlePosition.y*GetScale(), pFrame->m_szName);

	//	pDC->Text(r.x+14, r.y+10, pFrame->m_szName);
	}
}

MBFrameLook::MBFrameLook(void)	: MScalableLook()
{
	m_szDefaultTitle[0]=0;
	m_TitlePosition = MPOINT(14, 10);
	m_FontColor = MCOLOR(255,255,255,255);
	m_bStretch = true;

	for(int i=0; i<FRAME_BITMAP_COUNT; i++){
		m_pFrameBitmaps[i] = NULL;
	}
	for (int i=0; i<FRAME_BUTTON_BITMAP_COUNT; i++)
	{
		m_pCloseButtonBitmaps[i] = NULL;
		m_pMinimizeButtonBitmaps[i] = NULL;
	}
	m_iCustomLook = 0;
}

#define RECT_DEFAULT_SPACE 12
MRECT MBFrameLook::GetClientRect(MFrame* pFrame, const MRECT& r)
{
	if(GetCustomLook() == 1)
	{
 		int al = FRAME_OUTLINE_WIDTH+FRAME_WIDTH+FRAME_INLINE_WIDTH+RECT_DEFAULT_SPACE;
		int au = m_pFrameBitmaps[7]->GetHeight() + FRAME_OUTLINE_WIDTH + FRAME_INLINE_WIDTH+RECT_DEFAULT_SPACE;
		return MRECT(r.x+al, r.y+au, r.w-(al*2), r.h-(al+au));
	}
	if( GetCustomLook() == 2 )
	{
#define CUSTOM2_FRAME_OFFSET_WIDTH 2
#define CUSTOM2_FRAME_OFFSET_HEIGHT 10
		return MRECT(r.x + CUSTOM2_FRAME_OFFSET_WIDTH, r.y + CUSTOM2_FRAME_OFFSET_HEIGHT , r.w - 2 * CUSTOM2_FRAME_OFFSET_WIDTH, r.h - 2 * CUSTOM2_FRAME_OFFSET_HEIGHT);
	}
	else
	{
		if(IsNull(m_pFrameBitmaps, FRAME_BITMAP_COUNT)==true) return r;

		float fScale = GetScale();
		int al = (int)(fScale * m_pFrameBitmaps[3]->GetWidth());
		int au = (int)(fScale * m_pFrameBitmaps[7]->GetHeight());
		int ar = (int)(fScale * m_pFrameBitmaps[5]->GetWidth());
		int ab = (int)(fScale * m_pFrameBitmaps[1]->GetHeight());
		return MRECT(r.x+al, r.y+au, r.w-(al+ar), r.h-(au+ab));
	}
}
