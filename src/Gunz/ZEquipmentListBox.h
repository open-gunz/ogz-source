#pragma once

#include "ZPrerequisites.h"
#include "MListBox.h"
#include "ZStringResManager.h"
#include "SafeString.h"

MBitmap* GetItemIconBitmap(MMatchItemDesc* pItemDesc, bool bSmallIcon = false);
bool ZGetIsCashItem(u32 nItemID);

class ZEquipmentListItem : public MListItem{
protected:
	MBitmap*			m_pBitmap;
	int					m_nAIID;
	u32		m_nItemID;
public:
	MUID				m_UID;
public:
	char	m_szName[256];
	char	m_szLevel[256];
	char	m_szPrice[256];
public:

	ZEquipmentListItem(const MUID& uidItem, const u32 nItemID, MBitmap* pBitmap, const char* szName, const char* szLevel, const char* szPrice)
	{
		m_nAIID = 0;
		m_nItemID = nItemID;
		m_pBitmap = pBitmap;
		m_UID = uidItem;
		strcpy_safe(m_szName, szName);
		strcpy_safe(m_szLevel, szLevel);
		strcpy_safe(m_szPrice, szPrice);
	}
	ZEquipmentListItem(const int nAIID, const u32 nItemID, MBitmap* pBitmap, const char* szName, const char* szLevel)
	{
		m_nAIID = nAIID;
		m_nItemID = nItemID;
		m_pBitmap = pBitmap;
		m_UID = MUID(0,0);
		strcpy_safe(m_szName, szName);
		strcpy_safe(m_szLevel, szLevel);
		m_szPrice[0] = 0;
	}

	ZEquipmentListItem(void)
	{
		m_nAIID = 0;
		m_nItemID = 0;
		m_pBitmap = NULL;
		m_UID = MUID(0,0);
		m_szName[0] = 0;
		m_szLevel[0] = 0;
		m_szPrice[0] = 0;
	}
	virtual const char* GetString(void)
	{
		return m_szName;
	}
	virtual const char* GetString(int i)
	{
		if(i==1) return m_szName;
		else if(i==2) return m_szLevel;
		else if(i==3) {
			if ( ZGetIsCashItem(GetItemID()) == true)
			{
				return ZMsg( MSG_WORD_CASH);
			}
			else
				return m_szPrice;
		}
		return NULL;
	}
	virtual bool GetDragItem(MBitmap** ppDragBitmap, char* szDragString, char* szDragItemString) override
	{
		*ppDragBitmap = GetBitmap(0);
		strcpy_unsafe(szDragString, m_szName);
		strcpy_unsafe(szDragItemString, m_szName);
		return true;
	}
	virtual MBitmap* GetBitmap(int i)
	{
		if (i == 0) return m_pBitmap;
		return NULL;
	}
	MUID& GetUID() { return m_UID; }
	int GetAIID() { return m_nAIID; }
	u32 GetItemID() { return m_nItemID; }
};


class ZItemMenu;

class ZEquipmentListBox : public MListBox
{
protected:
	virtual bool IsDropable(MWidget* pSender);
	virtual bool OnEvent(MEvent* pEvent, MListener* pListener);

protected:
	MWidget*			m_pDescFrame;
protected:
	ZItemMenu*			m_pItemMenu;
	ZItemMenu* GetItemMenu()	{ return m_pItemMenu; }

public:
	ZEquipmentListBox(const char* szName, MWidget* pParent=NULL, MListener* pListener=NULL);
	virtual ~ZEquipmentListBox(void);
	void AttachMenu(ZItemMenu* pMenu);

	void Add(const MUID& uidItem, u32 nItemID, MBitmap* pIconBitmap, const char* szName, const char* szLevel, const char* szPrice);
	void Add(const MUID& uidItem, u32 nItemID, MBitmap* pIconBitmap, const char* szName, int nLevel,int nBountyPrice);
	void Add(const int nAIID, u32 nItemID, MBitmap* pIconBitmap, const char* szName, int nLevel);

	void SetDescriptionWidget(MWidget *pWidget)	{ m_pDescFrame = pWidget; }

public:
	#define MINT_EQUIPMENTLISTBOX	"EquipmentListBox"
	virtual const char* GetClassName(void){ return MINT_EQUIPMENTLISTBOX; }

	u32	m_dwLastMouseMove;
	int		m_nLastItem;
};

void ShopPurchaseItemListBoxOnDrop(void* pSelf, MWidget* pSender, MBitmap* pBitmap, const char* szString, const char* szItemString);
void ShopSaleItemListBoxOnDrop(void* pSelf, MWidget* pSender, MBitmap* pBitmap, const char* szString, const char* szItemString);
void CharacterEquipmentItemListBoxOnDrop(void* pSelf, MWidget* pSender, MBitmap* pBitmap, const char* szString, const char* szItemString);

MListener* ZGetShopAllEquipmentFilterListener(void);
MListener* ZGetEquipAllEquipmentFilterListener(void);

MListener* ZGetShopSaleItemListBoxListener(void);
MListener* ZGetCashShopItemListBoxListener(void);
MListener* ZGetShopPurchaseItemListBoxListener(void);
MListener* ZGetEquipmentItemListBoxListener(void);
MListener* ZGetAccountItemListBoxListener(void);