#include "stdafx.h"
#include "ZRuleGunGame.h"

bool ZRuleGunGame::OnCommand(MCommand* pCommand)
{
	if (!ZGetGame())
		return false;

	switch (pCommand->GetID())
	{
	case MC_MATCH_GUNGAME_SEND_NEW_WEAPON:
	{
		u32 WeaponSet[5];
		MUID uidPlayer;

		if (!pCommand->GetParameter(&uidPlayer, 0, MPT_UID))
			break;

		for (int i = 0; i < 5; ++i)
		{
			if (!pCommand->GetParameter(&WeaponSet[i], i + 1, MPT_UINT))
				break;
		}

		auto* Player = ZGetCharacterManager()->Find(uidPlayer);
		if (!Player)
			break;
		SetPlayerWeapons(Player, WeaponSet);
	}
	break;
	}
	return false;
}

void ZRuleGunGame::SetPlayerWeapons(ZCharacter* pChar, const u32* WeaponSetArray)
{
	for (int i = 0; i < 5; ++i)
	{
		auto Parts = MMatchCharItemParts(int(MMCIP_MELEE) + i);
		pChar->m_Items.EquipItem(Parts, WeaponSetArray[i]);
	}

	// Refresh bullet counts for our new weapon set.
	pChar->InitBullet();

	// If our selected slot is now empty, we want to select a new weapon.
	auto SelectedParts = pChar->GetItems()->GetSelectedWeaponParts();
	if (pChar->GetItems()->GetItem(SelectedParts)->IsEmpty())
	{
		pChar->SelectWeapon();
	}
	else
	{
		// If we didn't change slot, the model and motion type won't have been updated if they
		// differ, so we do that manually.
		auto* Desc = pChar->GetSelectItemDesc();
		if (Desc)
		{
			pChar->OnChangeWeapon(Desc);
		}
	}
}