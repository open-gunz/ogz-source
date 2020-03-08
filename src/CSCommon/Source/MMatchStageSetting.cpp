#include "stdafx.h"
#include "MMatchStageSetting.h"

u32 MMatchStageSetting::GetChecksum()
{
	return (m_StageSetting.nMapIndex + m_StageSetting.nGameType + m_StageSetting.nMaxPlayers);
}

void MMatchStageSetting::SetDefault()
{
	m_StageSetting.SetDefaults();
}

void MMatchStageSetting::SetMapName(const char* pszName)
{ 
	strcpy_safe(m_StageSetting.szMapName, pszName); 

	m_StageSetting.nMapIndex = 0;
	for (int i = 0; i < MMATCH_MAP_MAX; i++)
	{
		if (!_stricmp(g_MapDesc[i].szMapName, pszName))
		{
			m_StageSetting.nMapIndex = g_MapDesc[i].nMapID;
			break;
		}
	}
}

void MMatchStageSetting::SetMapIndex(int nMapIndex)
{
	m_StageSetting.nMapIndex = nMapIndex; 

	if (MIsCorrectMap(nMapIndex))
	{
		strcpy_safe(m_StageSetting.szMapName, g_MapDesc[nMapIndex].szMapName); 
	}
}

void MMatchStageSetting::Clear()
{
	SetDefault();
	m_CharSettingList.clear();
	m_uidMaster = MUID(0,0);
	m_nStageState = STAGE_STATE_STANDBY;

}

MSTAGE_CHAR_SETTING_NODE* MMatchStageSetting::FindCharSetting(const MUID& uid)
{
	for (auto&& node : m_CharSettingList)
	{
		if (uid == node.uidChar)
			return &node;
	}
	return nullptr;
}

bool MMatchStageSetting::IsTeamPlay()
{
	return MGetGameTypeMgr()->IsTeamGame(m_StageSetting.nGameType);
}

bool MMatchStageSetting::IsWaitforRoundEnd()
{
	return MGetGameTypeMgr()->IsWaitForRoundEnd(m_StageSetting.nGameType);
}

bool MMatchStageSetting::IsQuestDrived()
{
	return MGetGameTypeMgr()->IsQuestDerived(m_StageSetting.nGameType);
}

void MMatchStageSetting::UpdateStageSetting(MSTAGE_SETTING_NODE* pSetting)
{
	m_StageSetting = *pSetting;
}

void MMatchStageSetting::UpdateCharSetting(const MUID& uid, unsigned int nTeam, MMatchObjectStageState nStageState)
{
	auto pNode = FindCharSetting(uid);
	if (pNode) {
		pNode->nTeam = nTeam;
		pNode->nState = nStageState;
	} else {
		MSTAGE_CHAR_SETTING_NODE NewNode;
		NewNode.uidChar = uid;
		NewNode.nTeam = nTeam;
		NewNode.nState = nStageState;
		m_CharSettingList.push_back(NewNode);
	}			
}

const MMatchGameTypeInfo* MMatchStageSetting::GetCurrGameTypeInfo()
{ 
	return MGetGameTypeMgr()->GetInfo(m_StageSetting.nGameType); 
}
