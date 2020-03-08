#include "stdafx.h"

#include "ZMyItemList.h"
#include "ZGameInterface.h"
#include "MListBox.h"
#include "MLabel.h"
#include "ZApplication.h"
#include "ZEquipmentListBox.h"
#include "MMatchTransDataType.h"
#include "ZMyInfo.h"
#include "ZCharacterView.h"
#include "ZShop.h"

ZMyItemList::ZMyItemList() : m_bCreated(false)
{
	m_ListFilter = zshop_item_filter_all;
}

ZMyItemList::~ZMyItemList()
{

}

bool ZMyItemList::Create()
{
	if (m_bCreated) return false;

	Clear();

	return true;
}
void ZMyItemList::Destroy()
{
	if (!m_bCreated) return;

}
void ZMyItemList::Clear()
{
	for (int i = 0; i < MMCIP_END; i++) 
		m_uidEquipItems[i] = MUID(0,0);

	memset(m_nEquipItemID, 0, sizeof(m_nEquipItemID));

	ClearItemMap();
	m_ItemIndexVector.clear();
	m_ItemIndexVectorEquip.clear();

	ClearAccountItems();
}


u32 ZMyItemList::GetEquipedItemID(MMatchCharItemParts parts)
{
	if (parts < MMCIP_HEAD || parts >= MMCIP_END)
		return 0;

	MITEMNODEMAP::iterator itor = m_ItemMap.find(m_uidEquipItems[(int)parts]);
	if (itor != m_ItemMap.end())
	{
		ZMyItemNode* pItemNode = (*itor).second;
		return pItemNode->GetItemID();
	}
	
	return m_nEquipItemID[parts];
}

MUID ZMyItemList::GetEquipedItemUID(MMatchCharItemParts parts)
{
	MUID uid = m_uidEquipItems[parts];

	MITEMNODEMAP::iterator itor = m_ItemMap.find(uid);
	if (itor != m_ItemMap.end())
	{
		return uid;
	}
	else
	{
		return MUID(0,0);
	}
}

u32 ZMyItemList::GetItemID(int nItemIndex)
{
	ZMyItemNode* pItemNode = GetItem(nItemIndex);
	if (pItemNode == NULL) return 0;
	return pItemNode->GetItemID();

}

u32 ZMyItemList::GetItemIDEquip(int nItemIndex)
{
	ZMyItemNode* pItemNode = GetItemEquip(nItemIndex);
	if (pItemNode == NULL) return 0;
	return pItemNode->GetItemID();

}

bool ZMyItemList::CheckAddType(int type)
{
		 if(m_ListFilter == zshop_item_filter_all)		return true;
	else if(m_ListFilter == zshop_item_filter_head)		{ if(type == MMIST_HEAD) return true; }
	else if(m_ListFilter == zshop_item_filter_chest)	{ if(type == MMIST_CHEST) return true; }
	else if(m_ListFilter == zshop_item_filter_hands)	{ if(type == MMIST_HANDS) return true; }
	else if(m_ListFilter == zshop_item_filter_legs)		{ if(type == MMIST_LEGS) return true; }
	else if(m_ListFilter == zshop_item_filter_feet)		{ if(type == MMIST_FEET) return true; }
	else if(m_ListFilter == zshop_item_filter_melee)	{ if(type == MMIST_MELEE) return true; }
	else if(m_ListFilter == zshop_item_filter_range)	{ if(type == MMIST_RANGE) return true; }
	else if(m_ListFilter == zshop_item_filter_custom)	{ if(type == MMIST_CUSTOM) return true; }
	else if(m_ListFilter == zshop_item_filter_extra)
	{
		if(type == MMIST_EXTRA) return true;
		else if(type == MMIST_FINGER) return true;
	}

	return false;
}

void ZMyItemList::Serialize()
{
	// m_ItemIndexVector를 초기화
	m_ItemIndexVector.clear();
	m_ItemIndexVectorEquip.clear();

	MMatchItemDesc* pItemDesc = NULL;
	ZIDLResource* pResource = ZApplication::GetGameInterface()->GetIDLResource();

	// 장비하고 있는 아이템은 제외한다
	for (MITEMNODEMAP::iterator itor = m_ItemMap.begin(); itor != m_ItemMap.end(); ++itor)
	{
		bool bExist = false;

		for (int i = 0; i < MMCIP_END; i++)
		{
			if (m_uidEquipItems[i] == (*itor).first) 
				bExist = true;
		}

		if (bExist == false) {

			m_ItemIndexVector.push_back( (*itor).first );
		}
	}


	// Shop - 팔기물품
    int nStartIndexSell;
	int nSelIndexSell;
	MListBox* pListBox = (MListBox*)pResource->FindWidget("MyAllEquipmentList");
	if ( pListBox)
	{
		nStartIndexSell = pListBox->GetStartItem();
		nSelIndexSell   = pListBox->GetSelIndex();
		pListBox->RemoveAll();

		for (int i = 0; i < (int)m_ItemIndexVector.size(); i++)
		{
			MMatchItemDesc* pItemDesc = NULL;

			MITEMNODEMAP::iterator itor = m_ItemMap.find(m_ItemIndexVector[i]);
			if (itor != m_ItemMap.end())
			{
				ZMyItemNode* pItemNode = (*itor).second;
				pItemDesc = MGetMatchItemDescMgr()->GetItemDesc(pItemNode->GetItemID());
				if (pItemDesc != NULL)
				{
					MUID uidItem = (*itor).first;

					if(CheckAddType(pItemDesc->m_nSlot))
					{
						((ZEquipmentListBox*)(pListBox))->Add( uidItem,
															   pItemDesc->m_nID,
															   GetItemIconBitmap(pItemDesc, true),
															   pItemDesc->m_szName,
															   pItemDesc->m_nResLevel,pItemDesc->GetBountyValue());
					}
				}
			}
		}
	}


	// 갖고 있는 아이템 물품(인벤)
    int nStartIndexEquip;
	int nSelIndexEquip;
	pListBox = (MListBox*)pResource->FindWidget("EquipmentList");
	if ( pListBox)
	{
		nStartIndexEquip = pListBox->GetStartItem();
		nSelIndexEquip   = pListBox->GetSelIndex();
		pListBox->RemoveAll();


		// 일반 장비 추가
		for (int i = 0; i < (int)m_ItemIndexVector.size(); i++)
		{
			MMatchItemDesc* pItemDesc = NULL;
			
			MITEMNODEMAP::iterator itor = m_ItemMap.find(m_ItemIndexVector[i]);
			if (itor != m_ItemMap.end())
			{
				ZMyItemNode* pItemNode = (*itor).second;
				pItemDesc = MGetMatchItemDescMgr()->GetItemDesc(pItemNode->GetItemID());
				if (pItemDesc != NULL)
				{
					MUID uidItem = (*itor).first;

					// 장비 목록 창
					if ( CheckAddType( pItemDesc->m_nSlot))
					{
						((ZEquipmentListBox*)(pListBox))->Add( uidItem,
															   pItemDesc->m_nID,
															   GetItemIconBitmap(pItemDesc, true),
															   pItemDesc->m_szName,
															   pItemDesc->m_nResLevel,pItemDesc->GetBountyValue());

						m_ItemIndexVectorEquip.push_back( (*itor).first );
					}
				}
			}
		}

#ifdef _QUEST_ITEM
		// 퀘스트 아이템 추가
		MListBox* pSellListBox = (MListBox*)pResource->FindWidget("MyAllEquipmentList");
		for ( MQUESTITEMNODEMAP::iterator questitem_itor = m_QuestItemMap.begin();  questitem_itor != m_QuestItemMap.end();  questitem_itor++)
		{
			ZMyQuestItemNode* pItemNode = (*questitem_itor).second;
			MQuestItemDesc* pItemDesc = GetQuestItemDescMgr().FindQItemDesc( pItemNode->GetItemID());
			if ( pItemDesc != NULL)
			{
				MUID uidItem;			// NULL

				char szName[ 128];
				if ( pItemNode->m_nCount > 0)
				{
					sprintf_safe( szName, "%s(x%d)", pItemDesc->m_szQuestItemName, pItemNode->m_nCount);

					char szPrice[ 128];
					sprintf_safe( szPrice, "%d", (int)( pItemDesc->m_nPrice * 0.25));

					if ( (m_ListFilter == zshop_item_filter_all) || (m_ListFilter == zshop_item_filter_quest))
					{
						((ZEquipmentListBox*)(pSellListBox))->Add( uidItem,
																pItemDesc->m_nItemID,
																ZApplication::GetGameInterface()->GetQuestItemIcon( pItemDesc->m_nItemID, true),
																szName,
																"-",
																szPrice);

						((ZEquipmentListBox*)(pListBox))->Add( uidItem,
																pItemDesc->m_nItemID,
																ZApplication::GetGameInterface()->GetQuestItemIcon( pItemDesc->m_nItemID, true),
																szName,
																"-",
																szPrice);
					}
				}
			}
		}
#endif
	}

	// 선택바 위치 다시 지정
	pListBox = (MListBox*)pResource->FindWidget("MyAllEquipmentList");
	if ( pListBox)
	{
		pListBox->SetStartItem( nStartIndexSell);
		pListBox->SetSelIndex( min( (pListBox->GetCount() - 1), nSelIndexSell));
	}
	pListBox = (MListBox*)pResource->FindWidget("EquipmentList");
	if ( pListBox)
	{
		pListBox->SetStartItem( nStartIndexEquip);
		pListBox->SetSelIndex( min( (pListBox->GetCount() - 1), nSelIndexEquip));
	}
    

	// 장비 캐릭터 뷰어
	BEGIN_WIDGETLIST("EquipmentInformation", pResource, ZCharacterView*, pCharacterView);
	ZMyInfo* pmi = ZGetMyInfo();
	u32 nEquipedItemID[MMCIP_END];
	for (int i = 0; i < MMCIP_END; i++)
	{
		nEquipedItemID[i] = GetEquipedItemID(MMatchCharItemParts(i));
	}
	pCharacterView->InitCharParts(pmi->GetSex(), pmi->GetHair(), pmi->GetFace(), nEquipedItemID);
	auto* pStageCharacterView = (ZCharacterView*)ZGetGameInterface()->GetIDLResource()->FindWidget("Stage_Charviewer");
	if(pStageCharacterView!= NULL)
		pStageCharacterView->InitCharParts(pmi->GetSex(), pmi->GetHair(), pmi->GetFace(), nEquipedItemID);

	END_WIDGETLIST();

	ZApplication::GetGameInterface()->ChangeEquipPartsToolTipAll();

	BEGIN_WIDGETLIST("EquipmentInformationShop", pResource, ZCharacterView*, pCharacterView);
	ZMyInfo* pmi = ZGetMyInfo();
	u32 nEquipedItemID[MMCIP_END];
	for (int i = 0; i < MMCIP_END; i++)
	{
		nEquipedItemID[i] = GetEquipedItemID(MMatchCharItemParts(i));
	}
	pCharacterView->InitCharParts(pmi->GetSex(), pmi->GetHair(), pmi->GetFace(), nEquipedItemID);
	END_WIDGETLIST();
}

void ZMyItemList::SerializeAccountItem()
{
	ZIDLResource* pResource = ZApplication::GetGameInterface()->GetIDLResource();

	MListBox* pListBox = (MListBox*)pResource->FindWidget("AccountItemList");

	if (pListBox)
	{
		pListBox->RemoveAll();
		m_AccountItemVector.clear();

		for (MACCOUNT_ITEMNODEMAP::iterator itor = m_AccountItemMap.begin();
		itor != m_AccountItemMap.end(); ++itor)
		{
			int nAIID = (*itor).first;
			ZMyItemNode* pItemNode = (*itor).second;
			u32 nItemID = pItemNode->GetItemID();

			MMatchItemDesc* pItemDesc;
			pItemDesc = MGetMatchItemDescMgr()->GetItemDesc(nItemID);

			if (pItemDesc)
			{
				((ZEquipmentListBox*)(pListBox))->Add(nAIID, nItemID, GetItemIconBitmap(pItemDesc, true),
					pItemDesc->m_szName, pItemDesc->m_nResLevel);

				m_AccountItemVector.push_back(nItemID);
			}
		}
	}
}

void ZMyItemList::SetEquipItemID(u32* pEquipItemID)
{
	memcpy(m_nEquipItemID, pEquipItemID, sizeof(m_nEquipItemID));
}

void ZMyItemList::SetEquipItemsAll(MUID* puidEquipItems)
{
	memcpy(m_uidEquipItems, puidEquipItems, sizeof(m_uidEquipItems));

	for (int i = 0; i < MMCIP_END; i++)
	{
		m_nEquipItemID[i] = GetItemID(m_uidEquipItems[i]);
	}
}

void ZMyItemList::SetItemsAll(MTD_ItemNode* pItemNodes, const int nItemCount)
{
	ClearItemMap();

	for (int i = 0; i < nItemCount; i++)
	{
		ZMyItemNode* pNewItemNode = new ZMyItemNode();
		
		bool bIsRentItem = false;
		if ( pItemNodes[i].nRentMinutePeriodRemainder < RENT_MINUTE_PERIOD_UNLIMITED)
			bIsRentItem = true;

		if (bIsRentItem  == false)
		{
			pNewItemNode->Create(pItemNodes[i].uidItem, pItemNodes[i].nItemID);
		}
		else
		{
			pNewItemNode->Create(pItemNodes[i].uidItem, pItemNodes[i].nItemID, bIsRentItem, pItemNodes[i].nRentMinutePeriodRemainder);
		}
		

		m_ItemMap.insert(MITEMNODEMAP::value_type(pItemNodes[i].uidItem, pNewItemNode));
	}
}

MUID ZMyItemList::GetItemUID(int nItemIndex)
{
	if ((nItemIndex < 0) || (nItemIndex >= (int)m_ItemIndexVector.size())) return MUID(0,0);

	return m_ItemIndexVector[nItemIndex];
}

MUID ZMyItemList::GetItemUIDEquip(int nItemIndex)
{
	if ((nItemIndex < 0) || (nItemIndex >= (int)m_ItemIndexVectorEquip.size())) return MUID(0,0);

	return m_ItemIndexVectorEquip[nItemIndex];
}


u32 ZMyItemList::GetAccountItemID(int nPos)
{
	int nSIze = (int)m_AccountItemVector.size();

	if(nPos < 0 || nPos >= nSIze)
		return 0;

	return m_AccountItemVector[nPos];
}

u32 ZMyItemList::GetItemID(const MUID& uidItem)
{
	ZMyItemNode* pItemNode = GetItem(uidItem);
	if (pItemNode == NULL) return 0;
	return pItemNode->GetItemID();

}

int ZMyItemList::GetEquipedTotalWeight()
{
	MMatchItemDesc* pItemDesc = NULL;
	int nTotalWeight = 0;

	for (int i=0; i < MMCIP_END; i++)
	{
		if (m_nEquipItemID[i] != 0)
		{
			pItemDesc = MGetMatchItemDescMgr()->GetItemDesc(m_nEquipItemID[i]);
			if (pItemDesc)
			{
				nTotalWeight += pItemDesc->m_nWeight;
			}
		}
	}
	return nTotalWeight;
}

int ZMyItemList::GetEquipedHPModifier()
{
	MMatchItemDesc* pItemDesc = NULL;
	int nTotalHPModifier = 0;

	for (int i=0; i < MMCIP_END; i++)
	{
		if (m_nEquipItemID[i] != 0)
		{
			pItemDesc = MGetMatchItemDescMgr()->GetItemDesc(m_nEquipItemID[i]);
			if (pItemDesc)
			{
				nTotalHPModifier += pItemDesc->m_nHP;
			}
		}
	}
	return nTotalHPModifier;
}

int ZMyItemList::GetEquipedAPModifier()
{
	MMatchItemDesc* pItemDesc = NULL;
	int nTotalAPModifier = 0;

	for (int i=0; i < MMCIP_END; i++)
	{
		if (m_nEquipItemID[i] != 0)
		{
			pItemDesc = MGetMatchItemDescMgr()->GetItemDesc(m_nEquipItemID[i]);
			if (pItemDesc)
			{
				nTotalAPModifier += pItemDesc->m_nAP;
			}
		}
	}
	return nTotalAPModifier;

}

int ZMyItemList::GetMaxWeight()
{
	MMatchItemDesc* pItemDesc = NULL;
	int nMaxWT = MAX_ITEM_COUNT;

	for (int i=0; i < MMCIP_END; i++)
	{
		if (m_nEquipItemID[i] != 0)
		{
			pItemDesc = MGetMatchItemDescMgr()->GetItemDesc(m_nEquipItemID[i]);
			if (pItemDesc)
			{
				nMaxWT += pItemDesc->m_nMaxWT;
			}
		}
	}
	return nMaxWT;
}


void ZMyItemList::AddAccountItem(int nAIID, u32 nItemID, int nRentMinutePeriodRemainder)
{
	ZMyItemNode* pItemNode = new ZMyItemNode();
	bool bIsRentItem=false;
	if (nRentMinutePeriodRemainder < RENT_MINUTE_PERIOD_UNLIMITED) bIsRentItem=true;	// 기간제 아이템

	pItemNode->Create(nItemID, bIsRentItem, nRentMinutePeriodRemainder);
	m_AccountItemMap.insert(MACCOUNT_ITEMNODEMAP::value_type(nAIID, pItemNode));
}


void ZMyItemList::ClearAccountItems()
{
	ClearAccountItemMap();
}

void ZMyItemList::ClearItemMap()
{
	for (MITEMNODEMAP::iterator itor = m_ItemMap.begin(); itor != m_ItemMap.end(); ++itor)
	{
		delete (*itor).second;
	}
	m_ItemMap.clear();
}

void ZMyItemList::ClearAccountItemMap()
{
	for (MACCOUNT_ITEMNODEMAP::iterator itor = m_AccountItemMap.begin(); itor != m_AccountItemMap.end(); ++itor)
	{
		delete (*itor).second;
	}
	m_AccountItemMap.clear();
}



ZMyItemNode* ZMyItemList::GetItem(int nItemIndex)
{
	if ((nItemIndex < 0) || (nItemIndex >= (int)m_ItemIndexVector.size())) return NULL;

	MUID uidItem = m_ItemIndexVector[nItemIndex];
	
	MITEMNODEMAP::iterator itor = m_ItemMap.find(uidItem);
	if (itor != m_ItemMap.end())
	{
		ZMyItemNode* pItemNode = (*itor).second;
		return pItemNode;
	}

	return NULL;
}

ZMyItemNode* ZMyItemList::GetItemEquip(int nItemIndex)
{
	if ((nItemIndex < 0) || (nItemIndex >= (int)m_ItemIndexVectorEquip.size())) return NULL;

	MUID uidItem = m_ItemIndexVectorEquip[nItemIndex];

	MITEMNODEMAP::iterator itor = m_ItemMap.find(uidItem);
	if (itor != m_ItemMap.end())
	{
		ZMyItemNode* pItemNode = (*itor).second;
		return pItemNode;
	}

	return NULL;
}

ZMyItemNode* ZMyItemList::GetItem(const MUID& uidItem)
{
	MITEMNODEMAP::iterator itor = m_ItemMap.find(uidItem);
	if (itor != m_ItemMap.end())
	{
		ZMyItemNode* pItemNode = (*itor).second;

		return pItemNode;
	}

	return NULL;
}

ZMyItemNode* ZMyItemList::GetEquipedItem(MMatchCharItemParts parts)
{
	MITEMNODEMAP::iterator itor = m_ItemMap.find(m_uidEquipItems[(int)parts]);
	if (itor != m_ItemMap.end())
	{
		ZMyItemNode* pItemNode = (*itor).second;
		return pItemNode;
	}
	
	return NULL;
}


ZMyItemNode* ZMyItemList::GetAccountItem(int nPos)
{
	int nCnt=0;
	for (MACCOUNT_ITEMNODEMAP::iterator itor = m_AccountItemMap.begin();
		itor != m_AccountItemMap.end(); ++itor)
	{
		if (nPos == nCnt)
		{
			ZMyItemNode* pItemNode = (*itor).second;
			return pItemNode;
		}

		nCnt++;
	}

	return NULL;
}


#ifdef _QUEST_ITEM
void ZMyItemList::SetQuestItemsAll( MTD_QuestItemNode* pQuestItemNode, const int nQuestItemCount )
{
	if( 0 == pQuestItemNode)
		return;

	// 전체 리스트를 업데이트 하기 위해서 이전의 데이터를 초기화 함.
	m_QuestItemMap.Clear();

	for( int i = 0; i < nQuestItemCount; ++i )
	{
		if( !m_QuestItemMap.CreateQuestItem(pQuestItemNode[i].m_nItemID, 
											pQuestItemNode[i].m_nCount, 
											GetQuestItemDescMgr().FindQItemDesc(pQuestItemNode[i].m_nItemID)) )
		{
			// error...
		}
	}
}


#endif


///////////////////////////////////////////////////////////////////////////////////


#ifdef _QUEST_ITEM

ZMyQuestItemMap::ZMyQuestItemMap()
{
}


ZMyQuestItemMap::~ZMyQuestItemMap()
{
	// 만약을 위해서.
	Clear();
}



bool ZMyQuestItemMap::Add( const u32 nItemID, ZMyQuestItemNode* pQuestItem )
{
	if( 0 == pQuestItem )
		return false;

	insert( value_type(nItemID, pQuestItem) );

	return true;
}


void ZMyQuestItemMap::Clear()
{
	if( empty() )
		return;

	iterator It, End;

	End = end();
	for( It = begin(); It != End; ++It )
	{
		delete It->second;
	}

	clear();
}


bool ZMyQuestItemMap::CreateQuestItem( const u32 nItemID, const int nCount, MQuestItemDesc* pDesc )
{
	if( (0 > nCount) || (0 == pDesc) )
		return false;

	ZMyQuestItemNode* pQuestItem = new ZMyQuestItemNode;
	if( 0 == pQuestItem )
		return false;

	pQuestItem->Create( nItemID, nCount, pDesc );

	return Add( nItemID, pQuestItem );
}

#endif