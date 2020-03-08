#include "stdafx.h"
#include "MMatchClan.h"
#include "MMatchServer.h"
#include "MMatchObject.h"
#include "MSmartRefreshImpl.h"
#include "MDebug.h"
#include "MMatchUtil.h"

#define MTICK_CLAN_RUN							500
#define MTICK_CLAN_DBREFRESH_PERIOD_LIMIT		7200000			// (1000*60*120) - 2 hours
#define MTICK_CLAN_EMPTY_PERIOD_LIMIT			600000			// (1000*60*10) - 10 secs


MMatchClan::MMatchClan()
{
	Clear();
}

MMatchClan::~MMatchClan()
{


}


void MMatchClan::Clear()
{
	m_nCLID = 0;
	m_szClanName[0] = 0;
	m_nSeriesOfVictories = 0;

	memset(&m_ClanInfoEx, 0, sizeof(ClanInfoEx));
	m_nDBRefreshLifeTime = 0;
	m_Members.clear();

	m_nEmptyPeriod = 0;
}

void MMatchClan::InitClanInfoFromDB()
{
	if (m_nCLID == 0) return;

	MDB_ClanInfo dbClanInfo;

	if (MMatchServer::GetInstance()->GetDBMgr()->GetClanInfo(m_nCLID, &dbClanInfo))
	{
		// 0 means not initiated so the first value of InitClanInfoEx should be non-zero
		InitClanInfoEx(1, dbClanInfo.nTotalPoint, dbClanInfo.nPoint, dbClanInfo.nRanking,
			dbClanInfo.nWins, dbClanInfo.nLosses, dbClanInfo.nTotalMemberCount, dbClanInfo.szMasterName,
			dbClanInfo.szEmblemUrl, dbClanInfo.nEmblemChecksum);
	}
	else
	{
		mlog("DB Query(GetClanInfo) Failed\n");
	}

}

void MMatchClan::Create(int nCLID, const char* szClanName)
{
	m_nCLID = nCLID;
	strcpy_safe(m_szClanName, szClanName);


	m_SmartRefresh.AddCategory(new MRefreshCategoryClanMemberImpl(this, 0));	// Category 0 로 전체목록 모두 담당
}

void MMatchClan::InitClanInfoEx(const int nLevel, const int nTotalPoint, const int nPoint, const int nRanking,
		            const int nWins, const int nLosses, const int nTotalMemberCount, const char* szMaster,
					const char* szEmblemUrl, int nEmblemChecksum)
{
	m_ClanInfoEx.nLevel = nLevel;
	m_ClanInfoEx.nTotalPoint = nTotalPoint;
	m_ClanInfoEx.nPoint = nPoint;
	m_ClanInfoEx.nRanking = nRanking;
	m_ClanInfoEx.nWins = nWins;
	m_ClanInfoEx.nLosses = nLosses;
	m_ClanInfoEx.nTotalMemberCount = nTotalMemberCount;
	strcpy_safe(m_ClanInfoEx.szMaster, szMaster);
	strcpy_safe(m_ClanInfoEx.szEmblemUrl, szEmblemUrl);
	m_ClanInfoEx.nEmblemChecksum = nEmblemChecksum;
}

void MMatchClan::AddObject(const MUID& uid, MMatchObject* pObj)
{
	m_Members.Insert(uid, pObj);

}

void MMatchClan::RemoveObject(const MUID& uid)
{
	auto itor = m_Members.find(uid);
	if (itor != m_Members.end())
	{
		m_Members.erase(itor);
	}
}


void MMatchClan::Tick(u64 nClock)
{
	m_nDBRefreshLifeTime += MTICK_CLAN_RUN;
	if (m_nDBRefreshLifeTime >= MTICK_CLAN_DBREFRESH_PERIOD_LIMIT)
	{
		InitClanInfoFromDB();
		m_nDBRefreshLifeTime = 0;
	}

	m_SmartRefresh.UpdateCategory(nClock);

	if (GetMemberCount() <= 0) m_nEmptyPeriod += MTICK_CLAN_RUN; 
	else m_nEmptyPeriod = 0;
}

void MMatchClan::SyncPlayerList(MMatchObject* pObj, int nCategory)
{
	m_SmartRefresh.SyncClient(pObj->GetRefreshClientClanMemberImplement());
}

void MMatchClan::InsertMatchedClanID(int nCLID)
{
	m_MatchedClanList.push_back(nCLID);
	if (m_MatchedClanList.size() >= 10)
	{
		m_MatchedClanList.erase(m_MatchedClanList.begin());
	}
}

bool MMatchClan::CheckLifePeriod()
{
	if (GetMemberCount() > 0) return true;
	if (m_nEmptyPeriod < MTICK_CLAN_EMPTY_PERIOD_LIMIT) return true;

	return false;
}

/////////////////////////////////////////////////////////

MMatchClanMap::MMatchClanMap()
{
	m_nLastTick = 0;

}

MMatchClanMap::~MMatchClanMap()
{


}


void MMatchClanMap::Destroy()
{
	for (iterator itor = begin(); itor != end(); ++itor)
	{
		MMatchClan* pClan = (*itor).second;
		delete pClan;
	}

	clear();
}

void MMatchClanMap::CreateClan(int nCLID, const char* szClanName)
{
	MMatchClan* pNewClan = new MMatchClan;
	pNewClan->Create(nCLID, szClanName);

	insert(value_type(nCLID, pNewClan));
	m_ClanNameMap.insert(map<std::string, MMatchClan*>::value_type(string(szClanName), pNewClan));
}

void MMatchClanMap::DestroyClan(int nCLID, MMatchClanMap::iterator* pNextItor)
{
	iterator itor = find(nCLID);
	if (itor != end())
	{
		MMatchClan* pClan = (*itor).second;


		map<std::string, MMatchClan*>::iterator itorClanNameMap = m_ClanNameMap.find(string(pClan->GetName()));
		if (itorClanNameMap != m_ClanNameMap.end())
		{
			m_ClanNameMap.erase(itorClanNameMap);
		}

		delete pClan;
		MMatchClanMap::iterator itorTemp = erase(itor);
		if (pNextItor) *pNextItor = itorTemp;
	}
}

void MMatchClanMap::AddObject(const MUID& uid, MMatchObject* pObj)
{
	if (! IsEnabledObject(pObj)) return;

	int nCLID = pObj->GetCharInfo()->m_ClanInfo.m_nClanID;
	if (nCLID == 0) return;

	// 클랜이 없으면 새로 생성
	iterator itor = find(nCLID);
	if (itor == end()) 
	{
		CreateClan(nCLID, pObj->GetCharInfo()->m_ClanInfo.m_szClanName);
	}

	itor = find(nCLID);
	if (itor != end())
	{
		MMatchClan* pClan = (*itor).second;

		pClan->AddObject(uid, pObj);
	}
}

void MMatchClanMap::RemoveObject(const MUID& uid, MMatchObject* pObj)
{
	if (! IsEnabledObject(pObj)) return;
	int nCLID = pObj->GetCharInfo()->m_ClanInfo.m_nClanID;
	if (nCLID == 0) return;

	iterator itor = find(nCLID);
	if (itor != end())
	{
		MMatchClan* pClan = (*itor).second;

		pClan->RemoveObject(uid);
	}
}

bool MMatchClanMap::CheckTick(u64 nClock)
{
	if (MGetTimeDistance(m_nLastTick, nClock) < MTICK_CLAN_RUN) return false;

	m_nLastTick = nClock;
	return true;
}


void MMatchClanMap::Tick(u64 nClock)
{
	if (!CheckTick(nClock)) return;

	// Update Clans
	for(MMatchClanMap::iterator iClan=begin(); iClan!=end();)
	{
		MMatchClan* pClan = (*iClan).second;
		pClan->Tick(nClock);

		if (pClan->CheckLifePeriod() == false) 
		{
			DestroyClan(pClan->GetCLID(), &iClan);
			continue;
		}
		else
		{
			++iClan;
		}
	}
}

MMatchClan* MMatchClanMap::GetClan(const int nCLID)
{
	iterator itor = find(nCLID);
	if (itor != end())
	{
		return (*itor).second;
	}
	
	return NULL;
}

MMatchClan* MMatchClanMap::GetClan(const char* szClanName)
{
	map<string, MMatchClan*>::iterator itor = m_ClanNameMap.find(string(szClanName));
	if (itor != m_ClanNameMap.end())
	{
		return (*itor).second;
	}

	return NULL;
}

