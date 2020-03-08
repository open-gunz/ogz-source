#ifndef MSKIN_H
#define MSKIN_H

#include "MTypes.h"

class MBitmap;
class MDrawContext;

// 한 bitmap 에서 9개로 나뉘어진 스킨프레임 데이터를 저장, 그리는 클래스 
class MSkin {

public:
	MSkin();

	MBitmap *m_pBitmap;
	MRECT	m_Rect[9];


	void Draw(MDrawContext *pDC, MRECT rt);

};


#endif