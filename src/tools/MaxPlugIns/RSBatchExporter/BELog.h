#ifndef __BELOG_H
#define __BELOG_H

bool BEInitLog(HWND hWnd,HINSTANCE hInstance);
bool BEInitLog(HWND hWnd);
void __cdecl log(const char *pFormat,...);
void BECloseLog();

#endif