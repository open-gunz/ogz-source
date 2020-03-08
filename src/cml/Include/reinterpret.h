#pragma once

#include <type_traits>
#include <cstring>

template <typename T1, size_t Index, typename T2>
T1 reinterpret_at(const T2& val)
{
	static_assert(std::is_trivially_copyable<T1>::value &&
		std::is_trivially_copyable<T2>::value,
		"Types must be trivially copyable to reinterpret");

	static_assert(sizeof(T2) - Index >= sizeof(T1), "Index is out of bounds");

	T1 T1_rep;
	memcpy(&T1_rep, &val, sizeof(T1_rep));
	return T1_rep;
}

template <typename T1, typename T2>
T1 reinterpret(const T2& val)
{
	static_assert(!std::is_pointer<T2>::value, "reinterpret cannot be used with pointers. "
		"Use either reinterpret_address or reinterpret_pointee instead (depending on your intent).");

	static_assert(sizeof(T1) == sizeof(T2),
		"The size of the types must be the same for reinterpret. "
		"Use reinterpret_at for differently sized types.");

	return reinterpret_at<T1, 0>(val);
}

// Reinterprets the address itself in a pointer.
template <typename T1, typename T2>
T1 reinterpret_address(const void* val)
{
	return reinterpret_at<T1, 0>(val);
}

// Reinterprets the data pointed to by a pointer.
// This is the non-void* overloads, so it performs a size check.
template <typename T1, typename T2>
std::enable_if_t<!std::is_same<T2, void>::value, T1> reinterpret_pointee(const T2* val)
{
	return reinterpret<T1>(*val);
}

// Reinterprets the data pointed to by a pointer.
// This is the void* overload, so it doesn't perform a size check.
template <typename T1, typename T2>
std::enable_if_t<std::is_same<T2, void>::value, T1> reinterpret_pointee(const T2* val)
{
	return reinterpret<T1>(*reinterpret_cast<const char(*)[sizeof(T1)]>(val));
}