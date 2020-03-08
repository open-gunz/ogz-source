#ifndef _ZEFFECTBULLETMARKLIST_H
#define _ZEFFECTBULLETMARKLIST_H

//#pragma once

#include "ZEffectBase.h"
#include "ZEffectBillboardList.h"

#include "mempool.h"

struct ZEFFECTBULLETMARKITEM : public ZEFFECTITEM , public CMemPoolSm<ZEFFECTBULLETMARKITEM>{
	ZEFFECTCUSTOMVERTEX v[4];
};

class ZEffectBulletMarkList : public ZEffectBase
{
public:
	ZEffectBulletMarkList(void);

	void Add(const rvector &pos, const rvector &normal);

	virtual void BeginState();
	virtual void Update(float fElapsed);
	virtual bool Draw();
};


#endif