#ifndef _ZRULE_BERSERKER_H
#define _ZRULE_BERSERKER_H

#include "ZRule.h"



class ZRuleBerserker : public ZRule
{
private:
	MUID	m_uidBerserker;
	float	m_fElapsedHealthUpdateTime;
	void AssignBerserker(MUID& uidBerserker);
	virtual void OnUpdate(float fDelta);
	void BonusHealth(ZCharacter* pBerserker);
	void PenaltyHealth(ZCharacter* pBerserker);
public:
	ZRuleBerserker(ZMatch* pMatch);
	virtual ~ZRuleBerserker();
	virtual bool OnCommand(MCommand* pCommand);
	virtual void OnResponseRuleInfo(MTD_RuleInfo* pInfo);
	MUID GetBerserkerUID() const { return m_uidBerserker; }
};

#define BERSERKER_DAMAGE_RATIO			2.0f		// 버서커가 되면 파워가 2배로 된다.

#endif