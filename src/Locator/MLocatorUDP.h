#pragma once

#include <string>
#include <map>
#include "MemPool.h"
#include "MSync.h"
#include "MTime.h"

class MLocatorUDPInfo : public CMemPool<MLocatorUDPInfo>
{
	friend class MLocator;
	friend class MUDPManager;

	enum MAX_SIZE
	{
		USE_COUNT_SIZE = 2100000000,
	};

public:
	MLocatorUDPInfo() : m_nUseCount(0), m_nUsedCount(0) {}
	~MLocatorUDPInfo() {}

	auto GetIP() const { return m_dwIP; }
	auto& GetStrIP() const { return m_strIP; }
	auto GetPort() const { return m_nPort; }
	auto GetUseStartTime() const { return m_dwUseStartTime; }
	auto GetUseCount() const { return m_nUseCount; }
	auto GetUsedCount() const { return m_nUsedCount; }
	auto GetTotalUseCount() const { return m_nUsedCount + m_nUseCount; }

	void IncreaseUseCount(const unsigned int nCount = 1)
	{
		if (USE_COUNT_SIZE > (nCount + m_nUseCount))
			m_nUseCount += nCount;
	}
	void IncreaseUsedCount(const unsigned int nCount = 1)
	{
		if (USE_COUNT_SIZE > (nCount + m_nUsedCount))
			m_nUsedCount += nCount;
	}

	auto CalcuUseElapsedTime() { return GetGlobalTimeMS() - GetUseStartTime(); }
	auto CalcuUseElapsedTime(const u32 dwTime) { return (dwTime - m_dwUseStartTime); }

private:
	void SetIP(const u32 dwIP) { m_dwIP = dwIP; }
	void SetStrIP(const std::string& strIP) { m_strIP = strIP; }
	void SetPort(const int nPort) { m_nPort = nPort; }
	void SetUseStartTime(u64 dwUseStartTime) { m_dwUseStartTime = dwUseStartTime; }
	void SetUseCount(const unsigned int nUseCount)
	{
		if (USE_COUNT_SIZE > nUseCount)
			m_nUseCount = nUseCount;
		else
			m_nUseCount = USE_COUNT_SIZE;
	}
	void SetUsedCount(const unsigned int nUsedCount)
	{
		if (USE_COUNT_SIZE > nUsedCount)
			m_nUsedCount = nUsedCount;
		else
			m_nUsedCount = USE_COUNT_SIZE;
	}

	void SetInfo(const u32 dwIP,
		const int nPort,
		const unsigned int nUseCount,
		const u64 dwUseStartTime,
		const std::string& strIP);

private:
	u32				m_dwIP;
	std::string		m_strIP;
	int				m_nPort;
	u64				m_dwUseStartTime;
	unsigned int	m_nUseCount;
	unsigned int	m_nUsedCount;
};


template <typename T>
class MDeadUDPFinder
{
public:
	MDeadUDPFinder(u64 dwLimitTime, u64 dwEventTime) :
		m_dwLimitTime(dwLimitTime), m_dwEventTime(dwEventTime) {}
	~MDeadUDPFinder() {}

	bool operator() (const T& tObj)
	{
		return m_dwLimitTime < (m_dwEventTime - tObj.second->GetUseStartTime());
	}

private:
	MDeadUDPFinder();

	u64 m_dwLimitTime;
	u64 m_dwEventTime;
};


class MUDPManager : public std::map<u32, MLocatorUDPInfo*>
{
public:
	MUDPManager();
	~MUDPManager();

	void InitRecvUDPInfoMemPool();
	void ReleaseRecvUDPInfoMemPool();

	bool Insert(const u32 dwIPKey, const int nPort, u64 dwUseStartTime);
	bool Insert(const u32 dwIPKey, MLocatorUDPInfo* pRecvUDPInfo);
	bool SafeInsert(const u32 dwIPKey, const int nPort, u64 dwUseStartTime);
	bool SafeInsert(const u32 dwIPKey, MLocatorUDPInfo* pRecvUDPInfo);

	MLocatorUDPInfo* PopFirst();
	MLocatorUDPInfo* SafePopFirst();

	MLocatorUDPInfo* PopByIPKey(const u32 dwIPKey);
	MLocatorUDPInfo* SafePopByIPKey(const u32 dwIPKey);

	bool CheckWasInserted(const u32 dwIPKey);
	bool SafeCheckWasInserted(const u32 dwIPKey);

	const unsigned char GetUseCount(const u32 dwIPKey);
	const unsigned char SafeGetUseCount(const u32 dwIPKey);

	MLocatorUDPInfo* Find(const u32 dwIPKey);
	MLocatorUDPInfo* SafeFind(const u32 dwIPKey);

	void Delete(const u32 dwIPKey);
	void SafeDelete(const u32 dwIPKey);

	void ClearElapsedLiveTimeUDP(u64 dwLiveTime, u64 dwEventTime);
	void SafeClearElapsedLiveTimeUDP(u64 dwLiveTime, u64 dwEventTime);

	void SafeDestroy();

	void Lock() { m_csLock.unlock(); }
	void Unlock() { m_csLock.unlock(); }

	void DumpStatusInfo();
	void DumpUDPInfo();

	void SetBegin();
	MLocatorUDPInfo* GetCurPosUDP();
	bool MoveNext();

private:
	MCriticalSection m_csLock;

	iterator m_itBegin;

#ifdef _DEBUG
public:
	std::string m_strExDbgInfo;
#endif

};