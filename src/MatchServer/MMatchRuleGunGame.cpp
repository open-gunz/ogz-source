#include "stdafx.h"
#include "MMatchRuleGunGame.h"
#include "MBlobArray.h"
#include <algorithm>
#include <random>

void MMatchRuleGunGame::OnBegin()
{
	if (!MGetGunGame()->RandomizeWeaponOrder)
	{
		CurrentWeaponSet = &MGetGunGame()->GetRandomWeaponSet();
	}
	else
	{
		RandomizedWeaponSet = MGetGunGame()->GetRandomWeaponSet();

		std::random_device Device;
		std::mt19937 RNG(Device());

		std::shuffle(RandomizedWeaponSet.begin(), RandomizedWeaponSet.end(), RNG);
	}
}

void MMatchRuleGunGame::OnEnd()
{
	RandomizedWeaponSet.clear();
}

bool MMatchRuleGunGame::RoundCount()
{
	++m_nRoundCount;

	return m_nRoundCount < 1;
}

void MMatchRuleGunGame::OnRoundTimeOut()
{
	SetRoundArg(MMATCH_ROUNDRESULT_DRAW);
}

void MMatchRuleGunGame::OnGameKill(const MUID& uidAttacker, const MUID& uidVictim)
{
	MMatchObject* pAttacker = MGetMatchServer()->GetObject(uidAttacker);
	MMatchObject* pVictim = MGetMatchServer()->GetObject(uidVictim);
	if (!GetStage())
		return;

	if (!pAttacker || !pVictim)
		return;

	if (pAttacker == pVictim)
		return;

	auto KillCount = pAttacker->GetKillCount();
	if (KillCount % MGetGunGame()->MinKillsPerLevel != 0)
		return;

	SendNewSet(pAttacker, KillCount);
}

bool MMatchRuleGunGame::CheckKillCount(MMatchObject* pOutObj)
{
	MMatchStage* pStage = GetStage();
	for (auto* pObj : pStage->GetObjectList())
	{
		if (!pObj->GetEnterBattle())
			continue;

		if (pObj->GetKillCount() >= (unsigned int)pStage->GetStageSetting()->GetRoundMax())
		{
			pOutObj = pObj;
			return true;
		}
	}
	return false;
}

bool MMatchRuleGunGame::OnCheckRoundFinish()
{
	MMatchObject* pObj = nullptr;

	if (CheckKillCount(pObj))
	{
		return true;
	}
	return false;
}

void MMatchRuleGunGame::SendNewSet(MMatchObject* Player, int KillCount)
{
	int Level = KillCount;
	if (MGetGunGame()->MinKillsPerLevel > 1)
	{
		Level = KillCount / MGetGunGame()->MinKillsPerLevel;
	}

	int MaxLevel = int(GetWeaponSet().size());
	if (Level >= MaxLevel)
	{
		// If Level == MaxLevel, this is normal; they achieved their final kill. In This case, we
		// want to return early, since they can't advance further, but we don't want to log an
		// error.
		if (Level > MaxLevel)
		{
			MLog("MMatchRuleGunGame::SendNewSet -- Player reached level %d, even though there are "
				"only %zu levels.", Level, GetWeaponSet().size());
			assert(false);
		}
		return;
	}

	auto&& Set = GetWeaponSet()[Level];

	MCommand* pCommand = MGetMatchServer()->CreateCommand(MC_MATCH_GUNGAME_SEND_NEW_WEAPON, MUID(0, 0));
	pCommand->AddParameter(new MCmdParamUID(Player->GetUID()));
	for (int i = 0; i < 5; ++i)
	{
		u32 WeaponID = Set.WeaponIDs[i];
		if (Set.IsInventory(i))
		{
			auto Parts = MMatchCharItemParts(int(MMCIP_MELEE) + i);
			auto* Item = Player->GetCharInfo()->m_EquipedItem.GetItem(Parts);
			auto* Desc = Item ? Item->GetDesc() : nullptr;
			if (!Desc)
			{
				WeaponID = 0;
			}
			else
			{
				WeaponID = Desc->m_nID;
			}
		}

		pCommand->AddParameter(new MCmdParamUInt(WeaponID));
	}

	MGetMatchServer()->RouteToBattle(GetStage()->GetUID(), pCommand);
}

void MMatchRuleGunGame::OnEnterBattle(MUID& uidPlayer)
{
	MMatchObject* Object = MGetMatchServer()->GetObject(uidPlayer);
	if (!Object)
		return;

	SendNewSet(Object, 0);

	for (auto* pObj : GetStage()->GetObjectList())
	{
		if (pObj->GetUID() == uidPlayer || !pObj->GetEnterBattle())
			continue;

		SendNewSet(pObj, pObj->GetKillCount());
	}
}