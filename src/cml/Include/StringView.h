#pragma once

#include <cstring>
#include <cassert>
#include <string>
#include <cwchar>
#include <cctype>
#include <cwctype>
#include <utility>
#include "GlobalTypes.h"
#include "ArrayView.h"

namespace StringViewDetail
{
inline size_t len(char* ptr) { return strlen(ptr); }
inline size_t len(wchar_t* ptr) { return wcslen(ptr); }
template <typename T>
inline size_t len(T* ptr) {
	size_t ret = 0;
	while (ptr[ret])
		++ret;
	return ret;
}
}

template <typename CharType>
struct BasicStringView
{
	using value_type = CharType;
	using reference = const value_type&;
	using const_reference = reference;
	using pointer = const value_type*;
	using const_pointer = pointer;
	using iterator = pointer;
	using const_iterator = iterator;
	using difference_type = ptrdiff_t;
	using size_type = size_t;
	using reverse_iterator = std::reverse_iterator<iterator>;
	using const_reverse_iterator = reverse_iterator;

	BasicStringView() : ptr{""}, sz{0} {}
	BasicStringView(const CharType* ptr) : ptr{ptr}, sz{StringViewDetail::len(ptr)} {}
	BasicStringView(const CharType* ptr, size_t sz) : ptr{ptr}, sz{sz} {}
	template <typename U, typename = decltype(std::declval<pointer&>() = std::declval<U&>().data()),
		typename = decltype(std::declval<U&>().size())>
	BasicStringView(U&& x) : BasicStringView(x.data(), x.size()) {}

	const CharType& operator[](size_t i) const {
		assert(i < sz);
		return ptr[i];
	}

	const CharType* data() const { return ptr; }
	size_t size() const { return sz; }
	bool empty() const { return sz == 0; }

	iterator begin() const { return ptr; }
	iterator end() const { return ptr + size(); }

	reverse_iterator rbegin() const { return reverse_iterator{ end() }; }
	reverse_iterator rend() const { return reverse_iterator{ begin() }; }

	std::basic_string<CharType> str() const { return{ data(), size() }; }

	void remove_prefix(size_t n) {
		assert(n <= size());
		ptr += n;
		sz -= n;
	}

	void remove_suffix(size_t n) {
		assert(n <= size());
		sz -= n;
	}

	BasicStringView substr(size_t pos, size_t count = npos) const {
		assert(pos <= size());
		return{data() + pos, (std::min)(count, size() - pos)};
	}

	const CharType& front() const {
		assert(!empty());
		return (*this)[0];
	}

	const CharType& back() const {
		assert(!empty());
		return (*this)[size() - 1];
	}

	size_t find(const BasicStringView& needle) const {
		return it2sz(std::search(begin(), end(), needle.begin(), needle.end()));
	}

	size_t find_first_of(const BasicStringView& needle, size_t pos = 0) const {
		return find_impl(begin() + pos, end(), needle, false);
	}

	size_t find_first_of(CharType c, size_t pos = 0) const {
		return find_first_of(BasicStringView{ &c, 1 }, pos);
	}

	size_t find_first_not_of(const BasicStringView& needle, size_t pos = 0) const {
		return find_impl(begin() + pos, end(), needle, true);
	}

	size_t find_first_not_of(CharType c, size_t pos = 0) const {
		return find_first_not_of(BasicStringView{&c, 1}, pos);
	}

	size_t find_last_of(const BasicStringView& needle, size_t pos = npos) const {
		if (pos == npos)
			pos = size();
		return find_impl(rbegin() + (size() - pos), rend(), needle, false);
	}

	size_t find_last_of(CharType c, size_t pos = npos) const {
		return find_last_of(BasicStringView{&c, 1}, pos);
	}

	size_t find_last_not_of(const BasicStringView& needle, size_t pos = npos) const {
		if (pos == npos)
			pos = size();
		return find_impl(rbegin() + (size() - pos), rend(), needle, true);
	}

	size_t find_last_not_of(CharType c, size_t pos = npos) const {
		return find_last_not_of(BasicStringView{&c, 1}, pos);
	}

	static constexpr size_t npos = static_cast<size_t>(-1);

	size_t it2sz(iterator it) const {
		return it == end() ? npos : size_t(it - begin());
	}

	size_t it2sz(reverse_iterator it) const {
		return it == rend() ? npos : size_t(std::prev(it.base()) - begin());
	}

private:
	template <typename T>
	size_t find_impl(T a, T b, BasicStringView needle, bool negate) const
	{
		assert(a <= b);
		return it2sz(std::find_if(a, b, [&](CharType c) {
			bool found = std::find(needle.begin(), needle.end(), c) != needle.end();
			return negate ^ found;
		}));
	}

	const CharType* ptr;
	size_t sz;
};

using StringView = BasicStringView<char>;
using WStringView = BasicStringView<wchar_t>;

inline StringView operator "" _sv(const char* ptr, size_t sz) { return {ptr, sz}; }
inline WStringView operator "" _sv(const wchar_t* ptr, size_t sz) { return {ptr, sz}; }

template <typename C>
bool operator==(const BasicStringView<C>& lhs, const BasicStringView<C>& rhs) {
	return lhs.size() == rhs.size() && !memcmp(lhs.data(), rhs.data(), lhs.size() * sizeof(C));
}

template <typename C>
bool operator!=(const BasicStringView<C>& lhs, const BasicStringView<C>& rhs) {
	return !(lhs == rhs);
}

template <typename C>
bool operator<(const BasicStringView<C>& lhs, const BasicStringView<C>& rhs) {
	return std::lexicographical_compare(lhs.begin(), lhs.end(), rhs.begin(), rhs.end());
}

template <typename C>
bool operator<=(const BasicStringView<C>& lhs, const BasicStringView<C>& rhs) {
	return lhs < rhs || lhs == rhs;
}

template <typename C>
bool operator>(const BasicStringView<C>& lhs, const BasicStringView<C>& rhs) {
	return !(lhs <= rhs);
}

template <typename C>
bool operator>=(const BasicStringView<C>& lhs, const BasicStringView<C>& rhs) {
	return !(lhs < rhs);
}

inline bool operator==(StringView lhs, StringView rhs) { return operator==<char>(lhs, rhs); }
inline bool operator!=(StringView lhs, StringView rhs) { return operator!=<char>(lhs, rhs); }
inline bool operator< (StringView lhs, StringView rhs) { return operator< <char>(lhs, rhs); }
inline bool operator<=(StringView lhs, StringView rhs) { return operator<=<char>(lhs, rhs); }
inline bool operator> (StringView lhs, StringView rhs) { return operator> <char>(lhs, rhs); }
inline bool operator>=(StringView lhs, StringView rhs) { return operator>=<char>(lhs, rhs); }

inline bool operator==(WStringView lhs, WStringView rhs) { return operator==<wchar_t>(lhs, rhs); }
inline bool operator!=(WStringView lhs, WStringView rhs) { return operator!=<wchar_t>(lhs, rhs); }
inline bool operator< (WStringView lhs, WStringView rhs) { return operator< <wchar_t>(lhs, rhs); }
inline bool operator<=(WStringView lhs, WStringView rhs) { return operator<=<wchar_t>(lhs, rhs); }
inline bool operator> (WStringView lhs, WStringView rhs) { return operator> <wchar_t>(lhs, rhs); }
inline bool operator>=(WStringView lhs, WStringView rhs) { return operator>=<wchar_t>(lhs, rhs); }

namespace detail
{
struct ieq
{
	bool operator()(char a, char b) { return tolower(a) == tolower(b); }
	bool operator()(wchar_t a, wchar_t b) { return towlower(a) == towlower(b); }
};
}

template <typename CharType>
bool equals(BasicStringView<CharType> a, BasicStringView<CharType> b) { return a == b; }
inline bool equals(StringView a, StringView b) { return a == b; }
inline bool equals(WStringView a, WStringView b) { return a == b; }

template <typename CharType>
bool iequals(const BasicStringView<CharType>& lhs, const BasicStringView<CharType>& rhs) {
	return std::equal(std::begin(lhs), std::end(lhs),
		std::begin(rhs), std::end(rhs),
		detail::ieq{});
}

inline bool iequals(const StringView& lhs, const StringView& rhs) {
	return iequals<char>(lhs, rhs);
}

inline bool iequals(const WStringView& lhs, const WStringView& rhs) {
	return iequals<wchar_t>(lhs, rhs);
}

template <typename CharType>
size_t ifind(const BasicStringView<CharType>& haystack, const BasicStringView<CharType>& needle) {
	return haystack.it2sz(std::search(std::begin(haystack), std::end(haystack),
		std::begin(needle), std::end(needle), detail::ieq{}));
}

inline size_t ifind(const StringView& haystack, const StringView& needle) {
	return ifind<char>(haystack, needle);
}

inline size_t ifind(const WStringView& haystack, const WStringView& needle) {
	return ifind<wchar_t>(haystack, needle);
}

template <typename CharType>
bool icontains(const BasicStringView<CharType>& haystack, const BasicStringView<CharType>& needle) {
	return ifind(haystack, needle) != haystack.npos;
}

inline bool icontains(const StringView& haystack, const StringView& needle) {
	return icontains<char>(haystack, needle);
}

inline bool icontains(const WStringView& haystack, const WStringView& needle) {
	return icontains<wchar_t>(haystack, needle);
}

template <typename CharType>
bool starts_with(BasicStringView<CharType> a, BasicStringView<CharType> b)
{
	if (b.size() > a.size())
		return false;
	return a.substr(0, b.size()) == b;
}

inline bool starts_with(StringView a, StringView b) { return starts_with<char>(a, b); }
inline bool starts_with(WStringView a, WStringView b) { return starts_with<wchar_t>(a, b); }

template <typename CharType>
bool ends_with(BasicStringView<CharType> a, BasicStringView<CharType> b)
{
	if (b.size() > a.size())
		return false;
	return a.substr(a.size() - b.size(), b.size()) == b;
}

inline bool ends_with(StringView a, StringView b) { return ends_with<char>(a, b); }
inline bool ends_with(WStringView a, WStringView b) { return ends_with<wchar_t>(a, b); }

template <typename CharType>
bool istarts_with(BasicStringView<CharType> a, BasicStringView<CharType> b)
{
	if (b.size() > a.size())
		return false;
	return iequals(a.substr(0, b.size()), b);
}

inline bool istarts_with(StringView a, StringView b) { return istarts_with<char>(a, b); }
inline bool istarts_with(WStringView a, WStringView b) { return istarts_with<wchar_t>(a, b); }

template <typename CharType>
bool iends_with(BasicStringView<CharType> a, BasicStringView<CharType> b)
{
	if (b.size() > a.size())
		return false;
	return iequals(a.substr(a.size() - b.size(), b.size()), b);
}

inline bool iends_with(StringView a, StringView b) { return iends_with<char>(a, b); }
inline bool iends_with(WStringView a, WStringView b) { return iends_with<wchar_t>(a, b); }

template <typename CharType>
inline BasicStringView<CharType> trim(BasicStringView<CharType> Str)
{
	CharType SpaceChar = ' ';
	BasicStringView<CharType> Space{&SpaceChar, 1};
	while (starts_with(Str, Space))
		Str.remove_prefix(1);
	while (ends_with(Str, Space))
		Str.remove_suffix(1);
	return Str;
}

inline StringView trim(StringView Str) { return trim<char>(Str); }
inline WStringView trim(WStringView Str) { return trim<wchar_t>(Str); }