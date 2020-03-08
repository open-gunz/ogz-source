#include "stdafx.h"
#include "MBaseLocale.h"

MBaseLocale::MBaseLocale()
{
	m_bIsComplete = false;
}

MBaseLocale::~MBaseLocale()
{

}

bool MBaseLocale::Init(MCountry nCountry)
{
	m_nCountry = nCountry;
	InitLanguageFromCountry();

	m_bIsComplete = OnInit();

	return m_bIsComplete;
}

void MBaseLocale::InitLanguageFromCountry()
{
	switch (m_nCountry)
	{
	case MC_KOREA:		m_nLanguage = ML_KOREAN;		break;
	case MC_US:			m_nLanguage = ML_ENGLISH;		break;
	case MC_JAPAN:		m_nLanguage = ML_JAPANESE;		break;
	case MC_BRAZIL:		m_nLanguage = ML_BRAZIL;		break;
	case MC_INDIA:		m_nLanguage = ML_INDIA;			break;
	default:
		{
			_ASSERT(0);
		}
	};
}

MCountry GetCountryID( const char* pCountry )
{
	ASSERT( (0 != pCountry) && (0 < strlen(pCountry)) );

	if( 0 == pCountry )			return MC_INVALID;
	if( 0 == strlen(pCountry) ) return MC_INVALID;

	if( 0 == _stricmp("kor", pCountry) )
		return MC_KOREA;
	else if( 0 == _stricmp("international", pCountry) )
		return MC_US;
	else if( 0 == _stricmp("jpn", pCountry) )
		return MC_JAPAN;
	else if( 0 == _stricmp("brz", pCountry) )
		return MC_BRAZIL;
	else if( 0 == _stricmp("ind", pCountry) )
		return MC_INDIA;

	return MC_INVALID;
}

MLanguage GetLanguageID( const char* pLanguage )
{
	ASSERT( (0 != pLanguage) && (0 < strlen(pLanguage)) );

	if( 0 == pLanguage )		 return ML_INVALID;
	if( 0 == strlen(pLanguage) ) return ML_INVALID;

	if( 0 != _stricmp("kor", pLanguage) )
		return ML_KOREAN;
	else if( 0 != _stricmp("international", pLanguage) )
		return ML_ENGLISH;
	else if( 0 != _stricmp("jpn", pLanguage) )
		return ML_JAPANESE;
	else if( 0 != _stricmp("brz", pLanguage) )
		return ML_BRAZIL;
	else if( 0 != _stricmp("ind", pLanguage) )
		return ML_INDIA;

	return ML_INVALID;
}