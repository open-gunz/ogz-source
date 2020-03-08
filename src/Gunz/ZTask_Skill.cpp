#include "stdafx.h"
#include "ZTask_Skill.h"
#include "ZModule_Skills.h"
#include "ZGame.h"

ZTask_Skill::ZTask_Skill(ZActor* pParent,int nSkill,MUID& uidTarget,rvector& targetPosition) : ZTaskBase_Attack(pParent),
					m_nSkill(nSkill), m_uidTarget(uidTarget), m_TargetPosition(targetPosition), m_pSkillDesc(NULL)
{
}

ZTask_Skill::~ZTask_Skill()
{

}


void ZTask_Skill::OnStart()
{
	m_pParent->Skill(m_nSkill);
	ZPostNPCSkillStart(m_pParent->GetUID(),m_nSkill,m_uidTarget,m_TargetPosition);

//	m_pParent->Attack_Range(m_vDirection);

	m_fStartTime = g_pGame->GetTime(); 
	m_bExecuted = false;

	ZModule_Skills *pmod = (ZModule_Skills *)m_pParent->GetModule(ZMID_SKILLS);
	_ASSERT(pmod!=NULL);
	m_pSkillDesc = pmod->GetSkill(m_nSkill)->GetDesc();

	//pmod->PreExcute(m_nSkill,m_uidTarget,m_TargetPosition);

	//_ASSERT(m_pSkillDesc!=NULL);
}

ZTaskResult ZTask_Skill::OnRun(float fDelta)
{
	float fStartTime =  g_pGame->GetTime()-m_fStartTime;
	if(!m_bExecuted && fStartTime > .001f*m_pSkillDesc->nEffectStartTime) {
	//if(!m_bExecuted && g_pGame->GetTime()-m_fStartTime > .001f*m_pSkillDesc->nEffectStartTime) {
		m_bExecuted = true;

		ZPostNPCSkillExecute(m_pParent->GetUID(),m_nSkill,m_uidTarget,m_TargetPosition);

		//ZModule_Skills *pmod = (ZModule_Skills *)m_pParent->GetModule(ZMID_SKILLS);
		//pmod->Excute(m_nSkill,m_uidTarget,m_TargetPosition);
	}

	if ( (m_bExecuted==false) || 
		 (m_pParent->GetCurrAni() == ZA_ANIM_SPECIAL1) || 
		 (m_pParent->GetCurrAni() == ZA_ANIM_SPECIAL2) ||
		 (m_pParent->GetCurrAni() == ZA_ANIM_SPECIAL3) ||
		 (m_pParent->GetCurrAni() == ZA_ANIM_SPECIAL4))
		return ZTR_RUNNING;


	return ZTR_COMPLETED;
}

void ZTask_Skill::OnComplete()
{
	//ZModule_Skills *pmod = (ZModule_Skills *)m_pParent->GetModule(ZMID_SKILLS);
	//_ASSERT(pmod!=NULL);
	//pmod->LastExcute(m_nSkill,m_uidTarget,m_TargetPosition);
}


bool ZTask_Skill::OnCancel()
{
	return false;
}