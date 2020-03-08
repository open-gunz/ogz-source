#include "stdafx.h"
#include "MMatchRuleSkillmap.h"

MMatchRuleSkillmap::MMatchRuleSkillmap(MMatchStage* pStage) : MMatchRule(pStage)
{

}

void MMatchRuleSkillmap::OnBegin()
{

}
void MMatchRuleSkillmap::OnEnd()
{
}

bool MMatchRuleSkillmap::RoundCount()
{
	if (++m_nRoundCount < 1) return true;
	return false;
}

bool MMatchRuleSkillmap::OnCheckRoundFinish()
{
	return false;
}

void MMatchRuleSkillmap::OnRoundTimeOut()
{
	SetRoundArg(MMATCH_ROUNDRESULT_DRAW);
}