#include "stdafx.h"
#include "MBaseGameType.h"
#include "MMatchMap.h"

#define MMATCH_GAMETYPE_DEATHMATCH_SOLO_STR		"Death Match(개인)"
#define MMATCH_GAMETYPE_DEATHMATCH_TEAM_STR		"Death Match(단체)"
#define MMATCH_GAMETYPE_GLADIATOR_SOLO_STR		"Gladiator(개인)"
#define MMATCH_GAMETYPE_GLADIATOR_TEAM_STR		"Gladiator(단체)"
#define MMATCH_GAMETYPE_ASSASSINATE_STR			"암살전"
#define MMATCH_GAMETYPE_TRAINING_STR			"트레이닝"
#define MMATCH_GAMETYPE_CLASSIC_SOLO_STR		"클래식(개인)"
#define MMATCH_GAMETYPE_CLASSIC_TEAM_STR		"클래식(단체)"
#define MMATCH_GAMETYPE_SURVIVAL_STR			"서바이벌"
#define MMATCH_GAMETYPE_QUEST_STR				"퀘스트"
#define MMATCH_GAMETYPE_BERSERKER_STR			"버서커"
#define MMATCH_GAMETYPE_DEATHMATCH_TEAM2_STR	"데스매치(단체 무한)"
#define MMATCH_GAMETYPE_DUEL_STR		"Duel"

void MMatchGameTypeInfo::Set(const MMATCH_GAMETYPE a_nGameTypeID, const char* a_szGameTypeStr, const float a_fGameExpRatio,
		    const float a_fTeamMyExpRatio, const float a_fTeamBonusExpRatio)
{
	MMatchGameTypeInfo::nGameTypeID = a_nGameTypeID;
	strcpy_safe(MMatchGameTypeInfo::szGameTypeStr, a_szGameTypeStr);
	MMatchGameTypeInfo::fGameExpRatio = a_fGameExpRatio;
	MMatchGameTypeInfo::fTeamMyExpRatio = a_fTeamMyExpRatio;
	MMatchGameTypeInfo::fTeamBonusExpRatio = a_fTeamBonusExpRatio;
}

void MMatchGameTypeInfo::AddMap(int nMapID)
{
	MapSet.insert(nMapID);
}

void MMatchGameTypeInfo::AddAllMap()
{
	for (int i = 0; i < MMATCH_MAP_MAX; i++)
	{
		AddMap(i);
	}
}

MBaseGameTypeCatalogue::MBaseGameTypeCatalogue()
{
#define _InitGameType(index, id, szGameTypeStr, fGameExpRatio, fTeamMyExpRatio, fTeamBonusExpRatio)		\
m_GameTypeInfo[index].Set(id, szGameTypeStr, fGameExpRatio, fTeamMyExpRatio, fTeamBonusExpRatio);

// index,		id,									게임타입이름,	경험치배분 비율, 팀전 개인 경험치 배분율, 팀전 팀 경험치 배분율
_InitGameType(0, MMATCH_GAMETYPE_DEATHMATCH_SOLO,	MMATCH_GAMETYPE_DEATHMATCH_SOLO_STR, 1.0f,	1.0f,	0.0f);
_InitGameType(1, MMATCH_GAMETYPE_DEATHMATCH_TEAM,	MMATCH_GAMETYPE_DEATHMATCH_TEAM_STR, 1.0f,	0.8f,	0.3f);
_InitGameType(2, MMATCH_GAMETYPE_GLADIATOR_SOLO,	MMATCH_GAMETYPE_GLADIATOR_SOLO_STR,  0.5f,	1.0f,	0.0f);
_InitGameType(3, MMATCH_GAMETYPE_GLADIATOR_TEAM,	MMATCH_GAMETYPE_GLADIATOR_TEAM_STR,  0.5f,	0.8f,	0.3f);
_InitGameType(4, MMATCH_GAMETYPE_ASSASSINATE,		MMATCH_GAMETYPE_ASSASSINATE_STR,	 1.0f,	0.8f,	0.3f);
_InitGameType(5, MMATCH_GAMETYPE_TRAINING,			MMATCH_GAMETYPE_TRAINING_STR,		 0.0f,	0.0f,	0.0f);

#ifdef _QUEST
_InitGameType(MMATCH_GAMETYPE_SURVIVAL, MMATCH_GAMETYPE_SURVIVAL,	MMATCH_GAMETYPE_SURVIVAL_STR,	0.0f,	0.0f,	0.0f);
_InitGameType(MMATCH_GAMETYPE_QUEST,	MMATCH_GAMETYPE_QUEST,		MMATCH_GAMETYPE_QUEST_STR,		0.0f,	0.0f,	0.0f);
#endif

_InitGameType(MMATCH_GAMETYPE_BERSERKER,	MMATCH_GAMETYPE_BERSERKER, MMATCH_GAMETYPE_BERSERKER_STR,		1.0f,	1.0f,	0.0f);

_InitGameType(MMATCH_GAMETYPE_DEATHMATCH_TEAM2,	MMATCH_GAMETYPE_DEATHMATCH_TEAM2, MMATCH_GAMETYPE_DEATHMATCH_TEAM2_STR,		1.0f,	0.6f,	0.5f);

_InitGameType(MMATCH_GAMETYPE_DUEL, MMATCH_GAMETYPE_DUEL, MMATCH_GAMETYPE_DUEL_STR, 1.0f, 1.0f, 0.0f);

_InitGameType(MMATCH_GAMETYPE_SKILLMAP, MMATCH_GAMETYPE_SKILLMAP, "Skillmap", 1.0f, 1.0f, 0.0f);

_InitGameType(MMATCH_GAMETYPE_GUNGAME, MMATCH_GAMETYPE_GUNGAME, "GunGame", 1.0f, 1.0f, 0.0f);

/*
#ifdef _CLASSIC
_InitGameType(MMATCH_GAMETYPE_CLASSIC_SOLO, MMATCH_GAMETYPE_CLASSIC_SOLO,
			  MMATCH_GAMETYPE_CLASSIC_SOLO_STR,		 1.0f,	1.0f,	0.0f);
_InitGameType(MMATCH_GAMETYPE_CLASSIC_TEAM, MMATCH_GAMETYPE_CLASSIC_TEAM,
			  MMATCH_GAMETYPE_CLASSIC_TEAM_STR,		 1.0f,	0.8f,	0.3f);
#endif
*/

	// 이 게임타입에서 플레이 가능한 맵 - 현재는 모든 맵이 다 가능
	for (int i = 0; i < MMATCH_GAMETYPE_DUEL; i++)
	{
		m_GameTypeInfo[i].AddAllMap();
	}
}

MBaseGameTypeCatalogue::~MBaseGameTypeCatalogue()
{

}



