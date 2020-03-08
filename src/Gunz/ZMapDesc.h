#pragma once

#include "ZPrerequisites.h"
#include <list>
#include <vector>

#include "MXml.h"
#include "RTypes.h"
#include "MZFileSystem.h"

enum ZMapSpawnType
{
	ZMST_SOLO		= 0,
	ZMST_TEAM1,
	ZMST_TEAM2,

	ZMST_NPC_MELEE,
	ZMST_NPC_RANGE,
	ZMST_NPC_BOSS,

	ZMST_MAX
};

struct ZMapSpawnData
{
	ZMapSpawnType	m_nType;
	char			m_szSpawnName[32];
	rvector			m_Pos;
	rvector			m_Dir;
};

struct ZMAPSPAWN
{
	rvector pos;
	rvector dir;
};

using ZMapSpawnList = std::vector<ZMapSpawnData*>;

#define MAX_BACKUP_SPAWN 5

class ZMapSpawnManager
{
private:
	bool Add(ZMapSpawnData* pMapSpawnData);
	bool IsExistOtherCharacter(rvector pos);
	int	 m_nBackUpIndex[MAX_BACKUP_SPAWN];
	int  m_nBackUpIndexCnt;
public:
	ZMapSpawnManager();
	~ZMapSpawnManager();

	ZMapSpawnList	m_Spawns;
	ZMapSpawnList	m_SpawnArray[ZMST_MAX];

	bool AddSpawnData(const char* szName, const rvector& pos, const rvector& dir);
	void Clear();
	int GetCount() const { return (int)m_Spawns.size(); }
	int GetSoloCount() const { return (int)m_SpawnArray[ZMST_SOLO].size(); }
	int GetTeamCount(int nTeamIndex);
	int GetSpawnCount(ZMapSpawnType nSpawnType) { return (int)m_SpawnArray[nSpawnType].size(); }

	bool CheckBackup(int index);
	void ShiftBackupIndex(int index);

	ZMapSpawnData* GetData(int nIndex);
	ZMapSpawnData* GetSoloData(int nIndex);
	ZMapSpawnData* GetTeamData(int nTeamIndex, int nDataIndex);

	ZMapSpawnData* GetSoloRandomData();

	ZMapSpawnData* GetSpawnData(ZMapSpawnType nSpawnType, int nIndex);
};

enum ZMapSmokeType
{
	ZMapSmokeType_None = 0 ,

	ZMapSmokeType_SS ,
	ZMapSmokeType_ST ,
	ZMapSmokeType_TS ,

	ZMapSmokeType_End
};

class ZMapSmokeDummy 
{
public:
	ZMapSmokeDummy() {

		m_fPower = 0.f;
		m_dwColor = 0x01010101;
		m_SmokeType = ZMapSmokeType_None;
		m_fLife = 1.f;
		m_nDelay = 100;
		m_fToggleMinTime = 2.f;
		m_fSize = 40.f;
		m_fDrawBackupTime = 0.f;
	}

	bool NameCmp(const std::string& name) {
		if(name==m_Name)
			return true;
		return false;
	}

public:

	ZMapSmokeType	m_SmokeType;

	string  m_Name;
	rvector m_vPos;
	rvector m_vDir;

	DWORD  m_dwColor;
	float  m_fDrawBackupTime;
	float  m_fLife;
	float  m_fPower;
	int	   m_nDelay;
	float  m_fSize;
	float  m_fToggleMinTime;
	
};

class ZMapSmokeSS : public ZMapSmokeDummy
{
public:
	ZMapSmokeSS() {
		m_SmokeType = ZMapSmokeType_SS;
		m_fLife = 6.f;
		m_nDelay = 600;
	}
};

class ZMapSmokeST : public ZMapSmokeDummy	// train steam
{
public:
	ZMapSmokeST() {
		m_SmokeType = ZMapSmokeType_ST;
		m_vSteamDir = rvector(0,0,1);
		m_bToggle = false;
		m_fToggleTime = 2.f;
		m_fToggleSaveTime = 0.f;
		m_fLife = 1.f;
		m_nDelay = 100;
	}

	rvector m_vSteamDir;
	float	m_fToggleSaveTime;
	float	m_fToggleTime;
	bool	m_bToggle;
};

class ZMapSmokeTS : public ZMapSmokeDummy	// train smoke
{
public:
	ZMapSmokeTS() {
		m_SmokeType = ZMapSmokeType_TS;
	}
};

class ZMapSmokeDummyMgr : public vector<ZMapSmokeDummy*>
{
public:
	ZMapSmokeDummyMgr() {
		
	}

	virtual ~ZMapSmokeDummyMgr() {

		Destroy();
	}

	void Destroy() {

		while(!empty()) {
			delete *begin();
			erase(begin());
		}
	}

	ZMapSmokeDummy* Get(const std::string& name) {
		
		iterator it;

		for(it=begin();it!=end();++it) {

			if( (*it)->NameCmp(name) )
				return (*it);
		}

		return NULL;
	}
};

struct ZQuestMapDesc
{
	rvector	 m_vLinks[MAX_QUEST_MAP_SECTOR_COUNT];
	ZQuestMapDesc()
	{
		memset(m_vLinks, 0, sizeof(m_vLinks));
	}
};


class ZMapDesc
{
private:
protected:
	ZMapSpawnManager	m_SpawnManager;

	rvector				m_WaitCamDir;
	rvector				m_WaitCamPos;
	
	float				m_SmokeSSDrawBackupTime;
	float				m_SmokeSTDrawBackupTime;
	float				m_SmokeTSDrawBackupTime;

	DWORD				m_StartTime;

	ZQuestMapDesc		m_QuestMapDesc;
public:

	ZMapSmokeDummyMgr		m_SmokeDummyMgr;

private:

	bool m_bRenderedSS;
	bool m_bRenderedST;
	bool m_bRenderedTS;

public:
	char	m_szName[256];

	ZMapDesc();
	virtual ~ZMapDesc();
	bool Open(RBspObject* pBspObject);

	ZMapSpawnManager*	GetSpawnManager()	{ return &m_SpawnManager; }
	ZMapSmokeDummyMgr*	GetSmokeDummyMgr()	{ return &m_SmokeDummyMgr; }

	float GetThisTime();

	bool LoadSmokeDesc(const char* pFileName);	

	void DrawSmoke();

	void DrawSmokeSS(ZMapSmokeSS* pDummy);
	void DrawSmokeST(ZMapSmokeST* pDummy);
	void DrawSmokeTS(ZMapSmokeTS* pDummy);

	void DrawMapDesc();

	const rvector GetWaitCamDir()		{ return m_WaitCamDir; }
	const rvector GetWaitCamPos()		{ return m_WaitCamPos; }

	rvector GetQuestSectorLink(int nIndex);
};