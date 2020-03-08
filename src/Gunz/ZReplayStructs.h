#pragma once

#include "ZCharacterStructs.h"

#pragma pack(push)
#pragma pack(1)

struct REPLAY_HEADER
{
	u32 Header;
	u32 ReplayBinaryVersion;
};

// RG replay version 2 to 3
struct REPLAY_HEADER_RG_V3
{
	u32 Header;
	u32 ReplayBinaryVersion;
	u32 ClientVersion;
	i64 Timestamp;
};

// RG replay version 4 and onwards
struct REPLAY_HEADER_RG
{
	u32 Header;
	u32 ReplayBinaryVersion;
	i64 Timestamp;
	u32 ClientVersionMajor;
	u32 ClientVersionMinor;
	u32 ClientVersionPatch;
	u32 ClientVersionRevision;
};

struct REPLAY_STAGE_SETTING_NODE
{
	MUID				uidStage;
	char				szStageName[64];
	char				szMapName[MAPNAME_LENGTH];
	i32					nMapIndex;
	MMATCH_GAMETYPE		nGameType;
	i32					nRoundMax;
	i32					nLimitTime;
	i32					nLimitLevel;
	i32					nMaxPlayers;
	bool				bTeamKillEnabled;
	bool				bTeamWinThePoint;
	bool				bForcedEntryEnabled;
	bool				bAutoTeamBalancing;
	NetcodeType			Netcode;
	bool				ForceHPAP;
	i32					HP;
	i32					AP;
	bool				NoFlip;
	bool				SwordsOnly;
	bool				VanillaMode;
	bool				InvulnerabilityStates;
	char				Reserved[46];
};

struct REPLAY_STAGE_SETTING_NODE_RG_V1
{
	MUID				uidStage;
	char				szStageName[64];
	char				szMapName[MAPNAME_LENGTH];
	i32					nMapIndex;
	MMATCH_GAMETYPE		nGameType;
	i32					nRoundMax;
	i32					nLimitTime;
	i32					nLimitLevel;
	i32					nMaxPlayers;
	bool				bTeamKillEnabled;
	bool				bTeamWinThePoint;
	bool				bForcedEntryEnabled;
	bool				bAutoTeamBalancing;
};

struct REPLAY_STAGE_SETTING_NODE_RG_V2
{
	MUID				uidStage;
	char				szStageName[64];
	char				szMapName[MAPNAME_LENGTH];
	i32					nMapIndex;
	MMATCH_GAMETYPE		nGameType;
	i32					nRoundMax;
	i32					nLimitTime;
	i32					nLimitLevel;
	i32					nMaxPlayers;
	bool				bTeamKillEnabled;
	bool				bTeamWinThePoint;
	bool				bForcedEntryEnabled;
	bool				bAutoTeamBalancing;
	NetcodeType			Netcode;
	bool				ForceHPAP;
	i32					HP;
	i32					AP;
	bool				NoFlip;
	bool				SwordsOnly;
};

struct REPLAY_STAGE_SETTING_NODE_OLD
{
	MUID				uidStage;
	char				szMapName[32];
	i32					nMapIndex;
	MMATCH_GAMETYPE		nGameType;
	i32					nRoundMax;
	i32					nLimitTime;
	i32					nLimitLevel;
	i32					nMaxPlayers;
	bool				bTeamKillEnabled;
	bool				bTeamWinThePoint;
	bool				bForcedEntryEnabled;

	char				pad;
};
struct REPLAY_STAGE_SETTING_NODE_V11
{
	MUID				uidStage;
	char				szMapName[32];
	char				unk[32];
	i32					nMapIndex;
	MMATCH_GAMETYPE		nGameType;
	i32					nRoundMax;
	i32					nLimitTime;
	i32					nLimitLevel;
	i32					nMaxPlayers;
	bool				bTeamKillEnabled;
	bool				bTeamWinThePoint;
	bool				bForcedEntryEnabled;

	char				pad;
};

struct REPLAY_STAGE_SETTING_NODE_FG
{
	MUID				uidStage;
	char				szMapName[32];
	i32					nMapIndex;
	MMATCH_GAMETYPE		nGameType;
	i32					nRoundMax;
	i32					nLimitTime;
	i32					nLimitLevel;
	i32					nMaxPlayers;
	bool				bTeamKillEnabled;
	bool				bTeamWinThePoint;
	bool				bForcedEntryEnabled;
	char				szStageName[64];

	char				unk;
};

struct REPLAY_STAGE_SETTING_NODE_DG
{
	MUID				uidStage;
	char				szMapName[32];
	i32					nMapIndex;
	MMATCH_GAMETYPE		nGameType;
	i32					nRoundMax;
	i32					nLimitTime;
	i32					nLimitLevel;
	i32					nMaxPlayers;
	bool				bTeamKillEnabled;
	bool				bTeamWinThePoint;
	bool				bForcedEntryEnabled;

	char				unk[5];
};

struct MTD_CharInfo_V5
{
	char				szName[32];
	char				szClanName[16];
	MMatchClanGrade		nClanGrade;
	u16					nClanContPoint;
	char				nCharNum;
	u16					nLevel;
	char				nSex;
	char				nHair;
	char				nFace;
	u32					nXP;
	i32					nBP;
	float				fBonusRate;
	u16					nPrize;
	u16					nHP;
	u16					nAP;
	u16					nMaxWeight;
	u16					nSafeFalls;
	u16					nFR;
	u16					nCR;
	u16					nER;
	u16					nWR;
	u32					nEquipedItemDesc[12];
	MMatchUserGradeID	nUGradeID;
	u32					nClanCLID;
};

struct MTD_CharInfo_V6
{
	char				szName[32];
	char				szClanName[16];
	MMatchClanGrade		nClanGrade;
	u16					nClanContPoint;
	char				nCharNum;
	u16					nLevel;
	char				nSex;
	char				nHair;
	char				nFace;
	u32					nXP;
	i32					nBP;
	float				fBonusRate;
	u16					nPrize;
	u16					nHP;
	u16					nAP;
	u16					nMaxWeight;
	u16					nSafeFalls;
	u16					nFR;
	u16					nCR;
	u16					nER;
	u16					nWR;
	u32					nEquipedItemDesc[17];
	MMatchUserGradeID	nUGradeID;
	u32					nClanCLID;
	i32					nDTLastWeekGrade;
	MUID				uidEquipedItem[17];
	u32					nEquipedItemCount[17];
};

struct MTD_CharInfo_V11
{
	char				szName[32];
	char				szClanName[16];
	MMatchClanGrade		nClanGrade;
	u16					nClanContPoint;
	char				nCharNum;
	u16					nLevel;
	char				nSex;
	char				nHair;
	char				nFace;
	u32					nXP;
	i32					nBP;
	float				fBonusRate;
	u16					nPrize;
	u16					nHP;
	u16					nAP;
	u16					nMaxWeight;
	u16					nSafeFalls;
	u16					nFR;
	u16					nCR;
	u16					nER;
	u16					nWR;
	u32					nEquipedItemDesc[17];
	MMatchUserGradeID	nUGradeID;
	u32					nClanCLID;
	i32					nDTLastWeekGrade;
	MUID				uidEquipedItem[17];
	u32					nEquipedItemCount[17];

	char				unk[8];
};

using MTD_CharInfo_FG_V7_0 = MTD_CharInfo_V6;

struct MTD_CharInfo_FG_V7_1
{
	char				szName[32];
	char				szClanName[16];
	MMatchClanGrade		nClanGrade;
	u16					nClanContPoint;
	char				nCharNum;
	u16					nLevel;
	char				nSex;
	char				nHair;
	char				nFace;
	u32					nXP;
	i32					nBP;
	float				fBonusRate;
	u16					nPrize;
	u16					nHP;
	u16					nAP;
	u16					nMaxWeight;
	u16					nSafeFalls;
	u16					nFR;
	u16					nCR;
	u16					nER;
	u16					nWR;
	u32					nEquipedItemDesc[22];
	MMatchUserGradeID	nUGradeID;
	u32					nClanCLID;
	i32					nDTLastWeekGrade;

	MUID				uidEquipedItem[22];
	u32					nEquipedItemCount[22];
	u32					nEquipedItemRarity[22];
	u32					nEquipedItemLevel[22];
};

struct MTD_CharInfo_FG_V8
{
	char				szName[32];
	char				szClanName[16];
	MMatchClanGrade		nClanGrade;
	u16					nClanContPoint;
	char				nCharNum;
	u16					nLevel;
	char				nSex;
	char				nHair;
	char				nFace;
	u32					nXP;
	i32					nBP;
	float				fBonusRate;
	u16					nPrize;
	u16					nHP;
	u16					nAP;
	u16					nMaxWeight;
	u16					nSafeFalls;
	u16					nFR;
	u16					nCR;
	u16					nER;
	u16					nWR;
	u32					nEquipedItemDesc[22];
	MMatchUserGradeID	nUGradeID;
	u32					nClanCLID;
	i32					nDTLastWeekGrade;

	u32					unk[6];

	MUID				uidEquipedItem[22];
	u32					nEquipedItemCount[22];
	u32					nEquipedItemRarity[22];
	u32					nEquipedItemLevel[22];
};

struct MTD_CharInfo_FG_V9
{
	char				szName[32];
	char				szClanName[16];
	MMatchClanGrade		nClanGrade;
	u16					nClanContPoint;
	char				nCharNum;
	u16					nLevel;
	char				nSex;
	char				nHair;
	char				nFace;
	u32					nXP;
	i32					nBP;
	float				fBonusRate;
	u16					nPrize;
	u16					nHP;
	u16					nAP;
	u16					nMaxWeight;
	u16					nSafeFalls;
	u16					nFR;
	u16					nCR;
	u16					nER;
	u16					nWR;
	u32					nEquipedItemDesc[22];
	MMatchUserGradeID	nUGradeID;
	u32					nClanCLID;
	i32					nDTLastWeekGrade;

	u32					unk[6];

	MUID				uidEquipedItem[22];
	u32					nEquipedItemCount[22];
	u32					nEquipedItemRarity[22];
	u32					nEquipedItemLevel[22];

	char				unk2[24];
};

struct MTD_CharInfo_FG_V10
{
	char				szName[32];
	char				szClanName[16];
	MMatchClanGrade		nClanGrade;
	u16					nClanContPoint;
	char				nCharNum;
	u16					nLevel;
	char				nSex;
	char				nHair;
	char				nFace;
	u32					nXP;
	i32					nBP;
	float				fBonusRate;
	u16					nPrize;
	u16					nHP;
	u16					nAP;
	u16					nMaxWeight;
	u16					nSafeFalls;
	u16					nFR;
	u16					nCR;
	u16					nER;
	u16					nWR;
	u32					nEquipedItemDesc[22];
	MMatchUserGradeID	nUGradeID;
	u32					nClanCLID;
	i32					nDTLastWeekGrade;

	u32					unk[6];

	MUID				uidEquipedItem[22];
	u32					nEquipedItemCount[22];
	u32					nEquipedItemRarity[22];
	u32					nEquipedItemLevel[22];

	char				unk2[24];
	char				unk3[24];
};

struct BulletInfo
{
	i32 Clip;
	i32 Magazine;
};

template<size_t NumItems>
struct ZCharacterReplayStateImpl
{
	MUID UID;
	ZCharacterProperty Property;
	float HP;
	float AP;
	ZCharacterStatus Status;

	BulletInfo BulletInfos[NumItems];

	rvector Position;
	rvector Direction;

	MMatchTeam Team;

	bool Dead;

	bool HidingAdmin;
};

struct ZCharacterReplayState
{
	MUID UID;
	ZCharacterProperty Property;
	float HP;
	float AP;
	ZCharacterStatus Status;

	BulletInfo BulletInfos[MMCIP_END];

	rvector Position;
	rvector Direction;

	MMatchTeam Team;

	bool Dead;

	bool HidingAdmin;

	u8 LowerAnimation;
	u8 UpperAnimation;
	u8 SelectedItem;
};

using ZCharacterReplayState_RG_V4 = ZCharacterReplayStateImpl<12>;
using ZCharacterReplayState_FG_V7_0 = ZCharacterReplayStateImpl<17>;
using ZCharacterReplayState_FG_V7_1 = ZCharacterReplayStateImpl<22>;
using ZCharacterReplayState_FG_V8 = ZCharacterReplayStateImpl<23>;
using ZCharacterReplayState_FG_V9 = ZCharacterReplayStateImpl<24>;
using ZCharacterReplayState_Official_V5 = ZCharacterReplayStateImpl<12>;
using ZCharacterReplayState_Official_V6 = ZCharacterReplayStateImpl<17>;
using ZCharacterReplayState_Official_V11 = ZCharacterReplayStateImpl<34>;

struct ZCharacterReplayState_FG_V10
{
	MUID UID;
	ZCharacterProperty Property;
	float HP;
	float AP;
	ZCharacterStatus Status;

	BulletInfo BulletInfos[24];

	char unk[20];

	rvector Position;
	rvector Direction;

	MMatchTeam Team;

	bool Dead;

	bool HidingAdmin;
};

template <typename CharInfo, typename ReplayState>
struct ReplayPlayerInfoImpl
{
	bool IsHero;
	CharInfo Info;
	ReplayState State;
};

struct MTD_CharInfo_DG
{
	char				szName[32];
	char				szClanName[16];
	MMatchClanGrade		nClanGrade;
	u16					nClanContPoint;
	char				nCharNum;
	u16					nLevel;
	u16					unk2;
	char				nSex;
	char				nHair;
	char				nFace;
	u32					nXP;
	i32					nBP;
	float				fBonusRate;
	u16					nPrize;
	u16					nHP;
	u16					nAP;
	u16					nMaxWeight;
	u16					nSafeFalls;
	u16					nFR;
	u16					nCR;
	u16					nER;
	u16					nWR;
	u32					nEquipedItemDesc[17];
	MMatchUserGradeID	nUGradeID;
	u32					nClanCLID;
	i32					nDTLastWeekGrade;

	MUID				uidEquipedItem[17];
	u32					nEquipedItemCount[17];

	char				unk[4];
};

struct ZCharacterReplayState_DG
{
	MUID UID;
	ZCharacterProperty Property;
	float HP;
	float AP;
	ZCharacterStatus Status;

	BulletInfo BulletInfos[17];

	char unk[8];

	rvector Position;
	rvector Direction;

	MMatchTeam Team;

	bool Dead;

	bool HidingAdmin;
};

struct ReplayPlayerInfo_DG
{
	bool IsHero;
	char unk[32];
	MTD_CharInfo_DG Info;
	ZCharacterReplayState_DG State;
};

using ReplayPlayerInfo = ReplayPlayerInfoImpl<MTD_CharInfo, ZCharacterReplayState>;
using ReplayPlayerInfo_RG_V4 = ReplayPlayerInfoImpl<MTD_CharInfo, ZCharacterReplayState_RG_V4>;

using ReplayPlayerInfo_FG_V7_0 = ReplayPlayerInfoImpl<MTD_CharInfo_FG_V7_0, ZCharacterReplayState_FG_V7_0>;
using ReplayPlayerInfo_FG_V7_1 = ReplayPlayerInfoImpl<MTD_CharInfo_FG_V7_1, ZCharacterReplayState_FG_V7_1>;
using ReplayPlayerInfo_FG_V8 = ReplayPlayerInfoImpl<MTD_CharInfo_FG_V8, ZCharacterReplayState_FG_V8>;
using ReplayPlayerInfo_FG_V9 = ReplayPlayerInfoImpl<MTD_CharInfo_FG_V9, ZCharacterReplayState_FG_V9>;
using ReplayPlayerInfo_FG_V10 = ReplayPlayerInfoImpl<MTD_CharInfo_FG_V10, ZCharacterReplayState_FG_V10>;
using ReplayPlayerInfo_Official_V5 = ReplayPlayerInfoImpl<MTD_CharInfo_V5, ZCharacterReplayState_Official_V5>;
using ReplayPlayerInfo_Official_V6 = ReplayPlayerInfoImpl<MTD_CharInfo_V6, ZCharacterReplayState_Official_V6>;
using ReplayPlayerInfo_Official_V11 = ReplayPlayerInfoImpl<MTD_CharInfo_V11,
	ZCharacterReplayState_Official_V11>;

#pragma pack(pop)