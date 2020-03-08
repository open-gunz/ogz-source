#ifndef _MASYNCDBJOB_INSERTCONNLOG_H
#define _MASYNCDBJOB_INSERTCONNLOG_H


#include "MAsyncDBJob.h"



class MAsyncDBJob_InsertConnLog : public MAsyncJob {
protected:
	
protected:	// Input Argument
	u32		m_nAID;
	char 					m_szIP[64];
	string					m_strCountryCode3;
protected:	// Output Result

public:
	MAsyncDBJob_InsertConnLog()
		: MAsyncJob(MASYNCJOB_INSERTCONNLOG)
	{

	}
	virtual ~MAsyncDBJob_InsertConnLog()	{}

	bool Input(u32 nAID, char* szIP, const string& strCountryCode3 );
	virtual void Run(void* pContext);
};





#endif