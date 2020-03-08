#ifndef CMLEXICALANALYZER_H
#define CMLEXICALANALYZER_H

#include "CMPtrList.h"

class CMLexicalAnalyzer{
	CMPtrList<char>		m_Tokens;
	//char				m_szOriginal[256];
public:
	CMLexicalAnalyzer(void);
	~CMLexicalAnalyzer(void);

	bool Create(const char *pStr);
	void Destroy(void);
	char *GetByStr(int i);
	int GetByInt(int i);
	long GetByLong(int i);
	float GetByFloat(int i);
	int GetCount(void);

	bool IsNumber(int i);

	//char *GetOrgStr(void);
};

#endif
