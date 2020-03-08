#pragma once

#include <list>
#include "MUID.h"
#include "MMatchGlobal.h"
#include "MMatchGameType.h"
#include "MMatchMap.h"

#define MMATCH_TEAM_MAX_COUNT		2

#define MMATCH_SPECTATOR_STR		"SPECTATOR"
#define MMATCH_TEAM1_NAME_STR		"RED TEAM"
#define MMATCH_TEAM2_NAME_STR		"BLUE TEAM"

inline const char* GetTeamNameStr(MMatchTeam nTeam)
{
	switch (nTeam)
	{
	case MMT_SPECTATOR:
		return MMATCH_SPECTATOR_STR;
	case MMT_RED:
		return MMATCH_TEAM1_NAME_STR;
	case MMT_BLUE:
		return MMATCH_TEAM2_NAME_STR;
	default:
		return "";
	}
	return "";
}

enum STAGE_STATE {
	STAGE_STATE_STANDBY		= 0,
	STAGE_STATE_COUNTDOWN,
	STAGE_STATE_RUN,
	STAGE_STATE_CLOSE
};

#define MSTAGENODE_FLAG_FORCEDENTRY_ENABLED		1
#define MSTAGENODE_FLAG_PRIVATE					2
#define MSTAGENODE_FLAG_LIMITLEVEL				4

enum class NetcodeType : u8
{
	ServerBased,
	P2PAntilead,
	P2PLead,
};

#pragma pack(push, 1)
struct MSTAGE_SETTING_NODE {
	MUID				uidStage;
	char				szStageName[64];
	char				szMapName[MAPNAME_LENGTH];
	//MMatchMapDesc		szBannerName;
	
	// some room identifier
	char				nMapIndex;
	MMATCH_GAMETYPE		nGameType;
	int					nRoundMax;
	int					nLimitTime;
	int					nLimitLevel;
	int					nMaxPlayers;
	bool				bTeamKillEnabled;
	bool				bTeamWinThePoint;
	bool				bForcedEntryEnabled;
	bool				bAutoTeamBalancing;
#ifdef _VOTESETTING
	bool				bVoteEnabled;
	bool				bObserverEnabled;
#endif

	NetcodeType Netcode;

	bool ForceHPAP;
	int HP;
	int AP;
	bool NoFlip;
	bool SwordsOnly;
	bool VanillaMode;
	bool InvulnerabilityStates;

	MSTAGE_SETTING_NODE() { SetDefaults(); }

	void SetDefaults()
	{
		uidStage = MUID{ 0, 0 };
		memset(szStageName, 0, std::size(szStageName));
		// Default map.
		strcpy_safe(szMapName, "Mansion");
		//szBannerName = MMATCH_MAP_MANSION;

		nMapIndex = 0;
		nGameType = MMATCH_GAMETYPE_DEATHMATCH_SOLO;
		nRoundMax = 50;
		nLimitTime = 30;
		nLimitLevel = 0;
		nMaxPlayers = 8;
		bTeamKillEnabled = false;
		bTeamWinThePoint = false;
		bForcedEntryEnabled = true;
		bAutoTeamBalancing = true;

#ifdef _VOTESETTING
		bVoteEnabled = true;
		bObserverEnabled = false;
#endif

		Netcode = NetcodeType::P2PAntilead;
		ForceHPAP = false;
		HP = 100;
		AP = 50;
		NoFlip = false;
		SwordsOnly = false;
		VanillaMode = true;
		InvulnerabilityStates = true;
	}
};
#pragma pack(pop)

#define STAGESETTING_LIMITTIME_UNLIMITED				0

struct MSTAGE_CHAR_SETTING_NODE {
	MUID	uidChar{};
	int		nTeam{};
	MMatchObjectStageState	nState = MOSS_NONREADY;
};

using MStageCharSettingList = std::vector<MSTAGE_CHAR_SETTING_NODE>;

class MMatchStageSetting final
{
public:
	void Clear();
	void SetDefault();
	u32 GetChecksum();
	MSTAGE_CHAR_SETTING_NODE* FindCharSetting(const MUID& uid);

	// Get
	const char* GetMapName() const { return m_StageSetting.szMapName; }
	//MMatchMapDesc MGetBannerName() const { return m_StageSetting.szBannerName; }
	int GetMapIndex() const { return m_StageSetting.nMapIndex; }
	int GetRoundMax() const { return m_StageSetting.nRoundMax; }
	int GetLimitTime() const { return m_StageSetting.nLimitTime; }
	int GetLimitLevel() const { return m_StageSetting.nLimitLevel; }
	MUID GetMasterUID() const { return m_uidMaster; }
	STAGE_STATE GetStageState() const { return m_nStageState; }
	MMATCH_GAMETYPE GetGameType() const { return m_StageSetting.nGameType; }
	int GetMaxPlayers() const { return m_StageSetting.nMaxPlayers; }
	bool GetForcedEntry() const { return m_StageSetting.bForcedEntryEnabled; }
	bool GetAutoTeamBalancing() const { return m_StageSetting.bAutoTeamBalancing; }
	auto GetNetcode() const { return m_StageSetting.Netcode; }
	bool IsForcedHPAP() const { return m_StageSetting.ForceHPAP; }
	int GetForcedHP() const { return m_StageSetting.HP; }
	int GetForcedAP() const { return m_StageSetting.AP; }
	bool CanFlip() const { return !m_StageSetting.NoFlip; }
	bool IsSwordsOnly() const { return m_StageSetting.SwordsOnly; }
	bool IsVanillaMode() const { return m_StageSetting.VanillaMode; }
	bool InvulnerabilityStates() const { return m_StageSetting.InvulnerabilityStates; }
	MSTAGE_SETTING_NODE* GetStageSetting()			{ return &m_StageSetting; }
	const MMatchGameTypeInfo* GetCurrGameTypeInfo();

	// Set
	void SetMasterUID(const MUID& uid)		{ m_uidMaster = uid; }
	void SetMapName(const char* pszName);
	//void SetBannerName(MMatchMapDesc type)	{ m_StageSetting.szBannerName = type; }
	void SetMapIndex(int nMapIndex);
	void SetRoundMax(int nRound)			{ m_StageSetting.nRoundMax = nRound; }
	void SetLimitTime(int nTime)			{ m_StageSetting.nLimitTime = nTime; }
	void SetGameType(MMATCH_GAMETYPE type)	{ m_StageSetting.nGameType = type; }
	void SetStageState(STAGE_STATE nState)	{ m_nStageState = nState; }
	void SetTeamWinThePoint(bool bValue)	{ m_StageSetting.bTeamWinThePoint = bValue; }
	void SetAutoTeamBalancing(bool bValue)	{ m_StageSetting.bAutoTeamBalancing = bValue; }
	void SetNetcode(NetcodeType NewNetcode) { m_StageSetting.Netcode = NewNetcode; }
	
	void UpdateStageSetting(MSTAGE_SETTING_NODE* pSetting);
	void UpdateCharSetting(const MUID& uid, unsigned int nTeam, MMatchObjectStageState nStageState);

	void ResetCharSetting() { m_CharSettingList.clear(); }
	bool IsTeamPlay();
	bool IsWaitforRoundEnd();
	bool IsQuestDrived();
	bool IsTeamWinThePoint() const { return m_StageSetting.bTeamWinThePoint; }

	MStageCharSettingList	m_CharSettingList;

protected:
	MSTAGE_SETTING_NODE		m_StageSetting;
	MUID					m_uidMaster;
	STAGE_STATE				m_nStageState;
};