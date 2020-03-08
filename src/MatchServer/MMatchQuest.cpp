#include "stdafx.h"
#include "MMatchQuest.h"

#define FILENAME_NPC_DESC		"npc.xml"
#define FILENAME_SCENARIO		"scenario.xml"
#define FILENAME_QUESTMAP		"questmap.xml"
#define FILENAME_NPCSET_DESC	"npcset.xml"
#define FILENAME_DROPTABLE		"droptable.xml"


MMatchQuest::MMatchQuest() : MBaseQuest()
{

}

MMatchQuest::~MMatchQuest()
{

}

bool MMatchQuest::OnCreate()
{
	if (!m_DropTable.ReadXml(FILENAME_DROPTABLE))
	{
		mlog("Droptable Read Failed");
		return false;
	}
	if (!m_NPCCatalogue.ReadXml(FILENAME_NPC_DESC))
	{
		mlog("Read NPC Catalogue Failed");
		return false;
	}

	ProcessNPCDropTableMatching();

	if (!m_NPCSetCatalogue.ReadXml(FILENAME_NPCSET_DESC))
	{
		mlog("Read NPCSet Catalogue Failed");
		return false;
	}

	if (!m_ScenarioCatalogue.ReadXml(FILENAME_SCENARIO))
	{
		mlog("Read Scenario Catalogue Failed");
		return false;
	}
	
	
	if (!m_MapCatalogue.ReadXml(FILENAME_QUESTMAP))
	{
		mlog("Read Questmap Catalogue Failed");
		return false;
	}

#ifdef _DEBUG
	m_MapCatalogue.DebugReport();
#endif

	return true;
}

void MMatchQuest::OnDestroy()
{

}

