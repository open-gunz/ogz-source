#include "stdafx.h"
#include "MZFile.h"
#include "MDebug.h"
#include "zlib_util.h"

u32 MZFile::ReadMode = MZIPREADFLAG_ZIP | MZIPREADFLAG_MRS | MZIPREADFLAG_MRS2 | MZIPREADFLAG_FILE;

constexpr auto val = sizeof(MZFile);

template <size_t size>
static void GetFullFilePath(char(&Output)[size], const MZFileDesc& Desc, MZFileSystem& FS)
{
	sprintf_safe(Output, "%s%.*s",
		FS.GetBasePath(),
		Desc.Path.size(), Desc.Path.data());
}

template <size_t size>
static void GetFullArchivePath(char(&Output)[size], const MZFileDesc& Desc, MZFileSystem& FS)
{
	sprintf_safe(Output, "%s%.*s",
		FS.GetBasePath(),
		Desc.ArchivePath.size(), Desc.ArchivePath.data());
}

MZFile::MZFile() = default;

MZFile::~MZFile()
{
	Close();
}

bool MZFile::Open(const char* szFileName, MZFileSystem* pZFS)
{
	Close();

	if (!pZFS)
	{
		if (!isMode(MZIPREADFLAG_FILE))
		{
			MLog("MZFile::Open -- Was asked to read non-zipped file,"
				"but MZIPREADFLAG_FILE is not enabled\n");
			assert(false);
			return false;
		}

		fp = CFilePtr{fopen(szFileName, "rb")};
		if (!fp) {
			return false;
		}

		fseek(fp.get(), 0, SEEK_END);
		FileSize = ftell(fp.get());
		fseek(fp.get(), 0, SEEK_SET);

		return true;
	}
	else
	{
		auto pDesc = pZFS->GetFileDesc(szFileName);
		if (pDesc == nullptr) {
			return false;
		}

		// Check if it's in the file cache.
		{
			auto CacheIt = pZFS->CachedFileMap.find(pDesc);
			if (CacheIt != pZFS->CachedFileMap.end())
			{
				Data = DataPtr{ CacheIt->second.get(), MaybeArrayDeleter{ false } };
				FileSize = pDesc->Size;
				this->Desc = pDesc;
				return true;
			}
		}

		if (!pDesc->ArchivePath.empty())
		{
			return OpenArchive(*pDesc, *pZFS);
		}

		// ArchivePath is empty, so it's not in an archive. Try to read the plain file.
		if (!isMode(MZIPREADFLAG_FILE))
			return false;

		char FullFilePath[MFile::MaxPath];
		GetFullFilePath(FullFilePath, *pDesc, *pZFS);
		return Open(FullFilePath);
	}
}

bool MZFile::OpenArchive(const MZFileDesc& Desc, MZFileSystem& FS)
{
	char FullArchivePath[MFile::MaxPath];
	GetFullArchivePath(FullArchivePath, Desc, FS);

	fp = CFilePtr{fopen(FullArchivePath, "rb")};
	if (!fp)
	{
		MLog("MZFile::OpenArchive -- fopen failed on %s!\n", FullArchivePath);
		assert(false);
		return false;
	}
	
	this->Desc = &Desc;
	FileSize = Desc.Size;

	return true;
}

void MZFile::Close()
{
	fp = nullptr;
	Data = nullptr;
	Desc = nullptr;
	Pos = 0;
	FileSize = 0;
}

// Converts a MZFile::SeekPos value to an origin value for fseek.
static u32 ConvertSeek(MZFile::SeekPos Value)
{
	switch (Value)
	{
	case MZFile::begin:
		return SEEK_SET;
	case MZFile::current:
		return SEEK_CUR;
	case MZFile::end:
		return SEEK_END;
	}
	return static_cast<u32>(-1);
}

bool MZFile::Seek(i64 offset, SeekPos mode)
{
	const auto Length = i64(GetLength());
	if (mode == begin)
	{
		if (offset >= Length)
			return false;
	}
	else if (mode == current)
	{
		if (Pos + offset > Length)
			return false;
	}
	else if (mode == end)
	{
		if (Length + offset > Length)
			return false;
	}

	if (!IsArchive())
	{
		const auto origin = ConvertSeek(mode);
		fseek(fp.get(), static_cast<long>(offset), origin);
		return true;
	}

	if (mode == begin)
	{
		Pos = offset;
		return true;
	}
	else if (mode == current)
	{
		Pos += offset;
		return true;
	}
	else if (mode == end)
	{
		Pos = FileSize + offset;
		return true;
	}

	return false;
}

i64 MZFile::Tell() const
{
	if (IsArchive())
		return static_cast<i64>(Pos);

	return ftell(fp.get());
}

bool MZFile::Read(void* pBuffer, int nMaxSize)
{
	if (!IsArchive())
	{
		size_t numread = fread(pBuffer, 1, nMaxSize, fp.get());
		return numread == nMaxSize;
	}

	if (nMaxSize > FileSize - Pos)
		return false;

	if (!Data) {
		if (!LoadFile()) {
			return false;
		}
	}

	memcpy(pBuffer, Data.get() + Pos, nMaxSize);

	Pos += nMaxSize;

	return true;
}

bool MZFile::LoadFile()
{
	SetData(new char[FileSize + 1], true);
	Data[FileSize] = 0;

	if (!IsArchive()) {
		return fread(Data.get(), GetLength(), 1, fp.get()) == 1;
	}

	// Seek to the start of the DEFLATE data.
	auto err = fseek(fp.get(), Desc->ArchiveOffset, SEEK_SET);
	if (err != 0)
	{
		MLog("MZFile::LoadFile -- fseek failed (%d)\n", err);
		return false;
	}

	if (Desc->CompressedSize == 0)
	{
		// Not compressed. Just read the data.
		return fread(Data.get(), Desc->Size, 1, fp.get()) == 1;
	}

	// Compressed. Read and inflate the data.
	auto ret = InflateFile(Data.get(), Desc->Size, fp.get(), Desc->CompressedSize, -MAX_WBITS);
	if (ret.ErrorCode < 0)
	{
		MLog("MZFile::LoadFile -- InflateFile failed with error code %d, error message: %s, "
			"written %d, read %d\n",
			ret.ErrorCode, ret.ErrorMessage, ret.BytesWritten, ret.BytesRead);
		if (ret.ErrorCode == Z_ERRNO)
		{
			const auto err = errno;
			char strerror_buffer[512];
			auto strerror_ret = strerror_safe(err, strerror_buffer);
			if (strerror_ret != 0)
			{
				sprintf_safe(strerror_buffer, "strerror_safe failed with error code %d",
					strerror_ret);
			}

			MLog("errno = %d, strerror(errno) = %s\n",
				err, strerror_buffer);
		}
		assert(false);
		return false;
	}

	return true;
}

MZFile::DataPtr MZFile::Release()
{
	if (!Data) {
		LoadFile();
	}

	return std::move(Data);
}