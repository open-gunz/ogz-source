#ifndef _ZRULE_TRAINING_H
#define _ZRULE_TRAINING_H


#include "ZRule.h"
#include "ZRuleDeathMatch.h"


class ZRuleTraining : public ZRuleSoloDeathMatch
{
public:
	ZRuleTraining(ZMatch* pMatch);
	virtual ~ZRuleTraining();
};





#endif