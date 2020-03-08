#ifndef _MASYNCDBJOB_GETLOGININFO_H
#define _MASYNCDBJOB_GETLOGININFO_H

#include "MAsyncDBJob.h"

class MAsyncDBJob_GetLoginInfo : public MAsyncJob {
protected:
	MUID			m_uidComm;
	bool			m_bFreeLoginIP;
	u32	m_nChecksumPack;
	bool			m_bCheckPremiumIP;
	char			m_szIP[128];
	u32			m_dwIP;
protected:	// Input Argument
	char	m_szUserID[256];
	char	m_szUniqueID[1024];
	char	m_szCertificate[1024];
	char	m_szName[256];
	int		m_nAge;
	int		m_nSex;
	string	m_strCountryCode3;
protected:	// Output Result
	MMatchAccountInfo*	m_pAccountInfo;
	unsigned int		m_nAID;
	char				m_szDBPassword[32];
public:
	MAsyncDBJob_GetLoginInfo(const MUID& uidComm)
		: MAsyncJob(MASYNCJOB_GETLOGININFO)
	{
		m_uidComm = uidComm;
		m_pAccountInfo = NULL;
		m_nAID = 0;
		m_szDBPassword[0] = 0;
		m_bFreeLoginIP = false;
		m_nChecksumPack = 0;
		m_bCheckPremiumIP = false;
		m_szIP[0] = 0;
		m_dwIP = 0xFFFFFFFF;
	}
	virtual ~MAsyncDBJob_GetLoginInfo()	{}

	bool Input(MMatchAccountInfo* pNewAccountInfo,
			   const char* szUserID, 
			   const char* szUniqueID, 
			   const char* szCertificate, 
			   const char* szName, 
			   const int nAge, 
			   const int nSex,
			   const bool bFreeLoginIP,
		u32 nChecksumPack,
			   const bool bCheckPremiumIP,
			   const char* szIP,
			   u32 dwIP,
			   const string& strCountryCode3);
	virtual void Run(void* pContext);

	// output
	MMatchAccountInfo* GetAccountInfo() { return m_pAccountInfo; }
	unsigned int GetAID() { return m_nAID; }
	const char* GetDBPassword() { return m_szDBPassword; }
	const MUID& GetCommUID() { return m_uidComm; }
	bool IsFreeLoginIP() { return m_bFreeLoginIP; }
	u32 GetChecksumPack() { return m_nChecksumPack; }
	const string& GetCountryCode3() { return m_strCountryCode3; }
};





#endif