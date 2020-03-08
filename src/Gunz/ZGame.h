#pragma once

#include "ZPrerequisites.h"

#include "MMatchClient.h"
#include "MDataChecker.h"

#include "RTypes.h"
#include "ZMatch.h"
#include "ZGameTimer.h"
#include "ZWater.h"
#include "ZClothEmblem.h"
#include "ZEffectManager.h"
#include "ZWeaponMgr.h"
#include "ZHelpScreen.h"
#include "ZCharacterManager.h"
#include "ZObjectManager.h"
#include "ZWorld.h"
#include "ZGameAction.h"
#include <utility>
#include "ZGameDraw.h"
#include "BasicInfo.h"
#include "function_view.h"

_USING_NAMESPACE_REALSPACE2

class MZFileSystem;
class ZLoading;
class ZGameAction;
class ZSkyBox;
class ZFile;
class ZObject;
class ZCharacter;
class ZMyCharacter;
class ZMiniMap;
class ZMsgBox;
class ZInterfaceBackground;
class ZCharacterSelectView;
class ZScreenEffectManager;
class ZPlayerMenu;
class ZGameClient;
class ZMapDesc;
class ZReplayLoader;

enum ZGAME_READYSTATE {
	ZGAME_READYSTATE_INIT,
	ZGAME_READYSTATE_WAITSYNC,
	ZGAME_READYSTATE_RUN
};

struct ZPICKINFO;

struct ZObserverCommandItem {
	float fTime;
	MCommand *pCommand;
};

class ZObserverCommandList : public std::list<ZObserverCommandItem*> {
public:
	~ZObserverCommandList() { Destroy(); }
	void Destroy() {
		while(!empty())
		{
			ZObserverCommandItem *pItem=*begin();
			delete pItem->pCommand;
			delete pItem;
			erase(begin());
		}
	}
};

class PingMap
{
public:
	std::pair<bool, float> GetTime(u32 Timestamp) const
	{
		for (auto&& val : Pings)
		{
			if (val.Timestamp == Timestamp)
				return{ true, val.ReplayTime };
		}

		return{ false, 0.0f };
	}

	void AddTime(u32 Timestamp, float ReplayTime)
	{
		Pings.emplace_back(PingInfo{ Timestamp, ReplayTime });

		if (Pings.size() > 30)
		{
			Pings.erase(Pings.begin() + 10, Pings.end());
		}
	}

private:
	struct PingInfo
	{
		u32 Timestamp;
		float ReplayTime;
	};

	std::vector<PingInfo> Pings;
};

class ZGame
{
public:
	ZGame();
	~ZGame();

	bool Create(MZFileSystem *pfs, ZLoadingProgress *pLoading);

	void Draw();
	void Draw(MDrawContextR2 &dc);
	void Update(float fElapsed);
	void Destroy();

	void OnCameraUpdate(float fElapsed);
	void OnInvalidate();
	void OnRestore();

	void ParseReservedWord(char* pszDest, size_t maxlen, const char* pszSrc);
	template <size_t size>
	void ParseReservedWord(char(&Dest)[size], const char* Src) {
		return ParseReservedWord(Dest, size, Src);
	}

	bool m_bShowWireframe;

	void ShowReplayInfo(bool bShow);

	void OnExplosionGrenade(MUID uidOwner, rvector pos, float fDamage, float fRange, float fMinDamage, float fKnockBack, MMatchTeam nTeamID);
	void OnExplosionMagic(ZWeaponMagic *pWeapon, MUID uidOwner, rvector pos, float fMinDamage, float fKnockBack, MMatchTeam nTeamID, bool bSkipNpc);
	void OnExplosionMagicNonSplash(ZWeaponMagic *pWeapon, MUID uidOwner, MUID uidTarget, rvector pos, float fKnockBack);
	void OnReloadComplete(ZCharacter *pCharacter);
	void OnPeerShotSp(const MUID& uid, float fShotTime, const rvector& pos, const rvector& dir, int type, MMatchCharItemParts sel_type);
	void OnChangeWeapon(const MUID& uid, MMatchCharItemParts parts);

	rvector GetMyCharacterFirePosition(void);

	void CheckMyCharDead(float fElapsed);

	void CheckStylishAction(ZCharacter* pCharacter);
	void CheckCombo(ZCharacter *pOwnerCharacter, ZObject *pHitObject, bool bPlaySound);
	void UpdateCombo(bool bShot = false);

	void PostBasicInfo();
	void PostHPInfo();
	void PostHPAPInfo();
	void CheckSuicide();
	void PostPeerPingInfo();
	void PostSyncReport();

	int  SelectSlashEffectMotion(ZCharacter* pCharacter);
	bool CheckWall(ZObject* pObj1, ZObject* pObj2);

	void InitRound();
	void AddEffectRoundState(MMATCH_ROUNDSTATE nRoundState, int nArg);

	bool CreateMyCharacter(const MTD_CharInfo& pCharInfo);
	void DeleteCharacter(const MUID& uid);
	void RefreshCharacters();
	void ConfigureCharacter(const MUID& uidChar, MMatchTeam nTeam, unsigned char nPlayerFlags);

	bool OnCommand(MCommand* pCommand);
	bool OnCommand_Immediate(MCommand* pCommand);

	void ToggleRecording();
	void StartRecording();
	void StopRecording();

	bool OnLoadReplay(ZReplayLoader* pLoader);
	bool IsReplay() const { return m_bReplaying; }
	bool IsShowReplayInfo() const { return m_bShowReplayInfo; }
	void EndReplay();

	float GetReplayTime() const;
	float GetReplayLength() const;

	void OnReplayRun();

	void SetReplayTime(float Time);

	void OnObserverRun();
	void OnCommand_Observer(MCommand* pCommand);
	void FlushObserverCommands();
	int	GetObserverCommandListCount() const { return static_cast<int>(m_ObserverCommandList.size()); }
	void ClearObserverCommandList();

	void ReserveObserver();
	void ReleaseObserver();

	ZMatch* GetMatch() { return &m_Match; }
	ZMapDesc* GetMapDesc() { return GetWorld() ? GetWorld()->GetDesc() : NULL; }
	ZWorld* GetWorld() { return ZGetWorldManager()->GetCurrent(); }

	ZGameTimer* GetGameTimer() { return &m_GameTimer; }
	auto GetTickTime() const { return m_GameTimer.GetGlobalTick(); }
	auto GetTime() const { return m_fTime; }

	MDataChecker* GetDataChecker() { return &m_DataChecker; }

	rvector GetFloor(rvector pos, rplane *pimpactplane = NULL);

	bool Pick(ZObject *pOwnerObject, const rvector &origin, const rvector &dir, ZPICKINFO *pickinfo,
		DWORD dwPassFlag = RM_FLAG_ADDITIVE | RM_FLAG_HIDE, bool bMyChar = false);
	bool PickTo(ZObject *pOwnerObject, const rvector &origin, const rvector &to, ZPICKINFO *pickinfo,
		DWORD dwPassFlag = RM_FLAG_ADDITIVE | RM_FLAG_HIDE, bool bMyChar = false);
	bool PickHistory(ZObject *pOwnerObject, float fTime, const rvector &origin, const rvector &to,
		ZPICKINFO *pickinfo, DWORD dwPassFlag, bool bMyChar = false);
	bool ObjectColTest(ZObject* pOwner, const rvector& origin, const rvector& to, float fRadius,
		ZObject** poutTarget);

	char* GetSndNameFromBsp(const char* szSrcSndName, RMATERIAL* pMaterial);

	bool CheckGameReady();
	ZGAME_READYSTATE GetReadyState() { return m_nReadyState; }
	void SetReadyState(ZGAME_READYSTATE nState) { m_nReadyState = nState; }

	bool GetSpawnRequested() { return m_bSpawnRequested; }
	void SetSpawnRequested(bool bVal) { m_bSpawnRequested = bVal; }

	bool GetUserNameColor(MUID uid, MCOLOR& color, char* sp_name, size_t maxlen);
	template <size_t size>
	bool GetUserNameColor(MUID uid, MCOLOR& color, char(&sp_name)[size]) {
		return GetUserNameColor(uid, color, sp_name, size);
	}
	bool IsAttackable(ZObject *pAttacker, ZObject *pTarget);

	void OnPeerShot(const MUID& uid, float fShotTime, rvector& pos, rvector& to, MMatchCharItemParts sel_type);

	void PostSpMotion(ZC_SPMOTION_TYPE type);

	void OnPeerShot_Melee(const MUID& uidOwner, float fShotTime);
	void OnPeerShot_Range(MMatchCharItemParts sel_type, ZObject* pOwner, float fShotTime,
		rvector pos, rvector to, u32 seed);
	void OnPeerShot_Shotgun(ZItem *pItem, ZCharacter* pOwnerCharacter, float fShotTime,
		rvector& pos, rvector& to, u32 seed);

	void OnPeerSlash(ZCharacter *pOwner, const rvector &pos, const rvector &dir, int Type);
	void OnPeerMassive(ZCharacter *pOwner, const rvector &pos, const rvector &dir);

	void OnPeerChat(const MUID& Sender, MMatchTeam Team, const char* Message);

	void ReserveSuicide();
	bool IsReservedSuicide() { return m_bSuicide; }
	void CancelSuicide() { m_bSuicide = false; }

	ZObserverCommandList* GetReplayCommandList() { return &m_ReplayCommandList; }

	RParticles			*m_pParticles{};

	ZMyCharacter*		m_pMyCharacter{};
	ZCharacterManager	m_CharacterManager;
	ZObjectManager		m_ObjectManager;

	RVisualMeshMgr		m_VisualMeshMgr;

	ZEffectManager*		m_pEffectManager;
	ZWeaponMgr			m_WeaponManager;

	int					m_render_poly_cnt{};

	ZHelpScreen	m_HelpScreen;

protected:
	int	m_nGunzReplayNumber;
	ZFile *m_pReplayFile;
	bool m_bReplaying;
	bool m_bShowReplayInfo;

	bool m_bRecording;
	bool m_bReserveObserver;

	bool m_bSuicide;
	DWORD m_dwReservedSuicideTime;

	u64 m_nReservedObserverTime;
	int m_t;
	ZMatch m_Match;
	u64	m_nSpawnTime;
	bool m_bSpawnRequested;

	ZObserverCommandList m_ObserverCommandList;
	ZObserverCommandList m_ReplayCommandList;

	ZObserverCommandList m_DelayedCommandList;

	void CheckKillSound(ZCharacter* pAttacker);
	
	void OnReserveObserver();
	void DrawDebugInfo();

	void OnStageEnterBattle(MCmdEnterBattleParam nParam, MTD_PeerListNode* pPeerNode);
	void OnStageLeaveBattle(const MUID& uidChar, const MUID& uidStage);
	void OnPeerList(const MUID& uidStage, void* pBlob, int nCount);
	void OnAddPeer(const MUID& uidChar, DWORD dwIP, const int nPort =	
		MATCHCLIENT_DEFAULT_UDP_PORT, MTD_PeerListNode* pNode = NULL);
	void OnGameRoundState(const MUID& uidStage, int nRound, int nRoundState, int nArg);

	void OnGameResponseTimeSync(u64 nLocalTimeStamp, u64 nGlobalTimeSync);
	void OnEventUpdateJjang(const MUID& uidChar, bool bJjang);

	void OnPeerDead(const MUID& uidAttacker, const u32 nAttackerArg,
					const MUID& uidVictim, const u32 nVictimArg);
	void OnReceiveTeamBonus(const MUID& uidChar, const u32 nExpArg);
	void OnPeerDie(const MUID& uidVictim, const MUID& uidAttacker);
	void OnPeerDieMessage(ZCharacter* pVictim, ZCharacter* pAttacker);
	void OnChangeParts(const MUID& uid,int partstype,int PartsID);
	void OnDamage(const MUID& uid, const MUID& tuid,int damage);
	void OnPeerReload(const MUID& uid);
	void OnPeerSpMotion(const MUID& uid,int nMotionType);
	void OnPeerChangeCharacter(const MUID& uid);
	void OnPeerSpawn(const MUID& uid, const rvector& pos, const rvector& dir);

	void OnSetObserver(const MUID& uid);

	void OnPeerBasicInfo(MCommand *pCommand, bool bAddHistory = true, bool bUpdate = true);
	void OnPeerNewBasicInfo(MCommand *pCommand, bool bAddHistory = true, bool bUpdate = true);
	void OnPeerHPInfo(MCommand *pCommand);
	void OnPeerHPAPInfo(MCommand *pCommand);
	void OnPeerPing(MCommand *pCommand);
	void OnPeerPong(MCommand *pCommand);
	void OnPeerOpened(MCommand *pCommand);
	void OnPeerDash(MCommand* pCommand);
		
	bool FilterDelayedCommand(MCommand *pCommand);
	void ProcessDelayedCommand();

	void OnLocalOptainSpecialWorldItem(MCommand* pCommand);
	void OnResetTeamMembers(MCommand* pCommand);

	void AutoAiming();

private:
	friend class ZMyBotCharacter;

	bool OnRuleCommand(MCommand* pCommand);
	void PostNewBasicInfo();
	struct ShotInfo DoOneShot(ZObject* pOwner, v3 SrcPos, v3 DestPos, float ShotTime, ZItem* pItem,
		float KnockbackForceRatio = 1);

	ZGameAction GameAction;
	MDataChecker m_DataChecker;
	ZGameTimer m_GameTimer;
	float m_fTime;

	enum ZGAME_LASTTIME
	{
		ZLASTTIME_HPINFO = 0,
		ZLASTTIME_BASICINFO,
		ZLASTTIME_PEERPINGINFO,
		ZLASTTIME_SYNC_REPORT,
		ZLASTTIME_MAX
	};

	u64 m_nLastTime[ZLASTTIME_MAX];

	ZGAME_READYSTATE m_nReadyState;

	ZObserverCommandList::iterator ReplayIterator;
	float ReplayStartGameTime{};

	PingMap Pings;

	ZGameDraw DrawObj;

	BasicInfoNetState BasicInfoState;
	u32 LastSyncTime = 0;

	float LastHitSoundTime = 0;
};

extern ZGame* g_pGame;
extern MUID g_MyChrUID;
extern float g_fFarZ;
float GetFOV();
void SetFOV(float);

ZCharacterManager*	ZGetCharacterManager();
ZObjectManager*		ZGetObjectManager();
bool IsMyCharacter(ZObject* pObject);
bool GetUserGradeIDColor(MMatchUserGradeID gid, MCOLOR& UserNameColor, char* sp_name, size_t maxlen);
template <size_t size>
bool GetUserGradeIDColor(MMatchUserGradeID gid, MCOLOR& UserNameColor, char(&sp_name)[size]) {
	return GetUserGradeIDColor(gid, UserNameColor, sp_name, size);
}

inline float GetMeleeDistance(const v3& AttackerPos, const v3& TargetPos)
{
	auto SwordPos = AttackerPos + v3{ 0, 0, CHARACTER_HEIGHT * 0.5f };
	return GetDistanceLineSegment(SwordPos, TargetPos, TargetPos + v3{ 0, 0, CHARACTER_HEIGHT });
}

#define MAX_COMBO 99

#define PEERMOVE_TICK			100
#define PEERMOVE_AGENT_TICK		100