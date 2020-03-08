#pragma once

#include "MUID.h"
#include "MBaseLocale.h"
#include "GlobalTypes.h"
#include "Config.h"

#define MATCHOBJECT_NAME_LENGTH		32
#define MAX_CHAR_COUNT				4


#define CYCLE_STAGE_UPDATECHECKSUM	500

#define NUM_APPLYED_TEAMBONUS_TEAM_PLAYERS		3
#define RESPAWN_DELAYTIME_AFTER_DYING			7000

#define MAX_XP_BONUS_RATIO						2.0f
#define MAX_BP_BONUS_RATIO						2.0f

#define STAGENAME_LENGTH			64
#define STAGEPASSWD_LENGTH			8
#define STAGE_QUEST_MAX_PLAYER		4


#define TRANS_STAGELIST_NODE_COUNT				8
#define TRANS_STANDBY_CLANLIST_NODE_COUNT		4

#define MAX_REPLIER	16

//
// Clan
//
#ifdef SOLO_CLAN_CREATION
#define CLAN_SPONSORS_COUNT				0
#else
#define CLAN_SPONSORS_COUNT				4
#endif
#define CLAN_CREATING_NEED_BOUNTY		0 //1000
#define CLAN_CREATING_NEED_LEVEL		0 //10


#define CLAN_NAME_LENGTH			16
#define MIN_CLANNAME	4
#define MAX_CLANNAME	12

enum MMatchClanGrade : i32
{
	MCG_NONE = 0,
	MCG_MASTER = 1,
	MCG_ADMIN = 2,

	MCG_MEMBER = 9,
	MCG_END
};

inline bool IsUpperClanGrade(MMatchClanGrade nSrcGrade, MMatchClanGrade nDstGrade)
{
	if ((nSrcGrade != MCG_NONE) && ((int)nSrcGrade <= (int)nDstGrade)) return true;
	return false;
}

//
// Character
//
#define MIN_CHARNAME	3
#define MAX_CHARNAME	16
#define MAX_CHARNAME_LENGTH	24
#define MAX_CHAR_LEVEL	99

#define MATCH_SIMPLE_DESC_LENGTH	64

enum MMatchUserGradeID : i32
{
	MMUG_FREE = 0,
	MMUG_REGULAR = 1,
	MMUG_STAR = 2,

	MMUG_CRIMINAL = 100,
	MMUG_WARNING_1 = 101,
	MMUG_WARNING_2 = 102,
	MMUG_WARNING_3 = 103,
	MMUG_CHAT_LIMITED = 104,
	MMUG_PENALTY = 105,

	MMUG_VIP = 251,
	MMUG_EVENTMASTER = 252,
	MMUG_BLOCKED = 253,
	MMUG_DEVELOPER = 254,
	MMUG_ADMIN = 255
};

enum MMatchPlace
{
	MMP_OUTSIDE = 0,
	MMP_LOBBY = 1,
	MMP_STAGE = 2,
	MMP_BATTLE = 3,
	MMP_END
};

enum MMatchObjectStageState
{
	MOSS_NONREADY = 0,
	MOSS_READY = 1,
	MOSS_SHOP = 2,
	MOSS_EQUIPMENT = 3,
	MOSS_END
};

enum MMatchDisconnectStatus
{
	MMDS_CONNECTED = 1,
	MMDS_DISCONN_WAIT,
	MMDS_DISCONNECT,

	MMDS_END,
};

enum MMatchPremiumGradeID
{
	MMPG_FREE = 0,
	MMPG_PREMIUM_IP = 1
};

enum MMatchSex
{
	MMS_MALE = 0,
	MMS_FEMALE = 1
};

enum MMatchBlockType
{
	MMBT_NO = 0,
	MMBT_BANNED,
	MMBT_MUTED,

	MMBT_END,
};

enum MCmdEnterBattleParam
{
	MCEP_NORMAL = 0,
	MCEP_FORCED = 1,
	MCEP_END
};

#define DEFAULT_CHAR_HP				100
#define DEFAULT_CHAR_AP				0

inline bool IsAdminGrade(MMatchUserGradeID nGrade)
{
	if ((nGrade == MMUG_EVENTMASTER) ||
		(nGrade == MMUG_ADMIN) ||
		(nGrade == MMUG_DEVELOPER))
		return true;

	return false;
}

struct MINITIALCOSTUME
{
	u32 nMeleeItemID;
	u32 nPrimaryItemID;
	u32 nSecondaryItemID;
	u32 nCustom1ItemID;
	u32 nCustom2ItemID;

	u32 nChestItemID;
	u32 nHandsItemID;
	u32 nLegsItemID;
	u32 nFeetItemID;
};

#define MAX_COSTUME_TEMPLATE		6
const MINITIALCOSTUME g_InitialCostume[MAX_COSTUME_TEMPLATE][2] =
{
	{
		{ 1, 5001, 4001, 30301, 0,     21001, 0, 23001, 0 },
		{ 1, 5001, 4001, 30301, 0,     21501, 0, 23501, 0 }
	},
	{
		{ 2, 5002, 0,    30301, 0,     21001, 0, 23001, 0 },
		{ 2, 5002, 0,    30301, 0,     21501, 0, 23501, 0 }
	},
	{
		{ 1, 4005, 5001, 30401, 0,     21001, 0, 23001, 0 },
		{ 1, 4005, 5001, 30401, 0,     21501, 0, 23501, 0 }
	},
	{
		{ 2, 4001, 0,    30401, 0,     21001, 0, 23001, 0 },
		{ 2, 4001, 0,    30401, 0,     21501, 0, 23501, 0 }
	},
	{
		{ 2, 4002, 0,    30401, 30001, 21001, 0, 23001, 0 },
		{ 2, 4002, 0,    30401, 30001, 21501, 0, 23501, 0 }
	},
	{
		{ 1, 4006, 0,	 30101, 30001, 21001, 0, 23001, 0 },
		{ 1, 4006, 4006, 30101, 30001, 21501, 0, 23501, 0 }
	}
};

#define MAX_COSTUME_HAIR		5
const std::string g_szHairMeshName[MAX_COSTUME_HAIR][2] =
{
	{ "eq_head_01", "eq_head_pony" },
	{ "eq_head_02", "eq_head_hair001" },
	{ "eq_head_08", "eq_head_hair04" },
	{ "eq_head_05", "eq_head_hair006" },
	{ "eq_head_08", "eq_head_hair002" }
};

#define MAX_COSTUME_FACE		20
const std::string g_szFaceMeshName[MAX_COSTUME_FACE][2] =
{
	{ "eq_face_01", "eq_face_001" },
	{ "eq_face_02", "eq_face_002" },
	{ "eq_face_04", "eq_face_003" },
	{ "eq_face_05", "eq_face_004" },
	{ "eq_face_a01", "eq_face_001" },
	{ "eq_face_newface01", "eq_face_newface01" },
	{ "eq_face_newface02", "eq_face_newface02" },
	{ "eq_face_newface03", "eq_face_newface03" },
	{ "eq_face_newface04", "eq_face_newface04" },
	{ "eq_face_newface05", "eq_face_newface05" },
	{ "eq_face_newface06", "eq_face_newface06" },
	{ "eq_face_newface07", "eq_face_newface07" },
	{ "eq_face_newface08", "eq_face_newface08" },
	{ "eq_face_newface09", "eq_face_newface09" },
	{ "eq_face_newface10", "eq_face_newface10" },
	{ "eq_face_newface11", "eq_face_newface11" },
	{ "eq_face_newface12", "eq_face_newface12" },
	{ "eq_face_newface13", "eq_face_newface13" },
	{ "eq_face_newface13", "eq_face_newface14" },
	{ "eq_face_newface13", "eq_face_newface15" },
};

//
// Clan war
//
#define ACTIONLEAGUE_TEAM_MEMBER_COUNT		4
#define MAX_LADDER_TEAM_MEMBER				4
#define MAX_CLANBATTLE_TEAM_MEMBER			8

#define CLAN_BATTLE

//
// Channel
//
#define CHANNELNAME_LEN		64
#define CHANNELRULE_LEN		64
#define DEFAULT_CHANNEL_MAXPLAYERS			200
#define DEFAULT_CHANNEL_MAXSTAGES			100
#define MAX_CHANNEL_MAXSTAGES				500
#define NUM_PLAYERLIST_NODE					6
#define CHANNEL_NO_LEVEL					(-1)

enum MCHANNEL_TYPE {
	MCHANNEL_TYPE_PRESET = 0,
	MCHANNEL_TYPE_USER = 1,
	MCHANNEL_TYPE_PRIVATE = 2,
	MCHANNEL_TYPE_CLAN = 3,
	MCHANNEL_TYPE_MAX
};

struct MCHANNELLISTNODE {
	MUID			uidChannel;
	short			nNo;
	unsigned char	nPlayers;
	short			nMaxPlayers;
	short			nLevelMin;
	short			nLevelMax;
	char			nChannelType;
	char			szChannelName[CHANNELNAME_LEN];
};

//
// Round
//
enum MMATCH_ROUNDSTATE {
	MMATCH_ROUNDSTATE_PREPARE = 0,
	MMATCH_ROUNDSTATE_COUNTDOWN = 1,
	MMATCH_ROUNDSTATE_PLAY = 2,
	MMATCH_ROUNDSTATE_FINISH = 3,
	MMATCH_ROUNDSTATE_EXIT = 4,
	MMATCH_ROUNDSTATE_FREE = 5,
	MMATCH_ROUNDSTATE_FAILED = 6,
	MMATCH_ROUNDSTATE_END
};

enum MMATCH_ROUNDRESULT {
	MMATCH_ROUNDRESULT_DRAW = 0,
	MMATCH_ROUNDRESULT_REDWON,
	MMATCH_ROUNDRESULT_BLUEWON,
	MMATCH_ROUNDRESULT_END
};

enum MMatchTeam
{
	MMT_ALL			= 0,
	MMT_SPECTATOR	= 1,
	MMT_RED			= 2,
	MMT_BLUE		= 3,
	MMT_END
};

enum MMatchServerMode
{
	MSM_NORMAL_		= 0,
	MSM_CLAN		= 1,
	MSM_LADDER		= 2,
	MSM_EVENT		= 3,
	MSM_TEST		= 4,
	MSM_MAX,

	MSM_ALL			= 100,
};

inline const char* ToString(MMatchServerMode Mode)
{
	switch (Mode)
	{
	case MSM_NORMAL_: return "normal";
	case MSM_CLAN: return "clan";
	case MSM_LADDER: return "ladder";
	case MSM_EVENT: return "event";
	case MSM_TEST: return "test";
	}
	assert(false);
	return nullptr;
}

enum MMatchProposalMode
{
	MPROPOSAL_NONE = 0,
	MPROPOSAL_LADDER_INVITE,
	MPROPOSAL_CLAN_INVITE,
	MPROPOSAL_END
};

enum MLADDERTYPE {
	MLADDERTYPE_NORMAL_2VS2		= 0,
	MLADDERTYPE_NORMAL_3VS3,
	MLADDERTYPE_NORMAL_4VS4,
//	MLADDERTYPE_NORMAL_8VS8,
	MLADDERTYPE_MAX
};

const int g_nNeedLadderMemberCount[MLADDERTYPE_MAX] = {	2, 3, 4/*, 8*/};

#define DEFAULT_CLAN_POINT			1000
#define DAY_OF_DELETE_CLAN			7
#define MAX_WAIT_CLAN_DELETE_HOUR	24
#define UNDEFINE_DELETE_HOUR		2000000000

enum MMatchClanDeleteState
{
	MMCDS_NORMAL = 1,
	MMCDS_WAIT,
	MMCDS_DELETE,

	MMCDS_END,
};

enum MBITFLAG_USEROPTION {
	MBITFLAG_USEROPTION_REJECT_WHISPER	= 1,
	MBITFLAG_USEROPTION_REJECT_INVITE	= 1<<1
};

#define MAX_QUEST_MAP_SECTOR_COUNT					16
#define MAX_QUEST_NPC_INFO_COUNT					8


#define ALL_PLAYER_NOT_READY					1
#define QUEST_START_FAILED_BY_SACRIFICE_SLOT	2

#define MIN_QUESTITEM_ID							200001
#define MAX_QUESTITEM_ID							299999

// Keeper Manager Schedule
enum KMS_SCHEDULE_TYPE
{
	KMST_NO = 0,
	KMST_REPEAT,
	KMST_COUNT,
	KMST_ONCE,

	KMS_SCHEDULE_TYPE_END,
};

enum KMS_COMMAND_TYPE
{
	KMSC_NO = 0,
	KMSC_ANNOUNCE,
	KMSC_STOP_SERVER,
	KMSC_RESTART_SERVER,
	
	KMS_COMMAND_TYPE_END,
};

enum SERVER_STATE_KIND
{
	SSK_OPENDB = 0,

	SSK_END,
};

enum SERVER_ERR_STATE
{
	SES_NO = 0,
	SES_ERR_DB,
    
	SES_END,
};

enum SERVER_TYPE
{
	ST_NULL = 0,
	ST_DEBUG,
	ST_NORMAL,
	ST_CLAN,
	ST_QUEST,
	ST_EVENT,
};

enum MMatchBlockLevel
{
	MMBL_NO = 0,
	MMBL_ACCOUNT,
	MMBL_LOGONLY,

	MMBL_END,
};

inline MMatchTeam NegativeTeam(MMatchTeam nTeam)
{
	if (nTeam == MMT_RED) return MMT_BLUE;
	else if (nTeam == MMT_BLUE) return MMT_RED;
	return nTeam;
}
