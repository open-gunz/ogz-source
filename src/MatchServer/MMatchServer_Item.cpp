#include "stdafx.h"
#include "MMatchServer.h"
#include "MSharedCommandTable.h"
#include "MErrorTable.h"
#include "MBlobArray.h"
#include "MObject.h"
#include "MMatchObject.h"
#include "MMatchItem.h"
#include "MAgentObject.h"
#include "MMatchNotify.h"
#include "MMatchObjCache.h"
#include "MMatchStage.h"
#include "MMatchTransDataType.h"
#include "MMatchFormula.h"
#include "MMatchConfig.h"
#include "MCommandCommunicator.h"
#include "MMatchShop.h"
#include "MMatchTransDataType.h"
#include "MDebug.h"	
#include "MMatchAuth.h"
#include "MMatchStatus.h"
#include "MAsyncDBJob.h"
#include "MAsyncDBJob_BringAccountItem.h"
#include "MMatchUtil.h"

bool MMatchServer::InsertCharItem(const MUID& uidPlayer, const u32 nItemID, bool bRentItem, int nRentPeriodHour)
{
	MMatchObject* pObject = GetObject(uidPlayer);
	if (!IsEnabledObject(pObject)) return false;

	MMatchItemDesc* pItemDesc = MGetMatchItemDescMgr()->GetItemDesc(nItemID);
	if (pItemDesc == NULL) return false;


	// 디비에 아이템 추가
	u32 nNewCIID = 0;
	if (!GetDBMgr()->InsertCharItem(pObject->GetCharInfo()->m_nCID, nItemID, bRentItem, nRentPeriodHour, &nNewCIID))
	{
		return false;
	}

	// 오브젝트에 아이템 추가
	int nRentMinutePeriodRemainder = nRentPeriodHour * 60;
	MUID uidNew = MMatchItemMap::UseUID();
	pObject->GetCharInfo()->m_ItemList.CreateItem(uidNew, nNewCIID, nItemID, bRentItem, nRentMinutePeriodRemainder);

	return true;
}

bool MMatchServer::BuyItem(MMatchObject* pObject, unsigned int nItemID, bool bRentItem, int nRentPeriodHour)
{
	MMatchItemDesc* pItemDesc = MGetMatchItemDescMgr()->GetItemDesc(nItemID);
	if (pItemDesc == NULL) return false;


	int nPrice = pItemDesc->m_nBountyPrice;

	if (pObject->GetCharInfo() == NULL) return false;

	// 갖고 있는 아이템 개수가 한도를 넘었는지 판정
	if (pObject->GetCharInfo()->m_ItemList.GetCount() >= MAX_ITEM_COUNT)
	{
		MCommand* pNew = CreateCommand(MC_MATCH_RESPONSE_BUY_ITEM, MUID(0,0));
		pNew->AddParameter(new MCmdParamInt(MERR_TOO_MANY_ITEM));
		RouteToListener(pObject, pNew);

		return false;
	}


	// 아이템을 살 수 있는 자격이 되는지 판정
	if (pObject->GetCharInfo()->m_nBP < nPrice)
	{
		MCommand* pNew = CreateCommand(MC_MATCH_RESPONSE_BUY_ITEM, MUID(0,0));
		pNew->AddParameter(new MCmdParamInt(MERR_TOO_EXPENSIVE_BOUNTY));
		RouteToListener(pObject, pNew);

		return false;
	}

	// 캐릭터 정보 캐슁 업데이트를 먼저 해준다.
	UpdateCharDBCachingData(pObject);	


	u32 nNewCIID = 0;
	if (!GetDBMgr()->BuyBountyItem(pObject->GetCharInfo()->m_nCID, pItemDesc->m_nID, nPrice, &nNewCIID))
	{
		MCommand* pNew = CreateCommand(MC_MATCH_RESPONSE_BUY_ITEM, MUID(0,0));
		pNew->AddParameter(new MCmdParamInt(MERR_CANNOT_BUY_ITEM));
		RouteToListener(pObject, pNew);

		return false;
	}


	// 오브젝트에 바운티 깎는다.
	pObject->GetCharInfo()->m_nBP -= nPrice;

	// 오브젝트에 아이템 추가
	MUID uidNew = MMatchItemMap::UseUID();
	pObject->GetCharInfo()->m_ItemList.CreateItem(uidNew, nNewCIID, pItemDesc->m_nID);


	MCommand* pNew = CreateCommand(MC_MATCH_RESPONSE_BUY_ITEM, MUID(0,0));
	pNew->AddParameter(new MCmdParamInt(MOK));
	RouteToListener(pObject, pNew);

	return true;
}


void MMatchServer::OnRequestBuyItem(const MUID& uidPlayer, const u32 nItemID)
{
	ResponseBuyItem(uidPlayer, nItemID);
}
bool MMatchServer::ResponseBuyItem(const MUID& uidPlayer, const u32 nItemID)
{
	MMatchObject* pObj = GetObject(uidPlayer);
	if (pObj == NULL) return false;

	// 샵에서 팔고 있는 물품인지 확인한다.
	if (MGetMatchShop()->IsSellItem(nItemID) == false) return false;

	BuyItem(pObj, nItemID);

	return true;
}

void MMatchServer::OnRequestSellItem(const MUID& uidPlayer, const MUID& uidItem)
{
	ResponseSellItem(uidPlayer, uidItem);
}
bool MMatchServer::ResponseSellItem(const MUID& uidPlayer, const MUID& uidItem)
{
	MMatchObject* pObj = GetObject(uidPlayer);
	if (pObj == NULL) return false;
	if (pObj->GetCharInfo() == NULL) return false;

	int nPrice = 0;

	MUID uidCharItem = uidItem;
	MMatchItem* pItem = pObj->GetCharInfo()->m_ItemList.GetItem(uidCharItem);
	if ((pItem == NULL) || (pItem->GetDesc() == NULL))
	{
		// 아이템이 존재하지 않을때
		MCommand* pNew = CreateCommand(MC_MATCH_RESPONSE_SELL_ITEM, MUID(0,0));
		pNew->AddParameter(new MCmdParamInt(MERR_CANNOT_SELL_NONE_ITEM));
		RouteToListener(pObj, pNew);

		return false;
	}

	// 장비하고 있으면 팔 수 없다.
	if (pItem->IsEquiped())
	{
		MCommand* pNew = CreateCommand(MC_MATCH_RESPONSE_SELL_ITEM, MUID(0,0));
		pNew->AddParameter(new MCmdParamInt(MERR_CANNOT_SELL_EQUIPED_ITEM));
		RouteToListener(pObj, pNew);

		return false;
	}

	// 캐쉬아이템이면 팔 수 없다.
	if(pItem->GetDesc()->IsCashItem())
	{
		MCommand* pNew = CreateCommand(MC_MATCH_RESPONSE_SELL_ITEM, MUID(0,0));
		pNew->AddParameter(new MCmdParamInt(MERR_CANNOT_SELL_CASHITEM));
		RouteToListener(pObj, pNew);

		return false;
	}


	// 캐릭터 정보 캐슁 업데이트를 먼저 해준다.
	UpdateCharDBCachingData(pObj);	
	
	nPrice = pItem->GetDesc()->GetBountyValue();	
	unsigned int nCID = pObj->GetCharInfo()->m_nCID;
	unsigned int nSelItemID = pItem->GetDesc()->m_nID;
	unsigned int nCIID = pItem->GetCIID();
	int nCharBP = pObj->GetCharInfo()->m_nBP + nPrice;


	if (!GetDBMgr()->SellBountyItem(nCID, nSelItemID, nCIID, nPrice, nCharBP))
	{
		MCommand* pNew = CreateCommand(MC_MATCH_RESPONSE_SELL_ITEM, MUID(0,0));
		pNew->AddParameter(new MCmdParamInt(MERR_CANNOT_SELL_ITEM));
		RouteToListener(pObj, pNew);

		return false;
	}

	// 오브젝트에 바운티 더해준다.
	pObj->GetCharInfo()->m_nBP += nPrice;

	// 오브젝트에서 아이템 삭제
	pObj->GetCharInfo()->m_ItemList.RemoveItem(uidCharItem);


/*
	// 디비에 바운티 더해준다
	if (!GetDBMgr()->UpdateCharBP(pObj->GetCharInfo()->m_nCID, nPrice))
	{
		MCommand* pNew = CreateCommand(MC_MATCH_RESPONSE_SELL_ITEM, MUID(0,0));
		pNew->AddParameter(new MCmdParamInt(MERR_CANNOT_SELL_ITEM));
		RouteToListener(pObj, pNew);

		return false;
	}
	// 오브젝트에 바운티 더해준다.
	pObj->GetCharInfo()->m_nBP += nPrice;

	u32 nSelItemID = pItem->GetDesc()->m_nID;
	if (RemoveCharItem(pObj, uidCharItem) == true)
	{
		// RemoveCharItem 함수 이후에 pItem을 사용하면 안된다.
		pItem = NULL;
		GetDBMgr()->InsertItemPurchaseLogByBounty(nSelItemID, pObj->GetCharInfo()->m_nCID,
			nPrice, pObj->GetCharInfo()->m_nBP, MMatchDBMgr::IPT_SELL);
	}
	else
	{
		MCommand* pNew = CreateCommand(MC_MATCH_RESPONSE_SELL_ITEM, MUID(0,0));
		pNew->AddParameter(new MCmdParamInt(MERR_CANNOT_SELL_ITEM));
		RouteToListener(pObj, pNew);

		return false;
	}
*/

	MCommand* pNew = CreateCommand(MC_MATCH_RESPONSE_SELL_ITEM, MUID(0,0));
	pNew->AddParameter(new MCmdParamInt(MOK));
	RouteToListener(pObj, pNew);


	ResponseCharacterItemList(uidPlayer);	// 새로 바뀐 아이템 리스트도 다시 뿌려준다.

	return true;
}

// 실제 디비와 오브젝트에서 아이템을 삭제
bool MMatchServer::RemoveCharItem(MMatchObject* pObject, MUID& uidItem)
{
	MMatchItem* pItem = pObject->GetCharInfo()->m_ItemList.GetItem(uidItem);
	if (!pItem) return false;

	// 디비에서 아이템 삭제
	if (!GetDBMgr()->DeleteCharItem(pObject->GetCharInfo()->m_nCID, pItem->GetCIID()))
	{
		return false;
	}

	// 만약 장비중이면 해체
	MMatchCharItemParts nCheckParts = MMCIP_END;
	if (pObject->GetCharInfo()->m_EquipedItem.IsEquipedItem(pItem, nCheckParts))
	{
		pObject->GetCharInfo()->m_EquipedItem.Remove(nCheckParts);
	}

	// 오브젝트에서 아이템 삭제
	pObject->GetCharInfo()->m_ItemList.RemoveItem(uidItem);

	return true;
}

void MMatchServer::OnRequestShopItemList(const MUID& uidPlayer, const int nFirstItemIndex, const int nItemCount)
{
	ResponseShopItemList(uidPlayer, nFirstItemIndex, nItemCount);
}

void MMatchServer::ResponseShopItemList(const MUID& uidPlayer, const int nFirstItemIndex, const int nItemCount)
{
	MMatchObject* pObj = GetObject(uidPlayer);
	if (pObj == NULL) return;

	int nRealItemCount = 0;
	if ((nItemCount <= 0) || (nItemCount > MGetMatchShop()->GetCount())) nRealItemCount = MGetMatchShop()->GetCount();
	else nRealItemCount = nItemCount;


	MCommand* pNew = CreateCommand(MC_MATCH_RESPONSE_SHOP_ITEMLIST, MUID(0,0));
	void* pItemArray = MMakeBlobArray(sizeof(u32), nRealItemCount);
	int nIndex=0;

	for (int i = nFirstItemIndex; i < nFirstItemIndex+nRealItemCount; i++)
	{
		u32* pnItemID = (u32*)MGetBlobArrayElement(pItemArray, nIndex++);
		ShopItemNode* pSINode = MGetMatchShop()->GetSellItem(i);

		if (pSINode != NULL)
		{
			*pnItemID = pSINode->nItemID;
		}
		else
		{
			*pnItemID = 0;
		}

	}

	pNew->AddParameter(new MCommandParameterBlob(pItemArray, MGetBlobArraySize(pItemArray)));
	MEraseBlobArray(pItemArray);

	RouteToListener(pObj, pNew);	

}

void MMatchServer::OnRequestCharacterItemList(const MUID& uidPlayer)
{
	ResponseCharacterItemList(uidPlayer);
}

void MMatchServer::ResponseCharacterItemList(const MUID& uidPlayer)
{
	MMatchObject* pObj = GetObject(uidPlayer);
	if ((pObj == NULL) || (pObj->GetCharInfo() == NULL)) 
	{
		mlog("ResponseCharacterItemList > pObj or pObj->GetCharInfo() IS NULL\n");
		return;
	}

	// 이전에 디비 억세스를 안했었으면 디비에서 아이템 정보를 가져온다
	if (!pObj->GetCharInfo()->m_ItemList.IsDoneDbAccess())
	{
		if (!GetDBMgr()->GetCharItemInfo(*pObj->GetCharInfo()))
		{
			mlog("DB Query(ResponseCharacterItemList > GetCharItemInfo) Failed\n");
			return;
		}
	}

	if( MSM_TEST == MGetServerConfig()->GetServerMode() ) 
	{
		if( !pObj->GetCharInfo()->m_QuestItemList.IsDoneDbAccess() )
		{
			if( !GetDBMgr()->GetCharQuestItemInfo(pObj->GetCharInfo()) )
			{
				mlog( "MMatchServer::ResponseCharacterItemList - 디비 접근 실패후 제접근 실패. 게임 진행 불가.\n" );
				return;
			}
		}
	}

	MCommand* pNew = CreateCommand(MC_MATCH_RESPONSE_CHARACTER_ITEMLIST, MUID(0,0));

	// 바운티 전송
	pNew->AddParameter(new MCommandParameterInt(pObj->GetCharInfo()->m_nBP));

	// 장비한 아이템 전송
	int nRealEquipedItemCount = 0;
	int nIndex = 0;
	void* pEquipItemArray = MMakeBlobArray(sizeof(MUID), MMCIP_END);
	for (int i = 0; i < MMCIP_END; i++)
	{
		MUID* pUID = (MUID*)MGetBlobArrayElement(pEquipItemArray, nIndex++);

		if (!pObj->GetCharInfo()->m_EquipedItem.IsEmpty(MMatchCharItemParts(i)))
		{
			*pUID = pObj->GetCharInfo()->m_EquipedItem.GetItem(MMatchCharItemParts(i))->GetUID();
			nRealEquipedItemCount++;
		}
		else
		{
			*pUID = MUID(0,0);
		}
	}

	pNew->AddParameter(new MCommandParameterBlob(pEquipItemArray, MGetBlobArraySize(pEquipItemArray)));
	MEraseBlobArray(pEquipItemArray);


	// 갖고 있는 아이템 리스트 전송
	int nItemCount = pObj->GetCharInfo()->m_ItemList.GetCount();

	void* pItemArray = MMakeBlobArray(sizeof(MTD_ItemNode), nItemCount);
	MMatchItemMap* pItemList = &pObj->GetCharInfo()->m_ItemList;

	nIndex = 0;
	for (MMatchItemMap::iterator itor = pItemList->begin(); itor != pItemList->end(); ++itor)
	{
		MMatchItem* pItem = (*itor).second;

		MTD_ItemNode* pItemNode = (MTD_ItemNode*)MGetBlobArrayElement(pItemArray, nIndex++);

		auto nPassTime = MGetTimeDistance(pItem->GetRentItemRegTime(), GetTickTime());
		int nPassMinuteTime = static_cast<int>(nPassTime / (1000 * 60));

		int nRentMinutePeriodRemainder = RENT_MINUTE_PERIOD_UNLIMITED;
		if (pItem->IsRentItem())
		{
			nRentMinutePeriodRemainder = pItem->GetRentMinutePeriodRemainder() - nPassMinuteTime;
		}

		Make_MTDItemNode(pItemNode, pItem->GetUID(), pItem->GetDescID(), nRentMinutePeriodRemainder);
	}


	pNew->AddParameter(new MCommandParameterBlob(pItemArray, MGetBlobArraySize(pItemArray)));
	MEraseBlobArray(pItemArray);

	RouteToListener(pObj, pNew);	
}

void MMatchServer::OnRequestAccountItemList(const MUID& uidPlayer)
{
	ResponseAccountItemList(uidPlayer);
}
void MMatchServer::ResponseAccountItemList(const MUID& uidPlayer)
{
	MMatchObject* pObj = GetObject(uidPlayer);
	if ((pObj == NULL) || (pObj->GetCharInfo() == NULL)) 
	{
		mlog("ResponseAccountItemList > pObj or pObj->GetCharInfo() IS NULL\n");
		return;
	}

#define MAX_ACCOUNT_ITEM		1000		// 최고 1000개로 제한한다.
	MAccountItemNode accountItems[MAX_ACCOUNT_ITEM];
	int nItemCount = 0;

#define MAX_EXPIRED_ACCOUNT_ITEM	100
	MAccountItemNode ExpiredItemList[100];
	int nExpiredItemCount = 0;

	// 디비에서 AccountItem을 가져온다
	if (!GetDBMgr()->GetAccountItemInfo(pObj->GetAccountInfo()->m_nAID, accountItems, &nItemCount, MAX_ACCOUNT_ITEM,
										 ExpiredItemList, &nExpiredItemCount, MAX_EXPIRED_ACCOUNT_ITEM))
	{
		mlog("DB Query(ResponseAccountItemList > GetAccountItemInfo) Failed\n");
		return;
	}

	// 여기서 중앙은행의 기간만료 아이템이 있는지 체크한다.
	if (nExpiredItemCount > 0)
	{
		vector<u32> vecExpiredItemIDList;

		for (int i = 0; i < nExpiredItemCount; i++)
		{
			// 디비에서 기간만료된 AccountItem을 지운다.
			if (GetDBMgr()->DeleteExpiredAccountItem(ExpiredItemList[i].nAIID))
			{
				vecExpiredItemIDList.push_back(ExpiredItemList[i].nItemID);
			}
			else
			{
				mlog("DB Query(ResponseAccountItemList > DeleteExpiredAccountItem) Failed\n");
			}
		}
		
		if (!vecExpiredItemIDList.empty())
		{
			ResponseExpiredItemIDList(pObj, vecExpiredItemIDList);
		}
	}



	if (nItemCount > 0)
	{
		MCommand* pNew = CreateCommand(MC_MATCH_RESPONSE_ACCOUNT_ITEMLIST, MUID(0,0));

		// 갖고 있는 아이템 리스트 전송
		void* pItemArray = MMakeBlobArray(sizeof(MTD_AccountItemNode), nItemCount);
		

		for (int i = 0; i < nItemCount; i++)
		{
			MTD_AccountItemNode* pItemNode = (MTD_AccountItemNode*)MGetBlobArrayElement(pItemArray, i);

			Make_MTDAccountItemNode(pItemNode, 
									accountItems[i].nAIID, 
									accountItems[i].nItemID, 
									accountItems[i].nRentMinutePeriodRemainder);
		}

		pNew->AddParameter(new MCommandParameterBlob(pItemArray, MGetBlobArraySize(pItemArray)));
		MEraseBlobArray(pItemArray);

		RouteToListener(pObj, pNew);	
	}
}

void MMatchServer::OnRequestEquipItem(const MUID& uidPlayer, const MUID& uidItem, const i32 nEquipmentSlot)
{
	if (nEquipmentSlot >= MMCIP_END) return;
	MMatchCharItemParts parts = MMatchCharItemParts(nEquipmentSlot);

	ResponseEquipItem(uidPlayer, uidItem, parts);
}

void MMatchServer::ResponseEquipItem(const MUID& uidPlayer, const MUID& uidItem, const MMatchCharItemParts parts)
{
	MMatchObject* pObj = GetObject(uidPlayer);
	if (pObj == NULL) return;

	MMatchCharInfo* pCharInfo = pObj->GetCharInfo();
	if (pCharInfo == NULL) return;

	MUID uidRealItem = uidItem;
	MMatchItem* pItem = pCharInfo->m_ItemList.GetItem(uidRealItem);

	auto Respond = [&](int nResult) {
		MCommand* pNew = CreateCommand(MC_MATCH_RESPONSE_EQUIP_ITEM, MUID(0, 0));
		pNew->AddParameter(new MCommandParameterInt(nResult));
		RouteToListener(pObj, pNew);
	};

	if ((pItem == NULL) || (!IsSuitableItemSlot(pItem->GetDesc()->m_nSlot, parts)))
	{
		Respond(MERR_CANNOT_EQUIP_ITEM);
		return;
	}

	{
		auto nResult = ValidateEquipItem(pObj, pItem, parts);
		if (nResult != MOK)
		{
			Respond(nResult);
			return;
		}
	}

	u32 nItemCIID = 0;
	u32 nItemID = 0;

	nItemCIID = pItem->GetCIID();
	nItemID = pItem->GetDesc()->m_nID;

	if (GetDBMgr()->UpdateEquipedItem(pCharInfo->m_nCID, parts, nItemCIID, nItemID))
	{
		pCharInfo->m_EquipedItem.SetItem(parts, pItem);
	}
	else
	{
		Respond(MERR_CANNOT_EQUIP_ITEM);
		return;
	}

#ifdef UPDATE_STAGE_EQUIP_LOOK
	ResponseCharacterItemList(uidPlayer);

	if (FindStage(pObj->GetStageUID()))
	{
		MCommand* pEquipInfo = CreateCommand(MC_MATCH_ROUTE_UPDATE_STAGE_EQUIP_LOOK, MUID(0, 0));
		pEquipInfo->AddParameter(new MCmdParamUID(uidPlayer));
		pEquipInfo->AddParameter(new MCmdParamInt(parts));
		pEquipInfo->AddParameter(new MCmdParamInt(pItem->GetDescID()));
		RouteToStage(pObj->GetStageUID(), pEquipInfo);
	}
#else
	Respond(MOK);
#endif
}

void MMatchServer::OnRequestTakeoffItem(const MUID& uidPlayer, const u32 nEquipmentSlot)
{
	if (nEquipmentSlot >= MMCIP_END) return;
	MMatchCharItemParts parts = MMatchCharItemParts(nEquipmentSlot);

	ResponseTakeoffItem(uidPlayer, parts);
}



void MMatchServer::ResponseTakeoffItem(const MUID& uidPlayer, const MMatchCharItemParts parts)
{
	MMatchObject* pObj = GetObject(uidPlayer);
	if (pObj == NULL) return;

	MMatchCharInfo* pCharInfo = pObj->GetCharInfo();
	if (pCharInfo == NULL) return;
	if (pCharInfo->m_EquipedItem.IsEmpty(parts)) return;


	
	MMatchItem* pItem = pCharInfo->m_EquipedItem.GetItem(parts);
	MMatchItemDesc* pItemDesc = pItem->GetDesc();
	if (pItemDesc == NULL) return;

	auto Respond = [&](int nResult) {
		MCommand* pNew = CreateCommand(MC_MATCH_RESPONSE_TAKEOFF_ITEM, MUID(0, 0));
		pNew->AddParameter(new MCommandParameterInt(nResult));
		RouteToListener(pObj, pNew);
	};

	int nWeight=0, nMaxWeight=0;
	pCharInfo->m_EquipedItem.GetTotalWeight(&nWeight, &nMaxWeight);
	nMaxWeight = pCharInfo->m_nMaxWeight + nMaxWeight - pItemDesc->m_nMaxWT;
	nWeight -= pItemDesc->m_nWeight;

	if (nWeight > nMaxWeight)
	{
		Respond(MERR_CANNOT_TAKEOFF_ITEM_BY_WEIGHT);
		return;
	}

	pCharInfo->m_EquipedItem.Remove(parts);

	if (!GetDBMgr()->UpdateEquipedItem(pCharInfo->m_nCID, parts, 0, 0))
	{
		Respond(MERR_CANNOT_TAKEOFF_ITEM);
		return;
	}

#ifdef UPDATE_STAGE_EQUIP_LOOK
	ResponseCharacterItemList(uidPlayer);

	if (FindStage(pObj->GetStageUID()))
	{
		MCommand* pEquipInfo = CreateCommand(MC_MATCH_ROUTE_UPDATE_STAGE_EQUIP_LOOK, MUID(0, 0));
		pEquipInfo->AddParameter(new MCmdParamUID(uidPlayer));
		pEquipInfo->AddParameter(new MCmdParamInt(parts));
		pEquipInfo->AddParameter(new MCmdParamInt(0));
		RouteToStage(pObj->GetStageUID(), pEquipInfo);
	}
#else
	Respond(MOK);
#endif
}


void MMatchServer::OnRequestBringAccountItem(const MUID& uidPlayer, const int nAIID)
{
	ResponseBringAccountItem(uidPlayer, nAIID);
}


void MMatchServer::ResponseBringAccountItem(const MUID& uidPlayer, const int nAIID)
{
	MMatchObject* pObj = GetObject(uidPlayer);
	if ((pObj == NULL) || (pObj->GetCharInfo() == NULL)) return;

	MMatchCharInfo* pCharInfo = pObj->GetCharInfo();

	// Async DB
	MAsyncDBJob_BringAccountItem* pJob = new MAsyncDBJob_BringAccountItem(pObj->GetUID());
	pJob->Input(pObj->GetAccountInfo()->m_nAID, pObj->GetCharInfo()->m_nCID, nAIID);
	PostAsyncJob(pJob);
}


void MMatchServer::OnRequestBringBackAccountItem(const MUID& uidPlayer, const MUID& uidItem)
{
	ResponseBringBackAccountItem(uidPlayer, uidItem);
}

void MMatchServer::ResponseBringBackAccountItem(const MUID& uidPlayer, const MUID& uidItem)
{
	MMatchObject* pObj = GetObject(uidPlayer);
	if ((pObj == NULL) || (pObj->GetCharInfo() == NULL)) return;

	MMatchCharInfo* pCharInfo = pObj->GetCharInfo();

	int nRet = MERR_UNKNOWN;

	MUID uidCharItem = uidItem;
	MMatchItem* pItem = pObj->GetCharInfo()->m_ItemList.GetItem(uidCharItem);
	if ((pItem == NULL) || (pItem->GetDesc() == NULL))
	{
		// 아이템이 존재하지 않을때
		MCommand* pNew = CreateCommand(MC_MATCH_RESPONSE_BRING_BACK_ACCOUNTITEM, MUID(0,0));
		pNew->AddParameter(new MCmdParamInt(MERR_BRING_BACK_ACCOUNTITEM));
		RouteToListener(pObj, pNew);

		return;
	}

	// 장비하고 있으면 옮길 수 없다
	if (pItem->IsEquiped())
	{
		MCommand* pNew = CreateCommand(MC_MATCH_RESPONSE_BRING_BACK_ACCOUNTITEM, MUID(0,0));
		pNew->AddParameter(new MCmdParamInt(MERR_BRING_BACK_ACCOUNTITEM));
		RouteToListener(pObj, pNew);

		return;
	}

	// 캐쉬아이템이 아니면 옮길 수 없다
	if(!pItem->GetDesc()->IsCashItem())
	{
		MCommand* pNew = CreateCommand(MC_MATCH_RESPONSE_BRING_BACK_ACCOUNTITEM, MUID(0,0));
		pNew->AddParameter(new MCmdParamInt(MERR_BRING_BACK_ACCOUNTITEM_FOR_CASHITEM));
		RouteToListener(pObj, pNew);

		return;
	}

	// 디비에서 중앙은행으로 옮겨준다.
	if (!GetDBMgr()->BringBackAccountItem(pObj->GetAccountInfo()->m_nAID, 
										  pObj->GetCharInfo()->m_nCID, 
										  pItem->GetCIID()))
	{
		mlog("DB Query(ResponseBringBackAccountItem > BringBackAccountItem) Failed(ciid=%u)\n", pItem->GetCIID());

		MCommand* pNew = CreateCommand(MC_MATCH_RESPONSE_BRING_BACK_ACCOUNTITEM, MUID(0,0));
		pNew->AddParameter(new MCmdParamInt(MERR_BRING_BACK_ACCOUNTITEM));
		RouteToListener(pObj, pNew);

		return;
	}


	// 오브젝트에서 아이템 삭제
	pObj->GetCharInfo()->m_ItemList.RemoveItem(uidCharItem);

	MCommand* pNew = CreateCommand(MC_MATCH_RESPONSE_BRING_BACK_ACCOUNTITEM, MUID(0,0));
	pNew->AddParameter(new MCmdParamInt(MOK));
	RouteToListener(pObj, pNew);


	ResponseCharacterItemList(uidPlayer);	// 새로 바뀐 아이템 리스트도 다시 뿌려준다.
}
