#include "stdafx.h"

#include "ZApplication.h"
#include "ZGameInterface.h"
#include "MCommandLogFrame.h"
#include "ZConsole.h"
#include "ZInterface.h"
#include "Config.h"
#include "MDebug.h"
#include "RMeshMgr.h"
#include "RShadermgr.h"
#include "ZConfiguration.h"
#include "MProfiler.h"
#include "MChattingFilter.h"
#include "ZInitialLoading.h"
#include "ZWorldItem.h"
#include "MMatchWorlditemdesc.h"
#include "ZSecurity.h"
#include "ZReplay.h"
#include "ZTestGame.h"
#include "ZGameClient.h"
#include "MRegistry.h"
#include "ZUtil.h"
#include "ZStringResManager.h"
#include "ZFile.h"
#include "ZActionKey.h"
#include "ZInput.h"
#include "ZOptionInterface.h"
#include "RGMain.h"
#include "MeshManager.h"
#include "RS2.h"

#ifdef _QEUST_ITEM_DEBUG
#include "MQuestItem.h"
#endif

#ifdef _ZPROFILER
#include "ZProfiler.h"
#endif

ZApplication*	ZApplication::m_pInstance;
MZFileSystem	ZApplication::m_FileSystem;    
ZSoundEngine	ZApplication::m_SoundEngine;
RMeshMgr		ZApplication::m_NPCMeshMgr;
RMeshMgr		ZApplication::m_MeshMgr;
RMeshMgr		ZApplication::m_WeaponMeshMgr;
RAniEventMgr    ZApplication::m_AniEventMgr;
ZTimer			ZApplication::m_Timer;
ZEmblemInterface	ZApplication::m_EmblemInterface;
ZSkillManager	ZApplication::m_SkillManager;

MCommandLogFrame* m_pLogFrame;

ZApplication::ZApplication()
	: Time(timeGetTime())
{
	_ASSERT(m_pInstance==NULL);

	m_nTimerRes = 0;
	m_pInstance = this;


	m_pGameInterface=NULL;

	m_nInitialState = GUNZ_LOGIN;

	m_bLaunchDevelop = false;
	m_bLaunchTest = false;

	SetLaunchMode(ZLAUNCH_MODE_DEBUG);

#ifdef _ZPROFILER
	m_pProfiler = std::make_unique<ZProfiler>();
#endif
}

ZApplication::~ZApplication()
{
	m_pInstance = NULL;
}

bool GetNextName(char *szBuffer, int nBufferCount, const char *szSource)
{
	while(*szSource==' ' || *szSource=='\t') szSource++;

	const char *end=NULL;
	if (szSource[0] == '"')
	{
		end = strchr(szSource + 1, '"');
		szSource++;
		end--;
	}
	else
	{
		end=strchr(szSource,' ');
		if(NULL==end) end=strchr(szSource,'\t');
	}

	if(end)
	{
		int nCount=end-szSource;
		if(nCount==0 || nCount>=nBufferCount) return false;

		strncpy_safe(szBuffer, nBufferCount, szSource, nCount);
		szBuffer[nCount]=0;
	}
	else
	{
		int nCount=(int)strlen(szSource);
		if(nCount==0 || nCount>=nBufferCount) return false;

		strcpy_safe(szBuffer, nBufferCount, szSource);
	}

	return true;
}

bool ZApplication::ParseArguments(const char* pszArgs)
{
	m_szCmdLine = pszArgs;

	if(pszArgs[0]=='"') 
	{
		m_szFileName = pszArgs + 1;
		if (m_szFileName[m_szFileName.length() - 1] == '"')
			m_szFileName.resize(m_szFileName.length() - 1);
	}
	else
	{
		m_szFileName = pszArgs;
	}

	auto&& ZTOKEN_ASSETSDIR = "assetsdir";

	auto* str = strstr(pszArgs, ZTOKEN_ASSETSDIR);
	char buffer[256];

	if (str && GetNextName(buffer, sizeof(buffer), str + strlen(ZTOKEN_ASSETSDIR)))
	{
		AssetsDir = buffer;
	}

	if(_stricmp(m_szFileName.c_str() + m_szFileName.length() - strlen(GUNZ_REC_FILE_EXT),
		GUNZ_REC_FILE_EXT) == 0){
		SetLaunchMode(ZLAUNCH_MODE_STANDALONE_REPLAY);
		m_nInitialState = GUNZ_GAME;
		return true;
	}

	// TODO: Figure out whatever this affects
	if ( pszArgs[0] == '/')
	{
#ifndef _PUBLISH
		if ( strstr( pszArgs, "launchdevelop") != NULL)
		{
			SetLaunchMode( ZLAUNCH_MODE_STANDALONE_DEVELOP);
			m_bLaunchDevelop = true;

			return true;
		} 
		else if ( strstr( pszArgs, "launch") != NULL)
		{
			SetLaunchMode(ZLAUNCH_MODE_STANDALONE);
			return true;
		}
#endif
	}

#ifndef _PUBLISH
	{
		SetLaunchMode(ZLAUNCH_MODE_STANDALONE_DEVELOP);
		m_bLaunchDevelop=true;
		return true;
	}
#endif

	SetLaunchMode(ZLAUNCH_MODE_STANDALONE);

	return true;
}

void ZApplication::CheckSound()
{
#ifdef _BIRDSOUND

#else
	int size = m_MeshMgr.m_id_last;
	int ani_size = 0;

	RMesh* pMesh = NULL;
	RAnimationMgr* pAniMgr = NULL;
	RAnimation* pAni = NULL;

	for(int i=0;i<size;i++) {
		pMesh = m_MeshMgr.GetFast(i);
		if(pMesh) {
			pAniMgr = &pMesh->m_ani_mgr;
			if(pAniMgr){
				ani_size = pAniMgr->m_id_last;
				for(int j=0;j<ani_size;j++) {
					pAni = pAniMgr->m_node_table[j];
					if(pAni) {

						if(m_SoundEngine.isPlayAbleMtrl(pAni->m_sound_name)==false) {
							MLog("ClearSoundFile %s %s\n", pAni->GetName(), pAni->m_sound_name);
							pAni->ClearSoundFile();
						}
						else {
							int ok = 0;
						}
					}
				}
			}
		}
	}
#endif
}

void RegisterForbidKey()
{
	ZActionKey::RegisterForbidKey(0x3b);// f1
	ZActionKey::RegisterForbidKey(0x3c);
	ZActionKey::RegisterForbidKey(0x3d);
	ZActionKey::RegisterForbidKey(0x3e);
	ZActionKey::RegisterForbidKey(0x3f);
	ZActionKey::RegisterForbidKey(0x40);
	ZActionKey::RegisterForbidKey(0x41);
	ZActionKey::RegisterForbidKey(0x42);// f8
}

void ZProgressCallBack(void *pUserParam,float fProgress)
{
	ZLoadingProgress *pLoadingProgress = (ZLoadingProgress*)pUserParam;
	pLoadingProgress->UpdateAndDraw(fProgress);
}

static void ListSoundDevices()
{
	for (int i = 0; i < ZGetSoundEngine()->GetEnumDeviceCount(); ++i)
		mlog("Sound Device %d = %s\n", i, ZGetSoundEngine()->GetDeviceDescription(i));
}

namespace RealSpace2
{
	extern bool DynamicResourceLoading;
}


#ifdef TIMESCALE
unsigned long long GetGlobalTimeMSOverride()
{
	if (!ZApplication::GetInstance())
		return timeGetTime();

	return ZApplication::GetInstance()->GetTime();
}
#endif

bool ZApplication::OnCreate(ZLoadingProgress *pLoadingProgress)
{
	MInitProfile();

#ifdef TIMESCALE
	GetGlobalTimeMS = GetGlobalTimeMSOverride;
#endif

	TIMECAPS tc;

	mlog("ZApplication::OnCreate : begin\n");

	ZLoadingProgress InitialLoading("Initializing", pLoadingProgress, 0.05f);
	InitialLoading.UpdateAndDraw(0);

	InitialLoading.UpdateAndDraw(0.5f);

	//ListSoundDevices();

	[&]
	{
		if (!IsDynamicResourceLoad())
			return;

		auto Fail = [&]()
		{
			MLog("Failed to load parts index! Turning off dynamic resource loading\n");
			RealSpace2::DynamicResourceLoading = false;
		};

		auto ret = ReadMZFile("system/parts_index.xml");
		if (!ret.first)
			return Fail();

		if (!GetMeshManager()->LoadParts(ret.second))
			return Fail();
	}();

	__BP(2000, "ZApplication::OnCreate");

	GetRGMain().OnAppCreate();

	InitialLoading.UpdateAndDraw(1);

#define MMTIMER_RESOLUTION	1
	if (TIMERR_NOERROR == timeGetDevCaps(&tc, sizeof(TIMECAPS)))
	{
		m_nTimerRes = min(max(tc.wPeriodMin, static_cast<UINT>(MMTIMER_RESOLUTION)), tc.wPeriodMax);
		timeBeginPeriod(m_nTimerRes);
	}

	if (ZApplication::GetInstance()->GetLaunchMode() == ZApplication::ZLAUNCH_MODE_NETMARBLE)
		m_nInitialState = GUNZ_NETMARBLELOGIN;

	DWORD _begin_time,_end_time;
#define BEGIN_ { _begin_time = GetGlobalTimeMS(); }
#define END_(x) { _end_time = GetGlobalTimeMS(); float f_time = (_end_time - _begin_time) / 1000.f; \
	mlog("\n-------------------> %s : %f \n\n", x,f_time ); }

	__BP(2001,"m_SoundEngine.Create");

	ZLoadingProgress soundLoading("Sound", pLoadingProgress, 0.12f);
	BEGIN_;
#ifdef _BIRDSOUND
	m_SoundEngine.Create(RealSpace2::g_hWnd, 44100, Z_AUDIO_HWMIXING, GetFileSystem());
#else
	m_SoundEngine.Create(RealSpace2::g_hWnd, Z_AUDIO_HWMIXING, &soundLoading );
#endif
	END_("Sound Engine Create");
	soundLoading.UpdateAndDraw(1.f);

	__EP(2001);

	RegisterForbidKey();

	__BP(2002,"m_pInterface->OnCreate()");

	ZLoadingProgress giLoading("GameInterface",pLoadingProgress,.35f);

	BEGIN_;
	m_pGameInterface = new ZGameInterface("GameInterface",
		Mint::GetInstance()->GetMainFrame(), Mint::GetInstance()->GetMainFrame());
	m_pGameInterface->m_nInitialState = m_nInitialState;
	if(!m_pGameInterface->OnCreate(&giLoading))
	{
		mlog("Failed: ZGameInterface OnCreate\n");
		SAFE_DELETE(m_pGameInterface);
		return false;
	}

	m_pGameInterface->SetBounds(0,0,MGetWorkspaceWidth(),MGetWorkspaceHeight());
	END_("GameInterface Create");
	mlog("ZApplication::OnCreate : GameInterface Created\n");

	giLoading.UpdateAndDraw(1.f);

	mlog("ZApplication::OnCreate : m_pInterface->OnCreate() \n");

	__EP(2002);

#ifdef _BIRDTEST
	goto BirdGo;
#endif

	__BP(2003,"Character Loading");

	ZLoadingProgress meshLoading("Mesh",pLoadingProgress,.41f);
	BEGIN_;

	if(m_MeshMgr.LoadXmlList("model/character.xml",ZProgressCallBack,&meshLoading)==-1)
		return false;

	SetAnimationMgr(MMS_MALE, &m_MeshMgr.Get("heroman1")->m_ani_mgr);
	SetAnimationMgr(MMS_FEMALE, &m_MeshMgr.Get("herowoman1")->m_ani_mgr);

	mlog("ZApplication::OnCreate : m_MeshMgr.LoadXmlList(character.xml) \n");

	END_("Character Loading");
	meshLoading.UpdateAndDraw(1.f);

	__EP(2003);

	__BP(1985, "Quest NPC loading");
#ifdef _QUEST
	if(m_NPCMeshMgr.LoadXmlList("model/npc.xml") == -1)
		return false;
#endif
	__EP(1985);

	// This is really slow and doesn't seem to do anything of value
	/*__BP(1986, "CheckSound");
	CheckSound();
	__EP(1986);*/

	__BP(2004,"WeaponMesh Loading");

	BEGIN_;

	if(m_WeaponMeshMgr.LoadXmlList("model/weapon.xml") == -1)
		return false;

	END_("WeaponMesh Loading");

	__EP(2004);

	__BP(2005,"Worlditem Loading");

	ZLoadingProgress etcLoading("etc", pLoadingProgress, .02f);
	BEGIN_;

#ifdef	_WORLD_ITEM_
	m_MeshMgr.LoadXmlList("system/worlditem.xml");
#endif

	mlog("ZApplication::OnCreate : m_WeaponMeshMgr.LoadXmlList(weapon.xml) \n");

	END_("Worlditem Loading");
	__EP(2005);

#ifdef _BIRDTEST
BirdGo:
#endif

	__BP(2006,"ETC .. XML");

	BEGIN_;
	CreateConsole(ZGetGameClient()->GetCommandManager());

	mlog("ZApplication::OnCreate : CreateConsole \n");

	m_pLogFrame = new MCommandLogFrame("Command Log", Mint::GetInstance()->GetMainFrame(),
		Mint::GetInstance()->GetMainFrame());
	int nHeight = MGetWorkspaceHeight()/3;
	m_pLogFrame->SetBounds(0, MGetWorkspaceHeight()-nHeight-1, MGetWorkspaceWidth()-1, nHeight);
	m_pLogFrame->Show(false);

	m_pGameInterface->SetFocusEnable(true);
	m_pGameInterface->SetFocus();
	m_pGameInterface->Show(true);

	if (!MGetMatchItemDescMgr()->ReadXml(GetFileSystem(), FILENAME_ZITEM_DESC))
	{
		MLog("Error while Read Item Descriptor %s\n", FILENAME_ZITEM_DESC);
	}
	mlog("ZApplication::OnCreate : MGetMatchItemDescMgr()->ReadXml \n");

	if (!MGetMatchItemEffectDescMgr()->ReadXml(GetFileSystem(), FILENAME_ZITEMEFFECT_DESC))
	{
		MLog("Error while Read Item Descriptor %s\n", FILENAME_ZITEMEFFECT_DESC);
	}
	mlog("ZApplication::OnCreate : MGetMatchItemEffectDescMgr()->ReadXml \n");

	if (!MGetMatchWorldItemDescMgr()->ReadXml(GetFileSystem(), "system/worlditem.xml"))
	{
		MLog("Error while Read Item Descriptor %s\n", "system/worlditem.xml");
	}
	mlog("ZApplication::OnCreate : MGetMatchWorldItemDescMgr()->ReadXml \n");

	if (!ZGetChannelRuleMgr()->ReadXml(GetFileSystem(), "system/channelrule.xml"))
	{
		MLog("Error while Read Item Descriptor %s\n", "system/channelrule.xml");
	}
	mlog("ZApplication::OnCreate : ZGetChannelRuleMgr()->ReadXml \n");

	if (!MGetChattingFilter()->LoadFromFile(GetFileSystem(), "system/abuse.txt"))
	{
		MLog("Error while Read Abuse Filter %s\n", "system/abuse.xml");
	}

#ifdef _QUEST_ITEM
	if( !GetQuestItemDescMgr().ReadXml(GetFileSystem(), FILENAME_QUESTITEM_DESC) )
	{
		MLog( "Error while read quest item descrition xml file.\n" );
	}
#endif

	mlog("ZApplication::OnCreate : MGetChattingFilter()->Create \n");

	if(!m_SkillManager.Create()) {
		MLog("Error while create skill manager\n");
	}

	END_("ETC ..");

#ifndef _BIRDTEST
	etcLoading.UpdateAndDraw(1.f);
#endif

	ZGetEmblemInterface()->Create();

	__EP(2006);

	__EP(2000);

	__SAVEPROFILE("profile_loading.txt");

	ZSetupDataChecker_Global(&m_GlobalDataChecker);

	return true;

#undef BEGIN_
#undef END_
}

void ZApplication::OnDestroy()
{
	m_WorldManager.Destroy();
	ZGetEmblemInterface()->Destroy();

	MGetMatchWorldItemDescMgr()->Clear();

	m_SoundEngine.Destroy();
	DestroyConsole();

	mlog("ZApplication::OnDestroy : DestroyConsole() \n");

	SAFE_DELETE(m_pLogFrame);
	SAFE_DELETE(m_pGameInterface);

	m_NPCMeshMgr.DelAll();

	m_MeshMgr.DelAll();
	mlog("ZApplication::OnDestroy : m_MeshMgr.DelAll() \n");

	m_WeaponMeshMgr.DelAll();
	mlog("ZApplication::OnDestroy : m_WeaponMeshMgr.DelAll() \n");

	if (m_nTimerRes != 0)
	{
		timeEndPeriod(m_nTimerRes);
		m_nTimerRes = 0;
	}

	RGetParticleSystem()->Destroy();		

	mlog("ZApplication::OnDestroy done \n");
}

void ZApplication::ResetTimer()
{
	m_Timer.ResetFrame();
}

static void UpdateVulkan(float ElapsedTime)
{
	float RotX, RotY;
	ZGetInput()->GetRotation(&RotX, &RotY);
	static float AngleX, AngleZ;

	AngleX += RotY;
	AngleZ += RotX;

	AngleZ = fmod(AngleZ, 2 * PI);

	AngleX = max(CAMERA_ANGLEX_MIN, AngleX);
	AngleX = min(CAMERA_ANGLEX_MAX, AngleX);

	RCameraDirection = {
		cosf(AngleZ) * sinf(AngleX),
		sinf(AngleZ) * sinf(AngleX),
		cosf(AngleX) };

	auto Forward = RCameraDirection;
	v3 Up{ 0, 0, -1 };
	auto Right = Normalized(CrossProduct(Forward, Up));

	v3 Direction{ 0, 0, 0 };

	auto GetKey = [](auto Key) {
		return (GetAsyncKeyState(Key) & 0x8000) != 0;
	};

	if (GetKey('W'))
		Direction += Forward;
	if (GetKey('A'))
		Direction += -Right;
	if (GetKey('S'))
		Direction += -Forward;
	if (GetKey('D'))
		Direction += Right;

	Normalize(Direction);

	auto Speed = 1000 * ElapsedTime;

	RCameraPosition += Direction * Speed;
}

void ZApplication::OnUpdate()
{
	auto prof = MBeginProfile("ZApplication::OnUpdate");

	[&]
	{
		static u64 LastRealTime = timeGetTime();
		auto CurRealTime = timeGetTime();
		if (Timescale == 1.f)
		{
			Time += CurRealTime - LastRealTime;
		}
		else
		{
			auto Delta = double(CurRealTime - LastRealTime);
			Time += Delta * Timescale;
		}
		LastRealTime = CurRealTime;
	}();

	auto ElapsedTime = m_Timer.UpdateFrame();

	if (Timescale != 1.f)
		ElapsedTime *= Timescale;

	if (GetRS2().UsingVulkan())
	{
		UpdateVulkan(ElapsedTime);
		return;
	}

	GetRGMain().OnUpdate(ElapsedTime);

	__BP(1,"ZApplication::OnUpdate::m_pInterface->Update");
	if (m_pGameInterface) m_pGameInterface->Update(ElapsedTime);
	__EP(1);

	__BP(2,"ZApplication::OnUpdate::SoundEngineRun");

#ifdef _BIRDSOUND
	m_SoundEngine.Update();
#else
	m_SoundEngine.Run();
#endif

	__EP(2);

	if (ZWasActionKeyPressed(ZACTION_SCREENSHOT)) {
		if (m_pGameInterface)
		{
			m_pGameInterface->OnScreenshot();
		}
	}
}

bool g_bProfile=false;

#define PROFILE_FILENAME	"profile.txt"

bool ZApplication::OnDraw()
{
	static bool currentprofile = false;
	if(g_bProfile && !currentprofile)
	{
		currentprofile = true;
        MInitProfile();
	}

	if (!g_bProfile && currentprofile)
	{
		currentprofile = false;
		MSaveProfile(PROFILE_FILENAME);
	}

	__BP(3, "ZApplication::Draw");

	__BP(4, "ZApplication::Draw::Mint::Run");
	if (ZGetGameInterface()->GetState() != GUNZ_GAME)
	{
		Mint::GetInstance()->Run();
	}
	__EP(4);

	__BP(5, "ZApplication::Draw::Mint::Draw");

	Mint::GetInstance()->Draw();

	__EP(5);

	__EP(3);

#ifdef _ZPROFILER
	// profiler
	m_pProfiler->Update();
	m_pProfiler->Render();
#endif

	return m_pGameInterface->IsDone();
}

ZApplication* ZApplication::GetInstance(void)
{
	return m_pInstance;
}
ZGameInterface* ZApplication::GetGameInterface(void)
{
	ZApplication* pApp = GetInstance();
	if(pApp==NULL) return NULL;
	return pApp->m_pGameInterface;
}
ZStageInterface* ZApplication::GetStageInterface(void)
{
	ZApplication* pApp = GetInstance();
	if(pApp==NULL) return NULL;
	return &pApp->m_StageInterface;
}
ZOptionInterface* ZApplication::GetOptionInterface(void)
{
	ZApplication* pApp = GetInstance();
	if(pApp==NULL) return NULL;
	return &pApp->m_OptionInterface;
}
MZFileSystem* ZApplication::GetFileSystem(void)
{
	return &m_FileSystem;
}

ZGameClient* ZApplication::GetGameClient(void)
{
	return (GetGameInterface()->GetGameClient());
}

ZGame* ZApplication::GetGame(void)
{
	return (GetGameInterface()->GetGame());
}

ZTimer* ZApplication::GetTimer(void)
{
	return &m_Timer;
}

ZSoundEngine* ZApplication::GetSoundEngine(void)
{
	return &m_SoundEngine;
}

void ZApplication::OnInvalidate()
{
	RGetShaderMgr()->OnInvalidate();
	if(m_pGameInterface)
		m_pGameInterface->OnInvalidate();

	if (IsRGMainAlive())
		GetRGMain().OnInvalidate();
}

void ZApplication::OnRestore()
{
	if(m_pGameInterface)
		m_pGameInterface->OnRestore();
	if( ZGetConfiguration()->GetVideo()->bShader )
	{
		RMesh::mHardwareAccellated = true;
		RGetShaderMgr()->SetEnable();
	}

	if (IsRGMainAlive())
		GetRGMain().OnRestore();
}

void ZApplication::Exit()
{
	PostMessage(g_hWnd,WM_CLOSE,0,0);
}

#define ZTOKEN_GAME				"game"
#define ZTOKEN_REPLAY			"replay"
#define ZTOKEN_GAME_CHARDUMMY	"dummy"
#define ZTOKEN_GAME_AI			"ai"
#define ZTOKEN_QUEST			"quest"
#define ZTOKEN_FAST_LOADING		"fast"

void ZApplication::PreCheckArguments()
{
	if(strstr(m_szCmdLine.c_str(), ZTOKEN_FAST_LOADING)) {
		RMesh::SetPartsMeshLoadingSkip(1);
	}
}

void ZApplication::ParseStandAloneArguments(const char* pszArgs)
{
	char buffer[256];

	const char *str;
	str=strstr(pszArgs, ZTOKEN_GAME);
	if ( str != NULL) {
		ZApplication::GetInstance()->m_nInitialState = GUNZ_GAME;
		if(GetNextName(buffer,sizeof(buffer),str+strlen(ZTOKEN_GAME)))
		{
			m_szFileName = buffer;
			CreateTestGame(buffer);
			SetLaunchMode(ZLAUNCH_MODE_STANDALONE_GAME);
			return;
		}
	}

	str=strstr(pszArgs, ZTOKEN_GAME_CHARDUMMY);
	if ( str != NULL) {
		ZApplication::GetInstance()->m_nInitialState = GUNZ_GAME;
		char szTemp[256], szMap[256];
		int nDummyCount = 0, nShotEnable = 0;

		sscanf_s(str, "%s %s %d %d",
			szTemp, sizeof(szTemp),
			szMap, sizeof(szMap),
			&nDummyCount, &nShotEnable);

		bool bShotEnable = false;
		if (nShotEnable != 0) bShotEnable = true;

		SetLaunchMode(ZLAUNCH_MODE_STANDALONE_GAME);
		CreateTestGame(szMap, nDummyCount, bShotEnable);
		return;
	}

	str=strstr(pszArgs, ZTOKEN_QUEST);
	if ( str != NULL) {
		SetLaunchMode(ZLAUNCH_MODE_STANDALONE_QUEST);
		return;
	}

#ifndef _PUBLISH

	#ifdef _QUEST
		str=strstr(pszArgs, ZTOKEN_GAME_AI);
		if ( str != NULL) {
			SetLaunchMode(ZLAUNCH_MODE_STANDALONE_AI);

			ZApplication::GetInstance()->m_nInitialState = GUNZ_GAME;
			char szTemp[256], szMap[256];
			sscanf_s(str, "%s %s", szTemp, sizeof(szTemp), szMap, sizeof(szMap));

			ZGetGameClient()->GetMatchStageSetting()->SetGameType(MMATCH_GAMETYPE_QUEST);
			
			CreateTestGame(szMap, 0, false, true, 0);
			return;
		}
	#endif

#endif

}

void ZApplication::SetInitialState()
{
	if(GetLaunchMode()==ZLAUNCH_MODE_STANDALONE_REPLAY) {
		CreateReplayGame(m_szFileName.c_str());
		return;
	}

	ParseStandAloneArguments(m_szCmdLine.c_str());

	ZGetGameInterface()->SetState(m_nInitialState);
}


bool ZApplication::InitLocale()
{
	ZStringResManager::MakeInstance();
	ZGetStringResManager()->Init("system/", 0, GetFileSystem());

	return true;
}

bool ZApplication::GetSystemValue(const char* szField, char* szData, int maxlen)
{
	return MRegistry::Read(HKEY_CURRENT_USER, szField, szData, maxlen);
}

void ZApplication::SetSystemValue(const char* szField, const char* szData)
{
	MRegistry::Write(HKEY_CURRENT_USER, szField, szData);
}

void ZApplication::InitFileSystem()
{
	m_FileSystem.Create(AssetsDir);
	RSetFileSystem(ZApplication::GetFileSystem());
}