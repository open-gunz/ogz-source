#ifndef _ZCHANNELRULE_H
#define _ZCHANNELRULE_H

#include "MBaseChannelRule.h"

class ZChannelRuleMgr : public MChannelRuleMgr
{
public:
	ZChannelRuleMgr();
	virtual ~ZChannelRuleMgr();
	static ZChannelRuleMgr* GetInstance();
	MChannelRule* GetCurrentRule();
};
inline ZChannelRuleMgr* ZGetChannelRuleMgr() { return ZChannelRuleMgr::GetInstance(); }

#endif