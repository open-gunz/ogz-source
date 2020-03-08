#include "stdafx.h"
#include "MFrameDrawer.h"

void MFrameDrawer::DrawOuterBevel(MDrawContext* pDC, MRECT& r)
{
	pDC->SetColor(MCOLOR(128, 128, 128));
	pDC->HLine(r.x, r.y, r.w);
	pDC->VLine(r.x, r.y, r.h);
	pDC->SetColor(MCOLOR(64, 64, 64));
	pDC->HLine(r.x, r.y+r.h-1, r.w);
	pDC->VLine(r.x+r.w-1, r.y, r.h);
}

void MFrameDrawer::DrawInnerBevel(MDrawContext* pDC, MRECT& r)
{
	pDC->SetColor(MCOLOR(64, 64, 64));
	pDC->HLine(r.x, r.y, r.w);
	pDC->VLine(r.x, r.y, r.h);
	pDC->SetColor(MCOLOR(128, 128, 128));
	pDC->HLine(r.x, r.y+r.h-1, r.w);
	pDC->VLine(r.x+r.w-1, r.y, r.h);
}

void MFrameDrawer::DrawFlatBevel(MDrawContext* pDC, MRECT& r)
{
	pDC->SetColor(MCOLOR(96, 96, 96));
	pDC->Rectangle(r);
}

void MFrameDrawer::DrawOuterPlane(MDrawContext* pDC, MRECT& r)
{
	pDC->SetColor(MCOLOR(16, 16, 16));
	pDC->FillRectangle(r);
}

void MFrameDrawer::DrawInnerPlane(MDrawContext* pDC, MRECT& r)
{
	pDC->SetColor(MCOLOR(32, 32, 32));
	pDC->FillRectangle(r);
}

void MFrameDrawer::DrawFlatPlane(MDrawContext* pDC, MRECT& r)
{
	pDC->SetColor(MCOLOR(16, 16, 16));
	pDC->FillRectangle(r);
}

void MFrameDrawer::DrawOuterBorder(MDrawContext* pDC, MRECT& r)
{
	DrawOuterPlane(pDC, r);
	DrawOuterBevel(pDC, r);
}

void MFrameDrawer::DrawInnerBorder(MDrawContext* pDC, MRECT& r)
{
	DrawInnerPlane(pDC, r);
	DrawInnerBevel(pDC, r);
}

void MFrameDrawer::DrawFlatBorder(MDrawContext* pDC, MRECT& r)
{
	DrawFlatPlane(pDC, r);
	DrawFlatBevel(pDC, r);
}

void MFrameDrawer::Text(MDrawContext* pDC, MRECT& r, const char* szText, MAlignmentMode am, MFDTextStyle nTextStyle, bool bHighlight)
{
	MRECT rc;
	rc = pDC->GetClipRect();
	pDC->SetClipRect(r);

	switch(nTextStyle){
	case MFDTS_ACTIVE:
		pDC->SetColor(MCOLOR(255, 255, 255));
		if(bHighlight==true) pDC->TextWithHighlight(r, szText, am);
		else pDC->Text(r, szText, am);
		break;
	case MFDTS_DISABLE:
		{
		pDC->SetColor(MCOLOR(96, 96, 96));
		MCOLOR PrevHCol = pDC->SetHighlightColor(MCOLOR(128, 128, 128));
		if(bHighlight==true) pDC->TextWithHighlight(r, szText, am);
		else pDC->Text(r, szText, am);
		pDC->SetHighlightColor(PrevHCol);
		}
		break;
	case MFDTS_NORMAL:
	default:
		pDC->SetColor(MCOLOR(196, 196, 196));
		if(bHighlight==true) pDC->TextWithHighlight(r, szText, am);
		else pDC->Text(r, szText, am);
		break;
	}

	pDC->SetClipRect(rc);
}

void MFrameDrawer::Text(MDrawContext* pDC, MPOINT& p, const char* szText, MFDTextStyle nTextStyle, bool bHighlight, MRECT* r)
{
	bool bClip = false;

	MRECT rc;
	if(r!=NULL){
		rc = pDC->GetClipRect();
		pDC->SetClipRect(*r);
		bClip = true;
	}

	switch(nTextStyle){
	case MFDTS_ACTIVE:
		pDC->SetColor(MCOLOR(255, 255, 255));
		if(bHighlight==true) pDC->TextWithHighlight(p.x, p.y, szText);
		else pDC->Text(p.x, p.y, szText);
		break;
	case MFDTS_DISABLE:
		{
		pDC->SetColor(MCOLOR(96, 96, 96));
		MCOLOR PrevHCol = pDC->SetHighlightColor(MCOLOR(128, 128, 128));
		if(bHighlight==true) pDC->TextWithHighlight(p.x, p.y, szText);
		else pDC->Text(p.x, p.y, szText);
		pDC->SetHighlightColor(PrevHCol);
		}
		break;
	case MFDTS_NORMAL:
	default:
		pDC->SetColor(MCOLOR(196, 196, 196));
		if(bHighlight==true) pDC->TextWithHighlight(p.x, p.y, szText);
		else pDC->Text(p.x, p.y, szText);
		break;
	}

	if(r!=NULL){
		pDC->SetClipRect(rc);
	}
}

void MFrameDrawer::TextMC(MDrawContext* pDC, MRECT& r, const char* szText, MAlignmentMode am, MFDTextStyle nTextStyle, bool bHighlight)
{
	MRECT rc;
	rc = pDC->GetClipRect();
	pDC->SetClipRect(r);

	switch(nTextStyle){
	case MFDTS_ACTIVE:
		pDC->SetColor(MCOLOR(255, 255, 255));
		if(bHighlight==true) pDC->TextWithHighlight(r, szText, am);
		else pDC->TextMC(r, szText, am);
		break;
	case MFDTS_DISABLE:
		{
		pDC->SetColor(MCOLOR(96, 96, 96));
		MCOLOR PrevHCol = pDC->SetHighlightColor(MCOLOR(128, 128, 128));
		if(bHighlight==true) pDC->TextWithHighlight(r, szText, am);
		else pDC->TextMC(r, szText, am);
		pDC->SetHighlightColor(PrevHCol);
		}
		break;
	case MFDTS_NORMAL:
	default:
		pDC->SetColor(MCOLOR(196, 196, 196));
		if(bHighlight==true) pDC->TextWithHighlight(r, szText, am);
		else pDC->TextMC(r, szText, am);
		break;
	}

	pDC->SetClipRect(rc);
}

void MFrameDrawer::TextMC(MDrawContext* pDC, MPOINT& p, const char* szText, MFDTextStyle nTextStyle, bool bHighlight, MRECT* r)
{
	bool bClip = false;

	MRECT rc;
	if(r!=NULL){
		rc = pDC->GetClipRect();
		pDC->SetClipRect(*r);
		bClip = true;
	}

	switch(nTextStyle){
	case MFDTS_ACTIVE:
		pDC->SetColor(MCOLOR(255, 255, 255));
		if(bHighlight==true) pDC->TextWithHighlight(p.x, p.y, szText);
		else pDC->TextMC(p.x, p.y, szText);
		break;
	case MFDTS_DISABLE:
		{
		pDC->SetColor(MCOLOR(96, 96, 96));
		MCOLOR PrevHCol = pDC->SetHighlightColor(MCOLOR(128, 128, 128));
		if(bHighlight==true) pDC->TextWithHighlight(p.x, p.y, szText);
		else pDC->TextMC(p.x, p.y, szText);
		pDC->SetHighlightColor(PrevHCol);
		}
		break;
	case MFDTS_NORMAL:
	default:
		pDC->SetColor(MCOLOR(196, 196, 196));
		if(bHighlight==true) pDC->TextWithHighlight(p.x, p.y, szText);
		else pDC->TextMC(p.x, p.y, szText);
		break;
	}

	if(r!=NULL){
		pDC->SetClipRect(rc);
	}
}
