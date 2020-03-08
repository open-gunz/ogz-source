#include "stdafx.h"

#include "ZApplication.h"
#include "ZButton.h"
#include "ZSoundEngine.h"

bool g_bWaiting=false;

void ZButton::OnMouseIn(void)
{
	ZGetSoundEngine()->PlaySound("if_mouseover");
}

void ZButton::OnMouseOut(void)
{
}

void ZButton::OnButtonDown(void)
{
}

void ZButton::OnButtonUp(void)
{
}

ZButton::ZButton(const char* szName, MWidget* pParent, MListener* pListener)
: MButton(szName, pParent, pListener)
{
	m_nIllumination=0;
	m_bClicked=false;

	m_dwCurrentTime = 0;
	m_dwLastTime = 0;
	m_dwClickedTime = 0;
}

ZButton::~ZButton(void)
{
}

bool ZButton::OnShow(void)
{
	m_bClicked=false;
	m_dwCurrentTime=0;
	m_dwLastTime=GetGlobalTimeMS();
	return MButton::OnShow();
}

void ZButton::OnHide(void)
{
	m_dwCurrentTime=0;
	m_dwLastTime=GetGlobalTimeMS();

	MButton::OnHide();
}

void ZButton::OnDraw(MDrawContext* pDC)
{
	MButton::OnDraw(pDC);

	DWORD currenttime=GetGlobalTimeMS();
	DWORD elapsed=currenttime-m_dwLastTime;

	m_dwCurrentTime+=elapsed;

	int targetillu=0;

	if(GetType()==MBT_PUSH){
		if(GetCheck())
			targetillu=0;
		else
			targetillu=0;
	}
	else if(IsButtonDown())
		targetillu=64;
	else if(IsMouseOver())
		targetillu=64+(int)(196.f*pow(sin((float)m_dwCurrentTime/250.f),20));

	int d = (int)elapsed*(targetillu-(int)m_nIllumination)/200;

	m_nIllumination = max(min((int)m_nIllumination+d,255),0);

	DWORD ElapsedTimeFromClicked;
	if( m_dwCurrentTime - m_dwClickedTime  < 1 )
	{
		ElapsedTimeFromClicked = 0;
	}
	else
	{
		ElapsedTimeFromClicked = m_dwCurrentTime - m_dwClickedTime;
	}

	if(m_bClicked)
	{
		if(GetType()==MBT_NORMAL && m_dwCurrentTime-m_dwClickedTime<250.f)
		{
			//m_nIllumination=255*((int)((m_dwCurrentTime-m_dwClickedTime)/60)%2);
			m_nIllumination = 255 * ((ElapsedTimeFromClicked / 60) % 2 );
		}
		else
		{
			m_bClicked=false;
			g_bWaiting=false;
//			if(GetListener()) GetListener()->OnCommand(this, MBTN_CLK_MSG);
		}
	}

	MBitmapR2 *pBitmap=(MBitmapR2*)MBitmapManager::Get("button_glow.png");
	if(pBitmap)
	{
		DWORD defaultcolor = GetType()==MBT_NORMAL ? 0x20ff20 : 0xc0c020;
		DWORD opacity=(DWORD)pDC->GetOpacity()*(DWORD)m_nIllumination / 255 ;
		MRECT rt=GetRect();
		MDrawEffect prevEffect = pDC->GetEffect();
		pDC->SetEffect(MDE_ADD);
		MCOLOR prevColor = pDC->GetBitmapColor();
		pDC->SetBitmapColor(MCOLOR(defaultcolor));
		unsigned char prevOpacity = pDC->GetOpacity();
		pDC->SetOpacity(opacity);
		pDC->SetBitmap(pBitmap);
		pDC->Draw(0,0,rt.w,rt.h,0,0,64,32);
		pDC->SetBitmapColor(prevColor);
		pDC->SetEffect(prevEffect);
		pDC->SetOpacity(prevOpacity);
//		pBitmap->Draw(rt.x,rt.y,rt.w,rt.h,0,0,64,32,color,MDE_ADD);
	}

	m_dwLastTime=currenttime;
}

void ZButton::OnButtonClick()
{
	MButton::OnButtonClick();

	if(m_bClicked)	// 성질급한사람이 깜빡이는중에 또 누르면..
	{
		m_bClicked=false;	
		g_bWaiting=false;
//		if(GetListener()) GetListener()->OnCommand(this, MBTN_CLK_MSG);
		return;
	}

	// 깜빡이면서 기다리는동안 다른 버튼이 못눌리게 한다
//	if(g_bWaiting) return;

	ZGetSoundEngine()->PlaySound("if_click");

	m_dwClickedTime=m_dwCurrentTime;
	m_bClicked=true;
	g_bWaiting=true;
}




void ZBmButton::OnMouseIn(void)
{
	ZGetSoundEngine()->PlaySound("if_mouseover");
}

void ZBmButton::OnMouseOut(void)
{
}

void ZBmButton::OnButtonDown(void)
{
}

void ZBmButton::OnButtonUp(void)
{
}

ZBmButton::ZBmButton(const char* szName, MWidget* pParent, MListener* pListener)
: MBmButton(szName, pParent, pListener)
{
	m_nIllumination=0;
	m_bClicked=false;

	m_dwCurrentTime = 0;
	m_dwLastTime = 0;
	m_dwClickedTime = 0;
}

ZBmButton::~ZBmButton(void)
{
}

bool ZBmButton::OnShow(void)
{
	m_bClicked=false;
	m_dwCurrentTime=0;
	m_dwLastTime=GetGlobalTimeMS();
	return true;
}

void ZBmButton::OnHide(void)
{
	m_dwCurrentTime=0;
	m_dwLastTime=GetGlobalTimeMS();
}

void ZBmButton::OnDraw(MDrawContext* pDC)
{
  	MBmButton::OnDraw(pDC);

	DWORD currenttime=GetGlobalTimeMS();
	DWORD elapsed=currenttime-m_dwLastTime;

	m_dwCurrentTime+=elapsed;

	int targetillu=0;

	/*
	if(GetType()==MBT_PUSH){
		if(GetCheck())
			targetillu=255;
		else
			targetillu=0;
	}
	else 	*/
	/*
	if(IsButtonDown())
		targetillu=64;
	else if(IsMouseOver())
		targetillu=30+(int)(196.f*pow(sin((float)m_dwCurrentTime/250.f),20));
	*/

  	int d = (int)elapsed*(targetillu-(int)m_nIllumination)/200;

	DWORD ElapsedTimeFromClicked;
	if( m_dwCurrentTime - m_dwClickedTime  < 1 )
	{
		ElapsedTimeFromClicked = 0;
	}
	else
	{
		ElapsedTimeFromClicked = m_dwCurrentTime - m_dwClickedTime;
	}

  	m_nIllumination = max(min((int)m_nIllumination+d,255),0);
	if(m_bClicked)
	{
		if(GetType()==MBT_NORMAL && m_dwCurrentTime-m_dwClickedTime<250.f)
			//m_nIllumination=255*((int)((m_dwCurrentTime-m_dwClickedTime)/60)%2);
			m_nIllumination = 255 * ((ElapsedTimeFromClicked / 60) % 2 );
		else
		{
			m_bClicked=false;
			g_bWaiting=false;
//			if(GetListener()) GetListener()->OnCommand(this, MBTN_CLK_MSG);
		}
	}

	MBitmapR2 *pBitmap=(MBitmapR2*)MBitmapManager::Get("button_glow.png");
	if(pBitmap)
	{
		DWORD defaultcolor = GetType()==MBT_NORMAL ? 0x20ff20 : 0xc0c020;
		DWORD opacity=(DWORD)pDC->GetOpacity()*(DWORD)m_nIllumination / 255 ;
		MRECT rt=GetRect();
		MDrawEffect prevEffect = pDC->GetEffect();
		pDC->SetEffect(MDE_ADD);
		MCOLOR prevColor = pDC->GetBitmapColor();
		pDC->SetBitmapColor(MCOLOR(defaultcolor));
		unsigned char prevOpacity = pDC->GetOpacity();
		pDC->SetOpacity(opacity);
		pDC->SetBitmap(pBitmap);
		pDC->Draw(0,0,rt.w,rt.h,0,0,64,32);
		pDC->SetBitmapColor(prevColor);
		pDC->SetEffect(prevEffect);
		pDC->SetOpacity(prevOpacity);
// 		pBitmap->Draw(rt.x,rt.y,rt.w,rt.h,0,0,63,32,color,MDE_ADD);
	}

	m_dwLastTime=currenttime;
}

void ZBmButton::OnButtonClick()
{
	MBmButton::OnButtonClick();

	ZGetSoundEngine()->PlaySound("if_click");

	if(m_bClicked)	// 성질급한사람이 깜빡이는중에 또 누르면..
	{
		m_bClicked=false;
		g_bWaiting=false;
//		if(GetListener()) GetListener()->OnCommand(this, MBTN_CLK_MSG);
		return;
	}

	// 깜빡이면서 기다리는동안 다른 버튼이 못눌리게 한다
//	if(g_bWaiting) return;

	m_dwClickedTime=m_dwCurrentTime;
	m_bClicked=true;
	g_bWaiting=true;
}