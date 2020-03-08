#include "stdafx.h"
#include "MAsyncDBJob_InsertConnLog.h"

void MAsyncDBJob_InsertConnLog::Run(void* pContext)
{
	auto* pDBMgr = static_cast<IDatabase*>(pContext);

	pDBMgr->InsertConnLog(m_nAID, m_szIP, m_strCountryCode3);

	SetResult(MASYNC_RESULT_SUCCEED);
}


bool MAsyncDBJob_InsertConnLog::Input(u32 nAID, char* szIP, const string& strCountryCode3 )
{
	m_nAID = nAID;
	strcpy_safe(m_szIP, szIP);
	m_strCountryCode3 = strCountryCode3;

	return true;
}