#include "stdafx.h"
#include "MScrollBar.h"
#include "MColorTable.h"
#include <algorithm>

#define MSCROLLBAR_NAME				"Scroll Bar"


IMPLEMENT_LOOK(MThumb, MThumbLook)
IMPLEMENT_LOOK(MArrow, MArrowLook)
IMPLEMENT_LOOK(MScrollBar, MScrollBarLook)

MThumb::MThumb(const char* szName, MWidget* pParent, MListener* pListener)
		: MWidget(szName, pParent, pListener)
{
}

MArrow::MArrow(const char* szName, MWidget* pParent, MListener* pListener)
	: MButton(szName, pParent, pListener)
{
	m_nDirection = 0;
}

MSIZE MArrow::GetDefaultSize()
{
	if(GetLook()!=NULL) return GetLook()->GetDefaultSize(this);
	return MSIZE(MSCROLLBAR_DEFAULT_WIDTH, MSCROLLBAR_DEFAULT_WIDTH);
}


void MScrollBar::OnRun()
{
	MListener* pListener = GetListener();
	if(pListener==NULL) return;

	auto nCurrTime = GetGlobalTimeMS();
	if(nCurrTime-m_nPrevThumbRefresh<m_nThumbRefreshDelay) return;

	if(m_pUp->IsButtonDown()==true){
		if(m_nValue>m_nMinValue){
			m_nValue--;
			RecalcThumbPos();
			if(pListener!=NULL) pListener->OnCommand(this, MLIST_VALUE_CHANGED);
		}
		m_nPrevThumbRefresh = nCurrTime;
	}
	if(m_pDown->IsButtonDown()==true){
		if(m_nValue<m_nMaxValue){
			m_nValue++;
			RecalcThumbPos();
			if(pListener!=NULL) pListener->OnCommand(this, MLIST_VALUE_CHANGED);
		}
		m_nPrevThumbRefresh = nCurrTime;
	}
}

bool MScrollBar::OnEvent(MEvent* pEvent, MListener* pListener)
{
	MRECT r = GetClientRect();
	switch(pEvent->nMessage){
	case MWM_LBUTTONDOWN:

		if(m_pThumb->GetRect().InPoint(pEvent->Pos)==true){
			SetCapture();
			m_bThumbMove = true;
			if(m_nScrollBarType==MSBT_VERTICAL){
				m_nThumbPos = m_pThumb->GetRect().y - (r.y+m_pUp->GetRect().h);
				m_nThumbDownPos = pEvent->Pos.y;
			}
			else{
				m_nThumbPos = m_pThumb->GetRect().x - (r.x+m_pUp->GetRect().w);
				m_nThumbDownPos = pEvent->Pos.x;
			}
			return true;
		}
		else
		{
			if(r.InPoint(pEvent->Pos)==true){
				if(!m_pUp->GetRect().InPoint(pEvent->Pos)) {
					if(!m_pUp->GetRect().InPoint(pEvent->Pos)) {

						int nThumbMoveRange = GetThumbMoveRange();
						int nThumbPos;

						if(m_nScrollBarType==MSBT_VERTICAL)
						{
							nThumbPos  = pEvent->Pos.y;
							nThumbPos -= m_pThumb->GetRect().h/2;
						}
						else
						{
							nThumbPos  = pEvent->Pos.x;
							nThumbPos -= m_pThumb->GetRect().w/2;
						}
						
						if(nThumbPos<0) nThumbPos = 0;
						else if(nThumbPos>=nThumbMoveRange) nThumbPos = nThumbMoveRange-1;

						if(m_nScrollBarType==MSBT_VERTICAL)
							m_pThumb->SetPosition(m_pThumb->GetRect().x, r.y+m_pUp->GetRect().h + nThumbPos);
						else
							m_pThumb->SetPosition(r.x+m_pUp->GetRect().w + nThumbPos, m_pThumb->GetRect().y);

						float fThumbPos = nThumbPos / (float)nThumbMoveRange;
						m_nValue = int( fThumbPos * (m_nMaxValue-m_nMinValue+1) );
						if(pListener!=NULL) pListener->OnCommand(this, MLIST_VALUE_CHANGED);

						return true;

					}
				}
			}
		}
		break;
	case MWM_LBUTTONUP:
		if(m_bThumbMove==true){
			m_bThumbMove = false;
			ReleaseCapture();
			return true;
		}
		break;
	case MWM_MOUSEMOVE:
		if(m_bThumbMove==true){
			int nThumbMoveRange = GetThumbMoveRange();
			int nThumbPos;
			if(m_nScrollBarType==MSBT_VERTICAL)
				nThumbPos = m_nThumbPos + pEvent->Pos.y-m_nThumbDownPos;
			else
				nThumbPos = m_nThumbPos + pEvent->Pos.x-m_nThumbDownPos;

			if(nThumbPos<0) nThumbPos = 0;
			else if(nThumbPos>=nThumbMoveRange) nThumbPos = nThumbMoveRange-1;

			if(m_nScrollBarType==MSBT_VERTICAL)
				m_pThumb->SetPosition(m_pThumb->GetRect().x, r.y+m_pUp->GetRect().h + nThumbPos);
			else
				m_pThumb->SetPosition(r.x+m_pUp->GetRect().w + nThumbPos, m_pThumb->GetRect().y);

			float fThumbPos = nThumbPos / (float)nThumbMoveRange;
			m_nValue = int( fThumbPos * (m_nMaxValue-m_nMinValue+1) );
			if(pListener!=NULL) pListener->OnCommand(this, MLIST_VALUE_CHANGED);
			return true;
		}
		break;
	}

	if(pEvent->nMessage!=MWM_CHAR && r.InPoint(pEvent->Pos)==true) return true;

	return false;
}

void MScrollBar::OnSize(int w, int h)
{
	MRECT r = GetClientRect();

	MSIZE s = m_pUp->GetDefaultSize();
	if(m_nScrollBarType==MSBT_VERTICAL){
		m_pUp->SetBounds(MRECT(r.x, r.y, s.w, s.h));
		m_pDown->SetBounds(MRECT(r.x, r.y+r.h-s.h, s.w, s.h));
	}
	else{
		m_pUp->SetBounds(MRECT(r.x, r.y, s.w, s.h));
		m_pDown->SetBounds(MRECT(r.x+r.w-s.w, r.y, s.w, s.h));
	}

	RecalcThumbBounds();
}

int MScrollBar::GetThumbMoveRange()
{
	MRECT r = GetClientRect();
	if(m_nScrollBarType==MSBT_VERTICAL)
		return ( r.h - (m_pUp->GetRect().h+m_pDown->GetRect().h+m_pThumb->GetRect().h) );
	else
		return ( r.w - (m_pUp->GetRect().w+m_pDown->GetRect().w+m_pThumb->GetRect().w) );
}

int MScrollBar::GetMoveRange()
{
	MRECT r = GetClientRect();
	if(m_nScrollBarType==MSBT_VERTICAL)
		return ( r.h - (m_pUp->GetRect().h+m_pDown->GetRect().h) );
	else
		return ( r.w - (m_pUp->GetRect().w+m_pDown->GetRect().w) );
}

int MScrollBar::GetThumbSize()
{
	int nDiff = m_nMaxValue-m_nMinValue;
	int nMoveRange = GetMoveRange();
	int nThumbSize = nMoveRange - nDiff * 3;
	return std::max(nThumbSize, MSCROLLBAR_THUMB_HEIGHT);
}

void MScrollBar::RecalcThumbPos()
{
	MRECT r = GetClientRect();
	int nSpace = GetThumbMoveRange();
	int nValueWidth = m_nMaxValue - m_nMinValue;
	int nThumbPos = 0;
	if(nValueWidth!=0) nThumbPos = nSpace * m_nValue / nValueWidth;

	if(m_nScrollBarType==MSBT_VERTICAL)
		m_pThumb->SetPosition(r.x, r.y+m_pUp->GetRect().h+nThumbPos);
	else
		m_pThumb->SetPosition(r.x+m_pUp->GetRect().w+nThumbPos, r.y);
}

void MScrollBar::RecalcThumbBounds()
{
	MRECT r = GetClientRect();
	if(m_nScrollBarType==MSBT_VERTICAL)
		m_pThumb->SetSize(r.w, GetThumbSize());
	else
		m_pThumb->SetSize(GetThumbSize(), r.h);

	RecalcThumbPos();
}

void MScrollBar::Initialize(MScrollBarTypes t)
{
	m_nMinValue = 0;
	m_nMaxValue = 99;
	m_nValue = 0;

	m_pUp = new MArrow(NULL, this, this);
	MSIZE s = m_pUp->GetDefaultSize();
	m_pUp->SetSize(s.w, s.h);
	m_pDown = new MArrow(NULL, this, this);
	m_pDown->SetSize(s.w, s.h);
	m_pThumb = new MThumb(NULL, this, this);
	m_pThumb->SetSize(s.w, s.h);

	SetType(t);

	m_nThumbRefreshDelay = 80;
	m_nPrevThumbRefresh = 0;

	m_bThumbMove = false;

}

MScrollBar::MScrollBar(const char* szName, MWidget* pParent, MListener* pListener, MScrollBarTypes t)
: MWidget(szName, pParent, pListener)
{
	Initialize(t);
}

MScrollBar::MScrollBar(MWidget* pParent, MListener* pListener, MScrollBarTypes t)
: MWidget(MSCROLLBAR_NAME, pParent, pListener)
{
	Initialize(t);
}

MScrollBar::~MScrollBar()
{
	if(m_pUp!=NULL) delete m_pUp;
	if(m_pDown!=NULL) delete m_pDown;
	if(m_pThumb!=NULL) delete m_pThumb;
}

void MScrollBar::SetMinMax(int nMin, int nMax)
{
	m_nMinValue = nMin;
	m_nMaxValue = nMax;
	_ASSERT(nMin<=nMax);

	RecalcThumbBounds();
}

void MScrollBar::SetValue(int nValue)
{
	if(nValue>=m_nMinValue && nValue<=m_nMaxValue){
		m_nValue = nValue;
		RecalcThumbPos();
	}
}

int MScrollBar::GetValue()
{
	return m_nValue;
}

MScrollBarTypes MScrollBar::GetType()
{
	return m_nScrollBarType;
}

void MScrollBar::SetType(MScrollBarTypes t)
{
	m_nScrollBarType = t;

	m_pThumb->m_nDirection = (t==MSBT_HORIZONTAL)?0:1;

	MSIZE s = m_pUp->GetDefaultSize();

	if(m_nScrollBarType==MSBT_VERTICAL)
		SetSize(s.w, MSCROLLBAR_DEFAULT_HEIGHT);
	else
		SetSize(MSCROLLBAR_DEFAULT_HEIGHT, s.h);

	if(t==MSBT_VERTICAL){
		((MArrow*)m_pUp)->m_nDirection = 0;
		((MArrow*)m_pDown)->m_nDirection = 1;
	}
	else{
		((MArrow*)m_pUp)->m_nDirection = 3;
		((MArrow*)m_pDown)->m_nDirection = 4;
	}
}


void MScrollBar::ChangeCustomArrowLook(MArrowLook *pArrowLook)
{
	m_pUp->ChangeCustomLook(pArrowLook);
	m_pDown->ChangeCustomLook(pArrowLook);
}

void MScrollBar::ChangeCustomThumbLook(MThumbLook *pThumbLook)
{
	m_pThumb->ChangeCustomLook(pThumbLook);
}

int MScrollBar::GetDefaultBreadth()
{
	MSIZE s = m_pUp->GetDefaultSize();
	if(GetType()==MSBT_VERTICAL) return s.w;
	else return s.h;
}


void MThumbLook::OnDraw(MThumb* pThumb, MDrawContext* pDC)
{
	pDC->SetColor(MCOLOR(DEFCOLOR_FRAME_OUTLINE));
	pDC->FillRectangle(pThumb->GetClientRect());
}

MRECT MThumbLook::GetClientRect(MThumb* pThumb, const MRECT& r)
{
	return r;
}

void MArrowLook::OnDrawUpArrow(MDrawContext* pDC, MRECT& r, bool bPressed)
{
	if(bPressed==true){
		r.x ++;
		r.y ++;
	}

	pDC->SetColor(MCOLOR(DEFCOLOR_FRAME_OUTLINE));
	for(int yy=0; yy<r.h; yy++){
		int rw = r.w*yy/r.h;
		pDC->HLine(r.x+r.w/2-rw/2, r.y+yy, rw);
	}
}
void MArrowLook::OnDrawDownArrow(MDrawContext* pDC, MRECT& r, bool bPressed)
{
	if(bPressed==true){
		r.x ++;
		r.y ++;
	}

	pDC->SetColor(MCOLOR(DEFCOLOR_FRAME_OUTLINE));
	for(int yy=0; yy<r.h; yy++){
		int rw = r.w*yy/r.h;
		pDC->HLine(r.x+r.w/2-rw/2, r.y+r.h-1-yy, rw);
	}
}
void MArrowLook::OnDrawLeftArrow(MDrawContext* pDC, MRECT& r, bool bPressed)
{
	if(bPressed==true){
		r.x ++;
		r.y ++;
	}

	pDC->SetColor(MCOLOR(DEFCOLOR_FRAME_OUTLINE));
	for(int xx=0; xx<r.w; xx++){
		int rh = r.h*xx/r.w;
		pDC->VLine(r.x+xx, r.y+r.h/2-rh/2, rh);
	}
}
void MArrowLook::OnDrawRightArrow(MDrawContext* pDC, MRECT& r, bool bPressed)
{
	if(bPressed==true){
		r.x ++;
		r.y ++;
	}

	pDC->SetColor(MCOLOR(DEFCOLOR_FRAME_OUTLINE));
	for(int xx=0; xx<r.w; xx++){
		int rh = r.h*xx/r.w;
		pDC->VLine(r.x+r.w-1-xx, r.y+r.h/2-rh/2, rh);
	}
}
void MArrowLook::OnDraw(MArrow* pArrow, MDrawContext* pDC)
{
	int x = 0;
	int y = 0;
	bool bPressed = false;
	if(pArrow->IsButtonDown()==true || (pArrow->GetType()==MBT_PUSH && pArrow->GetCheck()==true)) bPressed = true;
	MRECT r = pArrow->GetRect();
	r.x = r.y = 0;
	switch(pArrow->m_nDirection){
	case 0:
		OnDrawUpArrow(pDC, MRECT(r.x+x, r.y+y, r.w, r.h), bPressed);
		break;
	case 1:
		OnDrawDownArrow(pDC, MRECT(r.x+x, r.y+y, r.w, r.h), bPressed);
		break;
	case 3:
		OnDrawLeftArrow(pDC, MRECT(r.x+x, r.y+y, r.w, r.h), bPressed);
		break;
	case 4:
		OnDrawRightArrow(pDC, MRECT(r.x+x, r.y+y, r.w, r.h), bPressed);
		break;
	default:
		pDC->FillRectangle(r.x+x, r.y+y, r.w, r.h);
	}
}

MRECT MArrowLook::GetClientRect(MArrow* pArrow, MRECT& r)
{
	return r;
}

MSIZE MArrowLook::GetDefaultSize(MArrow* pThumb)
{
	return MSIZE(MSCROLLBAR_DEFAULT_WIDTH, MSCROLLBAR_DEFAULT_WIDTH);
}

void MScrollBarLook::OnDraw(MScrollBar* pScrollBar, MDrawContext* pDC)
{
	if(pScrollBar->m_nDebugType==3)	{
		int k=0;
	}

	MRECT r = pScrollBar->GetInitialClientRect();
	pDC->SetColor(MCOLOR(0xFF000000));
	pDC->FillRectangle(r.x, r.y, r.w, r.h);
}

MRECT MScrollBarLook::GetClientRect(MScrollBar* pScrollBar, const MRECT& r)
{
	return r;
}
