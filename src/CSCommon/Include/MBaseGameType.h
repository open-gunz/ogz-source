#pragma once

#include <map>
#include <set>

#define _QUEST

enum MMATCH_GAMETYPE : i32
{
	MMATCH_GAMETYPE_DEATHMATCH_SOLO		=0,
	MMATCH_GAMETYPE_DEATHMATCH_TEAM		=1,
	MMATCH_GAMETYPE_GLADIATOR_SOLO		=2,
	MMATCH_GAMETYPE_GLADIATOR_TEAM		=3,
	MMATCH_GAMETYPE_ASSASSINATE			=4,
	MMATCH_GAMETYPE_TRAINING			=5,

#ifdef _QUEST
	MMATCH_GAMETYPE_SURVIVAL			=6,
	MMATCH_GAMETYPE_QUEST				=7,
#endif

	MMATCH_GAMETYPE_BERSERKER			=8,
	MMATCH_GAMETYPE_DEATHMATCH_TEAM2	=9,
	MMATCH_GAMETYPE_DUEL = 10,
	MMATCH_GAMETYPE_SKILLMAP = 11,
	MMATCH_GAMETYPE_GUNGAME = 12,

	MMATCH_GAMETYPE_MAX,

	MMATCH_GAMETYPE_ALL = 100,
};

const MMATCH_GAMETYPE MMATCH_GAMETYPE_DEFAULT = MMATCH_GAMETYPE_DEATHMATCH_SOLO;


struct MMatchGameTypeInfo
{
	MMATCH_GAMETYPE		nGameTypeID;
	char				szGameTypeStr[128];
	float				fGameExpRatio;
	float				fTeamMyExpRatio;
	float				fTeamBonusExpRatio;
	std::set<int>			MapSet;
	void Set(const MMATCH_GAMETYPE a_nGameTypeID, const char* a_szGameTypeStr, const float a_fGameExpRatio,
		     const float a_fTeamMyExpRatio, const float a_fTeamBonusExpRatio);
	void AddMap(int nMapID);
	void AddAllMap();
};


class MBaseGameTypeCatalogue
{
private:
	MMatchGameTypeInfo			m_GameTypeInfo[MMATCH_GAMETYPE_MAX];
public:
	MBaseGameTypeCatalogue();
	virtual ~MBaseGameTypeCatalogue();

	inline MMatchGameTypeInfo* GetInfo(MMATCH_GAMETYPE nGameType);
	inline const char* GetGameTypeStr(MMATCH_GAMETYPE nGameType);
	inline void SetGameTypeStr(MMATCH_GAMETYPE nGameType, const char* szName);
	inline bool IsCorrectGameType(const int nGameTypeID);
	inline bool IsTeamGame(MMATCH_GAMETYPE nGameType);
	inline bool IsTeamLimitTime(MMATCH_GAMETYPE nGameType);
	inline bool IsWaitForRoundEnd(MMATCH_GAMETYPE nGameType);
	inline bool IsQuestOnly(MMATCH_GAMETYPE nGameType);
	inline bool IsQuestDerived(MMATCH_GAMETYPE nGameType);
	inline bool IsWorldItemSpawnEnable(MMATCH_GAMETYPE nGameType);
};

inline bool MBaseGameTypeCatalogue::IsTeamGame(MMATCH_GAMETYPE nGameType)
{
	if ((nGameType == MMATCH_GAMETYPE_DEATHMATCH_TEAM) ||
		(nGameType == MMATCH_GAMETYPE_DEATHMATCH_TEAM2) ||
		(nGameType == MMATCH_GAMETYPE_GLADIATOR_TEAM) ||
		(nGameType == MMATCH_GAMETYPE_ASSASSINATE) )
	{
		return true;
	}
	return false;
}

inline bool MBaseGameTypeCatalogue::IsTeamLimitTime(MMATCH_GAMETYPE nGameType)
{
	if ((nGameType == MMATCH_GAMETYPE_DEATHMATCH_TEAM) ||
		(nGameType == MMATCH_GAMETYPE_GLADIATOR_TEAM) ||
		(nGameType == MMATCH_GAMETYPE_DUEL) ||
		(nGameType == MMATCH_GAMETYPE_ASSASSINATE) )
	{
		return true;
	}
	return false;
}

inline bool MBaseGameTypeCatalogue::IsWaitForRoundEnd(MMATCH_GAMETYPE nGameType)
{
	if ((nGameType == MMATCH_GAMETYPE_DEATHMATCH_TEAM) ||
		(nGameType == MMATCH_GAMETYPE_DUEL) ||
		(nGameType == MMATCH_GAMETYPE_GLADIATOR_TEAM) ||
		(nGameType == MMATCH_GAMETYPE_ASSASSINATE) )
	{
		return true;
	}
	return false;
}


inline bool MBaseGameTypeCatalogue::IsQuestDerived(MMATCH_GAMETYPE nGameType)
{
#ifdef _QUEST
	if ( (nGameType == MMATCH_GAMETYPE_SURVIVAL) ||(nGameType == MMATCH_GAMETYPE_QUEST) )
	{
		return true;
	}
#endif
	return false;
}

inline bool MBaseGameTypeCatalogue::IsQuestOnly(MMATCH_GAMETYPE nGameType)
{
#ifdef _QUEST
	if ( nGameType == MMATCH_GAMETYPE_QUEST)
	{
		return true;
	}
#endif
	return false;
}

inline const char* MBaseGameTypeCatalogue::GetGameTypeStr(MMATCH_GAMETYPE nGameType)
{
	return m_GameTypeInfo[nGameType].szGameTypeStr;
}

inline void MBaseGameTypeCatalogue::SetGameTypeStr(MMATCH_GAMETYPE nGameType, const char* szName)
{
	strcpy_safe( m_GameTypeInfo[nGameType].szGameTypeStr, szName) ;
}

bool MBaseGameTypeCatalogue::IsCorrectGameType(const int nGameTypeID)
{
	if ((nGameTypeID < 0) || (nGameTypeID >= MMATCH_GAMETYPE_MAX)) return false;
	return true;
}

inline MMatchGameTypeInfo* MBaseGameTypeCatalogue::GetInfo(MMATCH_GAMETYPE nGameType)
{
	_ASSERT((nGameType >= 0) && (nGameType < MMATCH_GAMETYPE_MAX));
	return &m_GameTypeInfo[nGameType];
}

inline bool MBaseGameTypeCatalogue::IsWorldItemSpawnEnable(MMATCH_GAMETYPE nGameType)
{
#ifdef _QUEST
	if ( (nGameType == MMATCH_GAMETYPE_SURVIVAL) ||(nGameType == MMATCH_GAMETYPE_QUEST) )
	{
		return false;
	}
#endif
	return true;

}

static bool IsSwordsOnly(MMATCH_GAMETYPE GameType)
{
	return GameType == MMATCH_GAMETYPE_GLADIATOR_SOLO
		|| GameType == MMATCH_GAMETYPE_GLADIATOR_TEAM;
}