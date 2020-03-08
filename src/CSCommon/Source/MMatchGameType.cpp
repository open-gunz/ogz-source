#include "stdafx.h"
#include "MMatchGameType.h"
#include "MMatchMap.h"

MMatchGameTypeMgr::MMatchGameTypeMgr() : MBaseGameTypeCatalogue()
{

}

MMatchGameTypeMgr::~MMatchGameTypeMgr()
{

}

MMatchGameTypeMgr* MMatchGameTypeMgr::GetInstance()
{
	static MMatchGameTypeMgr m_stGameTypeMgr;
	return &m_stGameTypeMgr;
}