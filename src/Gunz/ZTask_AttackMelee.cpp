#include "stdafx.h"
#include "ZTask_AttackMelee.h"

ZTask_AttackMelee::ZTask_AttackMelee(ZActor* pParent) : ZTaskBase_Attack(pParent)
{

}

ZTask_AttackMelee::~ZTask_AttackMelee()
{

}


void ZTask_AttackMelee::OnStart()
{
	m_pParent->Attack_Melee();
}

ZTaskResult ZTask_AttackMelee::OnRun(float fDelta)
{
	if (m_pParent->GetCurrAni() == ZA_ANIM_ATTACK_MELEE) return ZTR_RUNNING;
	return ZTR_COMPLETED;
}

void ZTask_AttackMelee::OnComplete()
{

}

