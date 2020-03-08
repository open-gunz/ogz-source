#include "stdafx.h"
#include "MCommandProfiler.h"
#include "MDebug.h"
#include "MCommand.h"

MCommandProfiler::MCommandProfiler()
{
	m_pCM = NULL;
	Reset();
}

MCommandProfiler::~MCommandProfiler()
{

}

void MCommandProfiler::Reset()
{
	 m_nTotalOnProcessTime = 0;
	 m_nFirstTime = 0;
	 m_nLastTime = 0;

	 m_nTotalSendCmdCount = 0;
	 m_nTotalRecvCmdCount = 0;
	 m_nTotalSendBytes = 0;
	 m_nTotalRecvBytes = 0;

	 memset(m_Items, 0, sizeof(MCommandProfileItem) * MAX_COMMANDPROFILER_CMD_COUNT);
}

void MCommandProfiler::Analysis()
{
	FILE* fp = fopen("CmdProfile.txt", "wt");
	if (fp == 0) return;

	PrintTitle(fp);
	PrintSummary(fp);
	PrintCmdItems(fp);

	fclose(fp);

	Reset();
}

void MCommandProfiler::PrintTitle(FILE* fp)
{
	fputs("----------------------------------------------------------------------------------------------------------------------------------------\n", fp);
	fputs("| Command Profile                                                                                                                      |\n", fp);
	fputs("----------------------------------------------------------------------------------------------------------------------------------------\n", fp);
}

static char* AddCommaToNum(int num, char *buf, int buflen)
{ 
	char strNum[128];
	itoa_safe(num, strNum, 10);
	int len = (int)strlen(strNum); 
	char* str = strNum;

	char* saved = buf;

	assert(len > 0); 
	assert(buflen >= len + ((len - 1) / 3) + 1); 

	switch ((len - 1) % 3) { 
		case 3: /* fake label to make gcc happy */ 
			while (*str) { 
					*buf++ = ','; 
				case 2: *buf++ = *str++; 
				case 1: *buf++ = *str++; 
				case 0: *buf++ = *str++; 
			} 
	} 
	*buf = '\0'; 

	return saved;
} 

void MCommandProfiler::PrintSummary(FILE* fp)
{
	// 측정 시간
	// 총 Send 커맨드 개수, 초당 개수, bytes, 비율, 초당 bytes
	// 총 Recv 커맨드 개수, 초당 개수, bytes, 비율, 초당 bytes

	unsigned int nTotalTimeSec = (unsigned int)(floor((m_nLastTime - m_nFirstTime) / 1000.0f));
	if (nTotalTimeSec == 0) nTotalTimeSec=1;

	unsigned int nTotalCmdCount = m_nTotalSendCmdCount + m_nTotalRecvCmdCount;
	float fSendCmdCountRate=0.0f, fRecvCmdCountRate=0.0f;
	if (nTotalCmdCount != 0)
	{
		fSendCmdCountRate = (m_nTotalSendCmdCount / ((float)(nTotalCmdCount))) * 100.0f;
		fRecvCmdCountRate = (m_nTotalRecvCmdCount / ((float)(nTotalCmdCount))) * 100.0f;
	}

	int nSendCmdCountPerSec = m_nTotalSendCmdCount / nTotalTimeSec;
	int nRecvCmdCountPerSec = m_nTotalRecvCmdCount / nTotalTimeSec;

	unsigned int nTotalBytes = m_nTotalSendBytes + m_nTotalRecvBytes;
	float fSendByteRate=0.0f, fRecvByteRate=0.0f;
	if (nTotalBytes!=0)
	{
		fSendByteRate = (m_nTotalSendBytes / ((float)(nTotalBytes))) * 100.0f;
		fRecvByteRate = (m_nTotalRecvBytes / ((float)(nTotalBytes))) * 100.0f;
	}

	int nSendBytePerSec = m_nTotalSendBytes / nTotalTimeSec;
	int nRecvBytePerSec = m_nTotalRecvBytes / nTotalTimeSec;

	char szTotalSendCmdCount[128]="", szTotalRecvCmdCount[128]="";
	char szTotalSendBytes[128]="", szTotalRecvBytes[128]="";

	AddCommaToNum(m_nTotalSendCmdCount, szTotalSendCmdCount, 256);
	AddCommaToNum(m_nTotalSendBytes, szTotalSendBytes, 256);
	AddCommaToNum(m_nTotalRecvCmdCount, szTotalRecvCmdCount, 256);
	AddCommaToNum(m_nTotalRecvBytes, szTotalRecvBytes, 256);

	fprintf(fp, "  측정 시간 = %u초\n", nTotalTimeSec);
	fprintf(fp, "  Send> Count=%s(%4.1f%%) , Count/Sec = %d \t, Bytes = %s(%4.1f%%) , Bytes/Sec = %d\n", 
				szTotalSendCmdCount,
				fSendCmdCountRate, 
				nSendCmdCountPerSec,
				szTotalSendBytes,
				fSendByteRate, 
				nSendBytePerSec);

	fprintf(fp, "  Recv> Count=%s(%4.1f%%) , Count/Sec = %d \t, Bytes = %s(%4.1f%%) , Bytes/Sec = %d\n", 
				szTotalRecvCmdCount, 
				fRecvCmdCountRate, 
				nRecvCmdCountPerSec,
				szTotalRecvBytes, 
				fRecvByteRate, 
				nRecvBytePerSec);
}

void MCommandProfiler::PrintCmdItems(FILE* fp)
{
	// send: 개수(비율), byte(비율), byte/sec

	fputs("----------------------------------------------------------------------------------------------------------------------------------------\n", fp);
	fputs("  ID     |  Size ||  구분  ||   Count(Rate)  |  Count/Sec  |  Bytes/Sec(Rate)  |  AvgTime(Rate)  |  MinTime  |  MaxTime  |  설명\n", fp);
	fputs("----------------------------------------------------------------------------------------------------------------------------------------\n", fp);

	// 측정시간(초)
	unsigned int nTotalTimeSec = (unsigned int)(floor((m_nLastTime - m_nFirstTime) / 1000.0f));
	if (nTotalTimeSec == 0) nTotalTimeSec=1;

	char text[512];
	for (int i = 0; i < MAX_COMMANDPROFILER_CMD_COUNT; i++)
	{
		if ((m_Items[i].nSendCount <= 0) && (m_Items[i].nRecvCount <= 0)) continue;

		int nID = i;
		char szDesc[256] = "";
		if (m_pCM)
		{
			MCommandDesc* pDesc = m_pCM->GetCommandDescByID(i);
			if (pDesc)
				strcpy_safe(szDesc, pDesc->GetName());
		}
		
		int nAvgSize = (m_Items[i].nSendBytes + m_Items[i].nRecvBytes) / (m_Items[i].nSendCount + m_Items[i].nRecvCount);


		float fSendCountRate=0.0f, fRecvCountRate=0.0f;
		float fSendBytesRate=0.0f, fRecvBytesRate=0.0f;

		// Count Rate
		if (m_nTotalSendCmdCount != 0)
			fSendCountRate = (m_Items[i].nSendCount / (float)m_nTotalSendCmdCount) * 100.0f;
		if (m_nTotalRecvCmdCount != 0)
			fRecvCountRate = (m_Items[i].nRecvCount / (float)m_nTotalRecvCmdCount) * 100.0f;

		// Count/Sec
		float fSendCountPerSec=0.0f, fRecvCountPerSec=0.0f;
		fSendCountPerSec = m_Items[i].nSendCount / (float)nTotalTimeSec;
		fRecvCountPerSec = m_Items[i].nRecvCount / (float)nTotalTimeSec;

		// Bytes/Sec
		float fSendBytesPerSec=0.0f, fRecvBytesPerSec=0.0f;
		fSendBytesPerSec = m_Items[i].nSendBytes / (float)nTotalTimeSec;
		fRecvBytesPerSec = m_Items[i].nRecvBytes / (float)nTotalTimeSec;

		// Bytes Rate
		if (m_nTotalSendBytes != 0)
			fSendBytesRate = (m_Items[i].nSendBytes / (float)m_nTotalSendBytes) * 100.0f;
		if (m_nTotalRecvBytes != 0)
			fRecvBytesRate = (m_Items[i].nRecvBytes / (float)m_nTotalRecvBytes) * 100.0f;


		unsigned int nAvgTime = 0;
		unsigned int nMinTime = 0;
		unsigned int nMaxTime = 0;
		unsigned int nTotalTime = 0;
		float fTimeRate = 0.0f;

		if (m_Items[i].nRecvCount != 0)
			nAvgTime = m_Items[i].nTotalTime / m_Items[i].nRecvCount;
		nMinTime = m_Items[i].nMinTime;
		nMaxTime = m_Items[i].nMaxTime;

		nTotalTime = m_Items[i].nTotalTime / 1000;

		if (m_nTotalOnProcessTime != 0)
			fTimeRate = (m_Items[i].nTotalTime / (float)(m_nTotalOnProcessTime)) * 100.0f;


		if (m_Items[i].nSendCount != 0)
		{
			sprintf_safe(text, "  %5d  |  %3d  ||  Send  || %6d(%4.1f%%)  |     %4.1f    |    %6.1f(%4.1f%%)  |                 |           |           |  %s\n",
				nID, 
				nAvgSize, 
				m_Items[i].nSendCount, 
				fSendCountRate, 
				fSendCountPerSec,
				fSendBytesPerSec, 
				fSendBytesRate, 
				szDesc);
		}
		else
		{
			sprintf_safe(text, "  %5d  |  %3d  ||   --   ||      -         |       -     |         -         |        -        |       -   |       -   |  %s\n",
				nID, 
				nAvgSize,
				szDesc);
		}
		fputs(text, fp);

		if (m_Items[i].nRecvCount != 0)
		{
			sprintf_safe(text, "\t\t ||  Recv  || %6d(%4.1f%%)  |     %4.1f    |    %6.1f(%4.1f%%)  |   %6u(%4.1f%%) |  %6u   |  %6u   |\n",
				m_Items[i].nRecvCount, 
				fRecvCountRate, 
				fRecvCountPerSec,
				fRecvBytesPerSec, 
				fRecvBytesRate,
				nAvgTime, 
				fTimeRate, 
				nMinTime, 
				nMaxTime);
		}
		else
		{
			sprintf_safe(text, "\t\t ||   --   ||      -         |       -     |         -         |        -        |       -   |       -   |    \n");
		}
		fputs(text, fp);
		fputs("----------------------------------------------------------------------------------------------------------------------------------------\n", fp);
	}

}

void MCommandProfiler::OnCommandBegin(MCommand* pCmd, unsigned int nTime)
{
	if (m_nFirstTime == 0) m_nFirstTime = nTime;
	m_nLastTime = nTime;

	int nCmdID = pCmd->GetID();
	if ((nCmdID <0) || (nCmdID >= MAX_COMMANDPROFILER_CMD_COUNT)) return;

	m_Items[nCmdID].nStartTime = nTime;
}

void MCommandProfiler::OnCommandEnd(MCommand* pCmd, unsigned int nTime)
{
	if (m_nFirstTime == 0) m_nFirstTime = nTime;
	m_nLastTime = nTime;

	int nCmdID = pCmd->GetID();
	if ((nCmdID <0) || (nCmdID >= MAX_COMMANDPROFILER_CMD_COUNT)) return;

	unsigned int nPlayTime = nTime - m_Items[nCmdID].nStartTime;
	m_Items[nCmdID].nTotalTime += nPlayTime;
	if (nPlayTime > m_Items[nCmdID].nMaxTime) m_Items[nCmdID].nMaxTime = nPlayTime;
	if (nPlayTime < m_Items[nCmdID].nMinTime) m_Items[nCmdID].nMinTime = nPlayTime;


	m_nTotalOnProcessTime += nPlayTime;
}

#include "MPacket.h"

void MCommandProfiler::OnSend(MCommand* pCmd)
{
	int nSize = pCmd->GetSize() + sizeof(MPacketHeader);
	m_nTotalSendBytes += (unsigned int)nSize;
	m_nTotalSendCmdCount++;

	int nCmdID = pCmd->GetID();
	if ((nCmdID <0) || (nCmdID >= MAX_COMMANDPROFILER_CMD_COUNT)) return;
	m_Items[nCmdID].nSendBytes += nSize;
	m_Items[nCmdID].nSendCount++;
}

void MCommandProfiler::OnRecv(MCommand* pCmd)
{
	int nSize = pCmd->GetSize() + sizeof(MPacketHeader);
	m_nTotalRecvBytes += (unsigned int)nSize;
	m_nTotalRecvCmdCount++;

	int nCmdID = pCmd->GetID();
	if ((nCmdID <0) || (nCmdID >= MAX_COMMANDPROFILER_CMD_COUNT)) return;
	m_Items[nCmdID].nRecvBytes += nSize;
	m_Items[nCmdID].nRecvCount++;

}


void MCommandProfiler::Init(MCommandManager* pCM)
{
	m_pCM = pCM;
}