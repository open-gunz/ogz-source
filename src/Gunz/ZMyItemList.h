#pragma once

#include "ZPrerequisites.h"
#include "MMatchItem.h"
#include "MBaseItem.h"
#include "MQuestItem.h"
#include <list>
#include <map>

class ZMyItemNode : public MBaseItem
{
protected:
	u32		m_nItemID;
	MUID	m_UID;
	u64		m_dwWhenReceivedClock;

public:
	ZMyItemNode() : MBaseItem(), m_nItemID(0), m_UID(MUID(0,0)) { }

	virtual	~ZMyItemNode() { }
	void Create(MUID& uidItem, u32 nItemID, bool bIsRentItem = false,
		int nRentMinutePeriodRemainder = RENT_MINUTE_PERIOD_UNLIMITED)
	{
		m_UID = uidItem;
		m_nItemID = nItemID;
		m_bIsRentItem = bIsRentItem;
		m_nRentMinutePeriodRemainder = nRentMinutePeriodRemainder;
		m_dwWhenReceivedClock = GetGlobalTimeMS();
	}
	void Create(u32 nItemID, bool bIsRentItem = false,
		int nRentMinutePeriodRemainder = RENT_MINUTE_PERIOD_UNLIMITED)
	{
		m_nItemID = nItemID;
		m_bIsRentItem = bIsRentItem;
		m_nRentMinutePeriodRemainder = nRentMinutePeriodRemainder;
		m_dwWhenReceivedClock = GetGlobalTimeMS();
	}
	auto GetWhenReceivedClock() const { return m_dwWhenReceivedClock; }

	auto GetItemID() const { return m_nItemID; }
	MUID& GetUID()					{ return m_UID; }

};

typedef std::map<MUID, ZMyItemNode*> MITEMNODEMAP;
typedef std::map<int, ZMyItemNode*> MACCOUNT_ITEMNODEMAP;

#define MAX_ZQUEST_ITEM_COUNT 99
#define MIN_ZQUEST_ITEM_COUNT 0

class ZMyQuestItemNode
{
public:
	ZMyQuestItemNode() : m_nCount( 0 ), m_nItemID( 0 )
	{
	}

	virtual ~ZMyQuestItemNode()
	{
	}

	u32	GetItemID()	{ return m_nItemID; }
	int					GetCount()	{ return m_nCount; }
	MQuestItemDesc*		GetDesc()	{ return m_pDesc; }

	void Increase( const int nCount = 1 ) 
	{
		m_nCount += nCount;
		if( m_nCount >= MAX_ZQUEST_ITEM_COUNT )
			m_nCount = MAX_ZQUEST_ITEM_COUNT;

	}

	void Decrease( const int nCount = 1 )
	{
		m_nCount -= nCount;
		if( MIN_ZQUEST_ITEM_COUNT > m_nCount )
			m_nCount = MIN_ZQUEST_ITEM_COUNT;
	}

	void SetItemID( const u32 nItemID )	{ m_nItemID = nItemID; }
	void SetCount( const int nCount )					{ m_nCount = nCount; }
	void SetDesc( MQuestItemDesc* pDesc )				{ m_pDesc = pDesc; }

	void Create( const u32 nItemID, const int nCount, MQuestItemDesc* pDesc )
	{
		m_nItemID	= nItemID;
		m_nCount	= nCount;
		m_pDesc		= pDesc;
	}

public :
	u32	m_nItemID;
	int					m_nCount;
	MQuestItemDesc*		m_pDesc;
};


class ZMyQuestItemMap : public std::map< u32, ZMyQuestItemNode* >
{
public :
	ZMyQuestItemMap();
	~ZMyQuestItemMap();
	
	bool Add( const u32 nItemID, ZMyQuestItemNode* pQuestItem );

	bool CreateQuestItem( const u32 nItemID, const int nCount, MQuestItemDesc* pDesc );

	void Clear();

	ZMyQuestItemNode* Find( const u32 nItemID )
	{
		ZMyQuestItemMap::iterator It;
		It = find( nItemID );
		if( end() != It )
		{
			return It->second;
		}

		return 0;
	}

private :

};

typedef std::map<u32, ZMyQuestItemNode*> MQUESTITEMNODEMAP;

enum ITEM_EQUIP_PARTS
{
	ITEM_EQUIP_PARTS_ALL = 0,
	ITEM_EQUIP_PARTS_HEAD,
	ITEM_EQUIP_PARTS_CHEST,
	ITEM_EQUIP_PARTS_HANDS,
	ITEM_EQUIP_PARTS_LEGS,
	ITEM_EQUIP_PARTS_FEET,
	ITEM_EQUIP_PARTS_MELEE,
	ITEM_EQUIP_PARTS_PRIMARY,
	ITEM_EQUIP_PARTS_SECONDARY,
	ITEM_EQUIP_PARTS_ITEM,

	ITEM_EQUIP_PARTS_END
};

class ZMyItemList
{
private:
protected:
	bool							m_bCreated;
	MUID							m_uidEquipItems[MMCIP_END];
	u32				m_nEquipItemID[MMCIP_END];
	MITEMNODEMAP					m_ItemMap;
	std::vector<MUID>					m_ItemIndexVectorEquip;
	std::vector<MUID>					m_ItemIndexVector;

	MACCOUNT_ITEMNODEMAP			m_AccountItemMap;
	std::vector<int>						m_AccountItemVector;

	void ClearItemMap();
	void ClearAccountItemMap();


#ifdef _QUEST_ITEM
protected :
	ZMyQuestItemMap	m_QuestItemMap;

public :
	ZMyQuestItemMap&	GetQuestItemMap()	{ return m_QuestItemMap; }
	int					GetQuestItemCount()	{ return static_cast< int >( m_QuestItemMap.size() ); }

	void SetQuestItemsAll( MTD_QuestItemNode* pQuestItemNode, const int nQuestItemCount );
#endif


public:

	int		m_ListFilter;

public:
	ZMyItemList();
	virtual ~ZMyItemList();

	bool Create();
	void Destroy();
	void Clear();

	bool CheckAddType(int type);

	int GetItemCount() { return (int)m_ItemMap.size(); }

	void GetNamedComp( int nItemID, char* szBmpWidgetName, char* szBmpName, char* szLabelWidgetName);

	u32 GetItemID(int nItemIndex);
	u32 GetItemIDEquip(int nItemIndex);
	u32 GetItemID(const MUID& uidItem);
	u32 GetAccountItemID(int nPos);
	u32 GetEquipedItemID(MMatchCharItemParts parts);

	ZMyItemNode* GetItem(int nItemIndex);
	ZMyItemNode* GetItemEquip(int nItemIndex);
	ZMyItemNode* GetItem(const MUID& uidItem);
	ZMyItemNode* GetEquipedItem(MMatchCharItemParts parts);
	ZMyItemNode* GetAccountItem(int nPos);


	MUID GetEquipedItemUID(MMatchCharItemParts parts);
	void SetEquipItemsAll(MUID* pnEquipItems);
	void SetEquipItemID(u32* pEquipItemID);
	void SetItemsAll(MTD_ItemNode* pItemNodes, const int nItemCount);
	bool IsCreated() { return m_bCreated; }

	MUID GetItemUID(int nItemIndex);
	MUID GetItemUIDEquip(int nItemIndex);

	void Serialize();
	void SerializeAccountItem();
	int GetEquipedTotalWeight();
	int GetEquipedHPModifier();
	int GetEquipedAPModifier();
	int GetMaxWeight();

	void AddAccountItem(int nAIID, u32 nItemID, int nRentMinutePeriodRemainder=RENT_MINUTE_PERIOD_UNLIMITED);
	void ClearAccountItems();
};