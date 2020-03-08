#include "stdafx.h"
#include "MQuestPlayer.h"
#include "MMatchStage.h"
#include "MMatchObject.h"
#include "MMatchServer.h"
#include "MSharedCommandTable.h"
#include "MMatchRuleQuest.h"
#include "MQuestPlayer.h"
#include "MQuestFormula.h"


MQuestPlayerManager::MQuestPlayerManager() : m_pStage(0)
{

}

MQuestPlayerManager::~MQuestPlayerManager()
{
	
}

void MQuestPlayerManager::Create(MMatchStage* pStage)
{
	m_pStage = pStage;

	for (auto itorObj = pStage->GetObjBegin(); itorObj != pStage->GetObjEnd(); ++itorObj)
	{
		MUID uidChar = itorObj->first;

		MMatchObject* pObj = MMatchServer::GetInstance()->GetObject(uidChar);
		if (IsAdminGrade(pObj) && pObj->CheckPlayerFlags(MTD_PlayerFlags_AdminHide)) continue;
		

		AddPlayer(uidChar);
	}
}

void MQuestPlayerManager::Destroy()
{
	Clear();
}

void MQuestPlayerManager::AddPlayer(MUID& uidPlayer)
{
	MQuestPlayerInfo* pPlayerInfo = new MQuestPlayerInfo();

	MMatchObject* pPlayerObject = MMatchServer::GetInstance()->GetObject(uidPlayer);
	if (IsEnabledObject(pPlayerObject))
	{
		int nPlayerLevel = pPlayerObject->GetCharInfo()->m_nLevel;
		int nQL = MQuestFormula::CalcQL(nPlayerLevel);

		pPlayerInfo->Init(pPlayerObject, nQL);

		insert(value_type(uidPlayer, pPlayerInfo));
	}
}

void MQuestPlayerManager::DelPlayer(MUID& uidPlayer)
{
	MQuestPlayerInfo* pDelPlayerInfo = NULL;

	iterator itor = find(uidPlayer);
	if (itor != end())
	{
		pDelPlayerInfo = (*itor).second;
		delete pDelPlayerInfo;

		erase(itor);
	}
}

void MQuestPlayerManager::Clear()
{
	for (iterator itor = begin(); itor != end(); ++itor)
	{
		delete (*itor).second;
	}

	clear();
}

MQuestPlayerInfo* MQuestPlayerManager::GetPlayerInfo(const MUID& uidPlayer)
{
	iterator itor = find(uidPlayer);
	if (itor != end())
	{
		return (*itor).second;
	}
	return NULL;
}