#include "stdafx.h"
#include <algorithm>
#include "ZGame.h"
#include "ZCharacterManager.h"
#include "ZMyCharacter.h"
#include "ZNetCharacter.h"
#include "MDebug.h"
#include "ZApplication.h"
#include "ZShadow.h"
#include "RCharCloth.h"
#include "ZObjectManager.h"

extern ZGame* g_pGame;
extern bool Enable_Cloth;

ZCharacterManager::ZCharacterManager()
{
}

ZCharacterManager::~ZCharacterManager()
{
	Clear();
}

void ZCharacterManager::Add(ZCharacter *pCharacter)
{	
	insert(value_type(pCharacter->GetUID(), pCharacter));
	ZGetObjectManager()->Add((ZObject*)pCharacter);
}

ZCharacter* ZCharacterManager::Add(MUID uid, rvector pos, bool bMyCharacter)
{	
	ZCharacter* pCharacter = Find(uid);
	if (pCharacter != NULL) return pCharacter;

	if (bMyCharacter) pCharacter = new ZMyCharacter;
	else pCharacter = new ZNetCharacter;

	pCharacter->SetUID(uid);
	pCharacter->m_Position= pos;

	Add(pCharacter);

	return pCharacter;
}

void ZCharacterManager::Delete(MUID uid)
{
	iterator itor = find(uid);

	if (itor != end()) {

		ZCharacter* pCharacter = (*itor).second;

		ZGetObjectManager()->Delete((ZObject*)pCharacter);

		g_pGame->m_VisualMeshMgr.Del(pCharacter->m_nVMID);
		delete pCharacter; pCharacter = NULL;
		erase(itor);
	}
}


void ZCharacterManager::Clear()
{
	while (!empty())
	{
		ZCharacter* pCharacter = (*begin()).second;

		if( g_pGame->m_VisualMeshMgr.m_list.size() )
			g_pGame->m_VisualMeshMgr.Del(pCharacter->m_nVMID);
		delete pCharacter;
		erase(begin());
	}

}


ZCharacter* ZCharacterManager::Find(MUID uid)
{
	iterator itor = find(uid);
	if (itor != end())
	{
		return (*itor).second;
	}
	return NULL;
}

void ZCharacterManager::OutputDebugString_CharacterState()
{
	ZMyCharacter* pMyChar = g_pGame->m_pMyCharacter;

	if(pMyChar) {
		pMyChar->OutputDebugString_CharacterState();
	}

	for(iterator i = begin(); i!=end(); i++) {
		ZCharacter* pCharacter = (*i).second;
		if(pCharacter&& (pCharacter!=pMyChar)) {
			pCharacter->OutputDebugString_CharacterState();
		}
	}
}

int ZCharacterManager::GetLiveCount()
{
	int nLiveCount = 0;
	for(iterator i = begin(); i!=end(); i++){
		ZCharacter* pCharacter = (*i).second;
		if(pCharacter->IsDie()==false) nLiveCount++;
	}

	return nLiveCount;
}

bool ZCharacterManager::ToggleClothSimulation()
{
	Enable_Cloth	= !Enable_Cloth;
	for( iterator i = begin(); i != end(); ++i )
	{
		ZCharacter* pCharacter = (*i).second ;
		if( pCharacter != 0 )
			pCharacter->ToggleClothSimulation();
	}
	return Enable_Cloth;
}

void ZCharacterManager::OnInvalidate()
{
	ZCharacter* pCharacter;
	
	for(iterator i = begin(); i!=end(); i++) {
		pCharacter = i->second;
		if( pCharacter && pCharacter->m_pVMesh ) 
			pCharacter->m_pVMesh->OnInvalidate();
	}
}

void ZCharacterManager::OnRestore()
{
	ZCharacter* pCharacter;
	
	for(iterator i = begin(); i!=end(); i++) {
		pCharacter = i->second;
		if( pCharacter && pCharacter->m_pVMesh ) 
			pCharacter->m_pVMesh->OnRestore();
	}
}

ZCharacter* ZCharacterManager::Get(int index)
{
	int nIndex = 0;

	for(iterator i = begin(); i!=end(); i++,nIndex++) {
		if(nIndex==index) return i->second;
	}

	return NULL;
}

void ZCharacterManager::InitRound()
{
	for(iterator i = begin(); i!=end(); i++) {
		ZCharacter* pCharacter = i->second;
		pCharacter->InitRound();
	}
}

int ZCharacterManager::GetCharacterIndex(const MUID& uid, bool bIncludeHiddenChar)
{
	int nIndex = 0;
	for(iterator i = begin(); i!=end(); i++) 
	{
		if (i->second->GetUID() == uid)
		{
			return nIndex;
		}
		if ((bIncludeHiddenChar) || (!i->second->IsAdminHide())) nIndex++;
	}

/*
	iterator itor = find(uid);
	if (itor != end())
	{
		return distance(begin(), itor);
	}
*/
	return -1;
}