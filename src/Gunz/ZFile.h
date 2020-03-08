#pragma once

#include <stdio.h>
#include "zlib.h"

#define BUFFER_SIZE	1024

class ZFile {
	FILE			*m_pFile;
	bool			m_bWrite;
	z_stream		m_Stream;
	unsigned char	m_Buffer[BUFFER_SIZE];

public:
	ZFile();
	virtual ~ZFile();

	bool Open(const char *szFileName,bool bWrite = false);

	int Read(void *pBuffer,int nByte);
	int Write(const void *pBuffer,int nByte);

	template <typename T>
	int Read(T &obj)
	{
		return Read(&obj, sizeof(T)) / sizeof(T);
	}
	template <typename T, size_t size>
	int Read(T(&obj)[size])
	{
		int ItemsRead = 0;
		for (int i = 0; i < size; i++)
		{
			ItemsRead += Read(obj[i]);
		}
		return ItemsRead;
	}
	template <typename T>
	bool Write(const T& Obj)
	{
		return Write(&Obj, sizeof(Obj)) == sizeof(Obj);
	}

	bool Close();
};

ZFile *zfopen(const char *szFileName,bool bWrite = false);
int zfread(void *pBuffer,int nItemSize,int nItemCount,ZFile *pFile);
int zfwrite(void *pBuffer,int nItemSize,int nItemCount,ZFile *pFile);
bool zfclose(ZFile *pFile);