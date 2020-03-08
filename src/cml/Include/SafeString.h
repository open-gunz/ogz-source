// # SafeString.h
//
// Functions for interacting with C strings (null-terminated arrays of characters) with
// bounds checking.
//
// # Return values
//
// If the return type of a function in this file is a pointer to a character, the value is the
// address of the null terminator in the destination string after writing.
//
// If the return type is instead integral, the value is the number of characters written, not
// including the null terminator.
//
#pragma once
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <cassert>
#include <algorithm>
#include <string>
#include "StringView.h"
#include "ArrayView.h"

extern void(*SafeStringOnOverflowFunc)();

#pragma warning(push)
#pragma warning(disable:4996)

inline size_t strlen_generic(char* ptr) { return strlen(ptr); }
inline size_t strlen_generic(wchar_t* ptr) { return wcslen(ptr); }
template <typename T>
size_t strlen_generic(T* ptr) {
	size_t ret = 0;
	while (ptr[ret])
		++ret;
	return ret;
}

namespace SafeStringDetail {
inline size_t GetMinSize(char* p, size_t s) {
	auto ptr = memchr(p, 0, s);
	return ptr ? size_t(static_cast<char*>(ptr) - p) : s;
}
template <typename T>
size_t GetMinSize(T* p, size_t s) {
	size_t r = 0;
	while (p[r] && r < s)
		r++;
	return r;
}
template <typename T>
inline size_t GetMinSize(BasicStringView<T> p, size_t s) {
	return (std::min(p.size(), s));
}
}

template <typename T>
size_t strlen_safe(ArrayView<T> str) {
	size_t ret = 0;
	while (true)
	{
		if (ret >= str.size())
		{
			SafeStringOnOverflowFunc();
			return str.size() - 1;
		}
		if (!str[ret])
			return ret;
		++ret;
	}
}

inline size_t strlen_safe(ArrayView<char> str) {
	return strlen_safe<char>(str);
}
inline size_t strlen_safe(ArrayView<wchar_t> str) {
	return strlen_safe<wchar_t>(str);
}

inline char* strcpy_unsafe(char* a, const char* b) {
	return strcpy(a, b);
}

template <typename T, size_t size1, size_t size2>
T* strcpy_literal(T(&Dest)[size1], const T(&Source)[size2]) {
	static_assert(size2 < size1, "Literal must be smaller than the destination string");
	memcpy(Dest, Source, size2);
	return Dest + size2 - 1;
}

template <typename T>
T* strcpy_trunc(ArrayView<T> Dest, BasicStringView<T> Source) {
	auto sz = (std::min)(Dest.size() - 1, Source.size());
	memcpy(Dest.data(), Source.data(), sz);
	Dest[sz] = 0;
	return &Dest[sz];
}
inline char* strcpy_trunc(ArrayView<char> Dest, StringView Source) {
	return strcpy_trunc<char>(Dest, Source);
}
inline wchar_t* strcpy_trunc(ArrayView<wchar_t> Dest, WStringView Source) {
	return strcpy_trunc<wchar_t>(Dest, Source);
}
template <typename T>
T* strcpy_trunc(T* Dest, size_t Size, const T* Source) {
	return strcpy_trunc<T>({Dest, Size}, Source);
}

template <typename T>
T* strcpy_safe(ArrayView<T> Dest, BasicStringView<T> Source) {
	if (Dest.size() - 1 < Source.size())
		SafeStringOnOverflowFunc();
	return strcpy_trunc(Dest, Source);
}
inline char* strcpy_safe(ArrayView<char> Dest, StringView Source) {
	return strcpy_safe<char>(Dest, Source);
}
inline wchar_t* strcpy_safe(ArrayView<wchar_t> Dest, WStringView Source) {
	return strcpy_safe<wchar_t>(Dest, Source);
}
template <typename T, typename U>
T* strcpy_safe(T* Dest, size_t Size, U Source) {
	return strcpy_safe<T>(ArrayView<T>(Dest, Size), BasicStringView<T>(Source));
}
template <typename T, size_t Size, typename U>
T* strcpy_safe(T (&Dest)[Size], U Source) {
	return strcpy_safe<T>(ArrayView<T>(Dest, Size), BasicStringView<T>(Source));
}

template <typename T>
T* strncpy_safe(ArrayView<T> Dest, BasicStringView<T> Source, size_t Count) {
	return strcpy_safe<T>(Dest, {Source.data(), (std::min)(Source.size(), Count)});
}
inline char* strncpy_safe(ArrayView<char> Dest, StringView Source, size_t Count) {
	return strncpy_safe<char>(Dest, Source, Count);
}
inline wchar_t* strncpy_safe(ArrayView<wchar_t> Dest, WStringView Source, size_t Count) {
	return strncpy_safe<wchar_t>(Dest, Source, Count);
}
inline char* strncpy_safe(ArrayView<char> Dest, const char* Source, size_t Count) {
	return strcpy_safe(Dest, {Source, SafeStringDetail::GetMinSize(Source, Count)});
}
inline wchar_t* strncpy_safe(ArrayView<wchar_t> Dest, const wchar_t* Source, size_t Count) {
	return strcpy_safe(Dest, {Source, SafeStringDetail::GetMinSize(Source, Count)});
}
template <typename T, typename U>
T* strncpy_safe(T* Dest, size_t DestSize, U Source, size_t Count) {
	return strncpy_safe<T>({Dest, DestSize}, Source, Count);
}

template <typename T>
T* strcat_safe(ArrayView<T> Dest, BasicStringView<T> Source) {
	return strcpy_safe(Dest.subview(strlen_safe(Dest)), Source);
}
inline char* strcat_safe(ArrayView<char> Dest, StringView Source) {
	return strcat_safe<char>(Dest, Source);
}
inline wchar_t* strcat_safe(ArrayView<wchar_t> Dest, WStringView Source) {
	return strcat_safe<wchar_t>(Dest, Source);
}
template <typename T, typename U>
T* strcat_safe(T* Dest, size_t Size, U Source) {
	return strcat_safe<T>({Dest, Size}, Source);
}

template <typename T>
T* strncat_safe(ArrayView<T> Dest, BasicStringView<T> Source, size_t Count) {
	return strcat_safe(Dest, {Source.data(), (std::min)(Source.size(), Count)});
}
inline char* strncat_safe(ArrayView<char> Dest, StringView Source, size_t Count) {
	return strncat_safe<char>(Dest, Source, Count);
}
inline wchar_t* strncat_safe(ArrayView<wchar_t> Dest, WStringView Source, size_t Count) {
	return strncat_safe<wchar_t>(Dest, Source, Count);
}
inline char* strncat_safe(ArrayView<char> Dest, const char* Source, size_t Count) {
	return strcat_safe(Dest, {Source, SafeStringDetail::GetMinSize(Source, Count)});
}
inline wchar_t* strncat_safe(ArrayView<wchar_t> Dest, const wchar_t* Source, size_t Count) {
	return strcat_safe(Dest, {Source, SafeStringDetail::GetMinSize(Source, Count)});
}
template <typename T, typename U>
T* strncat_safe(T* Dest, size_t DestSize, U Source, size_t Count) {
	return strncat_safe<T>({Dest, DestSize}, Source, Count);
}

template <typename T, int f(T*, size_t, const T*, va_list)>
inline int vsprintf_safe_impl(T* Dest, size_t size, const T* Format, va_list va)
{
	if (size == 0)
	{
		return f(nullptr, 0, Format, va);
	}
	auto ret = f(Dest, size, Format, va);
	if (ret < 0)
		return ret;
	if (ret > int(size) - 1)
	{
		SafeStringOnOverflowFunc();
		ret = int(size) - 1;
	}
	return ret;
}

inline int vsprintf_safe(char *Dest, size_t size, const char* Format, va_list va) {
	return vsprintf_safe_impl<char, vsnprintf>(Dest, size, Format, va);
}
inline int vsprintf_safe(wchar_t *Dest, size_t size, const wchar_t* Format, va_list va) {
	return vsprintf_safe_impl<wchar_t, vswprintf>(Dest, size, Format, va);
}
inline int vsprintf_safe(ArrayView<char> Dest, const char* Format, va_list va) {
	return vsprintf_safe(Dest.data(), Dest.size(), Format, va);
}
inline int vsprintf_safe(ArrayView<wchar_t> Dest, const wchar_t* Format, va_list va) {
	return vsprintf_safe(Dest.data(), Dest.size(), Format, va);
}

// Can't use a function here because va_start etc. are only valid in the originating function.
#define SPRINTF_SAFE_IMPL(ssi_dest, ssi_format)\
	va_list args;\
	va_start(args, ssi_format);\
	int ret = vsprintf_safe(ssi_dest, ssi_format, args);\
	va_end(args);\
	return ret

inline int sprintf_safe(ArrayView<char> Dest, const char* Format, ...) {
	SPRINTF_SAFE_IMPL(Dest, Format);
}
inline int sprintf_safe(ArrayView<wchar_t> Dest, const wchar_t* Format, ...) {
	SPRINTF_SAFE_IMPL(Dest, Format);
}
template <typename T>
int sprintf_safe(T* Dest, size_t Size, const T* Format, ...) {
	SPRINTF_SAFE_IMPL(ArrayView<T>(Dest, Size), Format);
}

#undef SPRINTF_SAFE_IMPL

template <typename T>
void vstrprintf_append(std::basic_string<T>& Output, const T* Format, va_list va)
{
	va_list va2;
	va_copy(va2, va);
	auto Size = static_cast<size_t>(vsprintf_safe(nullptr, 0, Format, va2));
	va_end(va2);
	auto CurSize = Output.size();
	Output.resize(CurSize + Size);
	vsprintf_safe(&Output[CurSize], Size + 1, Format, va);
}

template <typename T>
void strprintf_append(std::basic_string<T>& Output, const T* Format, ...)
{
	va_list va;
	va_start(va, Format);
	vstrprintf_append(Output, Format, va);
	va_end(va);
}

template <typename T>
std::basic_string<T> vstrprintf(const T* Format, va_list va)
{
	std::basic_string<T> Ret;
	vstrprintf_append(Ret, Format, va);
	return Ret;
}

template <typename T>
std::basic_string<T> strprintf(const T* Format, ...)
{
	std::basic_string<T> Ret;
	va_list va;
	va_start(va, Format);
	vstrprintf_append(Ret, Format, va);
	va_end(va);
	return Ret;
}

template <typename T>
char* itoa_safe(T val, ArrayView<char> dest, int radix = 10)
{
	assert(radix > 1 && radix <= 36);
	if (dest.empty())
		return dest.data();
	auto i = dest.data(), end = &dest.back();
	bool negative = val < 0;
	std::make_unsigned_t<T> absval;
	if (negative)
	{
		--end;
		absval = 0;
		absval -= val;
	}
	else
	{
		absval = val;
	}
	while (i < end)
	{
		auto digit = absval % radix;
		auto c = digit + (digit < 10 ? '0' : 'a' - 10);
		*i = c;
		absval /= radix;
		++i;
		if (!absval)
			break;
	}
	if (negative)
	{
		*i = '-';
		++i;
	}
	*i = 0;
	std::reverse(dest.data(), i);
	return i;
}

namespace detail {
inline int strerror_r_ret_impl(int ret) { return ret; }
inline int strerror_r_ret_impl(char*) { return 0; }
}

// Returns zero if ok, and a non-zero value on error.
inline int strerror_safe(int errnum, char* buf, size_t buflen)
{
#ifdef _MSC_VER
	return strerror_s(buf, buflen, errnum);
#else
	// Pass the result through an overloaded function to support the non-standard GNU strerror_r
	// version that returns char* instead of int.
	return detail::strerror_r_ret_impl(strerror_r(errnum, buf, buflen));
#endif
}

template <size_t buflen>
inline int strerror_safe(int errnum, char (&buf)[buflen]) {
	return strerror_safe(errnum, buf, buflen);
}

inline char* strlwr_safe(ArrayView<char> str)
{
	size_t i = 0;
	for (; str[i] && i < str.size(); ++i)
	{
		str[i] = tolower(str[i]);
	}
	return str.data() + i;
}

inline wchar_t* strlwr_safe(ArrayView<wchar_t> str)
{
	size_t i = 0;
	for (; str[i] && i < str.size(); ++i)
	{
		str[i] = towlower(str[i]);
	}
	return str.data() + i;
}

template <typename T>
inline T* strlwr_safe(T* str, size_t size) {
	return strlwr_safe({str, size});
}

#pragma warning(pop)