#pragma once

#include "MUID.h"

#include <list>
#include <string>

struct MMatchObjectAntiHackInfo {
	char		m_szRandomValue[32];
	MMatchObjectAntiHackInfo() { Clear(); }
	void		Clear() { memset(m_szRandomValue, 0, sizeof(m_szRandomValue)); }
};


class MMatchAntiHack
{
private:
	static std::list<unsigned int>	m_clientFileListCRC;

public:
	MMatchAntiHack() {}
	~MMatchAntiHack() {}

	static size_t			GetFielCRCSize();

	static void				ClearClientFileList() { m_clientFileListCRC.clear(); }
	static void				InitClientFileList();
	static bool				CheckClientFileListCRC(unsigned int crc, const MUID& uidUser);
};
