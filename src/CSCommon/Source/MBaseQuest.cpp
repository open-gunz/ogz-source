#include "stdafx.h"
#include "MBaseQuest.h"




MBaseQuest::MBaseQuest() : m_bCreated(false)
{

}

MBaseQuest::~MBaseQuest()
{
	if (m_bCreated) Destroy();
}

bool MBaseQuest::Create()
{
	if (m_bCreated) return true;

	bool ret = OnCreate();
	if (ret) m_bCreated = true;

	return ret;
}

void MBaseQuest::Destroy()
{
	OnDestroy();
	m_bCreated = false;
}

bool MBaseQuest::OnCreate()
{
	// 각각의 xml 로딩은 Client, Server 각각의 상속받은 클래스가 담당하도록 한다.

#ifdef _QUEST_ITEM
	return true;
#endif
	return false;
}

void MBaseQuest::OnDestroy()
{
}


void MBaseQuest::ProcessNPCDropTableMatching()
{
	for (MQuestNPCCatalogue::iterator itor = m_NPCCatalogue.begin(); itor != m_NPCCatalogue.end(); ++itor)
	{
		MQuestNPCInfo* pNPCInfo = (*itor).second;

		if (pNPCInfo->szDropTableName[0] != 0)
		{
			for (MQuestDropTable::iterator itorDT = m_DropTable.begin(); itorDT != m_DropTable.end(); ++itorDT)
			{
				MQuestDropSet* pDropSet = (*itorDT).second;
				if (!_stricmp(pDropSet->GetName(), pNPCInfo->szDropTableName))
				{
					pNPCInfo->nDropTableID = pDropSet->GetID();
					break;
				}
			}
		}
	}
}