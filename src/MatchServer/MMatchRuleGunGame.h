#pragma once
#include "MMatchRule.h"
#include "GunGame.h"

class MMatchRuleGunGame : public MMatchRule {
public:
	using MMatchRule::MMatchRule;

	virtual MMATCH_GAMETYPE GetGameType() override { return MMATCH_GAMETYPE_GUNGAME; }
	virtual void OnEnterBattle(MUID& uidPlayer) override;

protected:
	virtual void OnBegin() override;
	virtual void OnEnd() override;
	virtual void OnRoundTimeOut() override;
	virtual void OnGameKill(const MUID& uidAttacker, const MUID& uidVictim) override;
	virtual bool OnCheckRoundFinish() override;
	virtual bool RoundCount() override;

private:
	void SendNewSet(MMatchObject* Player, int KillCount);
	bool CheckKillCount(MMatchObject* pOutObj);

	// If MGetGunGame()->RandomizeWeaponOrder is false, CurrentWeaponSet will point to a weapon set
	// in GunGame and RandomizedWeaponSet will be empty. Otherwise, CurrentWeaponSet will be null
	// and RandomizedWeaponSet will hold a shuffled set from GunGame.
	const std::vector<GGSet>* CurrentWeaponSet{};
	std::vector<GGSet> RandomizedWeaponSet;

	const std::vector<GGSet>& GetWeaponSet() const
	{
		if (!MGetGunGame()->RandomizeWeaponOrder)
		{
			return *CurrentWeaponSet;
		}

		return RandomizedWeaponSet;
	}
};