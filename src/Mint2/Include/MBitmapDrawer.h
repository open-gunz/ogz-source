#ifndef MBitmapDrawer_H
#define MBitmapDrawer_H

#include "MTypes.h"


#define FRAME_OUTLINE_WIDTH	1
#define FRAME_WIDTH		5
#define FRAME_INLINE_WIDTH 1

class MDrawContext;
class MBitmap;

void HLineBitmap(MDrawContext* pDC, int x, int y, int w, MBitmap* pBm, bool bStretch, float fScale = 1.f);

void DrawBitmapFrame2(MDrawContext* pDC, MRECT& r, MRECT& cliprect,MBitmap* Bms[9]);
void DrawBitmapFrame9(MDrawContext* pDC, MRECT& r, MBitmap* Bms[9],bool bStretch = true, float fScale = 1.f);

void DrawBitmapFrameH3(MDrawContext* pDC, MRECT& r, MBitmap* Bms[3]);	// Draw Horizontal 3 Bitmaps
void DrawBitmapFrameV3(MDrawContext* pDC, MRECT& r, MBitmap* Bms[3]);	// Draw Vertical 3 Bitmaps
void DrawBitmapFrameCustom1(MDrawContext* pDC, MRECT& r, MBitmap* Bms[9], bool bStretch = true );
void DrawBitmapFrameCustom2(MDrawContext* pDC, MRECT& r, MBitmap* Bms[9], MCOLOR bgColor, bool bStretch = true );
void DrawBitmapButtonCustom1( MDrawContext* pDC, MRECT& r, MBitmap* Bms[9], bool bDown = false, bool bStretch = true );

bool IsNull(MBitmap** ps, int nCount);

// Safety Call
#define GETWIDTH(_Image)	((_Image==NULL)?0:_Image->GetWidth())
#define GETHEIGHT(_Image)	((_Image==NULL)?0:_Image->GetHeight())


#endif