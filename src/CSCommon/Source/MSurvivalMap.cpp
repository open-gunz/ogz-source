#include "stdafx.h"
#include "MSurvivalMap.h"


MSurvivalMapCatalogue::MSurvivalMapCatalogue()
{
	Clear();

	// 하드코딩으로 직접 삽입
	SetMap(MSURVIVAL_MAP_HALL2,		"Mansion_hall2");
	SetMap(MSURVIVAL_MAP_ROOM3,		"Mansion_room3");
}

MSurvivalMapCatalogue::~MSurvivalMapCatalogue()
{

}

MSurvivalMapInfo* MSurvivalMapCatalogue::GetInfo(MSURVIVAL_MAP nMap)
{
	_ASSERT(nMap >= 0 && nMap < MSURVIVAL_MAP_END);

	return &m_MapInfo[nMap];

}

void MSurvivalMapCatalogue::SetMap(MSURVIVAL_MAP nMap, const char* szMapName)
{
	_ASSERT(nMap >= 0 && nMap < MSURVIVAL_MAP_END);

	m_MapInfo[nMap].nID = nMap;
	strcpy_safe(m_MapInfo[nMap].szName, szMapName);
}

void MSurvivalMapCatalogue::Clear()
{
	memset(m_MapInfo, 0, sizeof(m_MapInfo));
}
