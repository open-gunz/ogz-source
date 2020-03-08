#pragma once

#include "GlobalTypes.h"
#include <string>

#ifdef WIN32
#pragma comment(lib, "Shlwapi.lib")
#endif

using HANDLE = void*;
using FILETIME = struct _FILETIME;

bool GetLastUpdate(const char *pFileName, FILETIME *ft);

bool IsModifiedOutside(const char *pFileName, FILETIME ot);
bool RemoveExtension(char *pRemoveExt, int maxlen, const char *pFileName);
void ReplaceExtension(char *pTargetName, int maxlen, const char *pSourceName, char *pExt);

template<size_t size> void GetRelativePath(char(&pRelativePath)[size], const char *pBasePath,
	const char *pPath)
{
	GetRelativePath(pRelativePath, size, pBasePath, pPath);
}

void GetRelativePath(char *pRelativePath, int maxlen, const char *pBasePath, const char *pPath);

void GetFullPath(char *pFullPath, int maxlen, const char *pBasePath, const char *pRelativePath);
void GetFullPath(char *pFullPath, int maxlen, const char *pRelativePath);

template<size_t size> void GetPurePath(char(&pPurePath)[size], const char *pFilename) {
	GetPurePath(pPurePath, size, pFilename);
}
void GetPurePath(char *pPurePath, int maxlen, const char *pFilename);
template <size_t size>
void GetPureFilename(char (&PureFilename)[size],const char *pFilename)
{
	char drive[3], dir[256], ext[256];
	_splitpath_s(pFilename, drive, dir, PureFilename, ext);
}
template <size_t size>
void GetPureExtension(char (&PureExtension)[size],const char *pFilename)
{
	char drive[3], dir[256], fname[256];
	_splitpath_s(pFilename, drive, dir, fname, PureExtension);
}

bool IsFullPath(const char *pPath);

bool ReadHeader(HANDLE hFile, void *pHeader, int nHeaderSize);

u32 GetFileCheckSum(char* pszFileName);

bool IsExist(const char *filename);

void GetParentDirectory(char* pszFileName);

bool MakePath(const char* pszFileName);

void time_tToFILETIME(time_t t, FILETIME* pft);

bool MSetFileTime(const char* lpszPath, FILETIME ft);

std::string GetMyDocumentsPath();

bool CreatePath(char* path);
