//
//	Common Open(Save) Dialog Module
//
//                                              written by lee jang ho
//
//////////////////////////////////////////////////////////////////////
#include "stdafx.h"
#include <windows.h>
#include <windowsx.h>
#include <stdio.h>

BOOL OpenDlg(HWND hwnd, char *pFileName, int nLen, char *pTitle, char *pFilter, char *pDefExt, BOOL bOpen, BOOL bMultiSel)
{
	OPENFILENAME OpenFileName;

	char InitDir[256];
	GetCurrentDirectory(256, InitDir);

	//char FileTitle[256]={0,};
	//FileTitle[0]=0;
	//pFileName[0]=0;

	ZeroMemory(&OpenFileName, sizeof(OPENFILENAME));
	OpenFileName.lStructSize       = sizeof(OPENFILENAME);
	OpenFileName.hwndOwner         = hwnd;
	OpenFileName.hInstance         = GetWindowInstance(hwnd);
	OpenFileName.lpstrFilter       = pFilter;
	OpenFileName.lpstrCustomFilter = NULL;
	OpenFileName.nMaxCustFilter    = 0;
	OpenFileName.nFilterIndex      = 0;
	OpenFileName.lpstrFile         = pFileName;
	OpenFileName.nMaxFile          = nLen;
	OpenFileName.lpstrFileTitle    = NULL;
	OpenFileName.nMaxFileTitle     = 0;
	//OpenFileName.lpstrFileTitle    = FileTitle;
	//OpenFileName.nMaxFileTitle     = 256;
	OpenFileName.lpstrInitialDir   = InitDir;
	OpenFileName.lpstrTitle        = pTitle;
	OpenFileName.nFileOffset       = 0;
	OpenFileName.nFileExtension    = 0;
	OpenFileName.lpstrDefExt       = pDefExt;
	OpenFileName.lCustData         = NULL;
	OpenFileName.lpfnHook 		   = NULL;
	OpenFileName.lpTemplateName    = NULL;
	OpenFileName.Flags             = OFN_HIDEREADONLY;
	if(bMultiSel==TRUE){
		OpenFileName.Flags |= (OFN_ALLOWMULTISELECT|OFN_EXPLORER);
	}

	BOOL bRetVal;
	if(bOpen==TRUE){
		OpenFileName.Flags |= (OFN_PATHMUSTEXIST|OFN_FILEMUSTEXIST);
		bRetVal = GetOpenFileName(&OpenFileName);
	}
	else{
		OpenFileName.Flags |= OFN_OVERWRITEPROMPT;
		bRetVal = GetSaveFileName(&OpenFileName);
	}

	return bRetVal;
}

int GetFileNameCount(const char *pFileNames)
{
	int nCount = 0;
	const char *pNextNames = pFileNames;

	int nLen = strlen(pNextNames);
	if(pFileNames[nLen+1]==0) return 1;

	nCount++;
	pNextNames += (nLen+1);

	while(1){
		int nLen = strlen(pNextNames);
		if(pNextNames[nLen+1]==0)
			return nCount;
		nCount++;
		pNextNames += (nLen+1);
	}
}

const char *GetFileName(const char *pFileNames, int idx)
{
	if(GetFileNameCount(pFileNames)==1)
		return pFileNames;
	
	char szDir[256];
	strcpy(szDir, pFileNames);

	static char szPath[256];
	const char *pNextNames = pFileNames;
	int nLen = strlen(pNextNames);
	pNextNames += (nLen+1);

	for(int i=0; i<idx; i++){
		nLen = strlen(pNextNames);
		pNextNames += (nLen+1);
	}
	sprintf(szPath, "%s\\%s", szDir, pNextNames);

	return szPath;
}

/*
char *g_pFileNames = NULL;	// Path의 다음 읽혀질 위치의 스트링
int g_nNextCount = 0;		// 다음에 더해질 Count

char *GetDirFirst(char *pFileNames)
{
	if(pFileNames==NULL) return NULL;
	if(pFileNames[0]==0) return NULL;
	
	g_nNextCount = 0;
	g_pFileNames = pFileNames;
	if(pFileNames[strlen(pFileNames)+1]==0){	// Multiple Selection
		char szDrive[4];
		char szDir[256];
		char szFileName[32];
		char szExt[16];
		_splitpath(pFileName, szDrive, szDir, szFileName, szExt);
	}
	else{
		g_nNextCount = (strlen(g_pFileNames)+1);
	}
	return g_pFileNames;
}

char *GetNextFile(void)
{
	g_pFileNames += g_nNextCount;
	g_nNextCount = (strlen(g_pFileNames)+1);
	if(g_pFileNames[0]==0) return NULL;
	return g_pFileNames;
}

int GetFileCount(char *pFileName)
{
	char *pDir = GetDirFirst(pFileName);
	int i = 0;
	while(1){
		char *pFileName = GetNextFile();
		if(pFileName==NULL)
			break;
		i++;
	}
	return i;
}
*/