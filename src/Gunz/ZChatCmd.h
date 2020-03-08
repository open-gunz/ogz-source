#pragma once

#include <string>
#include <list>
#include <vector>
#include <map>
#include "MXmlParser.h"

#define	ZCHATCMD_TEXSIZE	2048
#define	ZCHATCMD_NAMESIZE	256
#define	ZCHATCMD_LIMIT_REPEAT_COUNT 3

#define ARGVNoMin	(-1)
#define ARGVNoMax	(-1)

struct ZChatCmdArgvInfo
{
	char	*cargv[256];
	int		cargc;
	char	argbuf[2048];
};

class ZChatCmd;
class ZChatCmdManager;

using ZChatCmdMap = std::map<std::string, ZChatCmd>;
using ZChatCmdProc = void(*)(const char*, int, char **);

enum ZChatCmdFlag
{
	CCF_NONE	= 0,
	CCF_LOBBY	= 0x01,
	CCF_STAGE	= 0x02,
	CCF_GAME	= 0x04,
	CCF_ALL		= 0x0F,

	CCF_TEST	= 0x40,
	CCF_ADMIN	= 0x80	
};

class ZChatCmd
{
private:
	int					m_nID;
	char				m_szName[ZCHATCMD_NAMESIZE];
	char				m_szUsage[ZCHATCMD_TEXSIZE];
	char				m_szHelp[ZCHATCMD_TEXSIZE];
	int					m_nMinArgs;
	int					m_nMaxArgs;
	bool				m_bRepeatEnabled;
	u32					m_nFlag;
	ZChatCmdProc		m_fnProc;
protected:
public:
	ZChatCmd(int nID, const char* szName, u32 flag,
		int nMinArgs = ARGVNoMin, int nMaxArgs = ARGVNoMax, bool bRepeatEnabled = true,
		const char* szUsage = nullptr, const char* szHelp = nullptr);

	void OnProc(const char* line, const int argc, char **const argv);
	void SetProc(ZChatCmdProc fnProc) { m_fnProc = fnProc; }
	int GetID() const				{ return m_nID; }
	const char* GetName() const		{ return m_szName; }
	const char* GetUsage() const	{ return m_szUsage; }
	const char* GetHelp() const		{ return m_szHelp; }
	ZChatCmdProc& GetProc()			{ return m_fnProc; }
	int GetMinArgs() const			{ return m_nMinArgs; }
	int GetMaxArgs() const			{ return m_nMaxArgs; }
	u32 GetFlag() const				{ return m_nFlag; }
	bool GetRepeatEnabled() const { return m_bRepeatEnabled; }
};

class ZChatCmdManager
{
public:
	enum CmdInputFlag
	{
		CIF_NORMAL	= 0x1,
		CIF_ADMIN	= 0x2,
		CIF_TESTER	= 0x4,
	};

	ZChatCmdManager();
	~ZChatCmdManager();

	void AddCommand(int		nID,
		const char*			szName,
		ZChatCmdProc		fnProc,
		u32					flag,
		int					nMinArgs = ARGVNoMin,
		int					nMaxArgs = ARGVNoMax,
		bool				bRepeatEnabled = true,
		const char*			szUsage = nullptr,
		const char*			szHelp = nullptr);
	void AddAlias(const char* szNewCmdName, const char* szTarCmdName);
	bool IsRepeatEnabled(const char* szLine);
	bool DoCommand(const char* szLine, ZChatCmdFlag nCurrFlag, u32 nInputFlag=CIF_NORMAL);
	ZChatCmd* GetCommandByName(const char* szName);
	ZChatCmd* GetCommandByID(int nID);
	ZChatCmdMap::iterator GetCmdBegin() { return m_CmdMap.begin(); }
	ZChatCmdMap::iterator GetCmdEnd()	{ return m_CmdMap.end(); }
	int GetCmdCount() const { return (int)m_CmdMap.size(); }

private:
	ZChatCmdMap m_CmdMap;
	std::map<std::string, std::string> m_AliasMap;
	ZChatCmd* MakeArgv(const char* szLine, ZChatCmdArgvInfo* pAI);
};

void ZImplodeChatCmdArgs(char* szOut, size_t maxlen, int argc, char **const argv, int nFirstIndex = 0);
template <size_t size>
void ZImplodeChatCmdArgs(char (&szOut)[size], int argc, char **const argv, int nFirstIndex = 0) {
	return ZImplodeChatCmdArgs(szOut, size, argc, argv, nFirstIndex);
}

class ZCmdXmlParser : public MXmlParser
{
public:
	struct CmdStr
	{
		char szName[ZCHATCMD_NAMESIZE];
		char szUsage[ZCHATCMD_TEXSIZE];
		char szHelp[ZCHATCMD_TEXSIZE];
		std::vector<std::string> vecAliases;
		CmdStr()
		{
			szName[0] = szUsage[0] = szHelp[0] = 0;
		}
	};

	CmdStr* GetStr(int nID);

private:
	std::map<int, CmdStr> m_CmdMap;

	virtual void ParseRoot(const char* szTagName, MXmlElement* pElement);
	void ParseCmd(MXmlElement* pElement);
};