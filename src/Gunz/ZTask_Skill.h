#ifndef _ZTASK_SKILL_H
#define _ZTASK_SKILL_H

#include "ZTask.h"
#include "ZTaskBase_Attack.h"

class ZTask_Skill : public ZTaskBase_Attack
{
	int		m_nSkill;
	float	m_fStartTime;
	bool	m_bExecuted;
	ZSkillDesc *m_pSkillDesc;
	MUID	m_uidTarget;
	rvector m_TargetPosition;
protected:
	virtual void OnStart();
	virtual ZTaskResult OnRun(float fDelta);
	virtual void OnComplete();
	virtual bool OnCancel();
public:
	DECLARE_TASK_ID(ZTID_SKILL);

	ZTask_Skill(ZActor* pParent,int nSkill,MUID& uidTarget,rvector& targetPosition);
	virtual ~ZTask_Skill();
	virtual const char* GetTaskName() { return "Skill"; }
};

#endif