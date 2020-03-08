#pragma once

#include <deque>
#include <unordered_map>
#include <string>
#include <algorithm>
#include "MZip.h"
#include "MUtil.h"
#include "StringView.h"
#include "MHash.h"

#define DEF_EXT	"mrs"

struct MZFileDesc
{
	// Path to actual file.
	// Relative to the file system's base path.
	StringView Path;

	// The path to the archive it's in.
	// Empty if the file is not in an archive.
	// Relative to the file system's base path.
	StringView ArchivePath;

	// The offset into the archive file at which it is located.
	// Zero if not in archive.
	size_t ArchiveOffset;

	// The compressed size.
	// Zero if not compressed, or not in archive.
	size_t CompressedSize;

	// The uncompressed size of the file, in bytes.
	size_t Size;
};

struct MZDirDesc
{
	StringView Path;

	const MZDirDesc* Parent;

	const MZDirDesc* Subdirs;
	size_t NumSubdirs;

	const MZFileDesc* Files;
	size_t NumFiles;

	auto SubdirsRange() const {
		return ::Range<const MZDirDesc*>{ Subdirs, Subdirs + NumSubdirs };
	}

	auto FilesRange() const {
		return ::Range<const MZFileDesc*>{ Files, Files + NumFiles };
	}
};

struct MZNode
{
public:
	MZNode(const void* Desc, bool IsDirectory)
		: PackedField{ reinterpret_cast<uintptr_t>(Desc) | static_cast<uintptr_t>(IsDirectory) }
	{}

	bool IsDirectory() const {
		return (PackedField & 0x1) != 0;
	}

	bool IsFile() const {
		return !IsDirectory();
	}

	const MZDirDesc& AsDirectory() const {
		auto ptr = PackedField & ~0x1;
		return *reinterpret_cast<const MZDirDesc*>(ptr);
	}

	const MZFileDesc& AsFile() const {
		return reinterpret_cast<const MZFileDesc&>(AsDirectory());
	}

private:
	uintptr_t PackedField;
};

struct PreprocessedDir;
struct PreprocessedFileTree;

class MZFileSystem
{
public:
	MZFileSystem();
	~MZFileSystem();

	bool Create(std::string BasePathArgument);

	int GetFileCount() const;
	const StringView& GetFileName(int i) const;
	const MZFileDesc* GetFileDesc(int i) const;
	const MZFileDesc* GetFileDesc(const StringView& Path) const;
	const MZDirDesc* GetDirectory(const StringView& Path) const;
	const MZNode* GetNode(const StringView& Path) const;

	auto* GetBasePath() const { return BasePath.c_str(); }

	int GetFileLength(const StringView& szFileName);
	int GetFileLength(int i);

	void CacheArchive(const StringView& Filename);
	void ReleaseCachedArchives();

protected:
	friend class MZFile;

	void MakeDirectoryTree(PreprocessedFileTree& Tree, const StringView& FullPath, PreprocessedDir& Dir);
	void AddFilesInArchive(PreprocessedFileTree& Tree, PreprocessedDir& ArchiveDir, MZip& Zip);

	void UpdateFileList(PreprocessedDir& SrcNode, MZDirDesc& DestNode);
	void ClearFileList();
	StringView AllocateString(const StringView& Src);

	// The path the file system was initialized in.
	std::string BasePath;
	std::vector<MZFileDesc> Files;
	std::vector<MZDirDesc> Dirs;

	// Maps paths (relative to BasePath) to indices into FileNodeList.
	// The paths are case insensitive and do not differentiate between forward slashes and backslashes.
	std::unordered_map<StringView, MZNode, PathHasher, PathComparer> NodeMap;

	std::vector<std::unique_ptr<char[]>> Strings;

	std::unordered_map<const MZFileDesc*, std::unique_ptr<char[]>> CachedFileMap;
};

#include "MZFile.h"

struct RecursiveMZFileIterator
{
public:
	RecursiveMZFileIterator(const MZDirDesc& Dir,
		const MZDirDesc* CurrentSubdir,
		int FileIndex)
		: Dir{ Dir }, CurrentSubdir{ CurrentSubdir }, FileIndex{ FileIndex }
	{}

	bool operator==(const RecursiveMZFileIterator& rhs) const {
		return CurrentSubdir == rhs.CurrentSubdir && FileIndex == rhs.FileIndex;
	}

	bool operator!=(const RecursiveMZFileIterator& rhs) const {
		return !(*this == rhs);
	}

	const MZFileDesc& operator*() const { return CurrentSubdir->Files[FileIndex]; }

	RecursiveMZFileIterator& operator++() {
		AdvanceToNextFile();
		return *this;
	}

	RecursiveMZFileIterator operator++(int) {
		auto temp = *this;
		++*this;
		return temp;
	}

private:
	void AdvanceToNextFile();

	const MZDirDesc& Dir;
	const MZDirDesc* CurrentSubdir;
	int FileIndex;
};

Range<RecursiveMZFileIterator> FilesInDirRecursive(const MZDirDesc& Dir);

void GetRefineFilename(char *szRefine, int maxlen, const char *szSource);
template<size_t size> void GetRefineFilename(char(&szRefine)[size], const char *szSource) {
	GetRefineFilename(szRefine, size, szSource);
}
unsigned MGetCRC32(const char *data, int nLength);
