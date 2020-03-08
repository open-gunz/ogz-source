#pragma once

#include <string>
#include "SafeString.h"
#include "MSocket.h"
#include "StringView.h"

template <size_t size>
bool GetLocalIP(char (&szOutIP)[size])
{
	char szHostName[256];

	if (MSocket::gethostname(szHostName, sizeof(szHostName)) != 0)
		return false;

	auto pHostInfo = MSocket::gethostbyname(szHostName);
	if (!pHostInfo)
		return false;

	auto ip = MSocket::inet_ntop(MSocket::AF::INET, *(MSocket::in_addr *)*pHostInfo->h_addr_list,
		szOutIP, size);
	return true;
}

inline void GetIPv4String(MSocket::in_addr addr, ArrayView<char> ip_string)
{
	sprintf_safe(ip_string, "%d.%d.%d.%d",
		addr.S_un.S_un_b.s_b1,
		addr.S_un.S_un_b.s_b2,
		addr.S_un.S_un_b.s_b3,
		addr.S_un.S_un_b.s_b4);
}

inline std::string GetIPv4String(MSocket::in_addr addr)
{
	char buf[32];
	GetIPv4String(addr, buf);
	return buf;
}

template <typename... Args>
auto GetIPv4String(u32 addr, Args&... args)
{
	MSocket::in_addr addr2;
	addr2.s_addr = addr;
	return GetIPv4String(addr2, args...);
}

inline u32 GetIPv4Number(const char* addr)
{
	MSocket::in_addr ret;
	auto count = sscanf(addr, "%hhd.%hhd.%hhd.%hhd",
		&ret.S_un.S_un_b.s_b1,
		&ret.S_un.S_un_b.s_b2,
		&ret.S_un.S_un_b.s_b3,
		&ret.S_un.S_un_b.s_b4);
	
	if (count != 4)
		return MSocket::in_addr::None;

	return ret.S_un.S_addr;
}

namespace detail
{
// Adapted from curl/lib/escape.c
inline bool isunreserved(unsigned char in)
{
	switch (in)
	{
	case '0': case '1': case '2': case '3': case '4':
	case '5': case '6': case '7': case '8': case '9':
	case 'a': case 'b': case 'c': case 'd': case 'e':
	case 'f': case 'g': case 'h': case 'i': case 'j':
	case 'k': case 'l': case 'm': case 'n': case 'o':
	case 'p': case 'q': case 'r': case 's': case 't':
	case 'u': case 'v': case 'w': case 'x': case 'y': case 'z':
	case 'A': case 'B': case 'C': case 'D': case 'E':
	case 'F': case 'G': case 'H': case 'I': case 'J':
	case 'K': case 'L': case 'M': case 'N': case 'O':
	case 'P': case 'Q': case 'R': case 'S': case 'T':
	case 'U': case 'V': case 'W': case 'X': case 'Y': case 'Z':
	case '-': case '.': case '_': case '~':
		return true;
	default:
		return false;
	}
}
}

// Returns the number of characters written, not including the null terminator.
inline size_t URLEncode(char *Output, size_t OutputSize, const StringView& Input)
{
	size_t OutputIndex = 0;
	for (u8 c : Input)
	{
		const auto RemainingOutputSize = OutputSize - OutputIndex;
		if (RemainingOutputSize <= 0)
		{
			assert(false);
			break;
		}

		if (detail::isunreserved(c) /* HACK */ || c == '/')
		{
			Output[OutputIndex] = c;
			OutputIndex += 1;
		}
		else
		{
			auto CharactersWritten = sprintf_safe(&Output[OutputIndex], RemainingOutputSize, "%%%02X", c);
			OutputIndex += CharactersWritten;
		}
	}

	// Add a terminating null.
	if (OutputIndex >= OutputSize) {
		OutputIndex = OutputSize - 1;
	}
	Output[OutputIndex] = 0;

	return OutputIndex;
}

template <size_t OutputSize>
auto URLEncode(char(&Output)[OutputSize], const StringView& Input) {
	return URLEncode(Output, OutputSize, Input);
}