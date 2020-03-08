#include "stdafx.h"
#include "MXml.h"
#include "MObject.h"

IMPLEMENT_RTTI(MObject)

MObjectCacheNode* MObjectCache::FindCacheNode(MObject* pObj)
{
	for(iterator i=begin(); i!=end(); i++){
		MObjectCacheNode* pNode = *i;
		if (pNode->m_pObject == pObj)
			return pNode;
	}
	return NULL;
}

void MObjectCache::Invalidate()
{
	m_nUpdateCount = 0;
	for(iterator i=begin(); i!=end(); i++){
		MObjectCacheNode* pNode = *i;
		pNode->m_CacheState = OBJECTCACHESTATE_EXPIRE;
		++m_nUpdateCount;
	}
}

void MObjectCache::Update(MObject* pObject)
{
	MObjectCacheNode* pFoundNode = NULL;
	for(iterator i=begin(); i!=end(); i++){
		MObjectCacheNode* pNode = *i;
		if (pNode->m_pObject == pObject)
			pFoundNode = pNode;
	}

	if (pFoundNode) {
		pFoundNode->m_CacheState = OBJECTCACHESTATE_KEEP;
		--m_nUpdateCount;
	} else {
		MObjectCacheNode* pNewNode = new MObjectCacheNode;
		pNewNode->m_pObject = pObject;
		pNewNode->m_CacheState = OBJECTCACHESTATE_NEW;
		++m_nUpdateCount;
		push_back(pNewNode);
	}
}

void MObjectCache::RemoveExpired()
{
	for(iterator i=begin(); i!=end();){
		MObjectCacheNode* pNode = *i;
		if (pNode->m_CacheState == OBJECTCACHESTATE_EXPIRE)
			i = erase(i);
		else
			++i;
	}
}

MObject::MObject()
{
	m_UID = MUID::Invalid();
}

MObject::MObject(const MUID& uid)
{
	m_UID = uid;
}

const MUID MObject::GetUID(void) const
{
	return m_UID;
}

void MObject::AddCommListener(MUID ListenerUID)
{
	if (IsCommListener(ListenerUID)) return;
	m_CommListener.push_back(ListenerUID);
}

void MObject::RemoveCommListener(MUID ListenerUID)
{
	for (auto i=m_CommListener.begin(); i!=m_CommListener.end(); i++) {
		MUID uid = *i;
		if (uid == ListenerUID) {
			m_CommListener.erase(i);
			return;
		}
	}
}

bool MObject::IsCommListener(MUID ListenerUID)
{
	for (auto i=m_CommListener.begin(); i!=m_CommListener.end(); i++) {
		MUID uid = *i;
		if (uid == ListenerUID)
			return true;
	}
	return false;
}