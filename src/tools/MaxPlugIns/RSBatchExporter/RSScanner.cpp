#include <stdlib.h>
#include "RSScanner.h"
#include "string.h"

RSScanner::RSScanner()
{

}

RSScanner::~RSScanner()
{

}

bool RSScanner::Open(FILE *file)
{
	RSScanner::file=file;
	m_szLineBuffer[0]=0;
	m_CurrentPosition=m_szLineBuffer;
	m_nLineNumber=0;
	ReadToken();
	return true;
}

bool RSScanner::ReadLine()
{
	if(feof(file)) return false;
	if(fgets(m_szLineBuffer,sizeof(m_szLineBuffer),file)==NULL) return false;
	m_nLineNumber++;
	m_CurrentPosition=m_szLineBuffer;
	return true;
}

void RSScanner::ReadToken()
{
	m_szBuffer[0]=0;
	m_TokenType=RTOKEN_NULL;

	if(feof(file)&&(*m_CurrentPosition==0)) return;

	if(*m_CurrentPosition==0) 
		if(!ReadLine()) return;

	if(*m_CurrentPosition)		// skip spaces
	{
		char *spaces="\t\n\r ";
		while(strchr(spaces,*m_CurrentPosition)!=NULL)
		{
			m_CurrentPosition++;
			if(!*m_CurrentPosition)
			{
				ReadToken();
				return;
			}
		}
	}
	
	switch(*m_CurrentPosition)
	{
	case ';' :
		{
			m_TokenType=RTOKEN_SEMICOLON;
			strcpy(m_szBuffer,";");
			m_CurrentPosition++;
		}break;
	case '/' :
		{
			switch(*(m_CurrentPosition+1))
			{
			case '/' :
				{
					m_CurrentPosition=m_szLineBuffer+strlen(m_szLineBuffer);
					ReadToken();
					return;
				}break;
			case '*' :
				{
					for(;m_CurrentPosition<m_szLineBuffer+strlen(m_szLineBuffer)-2;m_CurrentPosition++)
					{
						if(((*m_CurrentPosition)=='*')&&((*(m_CurrentPosition+1))=='/'))
						{
							m_CurrentPosition+=2;
							ReadToken();
							return;
						}
					}
					while(ReadLine())
					{
						char *pos=strstr(m_szLineBuffer,"*/");
						if(pos)
						{
							m_CurrentPosition=pos+2;
							ReadToken();
							return;
						}
					}
					
				}break;
			default : goto DEFAULT;
			}
			m_CurrentPosition++;

		}break;
	case '"' :
		{
			char *dest=m_szBuffer;
			for(m_CurrentPosition++;m_CurrentPosition<m_szLineBuffer+strlen(m_szLineBuffer)-1;m_CurrentPosition++)
			{
				if((*m_CurrentPosition)=='"')
				{
					*dest=0;
					m_CurrentPosition++;
					m_TokenType=RTOKEN_CONSTANTSTRING;
					return;
				}
				*dest=*m_CurrentPosition;
				dest++;
			}
		}break;
	default:
DEFAULT:
		{
		char *end=m_CurrentPosition;
		char *seperators="\t\n /;""";
		while(strchr(seperators,*end)==NULL) end++;

		int nLength=end-m_CurrentPosition;
		memcpy(m_szBuffer,m_CurrentPosition,nLength);
		m_szBuffer[nLength]=0;
		m_TokenType=RTOKEN_STRING;

		if(strchr(m_szBuffer,'.') && sscanf(m_szBuffer,"%f",&m_fRealNumber))
			m_TokenType=RTOKEN_REALNUMBER;
		else
		if(sscanf(m_szBuffer,"%d",&m_nInteger))
			m_TokenType=RTOKEN_NUMBER;

		m_CurrentPosition=end;
		}break;
	}
}

bool RSScanner::GetToken(char *pBuffer,int nBufferSize)
{
	if(nBufferSize<(int)strlen(m_szBuffer))
		return false;
	strcpy(pBuffer,m_szBuffer);
	return true;
}

void RSScanner::GetToken(float *realnumber)
{
	*realnumber=m_fRealNumber;
}

void RSScanner::GetToken(int *integer)
{
	*integer=m_nInteger;
}

char *RSScanner::GetToken()
{
	return m_szBuffer;
}

int RSScanner::GetCurrentLineNumber()
{
	return m_nLineNumber;
}
