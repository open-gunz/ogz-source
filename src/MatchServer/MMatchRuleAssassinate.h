#ifndef _MMATCHRULE_ASSASSINATE_H
#define _MMATCHRULE_ASSASSINATE_H


#include "MMatchRuleDeathMatch.h"

///////////////////////////////////////////////////////////////////////////////////////////////
class MMatchRuleAssassinate : public MMatchRuleTeamDeath {
private:
	MUID		m_uidRedCommander;
	MUID		m_uidBlueCommander;
private:
	const MUID ChooseCommander(int nTeam);
protected:
	virtual void OnRoundBegin();
	virtual void OnRoundEnd();
	virtual bool OnCheckRoundFinish();
public:
	MMatchRuleAssassinate(MMatchStage* pStage);
	virtual ~MMatchRuleAssassinate()				{}
	virtual void* CreateRuleInfoBlob();
	virtual void CalcTeamBonus(MMatchObject* pAttacker, MMatchObject* pVictim,
								int nSrcExp, int* poutAttackerExp, int* poutTeamExp);
	virtual MMATCH_GAMETYPE GetGameType() { return MMATCH_GAMETYPE_ASSASSINATE; }
};



#endif