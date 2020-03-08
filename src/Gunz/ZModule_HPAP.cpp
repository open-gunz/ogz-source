#include "stdafx.h"
#include "ZModule_HPAP.h"
#include "ZGame.h"
#include "ZApplication.h"

void ZModule_HPAP::SetHP(int nHP)
{
	nHP = min(max(0, nHP), int(m_nMaxHP));
	fHP = nHP;
}

void ZModule_HPAP::SetAP(int nAP) 
{ 
	nAP = min(max(0,nAP), int(m_nMaxAP));
	fAP = nAP;
}

void ZModule_HPAP::OnDamage(MUID uidAttacker,int damage, float fRatio)
{
	m_LastAttacker = uidAttacker;

#ifndef _PUBLISH
	if (CheckQuestCheet() == true) return;
#endif

	if(m_bRealDamage)
	{
		ZObject* pAttacker = ZGetObjectManager()->GetObject(uidAttacker);
		if ((pAttacker) && (!IsPlayerObject(pAttacker)))
		{
			ZActor* pActor = (ZActor*)pAttacker;
			damage = (int)(damage * (pActor->GetQL() * 0.2f + 1));
		}

		int nHPDamage = (int)((float)damage * fRatio);
		int nAPDamage = damage - nHPDamage;

		if ((GetAP() - nAPDamage) < 0)
		{
			nHPDamage += (nAPDamage - GetAP());
			nAPDamage -= (nAPDamage - GetAP());
		}

		SetHP(GetHP() - nHPDamage);
		SetAP(GetAP() - nAPDamage);
	}
}


void ZModule_HPAP::InitStatus()
{
	m_LastAttacker = MUID(0,0);
}

bool ZModule_HPAP::CheckQuestCheet()
{
#ifdef _PUBLISH
	return false;
#endif

	if (IsMyCharacter((ZObject*)m_pContainer))
	{
		if ((ZIsLaunchDevelop()) && (ZGetGameClient()->GetServerMode() == MSM_TEST))
		{
			if (ZGetGameTypeManager()->IsQuestDerived(ZGetGameClient()->GetMatchStageSetting()->GetGameType()))
			{
				if (ZGetQuest()->GetCheet(ZQUEST_CHEET_GOD) == true) return true;
			}
		}
	}

	return false;
}