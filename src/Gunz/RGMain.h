#pragma once

// Sort this stuff sometime maybe '__'

#include <mutex>
#include <queue>
#include <memory>
#include "Config.h"
#include "VoiceChat.h"
#include "Tasks.h"
#include "Hitboxes.h"
#include "ZReplay.h"
#include "NewChat.h"
#include "optional.h"
#include <DxErr.h>

inline bool DXErr(HRESULT hr, const char* CallingFunction, const char* DXFunction)
{
	if (SUCCEEDED(hr))
		return false;

	MLog("In %s, %s failed -- error code: %s, description: %s\n",
		CallingFunction, DXFunction, DXGetErrorString(hr), DXGetErrorDescription(hr));

	return true;
}

// Returns true if DirectX expression failed, and false otherwise. Additionally, if it failed,
// it logs the error.
#define DXERR(expr) DXErr(expr, __func__, #expr)

class ZChatCmdManager;
class MEvent;
class ZIDLResource;

HRESULT GenerateTexture(IDirect3DDevice9 *pD3Ddev, IDirect3DTexture9 **ppD3Dtex, DWORD colour32);
void LoadRGCommands(ZChatCmdManager &CmdManager);
std::pair<bool, std::vector<unsigned char>> ReadMZFile(const char *szPath);
std::pair<bool, std::vector<unsigned char>> ReadZFile(const char *szPath);
void Invoke(std::function<void()> fn);
enum class PlayerFoundStatus
{
	FoundOne,
	NotFound,
	TooManyFound,
};
std::pair<PlayerFoundStatus, ZCharacter*> FindSinglePlayer(const char* NameSubstring);

struct ReplayInfo
{
	ReplayVersion Version;
	REPLAY_STAGE_SETTING_NODE StageSetting;
	std::string VersionString;
	time_t Timestamp = 0;
	bool Dead = true;

	struct PlayerInfo
	{
		char Name[MATCHOBJECT_NAME_LENGTH];
		int Kills = 0;
		int Deaths = 0;
	};

	std::unordered_map<MUID, PlayerInfo> PlayerInfos;
};

class RGMain
{
public:
	RGMain();
	RGMain(const RGMain&) = delete;
	~RGMain();

	void OnUpdate(double Elapsed);
	bool OnEvent(MEvent *pEvent);
	void OnInvalidate();
	void OnRestore();
	void OnInitInterface(ZIDLResource &IDLResource);
	void OnReplaySelected(MListBox* ReplayFileListWidget);
	void OnCreateDevice();
	void OnAppCreate();
#ifdef VOICECHAT
	auto MutePlayer(const MUID& UID) { return m_VoiceChat.MutePlayer(UID); }
#endif

	void OnDrawLobby(MDrawContext* pDC);
	void OnDrawGame();
	void OnPreDrawGame();
	void OnDrawGameInterface(MDrawContext* pDC);
	void OnGameCreate();
	bool OnGameInput();

	void Resize(int w, int h);

	void OnSlash(ZCharacter* Char, const rvector& Pos, const rvector& Dir);
	void OnMassive(ZCharacter* Char, const rvector& Pos, const rvector& Dir);

	void SetSwordColor(const MUID& UID, uint32_t Hue);

	void AddMapBanner(const char* MapName, MBitmap* Bitmap) { MapBanners.insert({ MapName, Bitmap }); }

	void OnReceiveVoiceChat(ZCharacter *Char, const uint8_t *Buffer, int Length);

	bool IsCursorEnabled() const;

	double GetTime() const { return Time; }
	double GetElapsedTime() const { return Time - LastTime; }

	std::pair<bool, uint32_t> GetPlayerSwordColor(const MUID& UID);

	// Invokes a callback on the main thread
	template<typename T>
	void Invoke(T Callback, double Delay = 0)
	{
		std::lock_guard<std::mutex> lock(QueueMutex);
		QueuedInvokations.push_back({ Callback, Time + Delay });
	}

	Chat& GetChat() { return m_Chat.value(); }
	const Chat& GetChat() const { return m_Chat.value(); }
	bool IsNewChatEnabled() const { return NewChatEnabled; }

	void DrawReplayInfo(MDrawContext* pDC, MWidget* Widget) const;

	void SetListeners();

	// Lines created with /drawline
	struct LineInfo
	{
		v3 v1, v2;
		u32 Color;
	};
	std::vector<LineInfo> Lines;

	struct {
		bool Godmode{};
		bool NoStuns{};
	} TrainingSettings;

private:
	friend void LoadRGCommands(ZChatCmdManager& CmdManager);

	double Time = 0;
	double LastTime = 0;

	struct Invokation
	{
		std::function<void()> fn;
		double Time;
	};

	std::vector<Invokation> QueuedInvokations;
	std::mutex QueueMutex;

	bool Selected = false;
	ReplayInfo SelectedReplayInfo;

	std::unordered_map<std::string, MBitmap *> MapBanners;

	std::unordered_map<MUID, uint32_t> SwordColors;

#ifdef VOICECHAT
	VoiceChat m_VoiceChat;
#endif
	HitboxManager m_HitboxManager;

	optional<Chat> m_Chat;

#ifdef NEW_CHAT
	bool NewChatEnabled = true;
#else
	bool NewChatEnabled = false;
#endif
};

 RGMain& GetRGMain();
 void CreateRGMain();
 void DestroyRGMain();
 bool IsRGMainAlive();
 inline Chat& GetNewChat() { return GetRGMain().GetChat(); }

inline ZMyCharacter* MyChar()
{
	return ZGetGame()->m_pMyCharacter;
}

inline auto FixedFOV(float x) {
	return 2.f * atan(tan((2.f * atan(tan(x / 2.f) / (4.f / 3.f))) / 2.f) * RGetAspect());
}
