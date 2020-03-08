#ifndef _ZRULE_QUEST_H
#define _ZRULE_QUEST_H



#include "ZRule.h"
#include "ZRuleBaseQuest.h"
#include "MQuestConst.h"

class ZRuleQuest : public ZRuleBaseQuest
{
protected:
	MQuestCombatState		m_nCombatState;
public:
	ZRuleQuest(ZMatch* pMatch);
	virtual ~ZRuleQuest();

	MQuestCombatState GetCombatState() { return m_nCombatState; }
};







#endif