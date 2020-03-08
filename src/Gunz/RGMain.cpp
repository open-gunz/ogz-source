#include "stdafx.h"
#include "RGMain.h"
#include "Portal.h"
#include "NewChat.h"
#include "ZConfiguration.h"
#include "Hitboxes.h"
#include "ZRule.h"
#include "ZRuleSkillmap.h"
#include "VoiceChat.h"
#include "ReplayControl.h"
#include "FileInfo.h"
#include "ZReplay.inl"
#include "ZInput.h"
#include <cstdint>
#include "MeshManager.h"
#include "hsv.h"
#include "dxerr.h"
#include "defer.h"
#include "RS2.h"
#include "ZMyBotCharacter.h"
#include "FunctionalListener.h"
#include <string>

static optional<RGMain> g_RGMain;

RGMain& GetRGMain() { return g_RGMain.value(); }
void CreateRGMain() { g_RGMain.emplace(); }
void DestroyRGMain() { g_RGMain.reset(); }
bool IsRGMainAlive() { return g_RGMain.has_value(); }

void RGMain::OnAppCreate()
{
	ZRuleSkillmap::CourseMgr.Init();

#ifdef PORTAL
	g_pPortal = std::make_unique<Portal>();
#endif
}

void RGMain::OnCreateDevice()
{
	auto&& Cfg = *ZGetConfiguration();
	auto&& c = *Cfg.GetChat();
	m_Chat.emplace(c.Font, c.BoldFont, c.FontSize);
	GetChat().SetBackgroundColor(c.BackgroundColor);

	GetRenderer().PostProcess.EnableEffect("ColorInvert", Cfg.GetColorInvert());
	GetRenderer().PostProcess.EnableEffect("Monochrome", Cfg.GetMonochrome());

#ifdef VOICECHAT
	m_VoiceChat.OnCreateDevice();
#endif

	m_HitboxManager.Create();
}

void RGMain::OnDrawGame()
{
	RSetTransform(D3DTS_WORLD, IdentityMatrix());
	RGetDevice()->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
	RGetDevice()->SetRenderState(D3DRS_LIGHTING, FALSE);
	RGetDevice()->SetFVF(D3DFVF_XYZ | D3DFVF_DIFFUSE);
	RGetDevice()->SetRenderState(D3DRS_ZENABLE, TRUE);
	RGetDevice()->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_SELECTARG1);
	RGetDevice()->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_DIFFUSE);
	RGetDevice()->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);
	RGetDevice()->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_DIFFUSE);

	for (auto&& Line : Lines)
	{
		RDrawLine(Line.v1, Line.v2, Line.Color);
	}

	if (ZGetGame()->GetMatch()->GetMatchType() == MMATCH_GAMETYPE_SKILLMAP)
		((ZRuleSkillmap *)ZGetGame()->GetMatch()->GetRule())->Draw();

	if (ZGetConfiguration()->GetShowHitboxes())
		m_HitboxManager.Draw();
}

void RGMain::OnPreDrawGame()
{
	auto&& Bot = GetBotInfo().MyBot;
	if (Bot && GetBotInfo().Settings.ShowCam)
	{
		Bot->RenderCam();
	}
}

void RGMain::OnDrawGameInterface(MDrawContext* pDC)
{
#ifdef VOICECHAT
	m_VoiceChat.OnDraw(pDC);
#endif

	if (NewChatEnabled)
		GetChat().OnDraw(pDC);

	if (ZGetGame()->IsReplay())
		g_ReplayControl.OnDraw(pDC);
}

bool RGMain::OnGameInput()
{
	return GetChat().IsInputEnabled();
}

void RGMain::Resize(int w, int h)
{
	GetChat().Resize(w, h);
}

HRESULT GenerateTexture(IDirect3DDevice9 *pD3Ddev, IDirect3DTexture9 **ppD3Dtex, DWORD colour32){
	if (pD3Ddev->CreateTexture(8, 8, 1, 0, D3DFMT_A4R4G4B4, D3DPOOL_MANAGED, ppD3Dtex, NULL))
		return E_FAIL;

	WORD colour16 = ((WORD)((colour32 >> 28) & 0xF) << 12) | (WORD)(((colour32 >> 20) & 0xF) << 8) | (WORD)(((colour32 >> 12) & 0xF) << 4) | (WORD)(((colour32 >> 4) & 0xF) << 0);

	D3DLOCKED_RECT d3dlr;
	(*ppD3Dtex)->LockRect(0, &d3dlr, 0, 0);
	WORD *pDst16 = (WORD*)d3dlr.pBits;

	for (int xy = 0; xy < 8 * 8; xy++)
		*pDst16++ = colour16;

	(*ppD3Dtex)->UnlockRect(0);

	return S_OK;
}

std::pair<bool, std::vector<unsigned char>> ReadMZFile(const char *szPath)
{
	MZFile File;

	if (!File.Open(szPath, ZApplication::GetFileSystem()))
	{
		return{ false, {} };
	}

	int FileLength = File.GetLength();

	if (FileLength <= 0)
	{
		return{ false, {} };
	}

	std::vector<unsigned char> InflatedFile;
	InflatedFile.resize(FileLength);

	File.Read(&InflatedFile[0], FileLength);

	return { true, InflatedFile };
}

std::pair<bool, std::vector<unsigned char>> ReadZFile(const char *szPath)
{
	ZFile File;

	if (!File.Open(szPath))
	{
		return{ false,{} };
	}

	std::vector<unsigned char> InflatedFile;
	int FileLength = 0;
	int ret = 0;

	do
	{
		InflatedFile.resize(FileLength + 1024);

		ret = File.Read(&InflatedFile[FileLength], 1024);

		FileLength += 1024;
	} while (ret == 1024);

	FileLength -= 1024 - ret;
	InflatedFile.resize(FileLength);

	return{ true, InflatedFile };
}

std::pair<PlayerFoundStatus, ZCharacter*> FindSinglePlayer(const char * NameSubstring)
{
	bool Found = false;
	ZCharacter* FoundChar = nullptr;

	for (auto Item : *ZGetCharacterManager())
	{
		auto Char = Item.second;

		if (!strstr(Char->GetUserName(), NameSubstring))
			continue;

		if (Found)
			return{ PlayerFoundStatus::TooManyFound, nullptr };

		Found = true;
		FoundChar = Char;
	}

	if (Found)
		return{ PlayerFoundStatus::FoundOne, FoundChar };

	return{ PlayerFoundStatus::NotFound, nullptr };
}

RGMain::RGMain() = default;
RGMain::~RGMain() {
#ifdef PORTAL
	g_pPortal.reset();
#endif

	ZRuleSkillmap::CourseMgr.Destroy();
}

void RGMain::OnUpdate(double Elapsed)
{
	LastTime = Time;

	Time += Elapsed;

	{
		std::lock_guard<std::mutex> lock(QueueMutex);

		for (auto it = QueuedInvokations.begin(); it != QueuedInvokations.end(); it++)
		{
			auto item = *it;

			if (Time < item.Time)
				continue;

			item.fn();

			it = QueuedInvokations.erase(it);

			if (it == QueuedInvokations.end())
				break;
		}
	}

	TaskManager::GetInstance().Update(Elapsed);

	GetChat().OnUpdate(Elapsed);
}

bool RGMain::OnEvent(MEvent *pEvent)
{
	if (NewChatEnabled)
		GetChat().OnEvent(pEvent);

	bool IsChatVisible = false;
	if (NewChatEnabled)
		IsChatVisible = GetChat().IsInputEnabled();
	else
		IsChatVisible = ZGetCombatInterface()->IsChatVisible();

	if (IsChatVisible)
	{
		if (ZGetGame()->IsReplay())
			g_ReplayControl.OnEvent(pEvent);

		return true;
	}

	bool ret = false;

#ifdef VOICECHAT
	static bool LastState = false;
	bool CurState = ZIsActionKeyDown(ZACTION_VOICE_CHAT);

	ret = [&]
	{
		if (CurState && !LastState)
		{
			m_VoiceChat.StartRecording();

			return true;
		}
		else if (!CurState && LastState)
		{
			m_VoiceChat.StopRecording();

			return true;
		}

		return false;
	}();

	LastState = CurState;
#endif

#ifdef PORTAL
	g_pPortal->OnShot();
#endif

	return ret;
}

void RGMain::OnInvalidate() { InvalidateBotCamRT(); }
void RGMain::OnRestore() { RestoreBotCamRT(); }
void RGMain::OnDrawLobby(MDrawContext* pDC) {}

struct CustomReplayFrame : MWidget
{
	using MWidget::MWidget;

	void OnDraw(MDrawContext* pDC) override
	{
		GetRGMain().DrawReplayInfo(pDC, this);
	}
};

void RGMain::OnInitInterface(ZIDLResource &IDLResource)
{
	// Create a new widget and attach it as a child to the ReplayGroup widget
	// in order to have our own custom draw code run in its OnDraw callback.
	auto ReplayWidget = IDLResource.FindWidget("ReplayGroup");
	auto Widget = new CustomReplayFrame{ "CustomReplayFrame", ReplayWidget };
	// m_Rect controls the area that is drawn to in OnDraw so we need to set it properly.
	// This is roughly the right part of the replay window.
	Widget->m_Rect = MRECT{ 310, 50, 290, 245 };
}

void RGMain::OnReplaySelected(MListBox* ReplayFileListWidget)
{
	SelectedReplayInfo.PlayerInfos.clear();

	auto SelectedReplay = ReplayFileListWidget->GetSelItem();
	if (!SelectedReplay)
		return;

	std::string Path = GetMyDocumentsPath();
	Path += GUNZ_FOLDER;
	Path += REPLAY_FOLDER;
	Path += "/";
	Path += SelectedReplay->GetString();

	auto PerCommand = [&](MCommand *Command, float Time)
	{
		switch (Command->GetID())
		{
		case MC_MATCH_RESPONSE_PEERLIST:
		{
			auto Param = Command->GetParameter(1);

			if (!Param || Param->GetType() != MPT_BLOB)
				break;

			auto pBlob = Param->GetPointer();

			int Size = MGetBlobArrayCount(pBlob);

			for (int i = 0; i < Size; i++) {
				MTD_PeerListNode* pNode = (MTD_PeerListNode*)MGetBlobArrayElement(pBlob, i);

				auto it = SelectedReplayInfo.PlayerInfos.find(pNode->uidChar);

				if (it != SelectedReplayInfo.PlayerInfos.end())
					continue;

				SelectedReplayInfo.PlayerInfos.insert({ pNode->uidChar, {} });
			}
		}
		break;
		case MC_PEER_DIE:
		{
			MUID Attacker;
			if (!Command->GetParameter(&Attacker, 0, MPT_UID))
				break;

			auto it = SelectedReplayInfo.PlayerInfos.find(Attacker);

			if (it == SelectedReplayInfo.PlayerInfos.end())
				break;

			it->second.Kills++;

			it = SelectedReplayInfo.PlayerInfos.find(Command->GetSenderUID());

			if (it == SelectedReplayInfo.PlayerInfos.end())
				break;

			it->second.Deaths++;
		}
		break;
		};
	};

	try
	{
		ZReplayLoader Loader;
		if (!Loader.LoadFile(Path.c_str()))
			return;
		SelectedReplayInfo.Version = Loader.GetVersion();

		if (SelectedReplayInfo.Version.Server == ServerType::None)
		{
			MLog("Unknown replay version selected\n");
			SelectedReplayInfo.Dead = true;
			return;
		}

		SelectedReplayInfo.Timestamp = Loader.GetTimestamp();
		Loader.GetStageSetting(SelectedReplayInfo.StageSetting);

		if (SelectedReplayInfo.StageSetting.nGameType == MMATCH_GAMETYPE_DUEL)
		{
			Loader.GetDuelQueueInfo();
		}
		else if (SelectedReplayInfo.StageSetting.nGameType == MMATCH_GAMETYPE_GUNGAME)
		{
			(void)Loader.GetGunGameWeaponInfo();
		}

		auto InitialCharInfos = Loader.GetCharInfo();

		for (const auto &CharInfo : InitialCharInfos)
		{
			ReplayInfo::PlayerInfo Player;
			strcpy_safe(Player.Name, CharInfo.Info.szName);
			SelectedReplayInfo.PlayerInfos.insert({ CharInfo.State.UID, Player });
		}

		uint32_t WantedCommands[] = { MC_MATCH_RESPONSE_PEERLIST, MC_PEER_DIE };

		auto Array = ArrayView<u32>(WantedCommands);
		Loader.GetCommands(PerCommand, false, &Array);

		SelectedReplayInfo.VersionString = SelectedReplayInfo.Version.GetVersionString();
		SelectedReplayInfo.Dead = false;
	}
	catch (EOFException& e)
	{
		MLog("Unexpected EOF while reading replay %s at position %d\n", SelectedReplay->GetString(), e.GetPosition());
		SelectedReplayInfo.Dead = true;
	}
	catch (...)
	{
		MLog("Something went wrong while reading replay %s\n", SelectedReplay->GetString());
		SelectedReplayInfo.Dead = true;
	}
}

void RGMain::DrawReplayInfo(MDrawContext* pDC, MWidget* Widget) const
{
	v2 Offset{ 0, 0 };

	pDC->SetColor(ARGB(255, 255, 255, 255));

	auto Print = [&](const char *Format, ...)
	{
		char buf[256];

		va_list args;

		va_start(args, Format);
		int ret = _vsnprintf_s(buf, sizeof(buf) - 1, Format, args);
		va_end(args);

		pDC->Text(Offset.x, Offset.y, buf);

		Offset.y += 12;
	};

	if (SelectedReplayInfo.Dead)
	{
		Print("%s", SelectedReplayInfo.VersionString.c_str());
		Print("Failed to load replay");
		return;
	}

	[&]
	{
		auto BannerName = MGetBannerName(SelectedReplayInfo.StageSetting.szMapName);

		if (!BannerName)
			return;

		auto it = MapBanners.find(BannerName);

		if (it == MapBanners.end())
			return;

		auto Bitmap = it->second;
		pDC->SetBitmap(Bitmap);
		pDC->Draw(Offset.x, Offset.y, 260, 30);

		Offset.y += 30;
	}();

	Print("%s", SelectedReplayInfo.VersionString.c_str());

	[&]()
	{
		if (SelectedReplayInfo.Timestamp == 0)
			return;

		tm Tm;
		auto err = localtime_s(&Tm, &SelectedReplayInfo.Timestamp);
		if (err != 0)
			return;

		char buf[64];
		strftime(buf, sizeof(buf), "%x", &Tm);

		Print("%s", buf);
	}();

	Print("Map: %s", SelectedReplayInfo.StageSetting.szMapName);

	if(SelectedReplayInfo.StageSetting.szStageName[0])
		Print("Stage name: %s", SelectedReplayInfo.StageSetting.szStageName);

	auto* GameTypeManager = ZGetGameInterface()->GetGameTypeManager();
	if (GameTypeManager)
		Print("Gametype: %s",
			GameTypeManager->GetGameTypeStr(SelectedReplayInfo.StageSetting.nGameType));

	for (auto &Item : SelectedReplayInfo.PlayerInfos)
	{
		auto& Player = Item.second;

		Print("%s - %d/%d", Player.Name, Player.Kills, Player.Deaths);
	}
}

void RGMain::SetListeners()
{
	auto& MySettings = TrainingSettings;
	auto& BotSettings = GetBotInfo().Settings;

	static const std::pair<const char*, bool&> Buttons[] = {
		{"TrainingGodmodeOption", MySettings.Godmode},
		{"TrainingNoStunsOption", MySettings.NoStuns},
		{"TrainingBotGodmodeOption", BotSettings.Godmode},
		{"TrainingBotNoStunsOption", BotSettings.NoStuns},
		{"TrainingBotLoopOption", BotSettings.Loop},
		// Gotta call SetShowBotCam to set this one.
		{"TrainingBotCamOption", BotSettings.ShowCam},
	};

	auto LastIndex = std::size(Buttons) - 1;
	for (size_t i = 0; i < LastIndex; ++i)
	{
		Listen(Buttons[i].first, SetToButtonState(Buttons[i].second));
	}

	auto&& LastButton = Buttons[LastIndex];
	Listen(LastButton.first, ApplyToButtonState(SetShowBotCam));

	Listen("TrainingFrame", OnMessage(MBTN_CLK_MSG, [] {
		if (auto&& Widget = ZFindWidget("Training"))
		{
			Widget->Show(true, true);
		}
		else
		{
			return;
		}

		for (auto&& Button : Buttons)
		{
			if (auto&& Widget = ZFindWidgetAs<MButton>(Button.first))
			{
				Widget->SetCheck(Button.second);
			}
		}

		UpdateToggleBotWidget();
		UpdateToggleRecordingWidget();
	}));

	Listen("TrainingToggleBotButton", [](MWidget* Widget, const char* Message) {
		if (!MWidget::IsMsg(Message, MBTN_CLK_MSG))
			return false;

		if (!GetBotInfo().MyBot)
		{
			RequestCreateBot();
		}
		else
		{
			RequestDestroyBot();
		}

		// Toggle*Widget will be called when the response packets are received, in CreateBot and
		// DestroyBot.

		return true;
	});

	Listen("TrainingToggleRecordingButton", [=](MWidget* Widget, const char* Message) {
		if (!MWidget::IsMsg(Message, MBTN_CLK_MSG) ||
			!GetBotInfo().MyBot)
			return false;

		if (GetBotInfo().MyBot->GetState() == BotStateType::Recording)
		{
			GetBotInfo().MyBot->SetState(BotStateType::Replaying);
		}
		else
		{
			GetBotInfo().MyBot->SetState(BotStateType::Recording);
		}

		UpdateToggleRecordingWidget();

		return true;
	});

	Listen("TrainingCloseButton", OnMessage(MBTN_CLK_MSG, [] {
		if (auto&& Widget = ZFindWidget("Training"))
		{
			Widget->Show(false);
		}
	}));
}

std::pair<bool, uint32_t> RGMain::GetPlayerSwordColor(const MUID& UID)
{
	auto it = SwordColors.find(UID);

	if (it == SwordColors.end())
		return{ false, 0 };

	return{ true, it->second };
}

void RGMain::SetSwordColor(const MUID& UID, uint32_t Color)
{
	SwordColors[UID] = Color;

	auto Char = ZGetCharacterManager()->Find(UID);

	if (!Char)
		return;

	Char->m_pVMesh->SetCustomColor(Color & 0x80FFFFFF, Color & 0x0FFFFFFF);
}

void RGMain::OnReceiveVoiceChat(ZCharacter *Char, const uint8_t *Buffer, int Length)
{
#ifdef VOICECHAT
	m_VoiceChat.OnReceiveVoiceChat(Char, Buffer, Length);
#endif
}

void RGMain::OnGameCreate()
{
	if (auto&& Widget = ZFindWidget("TrainingFrame"))
	{
		const auto GameType = ZGetGameClient()->GetMatchStageSetting()->GetGameType();
		Widget->Enable(GameType == MMATCH_GAMETYPE_TRAINING);
	}
}

void RGMain::OnSlash(ZCharacter * Char, const rvector & Pos, const rvector & Dir)
{
	m_HitboxManager.OnSlash(Pos, Dir);
}

void RGMain::OnMassive(ZCharacter * Char, const rvector & Pos, const rvector & Dir)
{
	m_HitboxManager.OnMassive(Pos);
}

bool RGMain::IsCursorEnabled() const
{
	return GetChat().IsInputEnabled();
}
