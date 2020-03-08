#pragma once

#include <utility>
#include <algorithm>
#include <cassert>

template <typename T>
class ArrayView
{
public:
	ArrayView() = default;

	ArrayView(T* p, size_t s) : ptr(p), sz(s) {}

	template <size_t arr_size>
	ArrayView(T(&arr)[arr_size]) : ptr(arr), sz(arr_size) {}

	template <typename U, typename = decltype(std::declval<U&>().data()),
		typename = decltype(std::declval<U&>().size())>
	ArrayView(U& x) : ArrayView(x.data(), x.size()) {}

	auto& operator[](size_t Index) const { return ptr[Index]; }

	size_t size() const { return sz; }
	bool empty() const { return sz == 0; }

	auto begin() const { return ptr; }
	auto end() const { return ptr + sz; }

	auto& front() const { return ptr[0]; }
	auto& back() const { return ptr[sz - 1]; }

	auto* data() const { return ptr; }

	void remove_prefix(size_t n)
	{
		assert(n <= size());
		ptr += n;
		sz -= n;
	}

	void remove_suffix(size_t n)
	{
		assert(n <= size());
		sz -= n;
	}

	ArrayView subview(size_t pos, size_t count = size_t(-1)) const {
		assert(pos <= size());
		return{data() + pos, (std::min)(count, size() - pos)};
	}

private:
	T* ptr{};
	size_t sz{};
};

template <typename T>
bool operator==(ArrayView<T> a, ArrayView<T> b) {
	return std::equal(a.begin(), a.end(), b.begin(), b.end());
}

template <typename T>
bool operator!=(ArrayView<T> a, ArrayView<T> b) {
	return !(a == b);
}

template <typename T>
bool operator<(ArrayView<T> a, ArrayView<T> b) {
	return std::lexicographical_compare(a.begin(), a.end(), b.begin(), b.end());
}

template <typename T>
bool operator<=(const ArrayView<T>& a, const ArrayView<T>& b) {
	return a < b || a == b;
}

template <typename T>
bool operator>(const ArrayView<T>& a, const ArrayView<T>& b) {
	return !(a <= b);
}

template <typename T>
bool operator>=(const ArrayView<T>& a, const ArrayView<T>& b) {
	return !(a < b);
}