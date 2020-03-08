#pragma once

#include <map>
#include <list>

#include "MMatchGlobal.h"
#include "MUID.h"
#include "MPageArray.h"
#include "MSmartRefresh.h"
#include "MMatchChannelRule.h"

class MMatchObject;
class MMatchStage;
class MCommand;

typedef std::map<std::string, MMatchObject*> MObjectStrMap;
typedef std::map<int, MMatchStage*>			MChannelStageMap;
typedef MPageArray<MMatchObject*>			MChannelUserArray;


class MMatchChannel {
private:
	MUID			m_uidChannel;
	char			m_szChannelName[CHANNELNAME_LEN];
	MCHANNEL_TYPE	m_nChannelType;
	int				m_nMaxPlayers;
	int				m_nLevelMin;
	int				m_nLevelMax;
	int				m_nMaxStages;
	char			m_szRuleName[CHANNELRULE_LEN];
	MCHANNEL_RULE	m_nRuleType;

	MMatchObjectMap	m_ObjUIDCaches;
	MMatchObjectMap	m_ObjUIDLobbyCaches;

	MMatchStage*	m_pStages[MAX_CHANNEL_MAXSTAGES];
	std::list<int>	m_UnusedStageIndexList;

	MChannelUserArray			m_UserArray;
	MSmartRefresh				m_SmartRefresh;

	u32	m_nChecksum;
	u64				m_nLastChecksumTick;

	u64				m_nLastTick;
	u32	m_nEmptyPeriod;

	void JoinLobby(const MUID& uid, MMatchObject* pObj);
	void LeaveLobby(const MUID& uid);
protected:
	bool IsChecksumUpdateTime(u64 nTick);
	void UpdateChecksum(u64 nTick);
	u32 GetEmptyPeriod()	{ return m_nEmptyPeriod; }

public:
	bool CheckTick(u64 nClock);
	void Tick(u64 nClock);

	u32 GetChecksum()		{ return m_nChecksum; }
	bool CheckLifePeriod();

public:
	bool Create(const MUID& uid, const char* pszName, const char* pszRuleName, 
				MCHANNEL_TYPE nType=MCHANNEL_TYPE_PRESET, int nMaxPlayers=DEFAULT_CHANNEL_MAXPLAYERS, 
				int nLevelMin=CHANNEL_NO_LEVEL, int nLevelMax=CHANNEL_NO_LEVEL);
	void Destroy();

	const char* GetName()			{ return m_szChannelName; }
	const char* GetRuleName()		{ return m_szRuleName; }
	MUID GetUID()					{ return m_uidChannel; }
	MCHANNEL_TYPE GetChannelType()	{ return m_nChannelType; }
	MCHANNEL_RULE GetRuleType()		{ return m_nRuleType; }

	int GetMaxPlayers()				{ return m_nMaxPlayers; }
	int GetLevelMin()				{ return m_nLevelMin; }
	int GetLevelMax()				{ return m_nLevelMax; }
	int	GetMaxStages()				{ return m_nMaxStages; }
	size_t GetObjCount()			{ return m_ObjUIDCaches.size(); }
	int GetPlayers();
	auto GetObjBegin()		{ return m_ObjUIDCaches.begin(); }
	auto GetObjEnd()		{ return m_ObjUIDCaches.end(); }
	auto GetLobbyObjBegin()	{ return m_ObjUIDLobbyCaches.begin(); }
	auto GetLobbyObjEnd()	{ return m_ObjUIDLobbyCaches.end(); }

	void AddObject(const MUID& uid, MMatchObject* pObj);
	void RemoveObject(const MUID& uid);
public:
	bool AddStage(MMatchStage* pStage);
	void RemoveStage(MMatchStage* pStage);
	bool IsEmptyStage(int nIndex);
	MMatchStage* GetStage(int nIndex);
	int GetPrevStageCount(int nStageIndex);
	int GetNextStageCount(int nStageIndex);

public:
	MChannelUserArray* GetUserArray()	{ return &m_UserArray; }

public:
	void SyncPlayerList(MMatchObject* pObj, int nPage);
};

class MMatchChannelMap : public std::map<MUID, MMatchChannel*> {
private:
	MUID						m_uidGenerate;
	u32				m_nChecksum;
	std::map<MUID, MMatchChannel*>	m_TypesChannelMap[MCHANNEL_TYPE_MAX];
	void Insert(const MUID& uid, MMatchChannel* pChannel)	{	insert(value_type(uid, pChannel));	}
	MUID UseUID()				{	m_uidGenerate.Increase();	return m_uidGenerate;	}

public:
	MMatchChannelMap()			{	m_uidGenerate = MUID(0,0);	m_nChecksum=0; }
	virtual ~MMatchChannelMap()	{	}
	void Destroy();
	
	MMatchChannel* Find(const MUID& uidChannel);
	MMatchChannel* Find(const MCHANNEL_TYPE nChannelType, const char* pszChannelName);

	bool Add(const char* pszChannelName, const char* pszRuleName, MUID* pAllocUID, MCHANNEL_TYPE nType=MCHANNEL_TYPE_PRESET, int nMaxPlayers=DEFAULT_CHANNEL_MAXPLAYERS, int nLevelMin=-1, int nLevelMax=-1);
	bool Remove(const MUID& uidChannel, MMatchChannelMap::iterator* pNextItor);
	void Update(u64 nClock);

	u32 GetChannelListChecksum() const { return m_nChecksum; }
	int GetChannelCount(MCHANNEL_TYPE nChannelType);

	std::map<MUID, MMatchChannel*>::iterator GetTypesChannelMapBegin(MCHANNEL_TYPE nType);
	std::map<MUID, MMatchChannel*>::iterator GetTypesChannelMapEnd(MCHANNEL_TYPE nType);

};