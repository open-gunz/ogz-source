#ifndef _ZTASK_ATTACK_H
#define _ZTASK_ATTACK_H


#include "ZTask.h"

class ZTaskBase_Attack : public ZTask
{
private:
protected:
	virtual bool OnCancel()
	{
		return false;
	}
public:
	ZTaskBase_Attack(ZActor* pParent) : ZTask(pParent) {}
	virtual ~ZTaskBase_Attack() {}
};


#endif