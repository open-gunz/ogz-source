#ifndef _ZBEHAVIOR_ATTACK_H
#define _ZBEHAVIOR_ATTACK_H

#include "ZBehavior.h"

class ZBehavior_Attack : public ZBehaviorState
{
protected:
	virtual void OnEnter();
	virtual void OnExit();
	virtual void OnRun(float fDelta);
public:
	ZBehavior_Attack(ZBrain* pBrain);
	virtual ~ZBehavior_Attack();
};








#endif