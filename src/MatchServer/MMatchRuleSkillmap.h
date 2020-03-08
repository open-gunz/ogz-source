#pragma once

#include "MMatchRule.h"

class MMatchRuleSkillmap : public MMatchRule {
public:
	MMatchRuleSkillmap(MMatchStage* pStage);
	virtual ~MMatchRuleSkillmap() { }
	virtual MMATCH_GAMETYPE GetGameType() { return MMATCH_GAMETYPE_SKILLMAP; }
protected:
	virtual void OnBegin();
	virtual void OnEnd();
	virtual void OnRoundTimeOut();
	virtual bool OnCheckRoundFinish();
	virtual bool RoundCount();
};