#pragma once

#include "GlobalTypes.h"

extern u64 (*GetGlobalTimeMS)();
float GetGlobalTime();

#ifdef _MSC_VER
// Link Winmm since timeGetTime depends on it
#pragma comment(lib, "Winmm.lib")
#endif

class MTime {
public:
	struct timeval
	{
		i32 tv_sec;
		i32 tv_usec;
	};

	MTime() {
		m = 2147483647; q = 127773; a = 16807; r = 2836;
		seed = static_cast<u32>(GetGlobalTimeMS());
	}
	u32 Random();
	int MakeNumber(int nFrom, int nTo);
	static void GetTime(MTime::timeval *t);
	static MTime::timeval TimeSub(MTime::timeval Src1, MTime::timeval Src2);
	static MTime::timeval TimeAdd(MTime::timeval Src1, MTime::timeval Src2);

private:
	u32	m;
	u32	q;
	u32	a;
	u32	r;
	u32	seed;
};