#ifndef _ZBEHAVIOR_PATROL_H
#define _ZBEHAVIOR_PATROL_H

#include "ZBehavior.h"

class ZBehavior_Patrol : public ZBehaviorState
{
protected:
	virtual void OnEnter();
	virtual void OnExit();
	virtual void OnRun(float fDelta);
public:
	ZBehavior_Patrol(ZBrain* pBrain);
	virtual ~ZBehavior_Patrol();
};


#endif