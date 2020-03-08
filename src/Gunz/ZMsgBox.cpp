#include "stdafx.h"

#include "ZGameClient.h"
#include "ZApplication.h"
#include "ZMsgBox.h"
#include "Mint.h"


// ZMsgBox Listener
class ZMsgBoxListener : public MListener{
public:
	virtual bool OnCommand(MWidget* pWidget, const char* szMessage){
		ZMsgBox* pMsgBox = (ZMsgBox*)pWidget;
		if (pMsgBox->GetCustomListener()) {
			bool bResult = pMsgBox->GetCustomListener()->OnCommand(pWidget, szMessage);
			pMsgBox->SetCustomListener(NULL);
			return bResult;
		}

		if(MWidget::IsMsg(szMessage, MMSGBOX_OK)==true){
			const char* pText = pWidget->GetText();

			// 인원이 꽉차 종료
//			if(!_stricmp(pText, MGetErrorString(MERR_CLIENT_FULL_PLAYERS)))
			if(!_stricmp(pText, ZErrStr(MERR_CLIENT_FULL_PLAYERS)))
			{
				if (ZApplication::GetInstance()->GetLaunchMode() == ZApplication::ZLAUNCH_MODE_NETMARBLE)
				{

					ZApplication::Exit();
					return true;
				}
			}
		}

		pWidget->Show(false);

		return false;
	}
};
ZMsgBoxListener	g_MsgBoxListener;

MListener* ZGetMsgBoxListener(void)
{
	return &g_MsgBoxListener;
}

// ZConfirmMsgBox Listener : Default Listener 일뿐. 물어볼때마다 CustomListener 지정해서 쓸것
class ZConfirmMsgBoxListener : public MListener{
public:
	virtual bool OnCommand(MWidget* pWidget, const char* szMessage){
		ZMsgBox* pMsgBox = (ZMsgBox*)pWidget;
		if (pMsgBox->GetCustomListener()) {
			bool bResult = pMsgBox->GetCustomListener()->OnCommand(pWidget, szMessage);
			pMsgBox->SetCustomListener(NULL);
			return bResult;
		}

		pWidget->Show(false);
		return false;
	}
};
ZConfirmMsgBoxListener	g_CofirmMsgBoxListener;

MListener* ZGetConfirmMsgBoxListener(void)
{
	return &g_CofirmMsgBoxListener;
}

constexpr auto ZMSGBOX_TRANSIENT_TIME = 200;

void ZMsgBox::Show(bool bVisible, bool bModal)
{
	auto elapsed = (GetGlobalTimeMS() - m_nShowTime);

	if (m_bNextVisible == m_bVisible && m_bVisible == bVisible && elapsed > ZMSGBOX_TRANSIENT_TIME)
		return;

	if(m_bNextVisible!=bVisible){
		if (elapsed < ZMSGBOX_TRANSIENT_TIME)
			m_nShowTime = GetGlobalTimeMS() - (ZMSGBOX_TRANSIENT_TIME - elapsed);
		else
			m_nShowTime=GetGlobalTimeMS();
	}
	m_bNextVisible = bVisible;

	MMsgBox::Show(bVisible,bModal);
	m_bVisible = true;

	Enable(bVisible);
}

#define ZMSGBOX_W	400
#define ZMSGBOX_X	(MGetWorkspaceWidth()/2-ZMSGBOX_W/2)
#define ZMSGBOX_H	110
#define ZMSGBOX_Y	(MGetWorkspaceHeight()/2-ZMSGBOX_H/2)

bool ZMsgBox::OnShow()
{
	if(m_pOK!=NULL && m_pOK->IsVisible()==true) 
		m_pOK->SetFocus();
	else if(m_pCancel!=NULL) m_pCancel->SetFocus();

	SetPosition(ZMSGBOX_X,ZMSGBOX_Y);

	return true;
}

void ZMsgBox::OnDraw(MDrawContext* pDC)
{
	float fOpacity = 0;
	if(m_bNextVisible==false){	// Hide
		fOpacity = 1.0f - min(float(GetGlobalTimeMS() - m_nShowTime) / (float)ZMSGBOX_TRANSIENT_TIME, 1.0f);
		if(fOpacity==0.0f) {
			m_bVisible = false;
			m_bExclusive = false;
		}
		SetOpacity(u8(fOpacity*0xFF));
	}
	else{	// Show
		fOpacity = min(float(GetGlobalTimeMS() - m_nShowTime) / (float)ZMSGBOX_TRANSIENT_TIME, 1.0f);
		SetOpacity(u8(fOpacity*0xFF));
	}

	MMsgBox::OnDraw(pDC);
}

ZMsgBox::ZMsgBox(const char* szMessage, MWidget* pParent, MListener* pListener, MMsgBoxType nType)
: MMsgBox(szMessage,pParent,pListener,nType)
{
	m_bCanShade = false;
	m_bNextVisible = false;
	m_nShowTime = GetGlobalTimeMS() - ZMSGBOX_TRANSIENT_TIME * 2;
	SetOpacity(0);
	m_pCustomListener = NULL;
}

ZMsgBox::~ZMsgBox(void)
{
}

void ZMsgBox::SetText(const char* szText)
{
	MMsgBox::SetText(szText);

	int nLineCount = MMGetLineCount(m_pMessage->GetFont(),szText,ZMSGBOX_W);

	SetBounds(MRECT(ZMSGBOX_X, ZMSGBOX_Y, ZMSGBOX_W, ZMSGBOX_H + nLineCount*m_pMessage->GetFont()->GetHeight()));
}
