#pragma once

#include <vector>
#include <string>
#include "MSync.h"

class MLogManager
{
public:
	MLogManager();
	~MLogManager();

	void InsertLog(const std::string& strLog);
	void SafeInsertLog(const std::string& strLog);

	void Lock() { m_csLock.lock(); }
	void Unlock() { m_csLock.unlock(); }

	void WriteMLog();
	void SafeWriteMLog();

	void Reset() { m_MLog.clear(); }
	void SafeReset();

	static MLogManager& GetInstance()
	{
		static MLogManager LogManager;
		return LogManager;
	}

private:
	MCriticalSection m_csLock;

	std::vector<std::string> m_MLog;
};

MLogManager& GetLogManager();