#ifndef _ZREPORTABUSE_H
#define _ZREPORTABUSE_H

#include "ZPrerequisites.h"
#include <vector>
#include <string>
using namespace std;


#define REPORT_MAX_HISTORY				200

class ZReportAbuse
{
private:
	struct _ChatHistory
	{
		u32	timestamp;
		string				str;
	};

	char					m_szReason[256];
	_ChatHistory			m_ChatHistory[REPORT_MAX_HISTORY];
	int						m_nChatTop;

	u32		m_nReportTime;

	void SendFile();
	void SaveFile();

public:
	ZReportAbuse();
	virtual ~ZReportAbuse();
	void Report(const char* szReason);
	void OutputString(const char* szStr);
	
};



#endif