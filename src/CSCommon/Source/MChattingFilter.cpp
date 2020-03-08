#include "stdafx.h"
#include "MChattingFilter.h"
#include "MZFileSystem.h"
#include "SafeString.h"

#ifdef _DEBUG
#include <direct.h>
#endif

MChattingFilter::MChattingFilter()
{
	m_strRemoveTokSkip		= "`-=\\[];'/~!@#$%^&*()_+|{}:\"<>";
	m_strRemoveTokInvalid	= "'";

	for ( int i = 0;  i <= 32;  i++)
	{
		m_strRemoveTokSkip		+= i;
		m_strRemoveTokInvalid	+= i;
	}

	m_szLastFilterdStr[ 0] = 0;
}



MChattingFilter::~MChattingFilter()
{
	m_AbuseMap.clear();
	m_InvalidNameMap.clear();
}


bool MChattingFilter::LoadFromFile( MZFileSystem* pfs, const char* szFileName)
{
	if ( szFileName == 0)
		return false;

	MZFile mzf;
	if ( !mzf.Open( szFileName, pfs)) 
		return false;


	char *buffer;
	char* tembuf;
	buffer = new char[ mzf.GetLength() + 1];
	mzf.Read( buffer, mzf.GetLength());
	buffer[ mzf.GetLength()] = 0;
	tembuf = buffer;


	m_AbuseMap.clear();

	while ( 1)
	{
		char szType[ 5];
		char szText[ 25];

		GetLine( tembuf, szType, szText);


		if ( strlen( szType) == 0)
			continue;

		if ( _stricmp( szType, "END") == 0)
			break;


		SkipBlock( tembuf);


		if ( strcmp( szType, "1") == 0)
			m_AbuseMap.push_back( szText);

		else if ( strcmp( szType, "2") == 0)
			m_InvalidNameMap.push_back( szText);
	}

	mzf.Close();
	tembuf = 0;
	delete [] buffer;


	return true;
}


void MChattingFilter::GetLine( char*& prfBuf, char* szType, char* szText)
{
	bool bType = true;
	int  nTypeCount = 0;
	int  nTextCount = 0;

	*szType = 0;
	*szText = 0;

	while ( 1)
	{
		char ch = *prfBuf++;

		if ( (ch == 0) || (ch == '\n') || (ch == '\r'))
			break;
			

		if ( ch == ',')
		{
			bType = false;

			continue;
		}


		if ( bType)
		{
			*(szType + nTypeCount++) = ch;
			*(szType + nTypeCount) = 0;
		}
		else
		{
			*(szText + nTextCount++) = ch;
			*(szText + nTextCount) = 0;
		}
	}
}


void MChattingFilter::SkipBlock( char*& prfBuf)
{
	for ( ; *prfBuf != '\n'; ++prfBuf )
		NULL;
		
	++prfBuf;
}

bool MChattingFilter::IsValidChatting( const char* szText)
{
	if ( szText == 0)
		return false;

	const std::string str = PreTranslate( szText);

	for ( STRFILTER_ITR itr = m_AbuseMap.begin();  itr != m_AbuseMap.end();  itr++)
	{
		if ( FindInvalidWord( (*itr).c_str(), str))
			return false;
	}

	return true;
}

bool MChattingFilter::IsValidName( const char* szText)
{
	if ( szText == 0)
		return false;

	const std::string str = PreTranslate( szText);

	for ( STRFILTER_ITR itr = m_AbuseMap.begin();  itr != m_AbuseMap.end();  itr++)
	{
		if ( FindInvalidWord( (*itr).c_str(), str))
			return false;
	}

	for ( STRFILTER_ITR itr = m_InvalidNameMap.begin();  itr != m_InvalidNameMap.end();  itr++)
	{
		if ( FindInvalidWord( (*itr).c_str(), str))
			return false;
	}

	if ( FindInvalidChar( szText))
		return false;


	return true;
}

std::string MChattingFilter::PreTranslate( const std::string& strText)
{
	return strText;
}

bool MChattingFilter::FindInvalidWord( const std::string& strWord, const std::string& strText)
{
	size_t nWordLen = strWord.length(); 

	int nHead = 0;

	auto it = std::search(strText.begin(), strText.end(), strWord.begin(), strWord.end());
	if (it == strText.end())
	{
		return false;
	}

	strcpy_safe(m_szLastFilterdStr, strWord);
	return true;
}

bool MChattingFilter::FindInvalidChar( const std::string& strText)
{
	for ( int i = 0;  i < (int)strText.size();  i++)
	{
		char ch1 = *(strText.c_str() + i);

		for ( int j = 0;  j < (int)m_strRemoveTokInvalid.size();  j++)
		{
			char ch2 = *(m_strRemoveTokInvalid.c_str() + j);

			if ( ch1 == ch2)
			{
				sprintf_safe( m_szLastFilterdStr, "%c", ch1);

				return true;
			}
		}
	}

	return false;
}
