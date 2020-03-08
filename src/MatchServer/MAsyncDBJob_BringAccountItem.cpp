#include "stdafx.h"
#include "MAsyncDBJob_BringAccountItem.h"

void MAsyncDBJob_BringAccountItem::Run(void* pContext)
{
	auto* pDBMgr = static_cast<IDatabase*>(pContext);


	if (!pDBMgr->BringAccountItem(m_nAID,
								 m_nCID, 
								 m_nAIID, 
								 &m_nNewCIID, 
								 &m_nNewItemID, 
								 &m_bIsRentItem, 
								 &m_nRentMinutePeriodRemainder))
	{
		SetResult(MASYNC_RESULT_FAILED);
		return;
	}


	SetResult(MASYNC_RESULT_SUCCEED);
}


bool MAsyncDBJob_BringAccountItem::Input(const int nAID, const int nCID, const int nAIID)
{
	m_nAID = nAID;
	m_nCID = nCID;
	m_nAIID = nAIID;
	return true;
}