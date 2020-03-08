#include "stdafx.h"
#include "ZBmNumLabel.h"

ZBmNumLabel::ZBmNumLabel(const char* szName, MWidget* pParent, MListener* pListener)
: MWidget(szName, pParent, pListener)
{
	m_pLabelBitmap = NULL;
	m_AlignmentMode = MAM_RIGHT;
	m_nIndexOffset = 0;

	SetCharSize( MSIZE(32, 32));
}


void ZBmNumLabel::SetLabelBitmap(MBitmap* pLabelBitmap)
{
	m_pLabelBitmap = pLabelBitmap;

	if(m_pLabelBitmap==NULL) return;
	SetSize(m_pLabelBitmap->GetWidth(), m_pLabelBitmap->GetHeight());
}

void ZBmNumLabel::SetCharSize(const MSIZE &size)
{
	m_CharSize = size;
	
	for ( int i = 0;  i < BMNUM_NUMOFCHARSET;  i++)
		m_nCharMargin[ i] = size.w;
}

void ZBmNumLabel::SetCharMargin( int* nMargin)
{
	for ( int i = 0;  i < BMNUM_NUMOFCHARSET;  i++)
		m_nCharMargin[ i] = *(nMargin + i);
}

void ZBmNumLabel::SetNumber(int n,bool bAddComma)
{
	char buffer[256];
	sprintf_safe(buffer,"%d",n);

	int nLength = (int)strlen(buffer);

	char *pDest = m_szName;
	for(int i=0;i<nLength;i++)
	{
		if ( bAddComma)
		{
			if(i>0 && ((nLength-i)%3 == 0))
				*pDest++=',';
		}

		*pDest++=buffer[i];
	}
	*pDest = 0;
}

// 폰트 순서 하드코드 =_=
int GetIndex(char c)
{
	int n=-1;
	switch (c)
	{
	case ',' : n = 10; break;
	case '/' : n = 11; break;
	case '.' : n = 12; break;
	default:
		if (isdigit(c))
			n = c - '0';
	}
	return n;
}

void ZBmNumLabel::OnDraw(MDrawContext* pDC)
{
	// 높이가 원래는 32 이다. 해상도에 따라 비율을 맞춘다
	float fRatio = (float)m_Rect.h / (float)32;

	if (m_pLabelBitmap == NULL) return;

	pDC->SetBitmap(m_pLabelBitmap);

	int sx = 0, sy = 0;
//	int tx = m_Rect.x, ty = m_Rect.y;

	int nLen = (int)strlen(m_szName);
	int nTexCol = m_pLabelBitmap->GetWidth() / m_CharSize.w;

	int nTextWidth = 0;
	for(int i=0;i<nLen; i++) {
		int n = GetIndex(m_szName[i]);
		if(n!=-1)
		{
			int nWidth = (int) ( fRatio * m_nCharMargin[n] );
			nTextWidth += nWidth;
		}
	}
	int nLastCharWidth = (int) ( fRatio * m_nCharMargin[nLen] );

	int tx = 0, ty = 0;

	if((m_AlignmentMode & MAM_LEFT) != NULL)
		tx = 0;

	else if((m_AlignmentMode & MAM_RIGHT) != NULL)
		tx = m_Rect.w - nTextWidth;

	else if((m_AlignmentMode & MAM_HCENTER) != NULL)
		tx = m_Rect.w / 2 - nTextWidth / 2;


	for (int i = 0; i < nLen; i++)
	{
		int n = GetIndex(m_szName[i]);
		if(n!=-1)
		{
			int nWidth = m_nCharMargin[n];

			sx = ((n+m_nIndexOffset) % nTexCol) * m_CharSize.w;
			sy = ((n+m_nIndexOffset) / nTexCol) * m_CharSize.h;
			MRECT SrcRect = MRECT(sx, sy, m_CharSize.w, m_CharSize.h);
//			pDC->SetOpacity(255);
			pDC->Draw(tx, ty, (int)(fRatio * nWidth), (int)(fRatio * m_CharSize.h), sx+(m_CharSize.w-nWidth)/2, sy, nWidth, m_CharSize.h);

			tx += (int)(fRatio * nWidth);
		}
	}
}