#pragma once

#include <vector>
#include "optional.h"
#include "BasicInfo.h"
#include "ZMyCharacter.h"
#include "ZNetCharacter.h"
#include "ZCamera.h"

enum class BotStateType
{
	Inactive,
	Recording,
	Replaying,
};

class ZMyBotCharacter : public ZCharacter
{
public:
	ZMyBotCharacter();
	~ZMyBotCharacter();

	void OnUpdate(float) override;
	virtual void OnDamaged(ZObject* pAttacker, rvector srcPos, ZDAMAGETYPE damageType, MMatchWeaponType weaponType,
		float fDamage, float fPiercingRatio = 1.f, int nMeleeType = -1) override;
	virtual void OnDamagedAnimation(ZObject *pAttacker, int type) override;
	virtual void OnKnockback(const rvector& dir, float fForce) override;
	virtual bool GetHistory(v3* Pos, v3* Dir, float Time, v3* CameraDir = nullptr) override;

	void RecordCommand(MCommand* Command);
	void SetState(BotStateType Value);
	auto GetState() const { return State; }

	void RenderCam();

private:
	void PostBotCmd(MCommand& Command);
	template <typename... T>
	void PostBotCmd(int ID, T&&... Args);
	void CheckDead();
	void PostBasicInfo();
	void RecordBasicInfo(MCommand* Command);

	void AddInitialSyncBasicInfo();

	BasicInfoNetState BasicInfoState;
	u64 LastBasicInfoSendTimeMS{};

	BotStateType State = BotStateType::Inactive;

	struct RecordedCommand
	{
		float RelativeTime;
		std::unique_ptr<MCommand> Command;
	};
	std::vector<RecordedCommand> RecordedCommands;

	BasicInfoHistoryManager RecordedHistory;

	float ReplayStartTime{};
	size_t CommandIndex{};

	void AdjustTimes(MCommand& Command);
	void ReplayCommands();

	float RecordingStartTime{};
	float RecordedPeriod{};

	bool DontDraw{};

	ZCamera Camera;
};

void RequestCreateBot();
void RequestDestroyBot();
void DestroyAllBots();
void DestroyBot(ZCharacter* Bot);
bool IsBot(ZCharacter* Char);
bool IsMyBot(ZCharacter* Char);

class ZNetBotCharacter : public ZNetCharacter
{
public:
	ZNetBotCharacter()
	{
		IsBot = true;
	}

	MUID OwnerUID;
};

void CreateBot(const MUID& OwnerUID, const MUID& BotUID,
	const MTD_CharInfo& CharInfo, const MTD_ExtendInfo& ExtendInfo);
void RunNetBotTunnelledCommand(MCommand* Command);

struct BotInfo
{
	ZMyBotCharacter* MyBot;
	std::vector<ZNetBotCharacter*> NetBots;
	D3DPtr<IDirect3DTexture9> BotCamRenderTarget;

	struct {
		bool Loop = true;
		bool Godmode = true;
		bool NoStuns{};
		bool ShowCam{};
	} Settings;
};

inline BotInfo& GetBotInfo()
{
	static BotInfo x;
	return x;
}

void SetShowBotCam(bool Value);
void InvalidateBotCamRT();
void RestoreBotCamRT();
void UpdateToggleBotWidget();
void UpdateToggleRecordingWidget();