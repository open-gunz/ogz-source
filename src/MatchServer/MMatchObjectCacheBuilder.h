#pragma once

#include "MMatchObjCache.h"

class MMatchObjectCacheBuilder {
	MMatchObjCacheList	m_ObjectCacheList;

public:
	MMatchObjectCacheBuilder();
	virtual ~MMatchObjectCacheBuilder();

	void AddObject(MMatchObject* pObj);
	void Reset();
	MCommand* GetResultCmd(MATCHCACHEMODE nType, MCommandCommunicator* pCmdComm);
};