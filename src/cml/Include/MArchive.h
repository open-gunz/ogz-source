#ifndef MARCHIVE_H
#define MARCHIVE_H

#include <list>
#include <stdio.h>

// 2005.08.01 디펜던시 제거를 위해 벡터를 삭제. 추후 필요하면 mvector3.h의 MVector3 를 사용할것.
//#include "rutils.h"

using namespace std;

enum MArchiveType{
	MAT_CHAR,
	MAT_BOOL,
	MAT_INT,
	MAT_FLOAT,
	MAT_STRING,
	MAT_RAW,
//	MAT_VECTOR,
};

// Archive Item
#define MAITEM_NAME_LENGTH	32
struct MAITEM{
	char			szName[MAITEM_NAME_LENGTH];
	MArchiveType	nType;
	int				nSize;
	void*			pData;
	
	char GetCharValue(void);
	bool GetBoolValue(void);
	int GetIntValue(void);
	float GetFloatValue(void);
	char* GetStringValue(void);
	void* GetRawValue(void);
//	rvector GetVectorValue(void);
};

typedef list<MAITEM*>			MAITEMLIST;
typedef MAITEMLIST::iterator	MAITEMITOR;

class MArchive{
	MAITEMLIST		m_Items;
	MAITEMITOR		m_CurItem;

protected:
	void Add(const char* szName, MArchiveType t, const void* pData, int nSize);

public:
	MArchive(void);
	virtual ~MArchive(void);

	void Add(const char* szName, char c);
	void Add(const char* szName, bool b);
	void Add(const char* szName, int i);
	void Add(const char* szName, float f);
	void Add(const char* szName, const char* sz);
	void Add(const char* szName, const void* pData, int nSize);
//	void Add(const char* szName, rvector& v);

	MAITEM* GetFirst(void);
	MAITEM* GetNext(void);
	MAITEM* GetFirst(const char* szName);		// 처음이 szName을 가진 아이템이면 리턴
	MAITEM* GetNext(const char* szName);		// 현재 szName을 가진 아이템이면 리턴
	
	bool GetFirst(const char* szName, char* c);
	bool GetFirst(const char* szName, bool* b);
	bool GetFirst(const char* szName, int* i);
	bool GetFirst(const char* szName, float* f);
	bool GetFirst(const char* szName, char** sz);
	bool GetFirst(const char* szName, void** pData, int* nSize);
//	bool GetFirst(const char* szName, rvector* v);
	bool GetNext(const char* szName, char* c);
	bool GetNext(const char* szName, bool* b);
	bool GetNext(const char* szName, int* i);
	bool GetNext(const char* szName, float* f);
	bool GetNext(const char* szName, char** sz);
	bool GetNext(const char* szName, void** pData, int* nSize);
//	bool GetNext(const char* szName, rvector* v);

	void* CreateRaw(void);
	bool OpenRaw(void* pData, int nSize);

	bool WriteFile(FILE* fp);
	bool ReadFile(FILE* fp, int nSize=-1);
};

#endif
