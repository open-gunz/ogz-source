#pragma once

#define MLEX_LOWER(c)   (((c)>='A'  && (c) <= 'Z') ? ((c)+('a'-'A')) : (c))
#define MLEX_UPPER(c)   (((c)>='a'  && (c) <= 'z') ? ((c)+('A'-'a')) : (c))


typedef bool(MLEXPROC)(void* pData, int nLevel, char* pszArg);


struct MLEXNODE {
	char*		szCmd;
	int			nLevel;
	MLEXPROC*	pProc;		
};


class MLex {
private:
	MLEXNODE*	m_pCmdTable;

private:
	void SkipSpaces(char **ppszString);
	int FindCommand(char* szString);

public:
	MLex(MLEXNODE* pNode = 0)		{ m_pCmdTable = pNode; }
	virtual ~MLex()		{}

	char* GetOneArg(char *pszArg, char *pszOutArg);
	char* GetTwoArgs(char* pszArg, char* pszOutArg1, char* pszOutArg2);

	void SetCmdTable(MLEXNODE* pCmdTable) { m_pCmdTable = pCmdTable; }
	MLEXNODE* GetCmdTable() { return m_pCmdTable; }
	void Interprete(void* pData, char* pszString);
};

template <size_t size1, size_t size2>
bool SplitValue(const char* pszSource, const char* pszSeperator, char(&pszField)[size1], char(&pszValue)[size2])
{
	const char* pszCursor = strstr(pszSource, pszSeperator);
	if (pszCursor == nullptr) return false;

	int nFieldLen = static_cast<int>(pszCursor - pszSource);
	if (nFieldLen <= 0) return false;

	int nValueBegin = static_cast<int>(pszCursor - pszSource) + 1;
	int nValueEnd = static_cast<int>(strlen(pszSource));
	if (nValueEnd - nValueBegin <= 0) return false;

	strncpy_safe(pszField, pszSource, nFieldLen + 1);
	strncpy_safe(pszValue, pszSource + nValueBegin, nValueEnd - nValueBegin + 1);

	return true;
}
