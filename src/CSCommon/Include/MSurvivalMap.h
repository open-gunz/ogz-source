#ifndef _MSURVIVAL_MAP_H
#define _MSURVIVAL_MAP_H

// ¼­¹ÙÀÌ¹ú ¸Ê Á¾·ù
enum MSURVIVAL_MAP
{
	MSURVIVAL_MAP_HALL2 = 0,
	MSURVIVAL_MAP_ROOM3,

	MSURVIVAL_MAP_END
};

// ¼­¹ÙÀÌ¹ú ¸Ê Á¤º¸
struct MSurvivalMapInfo
{
	MSURVIVAL_MAP		nID;
	char				szName[64];
};


class MSurvivalMapCatalogue
{
private:
	// ¸â¹ö º¯¼ö
	MSurvivalMapInfo		m_MapInfo[MSURVIVAL_MAP_END];

	// ÇÔ¼ö
	void SetMap(MSURVIVAL_MAP nMap, const char* szMapName);
	void Clear();
public:
	MSurvivalMapCatalogue();
	~MSurvivalMapCatalogue();
	MSurvivalMapInfo* GetInfo(MSURVIVAL_MAP nMap);
};



#endif