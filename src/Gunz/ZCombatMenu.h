#pragma once

#include <string>

class ZCombatMenu
{
public:
	enum ZCOMBAT_MENU_ITEM
	{
		ZCMI_OPTION = 0,
		ZCMI_CLOSE,
		ZCMI_BATTLE_EXIT,
		ZCMI_STAGE_EXIT,
		ZCMI_PROG_EXIT,

		ZCMI_END
	};

private:
	string		m_ItemStr[ZCMI_END];
	string		m_FrameStr;
public:
	ZCombatMenu();
	~ZCombatMenu();
	bool IsEnableItem(ZCOMBAT_MENU_ITEM nItem);
	void EnableItem(ZCOMBAT_MENU_ITEM nItem, bool bEnable);

	void ShowModal(bool bShow=true);
	bool IsVisible();
};