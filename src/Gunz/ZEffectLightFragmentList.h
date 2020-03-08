#ifndef _ZEFFECTLIGHTFRAGMENTLIST_H
#define _ZEFFECTLIGHTFRAGMENTLIST_H

//#pragma once

#include "ZEffectBillboardList.h"

class ZEffectLightFragmentList : public ZEffectBillboardList
{
public:
	ZEffectLightFragmentList(void);
	~ZEffectLightFragmentList(void);

	virtual void BeginState();
	virtual void Update(float fElapsed);
};


#endif