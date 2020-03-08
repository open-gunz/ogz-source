#ifndef _ZBEHAVIOR_IDLE_H
#define _ZBEHAVIOR_IDLE_H

#include "ZBehavior.h"

class ZBehavior_Idle : public ZBehaviorState
{
protected:
	virtual void OnEnter();
	virtual void OnExit();
	virtual void OnRun(float fDelta);
public:
	ZBehavior_Idle(ZBrain* pBrain);
	virtual ~ZBehavior_Idle();
};


#endif