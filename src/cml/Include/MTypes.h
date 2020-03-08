#ifndef _MTYPES_H
#define _MTYPES_H

#include "MVector3.h"

#ifdef _WIN32
#ifdef _MSC_VER
typedef signed __int64		int64;
typedef unsigned __int64	uint64;
#endif
#elif defined _LINUX
typedef long long			int64;
typedef unsigned long long	uint64;
#endif


typedef MVector3			MVector;

#endif