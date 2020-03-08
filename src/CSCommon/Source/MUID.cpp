#include "stdafx.h"
#include "MUID.h"

MUID MUID::Invalid(void)
{
	return MUID(0,0);
}

MUIDRefMap::MUIDRefMap(void)
{
	m_CurrentMUID.SetZero();
}
MUIDRefMap::~MUIDRefMap(void)
{
}

MUID MUIDRefMap::Generate(void* pRef)
{
	m_CurrentMUID.Increase();
	insert(value_type(m_CurrentMUID, pRef));

	return m_CurrentMUID;
}
void* MUIDRefMap::GetRef(const MUID& uid)
{
	iterator i = find(uid);
	if(i==end()) return NULL;
	return (*i).second;
}

void* MUIDRefMap::Remove(const MUID& uid)
{
	iterator i = find(uid);
	if(i==end()) return NULL;
	void* pRef = (*i).second;
	erase(i);
	return pRef;
}

MUIDRANGE MUIDRefMap::Reserve(int nSize)
{
	MUIDRANGE r;
	r.Start = m_CurrentMUID.Increase();
	r.End = m_CurrentMUID.Increase(nSize-1);
	return r;
}

MUIDRANGE MUIDRefMap::GetReservedCount(void)
{
	MUIDRANGE r;
	r.Start = MUID(0, 2);
	r.End = m_CurrentMUID;
	return r;
}