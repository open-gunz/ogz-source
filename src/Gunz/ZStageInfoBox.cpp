#include "stdafx.h"

#include "ZStageInfoBox.h"
#include "MBitmapDrawer.h"

//IMPLEMENT_LOOK(ZStageInfoBox, ZStageInfoBoxLook)

ZStageInfoBox::ZStageInfoBox(const char* szName, MWidget* pParent, MListener* pListener)
: MListBox(szName, pParent, pListener)
{
//	LOOK_IN_CONSTRUCTOR()

	SetItemHeight(GetItemHeight()*3);
	SetAlwaysVisibleScrollbar(true);

	m_pLook=NULL;
}

ZStageInfoBox::~ZStageInfoBox()
{
}

void ZStageInfoBoxLook::OnDraw(ZStageInfoBox* pBox, MDrawContext* pDC)
{
	MListBoxLook::OnDraw(pBox,pDC);
}

void ZStageInfoBox::OnDraw(MDrawContext* pDC)
{
	if(m_pLook)
	{
		MRECT r = GetInitialClientRect();

		int nShowCount=0;
		for(int i=GetStartItem(); i<GetCount(); i++){

			nShowCount++;

			if(nShowCount>=GetShowItemCount()) break;

			MRECT itemrect=MRECT(r.x,r.y+GetItemHeight()*(nShowCount-1),r.w,GetItemHeight());
			DrawBitmapFrame9(pDC, itemrect, m_pLook->m_pFrameBitmaps);
		}
	}
	
	MListBox::OnDraw(pDC);
}
