#include "stdafx.h"
#include "MZip.h"
#include "zip/zlib.h"
#include <memory.h>
#include <string.h>
#include "MDebug.h"
#include <cassert>
#include "zlib_util.h"
#include <algorithm>

typedef unsigned long dword;
typedef unsigned short word;

#define MRS_ZIP_CODE	0x05030207
#define MRS2_ZIP_CODE	0x05030208

#pragma pack(2)
struct MZIPLOCALHEADER{
	enum{
		SIGNATURE   = 0x04034b50,
		SIGNATURE2  = 0x85840000,
		COMP_STORE  = 0,
		COMP_DEFLAT = 8,
	};

	dword   sig;
	word    version;
	word    flag;
	word    compression;      // COMP_xxxx
	word    modTime;
	word    modDate;
	dword   crc32;
	dword   cSize;
	dword   ucSize;
	word    fnameLen;         // Filename string follows header.
	word    xtraLen;          // Extra field follows filename.
};

struct MZIPDIRHEADER{
	enum{
		SIGNATURE = 0x06054b50,
	};

	dword   sig;
	word    nDisk;
	word    nStartDisk;
	word    nDirEntries;
	word    totalDirEntries;
	dword   dirSize;
	dword   dirOffset;
	word    cmntLen;
};

struct MZIPDIRFILEHEADER{
	enum{
		SIGNATURE   = 0x02014b50,
		SIGNATURE2  = 0x05024b80,
		COMP_STORE  = 0,
		COMP_DEFLAT = 8,
	};

	dword   sig;
	word    verMade;
	word    verNeeded;
	word    flag;
	word    compression;      // COMP_xxxx
	word    modTime;
	word    modDate;
	dword   crc32;
	dword   cSize;            // Compressed size
	dword   ucSize;           // Uncompressed size
	word    fnameLen;         // Filename string follows header.
	word    xtraLen;          // Extra field follows filename.
	word    cmntLen;          // Comment field follows extra field.
	word    diskStart;
	word    intAttr;
	dword   extAttr;
	dword   hdrOffset;

	char *GetName   () const { return (char *)(this + 1);   }
	char *GetExtra  () const { return GetName() + fnameLen; }
	char *GetComment() const { return GetExtra() + xtraLen; }
};

#pragma pack()

void ConvertChar(char* pData,int _size)
{
	if(!pData) return;

	u16 w;
	u8 b, bh;

	for(int i=0;i<_size;i++) {
		b = *pData ^ 0xFF;
		w = b<<3;
		bh = (w&0xff00)>>8;
		b = w&0xff;
		*pData = u8( b | bh );
		pData++;

	}
}

static void RecoveryChar(char* data, size_t size)
{
	for (size_t i = 0; i < size; ++i)
	{
		u8& c = reinterpret_cast<u8*>(data)[i];
		const u8 bh = c & 0x07;
		const u8 d = (bh << 5) | (c >> 3);
		c = d ^ 0xFF;
	}
}

MZip::MZip()
{
	m_fp = NULL;
	m_pDirData = NULL;
	m_ppDir = NULL;
	m_nDirEntries = 0;
	m_nZipMode = ZMode_Zip;
	m_dwReadMode = 0;
}

MZip::~MZip()
{
	Finalize();
}

bool MZip::isReadAble(unsigned long mode)
{
	if (m_nZipMode == ZMode_Zip) {
		return (MZIPREADFLAG_ZIP & mode) != 0;
	}
	else if (m_nZipMode == ZMode_Mrs) {
		return (MZIPREADFLAG_MRS & mode) != 0;
	}
	else if (m_nZipMode == ZMode_Mrs2) {
		return (MZIPREADFLAG_MRS2 & mode) != 0;
	}
	return false;
}

bool MZip::Initialize(FILE* fp, unsigned long ReadMode)
{
	if (fp == NULL) return false;

	m_fp = fp;
	m_dwReadMode = ReadMode;

	return InitializeImpl();
}

bool MZip::Initialize(const char * File, size_t Size, unsigned long ReadMode)
{
	FileBuffer = File;
	FileSize = Size;
	m_fp = nullptr;
	m_dwReadMode = ReadMode;

	return InitializeImpl();
}

bool MZip::InitializeImpl()
{
	if (isZip()) {
		m_nZipMode = ZMode_Zip;
		if (!isMode(MZIPREADFLAG_ZIP))
			return false;
	}
	else if (isVersion1Mrs()) {
		m_nZipMode = ZMode_Mrs;
		if (!isMode(MZIPREADFLAG_MRS))
			return false;
	}
	else {
		m_nZipMode = ZMode_Mrs2;
		if (!isMode(MZIPREADFLAG_MRS2))
			return false;
	}

	MZIPDIRHEADER dh{};

	Seek(-static_cast<int>(sizeof(dh)), SEEK_END);
	auto dhOffset = Tell();
	Read(dh);

	if (m_nZipMode >= ZMode_Mrs2)
		RecoveryChar(reinterpret_cast<char*>(&dh), sizeof(MZIPDIRHEADER));

	if (dh.sig != MRS2_ZIP_CODE && dh.sig != MRS_ZIP_CODE && dh.sig != MZIPDIRHEADER::SIGNATURE) {
		DMLog("MZip::InitializeImpl - Directory header signature %08X is wrong\n", dh.sig);
		assert(false);
		return false;
	}

	Seek(dhOffset - dh.dirSize, SEEK_SET);

	m_pDirData = new char[dh.dirSize + dh.nDirEntries * sizeof(*m_ppDir)];
	memset(m_pDirData, 0, dh.dirSize + dh.nDirEntries * sizeof(*m_ppDir));
	ReadN(m_pDirData, dh.dirSize);

	if (m_nZipMode >= ZMode_Mrs2)
		RecoveryChar((char*)m_pDirData, dh.dirSize);

	char *pfh = m_pDirData;
	m_ppDir = (const MZIPDIRFILEHEADER **)(m_pDirData + dh.dirSize);

	for (int i = 0; i < dh.nDirEntries; i++) {
		MZIPDIRFILEHEADER& fh = *(MZIPDIRFILEHEADER*)pfh;

		m_ppDir[i] = &fh;

		if (fh.sig != MZIPDIRFILEHEADER::SIGNATURE) {
			if (fh.sig != MZIPDIRFILEHEADER::SIGNATURE2) {
				delete[] m_pDirData;
				m_pDirData = NULL;
				DMLog("MZip::InitializeImpl - File header signature %08X is wrong\n", fh.sig);
				assert(false);
				return false;
			}
		}

		{
			pfh += sizeof(fh);

			for (int j = 0; j < fh.fnameLen; j++)
				if (pfh[j] == '/')
					pfh[j] = '\\';

			pfh += fh.fnameLen + fh.xtraLen + fh.cmntLen;
		}
	}

	m_nDirEntries = dh.nDirEntries;

	return true;
}

bool MZip::Finalize()
{
	if(m_pDirData!=NULL) {
		delete[] m_pDirData;
		m_pDirData=NULL;
	}

	if (m_fp)
	{
		fclose(m_fp);
		m_fp = NULL;
	}
	m_ppDir = NULL;
	m_nDirEntries = 0;

	return true;
}

int MZip::GetFileCount(void) const
{
	return m_nDirEntries;
}

void MZip::GetFileName(int i, char *szDest) const
{
	if(szDest!=NULL){
		if (i < 0 || i >= m_nDirEntries){
			*szDest = '\0';
		}
		else{
			memcpy(szDest, m_ppDir[i]->GetName(), m_ppDir[i]->fnameLen);
			szDest[m_ppDir[i]->fnameLen] = '\0';
		}
	}
}

StringView MZip::GetFileName(int i) const
{
	if (i < 0 || i >= m_nDirEntries)
		return nullptr;

	return{ m_ppDir[i]->GetName(), m_ppDir[i]->fnameLen };
}

int t_strcmp(const char* str1,const char* str2)
{
	int len = strlen(str1);
	if(strlen(str2)!=len) return -1;
	
	for(int i=0;i<len;i++) {

		if(str1[i] != str2[i]) {
			if(	((str1[i]=='\\') || (str1[i]=='/')) && ((str1[i]=='\\') || (str1[i]=='/')) ) {
				continue;
			}
			else
				return -1;
		}
	}
	return 0;
}

int MZip::GetFileIndex(const char* szFileName) const
{
	if(szFileName==NULL) return -1;

	char szSourceName[256];
	for(int i=0; i<GetFileCount();i++){
		GetFileName(i, szSourceName);
		if(t_strcmp(szFileName, szSourceName)==0) 
			return i;
	}

	return -1;
}

int MZip::GetFileLength(int i) const
{
	if(i<0 || i>=m_nDirEntries)
		return 0;
	else
		return m_ppDir[i]->ucSize;
}

int MZip::GetFileLength(const char* filename) const
{
	int index = GetFileIndex(filename);

	if(index == -1) return 0;

	return GetFileLength(index);
}

unsigned int MZip::GetFileCRC32(int i) const
{
	if(i<0 || i>=m_nDirEntries)
		return 0;
	else
		return m_ppDir[i]->crc32;
}

unsigned int MZip::GetFileCRC32(const char* filename) const
{
	int index = GetFileIndex(filename);

	if(index == -1) return 0;

	return GetFileCRC32(index);
}

unsigned int MZip::GetFileTime(int i) const
{
	if(i<0 || i>=m_nDirEntries)
		return 0;
	else
		return m_ppDir[i]->modTime | (m_ppDir[i]->modDate << 16);
}

unsigned int MZip::GetFileTime(const char* filename) const
{
	int index = GetFileIndex(filename);

	if(index == -1) return 0;

	return GetFileCRC32(index);
}

size_t MZip::GetFileArchiveOffset(int i)
{
	if (i < 0 || i >= m_nDirEntries)
		return 0;

	// The fnameLen and xtraLen in m_ppDir[i] are wrong for some reason,
	// so need to get these fields from the actual local header.
	const auto PrevPos = Tell();

	Seek(m_ppDir[i]->hdrOffset, SEEK_SET);
	MZIPLOCALHEADER h;
	Read(h);

	Seek(PrevPos, SEEK_SET);

	if (m_nZipMode >= ZMode_Mrs2)
		RecoveryChar(reinterpret_cast<char*>(&h), sizeof(h));

	return m_ppDir[i]->hdrOffset + sizeof(MZIPLOCALHEADER) + h.fnameLen + h.xtraLen;
}

size_t MZip::GetFileCompressedSize(int i) const
{
	if (i < 0 || i >= m_nDirEntries)
		return 0;

	return m_ppDir[i]->cSize;
}

bool MZip::IsFileCompressed(int i) const
{
	if (i < 0 || i >= m_nDirEntries)
		return 0;

	return m_ppDir[i]->compression == MZIPDIRFILEHEADER::COMP_DEFLAT;
}

void MZip::Seek(i64 Offset, u32 Origin)
{
	if (FileBuffer)
	{
		if (Origin == SEEK_SET)
			Pos = Offset;
		else if (Origin == SEEK_CUR)
			Pos += Offset;
		else if (Origin == SEEK_END)
			Pos = FileSize + Offset;
		return;
	}

	assert(m_fp);
	fseek(m_fp, static_cast<long>(Offset), Origin);
}

i64 MZip::Tell()
{
	if (FileBuffer)
		return Pos;

	return ftell(m_fp);
}

void MZip::ReadN(void* Out, size_t Size)
{
	if (FileBuffer)
	{
		memcpy(Out, FileBuffer + Pos, Size);
		Pos += Size;
		return;
	}

	auto ret = fread(Out, Size, 1, m_fp);
	//assert(ret == 1);
}

bool MZip::ReadFile(int i, void* pBuffer, int nMaxSize)
{
	if (pBuffer==NULL || i<0 || i>=m_nDirEntries)
		return false;

	Seek(m_ppDir[i]->hdrOffset, SEEK_SET);
	MZIPLOCALHEADER h;
	Read(h);

	if(m_nZipMode >= ZMode_Mrs2)
		RecoveryChar((char*)&h,sizeof(h));

	if(h.sig!=MZIPLOCALHEADER::SIGNATURE)
		if(h.sig!=MZIPLOCALHEADER::SIGNATURE2) 
			return false;

	Seek(h.fnameLen + h.xtraLen, SEEK_CUR);

	if(h.compression==MZIPLOCALHEADER::COMP_STORE){
		ReadN(pBuffer, h.cSize);
		return true;
	}
	else if(h.compression!=MZIPLOCALHEADER::COMP_DEFLAT)
		return false;

	const auto OutputSize = std::min(static_cast<dword>(nMaxSize), h.ucSize);
	const auto WindowBits = -MAX_WBITS;
	IOResult ret;
	if (FileBuffer)
	{
		auto pData = FileBuffer + Pos;
		ret = InflateMemory(pBuffer, OutputSize, pData, h.cSize, WindowBits);
	}
	else
	{
		ret = InflateFile(pBuffer, OutputSize, m_fp, WindowBits);
	}

	if (ret.ErrorCode < 0)
	{
		MLog("MZip::ReadFile - inflate failed with error code %d, error message %d, written %d, read %d\n",
			ret.ErrorCode, ret.ErrorMessage, ret.BytesWritten, ret.BytesRead);
		return false;
	}

	return true;
}

bool MZip::ReadFile(const char* filename, void* pBuffer, int nMaxSize)
{
	int index = GetFileIndex(filename);

	if(index == -1) return false;

	return ReadFile(index , pBuffer , nMaxSize);
}

static char _fileheaderReader[1024*16]; // sizeof(fh) + fh.fnameLen + fh.xtraLen + fh.cmntLen
static int	_fileheaderReaderSize = 0;

bool MZip::UpgradeMrs(char* mrs_name) // Mrs To Mrs2
{
	auto fp = fopen(mrs_name, "rb+");

	if(!fp) {
		mlog("MZip::UpgradeMrs - Failed to open %s\n", mrs_name);
		return false;
	}

	if( isVersion1Mrs(fp)==false )
	{
		fclose(fp);
		return false;
	}

	fseek(fp, 0, SEEK_SET);
	int code = MZIPLOCALHEADER::SIGNATURE;
	fwrite(&code, 4, 1, fp);


	MZIPDIRHEADER dh;
	fseek(fp, -(int)sizeof(MZIPDIRHEADER), SEEK_END);

	long dhOffset = ftell(fp);

	fread(&dh, sizeof(dh), 1, fp);

	dh.sig = MZIPDIRHEADER::SIGNATURE;

	long dir_data_pos = dhOffset - dh.dirSize;
	long dir_data_size = dh.dirSize;

	//////////////////////////////////////////////////////////////////

	fseek(fp, dir_data_pos, SEEK_SET);

	char* pDirData = new char[dir_data_size];
	memset(pDirData, 0, dir_data_size);
	fread(pDirData, dir_data_size, 1, fp);

	u32 _sig = MZIPDIRFILEHEADER::SIGNATURE;

	for(int i=0;i<dir_data_size-3;i++) {

		if((u8)pDirData[i] == 0x80) {
			if((u8)pDirData[i+1] == 0x4b) {
				if((u8)pDirData[i+2] == 0x02) {
					if((u8)pDirData[i+3] == 0x05) {
						memcpy(&pDirData[i], &_sig,4);
					}
				}
			}
		}
	}

	////////////////////////////////////////////////////////////////
	// local header 

	char* pTPos = pDirData;

	MZIPDIRFILEHEADER fh;
	MZIPLOCALHEADER h;

	for (int i = 0; i < dh.nDirEntries; i++) {

		fh = *(MZIPDIRFILEHEADER*)pTPos;

		fseek(fp, fh.hdrOffset, SEEK_SET);

		fread(&h, sizeof(h), 1, fp);

		_fileheaderReaderSize = h.fnameLen + h.xtraLen;

		if(_fileheaderReaderSize)
			fread(_fileheaderReader, _fileheaderReaderSize, 1, fp);

		ConvertChar( (char*)&h , sizeof(h) );
		ConvertChar( _fileheaderReader , _fileheaderReaderSize );

		fseek(fp, fh.hdrOffset, SEEK_SET);

		fwrite(&h,sizeof(h), 1, fp);

		if(_fileheaderReaderSize)
			fwrite(_fileheaderReader , _fileheaderReaderSize, 1, fp);

		pTPos += sizeof(fh) + fh.fnameLen + fh.xtraLen + fh.cmntLen;
	}

	////////////////////////////////////////////////////////////////
	// mrs signature

	ConvertChar( (char*)pDirData , dir_data_size );

	fseek(fp, dir_data_pos, SEEK_SET);
	fwrite(pDirData,dir_data_size,1,fp);

	//////////////////////////////////////////////////////////

	fseek(fp, dhOffset - dh.dirSize, SEEK_SET);

	delete [] pDirData;

	///////////////////////////////////////////////////////////////////

	dh.sig = MRS2_ZIP_CODE;

	ConvertChar((char*)&dh,sizeof(MZIPDIRHEADER));

	fseek(fp, -(int)sizeof(MZIPDIRHEADER), SEEK_END);

	fwrite(&dh, sizeof(dh), 1, fp);

	fclose(fp);

	return true;
}

bool MZip::ConvertZip(char* zip_name)
{
	FILE* fp = fopen(zip_name, "rb+");

	if(!fp) {
		mlog("MZip::ConvertZip - Failed to open %s\n", zip_name);
		return false;
	}

	MZIPDIRHEADER dh;
	fseek(fp, -(int)sizeof(MZIPDIRHEADER), SEEK_END);

	long dhOffset = ftell(fp);

	fread(&dh, sizeof(dh), 1, fp);

	long dir_data_pos = dhOffset - dh.dirSize;
	long dir_data_size = dh.dirSize;

	//////////////////////////////////////////////////////////////////

	fseek(fp, dir_data_pos, SEEK_SET);

	char* pDirData = new char[dir_data_size];
	memset(pDirData, 0, dir_data_size);
	fread(pDirData, dir_data_size, 1, fp);

	////////////////////////////////////////////////////////////////
	// local header 

	char* pTPos = pDirData;

	MZIPDIRFILEHEADER fh;
	MZIPLOCALHEADER h;

	for (int i = 0; i < dh.nDirEntries; i++) {

		fh = *(MZIPDIRFILEHEADER*)pTPos;

		fseek(fp, fh.hdrOffset, SEEK_SET);

		fread(&h, sizeof(h), 1, fp);

		_fileheaderReaderSize = h.fnameLen + h.xtraLen;

		if(_fileheaderReaderSize)
			fread(_fileheaderReader, _fileheaderReaderSize, 1, fp);

		ConvertChar( (char*)&h , sizeof(h) );
		ConvertChar( _fileheaderReader , _fileheaderReaderSize );

		fseek(fp, fh.hdrOffset, SEEK_SET);

		fwrite(&h,sizeof(h), 1, fp);

		if(_fileheaderReaderSize)
			fwrite(_fileheaderReader , _fileheaderReaderSize, 1, fp);

		pTPos += sizeof(fh) + fh.fnameLen + fh.xtraLen + fh.cmntLen;
	}

	////////////////////////////////////////////////////////////////
	// mrs signature

	ConvertChar( (char*)pDirData , dir_data_size );

	fseek(fp, dir_data_pos, SEEK_SET);
	fwrite(pDirData,dir_data_size,1,fp);

	//////////////////////////////////////////////////////////

	fseek(fp, dhOffset - dh.dirSize, SEEK_SET);

	delete [] pDirData;

	///////////////////////////////////////////////////////////////////

	dh.sig = MRS2_ZIP_CODE;

	ConvertChar((char*)&dh,sizeof(MZIPDIRHEADER));

	fseek(fp, -(int)sizeof(MZIPDIRHEADER), SEEK_END);

	fwrite(&dh, sizeof(dh), 1, fp);

	fclose(fp);

	return true;
}

bool MZip::RecoveryMrs(FILE* fp)
{
	fseek(fp, 0, SEEK_SET);
	int code = MZIPLOCALHEADER::SIGNATURE;
	fwrite(&code, 4, 1, fp);

	MZIPDIRHEADER dh;
	fseek(fp, -(int)sizeof(MZIPDIRHEADER), SEEK_END);
	long dhOffset = ftell(fp);
	memset(&dh, 0, sizeof(dh));
	fread(&dh, sizeof(dh), 1, fp);

	dh.sig = MZIPDIRHEADER::SIGNATURE;

	long dir_data_pos = dhOffset - dh.dirSize;
	long dir_data_size = dh.dirSize;

	//////////////////////////////////////////////////////////////////

	fseek(fp, dir_data_pos, SEEK_SET);

	char* pDirData = new char[dir_data_size];
	memset(pDirData, 0, dir_data_size);
	fread(pDirData, dir_data_size, 1, fp);

	u32 _sig = MZIPDIRFILEHEADER::SIGNATURE;

	for(int i=0;i<dir_data_size-3;i++) {

		if((u8)pDirData[i] == 0x80) {
			if((u8)pDirData[i+1] == 0x4b) {
				if((u8)pDirData[i+2] == 0x02) {
					if((u8)pDirData[i+3] == 0x05) {
						memcpy(&pDirData[i], &_sig,4);
					}
				}
			}
		}
	}

	fseek(fp, dir_data_pos, SEEK_SET);
	fwrite(pDirData,dir_data_size,1,fp);

	delete [] pDirData;

	///////////////////////////////////////////////////////////////////

	fseek(fp, -(int)sizeof(MZIPDIRHEADER), SEEK_END);

	fwrite(&dh, sizeof(dh), 1, fp);

	return true;
}

bool MZip::RecoveryMrs2(FILE* fp)
{
	MZIPDIRHEADER dh;
	fseek(fp, -(int)sizeof(dh), SEEK_END);
	long dhOffset = ftell(fp);
	memset(&dh, 0, sizeof(dh));
	fread(&dh, sizeof(dh), 1, fp);

	RecoveryChar((char*)&dh,sizeof(dh));

	dh.sig = MZIPDIRHEADER::SIGNATURE; // ZipCode

	long dir_data_pos = dhOffset - dh.dirSize;
	long dir_data_size = dh.dirSize;

	//////////////////////////////////////////////////////////////////

	fseek(fp, dir_data_pos, SEEK_SET);

	char* pDirData = new char[dir_data_size];
	memset(pDirData, 0, dir_data_size);
	fread(pDirData, dir_data_size, 1, fp);

	RecoveryChar( (char*)pDirData , dir_data_size );//mrs 라면 변환..

	fseek(fp, dir_data_pos, SEEK_SET);
	fwrite(pDirData,dir_data_size,1,fp);


	////////////////////////////////////////////////////////////////
	// local header 

	char* pTPos = pDirData;

	MZIPDIRFILEHEADER	fh;
	MZIPLOCALHEADER h;

	for (int i = 0; i < dh.nDirEntries; i++) {

		fh = *(MZIPDIRFILEHEADER*)pTPos;

		fseek(fp, fh.hdrOffset, SEEK_SET);

		fread(&h, sizeof(h), 1, fp);

		RecoveryChar( (char*)&h , sizeof(h) );

		_fileheaderReaderSize = h.fnameLen + h.xtraLen;

		if(_fileheaderReaderSize)
			fread(_fileheaderReader, _fileheaderReaderSize, 1, fp);

		RecoveryChar( _fileheaderReader , _fileheaderReaderSize );

		fseek(fp, fh.hdrOffset, SEEK_SET);

		fwrite(&h,sizeof(h), 1, fp);

		if(_fileheaderReaderSize)
			fwrite(_fileheaderReader , _fileheaderReaderSize, 1, fp);

		pTPos += sizeof(fh) + fh.fnameLen + fh.xtraLen + fh.cmntLen;

	}

	//////////////////////////////////////////////////////////////////

	delete [] pDirData;

	///////////////////////////////////////////////////////////////////

	fseek(fp, -(int)sizeof(dh), SEEK_END);

	fwrite(&dh, sizeof(dh), 1, fp);

	return true;
}

bool MZip::isZip()
{
	Seek(0, SEEK_SET);
	u32 sig = 0;
	Read(sig);

	if (sig == MZIPLOCALHEADER::SIGNATURE)
		return true;

	return false;
}

bool MZip::isVersion1Mrs()
{
	Seek(0, SEEK_SET);
	u32 sig = 0;
	Read(sig);

	if (sig == MZIPLOCALHEADER::SIGNATURE2)
		return true;

	return false;
}

bool MZip::isZip(FILE* fp)
{
	fseek(fp, 0, SEEK_SET);
	u32 sig = 0;
	fread(&sig, sizeof(sig), 1, fp);

	if (sig == MZIPLOCALHEADER::SIGNATURE)
		return true;

	return false;
}

bool MZip::isVersion1Mrs(FILE* fp)
{
	fseek(fp, 0, SEEK_SET);
	u32 sig = 0;
	fread(&sig, sizeof(sig), 1, fp);

	if (sig == MZIPLOCALHEADER::SIGNATURE2)
		return true;

	return false;
}

bool MZip::RecoveryZip(char* zip_name)
{
	FILE* fp = fopen(zip_name, "rb+");

	if(fp == nullptr) {
		mlog("Couldn't open file %s \n", zip_name);
		return false;
	}

	// mrs1 인지 식별..이미 파일들이 나간상태여서 식별방법이 이것뿐...

	if( isVersion1Mrs(fp) ) {	// 최초모델...
		RecoveryMrs( fp );
	}
	else {
		RecoveryMrs2( fp );		// v2 부터는 헤더의 sig 값으로 버전구분....
	}

	fclose(fp);

	return true;
}

/////////////////////////////////////////////////////////////////////////////////

FNode::FNode()
{
	memset(m_name,0,256);
	m_size	 = 0;
	m_offset = 0;
}

void FNode::SetName(const char* str)	
{
	if(strlen(str) > 255) return;
	strcpy_safe(m_name,str);
}

/////////////////////////////////////////////////////////////////////////////////

FFileList::FFileList()
{

}

FFileList::~FFileList() 
{
	DelAll();
}

void FFileList::Add(FNode* pNode) 
{
	push_back(pNode);
}

void FFileList::DelAll() 
{
	if(size()==0) return;

	iterator node;

	for(node = begin(); node != end(); ) 
	{
		delete (*node);
		(*node) = NULL;
		node = erase(node);
	}
}

void FFileList::UpgradeMrs() 
{
	iterator node;
	FNode* pNode = NULL;

	for(node = begin(); node != end(); ++node) 
	{
		pNode = (*node);

		if(MZip::UpgradeMrs( pNode->m_name ))
			mlog(" Upgrade mrs : %s\n",pNode->m_name);
	}
}

void FFileList::ConvertZip() 
{
	iterator node;
	FNode* pNode = NULL;

	for(node = begin(); node != end(); ++node) {
		pNode = (*node);

		if(MZip::ConvertZip( pNode->m_name ))
			mlog("convert zip : %s\n",pNode->m_name);
	}
}

void FFileList::RecoveryZip() 
{
	iterator node;
	FNode* pNode = NULL;

	for(node = begin(); node != end(); ++node) {
		pNode = (*node);

		MZip::RecoveryZip( pNode->m_name );
	}
}

#ifdef WIN32
#include <Windows.h>
#include <shellapi.h>
#ifndef _T
#define _T(x) x
#endif
void FFileList::ConvertVtf() 
{
	iterator node;
	FNode* pNode = NULL;

	char temp[1024];
	char temp_arg[1024];
	int len;

	for(node = begin(); node != end(); ++node) {
		pNode = (*node);

		strcpy_safe(temp, pNode->m_name);
		len = strlen(temp);
		temp[len-3] = 0;
		strcat_safe(temp, "tga");

		sprintf_safe(temp_arg,"%s %s",pNode->m_name,temp);
		HINSTANCE hr = ShellExecuteA(NULL, _T("open"), _T("vtf2tga.exe"),_T(temp_arg), NULL, SW_HIDE);
	}
}
#endif

void FFileList::ConvertNameMRes2Zip() 
{
	iterator node;
	FNode* pNode = NULL;

	char _buf_rename[256];
	int len;

	for(node = begin(); node != end(); ++node) {
		pNode = (*node);

		strcpy_safe(_buf_rename,pNode->m_name);
		len = (int)strlen(pNode->m_name);

		_buf_rename[len-3] = 0;
		strcat_safe(_buf_rename,"zip");

		rename( pNode->m_name, _buf_rename);

		mlog("rename : %s -> %s \n",_buf_rename,pNode->m_name);
	}
}

void FFileList::ConvertNameZip2MRes() 
{
	iterator node;
	FNode* pNode = NULL;

	char _buf_rename[256];
	int len;

	for(node = begin(); node != end(); ++node) {
		pNode = (*node);

		strcpy_safe(_buf_rename,pNode->m_name);
		len = (int)strlen(pNode->m_name);

		_buf_rename[len-3] = 0;
		strcat_safe(_buf_rename,"mrs");

		rename( pNode->m_name, _buf_rename);

		mlog("rename : %s -> %s \n",pNode->m_name,_buf_rename);
	}
}

static bool GetEntityList(const char* path, FFileList& pList, bool Dirs)
{
	FNode* pNode;

	for (auto&& File : MFile::Glob(path))
	{
		if (strcmp(File.Name, ".") == 0)	continue;
		if (strcmp(File.Name, "..") == 0)	continue;
		if (bool(File.Attributes & MFile::Attributes::Subdir) == Dirs)	continue;

		pNode = new FNode;
		pNode->SetName(File.Name);
		pList.Add(pNode);

	}

	return true;
}

bool GetDirList(const char* path, FFileList& pList)
{
	return GetEntityList(path, pList, true);
}

bool GetFileList(const char* path, FFileList& pList)
{
	return GetEntityList(path, pList, false);
}

#if _WIN32
bool GetFileListWin(const char* path, FFileList& pList)
{
	WIN32_FIND_DATA		file_t;
	HANDLE				hFile;

	FNode* pNode;

	if( (hFile = FindFirstFile( path , &file_t )) != INVALID_HANDLE_VALUE ) {

		do {

			if(strcmp(file_t.cFileName, "." )==0)					continue;
			if(strcmp(file_t.cFileName, "..")==0)					continue;
			if(file_t.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY )	continue;

			pNode = new FNode;
			pNode->SetName(file_t.cFileName);
			pList.Add(pNode);

		} while( FindNextFile( hFile, &file_t ) );

		FindClose(hFile);
	}

	return true;
}

bool GetFindFileList(const char* path, const char* ext, FFileList& pList)
{
	_finddata_t file_t;
	long hFile;

	FNode* pNode;

	if( (hFile = _findfirst( path , &file_t )) != -1L ) {
		do{
			if(strcmp(file_t.name, "." )==0) continue;
			if(strcmp(file_t.name, "..")==0) continue;

			if(file_t.attrib & _A_SUBDIR ) {
				char _path[256];
				strcpy_safe(_path,file_t.name);
				strcat_safe(_path,"/");
				strcat_safe(_path,path);

				GetFindFileList(_path,ext,pList);
				continue;
			}

			int len = (int)strlen(ext);
			int filelen = (int)strlen(file_t.name);

			char* pName = &file_t.name[filelen-len];

			if(_stricmp(pName,ext)==0) {

				int len = (int)strlen(path);

				char temp_name[256];

				if(len > 3) {

					memcpy(temp_name,path,len-3);
					temp_name[len-3]=0;
					strcat_safe(temp_name,file_t.name);
				}
				else {
					strcpy_safe(temp_name,file_t.name);
				}

				pNode = new FNode;
				pNode->SetName(temp_name);
				pList.Add(pNode);

			}

		} 
		while( _findnext( hFile, &file_t ) == 0 );

		_findclose(hFile);
	}

	return true;
}

bool GetFindFileListWin(const char* path, const char* ext, FFileList& pList)
{
	WIN32_FIND_DATA		file_t;
	HANDLE				hFile;

	FNode* pNode;

	if( (hFile = FindFirstFile( path , &file_t )) != INVALID_HANDLE_VALUE ) {

		do{
			if(strcmp(file_t.cFileName, "." )==0)	continue;
			if(strcmp(file_t.cFileName, "..")==0)	continue;

			if(file_t.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY )	{
				char _path[1024];

				int len = strlen(path);

				strcpy_safe(_path,path);
				_path[len-1] = 0;
				strcat_safe(_path,file_t.cFileName);
				strcat_safe(_path,"/*");

				GetFindFileListWin(_path,ext,pList);
				continue;
			}

			int len = (int)strlen(ext);
			int filelen = (int)strlen(file_t.cFileName);

			char* pName = &file_t.cFileName[filelen-len];

			if(_stricmp(pName,ext)==0) {

				int len = (int)strlen(path);

				char temp_name[1024];

				if(len > 1) {

					strncpy_safe(temp_name, path, len);
					strcat_safe(temp_name,file_t.cFileName);
				}
				else {
					strcpy_safe(temp_name,file_t.cFileName);
				}

				pNode = new FNode;
				pNode->SetName(temp_name);
				pList.Add(pNode);

			}

		} while( FindNextFile( hFile, &file_t ) );

		FindClose(hFile);
	}

	return true;
}
#endif
