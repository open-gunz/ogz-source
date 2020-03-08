#ifdef _WIN32
#pragma once

#include <string>

using DWORD = unsigned long;
using LPEXCEPTION_POINTERS = struct _EXCEPTION_POINTERS*;

#ifdef __cplusplus
extern "C"
{
#endif

DWORD GetCrashInfo(LPEXCEPTION_POINTERS exceptionInfo, std::string& str);

#ifdef __cplusplus
}
#endif
#endif
