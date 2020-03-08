#include "stdafx.h"
#include "MMatchTransDataType.h"
#include "MMath.h"

void Make_MTDItemNode(MTD_ItemNode* pout, const MUID& uidItem, u32 nItemID, int nRentMinutePeriodRemainder)
{
	pout->uidItem = uidItem;
	pout->nItemID = nItemID;

	pout->nRentMinutePeriodRemainder = nRentMinutePeriodRemainder;		// 초단위
}

void Make_MTDAccountItemNode(MTD_AccountItemNode* pout, int nAIID, u32 nItemID, int nRentMinutePeriodRemainder)
{
	pout->nAIID = nAIID;
	pout->nItemID = nItemID;
	pout->nRentMinutePeriodRemainder = nRentMinutePeriodRemainder;		// 초단위
}


void Make_MTDQuestItemNode( MTD_QuestItemNode* pOut, const u32 nItemID, const int nCount )
{
	if( 0 == pOut )
		return;

	pOut->m_nItemID			= nItemID;
	pOut->m_nCount			= nCount;
}
