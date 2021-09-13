#include "stdafx.h"
#include <tuple>
#include "MMatchServer.h"
#include "MSharedCommandTable.h"
#include "MErrorTable.h"
#include "MBlobArray.h"
#include "MObject.h"
#include "MMatchObject.h"
#include "MMatchItem.h"
#include "MMatchNotify.h"
#include "MMatchObjCache.h"
#include "MMatchStage.h"
#include "MMatchTransDataType.h"
#include "MMatchFormula.h"
#include "MMatchConfig.h"
#include "MCommandCommunicator.h"
#include "MMatchShop.h"
#include "MDebug.h"
#include "MMatchAuth.h"
#include "MMatchStatus.h"
#include "MAsyncDBJob.h"
#include "MAsyncDBJob_FriendList.h"
#include "MAsyncDBJob_UpdateCharInfoData.h"
#include "MAsyncDBJob_GetLoginInfo.h"
#include "MMatchWorldItemDesc.h"
#include "MMatchQuestMonsterGroup.h"
#include "RTypes.h"
#include "MMatchChatRoom.h"
#include "MMatchUtil.h"
#include "MLadderStatistics.h"
#include "MMatchSchedule.h"
#include "MMatchGameType.h"
#include "MQuestFormula.h"
#include "MQuestItem.h"
#include "MSacrificeQItemTable.h"
#include "MMatchPremiumIPCache.h"
#include "MCommandBuilder.h"
#include "MMatchLocale.h"
#include "MMatchEvent.h"
#include "MMatchEventManager.h"
#include "MMatchEventFactory.h"
#include "HitRegistration.h"
#include "MUtil.h"
#include "MLadderMgr.h"
#include "MTeamGameStrategy.h"
#include "stuff.h"
#include "MPickInfo.h"
#include "reinterpret.h"
#include "GunGame.h"
#include <regex>

#define DEFAULT_REQUEST_UID_SIZE		4200000000
#define DEFAULT_REQUEST_UID_SPARE_SIZE	10000
#define DEFAULT_ASYNCPROXY_THREADPOOL	6
#define MAXUSER_WEIGHT					30

#define MAX_DB_QUERY_COUNT_OUT			5

#define MATCHSERVER_DEFAULT_UDP_PORT	7777

#define FILENAME_ITEM_DESC				"zitem.xml"
#define FILENAME_SHOP					"shop.xml"
#define FILENAME_CHANNEL				"channel.xml"
#define FILENAME_SHUTDOWN_NOTIFY		"shutdown.xml"
#define FILENAME_WORLDITEM_DESC			"worlditem.xml"
#define FILENAME_MONSTERGROUP_DESC		"monstergroup.xml"
#define FILENAME_CHANNELRULE			"channelrule.xml"

MMatchServer* MMatchServer::m_pInstance = NULL;

static void RcpLog(const char *pFormat, ...)
{
	char szBuf[256];

	va_list args;

	va_start(args, pFormat);
	vsprintf_safe(szBuf, pFormat, args);
	va_end(args);

	int nEnd = (int)strlen(szBuf) - 1;
	if ((nEnd >= 0) && (szBuf[nEnd] == '\n')) {
		szBuf[nEnd] = 0;
		strcat_safe(szBuf, "\n");
	}
	DMLog(szBuf);
}

class MPointerChecker
{
private:
	void* m_pPointer;
	bool	m_bPrinted;
public:
	MPointerChecker() : m_bPrinted(false), m_pPointer(0) {  }
	void Init(void* pPointer) { m_pPointer = pPointer; }
	void Check(void* pPointer, int nState, int nValue)
	{
		if ((pPointer != m_pPointer) && (!m_bPrinted))
		{
			m_bPrinted = true;
			mlog("### Invalid Pointer(%x, %x) - State(%d) , Value(%d) ###\n", m_pPointer, pPointer, nState, nValue);
		}
	}
};

#define NUM_CHECKPOINTER	3
static MPointerChecker g_PointerChecker[NUM_CHECKPOINTER];


void _CheckValidPointer(void* pPointer1, void* pPointer2, void* pPointer3, int nState, int nValue)
{
	if (pPointer1 != NULL) g_PointerChecker[0].Check(pPointer1, nState, nValue);
	if (pPointer2 != NULL) g_PointerChecker[1].Check(pPointer2, nState, nValue);
	if (pPointer3 != NULL) g_PointerChecker[2].Check(pPointer3, nState, nValue);
}

void CopyCharInfoForTrans(MTD_CharInfo* pDest, MMatchCharInfo* pSrcCharInfo, MMatchObject* pSrcObject)
{
	memset(pDest, 0, sizeof(MTD_CharInfo));

	if (pSrcCharInfo)
	{
		strcpy_safe(pDest->szName, pSrcCharInfo->m_szName);
		strcpy_safe(pDest->szClanName, pSrcCharInfo->m_ClanInfo.m_szClanName);
		pDest->nClanGrade = pSrcCharInfo->m_ClanInfo.m_nGrade;
		pDest->nClanContPoint = pSrcCharInfo->m_ClanInfo.m_nContPoint;

		pDest->nCharNum = (char)pSrcCharInfo->m_nCharNum;
		pDest->nLevel = (unsigned short)pSrcCharInfo->m_nLevel;
		pDest->nSex = (char)pSrcCharInfo->m_nSex;
		pDest->nFace = (char)pSrcCharInfo->m_nFace;
		pDest->nHair = (char)pSrcCharInfo->m_nHair;

		pDest->nXP = pSrcCharInfo->m_nXP;
		pDest->nBP = pSrcCharInfo->m_nBP;
		pDest->fBonusRate = pSrcCharInfo->m_fBonusRate;
		pDest->nPrize = (unsigned short)pSrcCharInfo->m_nPrize;
		pDest->nHP = (unsigned short)pSrcCharInfo->m_nHP;
		pDest->nAP = (unsigned short)pSrcCharInfo->m_nAP;
		pDest->nMaxWeight = (unsigned short)pSrcCharInfo->m_nMaxWeight;
		pDest->nSafeFalls = (unsigned short)pSrcCharInfo->m_nSafeFalls;
		pDest->nFR = (unsigned short)pSrcCharInfo->m_nFR;
		pDest->nCR = (unsigned short)pSrcCharInfo->m_nCR;
		pDest->nER = (unsigned short)pSrcCharInfo->m_nER;
		pDest->nWR = (unsigned short)pSrcCharInfo->m_nWR;

		for (int i = 0; i < MMCIP_END; i++)
		{
			if (pSrcCharInfo->m_EquipedItem.IsEmpty(MMatchCharItemParts(i)))
			{
				pDest->nEquipedItemDesc[i] = 0;
			}
			else
			{
				MMatchItem* pItem = pSrcCharInfo->m_EquipedItem.GetItem(MMatchCharItemParts(i));
				MMatchItemDesc* pItemDesc = pItem->GetDesc();
				if (pItemDesc)
				{
					pDest->nEquipedItemDesc[i] = pItemDesc->m_nID;
				}
			}

		}
	}


	if (pSrcObject)
	{
		pDest->nUGradeID = pSrcObject->GetAccountInfo()->m_nUGrade;
	}
	else
	{
		pDest->nUGradeID = MMUG_FREE;
	}

	pDest->nClanCLID = pSrcCharInfo->m_ClanInfo.m_nClanID;
}

void CopyCharInfoDetailForTrans(MTD_CharInfo_Detail* pDest, MMatchCharInfo* pSrcCharInfo, MMatchObject* pSrcObject)
{
	memset(pDest, 0, sizeof(MTD_CharInfo_Detail));

	if (pSrcCharInfo)
	{
		strcpy_safe(pDest->szName, pSrcCharInfo->m_szName);
		strcpy_safe(pDest->szClanName, pSrcCharInfo->m_ClanInfo.m_szClanName);
		pDest->nClanGrade = pSrcCharInfo->m_ClanInfo.m_nGrade;
		pDest->nClanContPoint = pSrcCharInfo->m_ClanInfo.m_nContPoint;

		pDest->nLevel = (unsigned short)pSrcCharInfo->m_nLevel;
		pDest->nSex = (char)pSrcCharInfo->m_nSex;
		pDest->nFace = (char)pSrcCharInfo->m_nFace;
		pDest->nHair = (char)pSrcCharInfo->m_nHair;
		pDest->nXP = pSrcCharInfo->m_nXP;
		pDest->nBP = pSrcCharInfo->m_nBP;

		pDest->nKillCount = pSrcCharInfo->m_nTotalKillCount;
		pDest->nDeathCount = pSrcCharInfo->m_nTotalDeathCount;


		unsigned long int nNowTime = MMatchServer::GetInstance()->GetTickTime();

		pDest->nConnPlayTimeSec = MGetTimeDistance(pSrcCharInfo->m_nConnTime, nNowTime) / 1000;
		pDest->nTotalPlayTimeSec = pDest->nConnPlayTimeSec + pSrcCharInfo->m_nTotalPlayTimeSec;

		for (int i = 0; i < MMCIP_END; i++)
		{
			if (pSrcCharInfo->m_EquipedItem.IsEmpty(MMatchCharItemParts(i)))
			{
				pDest->nEquipedItemDesc[i] = 0;
			}
			else
			{
				pDest->nEquipedItemDesc[i] = pSrcCharInfo->m_EquipedItem.GetItem(MMatchCharItemParts(i))->GetDesc()->m_nID;
			}
		}
	}


	if (pSrcObject)
	{
		pDest->nUGradeID = pSrcObject->GetAccountInfo()->m_nUGrade;
	}
	else
	{
		pDest->nUGradeID = MMUG_FREE;
	}

	pDest->nClanCLID = pSrcCharInfo->m_ClanInfo.m_nClanID;
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool IsExpiredBlockEndTime(const tm& st)
{
	auto tie_tm = [](const tm& t) { return std::tie(t.tm_year, t.tm_mon, t.tm_yday, t.tm_hour); };
	return tie_tm(st) <= tie_tm(*localtime(&unmove(time(0))));
}

IDatabase* MakeDatabaseFromConfig()
{
	switch (MGetServerConfig()->GetDatabaseType())
	{
	case DatabaseType::SQLite:
		return new SQLiteDatabase;
	}
	MLog("Invalid db config\n");
	return nullptr;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////

MMatchServer::MMatchServer() : m_pScheduler(0)
{
	_ASSERT(m_pInstance == NULL);
	m_pInstance = this;
	m_nTickTime = 0;

	m_This = MATCHSERVER_UID;

	SetName("MATCHSERVER");	// For Debug
	SetDefaultChannelName("PUBLIC-");

	m_bCreated = false;

	MMatchStringResManager::MakeInstance();
}

MMatchServer::~MMatchServer()
{
	Destroy();
}

bool MMatchServer::LoadInitFile()
{
	if (!MGetServerConfig()->Create())
	{
		LOG(LOG_ALL, "Load Config File Failed");
		return false;
	}

	if (!InitLocale()) {
		LOG(LOG_ALL, "Locale 설정 실패.");
		return false;
	}

	if (MGetServerConfig()->IsResMap())
	{
		char szText[512];
		sprintf_safe(szText, "Enable Maps: ");
		for (int i = 0; i < MMATCH_MAP_MAX; i++)
		{
			if (MGetServerConfig()->IsEnableMap(MMATCH_MAP(i)))
			{
				strcat_safe(szText, g_MapDesc[i].szMapName);
				strcat_safe(szText, ", ");
			}
		}
		LOG(LOG_ALL, szText);
	}

	if (!MMatchFormula::Create())
	{
		LOG(LOG_ALL, "Open Formula Table FAILED");
		return false;
	}
	if (!MQuestFormula::Create())
	{
		LOG(LOG_ALL, "Open Quest Formula Table FAILED");
		return false;
	}

	if (!MGetMatchWorldItemDescMgr()->ReadXml(FILENAME_WORLDITEM_DESC))
	{
		Log(LOG_ALL, "Read World Item Desc Failed");
		return false;
	}

#ifdef _QUEST_ITEM
	if (!GetQuestItemDescMgr().ReadXml(QUEST_ITEM_FILE_NAME))
	{
		Log(LOG_ALL, "Load quest item xml file failed.");
		return false;
	}
	if (!MSacrificeQItemTable::GetInst().ReadXML(SACRIFICE_TABLE_XML))
	{
		Log(LOG_ALL, "Load sacrifice quest item table failed.");
		return false;
	}
#endif
	if ( (MGetServerConfig()->GetServerMode() == MSM_CLAN) || (MGetServerConfig()->GetServerMode() == MSM_TEST))
	{
		GetLadderMgr()->Init();
	}

	if (!MGetMapsWorldItemSpawnInfo()->Read())
	{
		Log(LOG_ALL, "Read World Item Spawn Failed");
		return false;
	}

	if (!MGetMatchItemDescMgr()->ReadXml(FILENAME_ITEM_DESC))
	{
		Log(LOG_ALL, "Read Item Descriptor Failed");
		return false;
	}

	if (!GetQuest()->Create())
	{
		Log(LOG_ALL, "Read Quest Desc Failed");
		return false;
	}

	if (!MGetMatchShop()->Create(FILENAME_SHOP))
	{
		Log(LOG_ALL, "Read Shop Item Failed");
		return false;
	}
	if (!LoadChannelPreset())
	{
		Log(LOG_ALL, "Load Channel preset Failed");
		return false;
	}
	if (!m_MatchShutdown.LoadXML_ShutdownNotify(FILENAME_SHUTDOWN_NOTIFY))
	{
		Log(LOG_ALL, "Load Shutdown Notify Failed");
		return false;
	}
	if (!MGetChannelRuleMgr()->ReadXml(FILENAME_CHANNELRULE))
	{
		Log(LOG_ALL, "Load ChannelRule.xml Failed");
		return false;
	}

	if (!MGetGunGame()->ReadXML("gungame.xml"))
	{
		Log(LOG_ALL, "Load GunGame.xml Failed.\n");
		return false;
	}

	u32 nItemChecksum = MGetMZFileChecksum(FILENAME_ITEM_DESC);
	SetItemFileChecksum(nItemChecksum);

	if (!InitEvent())
	{
		Log(LOG_ALL, "init event failed.\n");
		return false;
	}
#ifdef _DEBUG
	CheckItemXML();
	CheckUpdateItemXML();
#endif

	return true;
}

bool MMatchServer::LoadChannelPreset()
{
#define MTOK_DEFAULTCHANNELNAME		"DEFAULTCHANNELNAME"
#define MTOK_DEFAULTRULENAME		"DEFAULTRULENAME"
#define MTOK_CHANNEL				"CHANNEL"

	MXmlDocument	xmlIniData;
	xmlIniData.Create();

	if (!xmlIniData.LoadFromFile(FILENAME_CHANNEL))
	{
		xmlIniData.Destroy();
		return false;
	}

	MXmlElement rootElement, childElement;
	char szTagName[256];
	char szBuf[256];

	rootElement = xmlIniData.GetDocumentElement();
	int iCount = rootElement.GetChildNodeCount();

	for (int i = 0; i < iCount; i++)
	{
		childElement = rootElement.GetChildNode(i);
		childElement.GetTagName(szTagName);
		if (szTagName[0] == '#') continue;

		if (!strcmp(szTagName, MTOK_CHANNEL))
		{
			char szRuleName[256] = "";
			int nMaxPlayers = 0;
			int nLevelMin = -1;
			int nLevelMax = -1;

			childElement.GetAttribute(szBuf, "name");
			if (childElement.GetAttribute(szRuleName, "rule") == false)
				strcpy_safe(szRuleName, GetDefaultChannelRuleName());
			childElement.GetAttribute(&nMaxPlayers, "maxplayers");
			childElement.GetAttribute(&nLevelMin, "levelmin");
			childElement.GetAttribute(&nLevelMax, "levelmax");

			MUID uidChannel;
			ChannelAdd(MGetStringResManager()->GetStringFromXml(szBuf),
				szRuleName, &uidChannel, MCHANNEL_TYPE_PRESET, nMaxPlayers, nLevelMin, nLevelMax);
		}
		else if (!strcmp(szTagName, MTOK_DEFAULTCHANNELNAME))
		{
			childElement.GetAttribute(szBuf, "name");
			SetDefaultChannelName(MGetStringResManager()->GetStringFromXml(szBuf));
		}
		else if (!strcmp(szTagName, MTOK_DEFAULTRULENAME))
		{
			childElement.GetAttribute(szBuf, "name");
			SetDefaultChannelRuleName(szBuf);
		}
	}

	xmlIniData.Destroy();
	return true;
}

bool MMatchServer::InitDB()
{
	Database = MakeDatabaseFromConfig();
	return bool(Database);
}

bool MMatchServer::Create(int nPort)
{
	srand(static_cast<unsigned int>(GetGlobalTimeMS()));

	m_NextUseUID.SetZero();
	m_NextUseUID.Increase(10);

	Net.SetLogCallback(RcpLog);
#ifdef _DEBUG
	Net.SetLogLevel(1);
#else
	Net.SetLogLevel(0);
#endif

	if (m_SafeUDP.Create(true, MATCHSERVER_DEFAULT_UDP_PORT) == false) {
		LOG(LOG_ALL, "Match Server SafeUDP Create FAILED (Port:%d)", MATCHSERVER_DEFAULT_UDP_PORT);
		return false;
	}

	m_SafeUDP.SetCustomRecvCallback(UDPSocketRecvEvent);

	if (!LoadInitFile()) return false;

	if (!InitDB()) return false;

	m_AsyncProxy.Create(DEFAULT_ASYNCPROXY_THREADPOOL);

	m_Admin.Create(this);

	if (MServer::Create(nPort) == false) return false;

	GetDBMgr()->UpdateServerInfo(MGetServerConfig()->GetServerID(), MGetServerConfig()->GetMaxUser(),
		MGetServerConfig()->GetServerName());


	MGetServerStatusSingleton()->Create(this);

	if (!InitScheduler()) {
		LOG(LOG_ALL, "Match Server Scheduler Create FAILED");
		return false;
	}

	MMatchAntiHack::InitClientFileList();

	if (OnCreate() == false) {
		LOG(LOG_ALL, "Match Server create FAILED (Port:%d)", nPort);
		return false;
	}

	m_bCreated = true;

	LOG(LOG_ALL, "Match Server Created (Port:%d)", nPort);

	g_PointerChecker[0].Init(NULL);
	g_PointerChecker[1].Init(m_pScheduler);

	LagComp.Create();

	return true;
}

void MMatchServer::Destroy()
{
	m_bCreated = false;

	OnDestroy();

	GetQuest()->Destroy();

	for (auto* Obj : MakePairValueAdapter(m_Objects))
		if (Obj)
			CharFinalize(Obj->GetUID());

	m_ClanMap.Destroy();
	m_ChannelMap.Destroy();
	m_Admin.Destroy();
	m_AsyncProxy.Destroy();
	MGetMatchShop()->Destroy();
	m_SafeUDP.Destroy();
	MServer::Destroy();

	MMatchStringResManager::FreeInstance();
}

void MMatchServer::Shutdown()
{
	Log(LOG_ALL, "MatchServer Shutting down...\n");
}

bool MMatchServer::OnCreate(void)
{
	return true;
}
void MMatchServer::OnDestroy(void)
{
	if (0 != m_pScheduler) {
		m_pScheduler->Release();
		delete m_pScheduler;
		m_pScheduler = 0;
	}
}

void MMatchServer::OnRegisterCommand(MCommandManager* pCommandManager)
{
	MCommandCommunicator::OnRegisterCommand(pCommandManager);
	MAddSharedCommandTable(pCommandManager,
		static_cast<MSharedCommandType::Type>(MSharedCommandType::MatchServer | MSharedCommandType::Client));
	Log(LOG_ALL, "Command registeration completed");

}


void MMatchServer::OnPrepareRun()
{
	MServer::OnPrepareRun();

	MGetServerStatusSingleton()->AddCmdCount(m_CommandManager.GetCommandQueueCount());
}

template <typename T>
MCommandParameterBlob* MakeBlobArrayParameter(uint32_t NumBlobs)
{
	uint32_t OneBlobSize = sizeof(T);
	size_t TotalSize = MGetBlobArrayInfoSize() + OneBlobSize * NumBlobs;
	auto Param = new MCmdParamBlob(TotalSize);
	memcpy(Param->GetPointer(), &OneBlobSize, sizeof(OneBlobSize));
	memcpy(((uint8_t*)Param->GetPointer()) + sizeof(uint32_t), &NumBlobs, sizeof(NumBlobs));
	return Param;
}

void MMatchServer::OnRun(void)
{
	MGetServerStatusSingleton()->SetRunStatus(100);

	SetTickTime(GetGlobalTimeMS());

	if (m_pScheduler)
		m_pScheduler->Update();

	MPremiumIPCache()->Update();

	MGetServerStatusSingleton()->SetRunStatus(101);

	// Update Objects
	auto nGlobalClock = GetGlobalClockCount();
	for (MMatchObjectList::iterator i = m_Objects.begin(); i != m_Objects.end();) {
		MMatchObject* pObj = (*i).second;
		pObj->Tick(nGlobalClock);

		if (pObj->GetDisconnStatusInfo().IsSendDisconnMsg())
		{
			MCommand* pCmd = CreateCommand(MC_MATCH_DISCONNMSG, pObj->GetUID());
			pCmd->AddParameter(new MCmdParamUInt(pObj->GetDisconnStatusInfo().GetMsgID()));
			Post(pCmd);

			pObj->GetDisconnStatusInfo().SendCompleted();
		}
		else if (pObj->GetDisconnStatusInfo().IsDisconnectable(nGlobalClock))
		{
			// BlockType
			if (pObj->GetDisconnStatusInfo().IsUpdateDB())
			{
				MAsyncDBJob_SetBlockAccount* pJob = new MAsyncDBJob_SetBlockAccount;

				pJob->Input(pObj->GetAccountInfo()->m_nAID,
					(0 != pObj->GetCharInfo()) ? pObj->GetCharInfo()->m_nCID : 0,
					pObj->GetDisconnStatusInfo().GetBlockType(),
					pObj->GetDisconnStatusInfo().GetBlockLevel(),
					pObj->GetDisconnStatusInfo().GetComment(),
					pObj->GetIPString(),
					pObj->GetDisconnStatusInfo().GetEndDate());

				PostAsyncJob(pJob);

				pObj->GetDisconnStatusInfo().UpdateDataBaseCompleted();
			}

			DisconnectObject(pObj->GetUID());
		}

		i++;
	}

	MGetServerStatusSingleton()->SetRunStatus(102);

	// Update Stages
	for (MMatchStageMap::iterator iStage = m_StageMap.begin(); iStage != m_StageMap.end();) {
		MMatchStage* pStage = (*iStage).second;

		pStage->Tick(nGlobalClock);

		if (pStage->GetState() == STAGE_STATE_CLOSE) {

			StageRemove(pStage->GetUID(), &iStage);
			continue;
		}
		else {
			iStage++;
		}
	}

	MGetServerStatusSingleton()->SetRunStatus(103);

	// Update Channels
	m_ChannelMap.Update(nGlobalClock);

	MGetServerStatusSingleton()->SetRunStatus(104);

	// Update Clans
	m_ClanMap.Tick(nGlobalClock);

	MGetServerStatusSingleton()->SetRunStatus(105);

	// Update Ladders
	if ( (MGetServerConfig()->GetServerMode() == MSM_CLAN) || (MGetServerConfig()->GetServerMode() == MSM_TEST))
	{
		GetLadderMgr()->Tick(nGlobalClock);
	}

	MGetServerStatusSingleton()->SetRunStatus(106);

	// Ping all ingame players every half second
	if (nGlobalClock - LastPingTime > 500)
	{
		// Send ping list
		for (auto Stage : MakePairValueAdapter(m_StageMap))
		{
			if (Stage->GetState() == STAGE_STATE_RUN
				&& Stage->GetStageSetting()->GetNetcode() == NetcodeType::ServerBased)
			{
				auto Command = CreateCommand(MC_MATCH_PING_LIST, MUID(0, 0));

				size_t NumPlayers = Stage->GetObjCount();

				auto Param = MakeBlobArrayParameter<MTD_PingInfo>(NumPlayers);
				auto PingInfos = static_cast<MTD_PingInfo*>(MGetBlobArrayPointer(Param->GetPointer()));

				auto ObjList = Stage->GetObjectList();
				auto it = ObjList.begin();
				auto end = ObjList.end();
				for (size_t i = 0; i < Stage->GetObjCount() && it != end; i++, it++)
				{
					PingInfos[i] = MTD_PingInfo{ it->GetUID(), uint16_t(it->GetPing() / 2) };
				}

				Command->AddParameter(Param);

				RouteToBattle(Stage->GetUID(), Command);
			}
		}

		// Ping all in-game clients
		MCommand* pNew = CreateCommand(MC_NET_PING, MUID(0, 0));
		pNew->AddParameter(new MCmdParamUInt(static_cast<u32>(GetGlobalClockCount())));
		RouteToAllClientIf(pNew, [](MMatchObject& Obj) {
			return Obj.GetPlace() == MMP_BATTLE; });
		LastPingTime = nGlobalClock;
	}

	// Garbage Session Cleaning
#define MINTERVAL_GARBAGE_SESSION_PING	(5 * 60 * 1000)	// 3 min
	static auto tmLastGarbageSessionCleaning = nGlobalClock;
	if (nGlobalClock - tmLastGarbageSessionCleaning > MINTERVAL_GARBAGE_SESSION_PING) {
		tmLastGarbageSessionCleaning = nGlobalClock;

		LOG(LOG_ALL, "ClientCount=%d, SessionCount=%d, AgentCount=%d",
			GetClientCount(), GetCommObjCount(), GetAgentCount());
		MCommand* pNew = CreateCommand(MC_NET_PING, MUID(0, 0));
		pNew->AddParameter(new MCmdParamUInt(static_cast<u32>(GetGlobalClockCount())));
		RouteToAllConnection(pNew);
	}

	// Garbage MatchObject Cleaning
#define MINTERVAL_GARBAGE_SESSION_CLEANING	10*60*1000		// 10 min
	for (MMatchObjectList::iterator i = m_Objects.begin(); i != m_Objects.end(); i++) {
		MMatchObject* pObj = (MMatchObject*)((*i).second);
		if (pObj->GetUID() < MUID(0, 3)) continue;
		if (pObj->GetPlayerFlags() & MTD_PlayerFlags_Bot) continue;
		if (GetTickTime() - pObj->GetTickLastPacketRecved() >= MINTERVAL_GARBAGE_SESSION_CLEANING) {
			LOG(LOG_PROG, "TIMEOUT CLIENT CLEANING : %s(%u%u, %s) (ClientCnt=%d, SessionCnt=%d)",
				pObj->GetName(), pObj->GetUID().High, pObj->GetUID().Low, pObj->GetIPString(), GetClientCount(), GetCommObjCount());

			MUID uid = pObj->GetUID();
			ObjectRemove(uid, &i);
			Disconnect(uid);

			if (i == m_Objects.end())
				break;
		}
	}

	MGetServerStatusSingleton()->SetRunStatus(107);

	MGetServerStatusSingleton()->SetRunStatus(108);

	// Process Async Jobs
	ProcessAsyncJob();

	MGetServerStatusSingleton()->SetRunStatus(109);

	// Update Logs
	UpdateServerLog();
	UpdateServerStatusDB();

	MGetServerStatusSingleton()->SetRunStatus(110);

	MGetServerStatusSingleton()->SetRunStatus(111);

	// Shutdown...
	m_MatchShutdown.OnRun(nGlobalClock);

	MGetServerStatusSingleton()->SetRunStatus(112);
}

void MMatchServer::UpdateServerLog()
{
	if (!IsCreated()) return;

#define SERVER_LOG_TICK		(60000)

	static u64 st_nElapsedTime = 0;
	static auto nLastTime = GetGlobalTimeMS();
	auto nNowTime = GetGlobalTimeMS();

	st_nElapsedTime += (nNowTime - nLastTime);

	if (st_nElapsedTime > SERVER_LOG_TICK)
	{
		st_nElapsedTime = 0;

		GetDBMgr()->InsertServerLog(MGetServerConfig()->GetServerID(),
			(int)m_Objects.size(), (int)m_StageMap.size(),
			0, 0);
	}

	nLastTime = nNowTime;
}

void MMatchServer::UpdateServerStatusDB()
{
	if (!IsCreated()) return;

#define SERVER_STATUS_TICK		(30000)	// 30초 (1000 * 30)

	static u64 st_nElapsedTime = 0;
	static auto nLastTime = GetGlobalTimeMS();
	auto nNowTime = GetGlobalTimeMS();

	st_nElapsedTime += (nNowTime - nLastTime);

	if (st_nElapsedTime > SERVER_STATUS_TICK)
	{
		st_nElapsedTime = 0;

		int nObjSize = (int)m_Objects.size();
		if (nObjSize > MGetServerConfig()->GetMaxUser()) nObjSize = MGetServerConfig()->GetMaxUser();

		static int st_ErrCounter = 0;
		if (GetDBMgr()->UpdateServerStatus(MGetServerConfig()->GetServerID(), nObjSize) == false)
		{
			LOG(LOG_ALL, "[CRITICAL ERROR] DB Connection Lost. ");

			if (auto DB = GetDBMgr())
			{
				InitDB();
			}
			st_ErrCounter++;
			if (st_ErrCounter > MAX_DB_QUERY_COUNT_OUT)
			{
				LOG(LOG_ALL, "[CRITICAL ERROR] UpdateServerStatusDB - Shutdown");
				Shutdown();
			}
		}
		else
		{
			st_ErrCounter = 0;
		}
	}

	nLastTime = nNowTime;
}

inline void MMatchServer::RouteToListener(MObject* pObject, MCommand* pCommand)
{
	if (pObject == NULL) return;

	size_t nListenerCount = pObject->m_CommListener.size();
	if (nListenerCount <= 0) {
		delete pCommand;
		return;
	}
	else if (nListenerCount == 1) {
		MUID TargetUID = *pObject->m_CommListener.begin();
		pCommand->m_Receiver = TargetUID;
		Post(pCommand);
	}
	else {
		int nCount = 0;
		for (list<MUID>::iterator itorUID = pObject->m_CommListener.begin(); itorUID != pObject->m_CommListener.end(); itorUID++) {
			MUID TargetUID = *itorUID;

			MCommand* pSendCmd;
			if (nCount <= 0)
				pSendCmd = pCommand;
			else
				pSendCmd = pCommand->Clone();
			pSendCmd->m_Receiver = TargetUID;
			Post(pSendCmd);
			nCount++;
		}
	}
}

void MMatchServer::RouteResponseToListener(MObject* pObject, const int nCmdID, int nResult)
{
	MCommand* pNew = CreateCommand(nCmdID, MUID(0, 0));
	pNew->AddParameter(new MCmdParamInt(nResult));
	RouteToListener(pObject, pNew);
}

void MMatchServer::OnVoiceChat(const MUID & Player, unsigned char* EncodedFrame, int Length)
{
	auto Obj = GetObject(Player);

	if (!Obj)
		return;

	auto Stage = Obj->GetStageUID();

	auto Command = CreateCommand(MC_MATCH_RECEIVE_VOICE_CHAT, MUID(0, 0));
	Command->AddParameter(new MCommandParameterUID(Player));
	Command->AddParameter(new MCommandParameterBlob(EncodedFrame, Length));

	RouteToBattleExcept(Stage, Command, Player);
}

void MMatchServer::OnRequestCreateBot(const MUID& OwnerUID)
{
	auto OwnerObj = GetObject(OwnerUID);
	if (!OwnerObj)
		return;

	auto StageUID = OwnerObj->GetStageUID();
	auto* Stage = FindStage(StageUID);
	if (!Stage)
		return;

	if (OwnerObj->BotUID.IsInvalid())
	{
		OwnerObj->BotUID = UseUID();
	}

	auto BotUID = OwnerObj->BotUID;

	Stage->Bots.push_back({ OwnerUID, BotUID, MTD_PeerListNode{} });
	auto&& Node = Stage->Bots.back().PeerListNode;

	Node.uidChar = BotUID;

	auto&& info = Node.CharInfo;

	strcpy_safe(info.szName, "Bot");
	info.nSex = MMS_FEMALE;
	info.nHP = 100;
	info.nAP = 0;
	info.nLevel = 0;

	// Copy owner's weapons to the bots weapons.
	// We don't have an MTD_CharInfo at hand so we have to get each item ID ourselves.
	for (size_t i = size_t(MMCIP_MELEE); i <= size_t(MMCIP_CUSTOM2); ++i)
	{
		auto& DestItemID = info.nEquipedItemDesc[i];
		auto& SrcItem = OwnerObj->GetCharInfo()->m_EquipedItem;
		auto Parts = MMatchCharItemParts(i);
		auto* Item = SrcItem.IsEmpty(Parts) ? nullptr : SrcItem.GetItem(Parts);
		DestItemID = Item ? Item->GetDescID() : 0;
	}

	Node.ExtendInfo.nTeam = MMT_ALL;
	Node.ExtendInfo.nPlayerFlags = MTD_PlayerFlags_Bot;

	// We use the dwIP and nPort fields to indicate the bots.
	// when sending the packet to someonm other than the owner, we use the owner's IP and port,
	// from which the receiving client identifies the owner.
	// However, since clients themselves don't know their own IP, we send sentinel values that
	// indicate "you're the owner" instead.
	Node.dwIP = OwnerObj->GetIP();
	Node.nPort = OwnerObj->GetPort();

	StaticBlobArray<MTD_PeerListNode, 1> BlobArray;
	auto& CommandNode = BlobArray.Get(0);
	CommandNode = Node;

	auto MakeCommand = [&] {
		auto* Command = CreateCommand(MC_MATCH_STAGE_ENTERBATTLE, MUID(0, 0));
		Command->AddParameter(new MCommandParameterUChar(MCEP_NORMAL));
		Command->AddParameter(new MCommandParameterBlob(BlobArray));
		return Command;
	};

	RouteToBattleExcept(StageUID, MakeCommand(), OwnerUID);

	CommandNode.dwIP = u32(-1);
	CommandNode.nPort = 0;
	RouteToListener(OwnerObj, MakeCommand());
}

MMatchObject* MMatchServer::AddBot(const MUID& StageUID, MMatchTeam Team)
{
	auto* Stage = FindStage(StageUID);
	if (!Stage)
		return nullptr;

	auto UID = UseUID();

	{
		auto Err = ObjectAdd(UID);
		if (Err != MOK)
		{
			MLog("AddVirtualClient -- Failed to add object\n");
			return nullptr;
		}
	}

	auto Object = GetObject(UID);
	Object->SetPeerAddr(u32(-1), "255.255.255.255", 1);
	Object->SetPlayerFlag(MTD_PlayerFlags_Bot, true);
	auto CharInfo = new MMatchCharInfo;
	strcpy_safe(CharInfo->m_szName, "Bot");
	Object->SetCharInfo(CharInfo);

	OnStageJoin(UID, StageUID);
	StageTeam(UID, StageUID, Team);
	OnStageEnterBattle(UID, StageUID);

	return Object;
}

void MMatchServer::OnRequestSpec(const MUID& UID, bool Value)
{
	auto Object = GetObject(UID);
	if (!Object) return;
	if (!Object->GetEnterBattle()) return;
	auto StageUID = Object->GetStageUID();
	auto Stage = FindStage(StageUID);
	if (!Stage) return;

	bool IsSpec = Object->GetTeam() == MMT_SPECTATOR;
	if (Value == IsSpec) return;
	MMatchTeam NewTeam;
	if (Value)
	{
		OnGameKill(UID, UID);
		NewTeam = MMT_SPECTATOR;
	}
	else if (Stage->GetStageSetting()->IsTeamPlay())
	{
		int Red = 0, Blue = 0;
		for (auto&& Object : MakePairValueAdapter(*GetObjects()))
		{
			auto Team = Object->GetTeam();
			if (Team == MMT_RED)
				++Red;
			else if (Team == MMT_BLUE)
				++Blue;
		}
		NewTeam = Red > Blue ? MMT_BLUE : MMT_RED;
	}
	else
	{
		NewTeam = MMT_ALL;
	}

	// For players ingame.
	auto* Command = CreateCommand(MC_MATCH_RESPONSE_SPEC, MUID(0, 0));
	Command->AddParameter(new MCmdParamUID(UID));
	Command->AddParameter(new MCmdParamUInt(u32(NewTeam)));
	RouteToBattle(StageUID, Command);

	// For players in the gameroom.
	StageTeam(UID, StageUID, NewTeam);
}

static int GetBlobCmdID(const char* Data)
{
	return *(u16*)(Data + 2);
}

void MMatchServer::OnTunnelledP2PCommand(const MUID & Sender, const MUID & Receiver, const char * Blob, size_t BlobSize)
{
	auto SenderObj = GetObject(Sender);
	if (!SenderObj)
		return;

	auto CommandID = GetBlobCmdID(Blob);
	auto uidStage = SenderObj->GetStageUID();
	auto Stage = FindStage(uidStage);
	if (!Stage)
		return;

	auto Netcode = Stage->GetStageSetting()->GetNetcode();

	if (Netcode == NetcodeType::ServerBased)
	{
		auto TrySuicide = [&](float Z, MUID Sender)
		{
			constexpr auto DIE_CRITICAL_LINE = -2500.f;
			if (Z <= DIE_CRITICAL_LINE)
			{
				OnGameKill(Sender, Sender);
			}
		};

		switch (CommandID)
		{
		case MC_PEER_BASICINFO:
		{
			if (!SenderObj->IsAlive())
				return;

			BasicInfoItem bi;
			ZPACKEDBASICINFO& pbi = *(ZPACKEDBASICINFO*)(Blob + 2 + 2 + 1 + 4);
			pbi.Unpack(bi);
			bi.SentTime = pbi.fTime;
			bi.RecvTime = MGetMatchServer()->GetGlobalClockCount() / 1000.0;
			SenderObj->BasicInfoHistory.AddBasicInfo(bi);

			TrySuicide(bi.position.z, Sender);
		}
		break;
		case MC_PEER_BASICINFO_RG:
		{
			if (!SenderObj->IsAlive())
				return;

			NewBasicInfo nbi;
			auto* pbi = reinterpret_cast<const u8*>(Blob + 2 + 2 + 1 + 4);
			if (!UnpackNewBasicInfo(nbi, pbi, BlobSize - 9))
				return;
			nbi.bi.SentTime = nbi.Time;
			nbi.bi.RecvTime = MGetMatchServer()->GetGlobalClockCount() / 1000.0;
			SenderObj->BasicInfoHistory.AddBasicInfo(nbi.bi);

			TrySuicide(nbi.bi.position.z, Sender);
		}
		break;
		case MC_PEER_SHOT:
		{
			ZPACKEDSHOTINFO& psi = *(ZPACKEDSHOTINFO*)(Blob + 2 + 2 + 1 + 4);
			OnPeerShot(*SenderObj, *Stage, psi);
		}
		break;
		case MC_PEER_SHOT_SP:
		{
			auto Cmd = MakeCmdFromSaneTunnelingBlob(Sender, MUID(0, 0), Blob, BlobSize);

			if (Cmd)
			{
				v3 Pos, Dir;
				ZC_SHOT_SP_TYPE Type;
				int SelectedSlot;
				if (!Cmd->GetParameter(&Pos, 1, MPT_POS) ||
					!Cmd->GetParameter(&Dir, 2, MPT_VECTOR) ||
					!Cmd->GetParameter(&Type, 3, MPT_INT) ||
					!Cmd->GetParameter(&SelectedSlot, 4, MPT_INT))
				{
					delete Cmd;
					return;
				}

				auto Item = SenderObj->GetCharInfo()->m_EquipedItem.GetItem(MMatchCharItemParts(SelectedSlot));
				if (!Item)
				{
					delete Cmd;
					return;
				}
				auto ItemDesc = Item->GetDesc();
				if (!ItemDesc)
				{
					delete Cmd;
					return;
				}

				switch (Type)
				{
				case ZC_WEAPON_SP_ROCKET:
					Stage->MovingWeaponMgr.AddRocket(SenderObj, ItemDesc, Pos, Dir);
					break;
				case ZC_WEAPON_SP_ITEMKIT:
					Stage->MovingWeaponMgr.AddItemKit(SenderObj, ItemDesc, Pos, Dir);
					break;
				case ZC_WEAPON_SP_GRENADE:
					auto GrenadeSpeed = 1200.f;
					Stage->MovingWeaponMgr.AddGrenade(SenderObj, ItemDesc, Pos, Dir,
						Dir * GrenadeSpeed + SenderObj->GetVelocity() + v3{ 0, 0, 300 });
					break;
				};

				delete Cmd;
			}
		}
		break;
		case MC_PEER_DIE:
			// Players can't choose to die. They can choose to /suicide, but that's a different command.
			return;
		case MC_PEER_HPAPINFO:
			return;
		};
	}

	MCommand* pCmd = CreateCommand(MC_MATCH_P2P_COMMAND, MUID(0, 0));
	pCmd->AddParameter(new MCmdParamUID(Sender));
	pCmd->AddParameter(new MCmdParamBlob(Blob, BlobSize));
	if (Receiver == MUID{ 0, 0 })
		RouteToBattleExcept(uidStage, pCmd, Sender);
	else
	{
		auto* ReceiverObj = GetObject(Receiver);
		if (ReceiverObj)
			RouteToListener(ReceiverObj, pCmd);
	}
}

void MMatchServer::OnPeerShot(MMatchObject& SenderObj, MMatchStage& Stage, const ZPACKEDSHOTINFO& psi)
{
	if (!SenderObj.IsAlive())
		return;

	auto&& SenderUID = SenderObj.GetUID();

	v3 src = v3(psi.posx, psi.posy, psi.posz);
	v3 dest = v3(psi.tox, psi.toy, psi.toz);
	v3 orig_dir = dest - src;
	RealSpace2::Normalize(orig_dir);

	//AnnounceF(Sender, "Shot! %f, %f, %f -> %f, %f, %f", src.x, src.y, src.z, dest.x, dest.y, dest.z);

	auto Time = double(GetGlobalClockCount() - SenderObj.GetPing()) / 1000;

	auto Slot = SenderObj.GetSelectedSlot();

	auto Item = SenderObj.GetCharInfo()->m_EquipedItem.GetItem(Slot);
	if (!Item)
		return;

	auto ItemDesc = Item->GetDesc();
	if (!ItemDesc)
		return;

	auto Damage = ItemDesc->m_nDamage;

	auto DamagePlayer = [&](auto& Obj, auto Damage, auto PiercingRatio, auto DamageType, auto WeaponType)
	{
		Obj.OnDamaged(SenderObj, { 0, 0, 0 }, DamageType, WeaponType, Damage, PiercingRatio);
	};

	if (ItemDesc->m_nWeaponType == MWT_SHOTGUN)
	{
		struct DamageInfo
		{
			int Damage = 0;
			float PiercingRatio = 0;
			ZDAMAGETYPE DamageType;
			MMatchWeaponType WeaponType;
		};

		auto DirGen = GetShotgunPelletDirGenerator(orig_dir, reinterpret<u32>(psi.fTime));

		std::unordered_map<MMatchObject*, DamageInfo> DamageMap;

		for (int i = 0; i < SHOTGUN_BULLET_COUNT; i++)
		{
			auto dir = DirGen();
			auto dest = src + dir * 10000;

			const u32 PassFlag = RM_FLAG_ADDITIVE | RM_FLAG_HIDE | RM_FLAG_PASSROCKET | RM_FLAG_PASSBULLET;

			MPICKINFO pickinfo;
			PickHistory(&SenderObj, src, dest, Stage.BspObject,
				pickinfo, MakePairValueAdapter(Stage.m_ObjUIDCaches), Time, PassFlag);

			if (pickinfo.bBspPicked)
			{
				/*AnnounceF(Sender, "Server: Hit wall at %d, %d, %d",
				(int)pickinfo.bpi.PickPos.x, (int)pickinfo.bpi.PickPos.y,
				(int)pickinfo.bpi.PickPos.z);*/
				continue;
			}

			if (!pickinfo.pObject)
			{
				if (SenderObj.clientSettings.DebugOutput)
					AnnounceF(SenderUID, "Server: No wall, no object");
				continue;
			}

			float PiercingRatio = GetPiercingRatio(ItemDesc->m_nWeaponType, pickinfo.info.parts);

			auto& item = DamageMap[pickinfo.pObject];

			int NewDamage = item.Damage + Damage;
			if (PiercingRatio != item.PiercingRatio)
				item.PiercingRatio = (item.Damage * item.PiercingRatio + Damage * PiercingRatio)
				/ NewDamage;
			item.Damage += Damage;
			auto DamageType = (pickinfo.info.parts == eq_parts_head) ? ZD_BULLET_HEADSHOT : ZD_BULLET;
			static_assert(ZD_BULLET_HEADSHOT > ZD_BULLET, "Fix me");
			item.DamageType = max(item.DamageType, DamageType);
			item.WeaponType = ItemDesc->m_nWeaponType;
		}

		for (auto& item : DamageMap)
		{
			DamagePlayer(*item.first,
				item.second.Damage, item.second.PiercingRatio,
				item.second.DamageType, item.second.WeaponType);

			if (SenderObj.clientSettings.DebugOutput)
			{
				AnnounceF(SenderUID, "Server: Hit %s for %d damage",
					item.first->GetName(), item.second.Damage);
				v3 Head, Origin;
				SenderObj.GetPositions(&Head, &Origin, Time);
				AnnounceF(SenderUID, "Server: Head: %d, %d, %d; origin: %d, %d, %d",
					int(Head.x), int(Head.y), int(Head.z),
					int(Origin.x), int(Origin.y), int(Origin.z));
			}
		}
	}
	else
	{
		const u32 PassFlag = RM_FLAG_ADDITIVE | RM_FLAG_HIDE | RM_FLAG_PASSROCKET | RM_FLAG_PASSBULLET;

		MPICKINFO pickinfo;
		PickHistory(&SenderObj, src, dest, Stage.BspObject, pickinfo,
			MakePairValueAdapter(Stage.m_ObjUIDCaches), Time, PassFlag);

		if (pickinfo.bBspPicked)
		{
			return;
		}

		if (!pickinfo.pObject)
		{
			if (SenderObj.clientSettings.DebugOutput)
				AnnounceF(SenderUID, "Server: No wall, no object");
			return;
		}

		float PiercingRatio = GetPiercingRatio(ItemDesc->m_nWeaponType, pickinfo.info.parts);
		auto DamageType = (pickinfo.info.parts == eq_parts_head) ? ZD_BULLET_HEADSHOT : ZD_BULLET;
		auto WeaponType = ItemDesc->m_nWeaponType;

		if (SenderObj.clientSettings.DebugOutput)
		{
			AnnounceF(SenderUID, "Server: Hit %s for %d damage", pickinfo.pObject->GetName(), Damage);
			v3 Head, Origin;
			SenderObj.GetPositions(&Head, &Origin, Time);
			AnnounceF(SenderUID, "Server: Head: %d, %d, %d; origin: %d, %d, %d",
				int(Head.x), int(Head.y), int(Head.z),
				int(Origin.x), int(Origin.y), int(Origin.z));
		}

		DamagePlayer(*pickinfo.pObject, Damage, PiercingRatio, DamageType, WeaponType);
	}
}

struct stRouteListenerNode
{
	uintptr_t			nUserContext;
	MPacketCrypterKey	CryptKey;
};

void MMatchServer::RouteToAllConnection(MCommand* pCommand)
{
	queue<stRouteListenerNode*>	ListenerList;

	// Queueing for SafeSend
	LockCommList();
	for (auto i = m_CommRefCache.begin(); i != m_CommRefCache.end(); i++) {
		MCommObject* pCommObj = i->second;
		if (pCommObj->GetUID() < MUID(0, 3)) continue;

		stRouteListenerNode* pNewNode = new stRouteListenerNode;
		pNewNode->nUserContext = pCommObj->GetUserContext();
		memcpy(&pNewNode->CryptKey, pCommObj->GetCrypter()->GetKey(), sizeof(MPacketCrypterKey));
		ListenerList.push(pNewNode);
	}
	UnlockCommList();

	// Send the queue (each all session)
	int nCmdSize = pCommand->GetSize();

	if (nCmdSize <= 0)
	{
		while (!ListenerList.empty())
		{
			stRouteListenerNode* pNode = ListenerList.front();
			ListenerList.pop();
			delete pNode;
		}
		return;
	}

	char* pCmdData = new char[nCmdSize];
	int nSize = pCommand->GetData(pCmdData, nCmdSize);
	_ASSERT(nSize < MAX_PACKET_SIZE && nSize == nCmdSize);


	if (pCommand->m_pCommandDesc->IsFlag(MCCT_NON_ENCRYPTED))
	{
		while (!ListenerList.empty())
		{
			stRouteListenerNode* pNode = ListenerList.front();
			ListenerList.pop();

			SendMsgCommand(pNode->nUserContext, pCmdData, nSize, MSGID_RAWCOMMAND, NULL);

			delete pNode;
		}
	}
	else
	{
		while (!ListenerList.empty())
		{
			stRouteListenerNode* pNode = ListenerList.front();
			ListenerList.pop();

			SendMsgCommand(pNode->nUserContext, pCmdData, nSize, MSGID_COMMAND, &pNode->CryptKey);

			delete pNode;
		}
	}

	delete[] pCmdData;
	delete pCommand;
}

void MMatchServer::RouteToAllClient(MCommand* pCommand)
{
	for (MMatchObjectList::iterator i = m_Objects.begin(); i != m_Objects.end(); i++) {
		MMatchObject* pObj = (MMatchObject*)((*i).second);
		if (pObj->GetUID() < MUID(0, 3)) continue;

		MCommand* pSendCmd = pCommand->Clone();
		pSendCmd->m_Receiver = pObj->GetUID();
		Post(pSendCmd);
	}
	delete pCommand;
}

// sends an admin message to all users
void MMatchServer::Shout(const char* msg)
{
	MCommand* pCmd = CreateCommand(MC_ADMIN_ANNOUNCE, MUID(0, 0));
	pCmd->AddParameter(new MCmdParamUID(MUID(0, 0)));
	pCmd->AddParameter(new MCmdParamStr(msg));
	pCmd->AddParameter(new MCmdParamUInt(ZAAT_CHAT));
	RouteToAllClient(pCmd);
}

void MMatchServer::RouteToChannel(const MUID& uidChannel, MCommand* pCommand)
{
	MMatchChannel* pChannel = FindChannel(uidChannel);
	if (pChannel == NULL)
	{
		delete pCommand;
		return;
	}

	for (auto i = pChannel->GetObjBegin(); i != pChannel->GetObjEnd(); i++) {
		MObject* pObj = i->second;

		MCommand* pSendCmd = pCommand->Clone();
		RouteToListener(pObj, pSendCmd);
	}
	delete pCommand;
}

void MMatchServer::RouteToChannelLobby(const MUID& uidChannel, MCommand* pCommand)
{
	MMatchChannel* pChannel = FindChannel(uidChannel);
	if (pChannel == NULL)
	{
		delete pCommand;
		return;
	}

	for (auto i = pChannel->GetLobbyObjBegin(); i != pChannel->GetLobbyObjEnd(); i++)
	{
		MObject* pObj = i->second;

		MCommand* pSendCmd = pCommand->Clone();
		RouteToListener(pObj, pSendCmd);
	}
	delete pCommand;
}

void MMatchServer::RouteToStage(const MUID& uidStage, MCommand* pCommand)
{
	MMatchStage* pStage = FindStage(uidStage);
	if (pStage == NULL)
	{
		delete pCommand;
		return;
	}

	for (auto i = pStage->GetObjBegin(); i != pStage->GetObjEnd(); i++) {
		MUID uidObj = i->first;
		MObject* pObj = (MObject*)GetObject(uidObj);
		if (pObj) {
			MCommand* pSendCmd = pCommand->Clone();
			RouteToListener(pObj, pSendCmd);
		}
		else {
			LOG(LOG_ALL, "WARNING(RouteToStage) : Not Existing Obj(%u:%u)\n", uidObj.High, uidObj.Low);
			i = pStage->RemoveObject(uidObj);
		}
	}
	delete pCommand;
}

void MMatchServer::RouteToStageWaitRoom(const MUID& uidStage, MCommand* pCommand)
{
	MMatchStage* pStage = FindStage(uidStage);
	if (pStage == NULL)
	{
		delete pCommand;
		return;
	}

	for (auto i = pStage->GetObjBegin(); i != pStage->GetObjEnd(); i++) {

		MUID uidObj = i->first;
		MMatchObject* pObj = (MMatchObject*)GetObject(uidObj);
		if (pObj) {
			if (!pObj->GetEnterBattle())
			{
				MCommand* pSendCmd = pCommand->Clone();
				RouteToListener(pObj, pSendCmd);
			}
		}
	}
	delete pCommand;
}

void MMatchServer::RouteToBattle(const MUID& uidStage, MCommand* pCommand)
{
	MMatchStage* pStage = FindStage(uidStage);
	if (pStage == nullptr)
	{
		delete pCommand;
		return;
	}

	for (auto i = pStage->GetObjBegin(); i != pStage->GetObjEnd(); i++) {
		//MMatchObject* pObj = (MMatchObject*)(*i).second;

		MUID uidObj = i->first;
		MMatchObject* pObj = (MMatchObject*)GetObject(uidObj);
		if (pObj) {
			if (pObj->GetEnterBattle())
			{
				MCommand* pSendCmd = pCommand->Clone();
				RouteToListener(pObj, pSendCmd);
			}
		}
		else {
			LOG(LOG_ALL, "WARNING(RouteToBattle) : Not Existing Obj(%u:%u)\n", uidObj.High, uidObj.Low);
			i = pStage->RemoveObject(uidObj);	// RAONHAJE : 방에 쓰레기UID 남는것 발견시 로그&청소
		}
	}
	delete pCommand;
}

void MMatchServer::RouteToBattleExcept(const MUID& uidStage, MCommand* pCommand, const MUID& uidExceptedPlayer)
{
	MMatchStage* pStage = FindStage(uidStage);
	if (pStage == nullptr)
	{
		delete pCommand;
		return;
	}

	for (auto i = pStage->GetObjBegin(); i != pStage->GetObjEnd(); i++) {
		MUID uidObj = i->first;

		if (uidObj == uidExceptedPlayer)
			continue;

		MMatchObject* pObj = (MMatchObject*)GetObject(uidObj);
		if (pObj) {
			if (pObj->GetEnterBattle())
			{
				MCommand* pSendCmd = pCommand->Clone();
				RouteToListener(pObj, pSendCmd);
			}
		}
		else {
			LOG(LOG_ALL, "WARNING(RouteToBattle) : Not Existing Obj(%u:%u)\n", uidObj.High, uidObj.Low);
			i = pStage->RemoveObject(uidObj);	// RAONHAJE : 방에 쓰레기UID 남는것 발견시 로그&청소
		}
	}
	delete pCommand;
}

void MMatchServer::RouteToClan(const int nCLID, MCommand* pCommand)
{
	MMatchClan* pClan = FindClan(nCLID);
	if (pClan == NULL)
	{
		delete pCommand;
		return;
	}

	for (auto i = pClan->GetMemberBegin(); i != pClan->GetMemberEnd(); i++) {
		MObject* pObj = i->second;

		MCommand* pSendCmd = pCommand->Clone();
		RouteToListener(pObj, pSendCmd);
	}
	delete pCommand;
}

void MMatchServer::ResponseRoundState(const MUID& uidStage)
{
	MMatchStage* pStage = FindStage(uidStage);
	if (pStage == NULL) return;
	MMatchRule* pRule = pStage->GetRule();
	if (pRule == NULL) return;

	MCommand* pCmd = CreateCommand(MC_MATCH_GAME_ROUNDSTATE, MUID(0, 0));
	pCmd->AddParameter(new MCommandParameterUID(pStage->GetUID()));
	pCmd->AddParameter(new MCommandParameterInt(pRule->GetRoundCount()));
	pCmd->AddParameter(new MCommandParameterInt(pRule->GetRoundState()));
	pCmd->AddParameter(new MCommandParameterInt(pRule->GetRoundArg()));

	// 게임 안에 있는 플레이어에게만 전송
	RouteToBattle(uidStage, pCmd);
}

void MMatchServer::ResponseRoundState(MMatchObject* pObj, const MUID& uidStage)
{
	if (pObj == NULL) return;
	MMatchStage* pStage = FindStage(uidStage);
	if (pStage == NULL) return;
	MMatchRule* pRule = pStage->GetRule();
	if (pRule == NULL) return;

	MCommand* pCmd = CreateCommand(MC_MATCH_GAME_ROUNDSTATE, MUID(0, 0));
	pCmd->AddParameter(new MCommandParameterUID(pStage->GetUID()));
	pCmd->AddParameter(new MCommandParameterInt(pRule->GetRoundCount()));
	pCmd->AddParameter(new MCommandParameterInt(pRule->GetRoundState()));
	pCmd->AddParameter(new MCommandParameterInt(pRule->GetRoundArg()));

	RouteToListener(pObj, pCmd);
}

void MMatchServer::NotifyMessage(const MUID& uidChar, int nMsgID)
{
	MMatchObject* pObj = GetObject(uidChar);
	if (pObj == NULL) return;

	MCommand* pNew = new MCommand(m_CommandManager.GetCommandDescByID(MC_MATCH_NOTIFY), MUID(0, 0), m_This);
	pNew->AddParameter(new MCommandParameterUInt(nMsgID));
	RouteToListener(pObj, pNew);
}

int MMatchServer::ObjectAdd(const MUID& uidComm)
{
	MMatchObject* pObj = new MMatchObject(uidComm);
	pObj->UpdateTickLastPacketRecved();

	m_Objects.insert(MMatchObjectList::value_type(pObj->GetUID(), pObj));
	//	*pAllocUID = pObj->GetUID();

		//LOG("Character Added (UID:%d%d)", pObj->GetUID().High, pObj->GetUID().Low);

	return MOK;
}

int MMatchServer::ObjectRemove(const MUID& uid, MMatchObjectList::iterator* pNextItor)
{
	MMatchObjectList::iterator i = m_Objects.find(uid);
	if (i == m_Objects.end()) return MERR_OBJECT_INVALID;

	MMatchObject* pObj = (*i).second;

	// Clear up the Object
	if (pObj->GetChatRoomUID() != MUID(0, 0)) {
		MMatchChatRoomMgr* pChatRoomMgr = GetChatRoomMgr();
		MMatchChatRoom* pRoom = pChatRoomMgr->FindChatRoom(pObj->GetChatRoomUID());
		if (pRoom)
			pRoom->RemovePlayer(pObj->GetUID());
	}
	if (pObj->GetStageUID() != MUID(0, 0)) {
		StageLeaveBattle(pObj->GetUID(), pObj->GetStageUID());
	}
	if (pObj->GetStageUID() != MUID(0, 0)) {
		StageLeave(pObj->GetUID(), pObj->GetStageUID());
	}
	if (pObj->GetChannelUID() != MUID(0, 0)) {
		ChannelLeave(pObj->GetUID(), pObj->GetChannelUID());
	}

	// m_ClanMap에서도 삭제
	m_ClanMap.RemoveObject(pObj->GetUID(), pObj);

	delete pObj;
	pObj = NULL;

	MMatchObjectList::iterator itorTemp = m_Objects.erase(i);
	if (pNextItor)
		*pNextItor = itorTemp;

	return MOK;
}

MMatchObject* MMatchServer::GetObject(const MUID& uid)
{
	MMatchObjectList::iterator i = m_Objects.find(uid);
	if (i == m_Objects.end()) return NULL;
	return (*i).second;
}

MMatchObject* MMatchServer::GetPlayerByCommUID(const MUID& uid)
{
	for (MMatchObjectList::iterator i = m_Objects.begin(); i != m_Objects.end(); i++) {
		MMatchObject* pObj = ((*i).second);
		for (list<MUID>::iterator j = pObj->m_CommListener.begin(); j != pObj->m_CommListener.end(); j++) {
			MUID TargetUID = *j;
			if (TargetUID == uid)
				return pObj;
		}
	}
	return NULL;
}

MMatchObject* MMatchServer::GetPlayerByName(const char* pszName)
{
	for (MMatchObjectList::iterator i = m_Objects.begin(); i != m_Objects.end(); i++) {
		MMatchObject* pObj = ((*i).second);
		if (_stricmp(pObj->GetName(), pszName) == 0)
			return pObj;
	}
	return NULL;
}

MMatchObject* MMatchServer::GetPlayerByAID(u32 nAID)
{
	if (nAID == 0) return NULL;

	for (auto i = m_Objects.begin(); i != m_Objects.end(); i++)
	{
		auto* pObj = i->second;
		if (pObj->GetAccountInfo()->m_nAID == nAID)
			return pObj;
	}
	return NULL;
}





MUID MMatchServer::UseUID(void)
{
	LockUIDGenerate();
	MUID ret = m_NextUseUID;
	m_NextUseUID.Increase();
	UnlockUIDGenerate();
	return ret;
}

void MMatchServer::SetClientClockSynchronize(const MUID& CommUID)
{
	MCommand* pNew = new MCommand(m_CommandManager.GetCommandDescByID(MC_CLOCK_SYNCHRONIZE), CommUID, m_This);
	pNew->AddParameter(new MCommandParameterUInt(static_cast<u32>(GetGlobalClockCount())));
	Post(pNew);
}

void MMatchServer::Announce(const MUID& CommUID, const char* pszMsg)
{
	MCommand* pCmd = CreateCommand(MC_MATCH_ANNOUNCE, CommUID);
	pCmd->AddParameter(new MCmdParamUInt(0));
	pCmd->AddParameter(new MCmdParamStr(pszMsg));
	Post(pCmd);
}

void MMatchServer::Announce(MObject* pObj, const char* pszMsg)
{
	MCommand* pCmd = CreateCommand(MC_MATCH_ANNOUNCE, MUID(0, 0));
	pCmd->AddParameter(new MCmdParamUInt(0));
	pCmd->AddParameter(new MCmdParamStr(pszMsg));
	RouteToListener(pObj, pCmd);
}

void MMatchServer::AnnounceErrorMsg(const MUID& CommUID, const int nErrorCode)
{
}

void MMatchServer::AnnounceErrorMsg(MObject* pObj, const int nErrorCode)
{
}




void MMatchServer::OnBridgePeer(const MUID& uidChar, u32 dwIP, u32 nPort)
{
	auto* pObj = GetObject(uidChar);
	if (!pObj)
		return;

	MSocket::in_addr addr;
	addr.s_addr = dwIP;
	auto IP = GetIPv4String(addr);

	pObj->SetPeerAddr(dwIP, IP.c_str(), static_cast<unsigned short>(nPort));
	pObj->SetBridgePeer(true);
	pObj->SetPlayerFlag(MTD_PlayerFlags_BridgePeer, true);

	ResponseBridgePeer(uidChar, 0);
}

MMatchServer* MMatchServer::GetInstance()
{
	return m_pInstance;
}

u64 MMatchServer::GetGlobalClockCount()
{
	return GetGlobalTimeMS();
}

u32 MMatchServer::ConvertLocalClockToGlobalClock(u32 nLocalClock, u32 nLocalClockDistance)
{
	return (nLocalClock + nLocalClockDistance);
}

u32 MMatchServer::ConvertGlobalClockToLocalClock(u32 nGlobalClock, u32 nLocalClockDistance)
{
	return (nGlobalClock - nLocalClockDistance);
}

void MMatchServer::DebugTest()
{
#ifndef _DEBUG
	return;
#endif

	///////////
	LOG(LOG_DEBUG, "DebugTest: Object List");
	for (MMatchObjectList::iterator it = m_Objects.begin(); it != m_Objects.end(); it++) {
		MMatchObject* pObj = (*it).second;
		LOG(LOG_DEBUG, "DebugTest: Obj(%d%d)", pObj->GetUID().High, pObj->GetUID().Low);
	}
	///////////
}

void MMatchServer::SendCommandByUDP(MCommand* pCommand, char* szIP, int nPort)
{
	_ASSERT(0);

	const int BUF_SIZE = 1024;

	char* szBuf = new char[BUF_SIZE];
	int iMaxPacketSize = BUF_SIZE;

	MPacketHeader a_PacketHeader;
	int iHeaderSize = sizeof(a_PacketHeader);
	int size = pCommand->GetData(szBuf + iHeaderSize, iMaxPacketSize - iHeaderSize);
	size += iHeaderSize;
	a_PacketHeader.nMsg = MSGID_COMMAND;
	a_PacketHeader.nSize = size;
	memcpy(szBuf, &a_PacketHeader, iHeaderSize);

	bool bRet = m_SafeUDP.Send(szIP, nPort, szBuf, size);
}

bool MMatchServer::UDPSocketRecvEvent(u32 dwIP, u16 wRawPort, char* pPacket, u32 dwSize)
{
	if (dwSize < sizeof(MPacketHeader)) return false;

	MPacketHeader*	pPacketHeader;
	pPacketHeader = (MPacketHeader*)pPacket;

	if ((dwSize < pPacketHeader->nSize) ||
		((pPacketHeader->nMsg != MSGID_COMMAND) && (pPacketHeader->nMsg != MSGID_RAWCOMMAND))) return false;

	MMatchServer* pServer = MMatchServer::GetInstance();
	pServer->ParseUDPPacket(&pPacket[sizeof(MPacketHeader)], pPacketHeader, dwIP, wRawPort);
	return true;
}

void MMatchServer::ParseUDPPacket(char* pData, MPacketHeader* pPacketHeader, u32 dwIP, u16 wRawPort)
{
	switch (pPacketHeader->nMsg)
	{
	case MSGID_RAWCOMMAND:
	{
		unsigned short nCheckSum = MBuildCheckSum(pPacketHeader, pPacketHeader->nSize);
		if (pPacketHeader->nCheckSum != nCheckSum) {
			static int nLogCount = 0;
			if (nLogCount++ < 100) {	// Log Flooding 방지
				mlog("MMatchServer::ParseUDPPacket() -> CHECKSUM ERROR(R=%u/C=%u)\n",
					pPacketHeader->nCheckSum, nCheckSum);
			}
			return;
		}
		else {
			MCommand* pCmd = new MCommand();
			pCmd->SetData(pData, &m_CommandManager);

			if (pCmd->GetID() == MC_MATCH_BRIDGEPEER) {
				pCmd->m_Sender = MUID(0, 0);
				pCmd->m_Receiver = m_This;

				u32 nPort = MSocket::ntohs(wRawPort);

				MCommandParameterUInt* pParamIP = (MCommandParameterUInt*)pCmd->GetParameter(1);
				MCommandParameterUInt* pParamPort = (MCommandParameterUInt*)pCmd->GetParameter(2);
				if (pParamIP == NULL || pParamIP->GetType() != MPT_UINT)
				{
					delete pCmd;
					break;
				}
				if (pParamPort == NULL || pParamPort->GetType() != MPT_UINT)
				{
					delete pCmd;
					break;
				}

				char pData[64];
				MCommandParameterUInt(dwIP).GetData(pData, 64);
				pParamIP->SetData(pData);
				MCommandParameterUInt(nPort).GetData(pData, 64);
				pParamPort->SetData(pData);

				PostSafeQueue(pCmd);
			}
		}
	}
	break;
	case MSGID_COMMAND:
	{
		_ASSERT(0);
		// 서버상에 암호화된 UDP는 사용하지 않음
		Log(LOG_DEBUG, "MMatchServer::ParseUDPPacket: Parse Packet Error");
	}
	break;
	default:
	{
		_ASSERT(0);
		Log(LOG_DEBUG, "MMatchServer::ParseUDPPacket: Parse Packet Error");
	}

	break;
	}
}

void MMatchServer::ResponseBridgePeer(const MUID& uidChar, int nCode)
{
	MMatchObject* pObj = GetObject(uidChar);
	if (pObj == NULL) return;

	MCommand* pNew = CreateCommand(MC_MATCH_BRIDGEPEER_ACK, MUID(0, 0));
	pNew->AddParameter(new MCmdParamUID(uidChar));
	pNew->AddParameter(new MCmdParamInt(nCode));
	RouteToListener(pObj, pNew);
}

// 난입한 유저가 방안에 있는 다른 사람들 정보 달라고 요청했을때 방안의 유저정보를 알려준다
void MMatchServer::ResponsePeerList(const MUID& uidChar, const MUID& uidStage)
{
	MMatchStage* pStage = FindStage(uidStage);
	if (pStage == NULL) return;
	MMatchObject* pObj = GetObject(uidChar);
	if (pObj == NULL) return;

	MCommand* pNew = CreateCommand(MC_MATCH_RESPONSE_PEERLIST, MUID(0, 0));
	pNew->AddParameter(new MCommandParameterUID(pStage->GetUID()));

	// Battle에 들어간 사람만 List를 만든다.
	int nPeerCount = pStage->GetObjInBattleCount() + pStage->Bots.size();

	void* pPeerArray = MMakeBlobArray(sizeof(MTD_PeerListNode), nPeerCount);
	int nIndex = 0;
	for (auto itor = pStage->GetObjBegin(); itor != pStage->GetObjEnd(); itor++) {
		MMatchObject* pObj = itor->second;
		if (pObj->GetEnterBattle() == false) continue;

		MTD_PeerListNode* pNode = (MTD_PeerListNode*)MGetBlobArrayElement(pPeerArray, nIndex++);
		pNode->uidChar = pObj->GetUID();

		pNode->dwIP = pObj->GetIP();
		//		strcpy_safe(pNode->szIP, pObj->GetIP());
		pNode->nPort = pObj->GetPort();

		CopyCharInfoForTrans(&pNode->CharInfo, pObj->GetCharInfo(), pObj);

		memset(&pNode->ExtendInfo, 0, sizeof(MTD_ExtendInfo));
		if (pStage->GetStageSetting()->IsTeamPlay())
			pNode->ExtendInfo.nTeam = (char)pObj->GetTeam();
		else
			pNode->ExtendInfo.nTeam = 0;
		pNode->ExtendInfo.nPlayerFlags = pObj->GetPlayerFlags();
	}

	for (auto&& Bot : pStage->Bots)
	{
		auto* Node = static_cast<MTD_PeerListNode*>(MGetBlobArrayElement(pPeerArray, nIndex++));
		*Node = Bot.PeerListNode;
		if (uidChar == Bot.OwnerUID)
		{
			Node->dwIP = u32(-1);
			Node->nPort = 0;
		}
	}

	pNew->AddParameter(new MCommandParameterBlob(pPeerArray, MGetBlobArraySize(pPeerArray)));
	MEraseBlobArray(pPeerArray);

	RouteToListener(pObj, pNew);
}


bool MMatchServer::CheckBridgeFault()
{
	for (MMatchObjectList::iterator i = m_Objects.begin(); i != m_Objects.end(); i++) {
		MMatchObject* pObj = (*i).second;
		if (pObj->GetBridgePeer() == false)
			return true;
	}
	return false;
}




void MMatchServer::OnUserWhisper(const MUID& uidComm, char* pszSenderName, char* pszTargetName, char* pszMessage)
{
	if (strlen(pszSenderName) < 2) return;
	if (strlen(pszTargetName) < 2) return;

	MMatchObject* pObj = GetPlayerByCommUID(uidComm);
	if (pObj == NULL) return;

	MMatchObject* pTargetObj = GetPlayerByName(pszTargetName);
	if (pTargetObj == NULL) {
		NotifyMessage(pObj->GetUID(), MATCHNOTIFY_GENERAL_USER_NOTFOUND);
		return;
	}
	if (pTargetObj->CheckUserOption(MBITFLAG_USEROPTION_REJECT_WHISPER) == true) {
		NotifyMessage(pObj->GetUID(), MATCHNOTIFY_USER_WHISPER_REJECTED);
		//		NotifyMessage(pTargetObj->GetUID(), MATCHNOTIFY_USER_WHISPER_IGNORED);
		return;
	}

	MCommand* pCmd = CreateCommand(MC_MATCH_USER_WHISPER, MUID(0, 0));
	pCmd->AddParameter(new MCmdParamStr(pObj->GetName()));
	pCmd->AddParameter(new MCmdParamStr(pszTargetName));
	pCmd->AddParameter(new MCmdParamStr(pszMessage));
	RouteToListener(pTargetObj, pCmd);
}

void MMatchServer::OnUserWhere(const MUID& uidComm, char* pszTargetName)
{
	if (strlen(pszTargetName) < 2) return;

	MMatchObject* pObj = GetPlayerByCommUID(uidComm);
	if (!IsEnabledObject(pObj)) return;

	MMatchObject* pTargetObj = GetPlayerByName(pszTargetName);
	if (pTargetObj == NULL) {
		NotifyMessage(pObj->GetUID(), MATCHNOTIFY_GENERAL_USER_NOTFOUND);
		return;
	}

	if ((IsAdminGrade(pObj) == false) && (IsAdminGrade(pTargetObj) == true)) {
		NotifyMessage(pObj->GetUID(), MATCHNOTIFY_GENERAL_USER_NOTFOUND);
		return;
	}

	char szLog[256] = "";

	bool bUnknownChannel = true;

	MMatchChannel* pChannel = FindChannel(pTargetObj->GetChannelUID());
	if (pChannel) {
		if (pTargetObj->GetPlace() == MMP_LOBBY)
		{
			bUnknownChannel = false;
			sprintf_safe(szLog, "[%s] '%s'",
				pTargetObj->GetName(),
				pChannel->GetName());
		}
		else if ((pTargetObj->GetPlace() == MMP_STAGE) || (pTargetObj->GetPlace() == MMP_BATTLE))
		{
			MMatchStage* pStage = FindStage(pTargetObj->GetStageUID());
			if (0 != pStage)
			{
				bUnknownChannel = false;
				sprintf_safe(szLog, "[%s] '%s' , '(%d)%s'",
					pTargetObj->GetName(),
					pChannel->GetName(),
					pStage->GetIndex() + 1,
					pStage->GetName());
			}
		}
	}

	if (bUnknownChannel)
		sprintf_safe(szLog, "%s , Unknown Channel", pTargetObj->GetName());

	Announce(pObj, szLog);
}

void MMatchServer::OnUserOption(const MUID& uidComm, u32 nOptionFlags)
{
	MMatchObject* pObj = GetPlayerByCommUID(uidComm);
	if (pObj == NULL) return;

	pObj->SetUserOption(nOptionFlags);
}

void MMatchServer::OnChatRoomCreate(const MUID& uidPlayer, const char* pszChatRoomName)
{
	MMatchChatRoomMgr* pChatRoomMgr = GetChatRoomMgr();
	MMatchChatRoom* pRoom = pChatRoomMgr->FindChatRoomByName(pszChatRoomName);
	if (pRoom) {
		NotifyMessage(uidPlayer, MATCHNOTIFY_CHATROOM_ALREADY_EXIST);	// Notify Already Exist
		return;
	}

	pRoom = pChatRoomMgr->AddChatRoom(uidPlayer, pszChatRoomName);
	if (pRoom == NULL) {
		NotifyMessage(uidPlayer, MATCHNOTIFY_CHATROOM_CREATE_FAILED);	// Notify Can't Create
		return;
	}

	if (pRoom->AddPlayer(uidPlayer) == true) {
		LOG(LOG_PROG, "ChatRoom Created : '%s' ", pszChatRoomName);
		NotifyMessage(uidPlayer, MATCHNOTIFY_CHATROOM_CREATE_SUCCEED);
	}
	else {
		NotifyMessage(uidPlayer, MATCHNOTIFY_CHATROOM_JOIN_FAILED);		// Notify Join Failed
	}
}

void MMatchServer::OnChatRoomJoin(const MUID& uidComm, char* pszPlayerName, char* pszChatRoomName)
{
	MMatchChatRoomMgr* pChatRoomMgr = GetChatRoomMgr();
	MMatchChatRoom* pRoom = pChatRoomMgr->FindChatRoomByName(pszChatRoomName);
	if (pRoom == NULL) return;

	MMatchObject* pObj = GetPlayerByCommUID(uidComm);
	if (pObj == NULL) return;

	if (pRoom->GetUserCount() > CHATROOM_MAX_ROOMMEMBER) {
		NotifyMessage(uidComm, MATCHNOTIFY_CHATROOM_USER_FULL);			// Notify Full Member
		return;
	}

	if (pRoom->AddPlayer(uidComm)) {
		// Notify Joinning to Participant
		MCommand* pCmd = CreateCommand(MC_MATCH_CHATROOM_JOIN, MUID(0, 0));
		pCmd->AddParameter(new MCmdParamStr(pObj->GetName()));
		pCmd->AddParameter(new MCmdParamStr(pszChatRoomName));
		pRoom->RouteCommand(pCmd);
	}
	else {
		NotifyMessage(uidComm, MATCHNOTIFY_CHATROOM_JOIN_FAILED);		// Notify Join a room Failed
	}
}

void MMatchServer::OnChatRoomLeave(const MUID& uidComm, char* pszPlayerName, char* pszChatRoomName)
{
	MMatchChatRoomMgr* pChatRoomMgr = GetChatRoomMgr();
	MMatchChatRoom* pRoom = pChatRoomMgr->FindChatRoomByName(pszChatRoomName);
	if (pRoom == NULL)
		return;

	MMatchObject* pObj = GetPlayerByCommUID(uidComm);
	if (pObj == NULL) return;

	pRoom->RemovePlayer(uidComm);

	// Notify to Player and Participant
	MCommand* pCmd = CreateCommand(MC_MATCH_CHATROOM_LEAVE, MUID(0, 0));
	pCmd->AddParameter(new MCmdParamStr(pObj->GetName()));
	pCmd->AddParameter(new MCmdParamStr(pszChatRoomName));
	pRoom->RouteCommand(pCmd);
}

void MMatchServer::OnChatRoomSelectWrite(const MUID& uidComm, const char* pszChatRoomName)
{
	MMatchObject* pPlayer = GetObject(uidComm);
	if (pPlayer == NULL) return;

	MMatchChatRoomMgr* pChatRoomMgr = GetChatRoomMgr();
	MMatchChatRoom* pRoom = pChatRoomMgr->FindChatRoomByName(pszChatRoomName);
	if (pRoom == NULL) {
		NotifyMessage(uidComm, MATCHNOTIFY_CHATROOM_NOT_EXIST);		// Notify Does not Exist
		return;
	}

	pPlayer->SetChatRoomUID(pRoom->GetUID());
}

void MMatchServer::OnChatRoomInvite(const MUID& uidComm, const char* pszTargetName)
{
	if (strlen(pszTargetName) < 2) return;

	MMatchObject* pPlayer = GetPlayerByCommUID(uidComm);
	if (pPlayer == NULL) return;

	MMatchChatRoomMgr* pChatRoomMgr = GetChatRoomMgr();
	MMatchChatRoom* pRoom = pChatRoomMgr->FindChatRoom(pPlayer->GetChatRoomUID());
	if (pRoom == NULL) {
		NotifyMessage(uidComm, MATCHNOTIFY_CHATROOM_NOT_EXIST);		// Notify Does not Exist
		return;
	}

	MMatchObject* pTargetObj = GetPlayerByName(pszTargetName);
	if (pTargetObj == NULL) {
		NotifyMessage(pPlayer->GetUID(), MATCHNOTIFY_GENERAL_USER_NOTFOUND);
		return;
	}

	if (pTargetObj->CheckUserOption(MBITFLAG_USEROPTION_REJECT_INVITE) == true) {
		NotifyMessage(pPlayer->GetUID(), MATCHNOTIFY_USER_INVITE_REJECTED);
		NotifyMessage(pTargetObj->GetUID(), MATCHNOTIFY_USER_INVITE_IGNORED);
		return;
	}

	MCommand* pCmd = CreateCommand(MC_MATCH_CHATROOM_INVITE, MUID(0, 0));
	pCmd->AddParameter(new MCmdParamStr(pPlayer->GetName()));
	pCmd->AddParameter(new MCmdParamStr(const_cast<char*>(pszTargetName)));
	pCmd->AddParameter(new MCmdParamStr(const_cast<char*>(pRoom->GetName())));
	RouteToListener(pTargetObj, pCmd);

}

// RAONHAJE 임시코드
#ifdef _DEBUG
#include "CMLexicalAnalyzer.h"
bool StageFinish(MMatchServer* pServer, const MUID& uidPlayer, char* pszChat)
{
	MMatchObject* pChar = pServer->GetObject(uidPlayer);
	if (pChar == NULL)	return false;
	//		if (pChar->GetPlace() != MMP_LOBBY) return false;
	MMatchStage* pStage = pServer->FindStage(pChar->GetStageUID());
	if (pStage == NULL) return false;

	bool bResult = false;
	CMLexicalAnalyzer lex;
	lex.Create(pszChat);

	if (lex.GetCount() >= 1) {
		char* pszCmd = lex.GetByStr(0);
		if (pszCmd) {
			if (_stricmp(pszCmd, "/finish") == 0) {
				pStage->GetRule()->DebugTest();
				bResult = true;
			}	// Finish
		}
	}

	lex.Destroy();
	return bResult;
}
#endif

void MMatchServer::OnChatRoomChat(const MUID& uidComm, const char* pszMessage)
{
	MMatchObject* pPlayer = GetObject(uidComm);
	if (pPlayer == NULL) return;

#ifdef _DEBUG
	if (StageFinish(this, uidComm, const_cast<char*>(pszMessage)))
		return;
#endif

	if (pPlayer->GetChatRoomUID() == MUID(0, 0)) {
		NotifyMessage(uidComm, MATCHNOTIFY_CHATROOM_NOT_USING);		// Notify No ChatRoom
		return;
	}

	MMatchChatRoomMgr* pChatRoomMgr = GetChatRoomMgr();
	MMatchChatRoom* pRoom = pChatRoomMgr->FindChatRoom(pPlayer->GetChatRoomUID());
	if (pRoom == NULL) {
		NotifyMessage(uidComm, MATCHNOTIFY_CHATROOM_NOT_EXIST);		// Notify Does not Exist
		return;
	}

	pRoom->RouteChat(pPlayer->GetUID(), const_cast<char*>(pszMessage));
}

void MMatchServer::DisconnectObject(const MUID& uidObject)
{
	MMatchObject* pObj = GetObject(uidObject);
	if (pObj == NULL) return;

	Disconnect(pObj->GetCommListener());
}



void MMatchServer::InsertChatDBLog(const MUID& uidPlayer, const char* szMsg)
{
	MMatchObject* pObj = GetObject(uidPlayer);
	if (pObj == NULL) return;
	auto nNowTime = GetGlobalTimeMS();

	static int stnLogTop = 0;
#define MAX_CHAT_LOG 1

	static struct MCHATLOG
	{
		u32 nCID;
		char szMsg[256];
		u64 nTime;
	} stChatLog[MAX_CHAT_LOG];

	stChatLog[stnLogTop].nCID = pObj->GetCharInfo()->m_nCID;
	if (strlen(szMsg) < 256) strcpy_safe(stChatLog[stnLogTop].szMsg, szMsg); else strcpy_safe(stChatLog[stnLogTop].szMsg, "");
	stChatLog[stnLogTop].nTime = GetGlobalTimeMS();
	stnLogTop++;

	// 일정 개수가 될때만 DB에 넣는다.
	if (stnLogTop >= MAX_CHAT_LOG)
	{
		for (int i = 0; i < stnLogTop; i++)
		{

			if (!GetDBMgr()->InsertChatLog(stChatLog[i].nCID, stChatLog[i].szMsg, stChatLog[i].nTime))
			{
				LOG(LOG_ALL, "DB Query(InsertChatDBLog > InsertChatLog) Failed");
			}
		}
		stnLogTop = 0;
	}
}



int MMatchServer::ValidateMakingName(const char* szCharName, int nMinLength, int nMaxLength)
{
	int nNameLen = (int)strlen(szCharName);

	if (nNameLen < nMinLength)
		return MERR_TOO_SHORT_NAME;

	if (nNameLen > nMaxLength)
		return MERR_TOO_LONG_NAME;

	if (!std::regex_match(std::string(szCharName), std::regex("^([0-9a-zA-Z\\-\\._]+)$"))) {
		return MERR_WRONG_WORD_NAME;
	}

	if (std::regex_search(std::string(szCharName), std::regex("(D.?e.?v.?e.?l.?o.?p.?e.?r|A.?d.?m.?i.?n|M.?o.?d.?e.?r.?a.?t.?o.?r|G.?a.?m.?e.?M.?a.?s.?t.?e.?r|M2O|M20)", std::regex_constants::icase))) {
		return MERR_WRONG_WORD_NAME;
	}

	return MOK;
}

int MMatchServer::ValidateStageJoin(const MUID& uidPlayer, const MUID& uidStage)
{
	MMatchObject* pObj = GetObject(uidPlayer);
	if (!IsEnabledObject(pObj)) return MERR_CANNOT_JOIN_STAGE;

	MMatchStage* pStage = FindStage(uidStage);
	if (pStage == NULL) return MERR_CANNOT_JOIN_STAGE;

	if (pStage->GetState() == STAGE_STATE_CLOSE) return MERR_CANNOT_JOIN_STAGE;

	if (!IsAdminGrade(pObj))
	{
		if (pStage->GetStageSetting()->GetMaxPlayers() <= pStage->GetCountableObjCount())
		{
			return MERR_CANNOT_JOIN_STAGE_BY_MAXPLAYERS;
		}

		if (pStage->GetStageSetting()->GetLimitLevel() != 0)
		{
			int nMasterLevel, nLimitLevel;
			MMatchObject* pMaster = GetObject(pStage->GetMasterUID());

			if (IsEnabledObject(pMaster))
			{
				nMasterLevel = pMaster->GetCharInfo()->m_nLevel;
				nLimitLevel = pStage->GetStageSetting()->GetLimitLevel();
				if (abs(pObj->GetCharInfo()->m_nLevel - nMasterLevel) > nLimitLevel)
				{
					return MERR_CANNOT_JOIN_STAGE_BY_LEVEL;
				}
			}
		}

		if ((pStage->GetStageSetting()->GetForcedEntry() == false) &&
			(pStage->GetState() != STAGE_STATE_STANDBY))
		{
			return MERR_CANNOT_JOIN_STAGE_BY_FORCEDENTRY;
		}

		// Ban Check
		if (pStage->CheckBanList(pObj->GetCharInfo()->m_nCID))
			return MERR_CANNOT_JOIN_STAGE_BY_BAN;
	}

	return MOK;
}

int MMatchServer::ValidateChannelJoin(const MUID& uidPlayer, const MUID& uidChannel)
{
	MMatchObject* pObj = GetObject(uidPlayer);
	if (!IsEnabledObject(pObj)) return MERR_CANNOT_JOIN_CHANNEL;
	MMatchChannel* pChannel = FindChannel(uidChannel);
	if (pChannel == NULL) return MERR_CANNOT_JOIN_CHANNEL;

	if (!IsAdminGrade(pObj))
	{
		if ((int)pChannel->GetObjCount() >= pChannel->GetMaxPlayers())
		{
			return MERR_CANNOT_JOIN_CHANNEL_BY_MAXPLAYERS;
		}

		if ((pChannel->GetLevelMin() > 0) &&
			(pChannel->GetLevelMin() > pObj->GetCharInfo()->m_nLevel))
		{
			return MERR_CANNOT_JOIN_CHANNEL_BY_LEVEL;
		}
		if ((pChannel->GetLevelMax() > 0) &&
			(pChannel->GetLevelMax() < pObj->GetCharInfo()->m_nLevel))
		{
			return MERR_CANNOT_JOIN_CHANNEL_BY_LEVEL;
		}

		if ((pChannel->GetRuleType() == MCHANNEL_RULE_NEWBIE) && (pObj->IsNewbie() == false))
		{
			return MERR_CANNOT_JOIN_CHANNEL_BY_NEWBIE;
		}
	}

	return MOK;
}


int MMatchServer::ValidateEquipItem(MMatchObject* pObj, MMatchItem* pItem, const MMatchCharItemParts parts)
{
	if (!IsEnabledObject(pObj)) return MERR_UNKNOWN;
	if (pItem == NULL) return MERR_UNKNOWN;

	if (!IsEquipableItem(pItem->GetDescID(), pObj->GetCharInfo()->m_nLevel, pObj->GetCharInfo()->m_nSex))
	{
		return MERR_LOW_LEVEL;
	}

	// 무게 체크
	int nWeight = 0;
	int nMaxWeight = 0;

	MMatchEquipedItem* pEquipedItem = &pObj->GetCharInfo()->m_EquipedItem;
	pObj->GetCharInfo()->GetTotalWeight(&nWeight, &nMaxWeight);

	// 교체할 아이템의 무게를 뺀다.
	if (!pEquipedItem->IsEmpty(parts))
	{
		if (pEquipedItem->GetItem(parts)->GetDesc() != NULL)
		{
			nWeight -= pEquipedItem->GetItem(parts)->GetDesc()->m_nWeight;
			nMaxWeight -= pEquipedItem->GetItem(parts)->GetDesc()->m_nMaxWT;
		}
	}

	// 장착할 아이템의 무게를 더한다.
	if (pItem->GetDesc() != NULL)
	{
		nWeight += pItem->GetDesc()->m_nWeight;
	}

	if (nWeight > nMaxWeight)
	{
		return MERR_TOO_HEAVY;
	}


	// checking if same item is equipped twice in primary or secondary slots 
	
	if ((parts == MMCIP_PRIMARY) || (parts == MMCIP_SECONDARY))
	{
		MMatchCharItemParts tarparts = MMCIP_PRIMARY;
		if (parts == MMCIP_PRIMARY) tarparts = MMCIP_SECONDARY;

		if (!pEquipedItem->IsEmpty(tarparts))
		{
			MMatchItem* pTarItem = pEquipedItem->GetItem(tarparts);
			if (pTarItem)
			{
				if (pTarItem->GetDescID() == pItem->GetDescID())
				{
					return MERR_CANNOT_EQUIP_EQUAL_ITEM;
				}
			}
		}
	}

	return MOK;
}

void MMatchServer::OnNetClear(const MUID& CommUID)
{
	MMatchObject* pObj = GetObject(CommUID);
	if (pObj)
		OnCharClear(pObj->GetUID());

	MAgentObject* pAgent = GetAgent(CommUID);
	if (pAgent)
		AgentRemove(pAgent->GetUID(), NULL);

	MServer::OnNetClear(CommUID);
}

void MMatchServer::OnNetPong(const MUID& CommUID, unsigned int nTimeStamp)
{
	MMatchObject* pObj = GetObject(CommUID);
	if (pObj) {
		pObj->UpdateTickLastPacketRecved();
		pObj->AddPing(static_cast<int>(GetGlobalClockCount() - nTimeStamp));
	}
}

void MMatchServer::UpdateCharDBCachingData(MMatchObject* pObject)
{
	if (!IsEnabledObject(pObject)) return;

	int	nAddedXP, nAddedBP, nAddedKillCount, nAddedDeathCount;

	nAddedXP = pObject->GetCharInfo()->GetDBCachingData()->nAddedXP;
	nAddedBP = pObject->GetCharInfo()->GetDBCachingData()->nAddedBP;
	nAddedKillCount = pObject->GetCharInfo()->GetDBCachingData()->nAddedKillCount;
	nAddedDeathCount = pObject->GetCharInfo()->GetDBCachingData()->nAddedDeathCount;

	if ((nAddedXP != 0) || (nAddedBP != 0) || (nAddedKillCount != 0) || (nAddedDeathCount != 0))
	{
		MAsyncDBJob_UpdateCharInfoData* pJob = new MAsyncDBJob_UpdateCharInfoData();
		pJob->Input(pObject->GetCharInfo()->m_nCID,
			nAddedXP,
			nAddedBP,
			nAddedKillCount,
			nAddedDeathCount);
		PostAsyncJob(pJob);

		// 실패했는지는 알 수 없지만, 악용을 위해 Reset한다.
		pObject->GetCharInfo()->GetDBCachingData()->Reset();

		/*
				if (GetDBMgr()->UpdateCharInfoData(pObject->GetCharInfo()->m_nCID,
					nAddedXP, nAddedBP, nAddedKillCount, nAddedDeathCount))
				{
					pObject->GetCharInfo()->GetDBCachingData()->Reset();
					st_ErrCounter = 0;
				}
				else
				{
					Log(LOG_ALL, "DB Query(UpdateCharDBCachingData > UpdateCharInfoData) Failed\n");

					LOG(LOG_ALL, "[CRITICAL ERROR] DB Connection Lost. ");

					GetDBMgr()->Disconnect();
					InitDB();

					st_ErrCounter++;
					if (st_ErrCounter > MAX_DB_QUERY_COUNT_OUT)
					{
						LOG(LOG_ALL, "[CRITICAL ERROR] UpdateCharInfoData - Shutdown");
						Shutdown();
					}
				}
		*/
	}
}

// item xml 체크용 - 테스트
bool MMatchServer::CheckItemXML()
{
	map<u32, string>	ItemXmlMap;

	MXmlDocument	xmlIniData;

	xmlIniData.Create();

	if (!xmlIniData.LoadFromFile(FILENAME_ITEM_DESC))
	{
		xmlIniData.Destroy();
		return false;
	}

	MXmlElement rootElement, chrElement, attrElement;
	char szTagName[256];

	rootElement = xmlIniData.GetDocumentElement();
	int iCount = rootElement.GetChildNodeCount();

	for (int i = 0; i < iCount; i++)
	{
		chrElement = rootElement.GetChildNode(i);
		chrElement.GetTagName(szTagName);
		if (szTagName[0] == '#') continue;

		if (!_stricmp(szTagName, MICTOK_ITEM))
		{
			u32 id;
			int n;
			char szItemName[256];
			chrElement.GetAttribute(&n, MICTOK_ID);
			id = n;
			chrElement.GetAttribute(szItemName, MICTOK_NAME);

			if (ItemXmlMap.find(id) != ItemXmlMap.end())
			{
				_ASSERT(0);	// 아이템 ID 중복
				char szTemp[256];
				sprintf_safe(szTemp, "item xml 아이디 중복: %u\n", id);
				mlog(szTemp);
				return false;
			}
			ItemXmlMap.insert(map<u32, string>::value_type(id, string(szItemName)));
		}
	}

	xmlIniData.Destroy();

	FILE* fp = fopen("item.sql", "wt");
	if (!fp)
	{
		MLog("Failed to open item.sql\n");
		return false;
	}

	for (map<u32, string>::iterator itor = ItemXmlMap.begin();
		itor != ItemXmlMap.end(); ++itor)
	{
		char szTemp2[256];
		u32 id = (*itor).first;
		size_t pos = (*itor).second.find(":");
		if (string::npos == pos)
		{
			// TODO: Fix
			//ASSERT( 0 && "구분자를 찾지 못함. 문법오류." );
			continue;
		}

		string name = (*itor).second.c_str() + pos + 1;

		if (0 == _stricmp("nomsg", MGetStringResManager()->GetString(name)))
			mlog("Item : %s\n", name.c_str());

		sprintf_safe(szTemp2, "INSERT INTO Item (ItemID, Name) Values (%u, '%s')\n", // id, name.c_str() );
			id, MGetStringResManager()->GetString(name));

		fputs(szTemp2, fp);
	}

	fputs("\n\n--------------------------------------\n\n", fp);

	for (MMatchItemDescMgr::iterator itor = MGetMatchItemDescMgr()->begin();
		itor != MGetMatchItemDescMgr()->end(); ++itor)
	{
		MMatchItemDesc* pItemDesc = (*itor).second;

		int nIsCashItem = 0;
		int nResSex = 1, nResLevel = 0, nSlot = 0, nWeight = 0, nHP = 0, nAP = 0, nMaxWT = 0;

		int nDamage = 0, nDelay = 0, nControl = 0, nMagazine = 0, nReloadTime = 0, nSlugOutput = 0, nMaxBullet = 0;

		if (pItemDesc->IsCashItem()) nIsCashItem = 1;
		switch (pItemDesc->m_nResSex)
		{
		case 0: nResSex = 1; break;
		case 1: nResSex = 2; break;
		case -1: nResSex = 3; break;
		}

		nResLevel = pItemDesc->m_nResLevel;
		nWeight = pItemDesc->m_nWeight;
		nHP = pItemDesc->m_nHP;
		nAP = pItemDesc->m_nAP;
		nMaxWT = pItemDesc->m_nMaxWT;

		switch (pItemDesc->m_nSlot)
		{
		case MMIST_MELEE: nSlot = 1; break;
		case MMIST_RANGE: nSlot = 2; break;
		case MMIST_CUSTOM: nSlot = 3; break;
		case MMIST_HEAD: nSlot = 4; break;
		case MMIST_CHEST: nSlot = 5; break;
		case MMIST_HANDS: nSlot = 6; break;
		case MMIST_LEGS: nSlot = 7; break;
		case MMIST_FEET: nSlot = 8; break;
		case MMIST_FINGER: nSlot = 9; break;
		case MMIST_EXTRA: nSlot = 9; break;
		}


		nDamage = pItemDesc->m_nDamage;
		nDelay = pItemDesc->m_nDelay;
		nControl = pItemDesc->m_nControllability;
		nMagazine = pItemDesc->m_nMagazine;
		nReloadTime = pItemDesc->m_nReloadTime;
		if (pItemDesc->m_bSlugOutput) nSlugOutput = 1;
		nMaxBullet = pItemDesc->m_nMaxBullet;

		fprintf(fp, "UPDATE Item SET TotalPoint=0, BountyPrice=0, Damage=%d, Delay=%d, Controllability=%d, Magazine=%d, ReloadTime=%d, SlugOutput=%d, Gadget=0, SF=0, FR=0,CR=0,PR=0,LR=0, BlendColor=0, ModelName='', MaxBullet=%d, LimitSpeed=%d, IsCashItem=%d, \n",
			nDamage, nDelay, nControl, nMagazine, nReloadTime, nSlugOutput, nMaxBullet,
			pItemDesc->m_nLimitSpeed, nIsCashItem);

		fprintf(fp, "ResSex=%d, ResLevel=%d, Slot=%d, Weight=%d, HP=%d, AP=%d, MAXWT=%d, \n",
			nResSex, nResLevel, nSlot, nWeight, nHP, nAP, nMaxWT);

		fprintf(fp, "Description='%s' \n", pItemDesc->m_szDesc);

		// 이거 절대로 지우지 마세요. DB작업할때 대형 사고 날수 있습니다. - by SungE.
		fprintf(fp, "WHERE ItemID = %u\n", pItemDesc->m_nID);

		/*
		fprintf(fp, "UPDATE Item SET Slot = %d WHERE ItemID = %u AND Slot IS NULL\n", nSlot, pItemDesc->m_nID );
		*/
	}



	fclose(fp);

	return true;

}

// sql파일 생성을 위해서. 게임을 위해서 사용되지는 않음.
struct ix
{
	string id;
	string name;
	string desc;
};

bool MMatchServer::CheckUpdateItemXML()
{
	// map<u32, string>	ItemXmlMap;

	map< string, ix > imName;
	map< string, ix > imDesc;

	MXmlDocument	xmlIniData;

	xmlIniData.Create();

	if (!xmlIniData.LoadFromFile("strings.xml"))
	{
		xmlIniData.Destroy();
		return false;
	}

	MXmlElement rootElement, chrElement, attrElement;
	char szTagName[256];

	rootElement = xmlIniData.GetDocumentElement();
	int iCount = rootElement.GetChildNodeCount();

	for (int i = 0; i < iCount; i++)
	{
		chrElement = rootElement.GetChildNode(i);
		chrElement.GetTagName(szTagName);
		if (szTagName[0] == '#') continue;

		if (!_stricmp(szTagName, "STR"))
		{
			ix tix;
			char szID[256] = { 0, };
			char szInfo[512] = { 0, };

			chrElement.GetAttribute(szID, "id");
			chrElement.GetContents(szInfo);

			if (0 == strncmp("ZITEM_NAME_", szID, 11))
			{
				if (imName.end() != imName.find(szID))
				{
					ASSERT("중복");
					continue;
				}

				tix.id = szID;
				tix.name = szInfo;

				imName.insert(map<string, ix>::value_type(szID, tix));
			}
			else if (0 == strncmp("ZITEM_DESC_", szID, 11))
			{
				if (imDesc.end() != imDesc.find(szID))
				{
					ASSERT("중복");
					continue;
				}

				tix.id = szID;
				tix.desc = szInfo;

				imDesc.insert(map<string, ix>::value_type(szID, tix));
			}
			else
			{
				// ASSERT( 0 && "이상하다...." );
			}
		}
	}

	int ic, dc;
	ic = static_cast<int>(imName.size());
	dc = static_cast<int>(imDesc.size());

	xmlIniData.Destroy();

	map< string, ix >::iterator it, end;
	it = imName.begin();
	end = imName.end();
	FILE* fpName = fopen("name.sql", "w");
	for (; it != end; ++it)
	{
		char szID[128];
		string a = it->second.name;
		strcpy_safe(szID, it->second.id.c_str() + 11);

		auto nID = StringToInt<u32>(szID).value_or(0);
		int k = 0;

		fprintf(fpName, "UPDATE Item SET Name = '%s' WHERE ItemID = %u\n",
			it->second.name.c_str(), nID);
	}
	fclose(fpName);

	it = imDesc.begin();
	end = imDesc.end();
	FILE* fpDesc = fopen("desc.sql", "w");
	if (!fpDesc)
	{
		MLog("Failed to open desc.spl\n");
		return false;
	}

	for (; it != end; ++it)
	{
		char szID[128];
		string a = it->second.name;
		strcpy_safe(szID, it->second.id.c_str() + 11);

		auto nID = StringToInt<u32>(szID).value_or(0);

		fprintf(fpDesc, "UPDATE Item SET Description = '%s' WHERE ItemID = %u\n",
			it->second.desc.c_str(), nID);
	}
	fclose(fpDesc);

	return true;
}


u32 MMatchServer::GetStageListChecksum(MUID& uidChannel, int nStageCursor, int nStageCount)
{
	MMatchChannel* pChannel = FindChannel(uidChannel);
	if (pChannel == NULL) return 0;

	u32 nStageListChecksum = 0;
	int nRealStageCount = 0;

	for (int i = nStageCursor; i < pChannel->GetMaxPlayers(); i++)
	{
		if (nRealStageCount >= nStageCount) break;

		if (pChannel->IsEmptyStage(i)) continue;
		MMatchStage* pStage = pChannel->GetStage(i);
		if ((pStage == NULL) || (pStage->GetState() == STAGE_STATE_CLOSE)) continue;

		nStageListChecksum += pStage->GetChecksum();

		nRealStageCount++;
	}

	return nStageListChecksum;
}




void MMatchServer::BroadCastClanRenewVictories(const char* szWinnerClanName, const char* szLoserClanName, const int nVictories)
{
	MCommand* pCmd = CreateCommand(MC_MATCH_BROADCAST_CLAN_RENEW_VICTORIES, MUID(0, 0));
	pCmd->AddParameter(new MCommandParameterString(szWinnerClanName));
	pCmd->AddParameter(new MCommandParameterString(szLoserClanName));
	pCmd->AddParameter(new MCommandParameterInt(nVictories));

	RouteToAllClient(pCmd);
}

void MMatchServer::BroadCastClanInterruptVictories(const char* szWinnerClanName, const char* szLoserClanName, const int nVictories)
{
	MCommand* pCmd = CreateCommand(MC_MATCH_BROADCAST_CLAN_INTERRUPT_VICTORIES, MUID(0, 0));
	pCmd->AddParameter(new MCommandParameterString(szWinnerClanName));
	pCmd->AddParameter(new MCommandParameterString(szLoserClanName));
	pCmd->AddParameter(new MCommandParameterInt(nVictories));

	RouteToAllClient(pCmd);
}

void MMatchServer::BroadCastDuelRenewVictories(const MUID& chanID, const char* szChampionName, const char* szChannelName, int nRoomNumber, const int nVictories)
{
	MCommand* pCmd = CreateCommand(MC_MATCH_BROADCAST_DUEL_RENEW_VICTORIES, MUID(0, 0));
	pCmd->AddParameter(new MCommandParameterString(szChampionName));
	pCmd->AddParameter(new MCommandParameterString(szChannelName));
	pCmd->AddParameter(new MCommandParameterInt(nRoomNumber));
	pCmd->AddParameter(new MCommandParameterInt(nVictories));

	RouteToChannel(chanID, pCmd);
}

void MMatchServer::BroadCastDuelInterruptVictories(const MUID& chanID, const char* szChampionName, const char* szInterrupterName, const int nVictories)
{
	MCommand* pCmd = CreateCommand(MC_MATCH_BROADCAST_DUEL_INTERRUPT_VICTORIES, MUID(0, 0));
	pCmd->AddParameter(new MCommandParameterString(szChampionName));
	pCmd->AddParameter(new MCommandParameterString(szInterrupterName));
	pCmd->AddParameter(new MCommandParameterInt(nVictories));

	RouteToChannel(chanID, pCmd);
}


bool MMatchServer::InitScheduler()
{
	// 스케쥴 업데이트시 커멘드를 포스트하기 위해서,
	//  MMatchServer의 주소를 인자로 받아 멤버로 저장해둠.
	m_pScheduler = new MMatchScheduleMgr(this);
	if (0 == m_pScheduler)
		return false;

	if (!m_pScheduler->Init()) {
		delete m_pScheduler;
		m_pScheduler = 0;
		return false;
	}

	// 검사 시간을 10초로 설정. 임시.
	m_pScheduler->SetUpdateTerm(10);

	// 상속한 클래스의 스케쥴 등록.
	if (!InitSubTaskSchedule()) {
		delete m_pScheduler;
		m_pScheduler = 0;
		return false;
	}

	return true;
}


bool MMatchServer::InitLocale()
{
	if (MGetServerConfig()->IsComplete())
	{

		MGetLocale()->Init(GetCountryID(MGetServerConfig()->GetLanguage().c_str()));
	}
	else
	{
		ASSERT(0 && "'MMatchConfig' is must be completed befor init 'MMatchLocale'.");
		return false;
	}

	MGetStringResManager()->Init("", MGetLocale()->GetCountry());

	return true;
}

bool MMatchServer::InitEvent()
{
	if (!MMatchEventDescManager::GetInstance().LoadEventXML(EVENT_XML_FILE_NAME))
	{
		ASSERT(0 && "fail to Load Event.xml");
		mlog("MMatchServer::InitEvent - fail to Load %s\n",
			EVENT_XML_FILE_NAME);
		return false;
	}

	if (!MMatchEventFactoryManager::GetInstance().LoadEventListXML(EVENT_LIST_XML_FILE_NAME))
	{
		ASSERT(0 && "fail to load EventList.xml");
		mlog("MMatchServer::InitEvent - fail to Load %s\n",
			EVENT_LIST_XML_FILE_NAME);
		return false;
	}

	MMatchEventFactoryManager::GetInstance().SetUsableState(MGetServerConfig()->IsUseEvent());

	EventPtrVec EvnPtrVec;
	if (!MMatchEventFactoryManager::GetInstance().GetEventList(MMATCH_GAMETYPE_ALL, ET_CUSTOM_EVENT, EvnPtrVec))
	{
		ASSERT(0 && "이벤트 리스트 생성 실패.\n");
		mlog("MMatchServer::InitEvent - 리스트 생성 실패.\n");
		MMatchEventManager::ClearEventPtrVec(EvnPtrVec);
		return false;
	}
	m_CustomEventManager.ChangeEventList(EvnPtrVec);

	return true;
}


void MMatchServer::CustomCheckEventObj(const u32 dwEventID, MMatchObject* pObj, void* pContext)
{
	m_CustomEventManager.CustomCheckEventObj(dwEventID, pObj, pContext);
}

void MMatchServer::PostDeath(const MMatchObject & Victim, const MMatchObject & Attacker)
{
	auto DeathCmd = MCommand(m_CommandManager.GetCommandDescByID(MC_PEER_DIE), MUID(0, 0), m_This);
	DeathCmd.AddParameter(new MCmdParamUID(Attacker.GetUID()));
	auto P2PCmd = CreateCommand(MC_MATCH_P2P_COMMAND, MUID(0, 0));
	P2PCmd->AddParameter(new MCmdParamUID(Victim.GetUID()));
	P2PCmd->AddParameter(CommandToBlob(DeathCmd));
	RouteToBattle(Victim.GetStageUID(), P2PCmd);
}

void MMatchServer::PostDamage(const MUID& Target, const MUID& Attacker, ZDAMAGETYPE DamageType, MMatchWeaponType WeaponType,
	int Damage, float PiercingRatio)
{
	MCommand* pCmd = CreateCommand(MC_MATCH_DAMAGE, Target);
	pCmd->AddParameter(new MCmdParamUID(Attacker));
	pCmd->AddParameter(new MCmdParamUShort(Damage));
	pCmd->AddParameter(new MCmdParamFloat(PiercingRatio));
	pCmd->AddParameter(new MCmdParamUChar(DamageType));
	pCmd->AddParameter(new MCmdParamUChar(WeaponType));
	Post(pCmd);
}

void MMatchServer::PostHPAPInfo(const MMatchObject& Object, int HP, int AP)
{
	auto DeathCmd = MCommand(m_CommandManager.GetCommandDescByID(MC_PEER_HPAPINFO), MUID(0, 0), m_This);
	DeathCmd.AddParameter(new MCmdParamFloat(static_cast<float>(HP)));
	DeathCmd.AddParameter(new MCmdParamFloat(static_cast<float>(AP)));
	auto P2PCmd = CreateCommand(MC_MATCH_P2P_COMMAND, MUID(0, 0));
	P2PCmd->AddParameter(new MCmdParamUID(Object.GetUID()));
	P2PCmd->AddParameter(CommandToBlob(DeathCmd));
	RouteToBattle(Object.GetStageUID(), P2PCmd);
}
