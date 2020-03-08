#include "stdafx.h"
#include "RToken.h"

char *Format(char *buffer,rvector &v)
{
	sprintf(buffer,FORMAT_FLOAT" "FORMAT_FLOAT" "FORMAT_FLOAT,v.x,v.y,v.z);
	return buffer;
}

char *Format(char *buffer,float f)
{
	sprintf(buffer,FORMAT_FLOAT,f);
	return buffer;
}

char *Format(char *buffer,DWORD dw)
{
	sprintf(buffer,"%x",dw);
	return buffer;
}