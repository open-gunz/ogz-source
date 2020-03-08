#include "stdafx.h"
#include "ZFile.h"

#include "crtdbg.h"
#define CHECK_ERR(err, msg) { \
	if (err != Z_OK) { \
	fprintf(stderr, "%s error: %d\n", msg, err); \
	return 0; \
	} \
}

ZFile::ZFile()
{
	m_pFile = NULL;
}

ZFile::~ZFile()
{
	if(m_pFile) Close();
}

bool ZFile::Open(const char *szFileName,bool bWrite)
{
	m_bWrite = bWrite;

	if(m_bWrite)
	{
		m_pFile = nullptr;
		auto err = fopen_s(&m_pFile, szFileName, "wb+");
		if(err != 0 || !m_pFile) return false;

		m_Stream.zalloc = (alloc_func)0;
		m_Stream.zfree = (free_func)0;
		m_Stream.opaque = (voidpf)0;

		err = deflateInit(&m_Stream, Z_DEFAULT_COMPRESSION);
		CHECK_ERR(err, "deflateInit");

		m_Stream.next_out = m_Buffer;
		m_Stream.avail_out = sizeof(m_Buffer);

	}
	else
	{
		m_pFile = nullptr;
		auto err = fopen_s(&m_pFile, szFileName, "rb");
		if(err != 0 || !m_pFile) return false;

		m_Stream.zalloc = (alloc_func)0;
		m_Stream.zfree = (free_func)0;
		m_Stream.opaque = (voidpf)0;

		err = inflateInit(&m_Stream);
		CHECK_ERR(err, "inflateInit");

		m_Stream.next_out = m_Buffer;
		m_Stream.avail_out = sizeof(m_Buffer);
		
		m_Stream.avail_in = 0;
	}

	return true;
}

int ZFile::Read(void *pBuffer,int nByte)
{
	if(!m_pFile || m_bWrite)
		return 0;

	int err;

	m_Stream.next_out = (Bytef*)pBuffer;
	m_Stream.avail_out = nByte;

	do {

		if(m_Stream.avail_in==0) {

			size_t nRead = fread(m_Buffer,1,sizeof(m_Buffer),m_pFile);
			if(nRead==0) return 0;

			m_Stream.avail_in = (uInt)nRead;
			m_Stream.next_in = m_Buffer;
		}

		err = inflate(&m_Stream, Z_NO_FLUSH);
		if (err == Z_STREAM_END) break;
		CHECK_ERR(err, "inflate");

	} while (m_Stream.avail_out > 0 && m_Stream.avail_in == 0);

	return nByte - m_Stream.avail_out;
}

int ZFile::Write(const void *pBuffer,int nByte)
{
	if (!m_pFile || !m_bWrite)
		return 0;

	m_Stream.next_in  = (Bytef*)pBuffer;
	m_Stream.avail_in = (uInt)nByte;

	int err = deflate(&m_Stream, Z_NO_FLUSH);
	CHECK_ERR(err, "deflate");

	while (m_Stream.avail_out == 0)
	{
		size_t nWritten = fwrite(m_Buffer,1,sizeof(m_Buffer),m_pFile);
		if(nWritten<sizeof(m_Buffer)) return 0;

		m_Stream.avail_out = sizeof(m_Buffer);
		m_Stream.next_out = m_Buffer;

		err = deflate(&m_Stream, Z_NO_FLUSH);
		CHECK_ERR(err, "deflate");
	}

	return nByte;
}

bool ZFile::Close()
{
	int err;

	if(m_bWrite) {
		/* Finish the stream, still forcing small buffers: */
		for (;;) {
			err = deflate(&m_Stream, Z_FINISH);

			fwrite(m_Buffer, 1, sizeof(m_Buffer) - m_Stream.avail_out, m_pFile);
			m_Stream.avail_out = sizeof(m_Buffer);
			m_Stream.next_out = m_Buffer;

			if (err == Z_STREAM_END) break;
			CHECK_ERR(err, "deflate");
		}

		err = deflateEnd(&m_Stream);
		CHECK_ERR(err, "deflateEnd");

		fclose(m_pFile);
		m_pFile = NULL;
	}else {
		err = inflateEnd(&m_Stream);
		CHECK_ERR(err, "inflateEnd");

		fclose(m_pFile);
		m_pFile = NULL;
	}

	return true;
}

ZFile *zfopen(const char *szFileName,bool bWrite) 
{
	ZFile *pNewFile = new ZFile;
	if(pNewFile->Open(szFileName,bWrite)) {
		return pNewFile;
	}

	delete pNewFile;
	return NULL;
}

int zfread(void *pBuffer,int nItemSize,int nItemCount,ZFile *pFile)
{ 
	int nByteRead = pFile->Read(pBuffer,nItemSize*nItemCount);
	return  nByteRead/nItemSize;
}

int zfwrite(void *pBuffer,int nItemSize,int nItemCount,ZFile *pFile)
{ 
	int nByteWritten = pFile->Write(pBuffer,nItemSize*nItemCount);
	return  nByteWritten/nItemSize;
}

bool zfclose(ZFile *pFile)
{
	bool bReturn = pFile->Close();
	delete pFile;
	return bReturn;
}