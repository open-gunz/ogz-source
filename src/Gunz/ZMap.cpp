#include "stdafx.h"
#include "ZMap.h"
#include "ZApplication.h"
#include "MComboBox.h"
#include "ZChannelRule.h"
#include "ZGameClient.h"
#include "Config.h"

static constexpr char MapExtension[] = ".rs.xml";

static bool IsQuestDerivedGameType()
{
	if (!ZGetGameClient() || !ZGetGameTypeManager())
		return false;

	auto&& GameType = ZGetGameClient()->GetMatchStageSetting()->GetGameType();
	return ZGetGameTypeManager()->IsQuestDerived(GameType);
}

void ZGetCurrMapPath(char* outPath, int maxlen)
{
#ifdef _QUEST
	if (IsQuestDerivedGameType() ||
		ZApplication::GetInstance()->GetLaunchMode() == ZApplication::ZLAUNCH_MODE_STANDALONE_QUEST)
	{
		strcpy_safe(outPath, maxlen, PATH_QUEST_MAPS);
		return;
	}
#endif

	strcpy_safe(outPath, maxlen, PATH_GAME_MAPS);
}

static StringView GetMapName(const StringView& Path)
{
	if (Path.size() < 2)
		return "";

	const auto SlashIndex = Path.find_last_of("/\\", Path.size() - 2);
	if (SlashIndex == Path.npos)
		return "";
	const auto NameIndex = SlashIndex + 1;
	return Path.substr(NameIndex, Path.size() - NameIndex - 1);
}

static bool IsMapPath(MZFileSystem& FS, const MZDirDesc& MapDirNode, const StringView& DirName)
{
	char MapXMLPath[_MAX_PATH];
	sprintf_safe(MapXMLPath, "%.*s%.*s%s",
		MapDirNode.Path.size(), MapDirNode.Path.data(),
		DirName.size(), DirName.data(),
		MapExtension);

	auto Desc = FS.GetFileDesc(MapXMLPath);
	return Desc != nullptr;
}

static bool ShouldAddMap(const StringView& MapName, MChannelRule& ChannelRule, MMATCH_GAMETYPE GameType)
{
#if defined(_DEBUG) || defined(ADD_ALL_MAPS)
	return true;
#endif

	if (GameType == MMATCH_GAMETYPE_SKILLMAP)
	{
		return icontains(MapName, "skill") || iequals(MapName, "Superflip");
	}
	else
	{
		const bool IsDuelGameType = GameType == MMATCH_GAMETYPE_DUEL;
		return ChannelRule.CheckMap(MapName, IsDuelGameType);
	}
}

static bool AddMaps(MComboBox& Widget, MChannelRule& ChannelRule)
{
	auto& FS = *ZGetFileSystem();
	auto&& GameType = ZGetGameClient()->GetMatchStageSetting()->GetGameType();

	char MapDirectory[64];
	ZGetCurrMapPath(MapDirectory);

	auto MapsDirNode = FS.GetDirectory(MapDirectory);
	if (!MapsDirNode) {
		MLog("AddMaps -- Couldn't find maps directory (%s)\n", MapDirectory);
		return false;
	}

	for (auto&& MapNode : MapsDirNode->SubdirsRange())
	{
		auto MapName = GetMapName(MapNode.Path);
		if (IsMapPath(FS, MapNode, MapName) &&
			ShouldAddMap(MapName, ChannelRule, GameType))
		{
			Widget.Add(MapName);
		}
	}

	return true;
}

bool InitMapSelectionWidget()
{
	auto MapSelection = ZFindWidgetAs<MComboBox>("MapSelection");
	if (!MapSelection)
		return false;

	MapSelection->RemoveAll();

	if (!ZGetGameClient())
	{
		MLog("InitMaps -- ZGetGameClient() is null\n");
		return false;
	}

	if (IsQuestDerivedGameType())
	{
		for (auto&& MapName : GetScenarioManager().MapNames)
			MapSelection->Add(MapName.data());
		return true;
	}

	auto* pRule = ZGetChannelRuleMgr()->GetCurrentRule();
	if (!pRule) {
		MLog("InitMaps -- No current channelrule\n");
		return false;
	}

	return AddMaps(*MapSelection, *pRule);
}