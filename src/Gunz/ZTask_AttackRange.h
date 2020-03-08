#ifndef _ZTASK_ATTACK_RANGE_H
#define _ZTASK_ATTACK_RANGE_H

#include "ZTask.h"
#include "ZTaskBase_Attack.h"

class ZTask_AttackRange : public ZTaskBase_Attack
{
private:
	rvector			m_vDirection;
protected:
	virtual void OnStart();
	virtual ZTaskResult OnRun(float fDelta);
	virtual void OnComplete();
public:
	DECLARE_TASK_ID(ZTID_ATTACK_RANGE);

	ZTask_AttackRange(ZActor* pParent, rvector& dir);
	virtual ~ZTask_AttackRange();
	virtual const char* GetTaskName() { return "AttackRange"; }
};











#endif