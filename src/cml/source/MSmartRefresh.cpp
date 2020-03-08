#include "stdafx.h"
#include "MSmartRefresh.h"


#define TIME_CATEGORY_UPDATE	500		// 0.5 sec


//// MRefreshCategory ////
MRefreshCategory::MRefreshCategory(int nCategory)
{
	m_nCategory = nCategory;
	SetChecksum(0);
	SetLastUpdateTick(0);
}

MRefreshCategory::~MRefreshCategory()
{
}

bool MRefreshCategory::UpdateChecksum(u64 nTick)
{
	if (nTick > GetLastUpdateTick() + TIME_CATEGORY_UPDATE) {
		SetLastUpdateTick(nTick);
		return OnUpdateChecksum(nTick);
	}
	return false;
}

//// MRefreshClient ////
MRefreshClient::MRefreshClient()
{
	SetCategory(0);
	Enable(false);
	SetChecksum(0);
	SetLastUpdatedTime(0);
}

MRefreshClient::~MRefreshClient()
{
}

bool MRefreshClient::Sync(u32 nChecksum)
{
	if (OnSync(nChecksum) == true) {
		SetChecksum(nChecksum);
		return true;
	} else {
		return false;
	}
}

//// MSmartRefresh ////
MSmartRefresh::MSmartRefresh()
{
}

MSmartRefresh::~MSmartRefresh()
{
	Clear();
}

void MSmartRefresh::Clear()
{
	while(m_CategoryMap.size() > 0) {
		MRefreshCategory* pCategory = (*m_CategoryMap.begin()).second;
		delete pCategory;
		m_CategoryMap.erase(m_CategoryMap.begin());
	}
}

MRefreshCategory* MSmartRefresh::GetCategory(int nCategory)
{
	MRefreshCategoryMap::iterator i = m_CategoryMap.find(nCategory);
	if(i==m_CategoryMap.end())
		return NULL;
	else
		return (*i).second;
	return NULL;
}

void MSmartRefresh::AddCategory(MRefreshCategory* pCategory)
{
	m_CategoryMap.insert(MRefreshCategoryMap::value_type(pCategory->GetCategory(), pCategory));
}

void MSmartRefresh::UpdateCategory(u64 nTick)
{
	for (MRefreshCategoryMap::iterator i=m_CategoryMap.begin(); i!=m_CategoryMap.end(); i++) {
		MRefreshCategory* pCategory = (*i).second;
		pCategory->UpdateChecksum(nTick);
	}
}

bool MSmartRefresh::SyncClient(MRefreshClient* pClient)
{
	MRefreshCategory* pCategory = GetCategory(pClient->GetCategory());
	if (pCategory == NULL) 
		return false;

	if (pClient->IsEnable() == false)
		return false;

	if (pCategory->GetChecksum() == pClient->GetChecksum())
		return false;

	return pClient->Sync(pCategory->GetChecksum());
}
