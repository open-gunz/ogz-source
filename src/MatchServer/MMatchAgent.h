#pragma once

#include "MObject.h"
#include "SafeString.h"

class MMatchAgent : public MObject {
protected:
	char 			m_szIP[64] = {};
	unsigned int	m_nTCPPort = 0;
	unsigned int	m_nUDPPort = 0;

public:
	MMatchAgent() = default;
	MMatchAgent(const MUID& uid) : MObject(uid) {}

	void SetAddr(const char* szIP, unsigned short nTCPPort, unsigned short nUDPPort) { 
		strcpy_safe(m_szIP, szIP); m_nTCPPort = nTCPPort; m_nUDPPort = nUDPPort; }
	auto GetIP() const			{ return m_szIP; }
	auto GetTCPPort() const		{ return m_nTCPPort; }
	auto GetUDPPort() const		{ return m_nUDPPort; }
};


using MMatchAgentMap = std::map<MUID, MMatchAgent*>;