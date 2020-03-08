#include "stdafx.h"
#include "MBMatchServer.h"
#include "MMatchConfig.h"

static std::string Line;
static std::vector<std::string> Splits;
// Doesn't count the command.
static int NumArguments;

static constexpr int NoArgumentLimit = -1;

void MBMatchServer::InitConsoleCommands()
{
	auto AddConsoleCommand = [&](const char* Name,
		int MinArgs, int MaxArgs,
		std::string Description, ::string Usage, std::string Help,
		auto&& Callback)
	{
		ConsoleCommand Command;
		Command.MinArgs = MinArgs;
		Command.MaxArgs = MaxArgs;
		Command.Description = std::move(Description);
		Command.Usage = std::move(Usage);
		Command.Help = std::move(Help);
		Command.Callback = std::move(Callback);
		ConsoleCommandMap.emplace(Name, std::move(Command));
	};

	AddConsoleCommand("hello",
		NoArgumentLimit, NoArgumentLimit,
		"Says hi.", "hello", "",
		[]{ MLog("Hi! ^__^\n"); });

	AddConsoleCommand("argv",
		NoArgumentLimit, NoArgumentLimit,
		"Prints all input.", "argv [Arg1 [Arg2 ... [ArgN]]]", "",
		[] {
		MLog("Line = %s\n", Line.c_str());
		MLog("NumArguments = %d\n", NumArguments);
		for (int i = 0; i < int(Splits.size()); ++i)
			MLog("Splits[%d] = \"%s\"\n", i, Splits[i].c_str());
	});

	AddConsoleCommand("help",
		NoArgumentLimit, 1,
		"Provides information about commands.",
		"help [command name]",
		"",
		[&] {
		if (NumArguments == 0)
		{
			// Print all the commands.
			for (auto&& Pair : ConsoleCommandMap)
			{
				MLog("%s: %s\n", Pair.first.c_str(), Pair.second.Description.c_str());
			}
		}
		else
		{
			auto it = ConsoleCommandMap.find(Splits[1]);
			if (it == ConsoleCommandMap.end())
			{
				MLog("help: Unknown command \"%s\"\n", Splits[1].c_str());
				return;
			}

			MLog("help: %s\n"
				"Description: %s\n"
				"Usage: %s\n"
				"Help: %s\n",
				it->first.c_str(),
				it->second.Description.c_str(),
				it->second.Usage.c_str(),
				it->second.Help.c_str());
		}
	});

	AddConsoleCommand("setversion", 2, 2,
		"Sets the expected client version.",
		"setversion <major/minor/patch/revision> <version number>",
		"Sets the specified field of MMatchConfig::Version to the specified value.\n"
		"This value is used when players log in, to check if their clients are up to date.\n"
		"Revision number is in hexadecimal.",
		[] {
		auto VersionTypeString = Splits[1].c_str();
		auto VersionValueString = Splits[2].c_str();

		auto SetVersion = [&](auto&& Version, int Radix) {
			const u32 NewVersion = strtoul(VersionValueString, nullptr, Radix);
			const auto OldVersion = Version;
			Version = NewVersion;
			MLog("Set %s version to %d! Previous %s version was %d.\n",
				VersionTypeString, NewVersion,
				VersionTypeString, OldVersion);
			return;
		};

		if (!_stricmp(VersionTypeString, "revision"))
		{
			SetVersion(MGetServerConfig()->Version.Revision, 16);
			return;
		}

		std::pair<const char*, u32*> Array[] = {
			{"major", &MGetServerConfig()->Version.Major},
			{"minor", &MGetServerConfig()->Version.Minor},
			{"patch", &MGetServerConfig()->Version.Patch},
		};
		std::pair<const char*, u32*>* Element = nullptr;
		for (auto&& pair : Array)
		{
			if (!_stricmp(VersionTypeString, pair.first))
			{
				Element = &pair;
				break;
			}
		}
		if (!Element)
		{
			MLog("Invalid version string %s.\n", VersionTypeString);
			return;
		}

		SetVersion(*Element->second, 10);
	});

	AddConsoleCommand("setversionchecking", 0, 1,
		"Toggles client version checking on login.",
		"setversionchecking [0/1]",
		"",
		[] {
		if (NumArguments == 0) {
			MGetServerConfig()->VersionChecking = !MGetServerConfig()->VersionChecking;
		}
		else {
			MGetServerConfig()->VersionChecking = atoi(Splits[1].c_str()) != 0;
		}
		MLog("Set version checking to %s.\n", MGetServerConfig()->VersionChecking ? "true" : "false");
	});

	AddConsoleCommand("getversion", 0, 0,
		"Outputs the current version.",
		"getversion",
		"",
		[] {
		MLog("%d.%d.%d-%X\n",
			MGetServerConfig()->Version.Major, MGetServerConfig()->Version.Minor,
			MGetServerConfig()->Version.Patch, MGetServerConfig()->Version.Revision);
	});

	AddConsoleCommand("players", 0, 0,
		"Lists information about all the players on the server.",
		"players",
		"",
		[&] {
		for (auto& Pair : m_Objects)
		{
			auto&& UID = Pair.first;
			auto&& Object = Pair.second;
			auto&& Stage = FindStage(Object->GetStageUID());
			MLog("%s: CID: %u, UID: 0x%llX, stage: {%s, UID: 0x%llX}\n",
				Object->GetName(), Object->GetCharInfo()->m_nCID, UID.AsU64(),
				Stage ? Stage->GetName() : "not in a stage",
				Stage ? Stage->GetUID().AsU64() : 0);
		}
	});

	AddConsoleCommand("stages", 0, 0,
		"Lists information about all the stages on the server.",
		"stages",
		"",
		[&] {
		for (auto& Pair : m_StageMap)
		{
			auto&& UID = Pair.first;
			auto&& Stage = Pair.second;
			MLog("%llX -- %s, map: %s\n", UID.AsU64(),
				Stage->GetName(), Stage->GetMapName());
		}
	});

	AddConsoleCommand("addbot", 1, 2,
		"",
		"addbot <stage UID> [team]",
		"",
		[&] {
		auto StageUID = StringToInt<u64>(Splits[1]);
		if (!StageUID)
		{
			MLog("Malformed UID\n");
			return;
		}
		MMatchTeam Team = MMT_ALL;
		if (Splits.size() == 3)
		{
			auto TeamVal = StringToInt<int>(Splits[2]);
			if (!TeamVal)
			{
				MLog("Malformed team\n");
				return;
			}
			Team = MMatchTeam(*TeamVal);
		}
		auto Object = AddBot(MUID(*StageUID), Team);
		MLog("UID = %llX\n", Object->GetUID().AsU64());
	});

	AddConsoleCommand("kill", 1, 1,
		"",
		"kill <player UID>",
		"",
		[&] {
		auto PlayerUIDVal = StringToInt<u64>(Splits[1]);
		if (!PlayerUIDVal)
		{
			MLog("Malformed UID\n");
			return;
		}
		auto PlayerUID = MUID(*PlayerUIDVal);
		auto Player = GetObject(PlayerUID);
		if (!Player)
		{
			MLog("Player with UID %llX not found\n", PlayerUID);
			return;
		}
		OnGameKill(PlayerUID, PlayerUID);
	});

	AddConsoleCommand("disconnect", 1, 1,
		"",
		"disconnect <player UID>",
		"",
		[&] {
		auto UID = StringToInt<u64>(Splits[1]);
		if (!UID)
		{
			MLog("Malformed UID\n");
			return;
		}
		Disconnect(MUID(*UID));
	});

	AddConsoleCommand("addquestitem", 2, 3,
		"",
		"addquestitem <player UID> <item ID> [count (default 1)]",
		"",
		[&] {
		auto UID = StringToInt<u64>(Splits[1]);
		auto ID = StringToInt<u32>(Splits[2]);
		auto Count = 1;
		if (Splits.size() > 3)
		{
			auto MCount = StringToInt<int>(Splits[3]);
			if (!MCount || *MCount <= 0)
			{
				MLog("Invalid count value\n");
				return;
			}
			Count = *MCount;
		}
		if (!UID)
		{
			MLog("Malformed UID\n");
			return;
		}

		if (!ID)
		{
			MLog("Malformed item ID\n");
			return;
		}

		auto Object = GetObject(MUID(*UID));
		if (!Object)
		{
			MLog("Could not find player from UID %llu\n", *UID);
			return;
		}

		auto QuestItem = GetQuestItemDescMgr().FindQItemDesc(*ID);
		if (!QuestItem)
		{
			MLog("Could not find item from ID %u\n", *ID);
			return;
		}
		auto& ci = *Object->GetCharInfo();
		auto& qil = ci.m_QuestItemList;
		auto it = qil.find(*ID);
		if (it != qil.end())
		{
			if (int Overflow = it->second->Increase(Count))
			{
				MLog("Could not add %d of %d count\n", Overflow, Count);
			}
		}
		else
		{
			if (!qil.CreateQuestItem(*ID, Count))
			{
				MLog("MQuestItemList::CreateQuestItem failed\n");
				return;
			}
		}

		if (!Database->UpdateQuestItem(ci.m_nCID, qil, ci.m_QMonsterBible))
		{
			MLog("IDatabase::UpdateQuestItem failed\n");
		}

		OnResponseCharQuestItemList(MUID(*UID));
	});

	AddConsoleCommand("additem", 2, 2,
		"",
		"additem <player UID> <item ID>",
		"",
		[&] {
		auto UID = StringToInt<u64>(Splits[1]);
		auto ID = StringToInt<u32>(Splits[2]);
		if (!UID)
		{
			MLog("Malformed player UID\n");
			return;
		}

		if (!ID)
		{
			MLog("Malformed ID\n");
			return;
		}

		auto Object = GetObject(MUID(*UID));
		if (!Object)
		{
			MLog("Could not find player from UID %llu\n", *UID);
			return;
		}

		auto Success = InsertCharItem(MUID(*UID), *ID, false, 0);
		ResponseCharacterItemList(MUID(*UID));
		MLog("Adding item %u %s\n", *ID, Success ? "succeeded" : "failed");
	});

	AddConsoleCommand("setlevel", 2, 2,
		"",
		"setlevel <player UID> <level>",
		"",
		[&] {
		auto UID = StringToInt<u64>(Splits[1]);
		auto Level = StringToInt<u32>(Splits[2]);
		if (!UID)
		{
			MLog("Malformed player UID\n");
			return;
		}

		if (!Level)
		{
			MLog("Malformed level\n");
			return;
		}

		auto Object = GetObject(MUID(*UID));
		if (!Object)
		{
			MLog("Could not find player from UID %llu\n", *UID);
			return;
		}

		auto& ci = *Object->GetCharInfo();
		ci.m_nLevel = *Level;
		Database->UpdateCharLevel(ci.m_nCID, ci.m_nLevel);
		ResponseMySimpleCharInfo(MUID(*UID));
	});

	AddConsoleCommand("quit", 0, 0, "", "", "", [] { exit(0); });
	AddConsoleCommand("exit", 0, 0, "", "", "", [] { exit(0); });
}

void MBMatchServer::OnInput(const std::string & Input)
{
	Line = Input;

	Splits.clear();

	Split(Input, " ", [](auto&& Str) { Splits.push_back(Str.str()); });

	if (Splits.empty())
		return;

	auto it = ConsoleCommandMap.find(Splits[0]);
	if (it == ConsoleCommandMap.end())
	{
		MLog("Unknown command \"%s\"\n", Splits[0].c_str());
		return;
	}

	auto&& Command = it->second;

	NumArguments = int(Splits.size()) - 1;
	if ((Command.MinArgs != NoArgumentLimit && NumArguments < Command.MinArgs) ||
		(Command.MaxArgs != NoArgumentLimit && NumArguments > Command.MaxArgs))
	{
		MLog("Incorrect number of arguments to %s, valid range is %d-%d, got %d\n",
			it->first.c_str(), Command.MinArgs, Command.MaxArgs, NumArguments);
		return;
	}

	Command.Callback();
}
