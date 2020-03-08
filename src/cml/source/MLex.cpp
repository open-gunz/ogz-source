#include "stdafx.h"
#include <ctype.h>
#include <string.h>
#include "MLex.h"

/*
const CMDNODE SampleTable[] = {
	{ "RESERVED",	0, 0 },
	{ "status",		0, 0 },
	{ "users",		0, 0 },
	{ "\n", 0, 0 }
};
*/

void MLex::SkipSpaces(char **szString)
{
	for (; **szString && (unsigned char)(**szString)<127 && isspace(**szString); (*szString)++);
}

char* MLex::GetOneArg(char *pszArg, char *pszOutArg)
{
	SkipSpaces(&pszArg);

	while( *pszArg && ( (unsigned char)(*pszArg)>=127 || !isspace(*pszArg) ) ) {
		if ( (unsigned char)(*pszArg)<127 && isalpha(*pszArg) )
			*(pszOutArg++) = MLEX_LOWER(*pszArg);
		else
			*(pszOutArg++) = *pszArg;
		pszArg++;
	}

	*pszOutArg = '\0';

	return pszArg;
}

char* MLex::GetTwoArgs(char* pszArg, char* pszOutArg1, char* pszOutArg2)
{
	return GetOneArg(GetOneArg(pszArg, pszOutArg1), pszOutArg2);
}

int MLex::FindCommand(char* szString)
{
	if (m_pCmdTable == NULL)
		return -1;

	int nCmd, nLen;
	for (nLen=strlen(szString),nCmd=0; *(m_pCmdTable[nCmd]).szCmd != '\n'; nCmd++) {
		if (!strncmp(szString, (m_pCmdTable[nCmd]).szCmd, nLen))
			return nCmd;
	}
	return -1;
}

void MLex::Interprete(void* pData, char* pszString)
{
	if ((m_pCmdTable == nullptr) || (pszString == nullptr))
		return; 

	char szCmd[128];
	char* pszNextArg = GetOneArg(pszString, szCmd);

	int nCmd = FindCommand(szCmd);
	if (nCmd == -1)
		return;

	if (m_pCmdTable[nCmd].pProc)
		(*m_pCmdTable[nCmd].pProc)(pData, m_pCmdTable[nCmd].nLevel, pszNextArg);
}
