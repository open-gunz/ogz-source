#include "stdafx.h"
#include "MStringTable.h"
#include <string.h>

static char g_StringTable[][256] = {
	"MINT",
	"확인",
	"취소",
	"예",
	"아니오",
	"메시지",
	"Overwrite?",
};

const char* MGetString(int nID)
{
	int nCount = sizeof(g_StringTable)/sizeof(char*);
	if(nID<0 || nID>=nCount) return NULL;

	return g_StringTable[nID];
}

void MSetString(int nID, const char* szString)
{
	int nCount = sizeof(g_StringTable)/sizeof(char*);
	if(nID<0 || nID>=nCount) return;

	strcpy_safe(g_StringTable[nID], szString);
}
