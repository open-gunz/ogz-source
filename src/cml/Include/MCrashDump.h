#ifndef _MCRASHDUMP_H
#define _MCRASHDUMP_H

#ifdef _WIN32

#include "MDebug.h"
#include "Shlwapi.h"
#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable: 4091)
#endif
#include <imagehlp.h>
#ifdef _MSC_VER
#pragma warning(pop)
#endif


DWORD CrashExceptionDump(PEXCEPTION_POINTERS ExceptionInfo, const char* szDumpFileName);

#endif

#endif