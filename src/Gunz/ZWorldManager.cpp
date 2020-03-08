#include "StdAfx.h"
#include "ZMap.h"
#include "ZWorld.h"
#include "ZWorldManager.h"
#include "ZInitialLoading.h"

ZWorldManager::ZWorldManager(void) : m_nCurrent(-1)
{
}

ZWorldManager::~ZWorldManager(void)
{
}

void ZWorldManager::Destroy()
{
	OnInvalidate();
	Clear();
}

void ZWorldManager::Clear()
{
	while(size()) {
		ZWorld *pWorld = back();
		pWorld->m_nRefCount--;
		if(pWorld->m_nRefCount==0)
		{
			m_Worlds.erase(m_Worlds.find(pWorld));
			delete pWorld;
		}
		pop_back();
	}
}

void ZWorldManager::AddWorld(const char* szMapName)
{
	for (iterator i = begin(); i!=end(); i++) {
		ZWorld *pWorld = *i;
		if(strcmp(pWorld->m_szName,szMapName)==0) {
			// 이미 있다
			pWorld->m_nRefCount++;
			push_back(pWorld);
			return;
		}
	}

	char szMapFileName[256];
	char szMapPath[64] = "";
	ZGetCurrMapPath(szMapPath);
	sprintf_safe(szMapFileName, "%s%s/%s.rs", szMapPath,szMapName,szMapName);

	ZWorld *pWorld = new ZWorld;
	strcpy_safe(pWorld->m_szName,szMapName);
	strcpy_safe(pWorld->m_szBspName,szMapFileName);
	push_back(pWorld);
	m_Worlds.insert(pWorld);
}

bool ZWorldManager::LoadAll(ZLoadingProgress *pLoading )
{
	int n=0;

	_ASSERT(m_Worlds.size()>0);
	for (set<ZWorld*>::iterator i = m_Worlds.begin(); i!=m_Worlds.end(); i++) {

		ZLoadingProgress loadingWorld("Map",pLoading,(float)1/(float)m_Worlds.size());

		ZWorld *pWorld = *i;
		if(!pWorld->Create(&loadingWorld))
			return false;
		loadingWorld.UpdateAndDraw(1.f);

		n++;
	}
	return true;
}


ZWorld *ZWorldManager::GetWorld(int i)
{
	if((int)size()<=i || i<0) return NULL;
	return at(i);
}

ZWorld *ZWorldManager::SetCurrent(int i)
{
	if((int)size()<=i || i<0) {
		_ASSERT(FALSE);
		return NULL;
	}

	m_nCurrent = i;

	ZWorld *pCurrent = GetCurrent();

	SetClearColor(pCurrent->m_bFog ? pCurrent->m_dwFogColor : 0);

	return pCurrent;
}

void ZWorldManager::OnInvalidate()
{
	ZWaterList::OnInvalidate();
	for (set<ZWorld*>::iterator i = m_Worlds.begin(); i!=m_Worlds.end(); i++) {
		ZWorld *pWorld = *i;
		pWorld->OnInvalidate();
	}
}

void ZWorldManager::OnRestore()
{
	ZWaterList::OnRestore();
	for (set<ZWorld*>::iterator i = m_Worlds.begin(); i!=m_Worlds.end(); i++) {
		ZWorld *pWorld = *i;
		pWorld->OnRestore();
	}
}
