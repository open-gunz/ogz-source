#pragma once

#include <StringView.h>

namespace MClipboard
{

bool Get(void* WindowHandle, char* Output, size_t OutputSize);
bool Get(void* WindowHandle, wchar_t* Output, size_t OutputSize);
bool Set(void* WindowHandle, const StringView& Str);
bool Set(void* WindowHandle, const WStringView& Str);

}