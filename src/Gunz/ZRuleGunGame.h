#pragma once
#include "ZRule.h"

class ZRuleGunGame : public ZRule
{
public:
	using ZRule::ZRule;
	virtual bool OnCommand(MCommand* pCommand) override;
	void SetPlayerWeapons(ZCharacter* pChar, const u32* WeaponSetArray);
};