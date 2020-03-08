#ifndef _MMATCHSHOP_H
#define _MMATCHSHOP_H

#include "MXml.h"
#include "MUID.h"
#include <map>
#include <vector>
#include <algorithm>
#include "MMatchItem.h"
#include "MQuestItem.h"

struct ShopItemNode
{
	unsigned int	nItemID;
	bool			bIsRentItem;
	int				nRentPeriodHour;
	ShopItemNode() : nItemID(0), bIsRentItem(false), nRentPeriodHour(0) {}
};

class MMatchShop
{
private:
protected:
	std::vector<ShopItemNode*>					m_ItemNodeVector;
	std::map<unsigned int, ShopItemNode*>		m_ItemNodeMap;

	void ParseSellItem(MXmlElement& element);
	bool ReadXml(const char* szFileName);
public:
	MMatchShop();
	virtual ~MMatchShop();
	bool Create(const char* szDescFileName);
	void Destroy();

	void Clear();
	int GetCount() { return static_cast< int >( m_ItemNodeVector.size() ); }
	bool IsSellItem(const u32 nItemID);
	ShopItemNode* GetSellItem(int nListIndex);

	static MMatchShop* GetInstance();
};

inline MMatchShop* MGetMatchShop() { return MMatchShop::GetInstance(); }

#define MTOK_SELL					"SELL"
#define MTOK_SELL_ITEMID			"itemid"
#define MTOK_SELL_RENT_PERIOD_HOUR	"rent_period"




#endif