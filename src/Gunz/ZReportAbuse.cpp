#include "stdafx.h"
#include "ZReportAbuse.h"
#include "ZGameInterface.h"
#include "ZGameClient.h"
#include "ZChat.h"
#include "MDrawContext.h"
#include "ZNetRepository.h"
#include "ZApplication.h"
#include "ZConfiguration.h"


#define REPORT_ABUSE_ACTIVATE_TIME		(1000 * 60 * 3)
#define REPORT_ABUSE_COOL_TIME			(1000 * 60)
#define REPORT_ABUSE_FILENAME			"119.txt"


ZReportAbuse::ZReportAbuse()
{
	m_szReason[0] = 0;
	m_nReportTime=0;
	m_nChatTop = 0;
	for (int i = 0; i < REPORT_MAX_HISTORY; i++)
	{
		m_ChatHistory[i].timestamp = 0;
	}
}

ZReportAbuse::~ZReportAbuse()
{

}

void ZReportAbuse::Report(const char* szReason)
{
	u32 nNowTime = GetGlobalTimeMS();
	if ((nNowTime - m_nReportTime) < REPORT_ABUSE_COOL_TIME)
	{
		ZChatOutput( ZMsg(MSG_119_REPORT_WAIT_ONEMINUTE) );
		return;
	}
	
	strcpy_safe(m_szReason, szReason);
	
	SaveFile();
	SendFile();
	m_nReportTime = nNowTime;

	ZChatOutput( ZMsg(MSG_119_REPORT_OK) );
}

void ZReportAbuse::OutputString(const char* szStr)
{
	char *pPureText = MDrawContext::GetPureText(szStr);
	char temp[512];
	strcpy_safe(temp, pPureText);
	free(pPureText);

	m_ChatHistory[m_nChatTop].str = temp;
	m_ChatHistory[m_nChatTop].timestamp = GetGlobalTimeMS();

	m_nChatTop++;
	if (m_nChatTop >= REPORT_MAX_HISTORY) m_nChatTop = 0;
}

void ZReportAbuse::SendFile()
{
	time_t currtime;
	time(&currtime);
	struct tm TM;
	auto err = localtime_s(&TM, &currtime);

	char szPlayer[128];
	wsprintf(szPlayer, "%s", ZGetMyInfo()->GetCharName());

	char szFileName[_MAX_DIR];
	wsprintf(szFileName, "%02d%02d_%02d%02d_%s_%s",
			TM.tm_mon+1, TM.tm_mday, TM.tm_hour, TM.tm_min, szPlayer, "119.txt");

	// BAReport
	char szCmd[4048];
	char szRemoteFileName[_MAX_DIR];
	wsprintf(szRemoteFileName, "%s/%s/%s", ZGetConfiguration()->GetBAReportDir(), "gunz119", szFileName);

	wsprintf(szCmd, "app=%s;addr=%s;port=21;id=ftp;passwd=ftp@;user=%s;localfile=%s;remotefile=%s;srcdelete=1;agree=1",
		"gunz", ZGetConfiguration()->GetBAReportAddr(), szPlayer, REPORT_ABUSE_FILENAME, szRemoteFileName);
	
	ShellExecute(g_hWnd, NULL, "BAReport.exe", szCmd, NULL, SW_HIDE);

}


void ZReportAbuse::SaveFile()
{
	auto nNowTime = GetGlobalTimeMS();

	FILE* fp{};
	auto err = fopen_s(&fp, REPORT_ABUSE_FILENAME, "wt");
	if (err != 0 || fp == NULL) return;

	fprintf(fp, "%s\n", m_szReason);

	for (int i = m_nChatTop; i < REPORT_MAX_HISTORY; i++)
	{
		if ((m_ChatHistory[i].timestamp != 0) && 
			((nNowTime - m_ChatHistory[i].timestamp) < REPORT_ABUSE_ACTIVATE_TIME))
		{
			fprintf(fp, "%s\n", m_ChatHistory[i].str.c_str());
		}
	}
	for (int i = 0; i < m_nChatTop; i++)
	{
		if ((m_ChatHistory[i].timestamp != 0) && 
			((nNowTime - m_ChatHistory[i].timestamp) < REPORT_ABUSE_ACTIVATE_TIME))
		{
			fprintf(fp, "%s\n", m_ChatHistory[i].str.c_str());
		}
	}

	fclose(fp);	
}

