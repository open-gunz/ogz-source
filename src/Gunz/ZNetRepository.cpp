#include "stdafx.h"
#include "ZNetRepository.h"


ZNetRepository::ZNetRepository()
{
	m_ClanInfo.nCLID = 0;
}

ZNetRepository::~ZNetRepository()
{


}

ZNetRepository*	ZNetRepository::GetInstance()
{
	static ZNetRepository m_stNetRepository;
	return &m_stNetRepository;
}
