#ifndef _MCOMMANDPROFILER_H
#define _MCOMMANDPROFILER_H

#include "limits.h"
#include "MCommandManager.h"

struct MCommandProfileItem
{
	// 측정에 필요한 값
	unsigned int		nStartTime;

	// 측정값
	unsigned int		nTotalTime;
	unsigned int		nMinTime;
	unsigned int		nMaxTime;

	unsigned int		nSendCount;
	unsigned int		nRecvCount;
	unsigned int		nSendBytes;
	unsigned int		nRecvBytes;

	MCommandProfileItem()
	{
		nStartTime = 0;
		nTotalTime = 0;
		nMinTime = UINT_MAX;
		nMaxTime = 0;
		nSendCount = 0;
		nRecvCount = 0;
		nSendBytes = 0;
		nRecvBytes = 0;
	}

};

#define MAX_COMMANDPROFILER_CMD_COUNT		65536		// 16비트

class MCommand;

class MCommandProfiler
{
private:
	MCommandManager*			m_pCM;
	unsigned int				m_nFirstTime;
	unsigned int				m_nLastTime;
	unsigned int				m_nTotalOnProcessTime;

	unsigned int				m_nTotalSendCmdCount;
	unsigned int				m_nTotalSendBytes;

	unsigned int				m_nTotalRecvCmdCount;
	unsigned int				m_nTotalRecvBytes;

	MCommandProfileItem			m_Items[MAX_COMMANDPROFILER_CMD_COUNT];
	void PrintTitle(FILE* fp);
	void PrintSummary(FILE* fp);
	void PrintCmdItems(FILE* fp);
	void Reset();
public:
	MCommandProfiler();
	virtual ~MCommandProfiler();
	void Init(MCommandManager* pCM);
	void Analysis();
	void OnCommandBegin(MCommand* pCmd, unsigned int nTime);
	void OnCommandEnd(MCommand* pCmd, unsigned int nTime);
	void OnSend(MCommand* pCmd);
	void OnRecv(MCommand* pCmd);
};




#endif