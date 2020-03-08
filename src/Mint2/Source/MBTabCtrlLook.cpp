#include "stdafx.h"
#include "MBTabCtrlLook.h"
#include "MBitmapDrawer.h"

void MBTabCtrlLook::OnDraw(MTabCtrl* pTabCtrl, MDrawContext* pDC)
{
	MRECT r = pTabCtrl->GetInitialClientRect();
	DrawBitmapFrame9(pDC, r, m_pFrameBitmaps, false, false);
}

MBTabCtrlLook::MBTabCtrlLook(void)
{
	for(int i=0; i<9; i++){
		m_pFrameBitmaps[i] = NULL;
	}
}

MRECT MBTabCtrlLook::GetClientRect(MTabCtrl* pTabCtrl, MRECT& r)
{
	int al = GETWIDTH(m_pFrameBitmaps[3]);
	int au = GETHEIGHT(m_pFrameBitmaps[7]);
	int ar = GETWIDTH(m_pFrameBitmaps[5]);
	int ab = GETHEIGHT(m_pFrameBitmaps[1]);

	int nWidth = r.w-(al+ar);

	const int nMargin = 2;

	return MRECT(r.x+al + nMargin, r.y+au + nMargin, nWidth -nMargin*2, r.h-(au+ab) -nMargin*2);
}
