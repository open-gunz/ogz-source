#pragma once

#include "MBaseStringResManager.h"

class MMatchStringResManager : public MBaseStringResManager
{
public:
	static void MakeInstance()
	{
		_ASSERT(m_pInstance == NULL);
		m_pInstance = new MMatchStringResManager();
	}
};