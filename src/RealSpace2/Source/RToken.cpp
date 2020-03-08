#include "stdafx.h"
#include "RToken.h"

char *Format(char *buffer, int maxlen, const rvector &v)
{
	sprintf_safe(buffer, maxlen, FORMAT_FLOAT " " FORMAT_FLOAT " " FORMAT_FLOAT,v.x,v.y,v.z);
	return buffer;
}

char *Format(char *buffer, int maxlen, float f)
{
	sprintf_safe(buffer, maxlen, FORMAT_FLOAT,f);
	return buffer;
}

char *Format(char *buffer, int maxlen, u32 dw)
{
	sprintf_safe(buffer, maxlen, "%x",dw);
	return buffer;
}
