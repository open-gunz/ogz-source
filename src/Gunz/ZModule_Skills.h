#pragma once

#include "ZModule.h"
#include "ZModuleID.h"
#include "MQuestNPC.h"
#include "ZSkill.h"

class ZModule_Skills : public ZModule {
public:
	DECLARE_ID(ZMID_SKILLS)

	void Init(int nSkills, const int *pSkillIDs);

	void OnUpdate(float fElapsed);
	void InitStatus();

	int GetSkillCount();
	ZSkill *GetSkill(int nSkill);

	void PreExcute(int nSkill,MUID uidTarget,rvector targetPosition);
	void Excute(int nSkill,MUID uidTarget,rvector targetPosition);
	void LastExcute(int nSkill,MUID uidTarget,rvector targetPosition);

private:
	int			m_nSkillCount;
	ZSkill		m_Skills[MAX_SKILL];
};