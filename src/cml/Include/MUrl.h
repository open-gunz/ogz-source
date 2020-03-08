#pragma once

namespace MUrl
{
bool GetPath(char* Output, size_t OutputSize, const char* URL);
template <size_t Size> auto GetPath(char(&Output)[Size], const char* URL) {
	return GetPath(Output, Size, URL); }
}