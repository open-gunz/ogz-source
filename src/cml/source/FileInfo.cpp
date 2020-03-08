#include "stdafx.h"
#include "FileInfo.h"
#include <stdio.h>
#include <string>
#include "MFile.h"

#ifdef _WIN32
#include <shlwapi.h>
#include <shlobj.h>
#else
#include <climits>
#include <cstdlib>
#include <libgen.h>
#endif

#ifdef _WIN32
bool GetLastUpdate(const char *pFileName, FILETIME *ft)
{
	HANDLE hFile;
	hFile = CreateFile(pFileName, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFile == INVALID_HANDLE_VALUE) {
		return FALSE;
	}

	BY_HANDLE_FILE_INFORMATION fi;
	if (GetFileInformationByHandle(hFile, &fi) == TRUE) {
		memcpy(ft, &(fi.ftLastWriteTime), sizeof(FILETIME));
	}

	CloseHandle(hFile);

	return TRUE;
}

bool IsModifiedOutside(const char *pFileName, FILETIME ot)
{
	FILETIME ft;
	if (GetLastUpdate(pFileName, &ft) == FALSE)
		return TRUE;

	if ((ft.dwHighDateTime == ot.dwHighDateTime) &&
		(ft.dwLowDateTime == ot.dwLowDateTime))
		return FALSE;
	return TRUE;
}
#endif

bool RemoveExtension(char *pRemoveExt, int maxlen, const char *pFileName)
{
	int nLen = strlen(pFileName);
	for (int i = 0; i < nLen; i++) {
		if (pFileName[nLen - i - 1] == '.') {
			memcpy(pRemoveExt, pFileName, nLen - i - 1);
			pRemoveExt[nLen - i - 1] = 0;
			return TRUE;
		}
	}
	strcpy_safe(pRemoveExt, maxlen, pFileName);
	return FALSE;
}

void ReplaceExtension(char *pTargetName, int maxlen, const char *pSourceName, char *pExt)
{
	RemoveExtension(pTargetName, maxlen, pSourceName);
	int nLen = strlen(pTargetName);
	pTargetName[nLen] = '.';
	memcpy(pTargetName + nLen + 1, pExt, strlen(pExt) + 1);
}

int CountStrEqual(const char *pStr0, const char *pStr1)
{
	int i;
	auto nStr0Len = int(strlen(pStr0));
	auto nStr1Len = int(strlen(pStr1));

	for (i = 0; i < (std::min)(nStr0Len, nStr1Len); i++) {
		if (pStr0[i] != pStr1[i]) return i;
	}

	return i;
}

int GetDirDepth(const char *pDir)
{
	int nDepth = 0;
	int nDirLen = strlen(pDir);
	for (int i = 0; i < nDirLen; i++) {
		if (pDir[i] == '\\') nDepth++;
	}
	if (pDir[0] == '\\') nDepth--;
	if (pDir[nDirLen - 1] == '\\') nDepth--;
	return (nDepth + 1);
}

int GetDirAsDepth(char *pDepthDir, const char *pDir, int nDepth)
{
	int nStartDir = 0;
	int nEndDir = 0;
	int nDepthCount = 0;

	int nCount = 0;
	if (pDir[0] == '\\') nCount++;
	int nDepthChecked = FALSE;
	for (int i = nCount; i < (int)strlen(pDir); i++) {
		if (pDir[i] == '\\') {
			nDepthCount++;
		}
		if (nDepth == nDepthCount && nDepthChecked == FALSE) {
			nStartDir = i;
			nDepthChecked = TRUE;
		}
		if (nDepth + 1 == nDepthCount) {
			nEndDir = i - 1;
			break;
		}
	}

	memcpy(pDepthDir, pDir + nStartDir, nEndDir - nStartDir + 1);
	pDepthDir[nEndDir - nStartDir + 1] = 0;

	return nStartDir;
}

int GetLenAsDepth(char *pDir, int nDepth)
{
	int nDepthCount = 0;

	int nCount = 0;
	if (pDir[0] == '\\') nCount++;

	if (nDepth == 0) return nCount;

	for (int i = nCount; i < (int)strlen(pDir); i++) {
		if (pDir[i] == '\\') {
			nDepthCount++;
		}
		if (nDepth == nDepthCount) {
			return i + 1;
		}
	}

	return -1;
}

void GetRelativePath(char *pRelativePath, int maxlen, const char *pBasePath, const char *pPath)
{
	char szBasePath[256], szPath[256];
	int i;

#ifdef _WIN32
	_fullpath(szBasePath, pBasePath, 256);
	_fullpath(szPath, pPath, 256);
#else
	realpath(pBasePath, szBasePath);
	realpath(pPath, szPath);
#endif

	strlwr_safe(szPath);
	strlwr_safe(szBasePath);

	if (szBasePath[0] != szPath[0]) {
		strcpy_safe(pRelativePath, maxlen, szPath);
		return;
	}

	char szBaseDir[MFile::MaxPath];
	char szDir[MFile::MaxPath];
	char szFileName[MFile::MaxPath];
#ifdef _WIN32
	char Unused[_MAX_DIR];
	_splitpath_s(szBasePath, Unused, szBaseDir, Unused, Unused);
	char szExt[MFile::MaxPath];
	_splitpath_s(szPath, Unused, szDir, szFileName, szExt);
	strcat_safe(szFileName, szExt);
#else
	strcpy_safe(szBaseDir, dirname(szBasePath));
	strcpy_safe(szDir, dirname(szPath));
#endif

	char szBaseDepthDir[256];
	char szDepthDir[256];
	int nEqualDepth = 0;
	int nEqualLen = 0;

	int nBaseDepth = GetDirDepth(szBaseDir);
	int nDepth = GetDirDepth(szDir);
	for (i = 0; i < (std::min)(nBaseDepth, nDepth); i++) {
		GetDirAsDepth(szBaseDepthDir, szBaseDir, i);
		GetDirAsDepth(szDepthDir, szDir, i);
		if (strcmp(szDepthDir, szBaseDepthDir) == 0) {
			nEqualDepth++;
		}
		else {
			break;
		}
	}

	nEqualLen = GetLenAsDepth(szDir, nEqualDepth);

	char szTemp[256]{};
	const char *pUp = "..\\";
	int nNextPos = 0;
	for (i = 0; i < nBaseDepth - nEqualDepth; i++) {
		memcpy(szTemp + nNextPos, pUp, 4);
		nNextPos += 3;
	}
	memcpy(szTemp + nNextPos, szDir + nEqualLen, strlen(szDir) - nEqualLen);

	sprintf_safe(pRelativePath, maxlen, "%s%s", szTemp, szFileName);
}

void GetFullPath(char *pFullPath, int maxlen, const char *pBasePath, const char *pRelativePath)
{
#ifdef _WIN32
	if (IsFullPath(pRelativePath) == TRUE) {
		strcpy_safe(pFullPath, maxlen, pRelativePath);
		return;
	}
	char szFullPath[_MAX_PATH];
	char szDrive[_MAX_DRIVE];
	char szDir[_MAX_DIR];
	char szFileName[_MAX_FNAME];
	char szExt[_MAX_EXT];

	_fullpath(szFullPath, pBasePath, 256);
	_splitpath_s(szFullPath, szDrive, szDir, szFileName, szExt);
	wsprintf(pFullPath, "%s%s%s", szDrive, szDir, pRelativePath);
#else
	char BasePathCopy[MFile::MaxPath];
	char FullPath[MFile::MaxPath];
	strcpy_safe(BasePathCopy, pBasePath);
	realpath(FullPath, BasePathCopy);
	auto Dir = dirname(FullPath);
	sprintf_safe(pFullPath, maxlen, "%s%s", Dir, pRelativePath);
#endif
}

void GetFullPath(char *pFullPath, int maxlen, const char *pRelativePath)
{
#ifdef _WIN32
	if (IsFullPath(pRelativePath) == TRUE) {
		strcpy_safe(pFullPath, maxlen, pRelativePath);
		return;
	}
	char szFullPath[256];
	char szDrive[8];
	char szDir[256];
	char szFileName[32];
	char szExt[16];
	char pBasePath[256];

	GetCurrentDirectory(sizeof(pBasePath), pBasePath);
	if (pBasePath[strlen(pBasePath)] != '\\')
		strcat_safe(pBasePath, "\\");
	_fullpath(szFullPath, pBasePath, 256);
	_splitpath_s(szFullPath, szDrive, szDir, szFileName, szExt);
	wsprintf(pFullPath, "%s%s%s", szDrive, szDir, pRelativePath);
#else
	realpath(pRelativePath, pFullPath);
#endif
}


bool IsFullPath(const char *pPath)
{
	if (pPath[1] == ':') return TRUE;
	if (pPath[0] == '\\' || pPath[0] == '/') return TRUE;
	return FALSE;
}

#ifdef _WIN32
bool ReadHeader(HANDLE hFile, void *pHeader, int nHeaderSize)
{
	int nRealSize;
	DWORD nRead;
	if (ReadFile(hFile, &nRealSize, sizeof(int), &nRead, NULL) == FALSE) return FALSE;

	nRealSize -= sizeof(int);
	char *pReadData = new char[nRealSize];
	if (ReadFile(hFile, pReadData, nRealSize, &nRead, NULL) == FALSE) {
		delete[] pReadData;
		return FALSE;
	}

	memset(pHeader, 0, nHeaderSize);
	memcpy((char *)pHeader + sizeof(int), pReadData, min((int)nRealSize, (int)(nHeaderSize - sizeof(int))));
	memcpy(pHeader, &nRealSize, sizeof(int));

	delete[] pReadData;

	return TRUE;
}
#endif

void GetPurePath(char *pPurePath, int maxlen, const char *pFilename)
{
#ifdef _WIN32
	char drive[_MAX_DRIVE], dir[_MAX_DIR], fname[_MAX_FNAME], ext[_MAX_EXT];
	_splitpath_s(pFilename, drive, dir, fname, ext);
	sprintf_safe(pPurePath, maxlen, "%s%s", drive, dir);
#else
	char FilenameCopy[MFile::MaxPath];
	strcpy_safe(FilenameCopy, pFilename);
	strcpy_safe(pPurePath, maxlen, dirname(FilenameCopy));
#endif

	int len = strlen(pPurePath);
	if (len)
	{
		char last = pPurePath[len - 1];
		if (last != '/' && last != '\\')
			strcat_safe(pPurePath, maxlen, "/");
	}
}

#ifdef _WIN32
u32 GetFileCheckSum(char* pszFileName)
{
	HANDLE hFile = CreateFile(pszFileName, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_FLAG_RANDOM_ACCESS, NULL);
	if ((hFile == NULL) || (hFile == INVALID_HANDLE_VALUE)) {
		return 0;
	}

	u32 dwCheckSum = 0;

	BY_HANDLE_FILE_INFORMATION fileInfo;
	GetFileInformationByHandle(hFile, &fileInfo);

	dwCheckSum += (u32)fileInfo.nFileSizeLow;

	while (true) {
		u32 dwBuffer = 0;
		DWORD dwNumToRead;
		if (ReadFile(hFile, &dwBuffer, sizeof(u32), &dwNumToRead, 0) == FALSE)
			break;
		dwCheckSum += dwBuffer;
		if (dwNumToRead == 0)
			break;
	}

	CloseHandle(hFile);

	return dwCheckSum;
}
#endif

bool IsExist(const char *filename)
{
	return MFile::Exists(filename);
}

void GetParentDirectory(char* pszFileName)
{
	// Remove Slash
	int nLen = (int)strlen(pszFileName);
	if ((pszFileName[nLen - 1] == '/') ||
		(pszFileName[nLen - 1] == '\\')) {
		pszFileName[nLen - 1] = 0;
	}
	nLen = (int)strlen(pszFileName);

	// Get Parent Directory
	int i;
	for (i = nLen; i >= 0; i--) {
		if ((pszFileName[i - 1] == '/') ||
			(pszFileName[i - 1] == '\\')) {
			pszFileName[i - 1] = 0;
			return;
		}
	}

	if (i < 0)
		pszFileName[0] = 0;
}

bool MakePath(const char* pszFileName)
{
	char szPath[MFile::MaxPath];
	strcpy_safe(szPath, pszFileName);

	if (MFile::IsDir(szPath) == false)
		GetParentDirectory(szPath);

	if (strlen(szPath) <= 0)
		return false;

	if (MFile::IsDir(szPath)) {
		return true;
	}
	else {
		if (MFile::CreateDir(szPath)) {
			return true;
		}
		else {
			char szParent[MFile::MaxPath];
			strcpy_safe(szParent, pszFileName);
			GetParentDirectory(szParent);
			MakePath(szParent);
			if (MFile::CreateDir(szPath))
				return true;
			else
				return false;
		}
	}
	return false;
}

#ifdef _WIN32
void time_tToFILETIME(time_t t, LPFILETIME pft)
{
	LONGLONG ll = Int32x32To64(t, 10000000) + 116444736000000000;
	pft->dwLowDateTime = (u32)ll;
	pft->dwHighDateTime = (u32)(ll >> 32);
}

bool MSetFileTime(const char* lpszPath, FILETIME ft)
{
	u32 attr;
	HANDLE hFile;
	BOOL bResult = FALSE;

	attr = GetFileAttributes(lpszPath);
	if (attr != INVALID_FILE_ATTRIBUTES) {
		if ((attr & FILE_ATTRIBUTE_DIRECTORY) == FILE_ATTRIBUTE_DIRECTORY)
			hFile = CreateFile(lpszPath, GENERIC_WRITE, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_BACKUP_SEMANTICS, NULL);
		else
			hFile = CreateFile(lpszPath, GENERIC_WRITE, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
		if (hFile != INVALID_HANDLE_VALUE) {
			bResult = SetFileTime(hFile, &ft, &ft, &ft);
			CloseHandle(hFile);
		}
	}
	return bResult != FALSE;
}
#endif

#ifdef _WIN32
std::string GetMyDocumentsPath()
{
	CHAR path[MAX_PATH];
	if (SUCCEEDED(SHGetFolderPath(NULL, CSIDL_PERSONAL, NULL, SHGFP_TYPE_CURRENT, path))) {
		return std::string(path);
	}
	return "";
}
#endif
