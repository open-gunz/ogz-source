#include "stdafx.h"
#include "MFrame.h"
#include "MColorTable.h"
#include "Mint.h"

#define MTITLEBAR_HEIGHT		18

#define FRAME_W	300
#define FRAME_X	10
#define FRAME_H	300
#define FRAME_Y	10

void MFrameLook::OnDraw(MFrame* pFrame, MDrawContext* pDC)
{
	MRECT r = pFrame->GetInitialClientRect();
	pDC->SetColor(MCOLOR(DEFCOLOR_MFRAME_PLANE));
	pDC->FillRectangle(r);
	pDC->SetColor(128,128,128,255);
	pDC->Rectangle(r);

	// TitleBar
	if(pFrame->m_bTitleBar==true){
		MRECT TitleBarRect(r.x, r.y, r.w, MTITLEBAR_HEIGHT);
		pDC->SetColor(MCOLOR(DEFCOLOR_MMENUBAR_PLANE));
		pDC->FillRectangle(TitleBarRect);
		pDC->SetColor(MCOLOR(DEFCOLOR_MMENUBAR_TEXT));
		pDC->Text(TitleBarRect, pFrame->m_szName, MAM_LEFT);
	}
}

MRECT MFrameLook::GetClientRect(MFrame* pFrame, const MRECT& r)
{
	int nTitleBarHeight = (pFrame->m_bTitleBar==true?MTITLEBAR_HEIGHT:0);
	return MRECT(r.x+1, r.y + nTitleBarHeight, r.w-2, r.h - nTitleBarHeight-2);
}

IMPLEMENT_LOOK(MFrame, MFrameLook)

bool MFrame::OnEvent(MEvent* pEvent, MListener* pListener)
{
	if (IsFocusEnable() == false)
		return false;

	MRECT TitleBarRect(0, 0, m_Rect.w, MTITLEBAR_HEIGHT);
	MRECT WidgetRect(0, 0, m_Rect.w, m_Rect.h);
	MPOINT sp = MClientToScreen(this, pEvent->Pos);

	switch(pEvent->nMessage){
	case MWM_LBUTTONDOWN:

		if(m_bTitleBar==true && TitleBarRect.InPoint(pEvent->Pos)==true) {

			if ( m_BtnClose.m_Rect.InPoint(pEvent->Pos)==true)
			{
				m_BtnClose.m_bLButtonDown = true;
			}
			else if (m_BtnMinimize.m_Rect.InPoint(pEvent->Pos)==true)
			{
				m_BtnMinimize.m_bLButtonDown = true;
			}
			else
			{
				SetCapture();
				m_bDragWidget = true;
				MPOINT wp = MClientToScreen(GetParent(), MPOINT(m_Rect.x, m_Rect.y));
				sp.x -= wp.x;
				sp.y -= wp.y;
				m_DragPoint = sp;
			}
			return true;
		}
		else if(WidgetRect.InPoint(pEvent->Pos)==true){
			return true;
		}
		break;
	case MWM_LBUTTONUP:
		if (m_bTitleBar==true && m_BtnClose.m_Rect.InPoint(pEvent->Pos)==true)
		{
			if (m_BtnClose.m_bLButtonDown==true) OnCloseButtonClick();
		}
		else if (m_bTitleBar==true && m_BtnMinimize.m_Rect.InPoint(pEvent->Pos)==true)
		{
			if (m_BtnMinimize.m_bLButtonDown==true) OnMinimizeButtonClick();
		}

		m_BtnClose.m_bLButtonDown = m_BtnMinimize.m_bLButtonDown = false;

		if(m_bDragWidget==true){
			ReleaseCapture();
			m_bDragWidget = false;
			return true;
		}
		break;
	case MWM_MOUSEMOVE:
		if(m_bDragWidget==true){
			sp.x -= m_DragPoint.x;
			sp.y -= m_DragPoint.y;
			if(sp.x<0) sp.x = 0;
			if(sp.y<0) sp.y = 0;
			if(sp.x+m_Rect.w>MGetWorkspaceWidth()-1) sp.x = MGetWorkspaceWidth()-m_Rect.w-1;
			if(sp.y+m_Rect.h>MGetWorkspaceHeight()-1) sp.y = MGetWorkspaceHeight()-m_Rect.h-1;
			MPOINT p = MScreenToClient(GetParent(), sp);
			if (m_bMovable == true) {
				SetPosition(p.x, p.y);
			}
			
			return true;
		}
		else if(m_bTitleBar==true)
		{
			if(m_BtnClose.m_Rect.InPoint(pEvent->Pos)==true)
			{
				if(m_BtnClose.m_bMouseOver==false) m_BtnClose.m_bMouseOver = true;
			}
			else
			{
				if(m_BtnClose.m_bMouseOver==true) m_BtnClose.m_bMouseOver = false;
			}
			if(m_BtnMinimize.m_Rect.InPoint(pEvent->Pos)==true)
			{
				if(m_BtnMinimize.m_bMouseOver==false) m_BtnMinimize.m_bMouseOver = true;
			}
			else
			{
				if(m_BtnMinimize.m_bMouseOver==true) m_BtnMinimize.m_bMouseOver = false;
			}

		}
		break;
	case MWM_LBUTTONDBLCLK:
		break;
	}
	return false;
}

bool MFrame::OnShow()
{
	if(GetChildCount()>0) GetChild(0)->SetFocus();
	return true;
}

MFrame::MFrame(const char* szName, MWidget* pParent, MListener* pListener)
: MWidget(szName, pParent, pListener)
{
	SetBounds(MRECT(FRAME_X, FRAME_Y, FRAME_W, FRAME_H));
	m_OldRect = m_Rect;

	Show(false);

	m_bDragWidget = false;

	m_bZOrderChangable = true;

	// Resizable
	m_bResizable = true;
	m_bMovable = true;

	m_bShade = false;
	m_bCanShade = true;

	m_bTitleBar = true;

	m_nMinWidth = 300;
	m_nMinHeight = 200;
}

MFrame::~MFrame() = default;

void MFrame::OnSize(int w, int h)
{
	ResizeBtnsByAnchors(w, h);
	m_OldRect = m_Rect;
}

bool MFrame::OnCommand(MWidget* pWindow, const char* szMessage)
{

	return false;
}

void MFrame::SetShade(bool bShade)
{
	if (!m_bCanShade) return;

	if (m_bShade == bShade) return;

	if(m_bShade==false){
		m_BeforeShade = MSIZE(m_Rect.w, m_Rect.h);
		SetSize(m_Rect.w, MTITLEBAR_HEIGHT);
		m_bShade = true;
		m_bResizable = false;
	}
	else{
		SetSize(m_BeforeShade);
		m_bShade = false;
		m_bResizable = true;
	}
}

void ResizeByAnchors(MFrameBtn* pFrameBtn, MRECT parentRect, int w, int h)
{
	MRECT r = pFrameBtn->m_Rect;
	if(pFrameBtn->m_Anchors.m_bLeft==true && pFrameBtn->m_Anchors.m_bRight==true)
	{
		r.w += (w-parentRect.w);
	}
	else if(pFrameBtn->m_Anchors.m_bRight==true)
	{
		r.x += (w-parentRect.w);
	}
	if(pFrameBtn->m_Anchors.m_bTop==true && pFrameBtn->m_Anchors.m_bBottom==true)
	{
		r.h += (h-parentRect.h);
	}
	else if(pFrameBtn->m_Anchors.m_bBottom==true)
	{
		r.y += (h-parentRect.h);
	}
	pFrameBtn->m_Rect = r;
}

void MFrame::ResizeBtnsByAnchors(int w, int h)
{
	ResizeByAnchors(&m_BtnClose, m_OldRect, w, h);
	ResizeByAnchors(&m_BtnMinimize, m_OldRect, w, h);
}

void MFrame::OnCloseButtonClick()
{
	Show(false);
}
void MFrame::OnMinimizeButtonClick()
{
	SetShade(!m_bShade);
}
