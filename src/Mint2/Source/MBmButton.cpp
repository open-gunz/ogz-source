#include "stdafx.h"
#include "MBmButton.h"

#define MDEPRECATED
#ifdef MDEPRECATED

void MBmButton::OnDownDraw(MDrawContext* pDC)
{
	if(m_pDownBitmap!=NULL){
		pDC->SetBitmap(m_pDownBitmap);
		MRECT r = GetClientRect();
		if( m_bStretch )
			pDC->Draw(r.x-2, r.y-2, r.w, r.h );
		else
			pDC->Draw(r.x-2, r.y-2);
	}
}

void MBmButton::OnUpDraw(MDrawContext* pDC)
{
	if(m_pUpBitmap!=NULL){
		pDC->SetBitmap(m_pUpBitmap);
		MRECT r = GetClientRect();
		if( m_bStretch )
			pDC->Draw(r.x-2, r.y-2, r.w, r.h );
		else
			pDC->Draw(r.x-2, r.y-2);
	}
}

void MBmButton::OnOverDraw(MDrawContext* pDC)
{
	if(m_pUpBitmap!=NULL){
		pDC->SetBitmap(m_pOverBitmap);
		MRECT r = GetClientRect();
		if( m_bStretch )
			pDC->Draw(r.x-2, r.y-2, r.w, r.h );
		else
			pDC->Draw(r.x-2, r.y-2);
	}
}

void MBmButton::OnDisableDraw(MDrawContext* pDC)
{
	if(m_pDisableBitmap!=NULL){
		pDC->SetBitmap(m_pDisableBitmap);
		MRECT r = GetClientRect();
		if( m_bStretch )
			pDC->Draw(r.x-2, r.y-2, r.w, r.h );
		else
			pDC->Draw(r.x-2, r.y-2);
	}
	else{
		if(m_pUpBitmap!=NULL){
			pDC->SetBitmap(m_pUpBitmap);
			MRECT r = GetClientRect();
			if( m_bStretch )
				pDC->Draw(r.x-2, r.y-2, r.w, r.h );
			else
				pDC->Draw(r.x-2, r.y-2);
		}
	}
}

void MBmButton::OnDraw(MDrawContext* pDC)
{
	if(IsEnable()==false){
		OnDisableDraw(pDC);
	}
	else if((GetType()==MBT_NORMAL && IsButtonDown()==true) 
		|| (GetType()==MBT_PUSH && GetCheck())) {
		OnDownDraw(pDC);
	}
	else if(IsMouseOver()==true){
		OnOverDraw(pDC);
	}
	else{
		OnUpDraw(pDC);
	}

	if( m_bTextColor )
	{
		pDC->SetColor( m_BmTextColor );
		pDC->Text(GetClientRect(), m_szName, GetAlignment());
	}
	else
        GetLook()->OnDrawText( this, this->GetClientRect(), pDC );
}

MBmButton::MBmButton(const char* szName, MWidget* pParent, MListener* pListener)
: MButton(szName, pParent, pListener)
{
	m_pUpBitmap = NULL;
	m_pDownBitmap = NULL;
	m_pDisableBitmap = NULL;
	m_pOverBitmap = NULL;
	m_bStretch = false;
	m_bTextColor	= false;
}

void MBmButton::SetUpBitmap(MBitmap* pBitmap)
{
 	m_pUpBitmap = pBitmap;
	if( !m_bStretch && m_pUpBitmap!=NULL)
		SetSize(m_pUpBitmap->GetWidth(), m_pUpBitmap->GetHeight());
}

void MBmButton::SetDownBitmap(MBitmap* pBitmap)
{
	m_pDownBitmap = pBitmap;
	if(!m_bStretch && m_pDownBitmap!=NULL && m_pUpBitmap==NULL)
		SetSize(m_pDownBitmap->GetWidth(), m_pDownBitmap->GetHeight());
}

void MBmButton::SetDisableBitmap(MBitmap* pBitmap)
{
	m_pDisableBitmap = pBitmap;
	if(!m_bStretch && m_pDisableBitmap!=NULL && m_pDownBitmap==NULL && m_pUpBitmap==NULL)
		SetSize(m_pDisableBitmap->GetWidth(), m_pDisableBitmap->GetHeight());
}

void MBmButton::SetOverBitmap(MBitmap* pBitmap)
{
	m_pOverBitmap = pBitmap;
}

#endif