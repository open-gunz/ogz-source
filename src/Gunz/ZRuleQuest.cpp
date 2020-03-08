#include "stdafx.h"
#include "ZRuleQuest.h"
#include "ZMatch.h"

ZRuleQuest::ZRuleQuest(ZMatch* pMatch) : ZRuleBaseQuest(pMatch), m_nCombatState(MQUEST_COMBAT_PREPARE)
{

}

ZRuleQuest::~ZRuleQuest()
{

}
