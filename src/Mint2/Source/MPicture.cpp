#include "stdafx.h"
#include "MPicture.h"
#include "math.h"

void MPicture::OnDraw(MDrawContext* pDC)
{
 	if(m_pBitmap==NULL) return;
	pDC->SetBitmap(m_pBitmap);

	MCOLOR tempColor = pDC->GetBitmapColor();
	pDC->SetBitmapColor( m_BitmapColor );
	auto tempDrawMode = m_pBitmap->GetDrawMode();
	m_pBitmap->SetDrawMode( m_DrawMode );

	int bw, bh;
	if( m_iStretch == 2 )	bw	= m_Rect.w;
	else		bw	= m_pBitmap->GetWidth();
	if( m_iStretch == 1 )	bh	= m_Rect.h; 
	else		bh	= m_pBitmap->GetHeight();
    
	int w, h;
	if (m_iStretch != 0)	{
		w = m_Rect.w;
		h = m_Rect.h;
	}
 	else	{
		w = bw;
		h = bh;
	}

 	if(m_bAnim)
		OnAnimDraw( pDC, 0,0, w, h, 0,0, bw, bh );
	else
 		pDC->Draw( 0, 0, w, h, 0, 0, bw, bh );

	pDC->SetBitmapColor( tempColor );
	m_pBitmap->SetDrawMode( tempDrawMode );
}

void MPicture::SetAnimation( int animType, float fAnimTime )
{
	m_bAnim = true;
	m_fAnimTime = fAnimTime;
	m_iAnimType = animType;
	m_CurrentTime = GetGlobalTimeMS();
}

void MPicture::OnAnimDraw( MDrawContext* pDC, int x, int y, int w, int h, int bx, int by, int bw, int bh )
{
   	auto e_time = GetGlobalTimeMS() - m_CurrentTime;
 	float ratio	= e_time / m_fAnimTime;
	
   	if( ratio >= 1.0f ) 
	{
		if(m_iAnimType == 2)
		{
 			m_iAnimType = 0;
			m_CurrentTime = GetGlobalTimeMS();
			if(m_bSwaped)
			{
				MCOLOR c = m_BitmapColor;
				m_BitmapColor = m_BitmapReseveColor;
				m_BitmapReseveColor = c;
				m_bSwaped = false;
			}
			return;
		}
		else if(m_iAnimType == 3) 
		{
			m_iAnimType = 1;
			m_CurrentTime = GetGlobalTimeMS();
			if(m_bSwaped)
			{
 				MCOLOR c = m_BitmapColor;
				m_BitmapColor = m_BitmapReseveColor;
				m_BitmapReseveColor = c;
				m_bSwaped = false;
			}
			return;
		}
		else
		{
			m_bAnim = false;
			pDC->Draw( 0, 0, w, h, 0, 0, bw, bh );
			return;
		}
	}

	int i_offset;
 	
  	if( m_iAnimType == 0 )
	{		
   		i_offset = (int)(bw*sqrtf(ratio));
		pDC->Draw( x,y,i_offset, h, bx+(bw-i_offset), by, i_offset, bh );
 	}
	else if( m_iAnimType == 1 )
	{
 		i_offset = (int)(bw*sqrtf(ratio));
		pDC->Draw( x+w-i_offset,y,i_offset, h, bx+(bw-i_offset), by, i_offset, bh );
	}
 	else if( m_iAnimType == 2 ) //<----
	{
//  		i_offset = w - w*ratio*ratio;	
		i_offset = (int)(bw - bw*sqrtf(ratio));	
		if(!m_bSwaped)
		{
			MCOLOR c = m_BitmapColor;
			m_BitmapColor = m_BitmapReseveColor;
			m_BitmapReseveColor = c;
			m_bSwaped = true;
			pDC->SetBitmapColor( m_BitmapColor );
		}
 		pDC->Draw( x,y,i_offset, h, bx+(bw-i_offset), by, i_offset, bh );
	}
	else if( m_iAnimType == 3 )
	{
		//i_offset = w - w*ratio*ratio;	
		i_offset = (int)(bw - bw*sqrtf(ratio));	
		if(!m_bSwaped)
		{
			MCOLOR c = m_BitmapColor;
			m_BitmapColor = m_BitmapReseveColor;
			m_BitmapReseveColor = c;
			m_bSwaped = true;
			pDC->SetBitmapColor( m_BitmapColor );
		}
		pDC->Draw( x+w-i_offset,y,i_offset, h, bx+(bw-i_offset), by, i_offset, bh );
	}
}

MPicture::MPicture(const char* szName, MWidget* pParent, MListener* pListener)
: MWidget(szName, pParent, pListener)
{
	m_pBitmap = NULL;
	m_bAnim = false;
	m_iStretch = 0;
	m_BitmapColor = 0xFFFFFFFF;
	m_BitmapReseveColor = 0xFFFFFFFF;
	m_DrawMode = MBM_Normal;
	m_bSwaped = false;
}

void MPicture::SetBitmap(MBitmap* pBitmap)
{
	m_pBitmap = pBitmap;

	if(m_pBitmap==NULL) return;

	if( m_iStretch == 0 ) SetSize(m_pBitmap->GetWidth(), m_pBitmap->GetHeight());
}

MBitmap* MPicture::GetBitmap(void)
{
	return m_pBitmap;
}

void MPicture::SetBitmapColor( MCOLOR color )
{
	m_BitmapReseveColor = m_BitmapColor;
	m_BitmapColor.r = color.r;
	m_BitmapColor.g = color.g;
	m_BitmapColor.b = color.b;
}