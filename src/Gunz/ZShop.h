#pragma once

#include "GlobalTypes.h"
#include "ZPrerequisites.h"
#include <vector>

enum {
	zshop_item_filter_all = 0,
	zshop_item_filter_head,
	zshop_item_filter_chest,
	zshop_item_filter_hands,
	zshop_item_filter_legs,
	zshop_item_filter_feet,
	zshop_item_filter_melee,
	zshop_item_filter_range,
	zshop_item_filter_custom,
	zshop_item_filter_extra,
	zshop_item_filter_quest,
};

class ZShop
{
private:
protected:
	int m_nPage;
	bool m_bCreated;
	std::vector<u32> m_ItemVector;

public:
	int m_ListFilter;

public:
	ZShop();
	virtual ~ZShop();
	bool Create();
	void Destroy();
	void Clear();
	void Serialize();

	bool CheckAddType(int type);

	int GetItemCount() { return (int)m_ItemVector.size(); }
	void SetItemsAll(u32* nItemList, int nItemCount);
	int GetPage() { return m_nPage; }
	u32 GetItemID(int nIndex);
	static ZShop* GetInstance();
};

inline ZShop* ZGetShop() { return ZShop::GetInstance(); }