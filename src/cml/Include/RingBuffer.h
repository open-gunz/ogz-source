#pragma once

#include <type_traits>
#include <limits>
#include <cstdint>
#include <cstddef>
#include <utility>
#include <algorithm>
#include "GlobalTypes.h"

// Use the lowest of u8, u16, and u32 that support the size
template <typename T, size_t N>
using RingBufferCursorType =
std::conditional_t<N <= (std::numeric_limits<u8>::max)() + 1u, u8,
	std::conditional_t<N <= (std::numeric_limits<u16>::max)() + 1u, u16,
	u32>>;

template <typename T, size_t N>
struct RingBuffer;

template <typename T, size_t N>
struct RingIterator
{
	RingBuffer<T, N>& Ring;
	size_t Offset;

	bool operator==(const RingIterator& rhs) const { return Offset == rhs.Offset; }
	bool operator!=(const RingIterator& rhs) const { return !(*this == rhs); }

	T& operator*();
	const T& operator*() const;

	RingIterator& operator++() {
		++Offset;
		return *this;
	}

	RingIterator& operator++(int) {
		auto temp = *this;
		++*this;
		return temp;
	}
};

template <typename T, size_t N>
struct RingBuffer
{
	RingBuffer() = default;
	RingBuffer(const RingBuffer& Src)
	{
		copy(Src);
	}
	RingBuffer& operator=(const RingBuffer& Src)
	{
		copy(Src);
	}
	RingBuffer(RingBuffer&& Src)
	{
		move(std::move(Src));
	}
	RingBuffer& operator=(RingBuffer&& Src)
	{
		move(std::move(Src));
	}
	~RingBuffer()
	{
		destroy();
	}

	using CursorType = RingBufferCursorType<T, N>;

	T& operator[](size_t i) { return *get(i); }
	const T& operator[](size_t i) const { return *get(i); }

	template <typename... ArgsType>
	void emplace_front(ArgsType&&... Args) {
		Cursor = (Cursor - 1) % N;
		new (get(0)) T{ std::forward<ArgsType>(Args)... };
		Size = (std::min)(Size + 1, N);
	}

	template <typename... ArgsType>
	void emplace_back(ArgsType&&... Args) {
		if (Size == N)
		{
			new (get(0)) T{std::forward<ArgsType>(Args)...};
			Cursor = (Cursor + 1) % N;
		}
		else
		{
			new (get(Size)) T{std::forward<ArgsType>(Args)...};
			++Size;
		}
	}

	void push_back(const T& Src) { emplace_back(Src); }
	void push_back(T&& Src) { emplace_back(std::move(Src)); }

	void clear()
	{
		destroy();
		Size = 0;
		Cursor = 0;
	}

	auto begin() { return RingIterator<T, N>{ *this, 0 }; }
	auto end() { return RingIterator<T, N>{ *this, Size }; }
	auto begin() const { return RingIterator<T, N>{ *this, 0 }; }
	auto end() const { return RingIterator<T, N>{ *this, Size }; }

	auto& front() { return *begin(); }
	auto& front() const { return *begin(); }
	auto& back() { return *std::prev(end());; }
	auto& back() const { return *std::prev(end()); }

	size_t size() const { return Size; }

	auto empty() const { return size() == 0; }

	constexpr auto max_size() const { return N; }

private:
	const T* get(size_t i) const {
		i = (Cursor + i) % N;
		return reinterpret_cast<const T*>(buf + sizeof(T) * i);
	}

	T* get(size_t i) {
		auto* ptr = static_cast<const RingBuffer*>(this)->get(i);
		return const_cast<T*>(ptr);
	}

	void copy(const RingBuffer& Src)
	{
		Cursor = Src.Cursor;
		Size = Src.Size;
		size_t i = 0;
		for (auto&& x : Src)
		{
			new (get(i)) T{x};
			++i;
		}
	}

	void move(RingBuffer&& Src)
	{
		Cursor = Src.Cursor;
		Size = Src.Size;
		size_t i = 0;
		for (auto&& x : Src)
		{
			new (get(i)) T{std::move(x)};
			++i;
		}
	}

	void destroy()
	{
		for (auto&& x : *this)
			x.~T();
	}

	alignas(T) unsigned char buf[sizeof(T) * N];
	CursorType Cursor{};
	size_t Size{};
};

template <typename T, size_t N>
T& RingIterator<T, N>::operator*() { return Ring[Offset]; }
template <typename T, size_t N>
const T& RingIterator<T, N>::operator*() const { return Ring[Offset]; }