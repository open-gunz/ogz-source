#include "stdafx.h"
#include "MCursor.h"
#include "MBitmap.h"
#include "MDrawContext.h"
#include "MAnimation.h"

MCursor::MCursor(const char* szName)
{
	_ASSERT(strlen(szName)<MCURSOR_NAME_LENGTH);
	strcpy_safe(m_szName, szName);
}

void MBitmapCursor::Draw(MDrawContext* pDC, int x, int y)
{
	pDC->SetBitmap(m_pBitmap);
	pDC->Draw(x, y);
}

MBitmapCursor::MBitmapCursor(const char* szName, MBitmap* pBitmap) : MCursor(szName)
{
	m_pBitmap = pBitmap;
}

void MAniBitmapCursor::Draw(MDrawContext* pDC, int x, int y)
{
	//m_pAnimation->SetScreenPosition(x, y);
	//MPOINT p = MClientToScreen(m_pAnimation, MPOINT(x, y));
	//m_pAnimation->SetPosition(p);
	//m_pAnimation->SetPosition(MPOINT(x,y));
	//m_pAnimation->Draw(pDC);
	MBitmap* pBitmap = m_pAnimation->GetBitmap();
	if( pBitmap == NULL ) return;
	
	pDC->SetBitmap(pBitmap);
	pDC->Draw(x,y);
}

MAniBitmapCursor::MAniBitmapCursor(const char* szName, MAniBitmap* pAniBitmap) : MCursor(szName)
{
	m_pAnimation = new MAnimation(szName, pAniBitmap);
	m_pAnimation->m_nPlayMode = MAPM_REPETITION;
}

MAniBitmapCursor::~MAniBitmapCursor(void)
{
	delete m_pAnimation;
}

