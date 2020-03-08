#pragma once

enum MCountry
{
	MC_INVALID			= 0,
	MC_KOREA			= 82,
	MC_US				= 1,
	MC_JAPAN			= 81,
	MC_BRAZIL			= 55,
	MC_INDIA			= 91,
};

enum MLanguage
{
	ML_INVALID				= 0x00,
	ML_CHINESE				= 4,
	ML_CHINESE_TRADITIONAL	= 1,
	ML_KOREAN				= 0x12,
	ML_ENGLISH				= 0x9,
	ML_JAPANESE				= 0x11,
	ML_BRAZIL				= 0x16,
	ML_INDIA				= 0x21,
};

class MBaseLocale
{
private:
	void InitLanguageFromCountry();
protected:
	MCountry			m_nCountry;
	MLanguage			m_nLanguage;

	bool				m_bIsComplete;

	virtual bool OnInit() = 0;
public:
	MBaseLocale();
	virtual ~MBaseLocale();
	bool Init(MCountry nCountry);

	MCountry GetCountry()		{ return m_nCountry; }
	MLanguage GetLanguage()		{ return m_nLanguage; }

	bool bIsComplete()			{ return m_bIsComplete; }
};

MCountry GetCountryID( const char* pCountry );
MLanguage GetLanguageID( const char* pLanguage );