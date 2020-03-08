#include "stdafx.h"
#include "MUrl.h"

#ifdef WIN32

#include <Windows.h>
#include <WinInet.h>
#include <Shlwapi.h>

bool MUrl::GetPath(char * Output, size_t OutputSize, const char * URL)
{
	URL_COMPONENTS uc{};
	uc.dwStructSize = sizeof(uc);
	uc.lpszUrlPath = Output;
	uc.dwUrlPathLength = OutputSize;

	if (!InternetCrackUrl(URL, lstrlen(URL), ICU_DECODE, &uc)) {
		return false;
	}
	PathStripPath(Output);
	return false;
}

#else

bool MUrl::GetPath(char * Output, size_t OutputSize, const char * URL) {
	// TODO
	return false;
}

#endif