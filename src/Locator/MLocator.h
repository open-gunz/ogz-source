#pragma once

#include "MUID.h"
#include "MCommandManager.h"
#include "MSync.h"

class MCommand;
class MCommandManager;
class MLocatorDBMgr;
class MSafeUDP;
class MServerStatusMgr;
class MLocatorUDPInfo;
class MUDPManager;
class MCountryFilter;

struct MPacketHeader;

class MLocator
{
public:
	MLocator();
	virtual ~MLocator();

	bool Create();
	void Destroy();

	bool IsBlocker(const u32 dwIPKey, const u64 dwEventTime);
	bool IsDuplicatedUDP(const u32 dwIPKey,
		MUDPManager& rfCheckUDPManager,
		const u64 dwEventTime);

	void IncreaseRecvCount() { ++m_nRecvCount; }
	void IncreaseSendCount() { ++m_nSendCount; }
	void IncreaseDuplicatedCount() { ++m_nDuplicatedCount; }

	auto* GetServerStatusMgr() { return m_pServerStatusMgr; }
	auto* GetServerStatusMgr() const { return m_pServerStatusMgr; }

	void DumpLocatorStatusInfo();

	void Run();

public:
	MCommandManager* GetCommandManager() { return &m_CommandManager; }
	MCommand* GetCommandSafe() { return m_CommandManager.GetCommand(); }

	MCommand* CreateCommand(int nCmdID, const MUID& TargetUID);

#ifdef _DEBUG
	void TestDo();
	void InitDebug();
	void DebugOutput(void* vp);
#endif

private:
	bool InitDBMgr();
	bool InitSafeUDP();
	bool InitServerStatusMgr();
	bool InitUDPManager();

	MLocatorDBMgr* GetLocatorDBMgr() { return m_pDBMgr; }

	bool GetServerStatus();
	void GetDBServerStatus(u64 dwEventTime, const bool bIsWithoutDelayUpdate = false);
	auto GetUpdatedServerStatusTime() { return m_dwLastServerStatusUpdatedTime; }
	auto GetLastUDPManagerUpdateTime() { return m_dwLastUDPManagerUpdateTime; }
	auto GetLastLocatorStatusUpdatedTime() { return m_dwLastLocatorStatusUpdatedTime; }

	MUDPManager& GetRecvUDPManager() { return *m_pRecvUDPManager; }
	MUDPManager& GetSendUDPManager() { return *m_pSendUDPManager; }
	MUDPManager& GetBlockUDPManager() { return *m_pBlockUDPManager; }

	auto GetRecvCount() const { return m_nRecvCount; }
	auto GetSendCount() const { return m_nSendCount; }
	auto GetDuplicatedCount() const { return m_nDuplicatedCount; }

	void ResetRecvCount() { m_nRecvCount = 0; }
	void ResetSendCount() { m_nSendCount = 0; }
	void ResetDuplicatedCount() { m_nDuplicatedCount = 0; }

	void ReleaseDBMgr();
	void ReleaseSafeUDP();
	void ReleaseServerStatusMgr();
	void ReleaseServerStatusInfoBlob();
	void ReleaseUDPManager();
	void ReleaseCommand();

	bool IsElapedServerStatusUpdatedTime(u64 dwEventTime);
	void UpdateLastServerStatusUpdatedTime(u64 dwTime) { m_dwLastServerStatusUpdatedTime = dwTime; }
	void UpdateLastUDPManagerUpdateTime(u64 dwTime) { m_dwLastUDPManagerUpdateTime = dwTime; }
	void UpdateLastLocatorStatusUpdatedTime(u64 dwTime) { m_dwLastLocatorStatusUpdatedTime = dwTime; }

	void ParseUDPPacket(char* pData,
		MPacketHeader* pPacketHeader,
		u32 dwIP,
		unsigned int nPort);
	void PostSafeCommand(MCommand* pCmd);

	void CommandQueueLock() { m_csCommandQueueLock.lock(); }
	void CommandQueueUnlock() { m_csCommandQueueLock.unlock(); }

	void ResponseServerStatusInfoList(u32 dwIP, int nPort);
	void ResponseBlockCountryCodeIP(u32 dwIP,
		int nPort,
		const std::string& strCountryCode,
		const std::string& strRoutingURL);

	bool IsLiveUDP(const MLocatorUDPInfo* pRecvUDPInfo, u64 dwEventTime);
	bool IsLiveBlockUDP(const MLocatorUDPInfo* pBlkRecvUDPInfo, u64 dwEventTime);
	bool IsOverflowedNormalUseCount(const MLocatorUDPInfo* pRecvUDPInfo);

	const int	MakeCmdPacket(char* pOutPacket, const int nMaxSize, MCommand* pCmd);
	void		SendCommandByUDP(u32 dwIP, int nPort, MCommand* pCmd);

	void UpdateUDPManager(u64 dwEventTime);
	void FlushRecvQueue(u64 dwEventTime);
	void UpdateLocatorStatus(u64 dwEventTime);
	void UpdateLocatorLog(u64 dwEventTime);
	void UpdateCountryCodeFilter(u64 dwEventTime);
	void UpdateLogManager();

	void OnRegisterCommand(MCommandManager* pCommandManager);

	static bool UDPSocketRecvEvent(u32 dwIP,
		u16 wRawPort,
		char* pPacket,
		u32 dwSize);

private:
	MCommandManager	m_CommandManager;
	MUID			m_This{};

	MSafeUDP*			m_pSafeUDP{};
	MServerStatusMgr*	m_pServerStatusMgr{};

	u64 m_dwLastServerStatusUpdatedTime{};
	u64 m_dwLastLocatorStatusUpdatedTime{};

	MCriticalSection m_csCommandQueueLock{};

	MUDPManager* m_pRecvUDPManager{};
	MUDPManager* m_pSendUDPManager{};
	MUDPManager* m_pBlockUDPManager{};
	u64 m_dwLastUDPManagerUpdateTime{};

	u32 m_nRecvCount{};
	u32 m_nSendCount{};
	u32 m_nDuplicatedCount{};

	MLocatorDBMgr*	m_pDBMgr{};

	void*		m_vpServerStatusInfoBlob{};
	int			m_nLastGetServerStatusCount{};
	int			m_nServerStatusInfoBlobSize{};
};