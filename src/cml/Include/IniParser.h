#pragma once
#include <string>
#include <unordered_map>
#include "ini.h"
#include "optional.h"
#include "MUtil.h"
#include "MHash.h"

template <typename K, typename V>
using imap = std::unordered_map<K, V, CaseInsensitiveStringHasher, CaseInsensitiveComparer>;
using IniMapType = imap<std::string, imap<std::string, std::string>>;

extern "C" {
inline int IniCallbackFwd(void* user, const char* section, const char* name, const char* value)
{
	auto& Map = *static_cast<IniMapType*>(user);
	StringView v = value;
	if (starts_with(v, "\""))
		v.remove_prefix(1);
	if (ends_with(v, "\""))
		v.remove_suffix(1);
	Map[section][name] = v.str();
	return 1;
}
}

struct IniParser
{
	IniMapType Map;

	bool Parse(const char* Filename)
	{
		return ini_parse(Filename, IniCallbackFwd, &Map) == 0;
	}

	optional<StringView> GetString(const char* arg_section, const char* arg_name) const
	{
		auto it = Map.find(arg_section);
		if (it == Map.end())
			return nullopt;

		auto it2 = it->second.find(arg_name);
		if (it2 == it->second.end())
			return nullopt;

		return it2->second;
	}

	StringView GetString(const char* arg_section, const char* arg_name,
		const char* default_value) const
	{
		return GetString(arg_section, arg_name).value_or(default_value);
	}

	template <typename T = int>
	optional<T> GetInt(const char* arg_section, const char* arg_name) const
	{
		auto Str = GetString(arg_section, arg_name);
		if (!Str)
			return nullopt;
		return StringToInt(*Str);
	};

	template <typename T>
	T GetInt(const char* arg_section, const char* arg_name, T default_value) const
	{
		return GetInt(arg_section, arg_name).value_or(default_value);
	};
};