/* 
	MDebugNew.h

	이 파일을 include 하면 MNewMemories::Init() 부터 
	MNewMemories::Shutdown() 사이에 있는 new / delete 를 추적합니다

	MNewMemories::Dump() 를 호출하면 현재 현재까지 할당된 메모리 목록을
	debug출력창과 memorydump.txt 라는 파일로 저장합니다

	또한 shutdown 할때도 dump를 하므로 delete되지 않은 목록을 출력합니다
*/



#ifndef _DEBUGNEW_H
#define _DEBUGNEW_H

#ifdef _MTRACEMEMORY

#include <assert.h>
#include <stdio.h>

#include <map>
#include <string>
#include <vector>
#include <list>

#include "Windows.h"

using namespace std;

struct MNEWINFO;

typedef list<MNEWINFO*>					MNEWLIST;
typedef map<void*,MNEWLIST::iterator>	MNEWPOINTERMAP;

class MNewMemories {
	static int				m_nID;		// 할당되는 번호
	static MNEWLIST			m_List;
	static MNEWPOINTERMAP	m_Pointers;
	static HINSTANCE		m_hModule;
	static bool	m_bInitialized;

public:
	static bool Init();
	static void Shutdown();

	static void OnNew(MNEWINFO *allocator,void* pPointer);
	static bool OnDelete(void* pPointer);
	static void Dump();
};


//-----------------
void* operator new(size_t _size);
void* operator new[]( size_t _size );
void operator delete(void* addr);
void operator delete[](void* addr);

#else	// 비활성화
class MNewMemories {
public:
	static bool Init();
	static void Shutdown();
	static void Dump();
};
#endif	// of _MTRACEMEMORY

#endif