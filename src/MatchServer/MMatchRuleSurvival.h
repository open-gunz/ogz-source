#ifndef _MMATCHRULE_SURVIVAL_H
#define _MMATCHRULE_SURVIVAL_H

#include "MMatchRule.h"
#include "MMatchRuleBaseQuest.h"
#include "MMatchNPCObject.h"
#include "MMatchQuestRound.h"

class MMatchRuleSurvival : public MMatchRuleBaseQuest {
protected:
	u64							m_nRountStartTime;
	MMatchQuestRound			m_QuestRound;
	bool						m_bReservedNextRound;
	u64							m_nReversedNextRoundTime;
protected:
	virtual void OnBegin();
	virtual void OnEnd();
	virtual bool OnRun();
//	virtual void OnRoundBegin();						// 라운드 시작할 때
//	virtual void OnRoundEnd();							// 라운드 끝날 때
//	virtual bool OnCheckRoundFinish();					// 라운드가 끝났는지 체크
//	virtual void OnRoundTimeOut();						// 라운드가 타임아웃으로 종료될 떄 OnRoundEnd() 전이다.
//	virtual bool RoundCount();							// 라운드 카운트. 모든 라운드가 끝나면 false를 반환한다.
//	virtual bool OnCheckEnableBattleCondition();		// 게임 가능한지 체크

	virtual void OnCommand(MCommand* pCommand);			// 퀘스트에서만 사용하는 커맨드 처리


	virtual void ProcessNPCSpawn();
	virtual bool CheckNPCSpawnEnable();					// NPC가 스폰 가능한지 여부
	virtual void RouteGameInfo();
	virtual void RouteStageGameInfo();			///< 대기중 스테이지에서 바뀐 게임 정보를 보내준다.
	virtual void RouteCompleted() {}
	virtual void RouteFailed() {}
	virtual void DistributeReward();


	void ProcessRound();
	void QuestRoundStart();
public:
	void RouteQuestRoundStart();
public:
	MMatchRuleSurvival(MMatchStage* pStage);
	virtual ~MMatchRuleSurvival();
	virtual MMATCH_GAMETYPE GetGameType() { return MMATCH_GAMETYPE_SURVIVAL; }
};











#endif