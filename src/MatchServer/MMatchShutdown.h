#pragma once

#include <vector>

#include "GlobalTypes.h"

class MShutdownNotify {
protected:
	unsigned short	m_nDelay;
	char			m_szMessage[128];	
public:
	MShutdownNotify(unsigned short nDelay, const char* pszMsg) {
		m_nDelay = nDelay;
		strcpy_safe(m_szMessage, pszMsg);
	}
	virtual ~MShutdownNotify() {}

	unsigned short GetDelay()	{ return m_nDelay; }
	char* GetString()			{ return m_szMessage; }
};

class MMatchShutdown {
protected:
	std::vector<MShutdownNotify*>	m_ShutdownNotifyArray;

	bool						m_bShutdown;
	unsigned short				m_nProgressIndex;
	u64							m_nTimeLastProgress;

	unsigned short GetProgressIndex() const	{ return m_nProgressIndex; }
	auto GetTimeLastProgress() const		{ return m_nTimeLastProgress; }
	void SetProgress(int nIndex, u64 nClock);

public:
	MMatchShutdown() { m_bShutdown = false; }
	virtual ~MMatchShutdown();

	bool LoadXML_ShutdownNotify(const char* pszFileName);

	void Start(u64 nClock);
	void Notify(int nIndex);
	void Terminate();

	bool IsShutdown()	{ return m_bShutdown; }

	void OnRun(u64 nClock);
};
