#pragma once

#include <string>
#include <map>

class MServerStatusMgr;

class MLocatorDBMgr
{
public:
	MLocatorDBMgr();
	virtual ~MLocatorDBMgr();

	CString BuildDSNString(const CString& strDSN,
		const CString& strUserName,
		const CString& strPassword);
	bool Connect(const CString& strDSNConnect);
	void Disconnect();

	bool GetServerStatus(MServerStatusMgr* pServerStatusMgr);

	bool StartUpLocaterStauts(const int nLocatorID,
		const std::string& strIP,
		const int nPort,
		const int nDuplicatedUpdateTime);
	bool UpdateLocaterStatus(const int nLocatorID,
		const u32 nRecvCount,
		const u32 nSendCount,
		const u32 nBlockCount,
		const u32 nDuplicatedCount);

	bool InsertLocatorLog(const int nLocatorID, const std::map<std::string, u32>& CountryStatistics);

private:
	bool CheckOpen();

private:
	CString				m_strDSNConnect;
};