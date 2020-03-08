#pragma once

#include <vector>
#include <map>
#include <algorithm>
#include <string>
#include <deque>
#include "MMatchItem.h"
#include "MUID.h"
#include "MObject.h"
#include "MMatchGlobal.h"
#include "MMatchFriendInfo.h"
#include "MMatchClan.h"
#include "MMatchChannel.h"
#include "MSmartRefreshImpl.h"
#include "MQuestItem.h"
#include "MMatchAntiHack.h"
#include "GlobalTypes.h"
#include "BasicInfoHistory.h"
#include "HitRegistration.h"
#include "DBQuestCachingData.h"

struct MMatchAccountInfo
{
	int						m_nAID;
	char					m_szUserID[32];
	MMatchUserGradeID		m_nUGrade;
	MMatchPremiumGradeID	m_nPGrade;
	MMatchBlockType			m_BlockType;
	tm						m_EndBlockDate;

	MMatchAccountInfo() : m_nAID(-1), m_nUGrade(MMUG_FREE),
		m_nPGrade(MMPG_FREE), m_BlockType(MMBT_NO), m_szUserID()
	{}
};

#define DBCACHING_REQUEST_POINT		40
struct DBCharCachingData
{
	int	nAddedXP;
	int	nAddedBP;
	int	nAddedKillCount;
	int	nAddedDeathCount;
	void Reset()
	{
		nAddedXP = 0;
		nAddedBP = 0;
		nAddedKillCount = 0;
		nAddedDeathCount = 0;
	}
	bool IsRequestUpdate()
	{
		if ((nAddedKillCount > DBCACHING_REQUEST_POINT) || (nAddedDeathCount > DBCACHING_REQUEST_POINT))
			return true;

		return false;
	}
};

struct MMatchCharClanInfo
{
	int					m_nClanID;
	char				m_szClanName[CLAN_NAME_LENGTH];
	MMatchClanGrade		m_nGrade;
	int					m_nContPoint;
	std::string			m_strDeleteDate;

	MMatchCharClanInfo() {  Clear(); }
	void Clear()
	{
		m_nClanID = 0; 
		m_szClanName[0] = 0; 
		m_nGrade=MCG_NONE;
		m_nContPoint = 0;
		m_strDeleteDate.clear();
	}
	bool IsJoined() { return (m_nClanID == 0) ? false : true; }
};

#define DEFAULT_CHARINFO_MAXWEIGHT		100
#define DEFAULT_CHARINFO_SAFEFALLS		0
#define DEFAULT_CHARINFO_BONUSRATE		0.0f
#define DEFAULT_CHARINFO_PRIZE			0

class MMatchCharInfo
{
public:
	u32	m_nCID;
	int					m_nCharNum;
	char				m_szName[MATCHOBJECT_NAME_LENGTH];
	int					m_nLevel;
	MMatchSex			m_nSex;
	int					m_nHair;
	int					m_nFace;
	u32	m_nXP;
	int					m_nBP;
	float				m_fBonusRate;
	int					m_nPrize;
	int					m_nHP;
	int					m_nAP;
	int					m_nMaxWeight;
	int					m_nSafeFalls;
	int					m_nFR;
	int					m_nCR;
	int					m_nER;
	int					m_nWR;
	u32	m_nEquipedItemCIID[MMCIP_END];
	MMatchItemMap		m_ItemList;
	MMatchEquipedItem	m_EquipedItem;
	MMatchCharClanInfo	m_ClanInfo;

	MQuestItemMap		m_QuestItemList;
	DBQuestCachingData	m_DBQuestCachingData;
	DBQuestCachingData& GetDBQuestCachingData() { return m_DBQuestCachingData; }

	MQuestMonsterBible	m_QMonsterBible;

	u32	m_nTotalPlayTimeSec;
	u64					m_nConnTime;

	u32	m_nTotalKillCount;
	u32	m_nTotalDeathCount;
	u32	m_nConnKillCount;
	u32	m_nConnDeathCount;
	u32   m_nConnXP;

protected:
	DBCharCachingData	m_DBCachingData;
public:
	MMatchCharInfo() : m_nCID(0), m_nCharNum(0), m_nLevel(0), m_nSex(MMS_MALE), m_nFace(0),
		               m_nHair(0), m_nXP(0), m_nBP(0), m_fBonusRate(DEFAULT_CHARINFO_BONUSRATE), m_nPrize(DEFAULT_CHARINFO_PRIZE), m_nHP(0),
					   m_nAP(0), m_nMaxWeight(DEFAULT_CHARINFO_MAXWEIGHT), m_nSafeFalls(DEFAULT_CHARINFO_SAFEFALLS),
					   m_nFR(0), m_nCR(0), m_nER(0), m_nWR(0),
					   m_nConnTime(0), m_nTotalKillCount(0), m_nTotalDeathCount(0), m_nConnKillCount(0), m_nConnDeathCount(0), 
					   m_nConnXP(0)
	{
		memset(m_szName, 0, sizeof(m_szName));
		memset(m_nEquipedItemCIID, 0, sizeof(m_nEquipedItemCIID));
		memset(&m_DBCachingData, 0, sizeof(m_DBCachingData));
		memset(&m_QMonsterBible, 0, sizeof(MQuestMonsterBible) );

		m_QuestItemList.Clear();
		m_DBQuestCachingData.Reset();
	}
	void EquipFromItemList();
	void ClearItems();
	void Clear();
	void GetTotalWeight(int* poutWeight, int* poutMaxWeight);

	void IncKill()
	{ 
		m_nTotalKillCount += 1;
		m_nConnKillCount += 1;
		m_DBCachingData.nAddedKillCount += 1;
	}
	void IncDeath()
	{ 
		m_nTotalDeathCount += 1;
		m_nConnDeathCount += 1;
		m_DBCachingData.nAddedDeathCount += 1;
	}
	void IncBP(int nAddedBP)		
	{ 
		m_nBP += nAddedBP;
		m_DBCachingData.nAddedBP += nAddedBP;
	}
	void DecBP(int nDecBP)
	{ 
		m_nBP -= nDecBP;
		m_DBCachingData.nAddedBP -= nDecBP;
	}
	void IncXP(int nAddedXP)
	{ 
		m_nConnXP += nAddedXP;
		m_nXP += nAddedXP;
		m_DBCachingData.nAddedXP += nAddedXP;
	}
	void DecXP(int nDecXP)
	{ 
		m_nConnXP -= nDecXP; 
		m_nXP -= nDecXP; 
		m_DBCachingData.nAddedXP -= nDecXP; 
	}

	DBCharCachingData* GetDBCachingData() { return &m_DBCachingData; }
};

class MMatchTimeSyncInfo final {
protected:
	int				m_nFoulCount = 0;
	u64				m_nLastSyncClock = 0;
public:
	int GetFoulCount()					{ return m_nFoulCount; }
	void AddFoulCount()					{ m_nFoulCount++; }
	void ResetFoulCount()				{ m_nFoulCount = 0; }
	auto GetLastSyncClock() const { return m_nLastSyncClock; }
	void Update(u64 nClock)				{ m_nLastSyncClock = nClock; }
};

struct MMatchObjectGameInfo
{
	bool		bJoinedGame;
};


class MMatchDisconnStatusInfo
{
public :
	MMatchDisconnStatusInfo() 
	{
		m_DisconnStatus						= MMDS_CONNECTED;
		m_dwLastDisconnStatusUpdatedTime	= GetGlobalTimeMS();
		m_bIsSendDisconnMsg					= false;
		m_dwDisconnSetTime					= 0;
		m_bIsUpdateDB						= false;
	}

	~MMatchDisconnStatusInfo() {}

	auto GetStatus() const { return m_DisconnStatus; }
	auto GetLastUpdatedTime() const { return m_dwLastDisconnStatusUpdatedTime; }
	auto GetMsgID() const { return m_dwMsgID; }
	auto GetBlockType() const { return m_BlockType; }
	auto GetBlockLevel() const { return m_BlockLevel; }
	auto& GetEndDate() const { return m_strEndDate; }
	auto& GetComment() const { return m_strComment; }
		
	const bool	IsSendDisconnMsg()	{ return m_bIsSendDisconnMsg; }
	void		SendCompleted()		{ m_bIsSendDisconnMsg = false; }

	const bool IsUpdateDB()			{ return m_bIsUpdateDB; }
	void UpdateDataBaseCompleted()	{ m_bIsUpdateDB = false; }

	const bool IsDisconnectable( const u64 dwTime = GetGlobalTimeMS() )
	{
		if( (MMDS_DISCONNECT == GetStatus()) && (MINTERVAL_DISCONNECT_STATUS_MIN < (dwTime - m_dwDisconnSetTime)) )
			return true;
		return false;
	}
	
	void SetStatus( const MMatchDisconnectStatus Status )	
	{
		m_DisconnStatus = Status; 
		if( MMDS_DISCONN_WAIT == Status )
			SendDisconnMsgPrepared();

		if( MMDS_DISCONNECT == Status )
			m_dwDisconnSetTime = (GetGlobalTimeMS() - 2000);
		
	}
	void SetUpdateTime(u64 dwTime) { m_dwLastDisconnStatusUpdatedTime = dwTime; }
	void SetMsgID(u32 dwMsgID) { m_dwMsgID = dwMsgID; }
	void SetBlockType(MMatchBlockType BlockType) { m_BlockType = BlockType; m_bIsUpdateDB = true; }
	void SetBlockLevel(MMatchBlockLevel BlockLevel) { m_BlockLevel = BlockLevel; }
	void SetEndDate(const std::string& strEndDate) { m_strEndDate = strEndDate; }
	void SetComment(const std::string& strComment) { m_strComment = strComment; }

	void Update( u64 dwTime )
	{
		if( (dwTime - GetLastUpdatedTime()) > MINTERVAL_DISCONNECT_STATUS_MIN ) 
		{
			if( MMDS_DISCONN_WAIT == GetStatus() )
				SetStatus( MMDS_DISCONNECT );
			
			SetUpdateTime( dwTime );
		}
	}

private :
	void SendDisconnMsgPrepared()	{ m_bIsSendDisconnMsg = true; }

private :
	MMatchDisconnectStatus	m_DisconnStatus;
	u64						m_dwLastDisconnStatusUpdatedTime;
	u64						m_dwDisconnSetTime;
	u32						m_dwMsgID;
	MMatchBlockType			m_BlockType;
	MMatchBlockLevel		m_BlockLevel;
	std::string				m_strEndDate;
	std::string				m_strComment;
	bool					m_bIsSendDisconnMsg;
	bool					m_bIsUpdateDB;

	const static u32 MINTERVAL_DISCONNECT_STATUS_MIN;
};


struct MMatchObjectChannelInfo
{
	MUID			uidChannel;
	MUID			uidRecentChannel;
	bool			bChannelListTransfer;
	MCHANNEL_TYPE	nChannelListType;
	u32	nChannelListChecksum;
	u64				nTimeLastChannelListTrans;
	void Clear()
	{
		uidChannel = MUID(0,0);
		uidRecentChannel = MUID(0,0);
		bChannelListTransfer = false;
		nChannelListType = MCHANNEL_TYPE_PRESET;
		nChannelListChecksum = 0;
		nTimeLastChannelListTrans = 0;
	}
};

struct ClientSettings
{
	bool DebugOutput;
};


class MMatchObject : public MObject {
protected:
	MMatchAccountInfo			m_AccountInfo;
	MMatchCharInfo*				m_pCharInfo;
	MMatchFriendInfo*			m_pFriendInfo;
	MMatchPlace					m_nPlace;
	MMatchTimeSyncInfo			m_nTimeSyncInfo;
	MMatchObjectChannelInfo		m_ChannelInfo;
	MMatchObjectGameInfo		m_GameInfo;
	MMatchObjectAntiHackInfo	m_AntiHackInfo;
	MMatchDisconnStatusInfo		m_DisconnStatusInfo;

	bool			m_bHacker;
	bool			m_bBridgePeer;
	bool			m_bRelayPeer;
	MUID			m_uidAgent;
	MMatchTeam		m_nTeam;

	u32				m_dwIP;
	char 			m_szIP[64];
	unsigned int	m_nPort;
	bool			m_bFreeLoginIP;

	unsigned char	m_nPlayerFlags;
	u32	m_nUserOptionFlags;

	MUID			m_uidStage;
	MUID			m_uidChatRoom;

	bool			m_bStageListTransfer;
	u32	m_nStageListChecksum;
	u32	m_nStageListLastChecksum;
	u64				m_nTimeLastStageListTrans;
	int				m_nStageCursor;

	MRefreshClientChannelImpl		m_RefreshClientChannelImpl;
	MRefreshClientClanMemberImpl	m_RefreshClientClanMemberImpl;

	MMatchObjectStageState	m_nStageState;
	int				m_nLadderGroupID;
	bool			m_bLadderChallenging;

	bool			m_bEnterBattle;
	bool			m_bAlive;
	u64				m_nDeadTime;

	bool			m_bNewbie;
	bool			m_bForcedEntried;
	bool			m_bLaunchedGame;

	unsigned int			m_nKillCount;
	unsigned int			m_nDeathCount;
	u32		m_nAllRoundKillCount;
	u32		m_nAllRoundDeathCount;

	bool			m_bWasCallVote;

	bool			m_bDBFriendListRequested;
	u64				m_nTickLastPacketRecved;
	std::string				m_strCountryCode3;

	u64		m_dwLastHackCheckedTime;
	u64		m_dwLastRecvNewHashValueTime;
	bool	m_bIsRequestNewHashValue;

	u64		LastSpawnTime;

	u64 m_nLastPingTime;
	mutable u32 m_nQuestLatency;
	
	// This is awkward.
	std::deque<int> Pings;
	int AveragePing;

	int MaxHP, MaxAP;
	int HP, AP;

	u64 LastHPAPInfoTime = 0;

	v3 Origin;
	v3 Direction;
	v3 Velocity;

protected:
	void UpdateChannelListChecksum(u32 nChecksum)	{ m_ChannelInfo.nChannelListChecksum = nChecksum; }
	u32 GetChannelListChecksum()					{ return m_ChannelInfo.nChannelListChecksum; }

	void UpdateStageListChecksum(u32 nChecksum)	{ m_nStageListChecksum = nChecksum; }
	u32 GetStageListChecksum()					{ return m_nStageListChecksum; }
	MMatchObject() : MObject()
	{
	}
	void DeathCount()				{ m_nDeathCount++; m_nAllRoundDeathCount++; }
	void KillCount()				{ m_nKillCount++; m_nAllRoundKillCount++; }
public:
	MMatchObject(const MUID& uid);
	virtual ~MMatchObject();

	ClientSettings clientSettings;
	MUID BotUID = MUID::Invalid();

	const char* GetName() { 
		if (m_pCharInfo)
			return m_pCharInfo->m_szName; 
		else
			return "Unknown";
	}
	char* GetAccountName()			{ return m_AccountInfo.m_szUserID; }

	bool GetBridgePeer()			{ return m_bBridgePeer; }
	void SetBridgePeer(bool bValue)	{ m_bBridgePeer = bValue; }
	bool GetRelayPeer()				{ return m_bRelayPeer; }
	void SetRelayPeer(bool bRelay)	{ m_bRelayPeer = bRelay; }
	const MUID& GetAgentUID()		{ return m_uidAgent; }
	void SetAgentUID(const MUID& uidAgent)	{ m_uidAgent = uidAgent; }

	void SetPeerAddr(u32 dwIP, const char* szIP, unsigned short nPort) {
		m_dwIP=dwIP; strcpy_safe(m_szIP, szIP); m_nPort = nPort;
	}
	u32 GetIP() const				{ return m_dwIP; }
	char* GetIPString()				{ return m_szIP; }
	unsigned short GetPort()		{ return m_nPort; }
	bool GetFreeLoginIP()			{ return m_bFreeLoginIP; }
	void SetFreeLoginIP(bool bFree)	{ m_bFreeLoginIP = bFree; }

	void ResetPlayerFlags()						{ m_nPlayerFlags = 0; }
	unsigned char GetPlayerFlags()				{ return m_nPlayerFlags; }
	bool CheckPlayerFlags(unsigned char nFlag)	{ return (m_nPlayerFlags&nFlag?true:false); }
	void SetPlayerFlag(unsigned char nFlagIdx, bool bSet)	
	{ 
		if (bSet) m_nPlayerFlags |= nFlagIdx; 
		else m_nPlayerFlags &= (0xff ^ nFlagIdx);
	}

	void SetUserOption(u32 nFlags)	{ m_nUserOptionFlags = nFlags; }
	bool CheckUserOption(u32 nFlag)	{ return (m_nUserOptionFlags&nFlag?true:false); }

	MUID GetChannelUID()						{ return m_ChannelInfo.uidChannel; }
	void SetChannelUID(const MUID& uid)			{ SetRecentChannelUID(m_ChannelInfo.uidChannel); m_ChannelInfo.uidChannel = uid; }
	MUID GetRecentChannelUID()					{ return m_ChannelInfo.uidRecentChannel; }
	void SetRecentChannelUID(const MUID& uid)	{ m_ChannelInfo.uidRecentChannel = uid; }

	MUID GetStageUID() const			{ return m_uidStage; }
	void SetStageUID(const MUID& uid)	{ m_uidStage = uid; }
	MUID GetChatRoomUID() const			{ return m_uidChatRoom; }
	void SetChatRoomUID(const MUID& uid){ m_uidChatRoom = uid; }

	bool CheckChannelListTransfer() const { return m_ChannelInfo.bChannelListTransfer; }
	void SetChannelListTransfer(const bool bVal, const MCHANNEL_TYPE nChannelType=MCHANNEL_TYPE_PRESET);

	bool CheckStageListTransfer() const { return m_bStageListTransfer; }
	void SetStageListTransfer(bool bVal)	{ m_bStageListTransfer = bVal; UpdateStageListChecksum(0); }

	MRefreshClientChannelImpl* GetRefreshClientChannelImplement()		{ return &m_RefreshClientChannelImpl; }
	MRefreshClientClanMemberImpl* GetRefreshClientClanMemberImplement()	{ return &m_RefreshClientClanMemberImpl; }

	MMatchTeam GetTeam() const { return m_nTeam; }
	void SetTeam(MMatchTeam nTeam);
	MMatchObjectStageState GetStageState() const { return m_nStageState; }
	void SetStageState(MMatchObjectStageState nStageState)	{ m_nStageState = nStageState; }
	bool GetEnterBattle() const		{ return m_bEnterBattle; }
	void SetEnterBattle(bool bEnter){ m_bEnterBattle = bEnter; }
	bool CheckAlive() const			{ return m_bAlive; }
	bool IsDie() const				{ return !m_bAlive; }
	bool IsAlive() const			{ return m_bAlive; }
	void SetAlive(bool bVal)		{ m_bAlive = bVal; }
	void SetKillCount(unsigned int nKillCount) { m_nKillCount = nKillCount; }
	unsigned int GetKillCount()		{ return m_nKillCount; }
	void SetDeathCount(unsigned int nDeathCount) { m_nDeathCount = nDeathCount; }
	unsigned int GetDeathCount()	{ return m_nDeathCount; }
	unsigned int GetAllRoundKillCount()	{ return m_nAllRoundKillCount; }
	unsigned int GetAllRoundDeathCount()	{ return m_nAllRoundDeathCount; }
	void SetAllRoundKillCount(unsigned int nAllRoundKillCount) { m_nAllRoundKillCount = nAllRoundKillCount; }
	void SetAllRoundDeathCount(unsigned int nAllRoundDeathCount) { m_nAllRoundDeathCount = nAllRoundDeathCount; }
	void FreeCharInfo();
	void FreeFriendInfo();
	void OnDead();
	void OnKill();
	bool IsEnabledRespawnDeathTime(u64 nNowTime) const;

	void SetForcedEntry(bool bVal) { m_bForcedEntried = bVal; }
	bool IsForcedEntried() { return m_bForcedEntried; }
	void SetLaunchedGame(bool bVal) { m_bLaunchedGame = bVal; }
	bool IsLaunchedGame() { return m_bLaunchedGame; }
	void CheckNewbie(int nCharMaxLevel);
	bool IsNewbie()					{ return m_bNewbie; }
	bool IsHacker()					{ return m_bHacker; }
	void SetHacker(bool bHacker)	{ m_bHacker = bHacker; }

	inline bool WasCallVote()						{ return m_bWasCallVote; }
	inline void SetVoteState( const bool bState )	{ m_bWasCallVote = bState; }

	inline auto	GetTickLastPacketRecved()		{ return m_nTickLastPacketRecved; }

	void UpdateTickLastPacketRecved();

	void SetLastRecvNewHashValueTime( const u32 dwTime )	
	{ 
		m_dwLastRecvNewHashValueTime = dwTime; 
		m_bIsRequestNewHashValue = false; 
	}

	auto GetLastSpawnTime(void) const				{ return LastSpawnTime;		}
	void SetLastSpawnTime(u64 Time)					{ LastSpawnTime = Time;	}

	inline u32					GetQuestLatency() const;
	inline void					SetQuestLatency(u64 l);
	inline void					SetPingTime(u64 t);
public:
	int GetLadderGroupID()			{ return m_nLadderGroupID; }
	void SetLadderGroupID(int nID)	{ m_nLadderGroupID = nID; }
	void SetLadderChallenging(bool bVal) { m_bLadderChallenging = bVal; }
	bool IsLadderChallenging()			{ return m_bLadderChallenging; }
public:
	MMatchAccountInfo* GetAccountInfo() { return &m_AccountInfo; }
	const MMatchAccountInfo* GetAccountInfo() const { return &m_AccountInfo; }
	MMatchCharInfo* GetCharInfo() { return m_pCharInfo; }
	const MMatchCharInfo* GetCharInfo() const { return m_pCharInfo; }
	void SetCharInfo(MMatchCharInfo* pCharInfo);
	
	MMatchFriendInfo* GetFriendInfo()	{ return m_pFriendInfo; }
	void SetFriendInfo(MMatchFriendInfo* pFriendInfo);
	bool DBFriendListRequested()	{ return m_bDBFriendListRequested; }
	MMatchPlace GetPlace()			{ return m_nPlace; }
	void SetPlace(MMatchPlace nPlace);
	MMatchTimeSyncInfo* GetSyncInfo()	{ return &m_nTimeSyncInfo; }
	MMatchObjectAntiHackInfo* GetAntiHackInfo()		{ return &m_AntiHackInfo; }
	MMatchDisconnStatusInfo& GetDisconnStatusInfo() { return  m_DisconnStatusInfo; }

	void Tick(u64 nTime);

	void OnStageJoin();
	void OnEnterBattle();
	void OnLeaveBattle();
	void OnInitRound();

	void SetStageCursor(int nStageCursor);
	const MMatchObjectGameInfo* GetGameInfo() { return &m_GameInfo; }

	void AddPing(int Ping)
	{
		Pings.push_front(Ping);

		while (Pings.size() > 10)
			Pings.pop_back();

		int sum = 0;
		for (auto val : Pings)
			sum += val;

		AveragePing = sum / Pings.size();
	}

	auto GetPing() const
	{
		return AveragePing;
	}

	void GetPositions(v3* Head, v3* Foot, double Time) const;
	auto& GetPosition() const { return Origin; }
	auto& GetDirection() const { return Direction; }
	auto& GetVelocity() const { return Velocity; }

	BasicInfoHistoryManager BasicInfoHistory;

	auto GetSelectedSlot() const
	{
		if (BasicInfoHistory.empty())
			return MMCIP_PRIMARY;

		return BasicInfoHistory.front().SelectedSlot;
	}

	auto HitTest(const v3& src, const v3& dest, double Time, v3* OutPos = nullptr)
	{
		v3 Head, Foot;
		GetPositions(&Head, &Foot, Time);
		return PlayerHitTest(Head, Foot, src, dest, OutPos);
	}

	void SetMaxHPAP();
	void OnDamaged(const MMatchObject& Attacker, const v3& SrcPos,
		ZDAMAGETYPE DamageType, MMatchWeaponType WeaponType, int Damage, float PiercingRatio);
	void Heal(int Amount);
	void ResetHPAP();

public:
	enum MMO_ACTION
	{
		MMOA_STAGE_FOLLOW,
		MMOA_MAX
	};
	bool CheckEnableAction(MMO_ACTION nAction);

	bool m_bQuestRecvPong;
};

using MMatchObjectList = std::map<MUID, MMatchObject*>;

bool IsEquipableItem(u32 nItemID, int nPlayerLevel, MMatchSex nPlayerSex);

inline bool IsEnabledObject(MMatchObject* pObject) 
{
	if ((pObject == NULL) || (pObject->GetCharInfo() == NULL)) return false;
	return true;
}

inline bool IsAdminGrade(MMatchObject* pObject)
{
	if (pObject == NULL) return false;

	return IsAdminGrade(pObject->GetAccountInfo()->m_nUGrade);
}

u32 MMatchObject::GetQuestLatency() const
{
	auto nowTime = GetGlobalTimeMS();

	auto ret = static_cast<u32>(nowTime - m_nLastPingTime);

	if (ret > m_nQuestLatency)
		m_nQuestLatency = ret;

	return m_nQuestLatency;
}

void MMatchObject::SetQuestLatency(u64 l)
{
	m_nQuestLatency = static_cast<u32>(l - m_nLastPingTime);
}

void MMatchObject::SetPingTime(u64 t)
{
	m_nLastPingTime = t;
}