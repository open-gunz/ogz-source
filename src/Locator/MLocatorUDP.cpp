#include "stdafx.h"
#include "MLocatorUDP.h"
#include "MDebug.h"
#include <utility>
#include <algorithm>
#include "SafeString.h"
#include "MInetUtil.h"
#include "MTime.h"

void MLocatorUDPInfo::SetInfo(const u32 dwIP,
	const int nPort,
	const unsigned int nUseCount,
	const u64 dwUseStartTime,
	const std::string& strIP)
{
	SetIP(dwIP);
	SetStrIP(strIP);
	SetPort(nPort);
	SetUseCount(nUseCount);
	SetUseStartTime(dwUseStartTime);
}

MUDPManager::MUDPManager() = default;
MUDPManager::~MUDPManager() { SafeDestroy(); }

bool MUDPManager::Insert(const u32 dwIPKey, const int nPort, u64 dwUseStartTime)
{
	const_iterator itFind = find(dwIPKey);
	if (itFind == end())
	{
		MLocatorUDPInfo* pRecvUDPInfo = new MLocatorUDPInfo;
		if (0 != pRecvUDPInfo)
		{
			MSocket::in_addr addr;
			addr.S_un.S_addr = dwIPKey;

			char ip[16];
			GetIPv4String(addr, ip);
			pRecvUDPInfo->SetInfo(dwIPKey, nPort, 1, dwUseStartTime, ip);

			insert(std::pair<u32, MLocatorUDPInfo*>(dwIPKey, pRecvUDPInfo));

			return true;
		}
		else
		{
			mlog("MUDPManager::Insert - memory error\n");
		}
	}

	return false;
}


bool MUDPManager::Insert(const u32 dwIPKey, MLocatorUDPInfo* pRecvUDPInfo)
{
	const_iterator itFind = find(dwIPKey);
	if (itFind == end())
	{
		insert(std::pair<u32, MLocatorUDPInfo*>(dwIPKey, pRecvUDPInfo));
		return true;
	}
	return false;
}


bool MUDPManager::SafeInsert(const u32 dwIPKey, const int nPort, u64 dwUseStartTime)
{
	Lock();
	const bool bResult = Insert(dwIPKey, nPort, dwUseStartTime);
	Unlock();

	return bResult;
}


bool MUDPManager::SafeInsert(const u32 dwIPKey, MLocatorUDPInfo* pRecvUDPInfo)
{
	Lock();
	const bool bResult = Insert(dwIPKey, pRecvUDPInfo);
	Unlock();

	return bResult;
}


MLocatorUDPInfo* MUDPManager::PopFirst()
{
	if (empty()) return 0;

	iterator			itBegin = begin();
	MLocatorUDPInfo*	pRecvUDPInfo = itBegin->second;

	erase(itBegin);

	return pRecvUDPInfo;
}


MLocatorUDPInfo* MUDPManager::SafePopFirst()
{
	Lock();
	MLocatorUDPInfo* pRecvUDPInfo = PopFirst();
	Unlock();

	return pRecvUDPInfo;
}


MLocatorUDPInfo* MUDPManager::PopByIPKey(const u32 dwIPKey)
{
	iterator itFind = find(dwIPKey);
	if (end() != itFind)
	{
		MLocatorUDPInfo* pRecvUDPInfo = itFind->second;

		erase(itFind);

		return pRecvUDPInfo;
	}

	return 0;
}


MLocatorUDPInfo* MUDPManager::SafePopByIPKey(const u32 dwIPKey)
{
	Lock();
	MLocatorUDPInfo* pRecvUDPInfo = PopByIPKey(dwIPKey);
	Unlock();

	return pRecvUDPInfo;
}


bool MUDPManager::CheckWasInserted(const u32 dwIPKey)
{
	const_iterator itFind = find(dwIPKey);
	if (itFind != end())
	{
		itFind->second->IncreaseUseCount();
		return true;
	}

	return false;
}


bool MUDPManager::SafeCheckWasInserted(const u32 dwIPKey)
{
	Lock();
	const bool bResult = CheckWasInserted(dwIPKey);
	Unlock();

	return bResult;
}


const unsigned char MUDPManager::GetUseCount(const u32 dwIPKey)
{
	const_iterator itFind = find(dwIPKey);
	if (end() != itFind)
		return itFind->second->GetUseCount();

	return 0;
}


const unsigned char MUDPManager::SafeGetUseCount(const u32 dwIPKey)
{
	Lock();
	const unsigned char nUseCount = GetUseCount(dwIPKey);
	Unlock();

	return nUseCount;
}


void MUDPManager::SafeDestroy()
{
	Lock();
	iterator It, End;
	for (It = begin(), End = end(); It != End; ++It)
		delete It->second;
	clear();
	Unlock();
}


MLocatorUDPInfo* MUDPManager::Find(const u32 dwIPKey)
{
	const_iterator itFind = find(dwIPKey);
	if (end() != itFind)
		return itFind->second;

	return 0;
}


MLocatorUDPInfo* MUDPManager::SafeFind(const u32 dwIPKey)
{
	Lock();
	MLocatorUDPInfo* pRecvUDPInfo = Find(dwIPKey);
	Unlock();

	return pRecvUDPInfo;
}


void MUDPManager::Delete(const u32 dwIPKey)
{
	MLocatorUDPInfo* pRecvUDPInfo = PopByIPKey(dwIPKey);
	if (0 != pRecvUDPInfo)
		delete pRecvUDPInfo;
}


void MUDPManager::SafeDelete(const u32 dwIPKey)
{
	Lock();
	Delete(dwIPKey);
	Unlock();
}


void MUDPManager::ClearElapsedLiveTimeUDP(u64 dwLiveTime, u64 dwEventTime)
{
	iterator itDeadUDP;
	while (true)
	{
		itDeadUDP = find_if(begin(), end(),
			MDeadUDPFinder< std::pair<u32, MLocatorUDPInfo*> >(dwLiveTime, dwEventTime));

		if (end() == itDeadUDP)
			break;

		delete itDeadUDP->second;
		erase(itDeadUDP);
	}
}


void MUDPManager::SafeClearElapsedLiveTimeUDP(u64 dwLiveTime, u64 dwEventTime)
{
	Lock();
	ClearElapsedLiveTimeUDP(dwLiveTime, dwEventTime);
	Unlock();
}


void MUDPManager::DumpStatusInfo()
{
	char szDbgInfo[1024];

	sprintf_safe(szDbgInfo, "Size:%d\n", size());

	mlog(szDbgInfo);
}


void MUDPManager::DumpUDPInfo()
{
	char szDbgInfo[1024];
	iterator It, End;

	for (It = begin(), End = end(); It != End; ++It)
	{
		MSocket::in_addr addr;
		addr.S_un.S_addr = It->second->GetIP();

		char ip[16];
		GetIPv4String(addr, ip);
		sprintf_safe(szDbgInfo, "IP:%s, Port:%d, Count:%d\n",
			ip,
			It->second->GetPort(),
			It->second->GetUseCount());
		mlog(szDbgInfo);
	}
}


void MUDPManager::InitRecvUDPInfoMemPool()
{
	InitMemPool(MLocatorUDPInfo);
}


void MUDPManager::ReleaseRecvUDPInfoMemPool()
{
	ReleaseMemPool(MLocatorUDPInfo);

	UninitMemPool(MLocatorUDPInfo);
}

void MUDPManager::SetBegin()
{
	m_itBegin = begin();
}


MLocatorUDPInfo* MUDPManager::GetCurPosUDP()
{
	if (end() != m_itBegin)
		return m_itBegin->second;
	return 0;
}


bool MUDPManager::MoveNext()
{
	++m_itBegin;
	if (end() == m_itBegin) return false;
	return true;
}