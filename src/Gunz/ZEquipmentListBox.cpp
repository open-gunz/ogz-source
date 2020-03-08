#include "stdafx.h"

#include "ZEquipmentListBox.h"
#include "ZPost.h"
#include "ZApplication.h"
#include "ZGameClient.h"
#include "ZItemSlotView.h"
#include "ZShop.h"
#include "ZCharacterView.h"
#include "MTextArea.h"
#include "ZMyInfo.h"
#include "ZMyItemList.h"
#include "MComboBox.h"
#include "ZItemMenu.h"


// 파일 이름은 그냥 하드코딩..-_-;
MBitmap* GetItemIconBitmap(MMatchItemDesc* pItemDesc, bool bSmallIcon)
{
	if (pItemDesc == NULL) return NULL;
	char szFileName[64] = "";

	switch (pItemDesc->m_nSlot)
	{

	case MMIST_CUSTOM:
		{
			MMatchWeaponType type = pItemDesc->m_nWeaponType;

			switch (type)
			{
				case MWT_FRAGMENTATION:
					strcpy_safe(szFileName, "slot_icon_grenade"); break;
				case MWT_FLASH_BANG:
				case MWT_SMOKE_GRENADE:
					strcpy_safe(szFileName, "slot_icon_flashbang"); break;
				case MWT_MED_KIT:
					strcpy_safe(szFileName, "slot_icon_medikit"); break;
				case MWT_FOOD:
					strcpy_safe(szFileName, "slot_icon_food"); break;
				case MWT_REPAIR_KIT:
					strcpy_safe(szFileName, "slot_icon_repairkit"); break;
				case MWT_BULLET_KIT:
					strcpy_safe(szFileName, "slot_icon_magazine"); break;
				case MWT_ENCHANT_FIRE:
					strcpy_safe(szFileName, "slot_icon_en_fire"); break;
				case MWT_ENCHANT_COLD:
					strcpy_safe(szFileName, "slot_icon_en_cold"); break;
				case MWT_ENCHANT_LIGHTNING:
					strcpy_safe(szFileName, "slot_icon_en_lightning"); break;
				case MWT_ENCHANT_POISON:
					strcpy_safe(szFileName, "slot_icon_en_poison"); break;
				default: _ASSERT(0);break;
			}
		}
		break;

	case MMIST_HEAD:	strcpy_safe(szFileName, "slot_icon_head");	break;
	case MMIST_CHEST:	strcpy_safe(szFileName, "slot_icon_chest");	break;
	case MMIST_HANDS:	strcpy_safe(szFileName, "slot_icon_hands");	break;
	case MMIST_LEGS:	strcpy_safe(szFileName, "slot_icon_legs");	break;
	case MMIST_FEET:	strcpy_safe(szFileName, "slot_icon_feet");	break;
	case MMIST_FINGER:	strcpy_safe(szFileName, "slot_icon_ringS");	break;
	case MMIST_MELEE:
	case MMIST_RANGE:
		{

			MMatchWeaponType type = pItemDesc->m_nWeaponType;

			{
				switch (type) {

				case MWT_DAGGER:		strcpy_safe(szFileName, "slot_icon_dagger"); break;
				case MWT_DUAL_DAGGER:	strcpy_safe(szFileName, "slot_icon_D_dagger"); break;
				case MWT_KATANA:		strcpy_safe(szFileName, "slot_icon_katana"); break;
				case MWT_GREAT_SWORD:	strcpy_safe(szFileName, "slot_icon_sword"); break;
				case MWT_DOUBLE_KATANA:	strcpy_safe(szFileName, "slot_icon_blade"); break;

				case MWT_PISTOL:		strcpy_safe(szFileName, "slot_icon_pistol"); break;
				case MWT_PISTOLx2:		strcpy_safe(szFileName, "slot_icon_D_pistol"); break;
				case MWT_REVOLVER:		strcpy_safe(szFileName, "slot_icon_pistol"); break;
				case MWT_REVOLVERx2:	strcpy_safe(szFileName, "slot_icon_D_pistol"); break;
				case MWT_SMG:			strcpy_safe(szFileName, "slot_icon_smg"); break;
				case MWT_SMGx2:			strcpy_safe(szFileName, "slot_icon_D_smg"); break;
				case MWT_SHOTGUN:		strcpy_safe(szFileName, "slot_icon_shotgun"); break;
				case MWT_SAWED_SHOTGUN:	strcpy_safe(szFileName, "slot_icon_shotgun"); break;
				case MWT_RIFLE:			strcpy_safe(szFileName, "slot_icon_rifle"); break;
				case MWT_SNIFER:		strcpy_safe(szFileName, "slot_icon_rifle"); break;
				case MWT_MACHINEGUN:	strcpy_safe(szFileName, "slot_icon_machinegun"); break;
				case MWT_ROCKET:		strcpy_safe(szFileName, "slot_icon_rocket"); break;

				}
			}
		}
		break;
	default: _ASSERT(0);break;
	}

	if ( bSmallIcon) 
	{
		char szTemp[256];
		if ( pItemDesc->IsCashItem())
		{
			strcpy_safe( szTemp, szFileName);
			strcat_safe( szTemp, "_cash_S.tga");

			if ( MBitmapManager::Get( szTemp))
				strcat_safe( szFileName, "_cash");
		}
	}
	
	if (pItemDesc->m_nSlot == MMIST_FINGER)
	{
		if ( bSmallIcon && pItemDesc->IsCashItem())
		{
			if ( MBitmapManager::Get( "slot_icon_ring_cash_S.tga"))
				strcpy_safe(szFileName, "slot_icon_ring_cash_S");
			else
				strcpy_safe(szFileName, "slot_icon_ringS");
		}
		else
			strcpy_safe(szFileName, "slot_icon_ringS");
	}

	strcat_safe(szFileName, ".tga");

	MBitmap *pBitmap = MBitmapManager::Get(szFileName);
	_ASSERT(pBitmap!=NULL);
	return pBitmap;
}

bool ZGetIsCashItem(u32 nItemID)
{
	MMatchItemDesc* pItemDesc = MGetMatchItemDescMgr()->GetItemDesc(nItemID);
	if (pItemDesc == NULL) 
		return false;
	if (pItemDesc->IsCashItem())
		return true;
	return false;
}

bool ZEquipmentListBox::IsDropable(MWidget* pSender)
{
	if (pSender == NULL) return false;
	if (strcmp(pSender->GetClassName(), MINT_ITEMSLOTVIEW)) return false;

	return true;
}

#define SHOW_DESCRIPTION		"showdesc"
#define HIDE_DESCRIPTION		"hidedesc"


bool ZEquipmentListBox::OnEvent(MEvent* pEvent, MListener* pListener)
{
	MRECT rtClient = GetClientRect();

	if(pEvent->nMessage==MWM_MOUSEMOVE)
	{
		m_dwLastMouseMove=GetGlobalTimeMS();
		
		MPOINT pt=MEvent::LatestPos;
		pt=MScreenToClient(this,pt);

		int nItemIndex = FindItem(pt);
		if(nItemIndex != m_nLastItem) 
		{
			if (m_pDescFrame) m_pDescFrame->Show(false);
			m_nLastItem=-1;
		}
	}
	else if(pEvent->nMessage==MWM_RBUTTONDOWN) {
		if(rtClient.InPoint(pEvent->Pos)==true) {
			int nSelItem = FindItem(pEvent->Pos);
			if ( (nSelItem != -1) && GetItemMenu())
			{
				SetSelIndex(nSelItem);

				ZEquipmentListItem* pNode = (ZEquipmentListItem*)Get(nSelItem);
				if (ZGetIsCashItem(pNode->GetItemID())) {
					ZItemMenu* pMenu = GetItemMenu();
					pMenu->SetupMenu();
					pMenu->SetTargetName(pNode->GetString());
					pMenu->SetTargetUID(pNode->GetUID());

					if (m_pDescFrame && m_pDescFrame->IsVisible())
						m_pDescFrame->Show(false);

					MPOINT posItem;
					GetItemPos(&posItem, nSelItem);
					MPOINT posMenu;
					posMenu.x = GetRect().w/4;
					posMenu.y = posItem.y + GetItemHeight()/4;
					pMenu->Show(posMenu.x, posMenu.y, true);

					return true;
				}
			}
		}
	}

	return MListBox::OnEvent(pEvent, pListener);
}

ZEquipmentListBox::ZEquipmentListBox(const char* szName, MWidget* pParent, MListener* pListener)
: MListBox(szName, pParent, pListener)
{
	m_bAbsoulteTabSpacing = true;

	AddField("ICON", 32);
	AddField("아이템", 160);
	AddField("레벨", 35);
	AddField("가격", 45);

	m_bVisibleHeader = true;

	SetItemHeight(48);

	m_nLastItem=-1;
	m_dwLastMouseMove=GetGlobalTimeMS();
	m_pDescFrame=NULL;

	m_pItemMenu = NULL;
}

ZEquipmentListBox::~ZEquipmentListBox(void)
{
	if (m_pItemMenu) {
		delete m_pItemMenu;
		m_pItemMenu = NULL;
	}
}

void ZEquipmentListBox::AttachMenu(ZItemMenu* pMenu) 
{ 
	m_pItemMenu = pMenu;
	((MPopupMenu*)m_pItemMenu)->Show(false);
}

void ZEquipmentListBox::Add(const MUID& uidItem, u32 nItemID, MBitmap* pIconBitmap, const char* szName, const char* szLevel, const char* szPrice)
{
	MListBox::Add(new ZEquipmentListItem(uidItem, nItemID, pIconBitmap, szName, szLevel, szPrice));
}

void ZEquipmentListBox::Add(const MUID& uidItem, u32 nItemID, MBitmap* pIconBitmap, const char* szName, int nLevel,int nBountyPrice)
{
	char szBounty[64], szLevel[64];
	
	itoa_safe(nLevel, szLevel, 10);
	itoa_safe(nBountyPrice, szBounty, 10);

	Add(uidItem, nItemID, pIconBitmap, szName, szLevel, szBounty);
}

void ZEquipmentListBox::Add(const int nAIID, u32 nItemID, MBitmap* pIconBitmap, const char* szName, int nLevel)
{
	char szLevel[64];
	itoa_safe(nLevel, szLevel, 10);

	MListBox::Add(new ZEquipmentListItem(nAIID, nItemID, pIconBitmap, szName, szLevel));
}

void ShopPurchaseItemListBoxOnDrop(void* pSelf, MWidget* pSender, MBitmap* pBitmap, const char* szString, const char* szItemString)
{

}

void ShopSaleItemListBoxOnDrop(void* pSelf, MWidget* pSender, MBitmap* pBitmap, const char* szString, const char* szItemString)
{

}

void CharacterEquipmentItemListBoxOnDrop(void* pSelf, MWidget* pSender, MBitmap* pBitmap, const char* szString, const char* szItemString)
{
	if (pSender == NULL) return;
	if (strcmp(pSender->GetClassName(), MINT_ITEMSLOTVIEW)) return;

	ZItemSlotView* pItemSlotView = (ZItemSlotView*)pSender;

	ZPostRequestTakeoffItem(ZGetGameClient()->GetPlayerUID(), pItemSlotView->GetParts());
	ZPostRequestCharacterItemList(ZGetGameClient()->GetPlayerUID());
}

class MShopSaleItemListBoxListener : public MListener{
public:
	virtual bool OnCommand(MWidget* pWidget, const char* szMessage)
	{
		if(MWidget::IsMsg(szMessage, MLB_ITEM_SEL)==true)
		{
			ZEquipmentListBox* pEquipmentListBox = (ZEquipmentListBox*)pWidget;
			u32 nItemID = 0;
			ZEquipmentListItem* pListItem;
			if (pEquipmentListBox->IsSelected())
			{
				pListItem = (ZEquipmentListItem*)pEquipmentListBox->GetSelItem();
				if (pListItem != NULL) 
				{
					nItemID = ZGetMyInfo()->GetItemList()->GetItemID(pListItem->GetUID());
				}
			}

			MMatchItemDesc* pItemDesc = MGetMatchItemDescMgr()->GetItemDesc(nItemID);
			ZMyItemNode* pItemNode = ZGetMyInfo()->GetItemList()->GetItem( pEquipmentListBox->GetSelIndex());
			if ( pItemDesc && pItemNode)
			{
				ZGetGameInterface()->SetupItemDescription( pItemDesc,
															"Shop_ItemDescription1",
															"Shop_ItemDescription2",
															"Shop_ItemDescription3",
															"Shop_ItemIcon",
															pItemNode);
				MButton* pButton = (MButton*)ZGetGameInterface()->GetIDLResource()->FindWidget( "SellConfirmCaller");
				if ( pButton)
				{
					if ( pItemDesc->IsCashItem())
						pButton->Enable( false);
					else
						pButton->Enable( true);

					pButton->Show( true);
				}
			}
			else
			{
				MButton* pButton = (MButton*)ZGetGameInterface()->GetIDLResource()->FindWidget( "SellConfirmCaller");
				if ( pButton)
					pButton->Show( false);
			}

			// 퀘스트 아이템
			MQuestItemDesc* pQuestItemDesc = GetQuestItemDescMgr().FindQItemDesc( pListItem->GetItemID());
			if ( pQuestItemDesc)
			{
				ZGetGameInterface()->SetupItemDescription( pQuestItemDesc,
															"Shop_ItemDescription1",
															"Shop_ItemDescription2",
															"Shop_ItemDescription3",
															"Shop_ItemIcon");

				ZGetGameInterface()->SetKindableItem( MMIST_NONE);

				MButton* pButton = (MButton*)ZGetGameInterface()->GetIDLResource()->FindWidget( "SellQuestItemConfirmCaller");
				if ( pButton)
				{
					pButton->Enable( true);
					pButton->Show( true);
				}
			}
			else
			{
				MButton* pButton = (MButton*)ZGetGameInterface()->GetIDLResource()->FindWidget( "SellQuestItemConfirmCaller");
				if ( pButton)
					pButton->Show( false);
			}

			return true;
		}
		else if ( MWidget::IsMsg(szMessage, MLB_ITEM_DBLCLK)==true)
		{
			MWidget* pWidget = (MWidget*)ZGetGameInterface()->GetIDLResource()->FindWidget( "Shop_SellConfirm");
//			if ( pWidget)
//				pWidget->Show( true);

			return true;
		}

		return false;
	}
};

MShopSaleItemListBoxListener g_ShopSaleItemListBoxListener;

MListener* ZGetShopSaleItemListBoxListener(void)
{
	return &g_ShopSaleItemListBoxListener;
}



// frame 을 툴팁처럼 보이게 하기 위해 하드코딩 되어있는데, 반복을 줄일수 있겠다.
class MCashShopItemListBoxListener : public MListener
{
public:
	virtual bool OnCommand( MWidget* pWidget, const char* szMessage)
	{
		if ( MWidget::IsMsg( szMessage, MLB_ITEM_SEL)==true)
		{
			u32 nItemID = 0;

			ZEquipmentListBox* pEquipmentListBox = (ZEquipmentListBox*)pWidget;
			ZEquipmentListItem* pListItem = ( ZEquipmentListItem*)pEquipmentListBox->GetSelItem();
			if ( pListItem)
			{
				MMatchItemDesc* pItemDesc = MGetMatchItemDescMgr()->GetItemDesc( pListItem->GetItemID());
				ZCharacterView* pCharacterView = (ZCharacterView*)ZGetGameInterface()->GetIDLResource()->FindWidget( "EquipmentInformationShop");
				if ( pItemDesc && pCharacterView)
				{
					MMatchCharItemParts nCharItemParts = GetSuitableItemParts( pItemDesc->m_nSlot);

					pCharacterView->SetSelectMyCharacter();
					pCharacterView->SetParts(nCharItemParts, pItemDesc->m_nID);

					if (IsWeaponCharItemParts( nCharItemParts))
						pCharacterView->ChangeVisualWeaponParts( nCharItemParts);
	
					ZGetGameInterface()->SetupItemDescription( pItemDesc,
																"Shop_ItemDescription1",
																"Shop_ItemDescription2",
																"Shop_ItemDescription3",
																"Shop_ItemIcon",
																NULL);
				}


				MButton* pButton = (MButton*)ZGetGameInterface()->GetIDLResource()->FindWidget( "BuyCashConfirmCaller");
				if ( pButton)
					pButton->Enable( true);


				return true;
			}
		}

		else if ( MWidget::IsMsg( szMessage, MLB_ITEM_DBLCLK) == true)
		{
			return true;
		}


		return false;
	}
};

MCashShopItemListBoxListener g_CashShopItemListBoxListener;

MListener* ZGetCashShopItemListBoxListener(void)
{
	return &g_CashShopItemListBoxListener;
}

/////////////////////////////////////////////////////////////////

class MShopAllEquipmentFilterListener : public MListener {
public:
	virtual bool OnCommand(MWidget* pWidget, const char* szMessage)
	{
		if(MWidget::IsMsg(szMessage, MCMBBOX_CHANGED)==true)
		{
			ZIDLResource* pResource = ZGetGameInterface()->GetIDLResource();

			MComboBox* pComboBox = (MComboBox*)pResource->FindWidget("Shop_AllEquipmentFilter");
			if ( pComboBox)
			{
				int sel = pComboBox->GetSelIndex();

				// 사기상태라면
				ZGetShop()->m_ListFilter = sel;
				ZGetShop()->Serialize();

				// 팔기상태라면 - 팔기는 다 보여준다..
				ZMyItemList* pil = ZGetMyInfo()->GetItemList();
				pil->m_ListFilter = sel;
				pil->Serialize();
			}

			ZGetGameInterface()->SelectShopTab( ZGetGameInterface()->m_nShopTabNum);
		}
		return true;
	}
};

MShopAllEquipmentFilterListener g_ShopAllEquipmentFilterListener;

MListener* ZGetShopAllEquipmentFilterListener()
{
	return &g_ShopAllEquipmentFilterListener;
}

/////////////////////////////////////////////////////////////////

class MEquipAllEquipmentFilterListener : public MListener {
public:
	virtual bool OnCommand(MWidget* pWidget, const char* szMessage)
	{
		if(MWidget::IsMsg(szMessage, MCMBBOX_CHANGED)==true) {

			ZIDLResource* pResource = ZGetGameInterface()->GetIDLResource();

			MComboBox* pComboBox = (MComboBox*)pResource->FindWidget("Equip_AllEquipmentFilter");
			if ( pComboBox)
			{
				int sel = pComboBox->GetSelIndex();

				ZMyItemList* pil = ZGetMyInfo()->GetItemList();
				pil->m_ListFilter = sel;
				pil->Serialize();
			}

			ZGetGameInterface()->SelectEquipmentTab( ZGetGameInterface()->m_nEquipTabNum);
		}
		return true;
	}
};

MEquipAllEquipmentFilterListener g_EquipAllEquipmentFilterListener;

MListener* ZGetEquipAllEquipmentFilterListener()
{
	return &g_EquipAllEquipmentFilterListener;
}

/////////////////////////////////////////////////////////////////

class MShopPurchaseItemListBoxListener : public MListener{
public:
	virtual bool OnCommand(MWidget* pWidget, const char* szMessage)
	{
		if(MWidget::IsMsg(szMessage, MLB_ITEM_SEL)==true) {
			u32 nItemID = 0;

			ZEquipmentListBox* pEquipmentListBox = (ZEquipmentListBox*)pWidget;
			ZEquipmentListItem* pListItem = (ZEquipmentListItem*)pEquipmentListBox->GetSelItem();
			if (pListItem != NULL) 
			{
				nItemID = ZGetShop()->GetItemID(pListItem->GetUID().Low - 1);
			}

			MMatchItemDesc* pItemDesc = MGetMatchItemDescMgr()->GetItemDesc(nItemID);

#ifdef _QUEST_ITEM
			// 만약 일반아이템이 없으면 퀘스트 아이템에서 검색을 함.
			if( 0 == pItemDesc )
			{
				// 퀘스트 아이템일 경우를 처리해 주고 함수를 종료함.
				MQuestItemDesc* pQuestItemDesc = GetQuestItemDescMgr().FindQItemDesc( nItemID );
				if( 0 == pQuestItemDesc )
					return false;

				ZGetGameInterface()->SetupItemDescription( pQuestItemDesc,
															"Shop_ItemDescription1",
															"Shop_ItemDescription2",
															"Shop_ItemDescription3",
															"Shop_ItemIcon");

				MButton* pButton1 = (MButton*)ZGetGameInterface()->GetIDLResource()->FindWidget( "BuyConfirmCaller");
				MButton* pButton2 = (MButton*)ZGetGameInterface()->GetIDLResource()->FindWidget( "BuyCashConfirmCaller");
				if( 0 != pQuestItemDesc )
				{
					if ( pButton1)
					{
						pButton1->Enable( true);
						pButton1->Show( true);
					}
					if ( pButton2)
						pButton2->Show( false);
				}	

				return true;
			}
#endif

			ZCharacterView* pCharacterView = (ZCharacterView*)ZGetGameInterface()->GetIDLResource()->FindWidget("EquipmentInformationShop");

			if ((pCharacterView) && (pItemDesc)) {
				MMatchCharItemParts nCharItemParts = GetSuitableItemParts(pItemDesc->m_nSlot);

				pCharacterView->SetSelectMyCharacter();
				pCharacterView->SetParts(nCharItemParts, pItemDesc->m_nID);

				if (IsWeaponCharItemParts(nCharItemParts))
				{
					pCharacterView->ChangeVisualWeaponParts(nCharItemParts);
				}

				ZGetGameInterface()->SetupItemDescription( pItemDesc,
															"Shop_ItemDescription1",
															"Shop_ItemDescription2",
															"Shop_ItemDescription3",
															"Shop_ItemIcon",
															NULL);
			}

			MButton* pButton1 = (MButton*)ZGetGameInterface()->GetIDLResource()->FindWidget( "BuyConfirmCaller");
			MButton* pButton2 = (MButton*)ZGetGameInterface()->GetIDLResource()->FindWidget( "BuyCashConfirmCaller");
			if ( pItemDesc->IsCashItem())
			{
				if ( pButton1)
					pButton1->Show( false);
				if ( pButton2)
				{
					pButton2->Enable( true);
					pButton2->Show( true);
				}
			}
			else
			{
				if ( pButton1)
				{
					pButton1->Enable( true);
					pButton1->Show( true);
				}
				if ( pButton2)
					pButton2->Show( false);
			}

			return true;
		}
		else if ( MWidget::IsMsg(szMessage, MLB_ITEM_DBLCLK)==true)
		{
			MWidget* pWidget = (MWidget*)ZGetGameInterface()->GetIDLResource()->FindWidget( "Shop_BuyConfirm");
//			if ( pWidget)
//				pWidget->Show( true);

			return true;
		}

		return false;
	}
};
MShopPurchaseItemListBoxListener g_ShopPurchaseItemListBoxListener;

MListener* ZGetShopPurchaseItemListBoxListener(void)
{
	return &g_ShopPurchaseItemListBoxListener;
}



////////
class MEquipmentItemListBoxListener : public MListener{
public:
	virtual bool OnCommand(MWidget* pWidget, const char* szMessage)
	{
		if ( MWidget::IsMsg(szMessage, MLB_ITEM_SEL)==true) {
			u32 nItemID = 0;

			ZEquipmentListBox* pEquipmentListBox = (ZEquipmentListBox*)pWidget;
			ZEquipmentListItem* pListItem = (ZEquipmentListItem*)pEquipmentListBox->GetSelItem();
			if (pListItem != NULL) 
				nItemID = ZGetMyInfo()->GetItemList()->GetItemID(pListItem->GetUID());

			// 일반 아이템...
			MButton* pButtonEquip = (MButton*)ZGetGameInterface()->GetIDLResource()->FindWidget( "Equip");
			MButton* pButtonAccItemBtn = (MButton*)ZGetGameInterface()->GetIDLResource()->FindWidget( "SendAccountItemBtn");

			MMatchItemDesc* pItemDesc = MGetMatchItemDescMgr()->GetItemDesc(nItemID);
			ZCharacterView* pCharacterView = (ZCharacterView*)ZGetGameInterface()->GetIDLResource()->FindWidget("EquipmentInformation");
			if ((pCharacterView) && (pItemDesc))
			{
				MMatchCharItemParts nCharItemParts = GetSuitableItemParts(pItemDesc->m_nSlot);

				pCharacterView->SetSelectMyCharacter();
				pCharacterView->SetParts(nCharItemParts, pItemDesc->m_nID);

				if (IsWeaponCharItemParts(nCharItemParts))
					pCharacterView->ChangeVisualWeaponParts(nCharItemParts);

				ZMyItemNode* pItemNode = ZGetMyInfo()->GetItemList()->GetItemEquip( pEquipmentListBox->GetSelIndex());
				if ( pItemNode)
					ZGetGameInterface()->SetupItemDescription( pItemDesc,
																"Equip_ItemDescription1",
																"Equip_ItemDescription2",
																"Equip_ItemDescription3",
																"Equip_ItemIcon",
																pItemNode);

				ZGetGameInterface()->SetKindableItem( pItemDesc->m_nSlot);

				if ( pButtonEquip)
					pButtonEquip->Enable( true);

				// 캐쉬 아이템일 경우 '중앙은행에 보내기'버튼 활성화, 아님 비활성화
				if ( pButtonAccItemBtn)
				{
					if ( ZGetIsCashItem( nItemID))
						pButtonAccItemBtn->Enable( true);
					else
						pButtonAccItemBtn->Enable( false);
				}
			}

			// 퀘스트 아이템
			MQuestItemDesc* pQuestItemDesc = GetQuestItemDescMgr().FindQItemDesc( pListItem->GetItemID());
			if ( pQuestItemDesc)
			{
				ZGetGameInterface()->SetupItemDescription( pQuestItemDesc,
															"Equip_ItemDescription1",
															"Equip_ItemDescription2",
															"Equip_ItemDescription3",
															"Equip_ItemIcon");

				ZGetGameInterface()->SetKindableItem( MMIST_NONE);

				if ( pButtonEquip)
					pButtonEquip->Enable( false);

				if ( pButtonAccItemBtn)
					pButtonAccItemBtn->Enable( false);
			}

			return true;
		}
		else if ( MWidget::IsMsg(szMessage, MLB_ITEM_DBLCLK)==true)
		{
			ZGetGameInterface()->Equip();
		
			return true;
		}

		return false;
	}
};
MEquipmentItemListBoxListener g_EquipmentItemListBoxListener;

MListener* ZGetEquipmentItemListBoxListener(void)
{
	return &g_EquipmentItemListBoxListener;
}

class MAccountItemListBoxListener : public MListener{
public:
	virtual bool OnCommand(MWidget* pWidget, const char* szMessage)
	{
		if ( MWidget::IsMsg(szMessage, MLB_ITEM_SEL)==true) {
			u32 nItemID = 0;

			ZEquipmentListBox* pEquipmentListBox = (ZEquipmentListBox*)pWidget;
			ZEquipmentListItem* pListItem = (ZEquipmentListItem*)pEquipmentListBox->GetSelItem();
			if (pListItem != NULL) 
				nItemID = ZGetMyInfo()->GetItemList()->GetAccountItemID( pEquipmentListBox->GetSelIndex());

			MMatchItemDesc* pItemDesc = MGetMatchItemDescMgr()->GetItemDesc( nItemID);
			ZCharacterView* pCharacterView = (ZCharacterView*)ZGetGameInterface()->GetIDLResource()->FindWidget("EquipmentInformation");

			if ((pCharacterView) && (pItemDesc))
			{
				MMatchCharItemParts nCharItemParts = GetSuitableItemParts(pItemDesc->m_nSlot);

				pCharacterView->SetSelectMyCharacter();
				pCharacterView->SetParts(nCharItemParts, pItemDesc->m_nID);

				if (IsWeaponCharItemParts(nCharItemParts))
					pCharacterView->ChangeVisualWeaponParts(nCharItemParts);

				ZMyItemNode* pAccountItemNode = ZGetMyInfo()->GetItemList()->GetAccountItem( pEquipmentListBox->GetSelIndex());
				if ( pAccountItemNode)
					ZGetGameInterface()->SetupItemDescription( pItemDesc,
																"Equip_ItemDescription1",
																"Equip_ItemDescription2",
																"Equip_ItemDescription3",
																"Equip_ItemIcon",
																pAccountItemNode);
			}

			// 성별이 맞지 않으면 버튼을 Disable 시킨다.
			MButton* pButton = (MButton*)ZGetGameInterface()->GetIDLResource()->FindWidget( "BringAccountItemBtn");
			if ( pButton)
			{
				if ( (pItemDesc->m_nResSex == -1) || (pItemDesc->m_nResSex == ZGetMyInfo()->GetSex()) )
					pButton->Enable( true);
				else
					pButton->Enable( false);
			}

			return true;
		}
		else if ( MWidget::IsMsg(szMessage, MLB_ITEM_DBLCLK)==true)
		{
			ZGetGameInterface()->GetBringAccountItem();

			return true;
		}

		return false;
	}
};

MAccountItemListBoxListener g_AccountItemListBoxListener;

MListener* ZGetAccountItemListBoxListener(void)
{
	return &g_AccountItemListBoxListener;
}
