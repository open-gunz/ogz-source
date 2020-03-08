#include "MSkin.h"
#include "MDrawContext.h"


MSkin::MSkin()
{
	m_pBitmap = NULL;
}

#define X(i)	m_Rect[i].x
#define Y(i)	m_Rect[i].y
#define W(i)	m_Rect[i].w
#define H(i)	m_Rect[i].h


void MSkin::Draw(MDrawContext *pDC, MRECT rt)
{

	pDC->SetBitmap(m_pBitmap);

	/*
	6 7 8
	3 4 5
	0 1 2
	*/

	// TODO : 일단 strect 만 구현, 나중에 tile 구현하자

	//         dest												source

	// 6
	pDC->Draw(rt.x, rt.y, W(6), H(6),							X(6),Y(6),W(6),H(6));

	// 7
	pDC->Draw(rt.x+W(6), rt.y,rt.w-W(6)-W(8), H(7),				X(7), Y(7), W(7), H(7));
 
	// 8
	pDC->Draw(rt.x+rt.w-W(8), rt.y, W(8), H(8),					X(8),Y(8),W(8),H(8));

	// 3
	pDC->Draw(rt.x, rt.y+H(6), W(3), rt.h-H(6)-H(0),			X(3), Y(3), W(3), H(3));

	// 5
	pDC->Draw(rt.x+rt.w-W(5), rt.y+H(8), W(5), rt.h-H(8)-H(2),	X(5), Y(5), W(5), H(5));

	// 0
	pDC->Draw(rt.x, rt.y+rt.h-H(0), W(0), H(0),					X(0),Y(0),W(0),H(0));

	// 1
	pDC->Draw(rt.x+W(0), rt.y+rt.h-H(1), rt.w-W(0)-W(2), H(1),	X(1), Y(1), W(1), H(1));

	// 2
	pDC->Draw(rt.x+rt.w-W(2), rt.y+rt.h-H(2), W(2), H(2),		X(2),Y(2),W(2),H(2));

	// 4 - center

	pDC->Draw(rt.x+W(6), rt.y+H(6), rt.w-W(3)-W(5), rt.h-H(3)-H(5),		X(4),Y(4),W(4),H(4));
}

#undef X
#undef Y
#undef W
#undef H
