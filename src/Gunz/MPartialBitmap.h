#ifndef _MPARTIAL_BITMAP
#define _MPARTIAL_BITMAP

#include "MBitmap.h"

class MPartialBitmap : public MBitmap {
	MBitmap *m_pSource;
	MRECT	m_rtSource;
};


#endif