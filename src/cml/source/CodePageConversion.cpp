#include "CodePageConversion.h"

#ifdef _WIN32

#include <Windows.h>

size_t CodePageConversion(wchar_t* Output, size_t OutputSize, const char* Input, int CodePage)
{
	auto Len = MultiByteToWideChar(CodePage, 0,
		Input, -1,
		Output, OutputSize);

	// 0 indicates error.
	// This is distinct from an empty string, which would cause it to return 1 instead since it'd
	// write just the null terminator in that case.
	if (Len == 0)
		return ConversionError;

	// The return value is chars written including null terminator (when you pass -1 for
	// cbMultiByte), so we need to subtract one.
	return size_t(Len) - 1;
}

size_t CodePageConversion(char* Output, size_t OutputSize, const wchar_t* Input, int CodePage)
{
	auto Len = WideCharToMultiByte(CodePage, 0,
		Input, -1,
		Output, OutputSize,
		nullptr, nullptr);

	if (Len == 0)
		return ConversionError;

	return size_t(Len) - 1;
}

#else

#endif