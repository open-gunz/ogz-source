#include "stdafx.h"
#include "MStrEx.h"

void MStrNCpy(char* szDest, int nDestLen, const char* szSource)
{
	for(int i=0; i<nDestLen-1 && *szSource!=0; i++){
		*szDest++ = *szSource++;
	}
	*szDest = 0;
}

