#pragma once

#include "rapidxml.hpp"
#include "rapidxml_print.hpp"

#include "StringView.h"
#include "Log.h"
#include "Download.h"
#include "File.h"
#include "LauncherConfig.h"

#include "GlobalTypes.h"
#include "MUtil.h"
#include "optional.h"
#include "MFile.h"

#include <vector>

inline bool TryParseXML(rapidxml::xml_document<>& doc, const StringView& filename, char* data, size_t size)
{
	try
	{
		doc.parse<rapidxml::parse_default>(data, size);
	}
	catch (rapidxml::parse_error &e)
	{
		Log(LogLevel::Error, "RapidXML threw parse_error on %.*s!\n"
			"e.what() = %s, e.where() = %s\n",
			filename.size(), filename.data(),
			e.what(), e.where<char>());
		return false;
	}

	return true;
}

// A class that stores an XML loaded from a file.
//
// It can be created from an actual file or from a internet URL, and provides member functions
// for getting common attribute values that log errors.
//
// It retains the file data with it (so that RapidXML's references don't become dangling) and saves
// the path to the file for logging on errors.
// It is immovable, because rapidxml::xml_document is.
struct XMLFile
{
	bool CreateFromFile(const char* PathArg)
	{
		strcpy_safe(Path, PathArg);

		if (!ReadFile(FileData, Path))
		{
			Log.Error("Failed to read \"%s\"!\n", Path);
			return false;
		}

		if (!TryParseXML(Doc, Path, reinterpret_cast<char*>(&FileData[0]), FileData.size()))
		{
			Log.Error("Failed to parse \"%s\"!\n", Path);
			return false;
		}

		return true;
	}

	bool CreateFromDownload(const DownloadManagerType& DownloadManager,
		const char* URL, int Port, const char* Filename = "")
	{
		// Download the file, inserting each incoming block of data directly into FileData

		strcpy_safe(Path, Filename);

		auto Callback = [&](const void* Buffer, size_t Size, DownloadInfo&)
		{
			auto* CharBuffer = static_cast<const char*>(Buffer);
			// Insert the data at the end of the FileData string.
			FileData.insert(FileData.end(), CharBuffer, CharBuffer + Size);
			return true;
		};

		DownloadError Error;
		auto DownloadSuccess = DownloadFile(DownloadManager, URL, LauncherConfig::PatchPort,
			std::ref(Callback), {}, nullptr, &Error);

		if (!DownloadSuccess)
		{
			Log(LogLevel::Fatal, "Failed to download \"%s\"!\n", Path);
			return false;
		}

		// Parse the downloaded file data.
		if (!TryParseXML(Doc, Path, reinterpret_cast<char*>(&FileData[0]), FileData.size()))
		{
			Log(LogLevel::Fatal, "Failed to parse \"%s\"!\n", Path);
			return false;
		}
		
		return true;
	}

	// Returns the string value of the name attribute, or an empty string on error.
	StringView GetName(rapidxml::xml_node<>& node) const {
		return GetAttribute("name", node, "");
	}
	// Returns the string value of the hash attribute, or an empty string on error.
	StringView GetHash(rapidxml::xml_node<>& node, const StringView& Filename) const {
		return GetAttribute("hash", node, Filename);
	}
	// Returns the integral value of the size attribute, or an empty optional on error.
	optional<u64> GetSize(rapidxml::xml_node<>& node, const StringView& Filename) const {
		return GetIntegralAttribute<u64>("size", node, Filename);
	}
	// Returns the integral value of the last_modified_time attribute, or an empty optional on error.
	optional<u64> GetLastModifiedTime(rapidxml::xml_node<>& node, const StringView& Filename) const {
		return GetIntegralAttribute<u64>("last_modified_time", node, Filename);
	}

	StringView GetAttribute(const StringView& AttributeName,
		rapidxml::xml_node<>& node,
		const StringView& Filename) const
	{
		const auto HasFilename = !Filename.empty();

		auto Attribute = node.first_attribute(AttributeName.data(), AttributeName.size());
		if (Attribute == nullptr)
		{
			if (HasFilename)
			{
				Log(LogLevel::Error, "In %s: File node of file \"%.*s\" doesn't have a %.*s attribute\n",
					Path,
					Filename.size(), Filename.data(),
					AttributeName.size(), AttributeName.data());
			}
			else
			{
				Log(LogLevel::Error, "In %s: File node doesn't have a %.*s attribute\n",
					Path,
					AttributeName.size(), AttributeName.data());
			}

			return{};
		}

		auto* Pointer = Attribute->value();
		auto Size = Attribute->value_size();
		if (Pointer == nullptr ||
			Size == 0)
		{
			if (HasFilename)
			{
				Log(LogLevel::Error, "In %s: File node of file \"%.*s\"'s %.*s attribute doesn't have a valid value\n",
					Path,
					Filename.size(), Filename.data(),
					AttributeName.size(), AttributeName.data());
			}
			else
			{
				Log(LogLevel::Error, "In %s: File node's %.*s attribute doesn't have a valid value\n",
					Path,
					AttributeName.size(), AttributeName.data());
			}

			return{};
		}

		return{ Pointer, Size };
	}

	template <typename T, int Radix = 10>
	optional<T> GetIntegralAttribute(const StringView& AttributeName,
		rapidxml::xml_node<>& node,
		const StringView& Filename) const
	{
		auto String = GetAttribute(AttributeName, node, Filename);
		if (String.empty())
			return nullopt;

		auto MaybeValue = StringToInt<T, Radix>(String);
		if (!MaybeValue.has_value())
		{
			Log(LogLevel::Error, "In %s: File node %.*s's %.*s attribute doesn't have a valid integral value\n",
				Path,
				Filename.size(), Filename.data(),
				AttributeName.size(), AttributeName.data());
		}

		return MaybeValue;
	}

	char Path[MFile::MaxPath];
	rapidxml::xml_document<> Doc; 
	std::vector<u8> FileData;
};

// Appends a new node to a node and returns the newly created one.
// NOTE: This functions stores a REFERENCE to the name string,
// it does not allocate persistent memory of its own.
inline rapidxml::xml_node<>& AppendNode(rapidxml::xml_document<>& doc, rapidxml::xml_node<>& node,
	const StringView& Name)
{
	auto* NewNode = doc.allocate_node(rapidxml::node_element, Name.data(), 0, Name.size());
	node.append_node(NewNode);
	return *NewNode;
}

// Appends a new attribute with a string value to a node.
// NOTE: This function stores REFERENCES to the name and value strings,
// it does not allocate persistent memory of its own.
inline void AppendAttribute(rapidxml::xml_document<>& doc, rapidxml::xml_node<>& node,
	const StringView& Name, const StringView& Value)
{
	auto* Attribute = doc.allocate_attribute(Name.data(), Value.data(), Name.size(), Value.size());
	node.append_attribute(Attribute);
}

inline void AppendNumberImpl(rapidxml::xml_document<>& doc, rapidxml::xml_node<>& node,
	const StringView& Name, u64 Value, const char* Format)
{
	char buf[64];
	auto Size = static_cast<size_t>(sprintf_safe(buf, Format, Value));
	auto StringValue = StringView{ doc.allocate_string(buf, Size), Size };
	AppendAttribute(doc, node, Name, StringValue);
}

// Appends a new attribute with a numerical value to a node.
// NOTE: This functions stores a REFERENCE to the name string,
// it does not allocate persistent memory of its own.
inline void AppendAttribute(rapidxml::xml_document<>& doc, rapidxml::xml_node<>& node,
	const StringView& Name, u64 Value) {
	AppendNumberImpl(doc, node, Name, Value, "%llu");
}

// Appends a new attribute with a numerical value to a node.
// NOTE: This functions stores a REFERENCE to the name string,
// it does not allocate persistent memory of its own.
inline void AppendAttribute(rapidxml::xml_document<>& doc, rapidxml::xml_node<>& node,
	const StringView& Name, i64 Value) {
	AppendNumberImpl(doc, node, Name, Value, "%lld");
}

struct RapidXMLNodeIterator : public IteratorBase<RapidXMLNodeIterator,
	rapidxml::xml_node<>*, std::forward_iterator_tag>
{
	RapidXMLNodeIterator(rapidxml::xml_node<>* node, const char* SiblingName)
		: node{ node }, SiblingName{ SiblingName } {}

	bool operator==(const RapidXMLNodeIterator& rhs) const { return node == rhs.node; }

	auto& operator*() { return *node; }

	RapidXMLNodeIterator& operator++() {
		node = node->next_sibling(SiblingName);
		return *this;
	}

	rapidxml::xml_node<>* node;
	const char* SiblingName = nullptr;
};

template <typename T>
auto GetFileNodeRange(T& node)
{
	using ItT = RapidXMLNodeIterator;
	return MakeRange(
		ItT{ node.first_node("file"), "file" },
		ItT{ nullptr, "file" });
}