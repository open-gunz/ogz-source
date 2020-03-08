#pragma once

#include <string>
#include "GlobalTypes.h"

using LANGID = u16;

class MLocale
{
private:
	static int			GetCodePageFromCharset(int nCharset);
#ifdef WIN32
	static std::string	TransCode(const wchar_t *pwszString, int nCodePage = 65001);
	static std::string	TransCode(const char *pszString, int nOldCodePage = 0, int nNewCodePage = 65001);
#endif
public:
	static int			GetCharsetFromLang(LANGID langid);
	static int			GetCodePageFromLang(LANGID langid);
	static std::string	ConvUTF8ToAnsi(const char* pszString, LANGID langid = 0x12);
	static std::string	ConvAnsiToUTF8(const char* pszString, LANGID langid = 0x12);
};