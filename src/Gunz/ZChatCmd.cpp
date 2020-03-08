#include "stdafx.h"
#include "ZChatCmd.h"
#include "MDebug.h"
#include "ZMyInfo.h"

void ZImplodeChatCmdArgs(char* szOut, size_t maxlen, const int argc, char **const argv, int nFirstIndex)
{
	szOut[0] = 0;

	for (int i = nFirstIndex; i < argc; i++)
	{
		strcat_safe(szOut, maxlen, argv[i]);

		if (i != (argc-1)) strcat_safe(szOut, maxlen, " ");
	}
}

int IsSpace(int c)
{
	if (c < 0) return 0;
	if (c > 127) return 0;
	return isspace(c);
}

ZChatCmd::ZChatCmd(int nID, const char* szName, u32 flag,
	int nMinArgs, int nMaxArgs, bool bRepeatEnabled,
					const char* szUsage, const char* szHelp)
{
	m_nID = nID;

	m_nFlag = flag;

	m_nMinArgs = nMinArgs;
	m_nMaxArgs = nMaxArgs;

	m_bRepeatEnabled = bRepeatEnabled;

	strcpy_safe(m_szName, szName);
	strcpy_safe(m_szUsage, szUsage);
	strcpy_safe(m_szHelp, szHelp);
}

void ZChatCmd::OnProc(const char* line, const int argc, char **const argv)
{
	if (m_fnProc) {
		m_fnProc(line, argc, argv);
	}
}



ZChatCmd* ZChatCmdManager::MakeArgv(const char* szLine, ZChatCmdArgvInfo* pAI)
{
	int nLen = (int)strlen(szLine);
	if (nLen >= 2048) NULL;

	char szBuffer[2048];
	strcpy_safe(szBuffer, szLine);

	for (int pos = nLen-1; pos >= 0; pos--)
	{
		if (IsSpace(szBuffer[pos])) szBuffer[pos] = '\0';
		else break;
	}


	ZChatCmd* pCmd = NULL;

	int c;
	char* scp;
	char* dcp;
	char* dlim;
	char* arg;

	int nArgcMax, nCmdArgcMax;
	nCmdArgcMax = nArgcMax = (sizeof(pAI->cargv) / sizeof(char*));
	

	scp = szBuffer;
	dcp = pAI->argbuf;
	dlim = dcp + sizeof(pAI->argbuf) - 1;

	for (pAI->cargc = 0; pAI->cargc < nArgcMax; )
	{
		for ( ; ; scp++)
		{
			c = *scp;
			if (IsSpace(c)) continue;

			if ( (c == '\0') || (c == '\n') )
			{
				pAI->cargv[pAI->cargc] = NULL;
				return pCmd;
			}
			break;
		}
		arg = dcp;
		pAI->cargv[pAI->cargc] = arg;
		(pAI->cargc)++;

		for ( ; ; )
		{
			c = *scp;
			if ( (c == '\0') || (c == '\n')) break;

			if (pAI->cargc != nCmdArgcMax)
			{
				if (IsSpace(c)) break;
			}
			scp++;

			// TODO: Handle a quoted string as a single argument.
			if (dcp >= dlim) return NULL;
			*dcp++ = c;
		}

		*dcp++ = '\0';

		if (pAI->cargc == 1)
		{
			pCmd = GetCommandByName(pAI->cargv[0]);
			if (pCmd != NULL) 
			{
				if (pCmd->GetMaxArgs() != ARGVNoMin)
				{
					nCmdArgcMax = pCmd->GetMaxArgs() + 1;
				}
			}
			else
			{
				return NULL;
			}
		}
	}
	
	return NULL;
}


void ZChatCmdManager::AddCommand(int nID, const char* szName, ZChatCmdProc fnProc, u32 flag,
	int nMinArgs, int nMaxArgs, bool bRepeatEnabled, const char* szUsage, const char* szHelp)
{
	char szLwrName[256];
	strcpy_safe(szLwrName, szName);
	_strlwr_s(szLwrName);

	ZChatCmd Cmd(nID, szLwrName, flag, nMinArgs, nMaxArgs, bRepeatEnabled,szUsage, szHelp);
	
	Cmd.SetProc(fnProc);
	m_CmdMap.emplace(szLwrName, std::move(Cmd));
}

bool ZChatCmdManager::DoCommand(const char* szLine, ZChatCmdFlag nCurrFlag, u32 nInputFlag)
{
	if ((szLine == 0) || (szLine[0] == 0)) return false;
	
	ZChatCmdArgvInfo ai;
	memset(&ai, 0, sizeof(ZChatCmdArgvInfo));

	ZChatCmd* pCmd = MakeArgv(szLine, &ai);
	if (pCmd != NULL)
	{
		int MinArgs = pCmd->GetMinArgs() + 1;
		int MaxArgs = pCmd->GetMaxArgs() + 1;

		if ((MinArgs != 0 && ai.cargc < MinArgs) ||
			(MaxArgs != 0 && ai.cargc > MaxArgs))
		{
			ZChatOutput(pCmd->GetUsage());
			return true;
		}

		if (pCmd->GetFlag() & CCF_ADMIN)
		{
			if ((nInputFlag & CIF_ADMIN) == false) return false;
		}
		else if (pCmd->GetFlag() & CCF_TEST)
		{
			if ((nInputFlag & CIF_TESTER) == false) return false;
		}

		// Common
		if ( pCmd->GetFlag() & nCurrFlag)
		{
			pCmd->OnProc(szLine, ai.cargc, ai.cargv);
			return true;
		}
		// Admin
		else if ((CIF_ADMIN & nInputFlag) && (pCmd->GetFlag() & CCF_ADMIN))
		{
			if ( IsAdminGrade(ZGetMyInfo()->GetUGradeID()) )
			{
				pCmd->OnProc(szLine, ai.cargc, ai.cargv);
				return true;
			}
		}
		// Test
		else if ((CIF_TESTER & nInputFlag) && (pCmd->GetFlag() & CCF_TEST))
		{
			pCmd->OnProc(szLine, ai.cargc, ai.cargv);
			return true;
		}
	}

	return false;
}

bool ZChatCmdManager::IsRepeatEnabled(const char* szLine)
{
	if (szLine == 0 || szLine[0] == 0)
		return false;
	
	ZChatCmdArgvInfo ai;
	memset(&ai, 0, sizeof(ZChatCmdArgvInfo));

	ZChatCmd* pCmd = MakeArgv(szLine, &ai);
	if (pCmd != NULL)
	{
		return pCmd->GetRepeatEnabled();
	}

	return false;
}

ZChatCmdManager::ZChatCmdManager() = default;
ZChatCmdManager::~ZChatCmdManager() = default;

ZChatCmd* ZChatCmdManager::GetCommandByName(const char* szName)
{
	char szLwrName[256];
	strcpy_safe(szLwrName, szName);
	_strlwr_s(szLwrName);

	char szCmdName[256]; szCmdName[0] = 0;

	auto itorAlias = m_AliasMap.find(string(szLwrName));
	if (itorAlias != m_AliasMap.end())
	{
		strcpy_safe(szCmdName, ((*itorAlias).second).c_str());
	}
	else
	{
		strcpy_safe(szCmdName, szLwrName);
	}

	auto pos = m_CmdMap.find(szCmdName);
	if (pos != m_CmdMap.end())
	{
		return &pos->second;
	}
	else
	{
		return nullptr;
	}
}

ZChatCmd* ZChatCmdManager::GetCommandByID(int nID)
{
	for (auto&& ChatCmd : MakePairValueAdapter(m_CmdMap))
	{
		if (ChatCmd.GetID() == nID) {
			return &ChatCmd;
		}
	}
	return nullptr;
}

void ZChatCmdManager::AddAlias(const char* szNewCmdName, const char* szTarCmdName)
{
	char szLwrName[256];
	strcpy_safe(szLwrName, szNewCmdName);
	_strlwr_s(szLwrName);

	m_AliasMap.emplace(szLwrName, szTarCmdName);
}




#define ZCMD_TOK_CMD		"CMD"
#define ZCMD_TOK_ALIAS		"ALIAS"
#define ZCMD_TOK_ATTR_ID	"id"
#define ZCMD_TOK_ATTR_NAME	"name"
#define ZCMD_TOK_ATTR_USAGE	"usage"
#define ZCMD_TOK_ATTR_HELP	"help"


void ZCmdXmlParser::ParseRoot(const char* szTagName, MXmlElement* pElement)
{
	if (!_stricmp(szTagName, ZCMD_TOK_CMD)) 
	{
		ParseCmd(pElement);
	}
}

void ZCmdXmlParser::ParseCmd(MXmlElement* pElement)
{
	char szTemp[256]="";
	int n = 0;
	char szAttrValue[256];
	char szAttrName[64];
	char szTagName[128];

	CmdStr NewCmdStr;

	int nID = 0;

	int nAttrCount = pElement->GetAttributeCount();
	for (int i = 0; i < nAttrCount; i++)
	{
		pElement->GetAttribute(i, szAttrName, szAttrValue);
		if (!_stricmp(szAttrName, ZCMD_TOK_ATTR_ID))
		{
			nID =  atoi(szAttrValue);
		}
		else if (!_stricmp(szAttrName, ZCMD_TOK_ATTR_NAME))
		{
			strcpy_safe(NewCmdStr.szName, ZGetStringResManager()->GetStringFromXml(szAttrValue));
		}
		else if (!_stricmp(szAttrName, ZCMD_TOK_ATTR_USAGE))
		{
			strcpy_safe(NewCmdStr.szUsage, ZGetStringResManager()->GetStringFromXml(szAttrValue));
		}
		else if (!_stricmp(szAttrName, ZCMD_TOK_ATTR_HELP))
		{
			strcpy_safe(NewCmdStr.szHelp, ZGetStringResManager()->GetStringFromXml(szAttrValue));
		}
	}

	int iChildCount = pElement->GetChildNodeCount();
	MXmlElement chrElement;
	for (int i = 0; i < iChildCount; i++)
	{
		chrElement = pElement->GetChildNode(i);
		chrElement.GetTagName(szTagName);
		if (szTagName[0] == '#') continue;

		if (!_stricmp(szTagName, ZCMD_TOK_ALIAS))
		{
			int nAttrCount = chrElement.GetAttributeCount();
			for (int i = 0; i < nAttrCount; i++)
			{
				chrElement.GetAttribute(i, szAttrName, szAttrValue);
				if (!_stricmp(szAttrName, ZCMD_TOK_ATTR_NAME))
				{
					string str = ZGetStringResManager()->GetStringFromXml(szAttrValue);
					if (str.size() > 0)
					{
						NewCmdStr.vecAliases.push_back(str);
					}
				}
			}
		}
	}

	if (nID != 0)
	{
		m_CmdMap.emplace(nID, NewCmdStr);
	}
}

ZCmdXmlParser::CmdStr* ZCmdXmlParser::GetStr(int nID)
{
	auto itor = m_CmdMap.find(nID);
	if (itor != m_CmdMap.end())
	{
		return &itor->second;
	}

	return nullptr;
}