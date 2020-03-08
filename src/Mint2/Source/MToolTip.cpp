#include "stdafx.h"
#include "MToolTip.h"
#include "MColorTable.h"
#include "Mint.h"

#define TOOLTIP_WIDTH_GAP	20
#define TOOLTIP_HEIGHT_GAP	5

void MToolTip::OnDraw(MDrawContext* pDC)
{
	MRECT r = GetClientRect();
	/*
	MFont* pFont = MFontManager::Get(NULL);
	if(pFont==NULL) return;
	pDC->SetFont(pFont);
	*/
	pDC->SetColor(MCOLOR(DEFCOLOR_MTOOLTIP_PLANE));
	pDC->FillRectangle(r);
	pDC->SetColor(MCOLOR(DEFCOLOR_MTOOLTIP_OUTLINE));
	pDC->Rectangle(r);
	pDC->SetColor(MCOLOR(DEFCOLOR_MTOOLTIP_TEXT));
	char* szName;
	if(m_bUseParentName==true) szName = GetParent()->m_szName;
	else szName = m_szName;
	pDC->TextWithHighlight(r, szName, (MAM_HCENTER|MAM_VCENTER));
	// 라인수 만큼 늘려 주기..
	//pDC->TextWithHighlight(r.x+TOOLTIP_WIDTH_GAP/2+1, r.y+TOOLTIP_HEIGHT_GAP/2, szName);
}

MToolTip::MToolTip(const char* szName, MWidget* pParent)
: MWidget(szName, pParent, NULL)
{
	_ASSERT(pParent!=NULL);
	SetText(szName);

	m_bClipByParent = false;

	SetFocusEnable(false);

	Show(false);
}

MToolTip::~MToolTip(void)
{
}

void MToolTip::SetBounds(void)
{
	MFont* pFont = GetFont();

	char szName[MWIDGET_NAME_LENGTH];
	RemoveAnd(szName, sizeof(szName), m_bUseParentName==true?GetParent()->m_szName:m_szName);
	int nWidth = pFont->GetWidthWithoutAmpersand(szName);
	int nHeight = pFont->GetHeight();
	int x, y;
	MRECT pr = GetParent()->GetClientRect();
	MRECT spr = MClientToScreen(GetParent(), pr);
	if(spr.x+(nWidth+TOOLTIP_WIDTH_GAP/2)<=MGetWorkspaceWidth())
		x = pr.x+TOOLTIP_WIDTH_GAP/2+1;
	else{
		MPOINT p = MScreenToClient(GetParent(), MPOINT(MGetWorkspaceWidth()-(nWidth+TOOLTIP_WIDTH_GAP/2), 0));
		x = p.x;
	}
	y = pr.y-(nHeight+TOOLTIP_HEIGHT_GAP);
	MPOINT p = MClientToScreen(GetParent(), MPOINT(0, y));
	if(p.y<0){
		y = p.y+pr.h+(nHeight+TOOLTIP_HEIGHT_GAP);
		if(y>MGetWorkspaceHeight()) y = 0;
		p = MScreenToClient(GetParent(), MPOINT(0, y));
		y = p.y;
	}
	MWidget::SetBounds(MRECT(x-TOOLTIP_WIDTH_GAP/2, y, nWidth+TOOLTIP_WIDTH_GAP, nHeight+TOOLTIP_HEIGHT_GAP));
}

void MToolTip::SetText(const char* szText)
{
	if(szText==NULL) m_bUseParentName = true;
	else m_bUseParentName = false;
	if(szText!=NULL) MWidget::SetText(szText);
	SetBounds();
}

bool MToolTip::IsUseParentName(void)
{
	return m_bUseParentName;
}
