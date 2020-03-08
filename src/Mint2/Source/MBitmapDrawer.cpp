#include "stdafx.h"
#include "MBitmapDrawer.h"
#include "MDrawContext.h"
#include <algorithm>
using std::min;
using std::max;

void HLineBitmap(MDrawContext* pDC, int x, int y, int w, MBitmap* pBm, bool bStretch, float fScale)
{
	if(pBm==NULL) return;
	int bw = pBm->GetWidth();		// width
	int bh = pBm->GetHeight();		// height
	int dw = (int)(fScale * bw);	// scale을 감안한 width
	int dh = (int)(fScale * bh);	// scale을 감안한 height

	pDC->SetBitmap(pBm);

	if(bStretch==true){
		pDC->Draw(x, y, w, dh);
	}
	else{
		int c = w/dw;
		for(int i=0; i<c; i++){
			pDC->Draw(x+i*dw, y, dw, dh);
		}

		// 남는 부분
		int ws = w%dw;
		if(ws>0){
			pDC->Draw(x+c*dw, y, ws, dh, 0, 0, bw*ws/dw , bh);
		}
	}
}

void VLineBitmap(MDrawContext* pDC, int x, int y, int h, MBitmap* pBm, bool bStretch, float fSclae = 1.f)
{
	if(pBm==NULL) return;
	int bw = pBm->GetWidth();
	int bh = pBm->GetHeight();
	int dw = (int)(fSclae * bw);
	int dh = (int)(fSclae * bh);
	pDC->SetBitmap(pBm);

	if(bStretch==true){
		pDC->Draw(x, y, dw, h);
	}
	else{
		int c = h/dh;
		for(int i=0; i<c; i++){
			pDC->Draw(x, y+i*dh, dw, dh);
		}

		// 남는 부분
		int hs = h%dh;
		if(hs>0){
			pDC->Draw(x, y+c*dh, dw, hs, 0, 0, bw, bh*hs/dh);
			//pDC->Draw(x, y+h-pBm->GetHeight(), true);
		}
	}
}

bool IsNull(MBitmap** ps, int nCount)
{
	for(int i=0; i<nCount; i++){
		if(ps[i]==NULL) return true;
	}
	return false;
}

// 비트맵 2장으로 돌려 찍는다..지금은 툴팁용으로 쓴다..

void DrawBitmapFrame2(MDrawContext* pDC, MRECT& r, MRECT& cr, MBitmap* Bms[9])
{
	if(Bms[0]&&Bms[0]) {

		//자신의 영역을 둘러싸는 이미지를 그려준다..
		// 9곳 회전해서 그리기..
		// 1 2 3 
		// 4 5 6
		// 7 8 9

		Bms[0]->SetDrawMode(MBM_Normal);
		Bms[1]->SetDrawMode(MBM_Normal);

		MRECT cliprect = pDC->GetClipRect();//잠시풀어준다..

		pDC->SetColor(MCOLOR(255,255,255,255));
//		pDC->SetOpacity(255);
//		pDC->SetClipRect(0,0,MGetWorkspaceWidth(),MGetWorkspaceHeight());
		pDC->SetClipRect( cr.x, cr.y, cr.w, cr.h );
//		pDC->SetBitmapColor(color);

		pDC->SetBitmap( Bms[0] );
		pDC->Draw(r.x, r.y, 16, 16);

		pDC->SetBitmap( Bms[1] );
		pDC->Draw(r.x+16, r.y, r.w-32,16);

		Bms[0]->SetDrawMode(MBM_FlipLR);

		pDC->SetBitmap( Bms[0] );
		pDC->Draw(r.x+r.w-16, r.y, 16, 16);

		//그려야할사이즈가있다면~ 중간단계
		if(r.h > 32) {

			Bms[1]->SetDrawMode(MBM_RotL90);
			pDC->SetBitmap( Bms[1] );
			pDC->Draw(r.x, r.y+16, 16, r.h-32);

			//중간색채우기
			pDC->SetColor(MCOLOR(0xffD9D9D9));//임시
			pDC->FillRectangle(MRECT(r.x+16,r.y+16,r.w-32,r.h-32));

			Bms[1]->SetDrawMode(MBM_RotR90);
			pDC->SetBitmap( Bms[1] );
			pDC->Draw(r.x+r.w-16, r.y+16, 16, r.h-32);
		}

		// 아래부분~

		Bms[0]->SetDrawMode(MBM_FlipUD);
		pDC->SetBitmap( Bms[0] );
		pDC->Draw(r.x, r.y+r.h-16, 16, 16);

		Bms[1]->SetDrawMode(MBM_FlipUD);
		pDC->SetBitmap( Bms[1] );
		pDC->Draw(r.x+16, r.y+r.h-16, r.w-32,16);

		Bms[0]->SetDrawMode(MBM_FlipUD|MBM_FlipLR);
		pDC->SetBitmap( Bms[0] );
		pDC->Draw(r.x+r.w-16, r.y+r.h-16, 16, 16);

		Bms[0]->SetDrawMode(MBM_Normal);
		Bms[1]->SetDrawMode(MBM_Normal);

//		pDC->SetBitmapColor(MCOLOR(255,255,255,255));
		pDC->SetClipRect(cliprect);
	}	
}

void DrawBitmapFrame9(MDrawContext* pDC, MRECT& r, MBitmap* Bms[9],bool bStretch,float fScale)
{
	//if(IsNull(Bms, 9)==true) return;

	/*
	6 7 8
	3 4 5
	0 1 2
	*/

	int add_h = 0;//버튼이 너무 작아졌을때 뭉개지는것 우선 땜질~

	//if(button && (r.h < 24)) add_h = 1;

	int bmw[9] = {0,};
	int bmh[9] = {0,};
	int dtw[9] = {0,};		// scale 이 감안된 크기
	int dth[9] = {0,};
	for(int i=0; i<9; i++){
		 bmw[i] = GETWIDTH(Bms[i]);
		 bmh[i] = GETHEIGHT(Bms[i]);
		 dtw[i] = (int)(fScale * bmw[i]);
		 dth[i] = (int)(fScale * bmh[i]);
	}

	// Body(4)
	pDC->SetBitmap(Bms[4]);

	int bdw = dtw[4];
	int bdh = dth[4];
	int sx = min(min(dtw[3], dtw[0]), dtw[6]);
	int sy = min(min(dth[7], dth[6]), dth[8]);
	int ex = min(min(dtw[5], dtw[8]), dtw[2]);
	int ey = min(min(dth[1], dth[0]), dth[2]);
	int bw = r.w-sx-ex;
	int bh = r.h-sy-ey;
	int wc = 0;
	if(bdw>0) wc = bw/bdw;
	int hc = 0;
	if(bdh>0) hc = bh/bdh;
	/*
	if(bw%bdw!=0) wc += 1;
	if(bh%bdh!=0) hc += 1;
	*/

	if (!bStretch)
	{
		for(int y=0; y<hc; y++){
			for(int x=0; x<wc; x++){
				pDC->Draw(r.x+sx+x*bdw, r.y+sy+y*bdh , bdw, bdh);
			}
		}
		// Bms[4] 크기로는 모자른 부분을 짤라서 맞춰 찍음
		if(bdw>0 && (bw%bdw)>0){
			for(int y=0; y<hc; y++){
				//pDC->Draw(r.x+sx+bw-bdw, r.y+sy+y*bdh, true);
				pDC->Draw(r.x+sx+wc*bdw, r.y+sy+y*bdh, bw%bdw, bdh, 0, 0, bw%bdw, bdh);
			}
		}
		if(bdh>0 && (bh%bdh)>0){
			for(int x=0; x<wc; x++){
				//pDC->Draw(r.x+sx+x*bdw, r.y+sy+bh-bdh, true);
				pDC->Draw(r.x+sx+x*bdw, r.y+sy+hc*bdh, bdw, bh%bdh, 0, 0, bdw, bh%bdh);
			}
		}
		//모서리
		if(bdw>0 && bdh>0 && (bw%bdw)>0 && (bh%bdh)>0){
			pDC->Draw(r.x+sx+wc*bdw, r.y+sy+hc*bdh, bw%bdw, bh%bdh, 0, 0, bw%bdw, bh%bdh);
			//pDC->Draw(r.x+sx+bw-bdw, r.y+sy+bh-bdh, true);
		}
	}
	else
	{
		MRECT rcClient = MRECT(sx, sy, bw, bh);
		pDC->Draw(rcClient);
	}

	// Upper
	int uw = r.w - (dtw[6]+dtw[8]);
	HLineBitmap(pDC, r.x+dtw[6], r.y, uw, Bms[7], bStretch, fScale);

	// Bottom
	int dw = r.w - (dtw[0]+dtw[2]);
	HLineBitmap(pDC, r.x+dtw[0], r.y+r.h-dth[1], dw, Bms[1], bStretch, fScale);


	// Left
	int lh = r.h - (dth[0]+dth[6]);
	VLineBitmap(pDC, r.x, r.y+dth[6]+add_h, lh, Bms[3], bStretch, fScale);
	
	// Right
	int rh = r.h - (dth[2]+dth[8]);
	VLineBitmap(pDC, r.x+r.w-dtw[5], r.y+dth[8]+add_h, rh, Bms[5], bStretch, fScale);


	// 6
	pDC->SetBitmap(Bms[6]);
	pDC->Draw(r.x, r.y, dtw[6], dth[6]);

	// 8
	pDC->SetBitmap(Bms[8]);
	pDC->Draw(r.x+r.w-dtw[8], r.y, dtw[8], dth[8]);

	// 0
	pDC->SetBitmap(Bms[0]);
	pDC->Draw(r.x, r.y+r.h-dth[0], dtw[0], dth[0]);

	// 2
	pDC->SetBitmap(Bms[2]);
	pDC->Draw(r.x+r.w-dtw[2], r.y+r.h-dth[2], dtw[2], dth[2]);
}

void DrawBitmapFrameCustom1( MDrawContext* pDC, MRECT& r, MBitmap* Bms[9], bool bStretch /* = true  */ )
{
	int upperFrame[2];
	upperFrame[0] = GETWIDTH(Bms[7]);
	upperFrame[1] = GETHEIGHT(Bms[7]);

//    pDC->SetOpacity(255);

	// Fill Frame innerSpace
	pDC->SetColor( 19,19,19,255);
	pDC->FillRectangle( r.x + FRAME_OUTLINE_WIDTH, r.y+FRAME_OUTLINE_WIDTH, r.w - 2*FRAME_OUTLINE_WIDTH, r.h - 2*FRAME_OUTLINE_WIDTH);

	//Draw Gradiation
	HLineBitmap(pDC, r.x+FRAME_OUTLINE_WIDTH+FRAME_WIDTH, r.y+FRAME_OUTLINE_WIDTH+upperFrame[1], r.w-2*FRAME_OUTLINE_WIDTH-2*FRAME_WIDTH, Bms[4], false);
 
	//Fill Frame OutLine...
  	pDC->SetColor(10,10,10,255);
	//pDC->FillRectangle( r.x, r.y, r.w, r.h );
	pDC->FillRectangle( r.x, r.y, r.w, FRAME_OUTLINE_WIDTH );
	pDC->FillRectangle( r.x, r.y, FRAME_OUTLINE_WIDTH, r.h );
	pDC->FillRectangle( r.x + r.w - FRAME_OUTLINE_WIDTH, r.y, FRAME_OUTLINE_WIDTH, r.h );
	pDC->FillRectangle( r.x, r.y + r.h - FRAME_OUTLINE_WIDTH, r.w, FRAME_OUTLINE_WIDTH );

//	//Draw Frame
  	HLineBitmap(pDC, r.x+FRAME_OUTLINE_WIDTH, r.y+FRAME_OUTLINE_WIDTH, r.w-2*FRAME_OUTLINE_WIDTH, Bms[7], bStretch);
//	
 	pDC->SetColor(61,61,61,255);
	pDC->FillRectangle( r.x+FRAME_OUTLINE_WIDTH, r.y+FRAME_OUTLINE_WIDTH+upperFrame[1], FRAME_WIDTH, r.h-(upperFrame[1]+FRAME_OUTLINE_WIDTH*2) );
  	pDC->FillRectangle( r.x + r.w - FRAME_OUTLINE_WIDTH-FRAME_WIDTH, r.y+upperFrame[1]+FRAME_OUTLINE_WIDTH, FRAME_WIDTH, r.h-(upperFrame[1]+FRAME_OUTLINE_WIDTH*2));
	pDC->FillRectangle( r.x+FRAME_OUTLINE_WIDTH+FRAME_WIDTH, r.y + r.h - FRAME_OUTLINE_WIDTH-FRAME_WIDTH, r.w-2*FRAME_OUTLINE_WIDTH-2*FRAME_WIDTH, FRAME_WIDTH );
//

	// Draw inOutline
  	pDC->SetColor( 0,0,0,255);
	pDC->FillRectangle( r.x + FRAME_OUTLINE_WIDTH + FRAME_WIDTH, r.y+FRAME_OUTLINE_WIDTH + upperFrame[1], r.w - 2*FRAME_OUTLINE_WIDTH - 2*FRAME_WIDTH, FRAME_INLINE_WIDTH);
	pDC->FillRectangle( r.x + FRAME_OUTLINE_WIDTH + FRAME_WIDTH, r.y+r.h - FRAME_OUTLINE_WIDTH -FRAME_WIDTH - FRAME_INLINE_WIDTH, r.w - 2*FRAME_OUTLINE_WIDTH - 2*FRAME_WIDTH, FRAME_INLINE_WIDTH);
	pDC->FillRectangle( r.x + FRAME_OUTLINE_WIDTH + FRAME_WIDTH,r.y+FRAME_OUTLINE_WIDTH + upperFrame[1], FRAME_INLINE_WIDTH, r.h-upperFrame[1]-2*FRAME_OUTLINE_WIDTH-FRAME_WIDTH);
	pDC->FillRectangle( r.x + r.w - FRAME_OUTLINE_WIDTH - FRAME_WIDTH-FRAME_INLINE_WIDTH,
		r.y+FRAME_OUTLINE_WIDTH + upperFrame[1]+FRAME_INLINE_WIDTH, 
		FRAME_INLINE_WIDTH, 
		r.h-upperFrame[1]-4*FRAME_OUTLINE_WIDTH-FRAME_WIDTH);	
}

void DrawBitmapFrameCustom2( MDrawContext* pDC, MRECT& r, MBitmap* Bms[9], MCOLOR bgColor, bool bStretch /* = true  */)
{
//	pDC->SetOpacity(255);

	// Fill Background
    pDC->SetColor( bgColor );
	pDC->FillRectangle(r);
    
	//Draw Top Frame
	HLineBitmap(pDC, r.x, r.y, r.w, Bms[7], bStretch );

	int h = Bms[1]->GetHeight();
	//Draw Bottom Frame
	HLineBitmap( pDC, r.x, r.y+r.h - h, r.w, Bms[1], bStretch );
}

void DrawBitmapButtonCustom1( MDrawContext* pDC, MRECT& r, MBitmap* Bms[9], bool bDown, bool bStretch )
{
	int bx,by;
	
	if( bDown )
	{
		bx = r.x + 1;
		by = r.y + 1;
	}
	else
	{
		bx = r.x;
		by = r.y;
	}	

//	pDC->SetOpacity(255);
	//쉐도우
	pDC->SetColor(10, 10, 10, 255);
	pDC->FillRectangle( r.x+2, r.y+2, r.w, r.h );
	// 아웃라인
	pDC->SetColor( 0, 0, 0, 255 );
	pDC->FillRectangle( bx - 1, by - 1, r.w + 1, r.h + 1);
	//버튼 그리기
	HLineBitmap( pDC, bx, by, r.w, Bms[4], bStretch );
}

void DrawBitmapFrameH3(MDrawContext* pDC, MRECT& r, MBitmap* Bms[3])
{
	//if(IsNull(Bms, 3)==true) return;

	// 0 1 2

	// Left
	pDC->SetBitmap(Bms[0]);
	pDC->Draw(r.x, r.y);

	// Body
	int bw = r.w-GETWIDTH(Bms[0])-GETWIDTH(Bms[2]);
	int iw = GETWIDTH(Bms[1]);
	int c = bw / iw;
	pDC->SetBitmap(Bms[1]);
	for(int i=0; i<c; i++){
		pDC->Draw(r.x+GETWIDTH(Bms[0])+i*iw, r.y);
	}
	if((bw%iw)>0){
		pDC->Draw(r.x+r.w-GETWIDTH(Bms[1])-GETWIDTH(Bms[2]), r.y);
	}

	// Right
	pDC->SetBitmap(Bms[2]);
	pDC->Draw(r.x+r.w-GETWIDTH(Bms[2]), r.y);
}

void DrawBitmapFrameV3(MDrawContext* pDC, MRECT& r, MBitmap* Bms[3])
{
	if(IsNull(Bms, 3)==true) return;

	// 0 1 2

	// Up
	pDC->SetBitmap(Bms[0]);
	pDC->Draw(r.x, r.y);

	// Body
	int bh = r.h-GETHEIGHT(Bms[0])-GETHEIGHT(Bms[2]);
	int ih = GETHEIGHT(Bms[1]);
	int c = bh / ih;
	if((bh%ih)>0) c++;
	pDC->SetBitmap(Bms[1]);
	for(int i=0; i<c; i++){
		pDC->Draw(r.x, r.y+GETHEIGHT(Bms[0])+i*ih);
	}

	// Bottom
	pDC->SetBitmap(Bms[2]);
	pDC->Draw(r.x, r.y+r.h-GETHEIGHT(Bms[2]));
}