#pragma once

#include "ZActorAnimation.h"
#include "ZTask.h"
#include "ZTaskManager.h"
#include "ZBehavior.h"
#include "ZTimer.h"

class ZActor;
class ZBrain_GoblinKing;

class ZBrain
{
	friend ZActor;
	friend ZActorAnimation;
private:
	virtual void OnBody_AnimEnter(ZA_ANIM_STATE nAnimState);
	virtual void OnBody_AnimExit(ZA_ANIM_STATE nAnimState);
	virtual void OnBody_CollisionWall();
	virtual void OnBody_OnTaskFinished(ZTASK_ID nLastID);

	ZUpdateTimer		m_PathFindingTimer;
	ZUpdateTimer		m_AttackTimer;
	ZUpdateTimer		m_DefaultAttackTimer;
protected:
	ZActor*				m_pBody;
	ZBehavior			m_Behavior;
	MUID				m_uidTarget;
	
	std::list<rvector>		m_WayPointList;
	bool BuildPath(rvector& vTarPos);
	void DrawDebugPath();
	void PushPathTask();

	
	MQUEST_NPC_ATTACK CheckAttackable();
	
	bool CheckSkillUsable(int *pnSkill, MUID *pTarget, rvector *pTargetPosition);

	bool FindTarget();
	void ProcessAttack(float fDelta);
	void ProcessBuildPath(float fDelta);
	void DefaultAttack(MQUEST_NPC_ATTACK nNpcAttackType);
	void UseSkill(int nSkill, MUID& uidTarget, rvector& vTargetPos);

	float MakePathFindingUpdateTime(char nIntelligence);
	float MakeAttackUpdateTime(char nAgility);
	float MakeDefaultAttackCoolTime();

	virtual bool CheckEnableTargetting(ZCharacter* pCharacter);
public:
	ZBrain();
	virtual ~ZBrain();
	void Init(ZActor* pBody);
	void Think(float fDelta);
	ZActor* GetBody()		{ return m_pBody; }
	ZObject* GetTarget();
	void DebugTest();

	static float MakeSpeed(float fSpeed);
	static ZBrain* CreateBrain(MQUEST_NPC nNPCType);
};



class ZBrain_GoblinKing : public ZBrain
{
private:
protected:
public:
	ZBrain_GoblinKing();
	virtual ~ZBrain_GoblinKing();
};