#include "stdafx.h"
#include "MMsgBox.h"
#include "MStringTable.h"
#include "Mint.h"

#ifdef GetMessage
#undef GetMessage
#endif

int charinstr(char* sz, char c)
{
	int len = strlen(sz);
	for(int i=0; i<len; i++){
		if(sz[i]==c) return i;
	}
	return -1;
}

/*
void MMsgBox::OnDrawText(MDrawContext* pDC)
{
	//MFrameDrawer* pFD = GetFrameDrawer();

	pDC->SetColor(MCOLOR(255, 255, 255));

	char* pStr = m_szMessage;
	int nLen = charinstr(pStr, '\n');
	int nY = m_Rect.y+10;
	if(nLen<0){
		//pFD->Text(pDC, MPOINT(m_Rect.x+10, m_Rect.y+10), pStr, MFDTS_NORMAL, false, &m_Rect);
		pDC->Text(m_Rect.x+10, m_Rect.y+10, pStr);
	}
	else{
		do{
			char temp[MWIDGET_NAME_LENGTH];
			memcpy(temp, pStr, nLen);
			temp[nLen] = 0;
			//pFD->Text(pDC, MPOINT(m_Rect.x+10, nY), temp, MFDTS_NORMAL, false, &m_Rect);
			pDC->Text(m_Rect.x+10, nY, temp);
			pStr += (nLen+1);
			nLen = charinstr(pStr, '\n');
			nY += GetFont()->GetHeight();
		}while(nLen>0);
	}
}

void MMsgBox::OnDraw(MDrawContext* pDC)
{
	OnDrawText(pDC);
}
*/
#define MMSGBOX_W	400
#define MMSGBOX_X	(MGetWorkspaceWidth()/2-MMSGBOX_W/2)
#define MMSGBOX_H	140
#define MMSGBOX_Y	(MGetWorkspaceHeight()/2-MMSGBOX_H/2)

#define MMSGBOX_OK_W	80
#define MMSGBOX_OK_H	32

bool MMsgBox::OnShow(void)
{
  	SetBounds( MRECT(MMSGBOX_X, MMSGBOX_Y, MMSGBOX_W, MMSGBOX_H) );
    
	if(m_pOK!=NULL && m_pOK->IsVisible()==true) 
		m_pOK->SetFocus();
	else if(m_pCancel!=NULL) m_pCancel->SetFocus();

	return true;
}

void MMsgBox::OnSize(int w, int h)
{
 	MRECT r = GetInitialClientRect();
	m_pMessage->SetBounds(MRECT(r.x+19, r.y+40, r.w-38, r.h-70));
	int nOKOffset = 0;
	if(m_nType==MT_OKCANCEL || m_nType==MT_YESNO) nOKOffset = MMSGBOX_OK_W+5;
	if(m_pOK!=NULL) m_pOK->SetBounds(MRECT(r.x+r.w-MMSGBOX_OK_W-nOKOffset - 19, r.y+r.h-MMSGBOX_OK_H-20, MMSGBOX_OK_W, MMSGBOX_OK_H));
	if(m_pCancel!=NULL) m_pCancel->SetBounds(MRECT(r.x+r.w-MMSGBOX_OK_W - 19, r.y+r.h-MMSGBOX_OK_H-20, MMSGBOX_OK_W, MMSGBOX_OK_H));
}

MMsgBox::MMsgBox(const char* szMessage, MWidget* pParent, MListener* pListener, MMsgBoxType nType)
: MFrame(MGetString(MSID_MESSAGE), pParent, (pListener==NULL)?pParent:pListener)
{
//	m_pMessage = new MLabel(szMessage, this, this);
	m_pMessage = new MTextArea(255, szMessage, this, this);
	m_pMessage->SetEditable(false);

	m_pOK = NULL;
	m_pCancel = NULL;

	if(nType!=MT_NOTDECIDED)
		SetType(nType);

	m_bResizable = false;

	SetBounds(MRECT(MMSGBOX_X, MMSGBOX_Y, MMSGBOX_W, MMSGBOX_H));

	MMsgBox::OnSize(MMSGBOX_W, MMSGBOX_H);

	Show(false);
}

MMsgBox::~MMsgBox(void)
{
	delete m_pMessage;
	if(m_pOK!=NULL) delete m_pOK;
	if(m_pCancel!=NULL) delete m_pCancel;
}

void MMsgBox::SetType(MMsgBoxType nType)
{
	m_nType = nType;

	if(m_pOK) delete m_pOK;
	if(m_pCancel) delete m_pCancel;

	if(nType!=MT_CANCEL){
		//m_pOK = new MButton(MGetString(MSID_OK), this, this);
		m_pOK = (MButton*)Mint::GetInstance()->NewWidget(MINT_BUTTON, MGetString(MSID_OK), this, this );
		m_pOK->SetLabelAccelerator();
		m_pOK->m_nKeyAssigned = MBKA_ENTER;
	}
	if(nType==MT_YESNO) m_pOK->SetText(MGetString(MSID_YES));

	if(nType==MT_OKCANCEL || nType==MT_YESNO || nType==MT_CANCEL){
		//m_pCancel = new MButton(MGetString(MSID_CANCEL), this, this);
		m_pCancel = (MButton*)Mint::GetInstance()->NewWidget(MINT_BUTTON, MGetString(MSID_CANCEL), this, this );
		if(nType==MT_YESNO) m_pCancel->SetText(MGetString(MSID_NO));
		m_pCancel->SetAccelerator(27);
		m_pCancel->m_nKeyAssigned = MBKA_ESC;
	}
	MMsgBox::OnSize(MMSGBOX_W, MMSGBOX_H);
}

bool MMsgBox::OnCommand(MWidget* pWindow, const char* szMessage)
{
	MListener* pListener = GetListener();
	if(pListener==NULL) return false;

	if(pWindow==m_pOK && strcmp(szMessage, MBTN_CLK_MSG)==0){
		if(m_nType==MT_YESNO) pListener->OnCommand(this, MMSGBOX_YES);
		else pListener->OnCommand(this, MMSGBOX_OK);
		return true;
	}
	else if(pWindow==m_pCancel && strcmp(szMessage, MBTN_CLK_MSG)==0){
		if(m_nType==MT_YESNO) pListener->OnCommand(this, MMSGBOX_NO);
		else pListener->OnCommand(this, MMSGBOX_CANCEL);
		return true;
	}

	return false;
}

void MMsgBox::SetTitle(const char* szTitle)
{
	strcpy_safe(m_szName, szTitle);
}

const char* MMsgBox::GetTitle(void)
{
	return m_szName;
}

void MMsgBox::SetMessage(const char* szMessage)
{
	m_pMessage->SetText(szMessage);
}

const char* MMsgBox::GetMessage(void)
{
	static char buffer[256];
	buffer[0]=0;
	if(m_pMessage->GetText(buffer,sizeof(buffer)))
		return buffer;
	return buffer;
}

void MMsgBox::SetText(const char* szText)
{
	SetMessage(szText);
}

const char* MMsgBox::GetText(void)
{
	return GetMessage();
}
