#include "MFile.h"
#include "MTime.h"
#include "defer.h"

#include "XML.h"
#include "Hash.h"
#include "Log.h"
#include "Sync.h"

#include "rapidxml.hpp"
#include "rapidxml_print.hpp"

#include "sodium.h"

static bool ExcludeFile(const StringView& Path)
{
	return iequals(Path, "PatchCreator.exe") ||
		iequals(Path, "patch.xml");
}

static bool TryHashFile(ArrayView<char> Output, const StringView& Path)
{
	Hash::Strong Hash;
	auto HashSuccess = Hash.HashFile(Path.data());
	if (!HashSuccess)
	{
		Log(LogLevel::Error, "Failed to hash file %.*s\n",
			Path.size(), Path.data());
		return false;
	}

	Hash.ToString(Output);

	return true;
}

struct PathPair
{
	PathPair(const StringView& BaseDir)
	{
		strcpy_safe(String, BaseDir);
		StringEnd = BaseDir.size();
		BaseOffset = BaseDir.size();
	}

	size_t AppendFile(const StringView& Name)
	{
		return Append(Name);
	}

	size_t AppendDir(const StringView& Name)
	{
		const auto OldStringEnd = AppendFile(Name);
		Append("/");
		return OldStringEnd;
	}
	
	size_t AppendExt(const StringView& Extension)
	{
		return Append(Extension);
	}

	void Revert(size_t OldStringEnd)
	{
		String[OldStringEnd] = 0;
		StringEnd = OldStringEnd;
		OldStringEnd = 0;
	}

	// Relative to current working directory.
	const char* CWD() const { return String; }

	// Relative to the base directory.
	// The base directory is the directory the client is actually in (<cwd>/client by default).
	const char* Base() const { return String + BaseOffset; }

private:
	size_t Append(const StringView& Str)
	{
		const auto OldStringEnd = StringEnd;
		memcpy(String + StringEnd, Str.data(), Str.size());
		StringEnd += Str.size();
		String[StringEnd] = 0;
		return OldStringEnd;
	}

	size_t StringEnd{};
	size_t BaseOffset{};
	char String[16 * 1024];
};

static StringView AllocateString(rapidxml::xml_document<>& doc, const StringView& Src)
{
	auto alloc_ptr = doc.allocate_string(Src.data(), Src.size());
	return StringView{ alloc_ptr, Src.size() };
}

// Returns u64(-1) on error.
static u64 TryGetFileSize(const char* Path)
{
	auto MaybeAttributes = MFile::GetAttributes(Path);
	if (!MaybeAttributes.has_value())
	{
		Log(LogLevel::Error, "Couldn't get attributes of file %s\n", Path);
		return u64(-1);
	}

	return MaybeAttributes.value().Size;
}

static void AppendFile(rapidxml::xml_document<>& doc,
	rapidxml::xml_node<>& ParentNode,
	PathPair& Paths,
	const StringView& Filename)
{
	if (ExcludeFile(Filename))
		return;

	const auto End = Paths.AppendFile(Filename);
	DEFER([&] { Paths.Revert(End); });

	char HashString[Hash::Strong::MinimumStringSize];

	if (!TryHashFile(HashString, Paths.CWD()))
		return;

	u64 FileSize = TryGetFileSize(Paths.CWD());
	if (FileSize == u64(-1))
		return;

	// allocate_node can never return null.
	auto&& node = *doc.allocate_node(rapidxml::node_element, "file");
	AppendAttribute(doc, node, "name", AllocateString(doc, Paths.Base()));
	AppendAttribute(doc, node, "size", FileSize);
	AppendAttribute(doc, node, "hash", AllocateString(doc, HashString));
	ParentNode.append_node(&node);

	Log(LogLevel::Info, "Hashed file %s (size: %llu) -> %s\n",
		Paths.Base(), FileSize, HashString);

	// Make the .sync file.
	char SyncFilename[16 * 1024];
	sprintf_safe(SyncFilename, "sync/%s.sync", Paths.Base());

	const auto Success = Sync::MakeSyncFile(SyncFilename, Paths.CWD());
	if (!Success)
	{
		Log(LogLevel::Error, "Failed to create sync file %s -> %s\n",
			Paths.CWD(), SyncFilename);
	}
}

static bool AppendFileNodes(rapidxml::xml_document<>& doc,
	rapidxml::xml_node<>& ParentNode,
	PathPair& Paths)
{
	char SearchPattern[MFile::MaxPath];
	sprintf_safe(SearchPattern, "%s*", Paths.CWD());

	auto&& Range = MFile::Glob(SearchPattern);
	for (auto&& FileData : Range)
	{
		if (FileData.Attributes & MFile::Attributes::Subdir)
		{
			// Recurse into the subdirectory.
			const auto End = Paths.AppendDir(FileData.Name);
			AppendFileNodes(doc, ParentNode, Paths);
			Paths.Revert(End);
			continue;
		}

		AppendFile(doc, ParentNode, Paths, FileData.Name);
	}
	if (Range.error())
	{
		char ErrorMessage[256];
		Range.error_message(ErrorMessage);
		Log.Error("MFile::Glob(%s) failed. error code = %d, error message = %s.\n",
			SearchPattern, Range.error_code(), ErrorMessage);
		return false;
	}
	
	return true;
}

static bool SaveDocumentToFile(rapidxml::xml_document<>& doc, const char* Filename)
{
	MFile::RWFile File{ Filename, MFile::Clear};
	if (File.error())
	{
		Log(LogLevel::Fatal, "Failed to open %s for writing\n", Filename);
		return false;
	}

	rapidxml::print(MFile::FileOutputIterator{ File }, doc);
	if (File.error())
	{
		Log(LogLevel::Fatal, "Failed to write data to %s\n", Filename);
		return false;
	}

	Log(LogLevel::Info, "Saved document to %s\n", Filename);

	return true;
}

int main()
{
	Log.Init("", LogTo::Stdout);

	rapidxml::xml_document<> doc;

	auto&& FilesNode = AppendNode(doc, doc, "files");

	PathPair Paths{ "client/" };
	if (!AppendFileNodes(doc, FilesNode, Paths) ||
	    !SaveDocumentToFile(doc, "patch.xml"))
		return -1;
}
