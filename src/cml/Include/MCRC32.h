#pragma once

#include "GlobalTypes.h"

// Referenced from http://www.gamedev.net/reference/articles/article1941.asp

class MCRC32 {
private:
	static u32 CRC32Table[256];
	static inline void LookupCRC32(const u8 byte, u32 &dwCRC32);

public:
	typedef u32 crc_t;

	enum CRC
	{
		SIZE = sizeof(u32),
	};

	static u32 BuildCRC32(const u8* pData, u32 dwSize);
};
