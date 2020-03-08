#pragma once

#include "MXml.h"
#include <map>
#include "MMatchMap.h"
#include "RTypes.h"

#define WORLDITEM_NAME_LENGTH		256

enum MMATCH_WORLD_ITEM_TYPE
{
	WIT_HP			= 0,
	WIT_AP			= 1,
	WIT_BULLET		= 2,
	WIT_HPAP		= 3,
	WIT_CLIENT		= 4,

	WIT_QUEST		= 5,
	WIT_BOUNTY		= 6,

	WIT_END
};


struct MMatchWorldItemDesc
{
	short					m_nID;
	MMATCH_WORLD_ITEM_TYPE	m_nItemType;
	float					m_fAmount;
	u32		m_nTime;
	char					m_szModelName[WORLDITEM_NAME_LENGTH];
	char					m_szDescName[WORLDITEM_NAME_LENGTH];
};

class MMatchWorldItemDescMgr;
class MZFileSystem;

class MMatchWorldItemDescMgr : public std::map<short, MMatchWorldItemDesc*>
{
private:
protected:
	void ParseWorldItem(MXmlElement& element);
public:
	MMatchWorldItemDescMgr();
	virtual ~MMatchWorldItemDescMgr();
	bool ReadXml(const char* szFileName);
	bool ReadXml(MZFileSystem* pFileSystem, const char* szFileName);
	void Clear();
	MMatchWorldItemDesc* GetItemDesc(short nID);
	static MMatchWorldItemDescMgr* GetInstance();
};

inline MMatchWorldItemDescMgr* MGetMatchWorldItemDescMgr()
{ 
	return MMatchWorldItemDescMgr::GetInstance();
}

#define WORLDITEM_EXTRAVALUE_NUM		2

struct MMatchWorldItem
{
	unsigned short		nUID;
	unsigned short		nItemID;
	short				nStaticSpawnIndex;
	v3					Origin;
	int					nLifeTime;

	union {
		struct {
			int			nDropItemID;
			int			nRentPeriodHour;
		};
		int				nExtraValue[WORLDITEM_EXTRAVALUE_NUM];
	};
};

struct MMatchWorldItemSpawnInfo
{
	unsigned short		nItemID;
	u32	nCoolTime;
	u32	nElapsedTime;
	float x;
	float y;
	float z;
	bool				bExist;
	bool				bUsed;
};

#define MAX_WORLDITEM_SPAWN		100

struct MMatchMapsWorldItemSpawnInfoSet
{
	MMatchWorldItemSpawnInfo	SoloSpawnInfo[MAX_WORLDITEM_SPAWN];
	MMatchWorldItemSpawnInfo	TeamSpawnInfo[MAX_WORLDITEM_SPAWN];
	int							m_nSoloSpawnCount;
	int							m_nTeamSpawnCount;
};

class MMatchMapsWorldItemSpawnInfo
{
private:
	void ParseSpawnInfo(MXmlElement& element, int nMapID);
	void SetMapsSpawnInfo(int nMapID, char* szGameTypeID, int nItemID, float x, float y, float z, 
						  u32 nCoolTime);
	bool ReadXml(const char* szFileName, int nMapID);
protected:
public:
	MMatchMapsWorldItemSpawnInfoSet		m_MapsSpawnInfo[MMATCH_MAP_COUNT];
	MMatchMapsWorldItemSpawnInfo();
	virtual ~MMatchMapsWorldItemSpawnInfo();
	
	bool Read();
	void Clear();
	static MMatchMapsWorldItemSpawnInfo* GetInstance();
};

inline MMatchMapsWorldItemSpawnInfo* MGetMapsWorldItemSpawnInfo() 
{ 
	return MMatchMapsWorldItemSpawnInfo::GetInstance();
}