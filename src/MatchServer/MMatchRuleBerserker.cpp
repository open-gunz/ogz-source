#include "stdafx.h"
#include "MMatchRuleBerserker.h"
#include "MMatchTransDataType.h"
#include "MBlobArray.h"
#include "MMatchServer.h"

//////////////////////////////////////////////////////////////////////////////////
// MMatchRuleBerserker ///////////////////////////////////////////////////////////
MMatchRuleBerserker::MMatchRuleBerserker(MMatchStage* pStage) : MMatchRuleSoloDeath(pStage), m_uidBerserker(0,0)
{

}

bool MMatchRuleBerserker::OnCheckRoundFinish()
{
	return MMatchRuleSoloDeath::OnCheckRoundFinish();
}

void MMatchRuleBerserker::OnRoundBegin()
{
	m_uidBerserker = MUID(0,0);
}

void* MMatchRuleBerserker::CreateRuleInfoBlob()
{
	void* pRuleInfoArray = MMakeBlobArray(sizeof(MTD_RuleInfo_Berserker), 1);
	MTD_RuleInfo_Berserker* pRuleItem = (MTD_RuleInfo_Berserker*)MGetBlobArrayElement(pRuleInfoArray, 0);
	memset(pRuleItem, 0, sizeof(MTD_RuleInfo_Berserker));
	
	pRuleItem->nRuleType = MMATCH_GAMETYPE_BERSERKER;
	pRuleItem->uidBerserker = m_uidBerserker;

	return pRuleInfoArray;
}

void MMatchRuleBerserker::RouteAssignBerserker()
{	MCommand* pNew = MMatchServer::GetInstance()->CreateCommand(MC_MATCH_ASSIGN_BERSERKER, MUID(0, 0));
	pNew->AddParameter(new MCmdParamUID(m_uidBerserker));
	MMatchServer::GetInstance()->RouteToBattle(m_pStage->GetUID(), pNew);
}


MUID MMatchRuleBerserker::RecommendBerserker()
{
	MMatchStage* pStage = GetStage();
	if (pStage == NULL) return MUID(0,0);

	int nCount = 0;
	for(auto itor=pStage->GetObjBegin(); itor!=pStage->GetObjEnd(); itor++) {
		MMatchObject* pObj = itor->second;
		if (pObj->GetEnterBattle() == false) continue;	// 배틀참가하고 있는 플레이어만 체크
		if (pObj->CheckAlive())
		{
			return pObj->GetUID();
		}
	}
	return MUID(0,0);

}


void MMatchRuleBerserker::OnEnterBattle(MUID& uidChar)
{
}

void MMatchRuleBerserker::OnLeaveBattle(MUID& uidChar)
{
	if (uidChar == m_uidBerserker)
	{
		m_uidBerserker = MUID(0,0);
		RouteAssignBerserker();
	}
}

void MMatchRuleBerserker::OnGameKill(const MUID& uidAttacker, const MUID& uidVictim)
{
	// 희생자가 버서커이거나 현재 버서커가 한명도 없을때
	if ((m_uidBerserker == uidVictim) || (m_uidBerserker == MUID(0,0)))
	{
		bool bAttackerCanBeBerserker = false;

		 // 공격자가 자신이 아닐 경우
		if (uidAttacker != uidVictim)
		{
			MMatchObject* pAttacker = MMatchServer::GetInstance()->GetObject(uidAttacker);

			// 공격자가 죽어있으면 버서커가 될 수 없다(러브샷)
			if ((pAttacker) && (pAttacker->CheckAlive()))
			{
				bAttackerCanBeBerserker = true;
			}
		}
		// 공격자가 자신일 경우 버서커는 아무도 되지 않는다.
		else if ((uidAttacker == MUID(0,0)) || (uidAttacker == uidVictim))
		{
			bAttackerCanBeBerserker = false;
		}

		if (bAttackerCanBeBerserker) m_uidBerserker = uidAttacker;
		else m_uidBerserker = MUID(0,0);

		RouteAssignBerserker();
	}
}

