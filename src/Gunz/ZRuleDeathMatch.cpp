#include "stdafx.h"
#include "ZRuleDeathMatch.h"

ZRuleSoloDeathMatch::ZRuleSoloDeathMatch(ZMatch* pMatch) : ZRule(pMatch)
{

}

ZRuleSoloDeathMatch::~ZRuleSoloDeathMatch()
{





}
/////////////////////////////////////////////////////////////////////////////////////////

ZRuleTeamDeathMatch::ZRuleTeamDeathMatch(ZMatch* pMatch) : ZRule(pMatch)
{

}

ZRuleTeamDeathMatch::~ZRuleTeamDeathMatch()
{

}


/////////////////////////////////////////////////////////////////////////////////////////

ZRuleTeamDeathMatch2::ZRuleTeamDeathMatch2(ZMatch* pMatch) : ZRule(pMatch)
{

}

ZRuleTeamDeathMatch2::~ZRuleTeamDeathMatch2()
{

}

bool ZRuleTeamDeathMatch2::OnCommand(MCommand* pCommand)
{
	if (!g_pGame) return false;

	switch (pCommand->GetID())
	{

	case MC_MATCH_GAME_DEAD:
		{
			MUID uidAttacker, uidVictim;

			pCommand->GetParameter(&uidAttacker, 0, MPT_UID);
			pCommand->GetParameter(&uidVictim, 2, MPT_UID);

			ZCharacter* pAttacker = g_pGame->m_CharacterManager.Find(uidAttacker);
			ZCharacter* pVictim = g_pGame->m_CharacterManager.Find(uidVictim);
			
			m_pMatch->AddTeamKills(pVictim->GetTeamID() == MMT_BLUE ? MMT_RED : MMT_BLUE);
		}
		break;
	}

	return false;
}
