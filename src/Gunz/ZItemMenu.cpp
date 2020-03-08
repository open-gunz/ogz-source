#include "stdafx.h"
#include "ZItemMenu.h"
#include "ZApplication.h"
#include "ZGameInterface.h"
#include "ZGameClient.h"
#include "ZPost.h"
#include "ZIDLResource.h"
#include "ZEquipmentListBox.h"


ZItemMenuListener listenerItemMenu;


//// ZItemMenuItem ////
ZItemMenuItem::ZItemMenuItem(ZCMD_ITEMMENU nCmdID, const char* szName) : MMenuItem(szName)
{
	m_nCmdID = nCmdID;
}


//// ZItemMenu ////
ZItemMenu::ZItemMenu(const char* szName, MWidget* pParent, MListener* pListener, MPopupMenuTypes t) 
: MPopupMenu(szName, pParent, pListener, t)
{
	m_szItemName[0] = NULL;
}

void ZItemMenu::AddMenuItem(ZItemMenuItem* pMenuItem)
{
	MPopupMenu::AddMenuItem(pMenuItem);
	pMenuItem->SetListener(&listenerItemMenu);
}

void ZItemMenu::SetupMenu()
{
	RemoveAllMenuItem();

	ZIDLResource* pResource = ZApplication::GetGameInterface()->GetIDLResource();
	ZEquipmentListBox* pItemListBox = (ZEquipmentListBox*)pResource->FindWidget("EquipmentList");

	ZItemMenuItem* pMenuItem = new ZItemMenuItem(ZCMD_ITEMMENU_BRINGBACK_ACCOUNTITEM, ZMsg( MSG_MENUITEM_SENTTOBANK));
	pMenuItem->SetSize(GetFont()->GetWidth(pMenuItem->GetText())+5, GetFont()->GetHeight());
	AddMenuItem(pMenuItem);	// AddÇÏ¸é¼­ MenuItem ResizeµÊ
}

void ZItemMenu::Show(int x, int y, bool bVisible)
{
	MPopupMenu::Show(x, y, bVisible);
}


//// ZItemMenuListener ////
bool ZItemMenuListener::OnCommand(MWidget* pWidget, const char* szMessage)
{
	GunzState GunzState = ZApplication::GetGameInterface()->GetState();
	ZIDLResource* pResource = ZApplication::GetGameInterface()->GetIDLResource();

	ZItemMenu* pMenu = (ZItemMenu*)pWidget->GetParent();
	((MPopupMenu*)pMenu)->Show(false);
	ZItemMenuItem* pItem = (ZItemMenuItem*)pWidget;

	switch(pItem->GetCmdID()) {
	case ZCMD_ITEMMENU_BRINGBACK_ACCOUNTITEM:
		{
			ZPostRequestBringBackAccountItem(ZGetMyUID(), pMenu->GetTargetUID());
		}
		return true;
	};
	return false;
}
