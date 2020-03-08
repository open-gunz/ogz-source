#pragma once

#include "MBaseChannelRule.h"

class MMatchChannelRuleMgr : public MChannelRuleMgr
{
public:
	static MMatchChannelRuleMgr* GetInstance();
};


inline MMatchChannelRuleMgr* MGetChannelRuleMgr() 
{
	return MMatchChannelRuleMgr::GetInstance();
}