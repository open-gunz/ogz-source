#pragma once

#include "MZFileSystem.h"
#include "ZSoundEngine.h"
#include "ZDirectInput.h"
#include "MDataChecker.h"
#include "ZTimer.h"
#include "ZEmblemInterface.h"
#include "ZStageInterface.h"
#include "ZSkill.h"
#include "ZWorldManager.h"
#include "ZStageInterface.h"
#include "ZOptionInterface.h"
#include "RAniEventInfo.h"
#include <string>
#include <memory>

class ZGame;
class ZGameClient;
class ZGameInterface;
class ZLoadingProgress;
class ZProfiler;
class ZOptionInterface;

enum GunzState {
	GUNZ_NA = 0,
	GUNZ_GAME = 1,
	GUNZ_LOGIN = 2,
	GUNZ_NETMARBLELOGIN = 3,
	GUNZ_LOBBY = 4,
	GUNZ_STAGE = 5,
	GUNZ_GREETER = 6,
	GUNZ_CHARSELECTION = 7,
	GUNZ_CHARCREATION = 8,
	GUNZ_PREVIOUS = 10,
	GUNZ_SHUTDOWN = 11,
	GUNZ_BIRDTEST
};

class ZApplication 
{
public:
	enum ZLAUNCH_MODE {
		ZLAUNCH_MODE_DEBUG,
		ZLAUNCH_MODE_NETMARBLE,
		ZLAUNCH_MODE_STANDALONE,
		ZLAUNCH_MODE_STANDALONE_DEVELOP,
		ZLAUNCH_MODE_STANDALONE_REPLAY,
		ZLAUNCH_MODE_STANDALONE_GAME,
		ZLAUNCH_MODE_STANDALONE_DUMMY,
		ZLAUNCH_MODE_STANDALONE_AI,
		ZLAUNCH_MODE_STANDALONE_QUEST,
	};

private:
	ZGameInterface*			m_pGameInterface;
	GunzState				m_nInitialState;
	ZStageInterface			m_StageInterface;
	ZOptionInterface		m_OptionInterface;
	ZLAUNCH_MODE			m_nLaunchMode;
    std::string				m_szFileName;
	std::string				m_szCmdLine;
	u32						m_nTimerRes;
	MDataChecker			m_GlobalDataChecker;
	bool					m_bLaunchDevelop;
	bool					m_bLaunchTest;
	u64 Time{};
	float Timescale = 1.f;
	std::string AssetsDir = "./";

	void ParseStandAloneArguments(const char* pszArgs);
protected:
	static ZApplication*	m_pInstance;
	static MZFileSystem		m_FileSystem;
	static ZSoundEngine		m_SoundEngine;
	static RMeshMgr			m_NPCMeshMgr;
	static RMeshMgr			m_MeshMgr;
	static RMeshMgr			m_WeaponMeshMgr;
	static ZTimer			m_Timer;
	static ZEmblemInterface	m_EmblemInterface;
	static ZSkillManager	m_SkillManager;
	ZWorldManager			m_WorldManager;
#ifdef _ZPROFILER
	std::unique_ptr<ZProfiler> m_pProfiler;
#endif
	static RAniEventMgr		m_AniEventMgr;

public:
	ZApplication();
	~ZApplication();

	bool ParseArguments(const char* pszArgs);
	void SetInterface();
	void SetNextInterface();
	void CheckSound();
	void SetInitialState();
	template<size_t size>
	bool GetSystemValue(const char* szField, char(&szData)[size]) {
		return GetSystemValue(szField, szData, size);
	}
	bool GetSystemValue(const char* szField, char* szData, int maxlen);
	void SetSystemValue(const char* szField, const char* szData);

	void InitFileSystem();

	bool OnCreate(ZLoadingProgress *pLoadingProgress);
	void OnDestroy();
	bool OnDraw();
	void OnUpdate();
	void OnInvalidate();
	void OnRestore();

	static void ResetTimer();
	static void Exit();
	static ZApplication*		GetInstance();
	static ZGameInterface*		GetGameInterface();
	static ZStageInterface*		GetStageInterface();
	static ZOptionInterface*	GetOptionInterface();
	static MZFileSystem*		GetFileSystem();
	static ZGameClient*			GetGameClient();
	static ZGame*				GetGame();
	static ZTimer*				GetTimer();
	static ZSoundEngine*		GetSoundEngine();
	static RMeshMgr*			GetNpcMeshMgr()			{ return &m_NPCMeshMgr;}
	static RMeshMgr*			GetMeshMgr()			{ return &m_MeshMgr; }
	static RMeshMgr*			GetWeaponMeshMgr()		{ return &m_WeaponMeshMgr; }
	static ZEmblemInterface*	GetEmblemInterface()	{ return &m_EmblemInterface; }
	static ZSkillManager*		GetSkillManager()		{ return &m_SkillManager; }
	static RAniEventMgr*		GetAniEventMgr()		{ return &m_AniEventMgr;}
	ZWorldManager*				GetWorldManager()		{ return &m_WorldManager; }

	ZLAUNCH_MODE GetLaunchMode() const		{ return m_nLaunchMode; }
	void SetLaunchMode(ZLAUNCH_MODE nMode)  { m_nLaunchMode = nMode; }
	bool IsLaunchDevelop() const			{ return m_bLaunchDevelop; }
	bool IsLaunchTest() const				{ return m_bLaunchTest; }

	bool InitLocale();
	void PreCheckArguments();

	auto GetTime() const { return Time; }
	auto GetTimescale() const { return Timescale; }
	void SetTimescale(float f) { Timescale = f; }

	bool IsDeveloperMode() const { return GetLaunchMode() == ZApplication::ZLAUNCH_MODE_STANDALONE_GAME; }
};