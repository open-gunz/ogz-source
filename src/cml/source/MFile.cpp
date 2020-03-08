#include "stdafx.h"
#include <climits>
#include "MFile.h"
#include "defer.h"
#include <string>

static bool IsDots(StringView Name) { return equals(Name, ".") || equals(Name, ".."); }

#ifdef WIN32
#include "MWindows.h"
#include <direct.h>
#include <io.h>
#undef DeleteFile
#else
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include <fcntl.h>
#endif

namespace MFile
{

optional<u64> Size(const char* Path)
{
	auto Attr = MFile::GetAttributes(Path);
	if (!Attr)
		return nullopt;
	return Attr->Size;
}

#ifdef _WIN32

bool Exists(const char* Path)
{
	constexpr auto F_OK = 0;
	return !_access(Path, F_OK);
}

bool Delete(const char* Path)
{
	auto Ret = IsDir(Path) ? ::RemoveDirectoryA(Path) : ::DeleteFileA(Path);
	return Ret != FALSE;
}

bool Move(const char* OldPath, const char* NewPath)
{
	return ::MoveFileA(OldPath, NewPath) != FALSE;
}

static bool EntityExists(const char* Path, u32 SubdirValue)
{
	if (!Exists(Path))
		return false;

	auto MaybeAttributes = GetAttributes(Path);
	if (!MaybeAttributes.has_value())
		return false;

	return (MaybeAttributes.value().Attributes & Attributes::Subdir) == SubdirValue;
}

bool IsFile(const char* Path)
{
	return EntityExists(Path, 0);
}

bool IsDir(const char* Path)
{
	return EntityExists(Path, Attributes::Subdir);
}

bool CreateFile(const char* Path)
{
	auto FileHandle = ::CreateFileA(Path, 0, 0, nullptr, CREATE_NEW, 0, nullptr);
	if (FileHandle == nullptr)
		return false;

	CloseHandle(FileHandle);
	return true;
}

bool CreateDir(std::string path)
{
	return ::CreateDirectoryA(path.c_str(), nullptr);
}

static constexpr u64 GetU64(u32 High, u32 Low) {
	return (static_cast<uint64_t>(High) << 32) | Low;
}

static constexpr u64 GetU64(FILETIME filetime) {
	return GetU64(filetime.dwHighDateTime, filetime.dwLowDateTime);
}

optional<FileAttributes> GetAttributes(const char* Path)
{
	WIN32_FILE_ATTRIBUTE_DATA wfad;
	if (!GetFileAttributesExA(Path, GetFileExInfoStandard, &wfad))
		return nullopt;

	FileAttributes ret{};

	ret.Attributes = wfad.dwFileAttributes;
	ret.CreationTime = GetU64(wfad.ftCreationTime);
	ret.LastAccessTime = GetU64(wfad.ftLastAccessTime);
	ret.LastModifiedTime = GetU64(wfad.ftLastWriteTime);
	ret.Size = GetU64(wfad.nFileSizeHigh, wfad.nFileSizeLow);

	return ret;
}

bool GetCWD(ArrayView<char> Output)
{
	return _getcwd(Output.data(), Output.size());
}

static void CopyFileData(FileData& dest, const _finddata_t& src)
{
	dest.Attributes = src.attrib;
	dest.CreationTime = src.time_create;
	dest.LastAccessTime = src.time_access;
	dest.LastModifiedTime = src.time_write;
	dest.Size = src.size;
	strcpy_safe(dest.Name, src.name);
}

static bool GetNextFileData(FileRange& Range)
{
	_finddata_t fd;
	int find_ret;
	do {
		find_ret = _findnext(reinterpret_cast<intptr_t>(Range.Handle.get()), &fd);
	} while (find_ret != -1 && IsDots(fd.name));

	if (find_ret == -1)
	{
		auto err = errno;
		Range.ErrorCode = err == ENOENT ? 0 : err;
		Range.Handle = nullptr;
		return false;
	}

	CopyFileData(Range.Data, fd);
	return true;
}


FileIterator& FileIterator::operator++()
{
	if (!GetNextFileData(Range))
	{
		End = true;
	}
	return *this;
}

void FileRange::Deleter::operator()(void* ptr) const
{
	if (!ptr)
		return;
	auto handle = reinterpret_cast<intptr_t>(ptr);
	auto ret = _findclose(handle);
	assert(ret == 0);
	(void)ret;
}

FileRange Glob(const char* Pattern)
{
	FileRange ret;

	_finddata_t fd;
	auto SearchHandle = _findfirst(Pattern, &fd);

	if (SearchHandle != -1)
	{
		ret.Handle = FileRange::HandleType(reinterpret_cast<void*>(SearchHandle));
		if (IsDots(fd.name))
			GetNextFileData(ret);
		else
			CopyFileData(ret.Data, fd);
	}
	else
	{
		ret.Handle = nullptr;
		ret.ErrorCode = errno;
	}

	return ret;
}

#else

bool Exists(const char* Path)
{
	return !access(Path, F_OK);
}

bool Delete(const char* Path)
{
	return remove(Path) == 0;
}

bool Move(const char* OldPath, const char* NewPath)
{
	return rename(OldPath, NewPath) == 0;
}

static bool EntityExists(const char* Path, u32 SubdirValue)
{
	if (!Exists(Path))
		return false;

	auto MaybeAttributes = GetAttributes(Path);
	if (!MaybeAttributes.has_value())
		return false;

	return (MaybeAttributes.value().Attributes & Attributes::Subdir) == SubdirValue;
}

bool IsFile(const char* Path)
{
	return EntityExists(Path, 0);
}

bool IsDir(const char* Path)
{
	return EntityExists(Path, Attributes::Subdir);
}

bool CreateFile(const char* Path)
{
	return creat(Path, 0777) != -1;
}

bool CreateDir(std::string path)
{
	return mkdir(path.c_str(), 0755) != -1;
}

optional<FileAttributes> GetAttributes(const char* Path)
{
	struct stat stat_ret;
	if (stat(Path, &stat_ret) != 0)
		return nullopt;

	FileAttributes ret{};

	ret.Attributes = stat_ret.st_mode & S_IFDIR ? Attributes::Subdir : 0;
	ret.CreationTime = UnknownCreationTime;
	ret.LastAccessTime = stat_ret.st_atime;
	ret.LastModifiedTime = stat_ret.st_mtime;
	ret.Size = stat_ret.st_size;

	return ret;
}

bool GetCWD(ArrayView<char> Output)
{
	return getcwd(Output.data(), Output.size());
}

static bool GetNextFileData(FileRange& Range)
{
	do {
		++Range.CurFileIndex;
	} while (Range.CurFileIndex < Range.GlobData.gl_pathc && IsDots(Range.GlobData.gl_pathv[Range.CurFileIndex]));

	if (Range.CurFileIndex >= Range.GlobData.gl_pathc)
	{
		Range.GlobData.gl_pathc = 0;
		return false;
	}

 	auto Path = Range.GlobData.gl_pathv[Range.CurFileIndex];
 	auto MaybeAttributes = GetAttributes(Path);
	if (!MaybeAttributes)
	{
		Range.Handle = nullptr;
		Range.ErrorCode = errno;
		return false;
	}
	auto& Attributes = *MaybeAttributes;
	FileData& dest = Range.Data;
	dest.Attributes = Attributes.Attributes;
	dest.CreationTime = Attributes.CreationTime;
	dest.LastAccessTime = Attributes.LastAccessTime;
	dest.LastModifiedTime = Attributes.LastModifiedTime;
	dest.Size = Attributes.Size;
	std::string PathCopy = Path;
	strcpy_safe(dest.Name, basename(const_cast<char *>(PathCopy.c_str())));

	return true;
}

FileIterator& FileIterator::operator++()
{
	if (!GetNextFileData(Range))
	{
		End = true;
	}

	return *this;
}

void FileRange::Deleter::operator()(void* ptr) const
{
	if (ptr)
		globfree(static_cast<glob_t*>(ptr));
}

FileRange Glob(const char* Pattern)
{
	FileRange Range;
	Range.CurFileIndex = size_t(-1);
	auto ret = glob(Pattern, 0, nullptr, &Range.GlobData);
	Range.Handle = FileRange::HandleType(&Range.GlobData);
	if (ret == 0)
	{
		GetNextFileData(Range);
	}
	else if (ret != GLOB_NOMATCH)
	{
		Range.ErrorCode = errno;
	}
	return Range;
}

#endif

bool CreateParentDirs(StringView PathArgument)
{
	auto Sep = "/\\"_sv;
	auto FirstIndex = PathArgument.find_first_of(Sep);
	if (FirstIndex == PathArgument.npos)
		return true; // No directories.

	// Create our own copy so we can add zeroes.
	char Path[MFile::MaxPath];
	strcpy_safe(Path, PathArgument);
	auto End = Path + PathArgument.size();

	char* p = Path + FirstIndex;
	while (true)
	{
		// Set the current slash to zero, and revert this change before the next loop iteration.
		auto OldValue = *p;
		*p = 0;
		// Capture by value to ignore the change to p at the end.
		DEFER([=] { *p = OldValue; });

		if (!MFile::IsDir(Path))
		{
			if (!MFile::CreateDir(Path))
			{
				return false;
			}
		}

		p = std::find_first_of(p + 1, End, Sep.begin(), Sep.end());
		if (p == End)
			break;
	}

	return true;
}

bool File::open_impl(const char* path, const char* mode)
{
	if (MFile::IsDir(path))
	{
		state.error = true;
		return false;
	}

#pragma warning(push)
#pragma warning(disable: 4996)
	file_ptr = CFilePtr{ fopen(path, mode) };
#pragma warning(pop)

	state.error = file_ptr == nullptr;
	return !state.error;
}

bool File::open(const char* path)
{
	return open_impl(path, "rb");
}

bool File::open(const char* path, TextType)
{
	return open_impl(path, "r");
}

void File::close()
{
	if (!file_ptr)
		return;

	// Close the FILE*.
	auto fclose_ret = fclose(fp());

	// Since we've already closed the file, we need to set the pointer to null
	// without triggering the fclose on reset, so we release it without doing
	// anything with the return value.
	(void)file_ptr.release();

	state.error = fclose_ret != 0;
}

bool File::is_open() const
{
	return file_ptr != nullptr;
}

u64 File::size()
{
#define E do { if (error()) return 0; } while (false)

	const auto prev_pos = tell();
	E;

	seek(0, Seek::End);
	E;

	const auto ret = tell();
	E;

	seek(prev_pos, Seek::Begin);

	return ret;

#undef E
}

bool File::seek(i64 offset, Seek origin)
{
	assert(fp());

#ifdef _MSC_VER
	auto fseek_ret = _fseeki64(fp(), offset, static_cast<int>(origin));
#else
	static_assert(sizeof(off_t) * CHAR_BIT == 64, "Wrong off_t");
	auto fseek_ret = fseeko(fp(), offset, static_cast<int>(origin));
#endif

	state.error = fseek_ret != 0;
	if (state.error)
		clearerr(fp());

	return !state.error;
}

i64 File::tell()
{
	assert(fp());

#ifdef _MSC_VER
	auto ftell_ret = _ftelli64(fp());
#else
	auto ftell_ret = ftello(fp());
#endif

	state.error = ftell_ret == tell_error;

	return ftell_ret;
}

size_t File::read(void* buffer, size_t size)
{
	assert(fp());

	auto fread_ret = fread(buffer, 1, size, fp());

	const auto error_or_eof = fread_ret != size;
	if (error_or_eof)
	{
		state.error = ferror(fp()) != 0;
		state.eof = feof(fp()) != 0;
		if (state.error)
			clearerr(fp());
	}
	else
	{
		state.error = false;
		state.eof = false;
	}

	return fread_ret;
}

bool File::flush()
{
	auto fflush_ret = fflush(fp());
	state.error = fflush_ret == EOF;
	if (state.error)
		clearerr(fp());
	return !state.error;
}

bool File::eof() const
{
	assert(fp());

	return state.eof;
}

bool File::error() const
{
	return state.error;
}

FILE* File::fp() const
{
	return file_ptr.get();
}

CFilePtr File::release()
{
	return std::move(file_ptr);
}

bool RWFile::open(const char* path, ExistingFileAction efa)
{
	if (efa.Value == Nonexistent.Value && Exists(path))
	{
		state.error = true;
		return false;
	}

	char mode_string[8];
	auto base_string = [&] {
		switch (efa.Value)
		{
		case Nonexistent.Value:
		case Clear.Value:
			return "w+";
		case Append.Value:
			return "a+";
		case Prepend.Value:
			// "r+" will fail if the file doesn't exist.
			return Exists(path) ? "r+" : "w+";
		}
		assert(false);
		return "r+";
	}();
	strcpy_safe(mode_string, base_string);
	if (!efa.Text)
		strcat_safe(mode_string, "b");

	return open_impl(path, mode_string);
}

size_t RWFile::write(const void* buffer, size_t size)
{
	assert(fp());

	auto fwrite_ret = fwrite(buffer, 1, size, fp());

	state.error = fwrite_ret != size;
	if (state.error)
		clearerr(fp());

	return fwrite_ret;
}

}
