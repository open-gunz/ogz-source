#include "stdafx.h"
#include "MArchive.h"

#ifndef _ASSERT
#include <cassert>
#define _ASSERT assert
#endif

char MAITEM::GetCharValue(void)
{
	_ASSERT(nType==MAT_CHAR);
	return *((char*)pData);
}
bool MAITEM::GetBoolValue(void)
{
	_ASSERT(nType==MAT_BOOL);
	return *((bool*)pData);
}

int MAITEM::GetIntValue(void)
{
	_ASSERT(nType==MAT_INT);
	return *((int*)pData);
}
float MAITEM::GetFloatValue(void)
{
	_ASSERT(nType==MAT_FLOAT);
	return *((float*)pData);
}
char* MAITEM::GetStringValue(void)
{
	_ASSERT(nType==MAT_STRING);
	return ((char*)pData);
}
void* MAITEM::GetRawValue(void)
{
	_ASSERT(nType==MAT_RAW);
	return pData;
}
//rvector MAITEM::GetVectorValue(void)
//{
//	_ASSERT(nType==MAT_VECTOR);
//	return *((rvector*)pData);
//}
//

#define MIN(_a, _b)	((_a<_b)?_a:_b)

void MArchive::Add(const char* szName, MArchiveType t, const void* pData, int nSize)
{
	MAITEM* pItem = new MAITEM;
	memcpy(pItem->szName, szName, MAITEM_NAME_LENGTH);
	pItem->szName[MIN(strlen(szName), MAITEM_NAME_LENGTH-1)] = 0;
	pItem->nType = t;
	pItem->pData = new char[nSize];
	memcpy(pItem->pData, pData, nSize);
	pItem->nSize = nSize;
	m_Items.insert(m_Items.end(), pItem);
}

MArchive::MArchive(void)
{
	m_CurItem = m_Items.end();
}

MArchive::~MArchive(void)
{
	while(m_Items.empty()==false){
		MAITEM* pItem = *(m_Items.begin());
		delete[] static_cast<char*>(pItem->pData);
		delete pItem;
		m_Items.erase(m_Items.begin());
	}
}

void MArchive::Add(const char* szName, char c)
{
	Add(szName, MAT_CHAR, &c, sizeof(char));
}

void MArchive::Add(const char* szName, bool b)
{
	Add(szName, MAT_BOOL, &b, sizeof(bool));
}

void MArchive::Add(const char* szName, int i)
{
	Add(szName, MAT_INT, &i, sizeof(int));
}

void MArchive::Add(const char* szName, float f)
{
	Add(szName, MAT_FLOAT, &f, sizeof(float));
}

void MArchive::Add(const char* szName, const char* sz)
{
	Add(szName, MAT_STRING, sz, strlen(sz)+1);
}
void MArchive::Add(const char* szName, const void* pData, int nSize)
{
	Add(szName, MAT_RAW, pData, nSize);
}

//void MArchive::Add(const char* szName, rvector& v)
//{
//	Add(szName, MAT_VECTOR, &v, sizeof(rvector));
//}
//
MAITEM* MArchive::GetFirst(void)
{
	m_CurItem = m_Items.begin();
	if(m_CurItem==m_Items.end()) return NULL;
	MAITEMITOR i = m_CurItem;
	m_CurItem++;
	return (*i);
}

MAITEM* MArchive::GetNext(void)
{
	if(m_CurItem==m_Items.end()) return NULL;
	MAITEMITOR i = m_CurItem;
	m_CurItem++;
	return (*i);
}

MAITEM* MArchive::GetFirst(const char* szName)
{
	m_CurItem = m_Items.begin();
	if(m_CurItem==m_Items.end()) return NULL;
	MAITEM* pItem = *m_CurItem;
	if(strncmp(pItem->szName, szName, MAITEM_NAME_LENGTH-1)!=0) return NULL;	// 이름이 같지 않으면 NULL 리턴

	MAITEMITOR i = m_CurItem;
	m_CurItem++;
	return (*i);
}

MAITEM* MArchive::GetNext(const char* szName)
{
	if(m_CurItem==m_Items.end()) return NULL;
	MAITEM* pItem = *m_CurItem;
	if(strncmp(pItem->szName, szName, MAITEM_NAME_LENGTH-1)!=0) return NULL;	// 이름이 같지 않으면 NULL 리턴

	MAITEMITOR i = m_CurItem;
	m_CurItem++;
	return (*i);
}

bool MArchive::GetFirst(const char* szName, char* c)
{
	MAITEM* pItem = GetFirst(szName);
	if(pItem==NULL) return false;
	*c = pItem->GetCharValue();
	return true;
}
bool MArchive::GetFirst(const char* szName, bool* b)
{
	MAITEM* pItem = GetFirst(szName);
	if(pItem==NULL) return false;
	*b = pItem->GetBoolValue();
	return true;
}
bool MArchive::GetFirst(const char* szName, int* i)
{
	MAITEM* pItem = GetFirst(szName);
	if(pItem==NULL) return false;
	*i = pItem->GetIntValue();
	return true;
}
bool MArchive::GetFirst(const char* szName, float* f)
{
	MAITEM* pItem = GetFirst(szName);
	if(pItem==NULL) return false;
	*f = pItem->GetFloatValue();
	return true;
}
bool MArchive::GetFirst(const char* szName, char** sz)
{
	MAITEM* pItem = GetFirst(szName);
	if(pItem==NULL) return false;
	*sz = pItem->GetStringValue();
	return true;
}
bool MArchive::GetFirst(const char* szName, void** pData, int* nSize)
{
	MAITEM* pItem = GetFirst(szName);
	if(pItem==NULL) return false;
	*pData = pItem->GetRawValue();
	if(nSize!=NULL) *nSize = pItem->nSize;
	return true;
}
//bool MArchive::GetFirst(const char* szName, rvector* v)
//{
//	MAITEM* pItem = GetFirst(szName);
//	if(pItem==NULL) return false;
//	*v = pItem->GetVectorValue();
//	return true;
//}
bool MArchive::GetNext(const char* szName, char* c)
{
	MAITEM* pItem = GetNext(szName);
	if(pItem==NULL) return false;
	*c = pItem->GetCharValue();
	return true;
}
bool MArchive::GetNext(const char* szName, bool* b)
{
	MAITEM* pItem = GetNext(szName);
	if(pItem==NULL) return false;
	*b = pItem->GetBoolValue();
	return true;
}
bool MArchive::GetNext(const char* szName, int* i)
{
	MAITEM* pItem = GetNext(szName);
	if(pItem==NULL) return false;
	*i = pItem->GetIntValue();
	return true;
}
bool MArchive::GetNext(const char* szName, float* f)
{
	MAITEM* pItem = GetNext(szName);
	if(pItem==NULL) return false;
	*f = pItem->GetFloatValue();
	return true;
}
bool MArchive::GetNext(const char* szName, char** sz)
{
	MAITEM* pItem = GetNext(szName);
	if(pItem==NULL) return false;
	*sz = pItem->GetStringValue();
	return true;
}
bool MArchive::GetNext(const char* szName, void** pData, int* nSize)
{
	MAITEM* pItem = GetNext(szName);
	if(pItem==NULL) return false;
	*pData = pItem->GetRawValue();
	if(nSize!=NULL) *nSize = pItem->nSize;
	return true;
}
//bool MArchive::GetNext(const char* szName, rvector* v)
//{
//	MAITEM* pItem = GetNext(szName);
//	if(pItem==NULL) return false;
//	*v = pItem->GetVectorValue();
//	return true;
//}

void* MArchive::CreateRaw(void)
{
	return NULL;
}

bool MArchive::OpenRaw(void* pData, int nSize)
{
	return true;
}

bool MArchive::WriteFile(FILE* fp)
{
	for(MAITEMITOR i=m_Items.begin(); i!=m_Items.end(); i++){
		MAITEM* pItem = *i;
#define HEADER_SIZE	(sizeof(MAITEM)-sizeof(void*))
		if(fwrite(pItem, HEADER_SIZE, 1, fp)!=1) return false;
		if(fwrite(pItem->pData, pItem->nSize, 1, fp)!=1) return false;
	}
	
	return true;
}

bool MArchive::ReadFile(FILE* fp, int nSize)
{
	int nReadSize = 0;
	while(feof(fp)==0){
		MAITEM* pItem = new MAITEM;
		if(fread(pItem, HEADER_SIZE, 1, fp)!=1){
			delete pItem;
			return true;
		}
		nReadSize += HEADER_SIZE;

		pItem->pData = new char[pItem->nSize];
		if(fread(pItem->pData, pItem->nSize, 1, fp)!=1){
			delete[] static_cast<char*>(pItem->pData);
			delete pItem;
			return false;
		}
		nReadSize += pItem->nSize;

		m_Items.insert(m_Items.end(), pItem);

		if(nSize>0){
			if(nReadSize==nSize) return true;
			if(nReadSize>nSize) return false;	// 초과하면 에러이다.
		}
	}

	return true;
}
