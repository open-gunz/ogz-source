#ifndef _ZRULE_GLADIATOR_H
#define _ZRULE_GLADIATOR_H


#include "ZRule.h"
#include "ZRuleDeathMatch.h"


class ZRuleSoloGladiator : public ZRuleSoloDeathMatch
{
public:
	ZRuleSoloGladiator(ZMatch* pMatch);
	virtual ~ZRuleSoloGladiator();
};


class ZRuleTeamGladiator : public ZRuleTeamDeathMatch
{
public:
	ZRuleTeamGladiator(ZMatch* pMatch);
	virtual ~ZRuleTeamGladiator();
};



#endif