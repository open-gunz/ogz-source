#pragma once

#include "ZRuleDuel.h"
#include "has_xxx.h"
#include "Arena.h"
#include "defer.h"

template <typename HeaderType, typename StageSettingType, typename PlayerInfoType>
inline bool ZReplayLoader::IsVersion()
try
{
	const auto OldPosition = Position;
	Position = 0;
	DEFER([&] { Position = OldPosition; });

	HeaderType Header;
	Read(Header);

	StageSettingType Setting;
	Read(Setting);

	if (Setting.nGameType < MMATCH_GAMETYPE_DEATHMATCH_SOLO) {
		return false;
	}

	const auto IsRGReplay = Header.Header != RG_REPLAY_MAGIC_NUMBER;
	if (IsRGReplay)
	{
		if (Setting.nGameType > MMATCH_GAMETYPE_MAX) {
			return false;
		}
	}
	else
	{
		// This isn't an RG replay, so any gametype above duel must be either a corrupt value or a
		// custom gametype (and we can't deal with custom gametypes at all right now) so just
		// return false.
		if (Setting.nGameType > MMATCH_GAMETYPE_DUEL) {
			return false;
		}
	}

	if (Setting.nGameType == MMATCH_GAMETYPE_DUEL)
	{
		MTD_DuelQueueInfo DuelQueueInfo;
		Read(DuelQueueInfo);
	}

	int PlayerCount = 0;
	Read(PlayerCount);

	if (PlayerCount <= 0 || PlayerCount > 64) {
		return false;
	}

	bool HasFoundHero = false;
	for (int i = 0; i < PlayerCount; ++i)
	{
		PlayerInfoType PlayerInfo;
		Read(PlayerInfo);

		if (PlayerInfo.IsHero)
		{
			if (!HasFoundHero) {
				HasFoundHero = true;
			} else {
				// Can't have two heroes.
				return false;
			}
		}

		auto Equals = [&](auto&& a, auto&& b) { return strcmp(a, b) == 0; };

		// Slightly shorter aliases for brevity.
		auto&& Info = PlayerInfo.Info;
		auto&& State = PlayerInfo.State;
		auto&& Property = PlayerInfo.State.Property;

		if (!Equals(Info.szName,     Property.szName) ||
			!Equals(Info.szClanName, Property.szClanName)) {
			return false;
		}

		if (Info.nLevel != Property.nLevel ||
			Info.nSex   != Property.nSex   ||
			Info.nHair  != Property.nHair  ||
			Info.nFace  != Property.nFace) {
			return false;
		}

		if (State.Team < MMT_ALL || State.Team >= MMT_END) {
			return false;
		}
	}

	return true;
}
catch (EOFException&)
{
	return false;
}

struct KnownReplayVersion
{
	ReplayVersion MinVersion;
	ReplayVersion MaxVersion;

	bool (ZReplayLoader::*IsVersion)();
};

inline ReplayVersion ZReplayLoader::GetVersion()
{
	// Distinguishing the replay version, in terms of both version number and which server it
	// belongs to, is problematic because the only indication of version in the replay itself is
	// the first 8 bytes: A 4 byte magic number, and a 4 byte version number. Pretty much every
	// server (except RG) uses the same magic number (GUNZ_REC_FILE_ID = 0x95b1308a), and several
	// different servers use the same version number even though they have different replay
	// structures.
	//
	// For instance, Freestyle Gunz initially had the version incremented to 7 for their custom
	// replays, up from the default V6 in the 1.5 source. However, after the release of the source
	// in late 2011, the official game kept getting updates on late Ijji and through Aeria, so they
	// ended up making *their own* V7 replay format as well, which was different.
	// In addition to that, Freestyle Gunz *also* updated their replay format again without
	// changing the version, so now, a version number of "7" has to be distinguished between
	// 1) official V7, 2) old FG V7, and 3) new FG V7.
	//
	// To distinguish replays in the most effective and simple way, what we do here is have a list
	// of known replay versions tied to the replay data structures that they use.
	// Then, when we read the replay version number from the replay file, we go through the list
	// and check whether any of them make sense with the data.
	//
	// If the version doesn't match, that's an immediate rejection. If it does, we go a bit further
	// and call ZReplayLoader::IsVersion, instantiated with the replay structures of the version
	// being tested.
	//
	// IsVersion then reads data from the replay and checks it for logical consistency.
	// For instance, the gametype in the stage setting is verified to be within valid bounds.
	// Since there are several resources of redundant data (e.g. each player's name is stored both
	// in their MTD_CharInfo *and* their ZCharacterProperty), we also read both of those fields and
	// check that they are both valid and the same.
	//
	// If no versions are matched, or more than one version is matched, GetVersion returns
	// ServerType::None along with the version number, indicating that it's unrecognized, but still
	// letting the user know the version number. If only one is matched, that one is returned.
	//
	// This process is done even if the version number is unique (i.e. there's only one server it
	// could've originated from), just to verify that the replay is not corrupt and is as expected.

#define VER_FULL(Server, MinVer, MinSub, MaxVer, MaxSub, Header, Setting, Player) \
		{   {ServerType::Server, MinVer, MinSub}, \
			{ServerType::Server, MaxVer, MaxSub}, \
			&ZReplayLoader::IsVersion<Header, Setting, Player> }

#define VER_RANGE(Server, MinVer, MaxVer, Header, Setting, Player) \
		VER_FULL(Server, MinVer, 0, MaxVer, 0, Header, Setting, Player)

#define VER(Server, Ver, Header, Setting, Player) \
		VER_FULL(Server, Ver, 0, Ver, 0, Header, Setting, Player)

#define VER_SUB(Server, Ver, SubVer, Header, Setting, Player) \
		VER_FULL(Server, Ver, SubVer, Ver, SubVer, Header, Setting, Player)

	static const KnownReplayVersion KnownVersions[] = {
		// Official
		VER_RANGE(Official,      1, 5,  REPLAY_HEADER,       REPLAY_STAGE_SETTING_NODE_OLD,   ReplayPlayerInfo_Official_V5),
		VER      (Official,      6,     REPLAY_HEADER,       REPLAY_STAGE_SETTING_NODE_OLD,   ReplayPlayerInfo_Official_V6),
		VER_RANGE(Official,      7, 10, REPLAY_HEADER,       REPLAY_STAGE_SETTING_NODE_V11,   ReplayPlayerInfo_Official_V6),
		VER      (Official,      11,    REPLAY_HEADER,       REPLAY_STAGE_SETTING_NODE_V11,   ReplayPlayerInfo_Official_V11),

		// Refined Gunz
		VER      (RefinedGunz,   1,     REPLAY_HEADER,       REPLAY_STAGE_SETTING_NODE_RG_V1, ReplayPlayerInfo),
		VER      (RefinedGunz,   2,     REPLAY_HEADER_RG_V3, REPLAY_STAGE_SETTING_NODE_RG_V2, ReplayPlayerInfo),
		VER      (RefinedGunz,   3,     REPLAY_HEADER_RG_V3, REPLAY_STAGE_SETTING_NODE,       ReplayPlayerInfo),
		VER      (RefinedGunz,   4,     REPLAY_HEADER_RG,    REPLAY_STAGE_SETTING_NODE,       ReplayPlayerInfo_RG_V4),
		VER      (RefinedGunz,   5,     REPLAY_HEADER_RG,    REPLAY_STAGE_SETTING_NODE,       ReplayPlayerInfo),

		// Freestyle Gunz
		VER      (FreestyleGunz, 7,     REPLAY_HEADER,       REPLAY_STAGE_SETTING_NODE_FG,    ReplayPlayerInfo_FG_V7_0),
		VER_SUB  (FreestyleGunz, 7, 1,  REPLAY_HEADER,       REPLAY_STAGE_SETTING_NODE_FG,    ReplayPlayerInfo_FG_V7_1),
		VER      (FreestyleGunz, 8,     REPLAY_HEADER,       REPLAY_STAGE_SETTING_NODE_FG,    ReplayPlayerInfo_FG_V8),
		VER      (FreestyleGunz, 9,     REPLAY_HEADER,       REPLAY_STAGE_SETTING_NODE_FG,    ReplayPlayerInfo_FG_V9),
		VER      (FreestyleGunz, 10,    REPLAY_HEADER,       REPLAY_STAGE_SETTING_NODE_FG,    ReplayPlayerInfo_FG_V10),

		// Dark Gunz
		VER      (DarkGunz,      6,     REPLAY_HEADER,       REPLAY_STAGE_SETTING_NODE_DG,    ReplayPlayerInfo_DG),
	};

	ReplayVersion Version;

	bool FoundServer = false;

	u32 header;
	Read(header);

	u32 version;
	Read(version);

	Version.nVersion = version;
	Version.nSubVersion = 0;

	if (header == RG_REPLAY_MAGIC_NUMBER)
	{
		Version.Server = ServerType::RefinedGunz;
		FoundServer = true;
	}
	else if (header != GUNZ_REC_FILE_ID)
	{
		Version.Server = ServerType::None;
		return Version;
	}

	if (!FoundServer)
	{
		const KnownReplayVersion* LastMatchedVersion;

		for (auto&& KnownVersion : KnownVersions)
		{
			if (Version.nVersion < KnownVersion.MinVersion.nVersion ||
				Version.nVersion > KnownVersion.MaxVersion.nVersion)
				continue;

			auto ret = std::invoke(KnownVersion.IsVersion, this);
			if (!ret)
				continue;

			if (FoundServer)
			{
				auto ToStr = [&](auto&& Ver) {
					if (Ver.MinVersion == Ver.MaxVersion)
						return Ver.MinVersion.GetVersionString();

					auto Min = Ver.MinVersion.GetVersionString();
					auto Max = Ver.MaxVersion.GetVersionString();

					return "["s + Min + ", "s + Max + "]";
				};

				MLog("Replay matches both %s and %s; can't disambiguate.\n",
					ToStr(*LastMatchedVersion).c_str(), ToStr(KnownVersion).c_str());

				Version.Server = ServerType::None;
				return Version;
			}

			FoundServer = true;
			LastMatchedVersion = &KnownVersion;
		}

		if (!FoundServer)
		{
			MLog("Couldn't match any replay version\n");

			Version.Server = ServerType::None;
			return Version;
		}

		Version.Server = LastMatchedVersion->MinVersion.Server;
		Version.nSubVersion = LastMatchedVersion->MinVersion.nSubVersion;
	}

	if (Version.Server == ServerType::RefinedGunz && Version.nVersion >= 2)
	{
		if (Version.nVersion <= 3)
		{
			u32 ClientVersion = 0;
			Read(ClientVersion);
			Version.nSubVersion = ClientVersion;

			i64 Time;
			Read(Time);
			Timestamp = Time;
		}
		else
		{
			i64 Time;
			Read(Time);
			Timestamp = Time;

			// Read all the four version numbers
			for (int i = 0; i < 4; ++i) {
				u32 ClientVersion = 0;
				Read(ClientVersion);
			}
		}
	}
	else if(Version.Server == ServerType::FreestyleGunz && Version.nVersion == 7)
	{
		REPLAY_STAGE_SETTING_NODE_FG Setting;
		Peek(Setting);

		int offset = 527;

		if (Setting.nGameType == MMATCH_GAMETYPE_DUEL)
			offset += sizeof(MTD_DuelQueueInfo);

		u8 Something;
		ReadAt(Something, offset);

		if (Version.nVersion == 7 && (Something == 0x00 || Something == 0x01))
		{
			Version.nSubVersion = 1;
		}
	}

	this->Version = Version;

	return Version;
}

HAS_XXX(szStageName);

template <typename T>
typename std::enable_if<has_szStageName<T>::value>::type CopyStageName(REPLAY_STAGE_SETTING_NODE &m_StageSetting, const T &Setting) {
	memcpy(m_StageSetting.szStageName, Setting.szStageName,
		min(sizeof(m_StageSetting.szStageName), sizeof(Setting.szStageName)));
	m_StageSetting.szStageName[sizeof(m_StageSetting.szStageName) - 1] = 0;
}

template <typename T>
typename std::enable_if<!has_szStageName<T>::value>::type CopyStageName(REPLAY_STAGE_SETTING_NODE &m_StageSetting, const T &Setting) {
	m_StageSetting.szStageName[0] = 0;
}

inline void ZReplayLoader::GetStageSetting(REPLAY_STAGE_SETTING_NODE& ret)
{
	// Zero out ret.
	// This is necessary since several members of our stage settings struct do not exist in the
	// ones of other servers, and are therefore never set by the conversion, so this gives them
	// default values.
	ret = REPLAY_STAGE_SETTING_NODE{};

#define COPY_SETTING(member) ret.member = Setting.member;

	auto CopySetting = [&](const auto &Setting)
	{
		COPY_SETTING(uidStage);
		// We don't want to do strcpy in case it's not null-terminated
		memcpy(ret.szMapName, Setting.szMapName, sizeof(ret.szMapName));
		ret.szMapName[sizeof(ret.szMapName) - 1] = 0;
		COPY_SETTING(nMapIndex);
		COPY_SETTING(nGameType);
		COPY_SETTING(nRoundMax);
		COPY_SETTING(nLimitTime);
		COPY_SETTING(nLimitLevel);
		COPY_SETTING(nMaxPlayers);
		COPY_SETTING(bTeamKillEnabled);
		COPY_SETTING(bTeamWinThePoint);
		COPY_SETTING(bForcedEntryEnabled);

		CopyStageName(ret, Setting);
	};

	switch (Version.Server)
	{
	case ServerType::Official:
	{
		if (Version.nVersion <= 6)
		{
			REPLAY_STAGE_SETTING_NODE_OLD Setting;
			Read(Setting);

			CopySetting(Setting);
		}
		else
		{
			REPLAY_STAGE_SETTING_NODE_V11 Setting;
			Read(Setting);

			CopySetting(Setting);
		}
	}
	break;
	case ServerType::RefinedGunz:
	{
		if (Version.nVersion == 1)
		{
			REPLAY_STAGE_SETTING_NODE_RG_V1 Setting;
			Read(Setting);
			CopySetting(Setting);
			COPY_SETTING(bAutoTeamBalancing);
		}
		else if (Version.nVersion == 2)
		{
			REPLAY_STAGE_SETTING_NODE_RG_V2 Setting;
			Read(Setting);
			CopySetting(Setting);
			COPY_SETTING(bAutoTeamBalancing);
			COPY_SETTING(Netcode);
			COPY_SETTING(ForceHPAP);
			COPY_SETTING(HP);
			COPY_SETTING(AP);
			COPY_SETTING(NoFlip);
			COPY_SETTING(SwordsOnly);
		}
		else if (Version.nVersion >= 3)
		{
			Read(ret);
		}
		else
		{
			MLog("ZReplayLoader::GetStageSetting -- Unknown RG version\n");
		}
	}
	break;
	case ServerType::FreestyleGunz:
	{
		REPLAY_STAGE_SETTING_NODE_FG Setting;
		Read(Setting);
		CopySetting(Setting);
	}
	break;
	case ServerType::DarkGunz:
	{
		REPLAY_STAGE_SETTING_NODE_DG Setting;
		Read(Setting);
		CopySetting(Setting);
	}
	break;
	};

	IsDojo = !_stricmp(ret.szMapName, "Dojo");
	GameType = ret.nGameType;

#undef COPY_SETTING
}

inline void ZReplayLoader::GetDuelQueueInfo(MTD_DuelQueueInfo* QueueInfo)
{
	if (GameType != MMATCH_GAMETYPE_DUEL)
		return;

	if (!QueueInfo)
	{
		Position += sizeof(*QueueInfo);
		return;
	}

	Read(*QueueInfo);
}

inline std::vector<MTD_GunGameWeaponInfo> ZReplayLoader::GetGunGameWeaponInfo()
{
	Read(GunGameCharacterCount);

	if (GunGameCharacterCount <= 0 || GunGameCharacterCount > 64)
	{
		MLog("Replay is corrupt: Gun game info character count is %d, "
			"expected value between 1 and 64\n",
			GunGameCharacterCount);
		throw std::runtime_error{ "Replay is corrupt" };
	}

	std::vector<MTD_GunGameWeaponInfo> ret;
	ret.resize(GunGameCharacterCount);
	for (int i = 0; i < GunGameCharacterCount; ++i)
	{
		Read(ret[i]);
	}

	return ret;
}

inline std::vector<ReplayPlayerInfo> ZReplayLoader::GetCharInfo()
{
	std::vector<ReplayPlayerInfo> ret;

	int nCharacterCount;
	Read(nCharacterCount);

	if (nCharacterCount <= 0 || nCharacterCount > 64)
	{
		MLog("Replay is corrupt: Char info character count is %d, "
			"expected value between 1 and 64\n",
			nCharacterCount);
		throw std::runtime_error{ "Replay is corrupt" };
	}

	if (GunGameCharacterCount != -1 && GunGameCharacterCount != nCharacterCount)
	{
		MLog("Replay is corrupt: Character counts in char info and gun game info differ.\n"
			"Char info count = %d, gun game info count = %d\n",
			nCharacterCount, GunGameCharacterCount);
		throw std::runtime_error{ "Replay is corrupt" };
	}

	DMLog("Char count = %d\n", nCharacterCount);

	for (int i = 0; i < nCharacterCount; i++)
	{
		bool IsHero;
		if (Version.Server != ServerType::DarkGunz)
			Read(IsHero);

		MTD_CharInfo CharInfo;

#define COPY_CHARINFO(member) CharInfo.member = oldinfo.member
		auto CopyCharInfo = [&](const auto& oldinfo)
		{
			strcpy_safe(CharInfo.szName, oldinfo.szName);
			strcpy_safe(CharInfo.szClanName, oldinfo.szClanName);
			COPY_CHARINFO(nClanGrade);
			COPY_CHARINFO(nClanContPoint);
			COPY_CHARINFO(nCharNum);
			COPY_CHARINFO(nLevel);
			COPY_CHARINFO(nSex);
			COPY_CHARINFO(nHair);
			COPY_CHARINFO(nFace);
			COPY_CHARINFO(nXP);
			COPY_CHARINFO(nBP);
			COPY_CHARINFO(fBonusRate);
			COPY_CHARINFO(nPrize);
			COPY_CHARINFO(nHP);
			COPY_CHARINFO(nAP);
			COPY_CHARINFO(nMaxWeight);
			COPY_CHARINFO(nSafeFalls);
			COPY_CHARINFO(nFR);
			COPY_CHARINFO(nCR);
			COPY_CHARINFO(nER);
			COPY_CHARINFO(nWR);
			for (size_t i = 0; i < min(static_cast<size_t>(MMCIP_END), std::size(oldinfo.nEquipedItemDesc)); i++)
				COPY_CHARINFO(nEquipedItemDesc[i]);
			COPY_CHARINFO(nUGradeID);
		};
#undef COPY_CHARINFO

#define READ_CHARINFO(type) do { type info; Read(info); CopyCharInfo(info); } while(false)

		if (Version.Server == ServerType::Official)
		{
			if (Version.nVersion <= 5)
			{
				MTD_CharInfo_V5 oldinfo;
				if (Version.nVersion < 2)
				{
					ReadN(&oldinfo, sizeof(oldinfo) - sizeof(oldinfo.nClanCLID));
					oldinfo.nClanCLID = 0;
				}
				else
				{
					Read(oldinfo);
				}
				CopyCharInfo(oldinfo);
			}
			else if (Version.nVersion >= 6 && Version.nVersion < 11)
			{
				READ_CHARINFO(MTD_CharInfo_V6);
			}
			else if (Version.nVersion == 11)
			{
				READ_CHARINFO(MTD_CharInfo_V11);
			}
		}
		else if (Version.Server == ServerType::FreestyleGunz)
		{
			if (Version.nVersion == 7)
			{
				if (Version.nSubVersion == 0)
				{
					READ_CHARINFO(MTD_CharInfo_FG_V7_0);
				}
				else if (Version.nSubVersion == 1)
				{
					READ_CHARINFO(MTD_CharInfo_FG_V7_1);
				}
			}
			else if (Version.nVersion == 8)
			{
				READ_CHARINFO(MTD_CharInfo_FG_V8);
			}
			else if (Version.nVersion == 9)
			{
				READ_CHARINFO(MTD_CharInfo_FG_V9);
			}
			else if (Version.nVersion == 10)
			{
				READ_CHARINFO(MTD_CharInfo_FG_V10);
			}
			else
			{
				assert(false);
			}
		}
		else if (Version.Server == ServerType::RefinedGunz)
		{
			Read(CharInfo);
		}
#undef READ_CHARINFO

		ZCharacterReplayState CharState;
		CharState.LowerAnimation = CharState.UpperAnimation = CharState.SelectedItem = u8(-1);

#define COPY_CHARSTATE(member) CharState.member = src.member
		auto CopyCharState = [&](const auto& src)
		{
			COPY_CHARSTATE(UID);
			COPY_CHARSTATE(Property);
			COPY_CHARSTATE(HP);
			COPY_CHARSTATE(AP);
			COPY_CHARSTATE(Status);

			for (size_t i = 0; i < min(static_cast<size_t>(MMCIP_END), std::size(src.BulletInfos)); i++)
				COPY_CHARSTATE(BulletInfos[i]);

			COPY_CHARSTATE(Position);
			COPY_CHARSTATE(Direction);
			COPY_CHARSTATE(Team);
			COPY_CHARSTATE(Dead);
			COPY_CHARSTATE(HidingAdmin);
		};
#undef COPY_CHARSTATE

#define READ_CHARSTATE(type) do { type state; Read(state); CopyCharState(state); } while (false)

		if (Version.Server == ServerType::FreestyleGunz)
		{
			if (Version.nVersion == 7)
			{
				if (Version.nSubVersion == 0)
				{
					READ_CHARSTATE(ZCharacterReplayState_FG_V7_0);
				}
				else
				{
					READ_CHARSTATE(ZCharacterReplayState_FG_V7_1);
				}
			}
			else if (Version.nVersion == 8)
			{
				READ_CHARSTATE(ZCharacterReplayState_FG_V8);
			}
			else if (Version.nVersion == 9)
			{
				READ_CHARSTATE(ZCharacterReplayState_FG_V9);
			}
			else if (Version.nVersion == 10)
			{
				READ_CHARSTATE(ZCharacterReplayState_FG_V10);
			}
			else
			{
				assert(false);
			}
		}
		else if (Version.Server == ServerType::RefinedGunz)
		{
			if (Version.nVersion <= 4)
			{
				READ_CHARSTATE(ZCharacterReplayState_RG_V4);
			}
			else
			{
				Read(CharState);
			}
		}
		else if (Version.Server == ServerType::Official)
		{
			if (Version.nVersion >= 6)
			{
				if (Version.nVersion == 11)
				{
					READ_CHARSTATE(ZCharacterReplayState_Official_V11);
				}
				else
				{
					READ_CHARSTATE(ZCharacterReplayState_Official_V6);
				}
			}
			else
			{
				READ_CHARSTATE(ZCharacterReplayState_Official_V5);
			}
		}
#undef READ_CHARSTATE

		if (Version.Server == ServerType::DarkGunz)
		{
			ReplayPlayerInfo_DG rpi;
			Read(rpi);
			IsHero = rpi.IsHero;
			CopyCharInfo(rpi.Info);
			CopyCharState(rpi.State);
		}

		DMLog("Char %d name = %s\n", i, CharInfo.szName);

		ret.push_back({ IsHero, CharInfo, CharState });
	}

	return ret;
}

template <typename T>
void ZReplayLoader::ReadAt(T& Obj, int Position)
{
	if (Position + sizeof(Obj) > InflatedFile.size())
		throw EOFException(Position);

	Obj = *((decltype(&Obj))&InflatedFile[Position]);
};

template <typename T>
void ZReplayLoader::Peek(T& Obj)
{
	ReadAt(Obj, Position);
};

template <typename T>
void ZReplayLoader::Read(T& Obj)
{
	Peek(Obj);
	Position += sizeof(Obj);
};

template <typename T>
bool ZReplayLoader::TryRead(T& Obj)
{
	if (Position + sizeof(Obj) > InflatedFile.size())
		return false;

	Obj = *((decltype(&Obj))&InflatedFile[Position]);
	Position += sizeof(Obj);

	return true;
};

inline void ZReplayLoader::ReadN(void* Obj, size_t Size)
{
	if (Position + Size > InflatedFile.size())
		throw EOFException(Position);

	memcpy(Obj, &InflatedFile[Position], Size);
	Position += Size;
};

inline bool ZReplayLoader::FixCommand(MCommand& Command)
{
	if (Version.Server == ServerType::FreestyleGunz && IsDojo)
	{
		auto Transform = [](float pos[3])
		{
			pos[0] -= 600;
			pos[1] += 2800;
			pos[2] += 400;
		};

		if (Command.GetID() == MC_PEER_BASICINFO)
		{
			MCommandParameter* pParam = Command.GetParameter(0);
			if (pParam->GetType() != MPT_BLOB)
				return false;

			ZPACKEDBASICINFO* ppbi = (ZPACKEDBASICINFO*)pParam->GetPointer();

			float pos[3] = { (float)ppbi->posx, (float)ppbi->posy, (float)ppbi->posz };

			if (pos[2] < 0)
			{
				Transform(pos);

				ppbi->posx = static_cast<short>(pos[0]);
				ppbi->posy = static_cast<short>(pos[1]);
				ppbi->posz = static_cast<short>(pos[2]);
			}
		}
	}
	else if (Version.Server == ServerType::Official && Version.nVersion == 11)
	{
		if (Command.GetID() == MC_PEER_BASICINFO)
		{
			auto Param = (MCommandParameterBlob*)Command.GetParameter(0);
			if (Param->GetType() != MPT_BLOB)
				return false;

			auto pbi = Param->GetPointer();

			[pbi = (short*)pbi]
			{
				for (int i = 0; i < 3; i++)
				{
					pbi[8 + i] = pbi[11 + i];
				}
			}();

			[pbi = (BYTE*)pbi]
			{
				for (int i = 0; i < 3; i++)
				{
					pbi[22 + i] = pbi[28 + i];
				}
			}();

			[pbi = (ZPACKEDBASICINFO*)pbi]
			{
				pbi->posz -= 125;
				pbi->velx = 0;
				pbi->vely = 0;
				pbi->velz = 0;

				ZBasicInfo bi;
				pbi->Unpack(bi);

				if (fabs(fabs(Magnitude(bi.direction)) - 1.f) > 0.001)
				{
					bi.direction = rvector(1, 0, 0);
					pbi->Pack(bi);
				}
			}();
		}
	}

	return true;
}

template <typename T>
bool ZReplayLoader::GetCommandsImpl(T fn, ArrayView<u32>* WantedCommandIDs)
{
	char CommandBuffer[1024];

	float fGameTime;
	Read(fGameTime);
	m_fGameTime = fGameTime;

	int nSize;
	float fTime;

	while (TryRead(fTime))
	{
		MUID uidSender;
		Read(uidSender);
		Read(nSize);

		if (nSize <= 0 || nSize > sizeof(CommandBuffer))
			return false;

		ReadN(CommandBuffer, nSize);

		if (WantedCommandIDs)
		{
			u32 CommandID = *(u16 *)(CommandBuffer + sizeof(u16));

			bool OuterContinue = true;

			for (auto id : *WantedCommandIDs)
			{
				if (id == CommandID)
				{
					OuterContinue = false;
					break;
				}
			}

			if (OuterContinue)
				continue;
		}
		
		fn(CommandBuffer, uidSender, fTime);
	}

	return true;
}

template <typename T>
bool ZReplayLoader::GetCommands(T ForEachCommand, bool PersistentMCommands, ArrayView<u32>* WantedCommandIDs)
{
	auto DoStuff = [&](MCommand& Command, const MUID& Sender, auto fTime)
	{
		Command.m_Sender = Sender;

		if (!FixCommand(Command))
			return;

		ForEachCommand(&Command, fTime);
	};

	if (PersistentMCommands)
	{
		auto Stuff = [&](const char *CommandBuffer, const MUID& Sender, auto fTime)
		{
			MCommand* Command = new MCommand;

			if (!CreateCommandFromStream(CommandBuffer, *Command))
				return;

			DoStuff(*Command, Sender, fTime);
		};

		if (!GetCommandsImpl(Stuff, WantedCommandIDs))
			return false;
	}
	else
	{
		MCommand StackCommand;
		u8 ArenaBuffer[4096];
		Arena arena{ ArenaBuffer, std::size(ArenaBuffer) };
		using AllocType = ArenaAllocator<u8>;
		AllocType Alloc{ &arena };

		auto Stuff = [&](const char *CommandBuffer, const MUID& Sender, auto fTime)
		{
			auto ClearStackCommandParams = [&]()
			{
				for (auto Param : StackCommand.m_Params)
				{
					std::allocator_traits<AllocType>::destroy(Alloc, Param);

					size_t size = 0;

					switch (Param->GetType())
					{
					case MPT_INT: size = sizeof(MCommandParameterInt); break;
					case MPT_UINT: size = sizeof(MCommandParameterUInt); break;
					case MPT_FLOAT: size = sizeof(MCommandParameterFloat); break;
					case MPT_STR: size = sizeof(MCommandParameterStringCustomAlloc<AllocType>); break;
					case MPT_VECTOR: size = sizeof(MCommandParameterVector); break;
					case MPT_POS: size = sizeof(MCommandParameterPos); break;
					case MPT_DIR: size = sizeof(MCommandParameterDir); break;
					case MPT_BOOL: size = sizeof(MCommandParameterBool); break;
					case MPT_COLOR: size = sizeof(MCommandParameterColor); break;
					case MPT_UID: size = sizeof(MCommandParameterUID); break;
					case MPT_BLOB: size = sizeof(MCommandParameterBlobCustomAlloc<AllocType>); break;
					case MPT_CHAR: size = sizeof(MCommandParameterChar); break;
					case MPT_UCHAR: size = sizeof(MCommandParameterUChar); break;
					case MPT_SHORT: size = sizeof(MCommandParameterShort); break;
					case MPT_USHORT: size = sizeof(MCommandParameterUShort); break;
					case MPT_INT64: size = sizeof(MCommandParameterInt64); break;
					case MPT_UINT64: size = sizeof(MCommandParameterUInt64); break;
					case MPT_SVECTOR: size = sizeof(MCommandParameterShortVector); break;
					};

					arena.deallocate((uint8_t*)Param, size);
				}

				StackCommand.m_Params.clear();
			};

			if (!StackCommand.m_Params.empty())
			{
				MLog("Not empty! size %d, command id %d\n",
					StackCommand.m_Params.size(), StackCommand.GetID());
				StackCommand.m_Params.clear();
			}

			if (CreateCommandFromStream(CommandBuffer, StackCommand, Alloc))
			{
				DoStuff(StackCommand, Sender, fTime);
			}
			else
			{
				//MLog("Failed to read command ID %d, total size %d\n", *(u16 *)(CommandBuffer + sizeof(u16)), *(u16*)CommandBuffer);
			}

			ClearStackCommandParams();
		};

		if (!GetCommandsImpl(Stuff, WantedCommandIDs))
			return false;
	}

	return true;
}
