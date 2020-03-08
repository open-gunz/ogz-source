#ifndef __BEWORKSHEET_H
#define __BEWORKSHEET_H

#include "Windows.h"
#include "BEParser.h"

#define	BEWORKSHEET_CANCEL	0
#define BEWORKSHEET_OK		1

BECommandList	*BEWorkSheet_GetCommandList();
int				BEWorkSheet_Work(HWND hWnd);
void			BEWorkSheet_SetFileName(const char *filename);
char			*BEWorkSheet_GetFileName();

#endif