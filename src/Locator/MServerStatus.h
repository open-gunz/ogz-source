#pragma once

#include <vector>
#include <string>
#include <utility>

class MServerStatus
{
public:
	MServerStatus() : m_nID(0), m_nMaxPlayer(0), m_nCurPlayer(0), m_dwIP(0),
		m_nPort(0), m_bIsOpened(false), m_nType(0), m_bIsLive(false)
	{}

	int				GetID() const { return m_nID; }
	int				GetMaxPlayer() const { return m_nMaxPlayer; }
	int				GetCurPlayer() const { return m_nCurPlayer; }
	const std::string&	GetLastUpdatedTime() const { return m_strLastUpdatedTime; }
	const std::string&	GetIPString() const { return m_strIP; }
	u32 			GetIP() const { return m_dwIP; }
	int				GetPort() const { return m_nPort; }
	const std::string&	GetServerName() const { return m_strServerName; }
	bool			IsOpened() const { return m_bIsOpened; }
	int				GetType() const { return m_nType; }
	bool			IsLive() const { return m_bIsLive; }

	void SetID(const int nID) { m_nID = nID; }
	void SetMaxPlayer(const int nMaxPlayer) { m_nMaxPlayer = nMaxPlayer; }
	void SetCurPlayer(const int nCurPlayer) { m_nCurPlayer = nCurPlayer; }
	void SetLastUpdatedTime(const std::string& strLastUpdatedTime) { m_strLastUpdatedTime = strLastUpdatedTime; }
	void SetIPString(const std::string& strIP) { m_strIP = strIP; }
	void SetIP(const u32 dwIP) { m_dwIP = dwIP; }
	void SetPort(const int nPort) { m_nPort = nPort; }
	void SetServerName(const std::string& strServerName) { m_strServerName = strServerName; }
	void SetOpenState(const bool bOpenState) { m_bIsOpened = bOpenState; }
	void SetType(const char nType) { m_nType = nType; }
	void SetLiveStatus(const bool bLiveState);

#ifdef _DEBUG
	const std::string GetDebugString()
	{
		char szBuf[1024];
		sprintf_safe(szBuf, "ID:%d, CurPlayer:%d, MaxPlayer:%d, Time:%s, IP:%s(%u), Port:%d, Name:%s, IsOpened:%d\n",
			GetID(), GetCurPlayer(), GetMaxPlayer(), GetLastUpdatedTime().c_str(), GetIPString().c_str(), GetIP(),
			GetPort(), GetServerName().c_str(), IsOpened());

		return szBuf;
	}
#endif

private:
	int		m_nID;
	int		m_nMaxPlayer;
	int		m_nCurPlayer;
	std::string	m_strLastUpdatedTime;
	std::string	m_strIP;
	u32		m_dwIP;
	int		m_nPort;
	std::string	m_strServerName;
	bool	m_bIsOpened;
	char	m_nType;
	bool	m_bIsLive;
};


const int MakeCustomizeMin(const std::string& strTime);
const float GetPlayerRate(const float fCurPlayer, const float fMaxPlayer);


typedef std::vector< MServerStatus > ServerStatusVec;
typedef std::vector< int >			ServerIDVec;


class MServerStatusMgr
{
public:
	void		Insert(const MServerStatus& ss);
	void		Reserve(const int nSize) { m_ServerStatusVec.reserve(nSize); }
	void		Clear() { m_ServerStatusVec.clear(); }
	const int	Capacity() const { return static_cast<int>(m_ServerStatusVec.capacity()); }
	const int	GetSize() const { return static_cast<int>(m_ServerStatusVec.size()); }
	void		CheckDeadServerByLastUpdatedTime(const int nMarginOfErrMin, const int nCmpCustomizeMin);
	const int	CalcuMaxCmpCustomizeMin();
	void		IncreaseLiveServerCount(const int nCount = 1) { m_nLiveServerCount += nCount; }
	void		IncreaseDeadServerCount(const int nCount = 1) { m_nDeadServerCount += nCount; }
	void		AddDeadServerID(const int nID) { m_vDeadServerIDList.push_back(nID); }
	const int	GetLiveServerCount() const { return m_nLiveServerCount; }
	const int	GetDeadServerCount() const { return m_nDeadServerCount; }
	const ServerIDVec& GetDeadServerIDList() const { return m_vDeadServerIDList; }


	MServerStatus& operator[] (const int nPos) { return m_ServerStatusVec[nPos]; }
	const MServerStatus& operator[] (const int nPos) const { return m_ServerStatusVec[nPos]; }

private:
	void SetLiveServerCount(const int nCount) { m_nLiveServerCount = 0; }
	void SetDeadServerCount(const int nCount) { m_nDeadServerCount = 0; }
	void ClearDeadServerIDList() { m_vDeadServerIDList.clear(); }

private:
	ServerStatusVec m_ServerStatusVec;
	int				m_nLiveServerCount;
	int				m_nDeadServerCount;
	ServerIDVec		m_vDeadServerIDList;
};

template< typename T >
class MDeadTimeServerChecker
{
public:
	MDeadTimeServerChecker(const int nMarginOfErrorMin,
		const int nCmpCustomizeMin,
		MServerStatusMgr* pServerStatusMgr) :
		m_nMarginOfErrorMin(nMarginOfErrorMin), m_nCmpCustomizeMin(nCmpCustomizeMin), m_pServerStatusMgr(pServerStatusMgr)
	{
		ASSERT(0 != pServerStatusMgr);
	}

	~MDeadTimeServerChecker() {}

	void operator() (T& tObj)
	{
		if (m_nMarginOfErrorMin > abs(m_nCmpCustomizeMin - MakeCustomizeMin(tObj.GetLastUpdatedTime())))
		{
			tObj.SetLiveStatus(true);
			m_pServerStatusMgr->IncreaseLiveServerCount();
		}
		else
		{
			tObj.SetLiveStatus(false);
			m_pServerStatusMgr->IncreaseDeadServerCount();
			m_pServerStatusMgr->AddDeadServerID(tObj.GetID());
		}
	}

private:
	MDeadTimeServerChecker();

	int					m_nMarginOfErrorMin;
	int					m_nCmpCustomizeMin;
	MServerStatusMgr*	m_pServerStatusMgr;
};