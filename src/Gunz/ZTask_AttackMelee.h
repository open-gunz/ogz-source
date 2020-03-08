#ifndef _ZTASK_ATTACK_MELEE_H
#define _ZTASK_ATTACK_MELEE_H

#include "ZTask.h"
#include "ZTaskBase_Attack.h"

class ZTask_AttackMelee : public ZTaskBase_Attack
{
private:
protected:
	virtual void OnStart();
	virtual ZTaskResult OnRun(float fDelta);
	virtual void OnComplete();
public:
	DECLARE_TASK_ID(ZTID_ATTACK_MELEE);

	ZTask_AttackMelee(ZActor* pParent);
	virtual ~ZTask_AttackMelee();
	virtual const char* GetTaskName() { return "AttackMelee"; }
};




#endif