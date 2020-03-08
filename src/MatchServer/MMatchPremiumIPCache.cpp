#include "stdafx.h"
#include "MMatchPremiumIPCache.h"

#define MAX_PREMIUMIP_CACHE_DBFAILED		5
#define MAX_PREMIUMIP_CACHE_FAILEDCHECK		100
#define MAX_PREMIUMIP_CACHE_TICK			10000	// ms , 10 Sec
#define MAX_PREMIUMIP_CACHE_TIMEOUT			3600000	// ms , 1 Hour

MMatchPremiumIPCache::MMatchPremiumIPCache() : m_nDBFailedCount(0), m_nFailedCheckCount(0)
{
}

MMatchPremiumIPCache::~MMatchPremiumIPCache()
{
}

MMatchPremiumIPCache* MMatchPremiumIPCache::GetInstance()
{
	static MMatchPremiumIPCache m_stPremiumIPCache;
	return &m_stPremiumIPCache;
}


bool MMatchPremiumIPCache::CheckPremiumIP(u32 dwIP, bool& outIsPremiumIP)
{
	MMatchPremiumIPMap::iterator itor;
	bool bExist = false;
	bool bPremiumIP = false;

	Lock(); //////////////////////////////////////

	// 먼저 프리미엄IP가 아닌 세트에서 검사
	itor = m_NotPremiumIPMap.find(dwIP);
	if (itor != m_NotPremiumIPMap.end())
	{
		bPremiumIP = false;
		bExist = true;
	}
	else
	{
		// 프리미엄IP 세트에서 검사
		itor = m_PremiumIPMap.find(dwIP);
		if (itor != m_PremiumIPMap.end())
		{
			bPremiumIP = true;
			bExist = true;
		}
	}

	// 만약 DB가 멈춰져있는 상태라면 캐쉬에 있는거라고 간주하고 무조건 false를 반환한다.
	if ((m_nDBFailedCount > MAX_PREMIUMIP_CACHE_DBFAILED) && (m_nFailedCheckCount < MAX_PREMIUMIP_CACHE_FAILEDCHECK))
	{
		++m_nFailedCheckCount;
		bPremiumIP = false;
		bExist = true;
	}
	else
	{
		m_nFailedCheckCount = 0;
	}


	Unlock(); ////////////////////////////////////

	if (bExist) outIsPremiumIP = bPremiumIP;
	

	return bExist;
}

void MMatchPremiumIPCache::AddIP(u32 dwIP, bool bPremiumIP)
{
	Lock(); ///////////////////

	// db 검사후 IP를 추가하는 것이니 DB가 살아있다는 증거
	m_nDBFailedCount = 0;

	MMatchPremiumIPNode node(dwIP, GetGlobalTimeMS());

	if (bPremiumIP)
	{
		m_PremiumIPMap.insert(MMatchPremiumIPMap::value_type(dwIP, node));
	}
	else
	{
		m_NotPremiumIPMap.insert(MMatchPremiumIPMap::value_type(dwIP, node));
	}

	Unlock(); /////////////////
}


void MMatchPremiumIPCache::OnDBFailed()
{
	Lock(); ///////////////////

	if (m_nDBFailedCount > MAX_PREMIUMIP_CACHE_DBFAILED) m_nDBFailedCount = 0;
	++m_nDBFailedCount;
	m_nFailedCheckCount = 0;

	Unlock(); /////////////////
}

void MMatchPremiumIPCache::Update()
{
	static u64 tmLastUpdate = 0;
	auto tmNow = GetGlobalTimeMS();
	if (tmNow - tmLastUpdate < MAX_PREMIUMIP_CACHE_TICK) {
		return;
	} else {
		tmLastUpdate = tmNow;
	}

	Lock(); ///////////////////

	for (MMatchPremiumIPMap::iterator i=m_PremiumIPMap.begin(); i!=m_PremiumIPMap.end();) {
		if (tmNow - ((*i).second).GetTime() >= MAX_PREMIUMIP_CACHE_TIMEOUT) {
			i=m_PremiumIPMap.erase(i);
			continue;
		} else {
			i++;
			continue;
		}
	}

	for (MMatchPremiumIPMap::iterator i=m_NotPremiumIPMap.begin(); i!=m_NotPremiumIPMap.end();) {
		if (tmNow - ((*i).second).GetTime() >= MAX_PREMIUMIP_CACHE_TIMEOUT) {
			i=m_NotPremiumIPMap.erase(i);
			continue;
		} else {
			i++;
			continue;
		}
	}

	Unlock(); /////////////////
}
