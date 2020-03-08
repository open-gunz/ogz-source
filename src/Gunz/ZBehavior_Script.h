#ifndef _ZBEHAVIOR_SCRIPT_H
#define _ZBEHAVIOR_SCRIPT_H

#include "ZBehavior.h"

class ZBehavior_Script : public ZBehaviorState
{
protected:
	virtual void OnEnter();
	virtual void OnExit();
	virtual void OnRun(float fDelta);
public:
	ZBehavior_Script(ZBrain* pBrain);
	virtual ~ZBehavior_Script();
};


#endif