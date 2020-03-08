#include "stdafx.h"
#include "MGroup.h"
#include "MColorTable.h"

void MGroupLook::OnDraw(MGroup* pGroup, MDrawContext* pDC)
{
	MRECT r = pGroup->GetInitialClientRect();
	pDC->SetColor(MCOLOR(DEFCOLOR_FRAME_PLANE));
	pDC->FillRectangle(r);
	pDC->SetColor(MCOLOR(DEFCOLOR_FRAME_OUTLINE));
	pDC->Rectangle(r);

	pDC->SetColor(MCOLOR(DEFCOLOR_FRAME_TEXT));
	pDC->Text(r.x+2, r.y+2, pGroup->m_szName);
}

MRECT MGroupLook::GetClientRect(MGroup* pGroup, const MRECT& r)
{
	return r;
}


IMPLEMENT_LOOK(MGroup, MGroupLook)

MGroup::MGroup(const char* szName, MWidget* pParent, MListener* pListener)
: MWidget(szName, pParent, pListener)
{
}

MGroup::~MGroup()
{
}
