#pragma once

#include "GlobalTypes.h"

class UPnP
{
public:
	UPnP();
	~UPnP();

	bool Create(u16 Port);
	void Destroy();

protected:
	bool GetIp();

	char m_Address[17]{};
	u16 m_Port{};
};
