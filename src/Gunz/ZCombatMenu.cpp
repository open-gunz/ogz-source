#include "stdafx.h"
#include "ZCombatMenu.h"
#include "MWidget.h"
#include "ZGameInterface.h"

ZCombatMenu::ZCombatMenu()
{
	m_FrameStr = "CombatMenuFrame";

	m_ItemStr[ZCMI_OPTION]		= "OptionFrame";
	m_ItemStr[ZCMI_CLOSE]		= "CombatMenuClose";
	m_ItemStr[ZCMI_BATTLE_EXIT] = "BattleExit";
	m_ItemStr[ZCMI_STAGE_EXIT]	= "StageExit";
	m_ItemStr[ZCMI_PROG_EXIT]	= "Exit";
}

ZCombatMenu::~ZCombatMenu()
{

}

bool ZCombatMenu::IsEnableItem(ZCOMBAT_MENU_ITEM nItem)
{
	MWidget *pWidget= ZGetGameInterface()->GetIDLResource()->FindWidget( m_ItemStr[nItem].c_str() );
	if(pWidget) return pWidget->IsEnable();
	return false;
}

void ZCombatMenu::EnableItem(ZCOMBAT_MENU_ITEM nItem, bool bEnable)
{
	MWidget *pWidget= ZGetGameInterface()->GetIDLResource()->FindWidget( m_ItemStr[nItem].c_str() );
	if(pWidget) pWidget->Enable(bEnable);
}


void ZCombatMenu::ShowModal(bool bShow)
{
	MWidget *pWidget= ZGetGameInterface()->GetIDLResource()->FindWidget( m_FrameStr.c_str() );
	if(pWidget) pWidget->Show(bShow, true);
}

bool ZCombatMenu::IsVisible()
{
	MWidget* pMenuWidget = ZGetGameInterface()->GetIDLResource()->FindWidget( m_FrameStr.c_str() );
	if (pMenuWidget) return pMenuWidget->IsVisible();
	return false;
}