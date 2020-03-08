#pragma once

#include "MObject.h"

class MAgentObject : public MObject {
protected:
	char 			m_szIP[64]{};
	unsigned int	m_nTCPPort{};
	unsigned int	m_nUDPPort{};

public:
	using MObject::MObject;

	void SetAddr(char* szIP, unsigned short nTCPPort, unsigned short nUDPPort)	{ 
		strcpy_safe(m_szIP, szIP); m_nTCPPort = nTCPPort; m_nUDPPort = nUDPPort;
	}
	const char* GetIP() const { return m_szIP; }
	unsigned short GetTCPPort() const { return m_nTCPPort; }
	unsigned short GetUDPPort() const { return m_nUDPPort; }

	int GetStageCount() const { return 0; }
	int GetAssignCount() const { return 0; }
};

using MAgentObjectMap = std::map<MUID, MAgentObject*>;