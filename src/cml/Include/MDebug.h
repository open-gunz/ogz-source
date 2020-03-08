#pragma once

#include "GlobalTypes.h"

#define MASSERT(x) _ASSERT(x)

#ifndef _DEBUG
#define IF_DEBUG(expr)
#else
#define IF_DEBUG(expr) do { expr } while (false)
#endif

#define MLOGSTYLE_FILE 0x0001
#define MLOGSTYLE_DEBUGSTRING 0x0002

#define MLOG_DEFAULT_HISTORY_COUNT	10

bool IsLogAvailable();

void InitLog(int logmethodflags = MLOGSTYLE_DEBUGSTRING, const char* pszLogFileName = "mlog.txt");

#if defined(_DEBUG) || defined(DEBUG_FAST)
void DMLog(const char* Format, ...);
#else
static inline void DMLog(...) {}
#endif

void MLogFile(const char* Msg);
void MLog(const char* Format,...);
#define mlog MLog

extern void (*CustomLog)(const char *Msg);

#ifdef _WIN32
void MMsg(const char *pFormat,...);
#endif

using LPEXCEPTION_POINTERS = struct _EXCEPTION_POINTERS*;
void MFilterException(LPEXCEPTION_POINTERS p);

void MInitProfile();
void MBeginProfile(int nIndex,const char *szName);
void MEndProfile(int nIndex);
void MSaveProfile(const char *file);

struct ProfilerGuard
{
	ProfilerGuard() = default;
	~ProfilerGuard();
	ProfilerGuard(ProfilerGuard&& src) : Active(src.Active) { src.Active = false; }

	bool Active = true;
};

#ifdef _DEBUG
ProfilerGuard MBeginProfile(const char *szName);
void MEndProfile(ProfilerGuard& guard);
void MCheckProfileCount();
#else
inline char MBeginProfile(const char *szName) { return{}; }
inline void MEndProfile(char guard) { (void)guard; }
inline void MCheckProfileCount() {}
#endif