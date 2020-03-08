#pragma once

/*

	MAIET Entertainment
	Zip Memory Extractor Class

*/

#include <cstdio>
#include <list>
#include "GlobalTypes.h"
#include "StringView.h"

#define MZIPREADFLAG_ZIP		1
#define MZIPREADFLAG_MRS		1<<1
#define MZIPREADFLAG_MRS2		1<<2
#define MZIPREADFLAG_FILE		1<<3

enum MZipMode{
	ZMode_Zip = 0,
	ZMode_Mrs,
	ZMode_Mrs2,
	ZMode_End
};

struct MZIPDIRHEADER;
struct MZIPDIRFILEHEADER;
struct MZIPLOCALHEADER;

class MZip final {
public:
	MZip();
	~MZip();

	bool Initialize(FILE* fp, unsigned long ReadMode);
	bool Initialize(const char* File, size_t Size, unsigned long ReadMode);
	bool Finalize();

	void SetReadMode(unsigned long mode) { m_dwReadMode = mode; }
	bool isMode(unsigned long mode) const { return (m_dwReadMode & mode) ? true : false; }
	bool isReadAble(unsigned long mode);

	int GetFileCount(void) const;

	void GetFileName(int i, char *szDest) const;
	StringView GetFileName(int i) const;

	int GetFileIndex(const char* szFileName) const;

	int GetFileLength(int i) const;
	int GetFileLength(const char* filename) const;

	// crc32
	unsigned int GetFileCRC32(int i) const;
	unsigned int GetFileCRC32(const char* filename) const;

	// get modified time
	unsigned int GetFileTime(int i) const;
	unsigned int GetFileTime(const char* filename) const;

	size_t GetFileArchiveOffset(int i);
	size_t GetFileCompressedSize(int i) const;
	bool IsFileCompressed(int i) const;

	// Read File Raw Data by Index
	bool ReadFile(int i, void* pBuffer, int nMaxSize);
	bool ReadFile(const char* filename, void* pBuffer, int nMaxSize);

	bool isVersion1Mrs();
	bool isZip();
	static bool isVersion1Mrs(FILE* fp);
	static bool isZip(FILE* fp);

	static bool ConvertZip(char* zip_name);
	static bool UpgradeMrs(char* mrs_name);//MrsToMrs2

	static bool RecoveryZip(char* zip_name);
	static bool RecoveryMrs(FILE* fp);
	static bool RecoveryMrs2(FILE* fp);

protected:
	bool InitializeImpl();
	void Seek(i64 Offset, u32 Origin);
	i64 Tell();
	void ReadN(void* Out, size_t Size);
	template <typename T>
	void Read(T& Out) { ReadN(&Out, sizeof(Out)); }

	FILE*						m_fp;			// Refered File Pointer
	char*						m_pDirData;		// Directory Data Block
	const MZIPDIRFILEHEADER**	m_ppDir;		// Directory File Header
	int							m_nDirEntries;	// Number of Directory Entries

	MZipMode					m_nZipMode;
	unsigned long				m_dwReadMode;

	const char* FileBuffer{};
	size_t FileSize{};
	i64 Pos{};
};

class FNode {
public:
	FNode();
	void SetName(const char* str);

public:
	int	 m_size;
	int	 m_offset;
	char m_name[256];
};

class FFileList : public std::list<FNode*>
{
public:
	FFileList();
	virtual ~FFileList();

	void Add(FNode* pNode);

	void DelAll();

	void UpgradeMrs();

	void ConvertZip();
	void RecoveryZip();
	void ConvertVtf();

	void ConvertNameMRes2Zip();
	void ConvertNameZip2MRes();
};

bool GetDirList(const char* path, FFileList& pList);
bool GetFileList(const char* path, FFileList& pList);
bool GetFileListWin(const char* path, FFileList& pList);
bool GetFindFileList(const char* path, const char* ext, FFileList& pList);
bool GetFindFileListWin(const char* path, const char* ext, FFileList& pList);