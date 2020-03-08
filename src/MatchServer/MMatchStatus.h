#ifndef _MMATCHSTATUS_H
#define _MMATCHSTATUS_H

class MMatchServer;

#define MATCHSTATUS_DUMP_LEN		4096

class MMatchStatus
{
private:
	bool			m_bCreated;
	MMatchServer*	m_pMatchServer;
protected:
	u64					m_nStartTime;
	u32	m_nTotalCommandQueueCount;
	u32	m_nTickCommandQueueCount;

#define MSTATUS_MAX_CMD_COUNT		100000
#define MSTATUS_MAX_DBQUERY_COUNT	60
#define MSTATUS_MAX_CMD_HISTORY		20

	u32	m_nCmdCount[MSTATUS_MAX_CMD_COUNT][3];
	u32	m_nDBQueryCount[MSTATUS_MAX_DBQUERY_COUNT][3];
	u32	m_nCmdHistory[MSTATUS_MAX_CMD_HISTORY];
	int					m_nHistoryCursor;
	int					m_nRunStatus;
	void AddCmdHistory(u32 nCmdID);

	char				m_szDump[MATCHSTATUS_DUMP_LEN];
public:
	MMatchStatus();
	virtual ~MMatchStatus();
	static MMatchStatus* GetInstance();
	bool Create(MMatchServer* pMatchServer);
public:
	void SaveToLogFile();
	void AddCmdCount(u32 nCmdCount)
	{
		m_nTickCommandQueueCount = nCmdCount;
		m_nTotalCommandQueueCount += m_nTickCommandQueueCount;
	}
	void AddCmd(u32 nCmdID, int nCount = 1, u64 nTime = 0)
	{
		AddCmdHistory(nCmdID);

		if (nCmdID >= MSTATUS_MAX_CMD_COUNT) return;
		m_nCmdCount[nCmdID][0] += nCount;
		m_nCmdCount[nCmdID][1] += static_cast<u32>(nTime);
		m_nCmdCount[nCmdID][2] = static_cast<u32>(nTime);
	}
	void AddDBQuery(u32 nDBQueryID, u64 nTime)
	{
		if (nDBQueryID >= MSTATUS_MAX_DBQUERY_COUNT) return;

		m_nDBQueryCount[nDBQueryID][0]++;
		m_nDBQueryCount[nDBQueryID][1] += static_cast<u32>(nTime);
		m_nDBQueryCount[nDBQueryID][2] = static_cast<u32>(nTime);
	}
	inline void SaveCmdHistory();
	void SetRunStatus(int value) { m_nRunStatus = value; }
	void SetLog(const char* szDump);
	inline void Dump();
};

inline MMatchStatus* MGetServerStatusSingleton() 
{
	return MMatchStatus::GetInstance();
}

inline void MMatchStatus::SaveCmdHistory()
{
	int t=0;
	for (int i = m_nHistoryCursor; i < MSTATUS_MAX_CMD_HISTORY; i++)
	{
		mlog("History(%d): %u\n", t++, m_nCmdHistory[i]);
	}
	for (int i = 0; i < m_nHistoryCursor; i++)
	{
		mlog("History(%d): %u\n", t++, m_nCmdHistory[i]);
	}

	mlog("RunStatus : %d\n", m_nRunStatus);
}

inline void MMatchStatus::Dump()
{
	SaveCmdHistory();

//	mlog("Dump: ");
//	mlog(m_szDump);
}
#endif