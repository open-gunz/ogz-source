#include "stdafx.h"
#include "MDebug.h"
#include "MLogManager.h"

MLogManager::MLogManager() = default;
MLogManager::~MLogManager() = default;

void MLogManager::InsertLog(const std::string& strLog)
{
	m_MLog.push_back(strLog);
}

void MLogManager::SafeInsertLog(const std::string& strLog)
{
	Lock();
	InsertLog( strLog );
	Unlock();
}

void MLogManager::WriteMLog()
{
	for (auto it = m_MLog.begin(), end = m_MLog.end(); it != end; ++it)
		mlog(it->c_str());
}

void MLogManager::SafeWriteMLog()
{
	Lock();
	WriteMLog();
	Unlock();
}

void MLogManager::SafeReset()
{
	Lock();
	Reset();
	Unlock();
}

MLogManager& GetLogManager()
{
	return MLogManager::GetInstance();
}