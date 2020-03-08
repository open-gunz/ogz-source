#pragma once

#ifdef WIN32
#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

#undef GetClassName
#undef CreateEvent
#undef GetUserName
#undef GetObject
#undef MoveFile
#undef DeleteFile
#undef CreateFile
#undef CreateDirectory
#endif