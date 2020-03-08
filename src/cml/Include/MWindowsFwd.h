#pragma once
#include "GlobalTypes.h"
using BYTE = unsigned char;
using WORD = unsigned short;
#ifdef _WIN32
using DWORD = unsigned long;
#else
using DWORD = u32;
#endif
using QWORD = unsigned long long;
using HANDLE = void*;
#ifdef _WIN32
#define MAKE_HANDLE(name) using name = struct name##__*;
#else
#define MAKE_HANDLE(name) using name = void*;
#endif
MAKE_HANDLE(HINSTANCE);
using HMODULE = HINSTANCE;
MAKE_HANDLE(HWND);
using LPVOID = void*;
using LPCTSTR = const char*;
using GUID = struct _GUID;
using LPGUID = GUID*;
using LPDIRECTSOUND = struct IDirectSound*;
using LPDIRECTSOUND8 = struct IDirectSound8*;
using LPDIRECTSOUND3DLISTENER8 = struct IDirectSound3DListener*;
