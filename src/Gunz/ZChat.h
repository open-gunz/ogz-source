#ifndef _ZCHAT_H
#define _ZCHAT_H

#include "ZPrerequisites.h"
#include <vector>
using namespace std;

#include "ZChatCmd.h"
#include "ZReportAbuse.h"
#include "ZColorTable.h"

class MWidget;
class Chat;

class ZChat
{
	friend Chat;
private:
	ZChatCmdManager		m_CmdManager;

	u32	m_nLastInputTime;
	int					m_nSameMsgCount;
	char				m_nLastInputMsg[512];
	u32	m_nLastAbuseTime;
	int					m_nAbuseCounter;
	char				m_szWhisperLastSender[64];		// 마지막에 나한테 귓말 보냈던 사람

	ZReportAbuse		m_ReportAbuse;

	void LobbyChatOutput(const char* szChat,MCOLOR color = MCOLOR(ZCOLOR_CHAT_LOBBY_DEFALT) );
	void StageChatOutput(const char* szChat,MCOLOR color = MCOLOR(ZCOLOR_CHAT_STAGE_DEFAULT) );

	void InitCmds();
	bool CheckRepeatInput(const char* szMsg);
public:
	enum ZCHAT_MSG_TYPE
	{
		CMT_NORMAL = 0,
		CMT_SYSTEM = 1,
		CMT_BROADCAST = 2,

		CMT_END
	};
	enum ZCHAT_LOC
	{
		CL_CURRENT = 0,		// 지금 보고 있는 채팅창
		CL_LOBBY = 1,		// 로비 채팅창
		CL_STAGE = 2,		// 스테이지 채팅창
		CL_GAME = 3,		// 게임안 채팅창
		CL_END
	};

	ZChat();
	virtual ~ZChat();

	bool Input(const char* szMsg);
	void Output(const char* szMsg, ZCHAT_MSG_TYPE msgtype = CMT_NORMAL, ZCHAT_LOC loc=CL_CURRENT,MCOLOR _color=MCOLOR(0,0,0));
	void Output(MCOLOR color, const char* szMsg, ZCHAT_LOC loc=CL_CURRENT);

	void Clear(ZCHAT_LOC loc=CL_CURRENT);
	void Report112(const char* szReason);
	bool CheckChatFilter(const char* szMsg);	///< 욕필터링 검사. 귓말등의 커맨드명령어에서는 따로 처리를 해줘야한다.
	void FilterWhisperKey(MWidget* pWidget);
	void SetWhisperLastSender(char* szSenderName) { strcpy_safe(m_szWhisperLastSender, szSenderName); }

	ZChatCmdManager* GetCmdManager() { return &m_CmdManager; }
};

// 편의를 위해서 만든 함수 ---
void ZChatOutput(const char* szMsg, ZChat::ZCHAT_MSG_TYPE msgtype=ZChat::CMT_NORMAL, ZChat::ZCHAT_LOC loc=ZChat::CL_CURRENT,MCOLOR _color=MCOLOR(ZCOLOR_CHAT_SYSTEM));
void ZChatOutput(MCOLOR color, const char* szMsg, ZChat::ZCHAT_LOC loc = ZChat::CL_CURRENT);
void ZChatOutputF(const char *szFormat, ...);


#endif