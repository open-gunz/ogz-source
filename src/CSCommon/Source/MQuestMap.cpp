#include "stdafx.h"
#include "MQuestMap.h"
#include "MZFileSystem.h"
#include "MDebug.h"

/////////////////////////////////////////////////
MQuestMapCatalogue::MQuestMapCatalogue()
{

}

MQuestMapCatalogue::~MQuestMapCatalogue() 
{
	Clear();
}

MQuestMapsetInfo* MQuestMapCatalogue::GetMapsetInfo(int nMapset)
{
	MQuestMapsetMap::iterator itor = m_MapsetInfo.find(nMapset);
	if (itor != m_MapsetInfo.end())
	{
		return (*itor).second;
	}

	_ASSERT(0);
	return NULL;
}

MQuestMapSectorInfo* MQuestMapCatalogue::GetSectorInfo(int nSector)
{
	MQuestMapSectorMap::iterator itor = m_SectorInfo.find(nSector);
	if (itor != m_SectorInfo.end())
	{
		return (*itor).second;
	}

 	_ASSERT(0);
	return NULL;
}

MQuestMapSectorInfo* MQuestMapCatalogue::GetSectorInfoFromName(char* szSectorTitle)
{
	// sector
	for (MQuestMapSectorMap::iterator itor = m_SectorInfo.begin(); itor != m_SectorInfo.end(); ++itor)
	{
		MQuestMapSectorInfo* pSector = (*itor).second;
		if (!_stricmp(pSector->szTitle, szSectorTitle))
		{
			return pSector;
		}
	}

	return NULL;
}

void MQuestMapCatalogue::Clear()
{
	// mapset
	for (MQuestMapsetMap::iterator itor = m_MapsetInfo.begin(); itor != m_MapsetInfo.end(); ++itor)
	{
		delete (*itor).second;
	}

	m_MapsetInfo.clear();


	// sector
	for (MQuestMapSectorMap::iterator itor = m_SectorInfo.begin(); itor != m_SectorInfo.end(); ++itor)
	{
		delete (*itor).second;
	}

	m_SectorInfo.clear();
}



void MQuestMapCatalogue::InsertMapset(MQuestMapsetInfo* pMapset)
{
	int nID = pMapset->nID;

	MQuestMapsetMap::iterator itor = m_MapsetInfo.find(nID);
	if (itor != m_MapsetInfo.end())
	{
		// 이미 존재한다.
		_ASSERT(0);
		return;
	}

	m_MapsetInfo.insert(MQuestMapsetMap::value_type(nID, pMapset));
}

void MQuestMapCatalogue::InsertSector(MQuestMapSectorInfo* pSector)
{
	int nID = pSector->nID;

	MQuestMapSectorMap::iterator itor = m_SectorInfo.find(nID);
	if (itor != m_SectorInfo.end())
	{
		// 이미 존재한다.
		_ASSERT(0);
		return;
	}


	m_SectorInfo.insert(MQuestMapSectorMap::value_type(nID, pSector));
}

///////////////////////////////////////////////////////////////////////////////
#define MTOK_QUESTMAP_TAG_MAPSET					"MAPSET"
#define MTOK_QUESTMAP_TAG_SECTOR					"SECTOR"
#define MTOK_QUESTMAP_TAG_LINK						"LINK"
#define MTOK_QUESTMAP_TAG_TARGET					"TARGET"

#define MTOK_QUESTMAP_ATTR_ID						"id"
#define MTOK_QUESTMAP_ATTR_TITLE					"title"
#define MTOK_QUESTMAP_ATTR_SECTOR					"sector"
#define MTOK_QUESTMAP_ATTR_NAME						"name"
#define MTOK_QUESTMAP_ATTR_MELEE_SPAWN				"melee_spawn"
#define MTOK_QUESTMAP_ATTR_RANGE_SPAWN				"range_spawn"
#define MTOK_QUESTMAP_ATTR_BOSS_SPAWN				"boss_spawn"


void MQuestMapCatalogue::ParseMapset(MXmlElement& element)
{
	char szTemp[256]="";
	int n = 0;
	char szAttrValue[256];
	char szAttrName[64];
	char szTagName[128];


	MQuestMapsetInfo* pMapsetInfo = new MQuestMapsetInfo();
	int nAttrCount = element.GetAttributeCount();
	for (int i = 0; i < nAttrCount; i++)
	{
		element.GetAttribute(i, szAttrName, szAttrValue);
		if (!_stricmp(szAttrName, MTOK_QUESTMAP_ATTR_ID))
		{
			pMapsetInfo->nID = atoi(szAttrValue);
		}
		else if (!_stricmp(szAttrName, MTOK_QUESTMAP_ATTR_TITLE))
		{
			strcpy_safe(pMapsetInfo->szTitle, szAttrValue);
		}
	}

	// sector 목록을 미리 읽는다.
	ParseMapsetSector1Pass(element, pMapsetInfo);


	int nChildCount = element.GetChildNodeCount();
	MXmlElement chrElement;
	for (int i = 0; i < nChildCount; i++)
	{
		chrElement = element.GetChildNode(i);
		chrElement.GetTagName(szTagName);
		if (szTagName[0] == '#') continue;

		if (!_stricmp(szTagName, MTOK_QUESTMAP_TAG_SECTOR))
		{
			int nAttrCount = chrElement.GetAttributeCount();
			int nSectorID = -1;
			for (int j = 0; j < nAttrCount; j++)
			{
				chrElement.GetAttribute(j, szAttrName, szAttrValue);
				if (!_stricmp(szAttrName, MTOK_QUESTMAP_ATTR_ID))
				{
					nSectorID = atoi(szAttrValue);
					break;
				}
			}

			MQuestMapSectorInfo* pSector = GetSectorInfo(nSectorID);
			if (pSector)
			{
				ParseSector(chrElement, pSector);
			}
		}
	}	


	InsertMapset(pMapsetInfo);
}

void MQuestMapCatalogue::ParseSector(MXmlElement& element, MQuestMapSectorInfo* pSector)
{
	char szTemp[256]="";
	int n = 0;
	char szAttrValue[256];
	char szAttrName[64];
	char szTagName[128];


	int nChildCount = element.GetChildNodeCount();

	MXmlElement chrElement;
	for (int i = 0; i < nChildCount; i++)
	{
		chrElement = element.GetChildNode(i);
		chrElement.GetTagName(szTagName);
		if (szTagName[0] == '#') continue;

		if (!_stricmp(szTagName, MTOK_QUESTMAP_TAG_LINK))
		{
			int nLinkIndex = pSector->nLinkCount;

			int nLinkAttrCount = chrElement.GetAttributeCount();
			for (int j = 0; j < nLinkAttrCount; j++)
			{
				chrElement.GetAttribute(j, szAttrName, szAttrValue);
				if (!_stricmp(szAttrName, MTOK_QUESTMAP_ATTR_NAME))
				{
					strcpy_safe(pSector->Links[nLinkIndex].szName, szAttrValue);
				}
			}


			int nLinkChildCount = chrElement.GetChildNodeCount();
			MXmlElement elementTarget;
			char szTargetTagName[128];

			for (int j = 0; j < nLinkChildCount; j++)
			{
				elementTarget = chrElement.GetChildNode(j);
				elementTarget.GetTagName(szTargetTagName);
				if (szTargetTagName[0] == '#') continue;

				if (!_stricmp(szTargetTagName, MTOK_QUESTMAP_TAG_TARGET))
				{
					elementTarget.GetAttribute(szAttrValue, MTOK_QUESTMAP_ATTR_SECTOR, "");
					MQuestMapSectorInfo* pTargetSector = GetSectorInfoFromName(szAttrValue);
					if (pTargetSector)
					{
						pSector->Links[nLinkIndex].vecTargetSectors.push_back(pTargetSector->nID);
					}
				}
			}

			pSector->nLinkCount++;

			// 링크수가 10개가 넘으면 안된다.
			_ASSERT(pSector->nLinkCount <= MAX_SECTOR_LINK);
		}
	}	

}

void MQuestMapCatalogue::ParseMapsetSector1Pass(MXmlElement& elementMapset, MQuestMapsetInfo* pMapset)
{
	char szTemp[256]="";
	int n = 0;
	char szAttrValue[256];
	char szAttrName[64];
	char szTagName[128];


	int nChildCount = elementMapset.GetChildNodeCount();

	MXmlElement chrElement;
	for (int i = 0; i < nChildCount; i++)
	{
		chrElement = elementMapset.GetChildNode(i);
		chrElement.GetTagName(szTagName);
		if (szTagName[0] == '#') continue;

		if (!_stricmp(szTagName, MTOK_QUESTMAP_TAG_SECTOR))
		{
			MQuestMapSectorInfo* pSectorInfo = new MQuestMapSectorInfo();

			int nAttrCount = chrElement.GetAttributeCount();
			for (int j = 0; j < nAttrCount; j++)
			{
				chrElement.GetAttribute(j, szAttrName, szAttrValue);
				if (!_stricmp(szAttrName, MTOK_QUESTMAP_ATTR_ID))
				{
					pSectorInfo->nID = atoi(szAttrValue);
				}
				else if (!_stricmp(szAttrName, MTOK_QUESTMAP_ATTR_TITLE))
				{
					strcpy_safe(pSectorInfo->szTitle, szAttrValue);
				}
				else if (!_stricmp(szAttrName, MTOK_QUESTMAP_ATTR_MELEE_SPAWN))
				{
					pSectorInfo->nSpawnPointCount[MNST_MELEE] = atoi(szAttrValue);
				}
				else if (!_stricmp(szAttrName, MTOK_QUESTMAP_ATTR_RANGE_SPAWN))
				{
					pSectorInfo->nSpawnPointCount[MNST_RANGE] = atoi(szAttrValue);
				}
				else if (!_stricmp(szAttrName, MTOK_QUESTMAP_ATTR_BOSS_SPAWN))
				{
					pSectorInfo->nSpawnPointCount[MNST_BOSS] = atoi(szAttrValue);
					if (pSectorInfo->nSpawnPointCount[MNST_BOSS] > 0) pSectorInfo->bBoss = true;
				}

			}

			InsertSector(pSectorInfo);

			pMapset->vecSectors.push_back(pSectorInfo->nID);
		}
	}	
}

bool MQuestMapCatalogue::ReadXml(const char* szFileName)
{
	MXmlDocument xmlIniData;

	xmlIniData.Create();

	if (!xmlIniData.LoadFromFile(szFileName)) {
		xmlIniData.Destroy();
		return false;
	}

	MXmlElement rootElement, chrElement, attrElement;

	char szTagName[256];

	rootElement = xmlIniData.GetDocumentElement();

	int iCount = rootElement.GetChildNodeCount();

	for (int i = 0; i < iCount; i++) {

		chrElement = rootElement.GetChildNode(i);
		chrElement.GetTagName(szTagName);

		if (szTagName[0] == '#') continue;

		if (!_stricmp(szTagName, MTOK_QUESTMAP_TAG_MAPSET))
		{
			ParseMapset(chrElement);
		}
	}

	xmlIniData.Destroy();

	InitBackLinks();


	return true;
}

bool MQuestMapCatalogue::ReadXml(MZFileSystem* pFileSystem,const char* szFileName)
{
	MXmlDocument xmlIniData;
	if(!xmlIniData.LoadFromFile(szFileName, pFileSystem)) {
		return false;
	}

	MXmlElement rootElement, chrElement, attrElement;
	char szTagName[256];

	rootElement = xmlIniData.GetDocumentElement();
	int iCount = rootElement.GetChildNodeCount();

	for (int i = 0; i < iCount; i++) {

		chrElement = rootElement.GetChildNode(i);
		chrElement.GetTagName(szTagName);

		if (szTagName[0] == '#') continue;

		if (!_stricmp(szTagName, MTOK_QUESTMAP_TAG_MAPSET)) {
			ParseMapset(chrElement);
		}
	}

	xmlIniData.Destroy();

	InitBackLinks();

	return true;
}


void MQuestMapCatalogue::DebugReport()
{
	FILE* fp = fopen("report_questmap.txt", "wt");
	if (!fp) return;

	for (MQuestMapsetMap::iterator itor = m_MapsetInfo.begin(); itor != m_MapsetInfo.end(); ++itor)
	{
		MQuestMapsetInfo* pMapset = (*itor).second;
		fprintf(fp, " + <MAPSET> %s (%d)\n", pMapset->szTitle, pMapset->nID);

		for (int i = 0; i < (int)pMapset->vecSectors.size(); i++)
		{
			MQuestMapSectorInfo* pSector = GetSectorInfo(pMapset->vecSectors[i]);
			if (pSector)
			{
				fprintf(fp, "  <SECTOR> %s (%d)\n", pSector->szTitle, pSector->nID);

				for (int j = 0; j < pSector->nLinkCount; j++)
				{
					fprintf(fp, "    <LINK> %s\n", pSector->Links[j].szName);

					for (int k = 0; k < (int)pSector->Links[j].vecTargetSectors.size(); k++)
					{
						MQuestMapSectorInfo* pTargetSector = 
							GetSectorInfo(pSector->Links[j].vecTargetSectors[k]);
						if (pTargetSector)
						{
							fprintf(fp, "      <TARGET> %s\n", pTargetSector->szTitle);
						}
					}
				}
			}
		}
		fprintf(fp, "\n\n");
	}

	fclose(fp);
}


void MQuestMapCatalogue::InitBackLinks()
{
	for (MQuestMapSectorMap::iterator itorA = m_SectorInfo.begin(); itorA != m_SectorInfo.end(); ++itorA)
	{
		MQuestMapSectorInfo* pSectorA = (*itorA).second;

		for (MQuestMapSectorMap::iterator itorB = m_SectorInfo.begin(); itorB != m_SectorInfo.end(); ++itorB)
		{
			MQuestMapSectorInfo* pSectorB = (*itorB).second;
			if (pSectorA == pSectorB) continue;

			for (int i = 0; i < pSectorB->nLinkCount; i++)
			{
				int target_count = (int)pSectorB->Links[i].vecTargetSectors.size();
				for (int j = 0; j < target_count; j++)
				{
					if (pSectorB->Links[i].vecTargetSectors[j] == pSectorA->nID)
					{
						MQuestSectorBacklink backlink;
						backlink.nSectorID = pSectorB->nID;
						backlink.nLinkIndex = i;
						pSectorA->VecBacklinks.push_back(backlink);
					}
				}
			}
		}

	}
	



}






















