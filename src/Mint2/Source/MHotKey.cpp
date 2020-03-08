#include "stdafx.h"
#include "MHotKey.h"
#include "Mint.h"

IMPLEMENT_LOOK(MHotKey, MEditLook)


void MHotKey::OnRun()
{
	if(IsFocus()==false) return;

	char szHotKeyName[128] = "";
	GetHotKeyName(szHotKeyName);
	SetText(szHotKeyName);
}

bool MHotKey::OnEvent(MEvent* pEvent, MListener* pListener)
{
	switch(pEvent->nMessage){
	case MWM_KEYDOWN:
		m_nKey = pEvent->nKey;
		if(pEvent->nKey==VK_SHIFT || pEvent->nKey==VK_CONTROL || pEvent->nKey==VK_MENU) m_nKey = -1;
		m_bCtrl = MEvent::GetCtrlState();
		m_bAlt = MEvent::GetAltState();
		m_bShift = MEvent::GetShiftState();
		return true;
	case MWM_KEYUP:
		if(pEvent->nKey==VK_SHIFT && m_nKey==-1) m_bShift = false;
		if(pEvent->nKey==VK_CONTROL && m_nKey==-1) m_bCtrl = false;
		if(pEvent->nKey==VK_MENU && m_nKey==-1) m_bAlt = false;
		return true;
	case MWM_CHAR:
		return true;
	}
	return false;
}

MHotKey::MHotKey(const char* szName, MWidget* pParent, MListener* pListener)
: MEdit(szName, pParent, pListener)
{
	m_bCtrl = false;
	m_bAlt = false;
	m_bShift = false;
	m_nKey = -1;
}

void MHotKey::GetHotKey(unsigned int* pKey, bool* pCtrl, bool* pAlt, bool* pShift)
{
	if(pKey!=NULL) *pKey = m_nKey;
	if(pCtrl!=NULL) *pCtrl = m_bCtrl;
	if(pAlt!=NULL) *pAlt = m_bAlt;
	if(pShift!=NULL) *pShift = m_bShift;
}

int MHotKey::RegisterHotKey()
{
	return Mint::GetInstance()->RegisterHotKey((m_bCtrl?MMODIFIER_CTRL:0)|(m_bAlt?MMODIFIER_ALT:0)|(m_bShift?MMODIFIER_SHIFT:0), m_nKey);
}

void MHotKey::UnregisterHotKey(int nID)
{
	Mint::GetInstance()->UnregisterHotKey(nID);
}
