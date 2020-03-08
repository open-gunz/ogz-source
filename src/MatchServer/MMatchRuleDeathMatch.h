#ifndef _MMATCHRULE_DEATHMATCH_H
#define _MMATCHRULE_DEATHMATCH_H


#include "MMatchRule.h"


class MMatchRuleSoloDeath : public MMatchRule {
protected:
	bool CheckKillCount(MMatchObject* pOutObject);
	virtual void OnBegin();
	virtual void OnEnd();
	virtual void OnRoundTimeOut();
	virtual bool OnCheckRoundFinish();
	virtual bool RoundCount();
public:
	MMatchRuleSoloDeath(MMatchStage* pStage);
	virtual ~MMatchRuleSoloDeath() { }
	virtual MMATCH_GAMETYPE GetGameType() { return MMATCH_GAMETYPE_DEATHMATCH_SOLO; }
};

///////////////////////////////////////////////////////////////////////////////////////////////
class MMatchRuleTeamDeath : public MMatchRule {
protected:
	bool GetAliveCount(int* pRedAliveCount, int* pBlueAliveCount);
	virtual void OnBegin();
	virtual void OnEnd();
	virtual bool OnRun();
	virtual void OnRoundBegin();
	virtual void OnRoundEnd();
	virtual bool OnCheckRoundFinish();
	virtual void OnRoundTimeOut();
	virtual bool RoundCount();
	virtual bool OnCheckEnableBattleCondition();
public:
	MMatchRuleTeamDeath(MMatchStage* pStage);
	virtual ~MMatchRuleTeamDeath()				{}
	virtual void CalcTeamBonus(MMatchObject* pAttacker, MMatchObject* pVictim,
								int nSrcExp, int* poutAttackerExp, int* poutTeamExp);
	virtual MMATCH_GAMETYPE GetGameType() { return MMATCH_GAMETYPE_DEATHMATCH_TEAM; }
};


// Ãß°¡ by µ¿¼·
///////////////////////////////////////////////////////////////////////////////////////////////
class MMatchRuleTeamDeath2 : public MMatchRule {
protected:
	void GetTeamScore(int* pRedTeamScore, int* pBLueTeamScore);
	virtual void OnBegin();
	virtual void OnEnd();
	virtual bool OnRun();
	virtual void OnRoundBegin();
	virtual void OnRoundEnd();
	virtual bool OnCheckRoundFinish();
	virtual void OnRoundTimeOut();
	virtual bool RoundCount();
	virtual void OnGameKill(const MUID& uidAttacker, const MUID& uidVictim);

public:
	MMatchRuleTeamDeath2(MMatchStage* pStage);
	virtual ~MMatchRuleTeamDeath2()				{}
	virtual void CalcTeamBonus(MMatchObject* pAttacker, MMatchObject* pVictim,
		int nSrcExp, int* poutAttackerExp, int* poutTeamExp);
	virtual MMATCH_GAMETYPE GetGameType() { return MMATCH_GAMETYPE_DEATHMATCH_TEAM2; }
};



#endif