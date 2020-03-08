/*
	LibPak.cpp
	----------

	Programming by Chojoongpil
	All copyright (c) 1997, MAIET entertainment software
*/
#include "stdafx.h"
#include <stdio.h>
#include "LibPak.h"

#ifdef _MSC_VER
#include "MWindows.h"
#else
#include <string.h>
#include "MDebug.h"
void OutputDebugString(const char* p){ mlog("%s", p); }
#endif

//	Modified by Leejangho ( 98-01-10 5:30:04 오전 )
//		u8* MemoryMappedFile::Open( char *lpszFileName )
u8* MemoryMappedFile::Open( const char *lpszFileName, bool bReadOnly )
//	Modified by ...
{
	Close();

#ifdef _MSC_VER
	// Modified by Leejangho ( 98-01-30 5:58:49 오후 )
	//		m_fileHandle=CreateFile(lpszFileName,GENERIC_READ,FILE_SHARE_READ,NULL,OPEN_EXISTING,FILE_FLAG_RANDOM_ACCESS,NULL);
	if(bReadOnly==TRUE)
		m_fileHandle=CreateFileA(lpszFileName,GENERIC_READ,FILE_SHARE_READ,NULL,OPEN_EXISTING,FILE_FLAG_RANDOM_ACCESS,NULL);
	else
		m_fileHandle=CreateFileA(lpszFileName,GENERIC_READ|GENERIC_WRITE,0,NULL,OPEN_EXISTING,FILE_FLAG_RANDOM_ACCESS,NULL);
	if(m_fileHandle==INVALID_HANDLE_VALUE){	
#ifdef _DEBUG
		OutputDebugString("MemoryMappedFile : (Open) Fail, CreateFile\n");
#endif
		return NULL; 
	}
	
	m_nFileSize=::GetFileSize(m_fileHandle,NULL);
	if(m_nFileSize==0xFFFFFFFF){
		CloseHandle(m_fileHandle);
#ifdef _DEBUG
		OutputDebugString("MemoryMappedFile : (Open) Fail, GetFileSize\n");
#endif
		return NULL;
	}

	//	Modified by Leejangho ( 98-01-10 5:30:54 오전 )
	//		Support Read&Write
	/*
	m_mapHandle=CreateFileMapping(m_fileHandle,NULL,PAGE_READONLY,0,0,NULL);
	if(m_mapHandle==NULL){
		CloseHandle(m_fileHandle);
#ifdef _DEBUG
		OutputDebugString("MemoryMappedFile : (Open) Fail, CreateFileMapping\n");
#endif
		return NULL;
	}
	*/
	if(bReadOnly==TRUE)
		m_mapHandle=CreateFileMapping(m_fileHandle,NULL,PAGE_READONLY,0,m_nFileSize,NULL);
	else
		m_mapHandle=CreateFileMapping(m_fileHandle,NULL,PAGE_READWRITE,0,m_nFileSize,NULL);
	if(m_mapHandle==NULL){
		CloseHandle(m_fileHandle);
#ifdef _DEBUG
		OutputDebugString("MemoryMappedFile : (Open) Fail, CreateFileMapping\n");
#endif
		return NULL;
	}
	//	Modified by ...

	
	if(bReadOnly==TRUE)
		m_lpPointer=(u8*)MapViewOfFile(m_mapHandle,FILE_MAP_READ,0,0,0);
	else
		m_lpPointer=(u8*)MapViewOfFile(m_mapHandle,FILE_MAP_READ|FILE_MAP_WRITE,0,0,0);
	if(m_lpPointer==NULL){
		CloseHandle(m_mapHandle);
		CloseHandle(m_fileHandle);
#ifdef _DEBUG
		OutputDebugString("MemoryMappedFile : (Open) Fail, MapViewOfFile\n");
#endif
		return NULL;
	}

#else
	assert(false);
#endif

	m_bOpened = TRUE;

	return m_lpPointer;	
}


void MemoryMappedFile::Close(void)
{
#ifdef _MSC_VER
	if(m_lpPointer){
		UnmapViewOfFile(m_lpPointer);
		m_lpPointer = NULL;
	}

	if( m_mapHandle ){
		CloseHandle(m_mapHandle);
		m_mapHandle = NULL;
	}

	if( m_fileHandle ){
		CloseHandle(m_fileHandle);
		m_fileHandle = NULL;
	}
#else
	assert(false);
#endif

	m_nPosition = 0;
	m_nFileSize = 0;

	m_bOpened = FALSE;
}


// ---------------------------------------------------------------------------------

bool Package::Open( char *szFileName )
{
	PAKFILEHEADER pfh;
#ifdef _DEBUG
	char szMessage[128];
#endif

	Close();

	if( m_mmf.Open( szFileName ) == NULL ){
#ifdef _DEBUG
		OutputDebugString("Package : (Open) Fail, m_mmf.Open\n");
#endif
		return FALSE;
	}
	m_mmf.ReadFile( &pfh, sizeof(PAKFILEHEADER) );
	if( strcmp( pfh.szID, "PAK" ) != 0 ){
		m_mmf.Close();
#ifdef _DEBUG
		OutputDebugString("Package : (Open) Fail, Unrecognize PAK Header\n");
#endif
		return FALSE;
	}
	
	m_nFiles = pfh.nCount;

	PAKDATAINFO* pInfo;

	for(DWORD i=0;i<m_nFiles;i++) {
		pInfo = new PAKDATAINFO;
//		m_mmf.SetFilePointer( sizeof(PAKFILEHEADER) , FILE_BEGIN );
		if(!m_mmf.ReadFile( pInfo, sizeof(PAKDATAINFO) )) return false;
		m_FileList.Add(pInfo);
	}

#ifdef _DEBUG
	sprintf_safe( szMessage, "Package : Success to open package. It contains %d file(s).\n", m_nFiles );
	OutputDebugString(szMessage);
#endif

	if( m_nFiles == 0 ){
		m_mmf.Close();
#ifdef _DEBUG
		OutputDebugString("Package : (Open) Fail, PAK data is empty\n");
#endif
		return FALSE;
	}
		
	return TRUE;
}

void Package::Close()
{
	if( m_mmf.IsOpen() ) m_mmf.Close();
	m_nFiles = 0;
	m_FileList.DeleteAll();
}

LPPAKDATA Package::GetPakData( unsigned long ulIndex )
{
	LPPAKDATA pPakData;
	PAKDATAINFO pdi;
	DWORD dwReadBytes;
	u8* pStartPointer;

	if( !m_mmf.IsOpen() ){ 
#ifdef _DEBUG
		OutputDebugString("Package : (GetPakData) Fail, memory mapped file don't be opened.\n");
#endif
		return NULL; 
	}
#ifdef _DEBUG	// zero based index
	_ASSERT( ulIndex < m_nFiles );
#endif
	m_mmf.SetFilePointer( sizeof(PAKFILEHEADER) + sizeof(PAKDATAINFO) * ulIndex, FILE_BEGIN );
	dwReadBytes = m_mmf.ReadFile( &pdi, sizeof(PAKDATAINFO) );
	if( !dwReadBytes ){
#ifdef _DEBUG
		OutputDebugString("Package : (GetPakData) Fail, m_mmf.ReadFile\n");
#endif
		return NULL;	
	}
	pStartPointer = m_mmf.GetStartFilePointer();
	pStartPointer += sizeof(PAKFILEHEADER)+sizeof(PAKDATAINFO)*m_nFiles+pdi.ulOffset;
	pPakData = new PAKDATA(pStartPointer, pdi.ulSize);
		
	return pPakData;
}

LPPAKDATA Package::GetPakData(char* Name)
{
	LPPAKDATA pPakData;
//	PAKDATAINFO pdi;
//	DWORD dwReadBytes;
	u8* pStartPointer;

	if( !m_mmf.IsOpen() ){ 
#ifdef _DEBUG
		OutputDebugString("Package : (GetPakData) Fail, memory mapped file don't be opened.\n");
#endif
		return NULL; 
	}

	PAKDATAINFO* pInfo = NULL;

	for(int i=0;i<m_FileList.GetCount();i++) {
		pInfo = m_FileList.Get(i);

		if(pInfo) {
			if(_stricmp(Name,pInfo->pszAlias)==0) {

				pStartPointer = m_mmf.GetStartFilePointer();
				pStartPointer += sizeof(PAKFILEHEADER)+sizeof(PAKDATAINFO)*m_nFiles+pInfo->ulOffset;
				pPakData = new PAKDATA( pStartPointer, pInfo->ulSize );
				return pPakData;
			}
		}
	}

	return NULL;
}

// ---------------------------------------------------------------------------------



