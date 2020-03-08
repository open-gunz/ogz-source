#ifndef _DIALOGPROC_H
#define _DIALOGPROC_H

#include "windows.h"

BOOL CALLBACK RBSExportDlgProc(HWND hWnd, UINT msg,WPARAM wParam, LPARAM lParam);
BOOL CALLBACK AboutBoxDlgProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

extern int g_nTreeDepth;
extern bool g_bExportRBS,g_bExportXML,g_bExportObjects;

#endif