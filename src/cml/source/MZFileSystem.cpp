#include "stdafx.h"
#include "MZFileSystem.h"
#include "MXml.h"
#include "MZip.h"
#include "FileInfo.h"
#include "zip/zlib.h"
#include "MDebug.h"
#include "MUtil.h"
#include <algorithm>
#include <cassert>
#include "Arena.h"

static void ReplaceBackSlashToSlash(char* szPath)
{
	int nLen = strlen(szPath);
	for(int i=0; i<nLen; i++){
		if(szPath[i]=='\\') szPath[i]='/';
	}
}

void GetRefineFilename(char *szRefine, int maxlen, const char *szSource)
{
	char pBasePath[256];
#ifdef _WIN32
	GetCurrentDirectory(sizeof(pBasePath),pBasePath);
#else
	getcwd(pBasePath, std::size(pBasePath));
#endif
	strcat_safe(pBasePath,"\\");

	GetRelativePath(szRefine, maxlen, pBasePath,szSource);

	ReplaceBackSlashToSlash(szRefine);
}

// Asserts that the capacity of vec remains unchanged.
template <typename T>
static auto CapacityChecker(T& vec)
{
#ifdef _DEBUG
	struct CapacityChecker
	{
		CapacityChecker(T& vec) : vec{ vec } { Capacity = vec.capacity(); };
		~CapacityChecker() { assert(Capacity == vec.capacity()); }
		T& vec;
		size_t Capacity;
	};
	return CapacityChecker{ vec };
#else
	return 0;
#endif
}

void MZFileSystem::ClearFileList()
{
	Files.clear();
	Dirs.clear();
	NodeMap.clear();
}

StringView MZFileSystem::AllocateString(const StringView& Src)
{
	const auto size = Src.size() + 1;
	Strings.emplace_back(std::make_unique<char[]>(size));

	auto ptr = Strings.back().get();
	strcpy_safe(ptr, size, Src);
	
	return{ ptr, Src.size() };
}

static bool IsArchivePath(const StringView& Path)
{
	const auto ExtIndex = Path.find_last_of('.');
	if (ExtIndex == Path.npos)
		return false;

	const auto ExtLen = Path.size() - ExtIndex;
	if (ExtLen < 4)
		return false;

	const auto Ext = Path.data() + ExtIndex;
	return Ext && (_stricmp(Ext, "." DEF_EXT) == 0 || _stricmp(Ext, ".zip") == 0);
}

template <typename T>
using AllocType = ArenaAllocator<T>;

struct PreprocessedDir
{
	explicit PreprocessedDir(const AllocType<char>& Alloc) : Subdirs{ Alloc }, Files{ Alloc } {}

	// Path to the directory.
	// If the directory is an archive, this is formatted as a directory (i.e. no archive extension).
	StringView Path;

	// If this directory is an archive, this is the path to the archive file.
	// Otherwise, it's empty.
	StringView ArchivePath;

	std::vector<PreprocessedDir, AllocType<PreprocessedDir>> Subdirs;
	std::vector<MZFileDesc, AllocType<MZFileDesc>> Files;
};

struct PreprocessedFileTree
{
	explicit PreprocessedFileTree(const AllocType<char>& Alloc) : Alloc{ Alloc }, Root { Alloc } {}

	AllocType<char> Alloc;
	int NumFiles{};
	int NumDirectories{};
	int NumArchives{};
	PreprocessedDir Root;
};

void MZFileSystem::AddFilesInArchive(PreprocessedFileTree& Tree, PreprocessedDir& ArchiveDir, MZip& Zip)
{
	const auto Count = int(Zip.GetFileCount());

	Tree.NumFiles += Count;

	for (int i = 0; i < Count; i++)
	{
		const auto Filename = Zip.GetFileName(i);

		{
			const auto lastchar = Filename.back();
			if (lastchar == '\\' || lastchar == '/') {
				continue;
			}
		}

		ArchiveDir.Files.emplace_back();
		auto&& Child = ArchiveDir.Files.back();

		char Path[MFile::MaxPath];
		sprintf_safe(Path, "%.*s%.*s",
			ArchiveDir.Path.size(), ArchiveDir.Path.data(),
			Filename.size(), Filename.data());
		Child.Path = AllocateString(Path);
		Child.ArchivePath = ArchiveDir.ArchivePath;

		Child.Size = Zip.GetFileLength(i);
		Child.ArchiveOffset = Zip.GetFileArchiveOffset(i);
		Child.CompressedSize = Zip.IsFileCompressed(i) ? Zip.GetFileCompressedSize(i) : 0;
	}
}

void MZFileSystem::MakeDirectoryTree(PreprocessedFileTree& Tree,
	const StringView& FullPath, PreprocessedDir& Dir)
{
	char szFilter[MFile::MaxPath];
	sprintf_safe(szFilter, "%.*s*",
		FullPath.size(), FullPath.data());

	for (auto&& FileData : MFile::Glob(szFilter))
	{
		if (strcmp(FileData.Name, ".") == 0 || strcmp(FileData.Name, "..") == 0)
			continue;

		auto AddSubdir = [&](const StringView& Filename) -> auto&
		{
			char Path[MFile::MaxPath];
			sprintf_safe(Path, "%.*s%.*s/",
				Dir.Path.size(), Dir.Path.data(),
				Filename.size(), Filename.data());

			Dir.Subdirs.emplace_back(Tree.Alloc);
			auto&& Subdir = Dir.Subdirs.back();

			Subdir.Path = AllocateString(Path);

			return Subdir;
		};

		const auto IsDirectory = (FileData.Attributes & MFile::Attributes::Subdir) != 0;

		if (IsDirectory)
		{
			auto&& Subdir = AddSubdir(FileData.Name);

			char SubdirFullPath[MFile::MaxPath];
			sprintf_safe(SubdirFullPath, "%.*s%s/",
				FullPath.size(), FullPath.data(),
				FileData.Name);

			MakeDirectoryTree(Tree, SubdirFullPath, Subdir);

			++Tree.NumDirectories;
		}
		else if (IsArchivePath(FileData.Name))
		{
			StringView Filename{ FileData.Name };
			Filename = Filename.substr(0, Filename.find_last_of('.'));
			auto&& Subdir = AddSubdir(Filename);

			char ArchivePath[MFile::MaxPath];
			sprintf_safe(ArchivePath, "%.*s%s",
				Dir.Path.size(), Dir.Path.data(),
				FileData.Name);

			Subdir.ArchivePath = AllocateString(ArchivePath);

			char FullArchivePath[MFile::MaxPath];
			sprintf_safe(FullArchivePath, "%s%.*s",
				BasePath.c_str(),
				Subdir.ArchivePath.size(), Subdir.ArchivePath.data());

			auto fp = fopen(FullArchivePath, "rb");
			if (!fp)
			{
				MLog("fopen on %s failed\n", FullArchivePath);
				assert(false);
				continue;
			}

			MZip Zip;
			Zip.Initialize(fp, MZFile::GetReadMode());

			AddFilesInArchive(Tree, Subdir, Zip);

			++Tree.NumArchives;
		}
		else
		{
			Dir.Files.emplace_back();
			auto&& File = Dir.Files.back();

			char Path[MFile::MaxPath];
			sprintf_safe(Path, "%.*s%s",
				Dir.Path.size(), Dir.Path.data(),
				FileData.Name);

			File.Path = AllocateString(Path);
			File.ArchiveOffset = 0;
			File.CompressedSize = 0;
			assert(FileData.Size <= SIZE_MAX);
			File.Size = static_cast<size_t>(FileData.Size);

			++Tree.NumFiles;
		}
	}
}

void MZFileSystem::UpdateFileList(PreprocessedDir& Src, MZDirDesc& Dest)
{
	if (Src.Subdirs.empty() && Src.Files.empty())
		return;

	auto&& chk1 = CapacityChecker(Files);
	auto&& chk2 = CapacityChecker(Dirs);

	Dest.Path = Src.Path;

	auto AddNode = [&](auto& x, bool IsDirectory) {
		NodeMap.emplace(x.Path, MZNode{ &x, IsDirectory });
	};

	if (Src.Files.empty())
	{
		Dest.Files = nullptr;
		Dest.NumFiles = 0;
	}
	else
	{
		size_t FilesIndex = Files.size();
		Files.insert(std::end(Files), std::begin(Src.Files), std::end(Src.Files));
		Dest.Files = &Files[FilesIndex];
		Dest.NumFiles = Src.Files.size();

		for (auto&& File : Dest.FilesRange())
			AddNode(File, false);
	}

	if (Src.Subdirs.empty())
	{
		Dest.Subdirs = nullptr;
		Dest.NumSubdirs = 0;
	}
	else
	{
		size_t SubdirsIndex = Dirs.size();
		Dest.NumSubdirs = Src.Subdirs.size();
		Dirs.resize(Dirs.size() + Dest.NumSubdirs);
		Dest.Subdirs = &Dirs[SubdirsIndex];

		for (size_t i = 0; i < Dest.NumSubdirs; ++i)
		{
			auto&& SrcSubdir = Src.Subdirs[i];
			auto&& DestSubdir = Dirs[SubdirsIndex + i];
			DestSubdir.Parent = &Dest;
			UpdateFileList(SrcSubdir, DestSubdir);
		}

		for (auto&& Subdir : Dest.SubdirsRange())
			AddNode(Subdir, true);
	}
}

unsigned MGetCRC32(const char *data, int nLength)
{
	uLong crc = crc32(0, Z_NULL, 0);
	crc = crc32(crc, reinterpret_cast<const Byte*>(data), nLength);
	return crc;
}

MZFileSystem::MZFileSystem() = default;
MZFileSystem::~MZFileSystem() = default;

bool MZFileSystem::Create(std::string BasePathArgument)
{
	BasePath = std::move(BasePathArgument);

	// Add slash to end of path if missing.
	if (BasePath.back() != '/' && BasePath.back() != '\\')
		BasePath += '/';

	u8 ArenaBuffer[1024 * 16];
	Arena arena{ ArenaBuffer, std::size(ArenaBuffer) };

	PreprocessedFileTree Tree{ ArenaAllocator<char>{ &arena } };
	Tree.NumDirectories++; // Root.
	MakeDirectoryTree(Tree, BasePath, Tree.Root);

	DMLog("Number of files: %d\n"
		"Number of archives: %d\n"
		"Number of directories: %d\n",
		Tree.NumFiles, Tree.NumArchives, Tree.NumDirectories);

	const auto NumNodes = Tree.NumArchives + Tree.NumDirectories + Tree.NumFiles;
	const auto NumDirs = Tree.NumArchives + Tree.NumDirectories;
	NodeMap.reserve(NumNodes);
	Files.reserve(Tree.NumFiles);
	Dirs.reserve(NumDirs);

	Strings.shrink_to_fit();

	Dirs.emplace_back();
	auto&& Root = Dirs.back();
	UpdateFileList(Tree.Root, Root);

	return true;
}

int MZFileSystem::GetFileCount() const
{
	return static_cast<int>(Files.size());
}

const StringView& MZFileSystem::GetFileName(int i) const
{
	assert(i >= 0 && i < int(GetFileCount()));
	return GetFileDesc(i)->Path;
}

const MZFileDesc* MZFileSystem::GetFileDesc(int i) const
{
	assert(i >= 0 && i < int(GetFileCount()));
	return &Files[i];
}

const MZFileDesc* MZFileSystem::GetFileDesc(const StringView& Path) const
{
	auto Node = GetNode(Path);
	if (!Node || !Node->IsFile())
		return nullptr;

	return &Node->AsFile();
}

const MZDirDesc* MZFileSystem::GetDirectory(const StringView& Path) const
{
	auto Node = GetNode(Path);
	if (!Node || !Node->IsDirectory())
		return nullptr;

	return &Node->AsDirectory();
}

const MZNode* MZFileSystem::GetNode(const StringView& Path) const
{
	auto it = NodeMap.find(Path);
	if (it == NodeMap.end())
		return nullptr;

	return &it->second;
}

int MZFileSystem::GetFileLength(int i)
{
	auto pDesc = GetFileDesc(i);
	if (!pDesc)
		return 0;

	return pDesc->Size;
}

int MZFileSystem::GetFileLength(const StringView& szFileName)
{
	auto pDesc = GetFileDesc(szFileName);
	if (!pDesc)
		return 0;

	return pDesc->Size;
}

#ifdef WIN32
class MMappedFile
{
public:
	MMappedFile() {}
	MMappedFile(const char* Filename);
	MMappedFile(const MMappedFile&) = delete;
	MMappedFile(MMappedFile&&);
	~MMappedFile();
	MMappedFile& operator =(MMappedFile&& src)
	{
		this->~MMappedFile();
		new (this) MMappedFile(std::move(src));
		return *this;
	}

	bool Dead() const { return bDead; }
	auto GetPointer() const { return reinterpret_cast<const char*>(View); }
	auto GetSize() const { return Size; }

private:
	bool bDead = true;
	HANDLE File = INVALID_HANDLE_VALUE;
	HANDLE Mapping = INVALID_HANDLE_VALUE;
	HANDLE View = INVALID_HANDLE_VALUE;
	size_t Size = 0;
};
#endif

#ifdef WIN32
#define ARCHIVE_CACHE_MMAP
#endif

void MZFileSystem::CacheArchive(const StringView& Filename)
{
	char FilenameWithExtension[MFile::MaxPath];
	sprintf_safe(FilenameWithExtension, "%.*s.%s", Filename.size(), Filename.data(), DEF_EXT);

	MZip Zip;

#ifdef ARCHIVE_CACHE_MMAP
	MMappedFile File{ FilenameWithExtension };
	if (File.Dead())
	{
		MLog("MZFileSystem::CacheArchive -- Failed to load file %s!\n", FilenameWithExtension);
		return;
	}

	const auto ZipInitialized = Zip.Initialize(File.GetPointer(), File.GetSize(), MZFile::GetReadMode());
#else
	auto fp = fopen(FilenameWithExtension, "rb");
	if (!fp)
	{
		MLog("MZFileSystem::CacheArchive -- fopen_s failed on %s\n", Filename);
		return;
	}

	const auto ZipInitialized = Zip.Initialize(fp, MZFile::GetReadMode());
#endif

	if (!ZipInitialized)
	{
		MLog("MZFileSystem::CacheArchive -- MZip::Initialize on %s failed!\n", Filename);
		return;
	}

	auto FileCount = Zip.GetFileCount();
	for (int i = 0; i < FileCount; i++)
	{
		char Name[64];
		Zip.GetFileName(i, Name);
		char AbsName[64];
		sprintf_safe(AbsName, "%.*s/%s", Filename.size(), Filename.data(), Name);
		auto Desc = GetFileDesc(AbsName);
		if (!Desc)
		{
			DMLog("Couldn't find file desc for %s\n", AbsName);
			continue;
		}
		if (Desc->ArchivePath == FilenameWithExtension)
			continue;
		auto Size = Zip.GetFileLength(i);

		auto p = std::make_unique<char[]>(Size);
		if (!Zip.ReadFile(i, p.get(), Size)) {
			MLog("MZFileSystem::CacheArchive - MZip::ReadFile on %s failed!\n", Name);
			continue;
		}

		CachedFileMap.emplace(Desc, std::move(p));
	}

	DMLog("Cached %d files for archive %s\n", FileCount, Filename);
}

void MZFileSystem::ReleaseCachedArchives()
{
	CachedFileMap.clear();
}

#ifdef WIN32
MMappedFile::MMappedFile(const char * Filename)
{
	File = CreateFileA(Filename, GENERIC_READ, 0, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
	if (File == INVALID_HANDLE_VALUE)
	{
		MLog("MMappedFile::MMappedFile - CreateFile failed on %s! GetLastError() = %d\n",
			Filename, GetLastError());
		return;
	}

	Size = GetFileSize(File, nullptr);

	char FileMappingName[256];
	sprintf_safe(FileMappingName, "GunzCache_%s", Filename);
	Mapping = CreateFileMapping(File, 0, PAGE_READONLY, 0, 0, FileMappingName);
	if (Mapping == NULL)
	{
		assert(false);
		return;
	}

	View = MapViewOfFile(Mapping, FILE_MAP_READ, 0, 0, 0);
	if (View == NULL)
	{
		assert(false);
		return;
	}

	bDead = false;
}

MMappedFile::~MMappedFile()
{
	if (View != INVALID_HANDLE_VALUE) {
		auto Success = UnmapViewOfFile(View);
		assert(Success);
	}
	if (Mapping != INVALID_HANDLE_VALUE) {
		auto Success = CloseHandle(Mapping);
		assert(Success);
	}
	if (File != INVALID_HANDLE_VALUE) {
		auto Success = CloseHandle(File);
		assert(Success);
	}
}

MMappedFile::MMappedFile(MMappedFile && src)
	: bDead(src.bDead), View(src.View), Mapping(src.Mapping), File(src.File)
{
	src.bDead = true;
	src.View = INVALID_HANDLE_VALUE;
	src.Mapping = INVALID_HANDLE_VALUE;
	src.File = INVALID_HANDLE_VALUE;
}
#endif

static const MZDirDesc* Down(const MZDirDesc* Dir);

static const MZDirDesc* DownThroughRange(Range<const MZDirDesc*> range)
{
	for (auto& Subdir : range)
		if (auto NextDir = Down(&Subdir))
			return NextDir;

	return nullptr;
}

static const MZDirDesc* DownThroughSubdirs(const MZDirDesc* Dir)
{
	return DownThroughRange(Dir->SubdirsRange());
}

static const MZDirDesc* Down(const MZDirDesc* Dir)
{
	if (Dir->NumFiles)
		return Dir;

	return DownThroughSubdirs(Dir);
}

static const MZDirDesc* AcrossOrUp(const MZDirDesc* Dir, const MZDirDesc* Root)
{
	while (true)
	{
		if (Dir == Root)
			return Dir;
		auto End = Dir->Parent->Subdirs + Dir->Parent->NumSubdirs;
		if (auto NextDir = DownThroughRange(MakeRange(Dir + 1, End)))
			return NextDir;
		Dir = Dir->Parent;
	}
}

void RecursiveMZFileIterator::AdvanceToNextFile()
{
	if (FileIndex < int(CurrentSubdir->NumFiles) - 1)
	{
		++FileIndex;
		return;
	}

	if (auto NextDir = DownThroughSubdirs(CurrentSubdir))
	{
		CurrentSubdir = NextDir;
		FileIndex = 0;
		return;
	}

	CurrentSubdir = AcrossOrUp(CurrentSubdir, &Dir);
	if (CurrentSubdir == &Dir)
	{
		FileIndex = Dir.NumFiles;
	}
	else
	{
		FileIndex = 0;
	}
}

Range<RecursiveMZFileIterator> FilesInDirRecursive(const MZDirDesc& Dir)
{
	auto CurrentSubdir = Down(&Dir);

	return{
		RecursiveMZFileIterator{ Dir, CurrentSubdir, 0 },
		RecursiveMZFileIterator{ Dir, &Dir, int(Dir.NumFiles) },
	};
}