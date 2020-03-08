#pragma once

#include <list>
#include "ZPrerequisites.h"
#include "MUID.h"
#include "MMatchGlobal.h"

class ZCharacter;
class MMatchStageSetting;
class ZRule;

class ZMatch
{
private:
protected:
	ZRule*					m_pRule{};
	MMatchStageSetting*		m_pStageSetting;
	MMATCH_ROUNDSTATE		m_nRoundState = MMATCH_ROUNDSTATE_PREPARE;
	int						m_nCurrRound{};
	int						m_nTeamScore[MMT_END]{};
	int						m_nRoundKills{};
	int						m_nTeamKillCount[MMT_END]{};

	u64	m_nNowTime{};
	u64	m_nStartTime{};
	u64	m_nLastDeadTime{};
	u64	m_nSoloSpawnTime{};
	u64	m_dwStartTime{};

	void SoloSpawn();
	void InitCharactersProperties();
	void InitRound();
	void ProcessRespawn();
public:
	ZMatch();

	bool Create();
	void Destroy();

	void Update(float fDelta);
	bool OnCommand(MCommand* pCommand);
	void OnResponseRuleInfo(MTD_RuleInfo* pInfo);
	void SetRound(int nCurrRound);
	void OnForcedEntry(ZCharacter* pCharacter);
	void InitCharactersPosition();
	void OnDrawGameMessage();
	void RespawnSolo(bool bForce = false);

	int GetRemainedSpawnTime();
	int GetRoundCount(); 
	int GetRoundReadyCount(void);
	void GetTeamAliveCount(int* pnRedTeam, int* pnBlueTeam);
	const char* GetTeamName(int nTeamID);
	void SetRoundState(MMATCH_ROUNDSTATE nRoundState, int nArg=0);

	void SetRoundStartTime( void);
	DWORD GetRemaindTime( void);

	inline int GetCurrRound();
	inline bool IsTeamPlay();
	inline bool IsWaitForRoundEnd();
	inline bool IsQuestDrived();
	inline MMATCH_ROUNDSTATE GetRoundState();
	inline MMATCH_GAMETYPE GetMatchType();
	inline bool GetTeamKillEnabled();
	inline int GetTeamScore(MMatchTeam nTeam);
	inline void SetTeamScore(MMatchTeam nTeam, int nScore);
	inline int GetTeamKills(MMatchTeam nTeam);
	inline void AddTeamKills(MMatchTeam nTeam, int amount = 1);
	inline void SetTeamKills(MMatchTeam nTeam, int amount);
	inline int GetRoundKills();
	inline void AddRoundKills();
	inline bool IsRuleGladiator();
	inline ZRule* GetRule();
};

#define DEFAULT_ONETURN_GAMETIME		300000
#define DEFAULT_WAITTIME				120000
#define DEFAULT_READY_TIME				5000

inline int ZMatch::GetCurrRound() 
{ 
	return m_nCurrRound; 
}
inline bool ZMatch::IsTeamPlay() 
{ 
	return m_pStageSetting->IsTeamPlay(); 
}

inline bool ZMatch::IsWaitForRoundEnd() 
{ 
	return m_pStageSetting->IsWaitforRoundEnd(); 
}

inline MMATCH_ROUNDSTATE ZMatch::GetRoundState()
{ 
	return m_nRoundState; 
}
inline MMATCH_GAMETYPE ZMatch::GetMatchType()
{ 
	return m_pStageSetting->GetStageSetting()->nGameType; 
}
inline bool ZMatch::GetTeamKillEnabled()
{ 
	return m_pStageSetting->GetStageSetting()->bTeamKillEnabled; 
}
inline int ZMatch::GetTeamScore(MMatchTeam nTeam)
{ 
	return m_nTeamScore[nTeam]; 
}

inline int ZMatch::GetTeamKills(MMatchTeam nTeam)
{ 
	return m_nTeamKillCount[nTeam]; 
}

inline void ZMatch::AddTeamKills(MMatchTeam nTeam, int amount)
{ 
	m_nTeamKillCount[nTeam]+=amount; 
}

inline void ZMatch::SetTeamKills(MMatchTeam nTeam, int amount)
{ 
	m_nTeamKillCount[nTeam]=amount; 
}

inline void ZMatch::SetTeamScore(MMatchTeam nTeam, int nScore)
{ 
	m_nTeamScore[nTeam] = nScore; 
}
inline int ZMatch::GetRoundKills()
{ 
	return m_nRoundKills; 
}
inline void ZMatch::AddRoundKills()
{ 
	m_nRoundKills++; 
}
inline bool ZMatch::IsRuleGladiator()
{
	return IsSwordsOnly(GetMatchType())
		|| m_pStageSetting->IsSwordsOnly();
}
inline ZRule* ZMatch::GetRule() 
{ 
	return m_pRule; 
}

inline bool ZMatch::IsQuestDrived()
{
	return m_pStageSetting->IsQuestDrived();
}