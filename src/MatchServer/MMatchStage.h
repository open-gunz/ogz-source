#pragma once
#include <list>
#include "MMatchItem.h"
#include "MMatchTransDataType.h"
#include "MUID.h"
#include "MMatchRule.h"
#include "MMatchObject.h"
#include "MMatchWorldItem.h"
#include "MMatchStageSetting.h"
#include "MVoteMgr.h"
#include "MMatchGlobal.h"
#include "MUtil.h"
#include "MovingWeaponManager.h"

#define MTICK_STAGE			100

class MMatchObject;
class MMatchStage;
class MMatchServer;
class MLadderGroup;

enum MMatchStageType
{
	MST_NORMAL	= 0,
	MST_LADDER,
	
	MST_MAX
};

struct MMatchStageTeamBonus
{
	bool		bApplyTeamBonus;
};

struct MMatchLadderTeamInfo
{
	int		nTID;
	int		nFirstMemberCount;

	int		nCLID;
	int		nCharLevel;
	int		nContPoint;
};

struct MMatchStageTeam
{
	int						nTeamBonusExp;
	int						nTeamTotalLevel;
	int						nScore;
	int						nSeriesOfVictories;
	int						nTotalKills;
	MMatchLadderTeamInfo	LadderInfo;
};

namespace RealSpace2
{
class RBspObject;
}

class MMatchStage {
private:
	int						m_nIndex;
	STAGE_STATE				m_nState;
	MMatchStageType			m_nStageType;
	MUID					m_uidStage;
	MUID					m_uidOwnerChannel;
	char					m_szStageName[STAGENAME_LENGTH];
	bool					m_bPrivate;
	char					m_szStagePassword[STAGENAME_LENGTH];
	MMatchStageTeamBonus	m_TeamBonus;
	MMatchStageTeam			m_Teams[MMT_END];
	MMatchObjectMap			m_ObjUIDCaches;

	list<int>				m_BanCIDList;

	u64						m_nStateTimer;
	u64						m_nLastTick;
	u32			m_nChecksum;
	u64						m_nLastChecksumTick;
	int						m_nAdminObjectCount;
	u64						m_nStartTime;

	
	MMatchStageSetting		m_StageSetting;
	MMatchRule*				m_pRule;

	MUID					m_uidAgent;
	bool					m_bAgentReady;
	int						m_nRoundObjCount[MMT_END];

	MVoteMgr				m_VoteMgr;

	char					m_szFirstMasterName[MATCHOBJECT_NAME_LENGTH];

	u64 LastPhysicsTick = 0;

	void SetMasterUID(const MUID& uid)	{ m_StageSetting.SetMasterUID(uid);}
	MMatchRule* CreateRule(MMATCH_GAMETYPE nGameType);
protected:
	bool IsChecksumUpdateTime(u64 nTick) const;
	void UpdateChecksum(u64 nTick);
	void OnStartGame();
	void OnFinishGame();
	void OnApplyTeamBonus(MMatchTeam nTeam);
protected:
	friend MMatchServer;
	void SetStageType(MMatchStageType nStageType);
	void SetLadderTeam(MMatchLadderTeamInfo* pRedLadderTeamInfo, MMatchLadderTeamInfo* pBlueLadderTeamInfo);
public:
	RealSpace2::RBspObject* BspObject = nullptr;
	MovingWeaponManager MovingWeaponMgr;
	MMatchWorldItemManager	m_WorldItemManager;

	struct Bot
	{
		MUID OwnerUID;
		MUID BotUID;
		MTD_PeerListNode PeerListNode;
	};
	std::vector<Bot> Bots;

	void UpdateStateTimer();
	auto GetStateTimer() const	{ return m_nStateTimer; }
	auto GetChecksum() const { return m_nChecksum; }
	auto GetStartTime() const { return m_nStartTime; }
public:
	MMatchStage();

	auto GetObjectList() { return MakePairValueAdapter(m_ObjUIDCaches); }

	bool Create(const MUID& uid, const char* pszName, bool bPrivate, const char* pszPassword);
	void Destroy();
	void OnCommand(MCommand* pCommand);
	void OnGameKill(const MUID& uidAttacker, const MUID& uidVictim);
	bool CheckAutoTeamBalancing();
	void ShuffleTeamMembers();

	const char* GetName()		{ return m_szStageName; }
	const char* GetPassword()	{ return m_szStagePassword; }
	void SetPassword(const char* pszPassword)	{ strcpy_safe(m_szStagePassword, pszPassword); }
	const bool IsPrivate()		{ return m_bPrivate; }
	void SetPrivate(bool bVal)	{ m_bPrivate = bVal; }
	MUID GetUID()				{ return m_uidStage; }

	const char* GetMapName()	{ return m_StageSetting.GetMapName(); }
	void SetMapName(const char* pszMapName)	{ m_StageSetting.SetMapName(pszMapName); }

	char* GetFirstMasterName()	{ return m_szFirstMasterName; }
	void SetFirstMasterName(const char* pszName)	{ strcpy_safe(m_szFirstMasterName, pszName); }

	MMatchObject* GetObj(const MUID& uid)
	{
		if (m_ObjUIDCaches.count(uid) == 0)
			return NULL;

		return (MMatchObject*)(m_ObjUIDCaches[uid]);
	}

	size_t GetObjCount() const { return m_ObjUIDCaches.size(); }
	int GetPlayers();
	auto GetObjBegin()	{ return m_ObjUIDCaches.begin(); }
	auto GetObjEnd()	{ return m_ObjUIDCaches.end(); }
	int GetObjInBattleCount();
	int GetCountableObjCount() const { return ((int)GetObjCount() - m_nAdminObjectCount); }

	void AddBanList(int nCID);
	bool CheckBanList(int nCID);

	void AddObject(const MUID& uid, MMatchObject* pObj);
	MMatchObjectMap::iterator RemoveObject(const MUID& uid);
	bool KickBanPlayer(const char* pszName, bool bBanPlayer=true);

	const MUID RecommandMaster(bool bInBattleOnly);
	void EnterBattle(MMatchObject* pObj);
	void LeaveBattle(MMatchObject* pObj);

	STAGE_STATE GetState() const { return m_nState; }
	void ChangeState(STAGE_STATE nState)	{ m_nState = nState; UpdateStateTimer(); }

	bool CheckTick(u64 nClock);
	void Tick(u64 nClock);

	MMatchStageSetting* GetStageSetting() { return &m_StageSetting; }

	MMatchRule* GetRule()			{ return m_pRule; }
	void ChangeRule(MMATCH_GAMETYPE nRule);
	void GetTeamMemberCount(int* poutnRedTeamMember, int* poutnBlueTeamMember, int* poutSpecMember, bool bInBattle);
	MMatchTeam GetRecommandedTeam();

	MVoteMgr* GetVoteMgr()			{ return &m_VoteMgr; }

	MUID GetAgentUID() const		{ return m_uidAgent; }
	void SetAgentUID(MUID uid)		{ m_uidAgent = uid; }
	bool GetAgentReady() const		{ return m_bAgentReady; }
	void SetAgentReady(bool bReady)	{ m_bAgentReady = bReady; }

	MUID GetMasterUID()	const		{ return m_StageSetting.GetMasterUID(); }
	int GetIndex()					{ return m_nIndex; }

	void SetOwnerChannel(const MUID& uidOwnerChannel, int nIndex);
	MUID GetOwnerChannel() const { return m_uidOwnerChannel; }

	void PlayerTeam(const MUID& uidPlayer, MMatchTeam nTeam);
	void PlayerState(const MUID& uidPlayer, MMatchObjectStageState nStageState);
	bool StartGame();
	bool FinishGame();
	bool CheckBattleEntry();

	void RoundStateFromClient(const MUID& uidStage, int nState, int nRound);
	void ObtainWorldItem(MMatchObject* pObj, const int nItemID);
	void RequestSpawnWorldItem(MMatchObject* pObj, const int nItemID, 
							   const float x, const float y, const float z);
	void SpawnServerSideWorldItem(MMatchObject* pObj, const int nItemID, 
							   const float x, const float y, const float z, 
							   int nLifeTime, int* pnExtraValues );

	bool IsApplyTeamBonus();
	void AddTeamBonus(int nExp, MMatchTeam nTeam);
	int GetTeamScore(MMatchTeam nTeam) const { return m_Teams[nTeam].nScore; }

	int GetTeamKills(MMatchTeam nTeam) const { return m_Teams[nTeam].nTotalKills; }
	void AddTeamKills(MMatchTeam nTeam, int amount=1)		{ m_Teams[nTeam].nTotalKills+=amount; }
	void InitTeamKills()		{ m_Teams[MMT_BLUE].nTotalKills = m_Teams[MMT_RED].nTotalKills = 0; }

	MMatchStageType GetStageType() const { return m_nStageType; }
	int GetMinPlayerLevel();

	bool CheckUserWasVoted( const MUID& uidPlayer );

	void OnRoundEnd_FromTeamGame(MMatchTeam nWinnerTeam);
	void OnInitRound();

	void UpdateWorldItems();

	void ResetTeams();
};

class MMatchStageMap : public std::map<MUID, MMatchStage*> {
	MUID	m_uidGenerate;
public:
	MMatchStageMap()			{	m_uidGenerate = MUID(0,0);	}
	virtual ~MMatchStageMap()	{	}
	MUID UseUID()				{	m_uidGenerate.Increase();	return m_uidGenerate;	}
	void Insert(const MUID& uid, MMatchStage* pStage)	{	insert(value_type(uid, pStage));	}
};

MMatchItemBonusType GetStageBonusType(MMatchStageSetting* pStageSetting);

#define TRANS_STAGELIST_NODE_COUNT	8