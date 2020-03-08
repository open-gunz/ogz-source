#ifndef ZEFFECTBULLETMARK_H
#define ZEFFECTBULLETMARK_H

#include "ZEffectBillboard.h"

class ZEffectBulletMark : public ZEffectBillboard{
protected:
	u64 m_nStartTime;
public:
	ZEffectBulletMark(ZEffectBillboardSource* pEffectBillboardSource, const rvector& Pos, const rvector& Normal);

	virtual bool Draw(u64 nTime) override;
};

#endif
