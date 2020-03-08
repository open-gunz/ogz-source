#pragma once
#include <MXml.h>

struct GGSet
{
	// Contains the zitem IDs of melee, primary, secondary, custom1, and custom2 weapons.
	u32 WeaponIDs[5]{};
	
	// A slot can be specified to hold whatever they had in their inventory when they joined the
	// stage, which is stored in the first 5 bits of this value.
	u32 Mask{};

	bool IsInventory(int Index) const
	{
		return static_cast<bool>(Mask & (1 << (Index + 5)));
	}

	void SetInventory(int Index, bool Value)
	{
		if (Value)
		{
			Mask |= 1 << (Index + 5);
		}
		else
		{
			Mask &= ~(1 << (Index + 5));
		}
	}
};

class GunGame
{
public:
	bool ReadXML(const char* szFileName);
	static GunGame* GetInstance();
	
	const std::vector<GGSet>& GetRandomWeaponSet();

	bool RandomizeWeaponOrder = false;
	int MinKillsPerLevel = 1;

private:
	bool ParseXML_ItemSet(MXmlElement& Element, GGSet& Node);
	bool ParseXML_Options(MXmlElement& Element);

	std::vector<std::vector<GGSet>> WeaponSets;
};

inline GunGame* MGetGunGame() { return GunGame::GetInstance(); }