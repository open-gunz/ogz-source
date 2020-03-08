#include "stdafx.h"
#include "ZFrame.h"
#include "Mint.h"

constexpr auto ZFRAME_TRANSIENT_TIME = 300;

void ZFrame::Show(bool bVisible, bool bModal)
{
	auto elapsed = (GetGlobalTimeMS() - m_nShowTime);

	if (m_bNextVisible == m_bVisible && m_bVisible == bVisible && elapsed > ZFRAME_TRANSIENT_TIME)
		return;

	if(m_bNextVisible!=bVisible){
		if (elapsed < ZFRAME_TRANSIENT_TIME)
			m_nShowTime = GetGlobalTimeMS() - (ZFRAME_TRANSIENT_TIME - elapsed);
		else
			m_nShowTime=GetGlobalTimeMS();
	}
	m_bNextVisible = bVisible;

	MFrame::Show(bVisible,bModal);
	m_bVisible = true;
	
 	Enable(bVisible);

	if(bVisible)
		m_bExclusive=bModal;
}

void ZFrame::OnDraw(MDrawContext* pDC)
{
	float fOpacity = 0;
	if (m_bNextVisible == false) {	// Hide
		fOpacity = 1.0f - min(float(GetGlobalTimeMS() - m_nShowTime) / (float)ZFRAME_TRANSIENT_TIME, 1.0f);
		if(fOpacity==0.0f) {
			m_bVisible = false;
			m_bExclusive = false;
		}
		SetOpacity(u8(fOpacity*0xFF));
	}
	else{	// Show
		fOpacity = min(float(GetGlobalTimeMS() - m_nShowTime) / (float)ZFRAME_TRANSIENT_TIME, 1.0f);
		SetOpacity(u8(fOpacity*0xFF));
	}

 	if( m_bExclusive ){

		MRECT Full(0, 0, MGetWorkspaceWidth()-1, MGetWorkspaceHeight()-1);
		MRECT PrevClip = pDC->GetClipRect();
		pDC->SetClipRect(Full);
		unsigned char oldopacity=pDC->SetOpacity(200*fOpacity);
		pDC->SetColor(0, 0, 0, 255);
		MPOINT PrevOrigin = pDC->GetOrigin();
		pDC->SetOrigin(0, 0);
		pDC->FillRectangle(Full);

		// º¹±¸
		pDC->SetClipRect(PrevClip);
		pDC->SetOrigin(PrevOrigin);
		pDC->SetOpacity(oldopacity);
	}

	MFrame::OnDraw(pDC);
}

ZFrame::ZFrame(const char* szName, MWidget* pParent, MListener* pListener)
			: MFrame(szName, pParent, pListener)
{
	m_bCanShade = false;
	m_bNextVisible = false;
	m_nShowTime = GetGlobalTimeMS() - ZFRAME_TRANSIENT_TIME * 2;
	SetOpacity(0);
}

ZFrame::~ZFrame(void)
{
}