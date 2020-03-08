#ifndef _MMATCHRULE_GLADIATOR_H
#define _MMATCHRULE_GLADIATOR_H


#include "MMatchRuleDeathMatch.h"

///////////////////////////////////////////////////////////////////////////////////////////////
class MMatchRuleSoloGladiator : public MMatchRuleSoloDeath  {
public:
	MMatchRuleSoloGladiator(MMatchStage* pStage);
	virtual ~MMatchRuleSoloGladiator() { }
	virtual MMATCH_GAMETYPE GetGameType() { return MMATCH_GAMETYPE_GLADIATOR_SOLO; }
};

///////////////////////////////////////////////////////////////////////////////////////////////
class MMatchRuleTeamGladiator : public MMatchRuleTeamDeath {
public:
	MMatchRuleTeamGladiator(MMatchStage* pStage);
	virtual ~MMatchRuleTeamGladiator()				{}
	virtual MMATCH_GAMETYPE GetGameType() { return MMATCH_GAMETYPE_GLADIATOR_TEAM; }
};




#endif