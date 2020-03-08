// MAIET String Extension
// 스트링 관련 헬퍼 펑션

#ifndef MSTREX_H
#define MSTREX_H

#include <ctype.h>


/// 타겟 스트링의 맥스멈 길이만큼까지만 스트링을 카피한다. ( NULL문자까지 기입 )
/// @param szDest	타겟 문자 버퍼
/// @param nDestLen	타겟 문자 버퍼 크기
/// @param szSource	소스 문자열
void MStrNCpy(char* szDest, int nDestLen, const char* szSource);

/// 스트링에서 단어단위로 읽어온다
class MStringCutter {
protected:
	static void SkipSpaces(char **szString) {
		for (; **szString && isspace(**szString); (*szString)++);
	}

public:
	static char* GetOneArg(char *pszArg, char *pszOutArg) {
		SkipSpaces(&pszArg);
		while(*pszArg && !isspace(*pszArg)) {
			*(pszOutArg++) = *pszArg;
			pszArg++;
		}
		*pszOutArg = '\0';
		return pszArg;
	}

	static char* GetTwoArgs(char* pszArg, char* pszOutArg1, char* pszOutArg2) {
		return GetOneArg(GetOneArg(pszArg, pszOutArg1), pszOutArg2);
	}
};

#endif