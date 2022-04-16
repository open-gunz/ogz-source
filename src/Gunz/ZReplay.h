#pragma once

#include "ZCharacter.h"
#include "ZReplayStructs.h"
#include "ArrayView.h"
#include "GlobalTypes.h"

bool CreateReplayGame(const char *filename);

#define GUNZ_REC_FILE_ID		0x95b1308a

#define GUNZ_REC_FILE_VERSION	4
#define GUNZ_REC_FILE_EXT		"gzr"

#define RG_REPLAY_MAGIC_NUMBER 0x00DEFBAD
#define RG_REPLAY_BINARY_VERSION 5

enum class ServerType
{
	None,
	Official, // original igunz, ijji gunz, aeria gunz
	RefinedGunz,
	FreestyleGunz,
	DarkGunz,
	Max,
};

struct ReplayVersion
{
	ServerType Server = ServerType::None;
	int nVersion = -1;
	int nSubVersion = -1;

	ReplayVersion() = default;

	ReplayVersion(ServerType Server, int nVersion, int nSubVersion)
		: Server{ Server }, nVersion{ nVersion }, nSubVersion{ nSubVersion } {}

	const char* GetServerString() const
	{
		switch (Server)
		{
		case ServerType::Official:
			return "Official";
		case ServerType::FreestyleGunz:
			return "Freestyle Gunz";
		case ServerType::RefinedGunz:
			return "Open GunZ";
		case ServerType::DarkGunz:
			return "Dark Gunz";
		case ServerType::None:
		default:
			return "Unknown";
		}
	}

	std::string GetVersionString() const
	{
		std::string ret = "Version: ";
		ret += GetServerString();
		ret += " V";
		ret += std::to_string(nVersion);

		if (Server == ServerType::FreestyleGunz && nVersion == 7)
		{
			ret += ".";
			ret += std::to_string(nSubVersion);
		}

		return ret;
	}

	bool operator==(const ReplayVersion& rhs) const {
		return Server == rhs.Server && nVersion == rhs.nVersion && nSubVersion == rhs.nSubVersion;
	}

	bool operator!=(const ReplayVersion& rhs) const {
		return !(*this == rhs);
	}
};

void GetReplayStageSetting(REPLAY_STAGE_SETTING_NODE& dest, const MSTAGE_SETTING_NODE& src);

class ZReplayLoader
{
public:
	bool LoadFile(const char* FileName);

	float GetGameTime() const { return m_fGameTime; }

	ReplayVersion GetVersion();
	time_t GetTimestamp() const { return Timestamp; }
	void GetStageSetting(REPLAY_STAGE_SETTING_NODE &ret);
	void GetDuelQueueInfo(MTD_DuelQueueInfo* QueueInfo = nullptr);
	std::vector<MTD_GunGameWeaponInfo> GetGunGameWeaponInfo();
	std::vector<ReplayPlayerInfo> GetCharInfo();
	template <typename T>
	bool GetCommands(T ForEachCommand, bool PersistentMCommands, ArrayView<u32>* WantedCommandIDs = nullptr);

	int GetPosition() const { return Position; }

private:
	std::vector<u8> InflatedFile;

	ReplayVersion Version;
	float m_fGameTime = 0.f;
	int Position = 0;
	bool IsDojo = false;
	MMATCH_GAMETYPE GameType = MMATCH_GAMETYPE_DEATHMATCH_SOLO;
	time_t Timestamp = 0;
	int GunGameCharacterCount = -1;

	template <typename T>
	void Read(T& Obj);
	template <typename T>
	void ReadAt(T& Obj, int Position);
	template <typename T>
	void Peek(T& Obj);
	void ReadN(void* Obj, size_t Size);
	template <typename T>
	bool TryRead(T& Obj);

	template <typename HeaderType, typename StageSettingType, typename PlayerInfoType>
	bool IsVersion();

	template <typename T>
	bool GetCommandsImpl(T fn, ArrayView<u32>* WantedCommandIDs);
	template <typename T = std::allocator<u8>>
	bool CreateCommandFromStream(const char* pStream, MCommand& Command, T& Alloc);
	bool CreateCommandFromStream(const char* pStream, MCommand& Command)
	{
		std::allocator<u8> alloc;
		return CreateCommandFromStream(pStream, Command, alloc);
	}
	bool FixCommand(MCommand& Command);
	static bool CreateCommandFromStreamVersion2(const char* pStream, MCommand& Command);
	static bool ParseVersion2Command(const char* pStream, MCommand* pCmd);
	static MCommandParameter* MakeVersion2CommandParameter(MCommandParameterType nType, const char* pStream, unsigned short int* pnDataCount);
};
