#pragma once

#include "GlobalTypes.h"
#include "StringView.h"
#include <cctype>
#include <functional>
#include <algorithm>
#include <climits>

namespace detail
{
template <size_t N = sizeof(void*) * CHAR_BIT>
struct FNV_values;

template <> struct FNV_values<32>
{
	static constexpr auto FNV_offset_basis = 2166136261U;
	static constexpr auto FNV_prime = 16777619U;
};

template <> struct FNV_values<64>
{
	static constexpr auto FNV_offset_basis = 14695981039346656037ULL;
	static constexpr auto FNV_prime = 1099511628211ULL;
};
}

template <class F>
inline size_t HashFNVApply(const void* Memory, size_t Length, F&& transform_byte)
{
	constexpr size_t FNV_offset_basis = detail::FNV_values<>::FNV_offset_basis;
	constexpr size_t FNV_prime = detail::FNV_values<>::FNV_prime;

	size_t ret = FNV_offset_basis;
	for (size_t Next = 0; Next < Length; ++Next)
	{	// fold in another byte
		unsigned char byte = reinterpret_cast<const unsigned char*>(Memory)[Next];
		ret ^= static_cast<size_t>(transform_byte(byte));
		ret *= FNV_prime;
	}
	return ret;
}

inline size_t HashFNV(const void* Memory, size_t Length) {
	return HashFNVApply(Memory, Length, [](unsigned char c) { return c; });
}
inline size_t HashFNVCaseInsensitive(const void* Memory, size_t Length) {
	return HashFNVApply(Memory, Length, [](unsigned char c) { return std::tolower(c); });
}
inline unsigned char PathTrans(unsigned char c) { return std::tolower(c == '\\' ? '/' : c); }
inline size_t HashFNVPath(const void* Memory, size_t Length) {
	return HashFNVApply(Memory, Length, PathTrans);
}

struct StringHasher {
	size_t operator()(StringView str) const {
		return HashFNV(str.data(), str.size());
	}
};

struct CaseInsensitiveStringHasher {
	size_t operator()(StringView str) const {
		return HashFNVCaseInsensitive(str.data(), str.size());
	}
};

struct CaseInsensitiveComparer {
	bool operator()(StringView a, StringView b) const { return iequals(a, b); }
};

struct PathHasher {
	size_t operator()(StringView str) const {
		return HashFNVPath(str.data(), str.size());
	}
};

struct PathComparer {
	bool operator()(StringView lhs, StringView rhs) const {
		return std::equal(lhs.begin(), lhs.end(), rhs.begin(), rhs.end(),
			[](char a, char b) { return PathTrans(u8(a)) == PathTrans(u8(b)); });
	}
};

namespace std
{
template <typename T>
struct hash<BasicStringView<T>> : StringHasher {};
}