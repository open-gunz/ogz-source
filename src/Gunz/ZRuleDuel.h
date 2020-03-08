#ifndef _ZRULE_DUEL_H
#define _ZRULE_DUEL_H

#include "ZRule.h"


class ZRuleDuel : public ZRule
{
public:
	MTD_DuelQueueInfo QInfo;

	ZRuleDuel(ZMatch* pMatch);
	virtual ~ZRuleDuel();

	virtual bool OnCommand(MCommand* pCommand);

	int	GetQueueIdx(const MUID& uidChar);			// 0 : 챔피언   1 : 도전자  2~ : 관전자
};

#endif