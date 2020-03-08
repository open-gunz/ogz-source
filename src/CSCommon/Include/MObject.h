#pragma once

#include "MUID.h"
#include "MGridMap.h"
#include "MObjectTypes.h"
#include <algorithm>

class MMap;
class MObject;

typedef MGridMap<MObject*>	MObjectGridMap;

#define DECLARE_RTTI()								public: static const char* _m_RTTI_szClassName; virtual const char* GetClassName(void){ return _m_RTTI_szClassName; }
#define IMPLEMENT_RTTI(_Class)						const char* _Class::_m_RTTI_szClassName = #_Class;
#define ISEQUALCLASS(_Class, _InstancePointer)		(_Class::_m_RTTI_szClassName==(_InstancePointer)->GetClassName())

class MObject;
enum OBJECTCACHESTATE {	OBJECTCACHESTATE_KEEP, OBJECTCACHESTATE_NEW, OBJECTCACHESTATE_EXPIRE };
class MObjectCacheNode {
public:
	MObject*			m_pObject;
	OBJECTCACHESTATE	m_CacheState;
};
class MObjectCache : public std::list<MObjectCacheNode*> {
	int		m_nUpdateCount;
public:
	int GetUpdateCount() { return m_nUpdateCount; }
	MObjectCacheNode* FindCacheNode(MObject* pObj);
	void Invalidate();
	void Update(MObject* pObject);
	void RemoveExpired();
};

class MObject{
protected:
	MUID			m_UID;

	MObjectType		m_ObjectType;

public:
	MObjectCache	m_ObjectCache;
	std::list<MUID>	m_CommListener;

	MObject();
	MObject(const MUID& uid);
	virtual ~MObject(void)	{};

	const MUID GetUID(void) const;

	void SetObjectType(MObjectType type) { m_ObjectType = type; }
	MObjectType GetObjectType()	{ return m_ObjectType; }

	void AddCommListener(MUID ListenerUID);
	void RemoveCommListener(MUID ListenerUID);
	bool IsCommListener(MUID ListenerUID);
	bool HasCommListener() { if (m_CommListener.size() > 0) return true; else return false; }
	const MUID GetCommListener() { 
		if (HasCommListener())
			return *m_CommListener.begin(); 
		else
			return MUID(0,0);
	}

	void InvalidateObjectCache() { m_ObjectCache.Invalidate(); } 
	void UpdateObjectCache(MObject* pObject) { m_ObjectCache.Update(pObject); }
	void ExpireObjectCache(MObject* pObject) { 
		MObjectCacheNode* pNode = m_ObjectCache.FindCacheNode(pObject);
		if (pNode == NULL) return;
		pNode->m_CacheState = OBJECTCACHESTATE_EXPIRE;
	}
	void RemoveObjectCacheExpired() { m_ObjectCache.RemoveExpired(); }
	int GetObjectCacheUpdateCount() { return m_ObjectCache.GetUpdateCount(); }

	DECLARE_RTTI()
};