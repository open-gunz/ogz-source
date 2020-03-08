#pragma once

#include <list>
#include <string>

using STRFILTER_MAP = std::list<std::string>;
using STRFILTER_ITR = STRFILTER_MAP::iterator;

class MChattingFilter
{
private:
	STRFILTER_MAP m_AbuseMap;
	STRFILTER_MAP m_InvalidNameMap;

	std::string	m_strRemoveTokSkip;
	std::string	m_strRemoveTokInvalid;

	char m_szLastFilterdStr[256];

public:
	MChattingFilter();
	~MChattingFilter();

	static MChattingFilter* GetInstance()
	{
		static MChattingFilter ChattingFilter;
		return &ChattingFilter;
	}

	bool LoadFromFile(class MZFileSystem* pfs, const char* szFileName);
	bool IsValidChatting(const char* strText);
	bool IsValidName(const char* strText);
	const char* GetLastFilteredStr() { return m_szLastFilterdStr; }
	bool FindInvalidChar(const std::string& strText);

protected:
	void GetLine(char*& prfBuf, char* szType, char* szText);
	void SkipBlock(char*& prfBuf);
	std::string PreTranslate(const std::string& strText);
	bool FindInvalidWord(const std::string& strWord, const std::string& strText);
};

inline MChattingFilter* MGetChattingFilter() { return MChattingFilter::GetInstance(); }