// FileCache.h
//
// The file cache stores cached data about files that have previously been
// included in the patch set into an XML file named launcher_cache.xml.
//
// The data it stores about files are the last recorded ...
// 1) size
// 2) last modified time
// 3) hash
// ... of the file.
//
// This is used to locate quickly identify the hashes of unchanged files:
// If the last modified time and size of the current file match the
// data for it in the cache, we can assume the saved hash is correct.
//
// This is not designed to be secure against malicious people trying to
// intentionally get the game to use the incorrect files; it assumes
// good faith on part of the user.

#pragma once

#include "GlobalTypes.h"
#include "StringView.h"
#include "Hash.h"
#include "MHash.h"
#include "MFile.h"

#include "File.h"
#include "XML.h"

#include <unordered_map>

struct CachedFileData
{
	u64 LastModifiedTime;
	u64 Size;
	char Hash[Hash::Strong::MinimumStringSize];
};

struct FileQueryResult
{
	enum {
		Found,
		NotFound,
		Outdated,
		Error,
	} Result;
	const CachedFileData* FileData;
};

struct FileCacheType
{
	bool Load()
	{
		auto CacheFilename = "launcher_cache.xml";

		if (!MFile::Exists(CacheFilename))
		{
			Log.Info("Cache does not exist\n");
			return true;
		}

		XMLFile xml;
		if (!xml.CreateFromFile(CacheFilename))
		{
			Log.Error("Failed to read and parse %s!\n", CacheFilename);
			return false;
		}

		for (auto&& node : GetFileNodeRange(xml.Doc))
		{
			auto Filename = xml.GetName(node);
			if (Filename.empty())
				continue;

			auto Hash = xml.GetHash(node, Filename);
			auto Size = xml.GetSize(node, Filename);
			auto LastModifiedTime = xml.GetLastModifiedTime(node, Filename);

			if (Hash.empty() || !Size.has_value() || !LastModifiedTime.has_value())
				continue;

			CachedFileData NewFile;
			NewFile.Size = Size.value();
			NewFile.LastModifiedTime = LastModifiedTime.value();
			strcpy_safe(NewFile.Hash, Hash);
			Map.emplace(Filename, NewFile);

			LOG_DEBUG("Mapped %.*s to {Size: %llu, LastModifiedTime: %llu, Hash: %s}\n",
				Filename.size(), Filename.data(), NewFile.Size, NewFile.LastModifiedTime, NewFile.Hash);
		}

		// Save the file data.
		CacheXMLFileData.swap(xml.FileData);

		return true;
	}

	bool Save()
	{
		if (!Changed)
			return true;

		rapidxml::xml_document<> doc;

		for (auto&& pair : Map)
		{
			// allocate_node can never return null.
			auto&& node = *doc.allocate_node(rapidxml::node_element, "file");
			AppendAttribute(doc, node, "name", pair.first);
			AppendAttribute(doc, node, "size", pair.second.Size);
			AppendAttribute(doc, node, "last_modified_time", pair.second.LastModifiedTime);
			AppendAttribute(doc, node, "hash", pair.second.Hash);
			doc.append_node(&node);
		}

		auto&& Filename = "launcher_cache.xml";

		MFile::RWFile File{ Filename, MFile::Clear };

		if (File.error())
		{
			Log.Error("Failed to open file %s for writing cache\n", Filename);
			return false;
		}

		rapidxml::print(MFile::FileOutputIterator{ File }, doc);

		if (File.error())
		{
			Log.Error("Failed to write cache to %s\n", Filename);
			return false;
		}

		Log.Info("Saved file cache to %s\n", Filename);

		return true;
	}

	FileQueryResult GetCachedFileData(const char* Path) const
	{
		auto it = Map.find(Path);
		if (it == Map.end())
			return {FileQueryResult::NotFound};

		auto& CachedData = it->second;

		CachedFileData CurrentData;
		if (!GetFileSizeAndTime(CurrentData, Path))
			return {FileQueryResult::Error};

		bool Unchanged = CachedData.LastModifiedTime == CurrentData.LastModifiedTime &&
			CachedData.Size == CurrentData.Size;

		if (!Unchanged)
		{
			return {FileQueryResult::Outdated};
		}

		return {FileQueryResult::Found, &CachedData};
	}

	// Note: The Path pointer is saved in the map, so the caller is responsible for
	// assuring its lifetime until calling FileCache::Save.
	bool Add(const char* Path, const StringView& Hash)
	{
		CachedFileData Data;
		if (!GetFileSizeAndTime(Data, Path))
		{
			Log(LogLevel::Error, "Failed to get size or time for file %s for cache!\n", Path);
			return false;
		}

		strcpy_safe(Data.Hash, Hash);
		Map[Path] = Data;

		Changed = true;

		return true;
	}

private:
	static bool GetFileSizeAndTime(CachedFileData& Output, const char* Path)
	{
		auto MaybeResult = MFile::GetAttributes(Path);

		if (!MaybeResult.has_value())
		{
			if (MFile::Exists(Path))
			{
				Log(LogLevel::Error, "MFile::GetAttributes on %s failed!\n", Path);
			}
			return false;
		}

		auto&& Result = MaybeResult.value();

		Output.Size = Result.Size;
		Output.LastModifiedTime = Result.LastModifiedTime;

		return true;
	}

	bool ConfirmUnchangedFileSizeAndTime(const char* Path, const CachedFileData& CachedData) const
	{
		CachedFileData CurrentData;

		if (!GetFileSizeAndTime(CurrentData, Path))
			return false;

		LOG_DEBUG("Cur.Size = %llu, Cur.LastModifiedTime = %llu\n"
			"Last.Size = %llu, Last.LastModifiedTime = %llu\n",
			CurrentData.Size, CurrentData.LastModifiedTime,
			CachedData.Size, CachedData.LastModifiedTime);

		return CurrentData.Size == CachedData.Size &&
			CurrentData.LastModifiedTime == CachedData.LastModifiedTime;
	}

	// Maps file paths to CachedFileDatas.
	// The keys in this map are references to strings; they don't own the memory.
	// The paths that are inserted by FileCache::Load are references to strings
	// contained within the launcher_cache.xml string.
	// The lifetimes of paths inserted by calling FileCache::Add are tracked by
	// the caller.
	using MapType = std::unordered_map<StringView, CachedFileData>;
	MapType Map;

	// Tracks whether the map was changed.
	// If there are no changes, FileCache::Save does nothing.
	bool Changed = false;

	// Data from the launcher_cache.xml file.
	// The vector is moved from XMLFile into this after FileCache::Load has
	// finished to preserve the validity of string pointers that point into it.
	// std::vector is used instead of std::string because std::vector::swap
	// guarantees that no iterators will be invalidated, which std::string::swap
	// does not.
	std::vector<u8> CacheXMLFileData;
};