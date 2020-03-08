#include "stdafx.h"
#include "MMatchChannelRule.h"

MMatchChannelRuleMgr* MMatchChannelRuleMgr::GetInstance()
{
	static MMatchChannelRuleMgr m_stChannelRuleMgr;
	return &m_stChannelRuleMgr;
}