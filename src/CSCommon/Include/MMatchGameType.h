#pragma once

#include "MBaseGameType.h"

class MMatchGameTypeMgr : public MBaseGameTypeCatalogue
{
public:
	MMatchGameTypeMgr();
	virtual ~MMatchGameTypeMgr();
	static MMatchGameTypeMgr* GetInstance();
};

inline MMatchGameTypeMgr* MGetGameTypeMgr()
{
	return MMatchGameTypeMgr::GetInstance();
}