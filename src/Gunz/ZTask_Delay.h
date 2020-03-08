#ifndef _ZTASK_DELAY_H
#define _ZTASK_DELAY_H

#include "ZTask.h"

class ZTask_Delay : public ZTask
{
private:

protected:
	virtual void OnStart();
	virtual ZTaskResult OnRun(float fDelta);
	virtual void OnComplete();
	virtual bool OnCancel();
public:
	DECLARE_TASK_ID(ZTID_DELAY);

	ZTask_Delay(ZActor* pParent);
	virtual ~ZTask_Delay();
	virtual const char* GetTaskName() { return "AttackDelay"; }
};


#endif