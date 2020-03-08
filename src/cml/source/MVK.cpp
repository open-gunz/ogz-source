#include "stdafx.h"
#include "MVK.h"

#ifdef WIN32

#include "MWindows.h"

bool GetKeyName(char* Output, size_t OutputSize, u32 VirtualKeyCode, bool Extended)
{
	if (VirtualKeyCode == VK_SHIFT || VirtualKeyCode == VK_CONTROL || VirtualKeyCode == VK_MENU) {
		Output[0] = 0;
		return true;
	}

	LONG lScan = MapVirtualKey(VirtualKeyCode, 0) << 16;

	// if it's an extended key, add the extended flag
	if (Extended)
		lScan |= 0x01000000L;

	GetKeyNameText(lScan, Output, OutputSize);

	return true;
}

#else

bool GetKeyName(char* Output, size_t OutputSize, u32 VirtualKeyCode, bool Extended)
{
	return false;
}

#endif