//
//	Misc Functions about File
//
//                                              written by lee jang ho
//
//////////////////////////////////////////////////////////////////////
#include "FileInfo.h"
#include "stdio.h"

BOOL GetLastUpdate(const char *pFileName, FILETIME *ft)
{
	HANDLE hFile;
	hFile = CreateFile(pFileName,GENERIC_READ,FILE_SHARE_READ|FILE_SHARE_WRITE,NULL,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,NULL);
	if(hFile==INVALID_HANDLE_VALUE){
		return FALSE;
	}

	BY_HANDLE_FILE_INFORMATION fi;
	if(GetFileInformationByHandle(hFile, &fi)==TRUE){
		memcpy(ft, &(fi.ftLastWriteTime), sizeof(FILETIME));
	}

	CloseHandle(hFile);

	return TRUE;
}

BOOL IsModifiedOutside(const char *pFileName, FILETIME ot)
{
	FILETIME ft;
	if(GetLastUpdate(pFileName, &ft)==FALSE)
		return TRUE;	// 열지 못하는 경우 수정되었다고 가정하고 리턴

	if((ft.dwHighDateTime==ot.dwHighDateTime)&&
		(ft.dwLowDateTime==ot.dwLowDateTime))
		return FALSE;
	return TRUE;
}

BOOL RemoveExtension(char *pRemoveExt, const char *pFileName)
{
	int nLen = strlen(pFileName);
	for(int i=0; i<nLen; i++){
		if(pFileName[nLen-i-1]=='.'){
			memcpy(pRemoveExt, pFileName, nLen-i-1);
			pRemoveExt[nLen-i-1] = 0;
			return TRUE;	// 성공적으로 Extension을 제거하였을 경우
		}
	}
	strcpy(pRemoveExt, pFileName);
	return FALSE;	// Extension이 존재하지 않는 경우 원본 복사
}

void ReplaceExtension(char *pTargetName, const char *pSourceName, char *pExt)
{
	RemoveExtension(pTargetName, pSourceName);
	int nLen = strlen(pTargetName);
	pTargetName[nLen] = '.';
	memcpy(pTargetName+nLen+1, pExt, strlen(pExt)+1);
}

int CountStrEqual(const char *pStr0, const char *pStr1)
{
	int nStr0Len = strlen(pStr0);
	int nStr1Len = strlen(pStr1);

	for(int i=0; i<min(nStr0Len, nStr1Len); i++){
		if(pStr0[i]!=pStr1[i]) return i;
	}

	return i;
}

//	pDir	:	\abc\ac\a\ 같은 형식
int GetDirDepth(const char *pDir)
{
	int nDepth = 0;
	int nDirLen = strlen(pDir);
	for(int i=0; i<nDirLen; i++){
		if(pDir[i]=='\\') nDepth++;
	}
	if(pDir[0]=='\\') nDepth--;
	if(pDir[nDirLen-1]=='\\') nDepth--;
	return (nDepth+1);
}

int GetDirAsDepth(char *pDepthDir, const char *pDir, int nDepth)
{
	int nStartDir = 0;
	int nEndDir = 0;
	int nDepthCount = 0;

	int nCount = 0;
	if(pDir[0]=='\\') nCount++;
	int nDepthChecked = FALSE;
	for(int i=nCount; i<(int)strlen(pDir); i++){
		if(pDir[i]=='\\'){
			nDepthCount++;
		}
		if(nDepth==nDepthCount && nDepthChecked==FALSE){
			nStartDir = i;
			nDepthChecked = TRUE;
		}
		if(nDepth+1==nDepthCount){
			nEndDir = i-1;
			break;
		}
	}

	memcpy(pDepthDir, pDir+nStartDir, nEndDir-nStartDir+1);
	pDepthDir[nEndDir-nStartDir+1] = 0;

	return nStartDir;
}

// 그 Depth까지의 스트링 크기
int GetLenAsDepth(char *pDir, int nDepth)
{
	int nDepthCount = 0;

	int nCount = 0;
	if(pDir[0]=='\\') nCount++;
	for(int i=nCount; i<(int)strlen(pDir); i++){
		if(pDir[i]=='\\'){
			nDepthCount++;
		}
		if(nDepth==nDepthCount){
			return i+1;
		}
	}

	return -1;
}

void GetRelativePath(char *pRelativePath, const char *pBasePath, const char *pPath)
{
	char szBasePath[256], szPath[256];
	_fullpath(szBasePath, pBasePath, 256);
	_fullpath(szPath, pPath, 256);

	if(szBasePath[0]!=szPath[0]){
		strcpy(pRelativePath, szPath);
		return;
	}

	char szBaseDir[256], szBaseDrive[4], szBaseFileName[256], szBaseExt[16];
	char szDir[256], szDrive[4], szFileName[256], szExt[16];
	_splitpath(szBasePath, szBaseDrive, szBaseDir, szBaseFileName, szBaseExt);
	_splitpath(szPath, szDrive, szDir, szFileName, szExt);

	char szBaseDepthDir[256];
	char szDepthDir[256];
	int nEqualDepth = 0;
	int nEqualLen = 0;

	int nBaseDepth = GetDirDepth(szBaseDir);
	int nDepth = GetDirDepth(szDir);
	//nEqualDepth = min(nBaseDepth, nDepth);
	for(int i=0; i<min(nBaseDepth, nDepth); i++){
		GetDirAsDepth(szBaseDepthDir, szBaseDir, i);
		GetDirAsDepth(szDepthDir, szDir, i);
		if(strcmp(szDepthDir, szBaseDepthDir)==0){
			nEqualDepth++;
		}
		else{
			break;
		}
	}

	nEqualLen = GetLenAsDepth(szDir, nEqualDepth);

	// Depth의 차이만큼 "../"넣기
	char szTemp[256]={0,};
	char *pUp = "..\\";
	int nNextPos = 0;
	for(i=0; i<nBaseDepth-nEqualDepth; i++){
		memcpy(szTemp+nNextPos, pUp, 4);
		nNextPos +=3;
	}
	memcpy(szTemp+nNextPos, szDir+nEqualLen, strlen(szDir)-nEqualLen);

	wsprintf(pRelativePath, "%s%s%s", szTemp, szFileName, szExt);
}

// pFullPath	:	리턴되는 Full Path
// pBasePath	:	기준이 되는 Path ( 이왕이면 FullPath, Relatvie Path는 현재 디렉토리를 기준으로 )
// pRelativePath:	Relative Path	( ex. abc\def.ghi )
void GetFullPath(char *pFullPath, const char *pBasePath, const char *pRelativePath)
{
	if(IsFullPath(pRelativePath)==TRUE){
		strcpy(pFullPath, pRelativePath);
		return;
	}
	char szFullPath[256];
	char szDrive[8];
	char szDir[256];
	char szFileName[32];
	char szExt[16];

	_fullpath(szFullPath, pBasePath, 256);
	_splitpath(szFullPath, szDrive, szDir, szFileName, szExt);
	wsprintf(pFullPath, "%s%s%s", szDrive, szDir, pRelativePath);
}

void GetFullPath(char *pFullPath, const char *pRelativePath)
{
	if(IsFullPath(pRelativePath)==TRUE){
		strcpy(pFullPath, pRelativePath);
		return;
	}
	char szFullPath[256];
	char szDrive[8];
	char szDir[256];
	char szFileName[32];
	char szExt[16];
	char pBasePath[256];

	GetCurrentDirectory(sizeof(pBasePath),pBasePath);
	if(pBasePath[strlen(pBasePath)]!='\\') strcat(pBasePath,"\\");
	_fullpath(szFullPath, pBasePath, 256);
	_splitpath(szFullPath, szDrive, szDir, szFileName, szExt);
	wsprintf(pFullPath, "%s%s%s", szDrive, szDir, pRelativePath);
}


BOOL IsFullPath(const char *pPath)
{
	if(pPath[1]==':') return TRUE;
	if(pPath[0]=='\\' && pPath[0]=='\\') return TRUE;
	return FALSE;
}

BOOL ReadHeader(HANDLE hFile, void *pHeader, int nHeaderSize)
{
	int nRealSize;
	DWORD nRead;
	if(ReadFile(hFile, &nRealSize, sizeof(int), &nRead, NULL)==FALSE) return FALSE;

	nRealSize-=sizeof(int);
	char *pReadData = new char[nRealSize];
	if(ReadFile(hFile, pReadData, nRealSize, &nRead, NULL)==FALSE){
		delete[] pReadData;
		return FALSE;
	}

	memset(pHeader, 0, nHeaderSize);
	memcpy((char *)pHeader+sizeof(int), pReadData, min((int)nRealSize, (int)(nHeaderSize-sizeof(int))));
	memcpy(pHeader, &nRealSize, sizeof(int));

	delete[] pReadData;

	return TRUE;
}

void GetPureFilename(char *pPureFilename,const char *pFilename)
{
	char drive[_MAX_DRIVE],dir[_MAX_DIR],ext[_MAX_EXT];
	_splitpath(pFilename,drive,dir,pPureFilename,ext);
}

bool IsExist(const char *filename)
{
	FILE *file=fopen(filename,"r");
	bool breturn=(file!=NULL);
	if(file) fclose(file);
	return breturn;
}

