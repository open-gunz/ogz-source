#ifndef _ZRULE_DEATH_MATCH_H
#define _ZRULE_DEATH_MATCH_H

#include "ZRule.h"


class ZRuleSoloDeathMatch : public ZRule
{
public:
	ZRuleSoloDeathMatch(ZMatch* pMatch);
	virtual ~ZRuleSoloDeathMatch();
};


class ZRuleTeamDeathMatch : public ZRule
{
public:
	ZRuleTeamDeathMatch(ZMatch* pMatch);
	virtual ~ZRuleTeamDeathMatch();
};


class ZRuleTeamDeathMatch2 : public ZRule
{
public:
	ZRuleTeamDeathMatch2(ZMatch* pMatch);
	virtual ~ZRuleTeamDeathMatch2();

	virtual bool OnCommand(MCommand* pCommand);
};



#endif