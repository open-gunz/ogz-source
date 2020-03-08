// RSScanner.h

#ifndef __RSSCANNER_H
#define __RSSCANNER_H

#include <stdio.h>

enum RTOKENTYPE
{
	RTOKEN_NULL,
	RTOKEN_STRING,
	RTOKEN_CONSTANTSTRING,
	RTOKEN_NUMBER,
	RTOKEN_REALNUMBER,
	RTOKEN_SEMICOLON,
	RTOKEN_REMARK
};

class RSScanner  
{
public:
	RSScanner();
	virtual ~RSScanner();

	bool Open(FILE *file);

	void ReadToken();
	RTOKENTYPE GetTokenType() { return m_TokenType; }

	char *GetToken();
	bool GetToken(char *buffer,int nBufferSize);
	void GetToken(int *nReturn);
	void GetToken(float *fReturn);
	int GetCurrentLineNumber();

private:
	bool ReadLine();

	FILE *file;

	float m_fRealNumber;
	int m_nInteger;
	int m_nLineNumber;

	char m_szBuffer[256];
	char m_szLineBuffer[256];
	char *m_CurrentPosition;

	RTOKENTYPE m_TokenType;
};

#endif
