#pragma once

#include <cstddef>
#include <cstdint>

using std::size_t;
using std::ptrdiff_t;
using std::nullptr_t;

using i8 = int8_t;
using i16 = int16_t;
using i32 = int32_t;
using i64 = int64_t;
using u8 = uint8_t;
using u16 = uint16_t;
using u32 = uint32_t;
using u64 = uint64_t;

struct v2;
struct v3;
struct v4;
using rvector = v3;
struct rmatrix;
struct rplane;
struct rquaternion;

#ifndef _WIN32
#include <cstring>
#include <cassert>
#include <cstddef>
inline int _stricmp(const char* a, const char* b) { return strcasecmp(a, b); }
#define _ASSERT assert
#ifndef __cpp_lib_nonmember_container_access
namespace std {
template <typename T, std::size_t N>
constexpr std::size_t size(T(&)[N]) { return N; }
template <typename T>
constexpr auto size(T&& x) -> decltype(x.size()) { return x.size(); }
}
#endif
#endif