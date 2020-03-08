#include "stdafx.h"
#include "ZStringResManager.h"
#include "ZConfiguration.h"

void ZStringResManager::MakeInstance()
{
	_ASSERT(m_pInstance == NULL);
	m_pInstance = new ZStringResManager();
}

ZStringResManager::ZStringResManager() = default;
ZStringResManager::~ZStringResManager() = default;

bool ZStringResManager::OnInit()
{
	auto strFileName = m_strPath + FILENAME_MESSAGES;

	if (!m_Messages.Initialize(strFileName.c_str(), 0, m_pFS))
	{
		_ASSERT(0);
		mlog("Error! -- Initialization of ZStringResManager from file \"%s\" failed\n", strFileName);
		return false;
	}

	return true;
}

const char* ZStringResManager::GetMessageStr(int nID)
{
	return m_Messages.GetStr(nID);
}