#include "stdafx.h"
#include "ZMyBotCharacter.h"
#include "ZGameConst.h"
#include "ZRuleSkillmap.h"
#include "RGMain.h"
#include "Renderer.h"
#include "RS2.h"
#include "reinterpret.h"

void RequestCreateBot()
{
	if (GetBotInfo().MyBot)
	{
		ZChatOutput("Your bot already exists");
		return;
	}

	if (ZGetGameInterface()->GetState() != GUNZ_GAME)
	{
		ZChatOutput("Must be in game to create bot");
		return;
	}

	if (ZGetGameClient()->GetMatchStageSetting()->GetGameType() != MMATCH_GAMETYPE_TRAINING)
	{
		ZChatOutput("Must be in a training room to create bot");
		return;
	}

	ZPostCommand(ZNewCmd(MC_MATCH_REQUEST_CREATE_BOT));
}

void RequestDestroyBot()
{
	if (!GetBotInfo().MyBot)
	{
		ZChatOutput("Your bot hasn't been created");
		return;
	}

	ZPostStageLeaveBattle(GetBotInfo().MyBot->GetUID(), ZGetGameClient()->GetStageUID());
}

void DestroyAllBots()
{
	if (GetBotInfo().MyBot)
	{
		ZGetCharacterManager()->Delete(GetBotInfo().MyBot->GetUID());
		GetBotInfo().MyBot = nullptr;
	}
	
	for (auto&& NetBot : GetBotInfo().NetBots)
	{
		ZGetCharacterManager()->Delete(NetBot->GetUID());
	}

	GetBotInfo().NetBots.clear();
}

bool IsBot(ZCharacter* Char)
{
	return Char->IsBot;
}

bool IsMyBot(ZCharacter* Char)
{
	return Char == GetBotInfo().MyBot;
}

void UpdateToggleBotWidget()
{
	if (auto&& Widget = ZFindWidget("TrainingToggleBotButton"))
	{
		Widget->SetText(GetBotInfo().MyBot ? "Destroy bot (&B)" : "Create bot (&B)");
	}
}

void UpdateToggleRecordingWidget()
{
	if (auto&& Widget = ZFindWidget("TrainingToggleRecordingButton"))
	{
		bool BotExists = GetBotInfo().MyBot;
		Widget->Enable(BotExists);
		bool Recording = BotExists ? GetBotInfo().MyBot->GetState() == BotStateType::Recording : false;
		Widget->SetText(Recording ? "Stop recording (&Y)" : "Start recording (&Y)");
	}
}

void DestroyBot(ZCharacter* Bot)
{
	if (Bot == GetBotInfo().MyBot)
	{
		GetBotInfo().MyBot = nullptr;

		UpdateToggleBotWidget();
		UpdateToggleRecordingWidget();
	}
	else
	{
		auto&& NetBots = GetBotInfo().NetBots;
		auto it = std::find(std::begin(NetBots), std::end(NetBots), Bot);
		if (it != std::end(NetBots))
		{
			NetBots.erase(it);
		}
	}

	ZGetCharacterManager()->Delete(Bot->GetUID());
}

void CreateBot(const MUID& OwnerUID, const MUID& BotUID,
	const MTD_CharInfo& CharInfo, const MTD_ExtendInfo& ExtendInfo)
{
	ZCharacter* Bot;
	if (OwnerUID == MyChar()->GetUID())
	{
		auto* MyBot = new ZMyBotCharacter;
		Bot = GetBotInfo().MyBot = MyBot;
		MyBot->SetPosition(MyChar()->GetPosition());

		UpdateToggleBotWidget();
		UpdateToggleRecordingWidget();
	}
	else
	{
		auto* NetBot = new ZNetBotCharacter;
		NetBot->OwnerUID = OwnerUID;
		GetBotInfo().NetBots.push_back(NetBot);
		Bot = NetBot;
	}

	Bot->SetUID(BotUID);

	Bot->Create(CharInfo);
	ZGetCharacterManager()->Add(Bot);
	ZGetGame()->ConfigureCharacter(Bot->GetUID(),
		MMatchTeam(ExtendInfo.nTeam),
		ExtendInfo.nPlayerFlags);
}

static bool VerifyOwnership(const MUID& OwnerUID, const MUID& BotUID)
{
	auto* Bot = ZGetCharacterManager()->Find(BotUID);
	if (!Bot)
		return false;

	if (Bot == GetBotInfo().MyBot)
		return OwnerUID == MyChar()->GetUID();

	auto&& NetBots = GetBotInfo().NetBots;
	auto it = std::find(std::begin(NetBots), std::end(NetBots), Bot);
	if (it == std::end(NetBots))
		return false;

	return (**it).OwnerUID == OwnerUID;
}

void RunNetBotTunnelledCommand(MCommand* Command)
{
	MUID BotUID;
	if (!Command->GetParameter(&BotUID, 0, MPT_UID)) return;
	if (Command->GetParameter(1)->GetType() != MPT_BLOB) return;

	if (!VerifyOwnership(Command->GetSenderUID(), BotUID))
		return;

	auto* Blob = static_cast<MCmdParamBlob*>(Command->GetParameter(1));

	auto* InnerCommand = new MCommand;

	auto* Data = static_cast<const char*>(Blob->GetPointer());
	auto* CmdMgr = ZGetGameClient()->GetCommandManager();
	auto Size = u16(Blob->GetPayloadSize());
	if (!InnerCommand->SetData(Data, CmdMgr, Size))
	{
		assert(false);
		return;
	}

	InnerCommand->SetSenderUID(BotUID);

	ZGetGameClient()->OnCommand(InnerCommand);
}

ZMyBotCharacter::ZMyBotCharacter()
{
	IsBot = true;
	m_pModule_HPAP->SetRealDamage(true);
	Camera.TargetCharacterOverride = this;
}

ZMyBotCharacter::~ZMyBotCharacter() = default;

static auto NewCmdFrom(int ID, const MUID& UID)
{
	MCommandDesc* pCmdDesc = ZGetGameClient()->GetCommandManager()->GetCommandDescByID(ID);

	MUID uidTarget;
	if (pCmdDesc->IsFlag(MCDT_PEER2PEER))
		uidTarget = MUID(0, 0);
	else
		uidTarget = ZGetGameClient()->GetServerUID();

	auto Cmd = std::make_unique<MCommand>(ID,
		UID,
		uidTarget,
		ZGetGameClient()->GetCommandManager());
	return Cmd;
}

void ZMyBotCharacter::PostBotCmd(MCommand& Command)
{
	auto* Blob = new MCommandParameterBlob{size_t(Command.GetSize())};
	if (!Command.GetData(static_cast<char*>(Blob->GetPointer()), Blob->GetSize()))
	{
		assert(false);
		return;
	}

	auto* WrapperCommand = ZNewCmd(MC_PEER_TUNNEL_BOT_COMMAND);
	WrapperCommand->AddParameter(new MCmdParamUID{GetUID()});
	WrapperCommand->AddParameter(Blob);
	ZPostCommand(WrapperCommand);
}

template <typename... T>
void ZMyBotCharacter::PostBotCmd(int ID, T&&... Args)
{
	auto InnerCommand = NewCmdFrom(ID, GetUID());
	ZPostCmd_AddParameters(InnerCommand.get(), std::forward<T>(Args)...);
	PostBotCmd(*InnerCommand);
}

void ZMyBotCharacter::PostBasicInfo()
{
	if (State == BotStateType::Replaying)
		return;

	CharacterInfo CharInfo;
	CharInfo.Pos = GetPosition();
	CharInfo.Dir = GetDirection();
	CharInfo.Vel = GetVelocity();
	CharInfo.LowerAni = GetStateLower();
	CharInfo.UpperAni = GetStateUpper();
	CharInfo.Slot = GetItems()->GetSelectedWeaponParts();
	CharInfo.HasCamDir = false;

	auto Blob = PackNewBasicInfo(CharInfo, BasicInfoState, ZGetGame()->GetTime()); 
	assert(Blob);

	auto Cmd = NewCmdFrom(MC_PEER_BASICINFO_RG, GetUID());
	Cmd->AddParameter(Blob);
	PostBotCmd(*Cmd);
}

void ZMyBotCharacter::AdjustTimes(MCommand& Command)
{
	switch (Command.GetID())
	{
	case MC_PEER_BASICINFO_RG:
	{
		auto& Blob = *static_cast<MCmdParamBlob*>(Command.GetParameter(0));
		auto* CharPtr = static_cast<char*>(Blob.GetPointer());
		auto* TimePtr = static_cast<void*>(CharPtr + 1);
		auto CurTime = reinterpret_pointee<float>(TimePtr);
		auto Delta = ReplayStartTime - RecordingStartTime;
		float NewTime = CurTime + Delta;
		memcpy(TimePtr, &NewTime, sizeof(float));
	}
		break;
	}
}

void ZMyBotCharacter::ReplayCommands()
{
	auto GameTime = ZGetGame()->GetTime();
	auto RelativeTime = GameTime - ReplayStartTime;

	auto GetIt = [&] { return RecordedCommands.begin() + CommandIndex; };
	auto it = GetIt();
	while (it != RecordedCommands.end() && it->RelativeTime <= RelativeTime)
	{
		{
			auto&& Command = *it->Command;
			AdjustTimes(Command);
			PostBotCmd(Command);
		}

		++CommandIndex;
		if (CommandIndex >= RecordedCommands.size())
		{
			if (GetBotInfo().Settings.Loop)
			{
				CommandIndex %= RecordedCommands.size();
				ReplayStartTime = GameTime;
				RelativeTime = 0;
			}
			else
			{
				SetState(BotStateType::Inactive);
				break;
			}
		}

		it = GetIt();
	}
}

void ZMyBotCharacter::OnUpdate(float Delta)
{
	if (GetBotInfo().BotCamRenderTarget)
	{
		Camera.Update(Delta);
	}

	if (State == BotStateType::Replaying)
	{
		ReplayCommands();

		v3 Pos, Dir, CamDir;
		if (GetHistory(&Pos, &Dir, g_pGame->GetTime(), &CamDir))
		{
			m_Position = Pos;
			m_Direction = CamDir;
			m_DirectionLower = Dir;
			m_TargetDir = CamDir;
		}

		ZCharacter::OnUpdate(Delta);
	}
	else
	{
		ZCharacter::OnUpdate(Delta);

		auto NowTime = GetGlobalTimeMS();
		if (NowTime - LastBasicInfoSendTimeMS >= BASICINFO_INTERVAL)
		{
			LastBasicInfoSendTimeMS = NowTime;

			PostBasicInfo();
		}

		CheckDead();

		UpdateAnimation();
		UpdateVelocity(Delta);

		if (GetDistToFloor() < 0 && !IsDie())
		{
			float fAdjust = 400.f * Delta;
			rvector diff = rvector(0, 0, min(-GetDistToFloor(), fAdjust));
			Move(diff);
		}
	}
}

bool ZMyBotCharacter::GetHistory(v3* Pos, v3* Dir, float Time, v3* CameraDir)
{
	if (State != BotStateType::Replaying)
	{
		return ZCharacter::GetHistory(Pos, Dir, Time, CameraDir);
	}

	const auto GetItemDesc = [&](auto Slot) { return m_Items.GetDesc(Slot); };
	const auto Sex = IsMan() ? MMS_MALE : MMS_FEMALE;
	const auto RelativeTime = Time - ReplayStartTime;

	BasicInfoHistoryManager::Info Info;
	Info.Pos = Pos;
	Info.Dir = Dir;
	Info.CameraDir = CameraDir;

	return RecordedHistory.GetInfo(Info, RelativeTime, std::ref(GetItemDesc), Sex, IsDie());
}

void ZMyBotCharacter::OnDamaged(ZObject* pAttacker, rvector srcPos, ZDAMAGETYPE damageType,
	MMatchWeaponType weaponType, float fDamage, float fPiercingRatio, int nMeleeType)
{
	if (!GetBotInfo().Settings.Godmode)
	{
		HandleDamage(pAttacker, srcPos, damageType, weaponType, fDamage, fPiercingRatio, nMeleeType);
	}
}

void ZMyBotCharacter::OnDamagedAnimation(ZObject *pAttacker, int type)
{
	if (!GetBotInfo().Settings.NoStuns)
	{
		ZCharacter::OnDamagedAnimation(pAttacker, type);
	}
}

void ZMyBotCharacter::OnKnockback(const rvector& dir, float fForce)
{
	if (!GetBotInfo().Settings.NoStuns)
	{
		ZCharacter::OnKnockback(dir, fForce);
	}
}

static bool ShouldRecord(MCommand* Command)
{
	switch (Command->GetID())
	{
	case MC_PEER_BASICINFO:
	case MC_PEER_BASICINFO_RG:
	case MC_PEER_RG_SLASH:
	case MC_PEER_RG_MASSIVE:
	case MC_PEER_SHOT:
	case MC_PEER_SHOT_SP:
	case MC_PEER_SHOT_MELEE:
	case MC_PEER_SKILL:
	case MC_PEER_REACTION:
		return true;
	default:
		return false;
	}
}

void ZMyBotCharacter::RecordBasicInfo(MCommand* Command)
{
	auto&& Blob = *static_cast<MCmdParamBlob*>(Command->GetParameter(0));
	const auto BlobPtr = static_cast<u8*>(Blob.GetPointer());
	const auto BlobSize = Blob.GetPayloadSize();

	NewBasicInfo nbi;
	if (!UnpackNewBasicInfo(nbi, BlobPtr, BlobSize))
	{
		return;
	}

	const auto Time = ZGetGame()->GetTime() - RecordingStartTime;
	nbi.bi.RecvTime = nbi.bi.SentTime = Time;

	RecordedHistory.AddBasicInfo(nbi.bi);
}

void ZMyBotCharacter::RecordCommand(MCommand* Command)
{
	if (!ShouldRecord(Command))
	{
		return;
	}

	if (Command->GetID() == MC_PEER_BASICINFO_RG)
	{
		RecordBasicInfo(Command);
	}

	auto NewCommand = std::unique_ptr<MCommand>{Command->Clone()};
	NewCommand->SetSenderUID(GetUID());

	assert(ZGetGame()->GetTime() >= RecordingStartTime);
	RecordedCommands.push_back({ZGetGame()->GetTime() - RecordingStartTime, std::move(NewCommand)});
}

void ZMyBotCharacter::AddInitialSyncBasicInfo()
{
	CharacterInfo CharInfo;
	CharInfo.Pos = MyChar()->GetPosition();
	CharInfo.Dir = MyChar()->GetDirection();
	CharInfo.Vel = MyChar()->GetVelocity();
	CharInfo.CamDir = RCameraDirection;
	CharInfo.LowerAni = MyChar()->GetStateLower();
	CharInfo.UpperAni = MyChar()->GetStateUpper();
	CharInfo.Slot = MyChar()->GetItems()->GetSelectedWeaponParts();
	CharInfo.HasCamDir = MyChar()->IsDirLocked();

	BasicInfoNetState ThrowawayState;
	auto Blob = PackNewBasicInfo(CharInfo, ThrowawayState, ZGetGame()->GetTime());
	assert(Blob);
	if (!Blob)
		return;

	auto Cmd = std::unique_ptr<MCommand>{ZNewCmd(MC_PEER_BASICINFO_RG)};
	Cmd->AddParameter(Blob);
	RecordCommand(Cmd.get());
}

void ZMyBotCharacter::SetState(BotStateType Value)
{
	State = Value;

	switch (State)
	{
	case BotStateType::Recording:
		RecordedCommands.clear();
		RecordedHistory.clear();
		RecordingStartTime = ZGetGame()->GetTime();
		AddInitialSyncBasicInfo();
		break;

	case BotStateType::Replaying:
		ReplayStartTime = ZGetGame()->GetTime();
		CommandIndex = 0;
		RecordedPeriod = ReplayStartTime - RecordingStartTime;
		break;
	}
}

static void CreateBotCamRenderTarget()
{
	const auto Width = RGetScreenWidth();
	const auto Height = RGetScreenHeight();
	const auto Levels = 1;
	const auto Usage = D3DUSAGE_RENDERTARGET;
	const auto Format = RGetPixelFormat();
	const auto Pool = D3DPOOL_DEFAULT;
	if (DXERR(RGetDevice()->CreateTexture(Width, Height, Levels,
		Usage, Format, Pool,
		MakeWriteProxy(GetBotInfo().BotCamRenderTarget), nullptr)))
	{
		GetBotInfo().Settings.ShowCam = false;
	}
}

void SetShowBotCam(bool Value)
{
	GetBotInfo().Settings.ShowCam = Value;
	if (Value && !GetBotInfo().BotCamRenderTarget)
	{
		CreateBotCamRenderTarget();
	}
}

void InvalidateBotCamRT()
{
	GetBotInfo().BotCamRenderTarget = nullptr;
}

void RestoreBotCamRT()
{
	if (GetBotInfo().Settings.ShowCam)
	{
		CreateBotCamRenderTarget();
	}
}

void ZMyBotCharacter::RenderCam()
{
	if (DontDraw || !GetBotInfo().BotCamRenderTarget)
	{
		return;
	}

	auto dev = RGetDevice();

	D3DPtr<IDirect3DSurface9> RT;
	GetBotInfo().BotCamRenderTarget->GetSurfaceLevel(0, MakeWriteProxy(RT));

	{
		auto OrigPos = RCameraPosition;
		auto OrigDir = RCameraDirection;
		auto OrigUp = RCameraUp;

		D3DPtr<IDirect3DSurface9> OrigRT;
		dev->GetRenderTarget(0, MakeWriteProxy(OrigRT));

		dev->SetRenderTarget(0, RT.get());

		dev->Clear(0, nullptr, D3DCLEAR_TARGET, 0x00000000, 1.0f, 0.0f);

		RSetTransform(D3DTS_WORLD, IdentityMatrix());

		auto Pos = Camera.GetPosition();
		auto Dir = Camera.GetCurrentDir();
		auto Up = v3{0, 0, 1};

		RSetCamera(Pos, Pos + Dir, Up);

		DontDraw = true;

		ZGetGame()->Draw();

		DontDraw = false;

		dev->SetRenderTarget(0, OrigRT.get());

		RSetCamera(OrigPos, OrigPos + OrigDir, OrigUp);
	}

	dev->Clear(0, nullptr, D3DCLEAR_ZBUFFER, 0x00000000, 1.0f, 0.0f);

	RSetTransform(D3DTS_WORLD, IdentityMatrix());

	dev->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
	dev->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_ONE);
	dev->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_ZERO);
	dev->SetRenderState(D3DRS_ZWRITEENABLE, true);

	dev->SetRenderState(D3DRS_LIGHTING, FALSE);
	dev->SetRenderState(D3DRS_ZENABLE, TRUE);

	dev->SetTexture(0, GetBotInfo().BotCamRenderTarget.get());
	dev->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
	dev->SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);
	dev->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
	dev->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG2);
	dev->SetTextureStageState(0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE);

	dev->SetTexture(1, nullptr);
	dev->SetTextureStageState(1, D3DTSS_COLOROP, D3DTOP_DISABLE);
	dev->SetTextureStageState(1, D3DTSS_ALPHAOP, D3DTOP_DISABLE);

	GetRenderer().SetScreenSpaceVertexDeclaration();

	auto w = float(RGetScreenWidth());
	auto h = float(RGetScreenHeight());
	v2 a = {w * 0.75f, h * 0.375f};
	v2 b = {w, h * 0.625f};
	DrawQuad(a, b);
}

void ZMyBotCharacter::CheckDead()
{
	if (IsDie())
		return;

	MUID uidAttacker = MUID(0, 0);

	if (GetPosition().z < DIE_CRITICAL_LINE)
	{
		if (ZGetGameClient()->GetMatchStageSetting()->GetGameType() == MMATCH_GAMETYPE_SKILLMAP)
		{
			static_cast<ZRuleSkillmap *>(ZGetGame()->GetMatch()->GetRule())->OnFall();
		}
		else
		{
			uidAttacker = GetLastThrower();

			ZObject *pAttacker = ZGetObjectManager()->GetObject(uidAttacker);
			if (pAttacker == NULL || !ZGetGame()->IsAttackable(pAttacker, this))
			{
				uidAttacker = ZGetMyUID();
				pAttacker = this;
			}
			OnDamaged(pAttacker, GetPosition(),
				ZD_FALLING, MWT_NONE, GetHP());
		}
	}

	if (ZGetGameClient()->GetMatchStageSetting()->GetNetcode() == NetcodeType::ServerBased)
		return;

	if ((IsDie() == false) && (GetHP() <= 0))
	{
		if (uidAttacker == MUID(0, 0) && GetLastAttacker() != MUID(0, 0))
			uidAttacker = GetLastAttacker();

		ActDead();
		Die();

		PostBotCmd(MC_PEER_DIE, MCmdParamUID(uidAttacker));

		if (!ZGetGameTypeManager()->IsQuestDerived(ZGetGameClient()->GetMatchStageSetting()->GetGameType()))
		{
			// We would normally send a MC_MATCH_GAME_KILL, but that has to be handled by the
			// server, which doesn't exist in local dev mode, so we just call OnPeerDead to
			// simulate receiving it.
			//PostBotCmd(MC_MATCH_GAME_KILL, MCommandParameterUID(uidAttacker));
			ZGetGame()->OnPeerDead(uidAttacker, 0, GetUID(), 0);
		}
		else
		{
			PostBotCmd(MC_MATCH_QUEST_REQUEST_DEAD);
		}

		Revival();
	}
}