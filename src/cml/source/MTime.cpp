#include "stdafx.h"
#include "MTime.h"
#include "GlobalTypes.h"

#ifdef WIN32
#include <Windows.h>
#include <mmsystem.h>
#else
#include <time.h>
#endif

u64 GetGlobalTimeMSDefault()
{
#ifdef WIN32
	return timeGetTime();
#else
	timespec spec;
	clock_gettime(CLOCK_REALTIME, &spec);

	return spec.tv_sec * 1000 + spec.tv_nsec / 1000000;
#endif
}

u64 (*GetGlobalTimeMS)() = GetGlobalTimeMSDefault;

float GetGlobalTime()
{
	return GetGlobalTimeMS() / 1000.f;
}

u32 MTime::Random()
{
	int lo, hi, test;

	hi = seed / q;
	lo = seed%q;

	test = a*lo - r*hi;

	if (test > 0)
		seed = test;
	else
		seed = test + m;

	return seed;
}

int MTime::MakeNumber(int nFrom, int nTo)
{
	if (nFrom > nTo) {
		int tmp = nFrom;
		nFrom = nTo;
		nTo = tmp;
	}
	return ((Random() % (nTo - nFrom + 1)) + nFrom);
}

void MTime::GetTime(MTime::timeval *t)
{
	auto millisec = GetGlobalTimeMS();

	t->tv_sec = (int)(millisec / 1000);
	t->tv_usec = millisec % 1000;
}

MTime::timeval MTime::TimeSub(MTime::timeval Src1, MTime::timeval Src2)
{
	MTime::timeval null_time = { 0, 0 };
	MTime::timeval Result;

	if (Src1.tv_sec < Src2.tv_sec)
		return null_time;
	else if (Src1.tv_sec == Src2.tv_sec) {
		if (Src1.tv_usec < Src2.tv_usec)
			return null_time;
		else {
			Result.tv_sec = 0;
			Result.tv_usec = Src1.tv_usec - Src2.tv_usec;
			return Result;
		}
	}
	else {			/* Src->tv_sec > Src2->tv_sec */
		Result.tv_sec = Src1.tv_sec - Src2.tv_sec;
		if (Src1.tv_usec < Src2.tv_usec) {
			Result.tv_usec = Src1.tv_usec + 1000 - Src2.tv_usec;
			Result.tv_sec--;
		}
		else
			Result.tv_usec = Src1.tv_usec - Src2.tv_usec;
		return Result;
	}
}

MTime::timeval MTime::TimeAdd(MTime::timeval Src1, MTime::timeval Src2)
{
	MTime::timeval Result;

	Result.tv_sec = Src1.tv_sec + Src2.tv_sec;
	Result.tv_usec = Src1.tv_usec + Src2.tv_usec;

	while (Result.tv_usec >= 1000000) {
		Result.tv_usec -= 1000000;
		Result.tv_sec++;
	}

	return Result;
}