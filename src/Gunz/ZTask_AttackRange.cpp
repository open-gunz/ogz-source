#include "stdafx.h"
#include "ZTask_AttackRange.h"

ZTask_AttackRange::ZTask_AttackRange(ZActor* pParent, rvector& dir) : ZTaskBase_Attack(pParent), m_vDirection(dir)
{

}

ZTask_AttackRange::~ZTask_AttackRange()
{

}


void ZTask_AttackRange::OnStart()
{
	m_pParent->Attack_Range(m_vDirection);
}

ZTaskResult ZTask_AttackRange::OnRun(float fDelta)
{
	if (m_pParent->GetCurrAni() == ZA_ANIM_ATTACK_RANGE)
		return ZTR_RUNNING;
	return ZTR_COMPLETED;
}

void ZTask_AttackRange::OnComplete()
{

}

