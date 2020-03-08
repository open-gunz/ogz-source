#include "stdafx.h"
#include "ZModule_Skills.h"

void ZModule_Skills::Init(int nSkills, const int *pSkillIDs)
{
	m_nSkillCount = min(nSkills,MAX_SKILL);

	for(int i=0;i<m_nSkillCount;i++)
	{
		m_Skills[i].Init(pSkillIDs[i],(ZObject*)m_pContainer);
	}
}

void ZModule_Skills::InitStatus()
{
	for(int i=0;i<m_nSkillCount;i++)
	{
		m_Skills[i].InitStatus();
	}
}

void ZModule_Skills::OnUpdate(float fElapsed)
{
	for (int i = 0; i < m_nSkillCount; i++)
	{
		Active |= m_Skills[i].Update(fElapsed);
	}
}

int ZModule_Skills::GetSkillCount()
{
	return m_nSkillCount;
}

ZSkill *ZModule_Skills::GetSkill(int nSkill)
{
	if(nSkill<0 || nSkill>=MAX_SKILL) return 0;
	return &m_Skills[nSkill];
}

void ZModule_Skills::PreExcute(int nSkill,MUID uidTarget,rvector targetPosition)
{
	if(nSkill<0 || nSkill>=MAX_SKILL) return;
	m_Skills[nSkill].PreExecute(uidTarget,targetPosition);
}

void ZModule_Skills::Excute(int nSkill,MUID uidTarget,rvector targetPosition)
{
	if(nSkill<0 || nSkill>=MAX_SKILL) return;
	m_Skills[nSkill].Execute(uidTarget,targetPosition);
	
	Active = true;
}

void ZModule_Skills::LastExcute(int nSkill,MUID uidTarget,rvector targetPosition)
{
	if(nSkill<0 || nSkill>=MAX_SKILL) return;
	m_Skills[nSkill].LastExecute(uidTarget,targetPosition);
}
