#include "stdafx.h"
#include "ZRuleAssassinate.h"
#include "ZMatch.h"
#include "ZGame.h"
#include "ZGlobal.h"
#include "MMatchTransDataType.h"

ZRuleAssassinate::ZRuleAssassinate(ZMatch* pMatch) : ZRuleTeamDeathMatch(pMatch)
{

}

ZRuleAssassinate::~ZRuleAssassinate()
{

}

bool ZRuleAssassinate::OnCommand(MCommand* pCommand)
{
	if (!g_pGame) return false;

	switch (pCommand->GetID())
	{
	case MC_MATCH_ASSIGN_COMMANDER:
		{
			MUID uidRedCommander, uidBlueCommander;

			pCommand->GetParameter(&uidRedCommander,		0, MPT_UID);
			pCommand->GetParameter(&uidBlueCommander,		1, MPT_UID);

			AssignCommander(uidRedCommander, uidBlueCommander);
		}
		break;

	}

	return false;
}

void ZRuleAssassinate::OnResponseRuleInfo(MTD_RuleInfo* pInfo)
{
	MTD_RuleInfo_Assassinate* pAssassinateRule = (MTD_RuleInfo_Assassinate*)pInfo;
	AssignCommander(pAssassinateRule->uidRedCommander, pAssassinateRule->uidBlueCommander);
}

void ZRuleAssassinate::AssignCommander(const MUID& uidRedCommander, const MUID& uidBlueCommander)
{
	if (!g_pGame) return;

	ZCharacter* pRedChar = g_pGame->m_CharacterManager.Find(uidRedCommander);
	ZCharacter* pBlueChar = g_pGame->m_CharacterManager.Find(uidBlueCommander);

	if(pRedChar) {
		ZGetEffectManager()->AddCommanderIcon(pRedChar,0);
		pRedChar->m_bCommander = true;
	}
	if(pBlueChar) {
		ZGetEffectManager()->AddCommanderIcon(pBlueChar,1);
		pBlueChar->m_bCommander = true;
	}

#ifdef _DEBUG
	//// DEBUG LOG ////
	const char *szUnknown = "unknown";
	char szBuf[128];
	sprintf_safe(szBuf, "RedCMDER=%s , BlueCMDER=%s \n", 
		pRedChar ? pRedChar->GetProperty()->szName : szUnknown , 
		pBlueChar ? pBlueChar->GetProperty()->szName : szUnknown );
	OutputDebugString(szBuf);
	///////////////////
#endif
}
