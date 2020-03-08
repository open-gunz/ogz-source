#pragma once

#include "MQuestConst.h"
#include <vector>

struct MQuestSectorLink
{
	char szName[64];
	std::vector<int> vecTargetSectors;
	MQuestSectorLink() 
	{
		szName[0] = 0;
	}
};

struct MQuestSectorBacklink
{
	int nSectorID;
	int nLinkIndex;
};

struct MQuestMapSectorInfo
{
	int								nID;
	char							szTitle[64];
	bool							bBoss;
	int								nLinkCount;
	MQuestSectorLink				Links[MAX_SECTOR_LINK];
	std::vector<MQuestSectorBacklink>	VecBacklinks;
	int								nSpawnPointCount[MNST_END];

	MQuestMapSectorInfo()
	{
		nID = -1;
		szTitle[0] = 0;
		nLinkCount = 0;
		bBoss = false;
		memset(nSpawnPointCount, 0, sizeof(nSpawnPointCount));
	}
};

struct MQuestMapsetInfo
{
	int				nID;
	char			szTitle[64];
	int				nLinkCount;
	std::vector<int>		vecSectors;

	MQuestMapsetInfo()
	{
		nID = -1;
		szTitle[0] = 0;
		nLinkCount = 0;
	}
};

typedef	std::map<int, MQuestMapsetInfo*>		MQuestMapsetMap;
typedef	std::map<int, MQuestMapSectorInfo*>		MQuestMapSectorMap;

class MQuestMapCatalogue
{
private:
	MQuestMapsetMap			m_MapsetInfo;
	MQuestMapSectorMap		m_SectorInfo;

	void InsertMapset(MQuestMapsetInfo* pMapset);
	void InsertSector(MQuestMapSectorInfo* pSector);
	void ParseMapset(MXmlElement& element);
	void ParseMapsetSector1Pass(MXmlElement& elementMapset, MQuestMapsetInfo* pMapset);
	void ParseSector(MXmlElement& element, MQuestMapSectorInfo* pSector);
	void InitBackLinks();
public:
	MQuestMapCatalogue();
	~MQuestMapCatalogue();

	void Clear();
	bool ReadXml(const char* szFileName);
	bool ReadXml(class MZFileSystem* pFileSystem,const char* szFileName);
	void DebugReport();

	MQuestMapSectorInfo*	GetSectorInfo(int nSector);
	MQuestMapsetInfo*		GetMapsetInfo(int nMapset);
	MQuestMapSectorInfo*	GetSectorInfoFromName(char* szSectorTitle);

	MQuestMapsetMap*		GetMapsetMap()
	{
		return &m_MapsetInfo;
	}
};