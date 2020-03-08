#ifndef _MMATCHRULE_BERSERKER_H
#define _MMATCHRULE_BERSERKER_H


#include "MMatchRule.h"
#include "MMatchRuleDeathMatch.h"


class MMatchRuleBerserker : public MMatchRuleSoloDeath {
protected:
	// 멤버변수 ---------------------
	MUID		m_uidBerserker;				// 현재 버서커인 플레이어

	// 함수 -------------------------
	bool CheckKillCount(MMatchObject* pOutObject);
	virtual void OnRoundBegin();
	virtual bool OnCheckRoundFinish();
	void RouteAssignBerserker();
	MUID RecommendBerserker();
public:
	MMatchRuleBerserker(MMatchStage* pStage);
	virtual ~MMatchRuleBerserker() { }
	virtual void* CreateRuleInfoBlob();
	virtual void OnEnterBattle(MUID& uidChar);			///< 게임중 난입할때 호출된다.
	virtual void OnLeaveBattle(MUID& uidChar);			///< 게임중 나갔을때 호출된다.
	virtual void OnGameKill(const MUID& uidAttacker, const MUID& uidVictim);
	virtual MMATCH_GAMETYPE GetGameType() { return MMATCH_GAMETYPE_BERSERKER; }
	MUID& GetBerserker() { return m_uidBerserker; }
};

#endif