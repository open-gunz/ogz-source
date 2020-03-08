#ifndef MFRAMEDRAWER_H
#define MFRAMEDRAWER_H

#include "MDrawContext.h"

enum MFDTextStyle{
	MFDTS_NORMAL,
	MFDTS_ACTIVE,
	MFDTS_DISABLE,
};

////////////////////////////////////////////////////
// Frame을 그리기 위한 클래스
// 이를 통해 스킨을 지원할 수 있다.
// Default Frame Drawer
class MFrameDrawer{
public:
	virtual void DrawOuterBevel(MDrawContext* pDC, MRECT& r);		// 바깥쪽 윤곽 그리기
	virtual void DrawInnerBevel(MDrawContext* pDC, MRECT& r);		// 안쪽 윤곽 그리기
	virtual void DrawFlatBevel(MDrawContext* pDC, MRECT& r);		// 평면 윤곽 그리기 ( 보통 눌렸을때... )
	virtual void DrawOuterPlane(MDrawContext* pDC, MRECT& r);		// 바깥쪽 면
	virtual void DrawInnerPlane(MDrawContext* pDC, MRECT& r);		// 안쪽 면
	virtual void DrawFlatPlane(MDrawContext* pDC, MRECT& r);		// 평면 면
	virtual void DrawOuterBorder(MDrawContext* pDC, MRECT& r);		// 바깥쪽 판(Bevel+Plane) 그리기
	virtual void DrawInnerBorder(MDrawContext* pDC, MRECT& r);		// 안쪽 판(Bevel+Plane) 그리기
	virtual void DrawFlatBorder(MDrawContext* pDC, MRECT& r);		// 일반적인 평면 판(Bevel+Plane) 그리기
	virtual void Text(MDrawContext* pDC, MRECT& r, const char* szText, MAlignmentMode am=MAM_NOTALIGN, MFDTextStyle nTextStyle=MFDTS_NORMAL, bool bHighlight=false);
	virtual void Text(MDrawContext* pDC, MPOINT& p, const char* szText, MFDTextStyle nTextStyle=MFDTS_NORMAL, bool bHighlight=false, MRECT* r=NULL);
	virtual void TextMC(MDrawContext* pDC, MRECT& r, const char* szText, MAlignmentMode am=MAM_NOTALIGN, MFDTextStyle nTextStyle=MFDTS_NORMAL, bool bHighlight=false);
	virtual void TextMC(MDrawContext* pDC, MPOINT& p, const char* szText, MFDTextStyle nTextStyle=MFDTS_NORMAL, bool bHighlight=false, MRECT* r=NULL);
};

#endif