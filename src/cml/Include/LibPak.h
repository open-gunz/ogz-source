/*
	LibPak.h
	--------

	Memory mapped file operations service library
	This library based on Jang-ho's memory mapped file library.
	(7/16/1996)
	
	Programming by Chojoongpil
	Update. 6/3/1997

	All copyright (c) 1997, MAIET entertainment software
*/
#include "CMList.h"

#ifndef __LIBPAK_HEADER__
#define __LIBPAK_HEADER__

/* --------------------------------------------------------
		MemoryMappedFile
   -------------------------------------------------------- */

#define FILE_BEGIN           0
#define FILE_CURRENT         1
#define FILE_END             2

class MemoryMappedFile {
private:
	void*	m_fileHandle;	// file handler
	void*	m_mapHandle;	// mapped File handler

	u32	m_nFileSize;	// file size of memory mapped file
	u8*	m_lpPointer;	// memory pointer of file head

	bool	m_bOpened;
	u32	m_nPosition;	// current memory position
public:
	bool IsOpen(){
		return m_bOpened; 
	}

	MemoryMappedFile(){
		m_lpPointer = nullptr;
		m_nPosition = 0;
		m_nFileSize = 0;

		m_fileHandle = nullptr;
		m_mapHandle = nullptr;
		m_bOpened = false;
	}

	~MemoryMappedFile(){
		Close();			// close all handles if avail.
	}

	// open memory mapped file.
	//	Modified by Leejangho ( 98-01-10 5:30:04 오전 )
	//		u8* Open( char *lpszFileName )
	u8* Open(const char *lpszFileName, bool bReadOnly = true);
	//	Modified by ...

	// close memory mapped file.
	void Close(void);
	
	// get size of memory mapped file.
	u32 GetFileSize(){ return m_nFileSize; }
	
	// get current file pointer
	u8* GetFilePointer(){ return (m_lpPointer + m_nPosition); }
	u8* GetStartFilePointer(){ return m_lpPointer; }
	
	// set current file pointer
	bool SetFilePointer(u32 lDistanceToMove, u32 dwMoveMethod = FILE_CURRENT){
		if( dwMoveMethod == FILE_BEGIN ){
			if( lDistanceToMove < m_nFileSize ){
				m_nPosition = lDistanceToMove;
				return true;
			}
		} else if( dwMoveMethod == FILE_END ){
			if( m_nFileSize - lDistanceToMove > 0 ){
				m_nPosition = (m_nFileSize - 1) - lDistanceToMove;
				return true;
			}
		} else {	// FILE_CURRENT
			if( m_nPosition + lDistanceToMove < m_nFileSize ){
				m_nPosition += lDistanceToMove;
				return true;
			}			
		}
		return false;
	}

	// read a byte from memory mapped file
	u8 ReadByte(){ 
		m_nPosition ++;
		return *(m_lpPointer+m_nPosition); 
	}

	// read data from memory mapped file
	// return value : actual reading data length
	u32 ReadFile(void* lpBuffer, u32 dwSize){
		u32 dwReadSize;

		assert(lpBuffer != nullptr);

		if( m_nPosition + dwSize >= m_nFileSize ){
			dwReadSize = m_nFileSize - 1 - m_nPosition;
		} else dwReadSize = dwSize;

		memcpy(lpBuffer, m_lpPointer+m_nPosition, dwReadSize);
		m_nPosition += dwReadSize;

		return dwReadSize;
	}
};

#define ALIASLENGTH		80

/* ---------------------------------
	Data Structures
	---------------

	PAK

	+-------------+
	|    Header   |
	+-------------+
	|   DATAINFO  |
	+-------------+
	|   DATAINFO  | DATAINFO는 Header에서
	+-------------+ 명시한 갯수 만큼이 놓여진다.
	|    DATA     |
	+-------------+
	|    DATA     |
	+-------------+
	
   --------------------------------- */
#pragma pack(1)
typedef struct _tagPakHeader {
	char			szID[4];	// PAK
	long			nVerMajor;	// Major version is 1
	long			nVerMinor;	// Minor version is 0
	unsigned long	nCount;		// 포함되어진 package 화일 갯수
} PAKFILEHEADER, *LPPAKFILEHEADER;

typedef struct {
	char			pszAlias[ALIASLENGTH];
	unsigned long	ulOffset;
	unsigned long	ulSize;
} PAKDATAINFO, *LPPAKDATAINFO;
#pragma pack()

/* --------------------------------------------------------
		PakData
   -------------------------------------------------------- */
class Package;

class PakData {
private:
	u8* m_lpPointer;		// start pointer of extract data
	u32 m_nPosition;		// current position

	unsigned long m_ulSize;	// block size

	// set start pointer (m_lpPointer) and sizes
	void ResetPakData(u8* p, unsigned long ulSize){
		assert(p!=nullptr);
		assert(ulSize!=0);

		m_lpPointer = p;
		m_ulSize = ulSize;
		m_nPosition = 0;
	}

public:
	// Constructor
	PakData(u8* p, unsigned long ulSize){
		ResetPakData(p, ulSize);
	}

	unsigned long GetFileSize(){ return m_ulSize; }
	u8* GetStartFilePointer(){ return m_lpPointer; }

	// get current file pointer
	u8* GetFilePointer(){ return (m_lpPointer + m_nPosition); }
	
	// set current file pointer
	bool SetFilePointer(u32 lDistanceToMove, u32 dwMoveMethod = FILE_CURRENT){
		if( dwMoveMethod == FILE_BEGIN ){
			if( lDistanceToMove < m_ulSize ){
				m_nPosition = lDistanceToMove;
				return true;
			}
		} else if( dwMoveMethod == FILE_END ){
			if( m_ulSize - lDistanceToMove > 0 ){
				m_nPosition = (m_ulSize - 1) - lDistanceToMove;
				return true;
			}
		} else {	// FILE_CURRENT
			if( m_nPosition + lDistanceToMove < m_ulSize ){
				m_nPosition += lDistanceToMove;
				return true;
			}			
		}
		return false;
	}

	// read data from pak data
	// return value : actual reading data length
	u32 ReadFile(void* lpBuffer, u32 dwSize){
		u32 dwReadSize;

		assert(lpBuffer != nullptr);

		if( m_nPosition + dwSize > m_ulSize ){
			dwReadSize = m_ulSize - 1 - m_nPosition;
		} else dwReadSize = dwSize;

		memcpy(lpBuffer, m_lpPointer+m_nPosition, dwReadSize);
		m_nPosition += dwReadSize;

		return dwReadSize;
	}
	
	friend Package;
};

typedef PakData PAKDATA, *LPPAKDATA;

/* --------------------------------------------------------
		Package
   -------------------------------------------------------- */

class Package {
private:
	unsigned long m_nFiles;		// Package내에 포함된 파일의 갯수
	MemoryMappedFile m_mmf;		// Memory mapped file object

public:
	CMLinkedList<PAKDATAINFO>	m_FileList;

	Package(){
		m_nFiles = 0;
	}
	~Package(){
		Close();
	}

	bool Open( char *lpszFilename );
	void Close();

	// get PAK data object from package file
	LPPAKDATA GetPakData(unsigned long ulIndex);
	LPPAKDATA GetPakData(char* Name);
};


#endif // __LIBPAK_HEADER__