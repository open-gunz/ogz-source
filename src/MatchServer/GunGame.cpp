#include "GunGame.h"
#include "GunGame.h"
#include "stdafx.h"
#include "GunGame.h"

GunGame* GunGame::GetInstance()
{
	static GunGame Instance;

	return &Instance;
}

const std::vector<GGSet>& GunGame::GetRandomWeaponSet()
{
	return WeaponSets[rand() % WeaponSets.size()];
}

bool GunGame::ReadXML(const char* szFileName)
{
	WeaponSets.clear();

	MXmlDocument xmlIniData;
	// Server side only, server doesn't use the filesystem.
	if (!xmlIniData.LoadFromFile(szFileName))
	{
		return false;
	}

	char szTagName[256], szChildName[256];

	auto rootElement = xmlIniData.GetDocumentElement();
	int Count = rootElement.GetChildNodeCount();

	WeaponSets.reserve(Count);

	for (int i = 0; i < Count; i++)
	{
		MXmlElement chrElement = rootElement.GetChildNode(i);
		chrElement.GetTagName(szTagName);
		if (szTagName[0] == '#') continue;

		if (iequals(szTagName, "OPTIONS"))
		{
			if (!ParseXML_Options(chrElement))
			{
				MLog("GunGame::ReadXML -- Error parsing options\n");
				return false;
			}
			continue;
		}

		if (!iequals(szTagName, "SET"))
		{
			MLog("GunGame::ReadXML -- Unrecognized tag %s, expected SET\n", szTagName);
			return false;
		}

		WeaponSets.emplace_back();
		auto&& WeaponSet = WeaponSets.back();

		int ChildCount = chrElement.GetChildNodeCount();
		for (int j = 0; j < ChildCount; ++j)
		{
			MXmlElement attrElement = chrElement.GetChildNode(j);
			attrElement.GetTagName(szChildName);

			if (!iequals(szChildName, "ITEMSET"))
			{
				MLog("GunGame::ReadXML -- Unrecognized tag %s, expected ITEMSET\n",
					szChildName);
				return false;
			}

			GGSet Node;
			if (!ParseXML_ItemSet(attrElement, Node))
			{
				MLog("GunGame::ReadXML -- Error parsing itemset\n");
				return false;
			}

			WeaponSet.emplace_back(Node);
		}
	}

	return true;
}

static bool ParseSlot(MXmlElement& Element, const char* AttributeName, int Index, GGSet& Output)
{
	char AttributeValue[512];
	if (!Element.GetAttribute(AttributeValue, AttributeName))
	{
		// No attribute implies empty.
		Output.WeaponIDs[Index] = 0;
		return true;
	}

	if (iequals(AttributeValue, "inventory"))
	{
		Output.SetInventory(Index, true);
		return true;
	}

	auto MaybeInt = StringToInt<int>(AttributeValue);
	if (!MaybeInt.has_value())
	{
		MLog("ParseSlot -- Invalid value of attribute %s. "
			"Expected integral value or \"inventory\", got %s.\n",
			AttributeName, AttributeValue);
		return false;
	}

	Output.WeaponIDs[Index] = MaybeInt.value();
	return true;
}

bool GunGame::ParseXML_ItemSet(MXmlElement& Element, GGSet& Node)
{
	const char* SlotNames[] = {
		"melee",
		"primary",
		"secondary",
		"custom1",
		"custom2",
	};
	for (int i = 0; i < 5; ++i)
	{
		if (!ParseSlot(Element, SlotNames[i], i, Node))
		{
			return false;
		}
	}

	return true;
}

bool GunGame::ParseXML_Options(MXmlElement& Element)
{
	auto NumAttributes = Element.GetAttributeCount();
	for (int i = 0; i < NumAttributes; ++i)
	{
		char AttributeName[512];
		char AttributeValue[512];
		Element.GetAttribute(i, AttributeName, AttributeValue);

		if (iequals(AttributeName, "RandomizeWeaponOrder"))
		{
			auto MaybeInt = StringToInt<int>(AttributeValue);
			if (!MaybeInt.has_value())
			{
				MLog("GunGame::ParseXML_Options -- Invalid RandomizeWeaponOrder value. "
					"Expected 0 or 1, got %s.\n", AttributeValue);
				return false;
			}

			auto Value = MaybeInt.value();
			if (Value < 0 || Value > 1)
			{
				MLog("GunGame::ParseXML_Options -- Invalid RandomizeWeaponOrder value. "
					"Expected 0 or 1, got %d.\n", Value);
				return false;
			}

			RandomizeWeaponOrder = Value != 0;
		}
		else if (iequals(AttributeName, "MinKillsPerLevel"))
		{
			auto MaybeInt = StringToInt<int>(AttributeValue);
			if (!MaybeInt.has_value())
			{
				MLog("GunGame::ParseXML_Options -- Invalid MinKillsPerLevel value. "
					"Expected integral value, got %s.\n", AttributeValue);
				return false;
			}

			auto Value = MaybeInt.value();
			if (Value <= 0)
			{
				MLog("GunGame::ParseXML_Options -- Invalid MinKillsPerLevel value. "
					"Expected value over zero, got %d.\n", Value);
				return false;
			}

			MinKillsPerLevel = Value;
		}
		else
		{
			MLog("GunGame::ParseXML_Options -- Unrecognized attribute %s\n", AttributeName);
			return false;
		}
	}

	return true;
}