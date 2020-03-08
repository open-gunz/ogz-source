#pragma once

#include <memory>

template <typename FallbackAllocatorType>
struct BasicArena
{
	BasicArena(unsigned char* Data, size_t Size, size_t Offset = 0)
		: Data{ Data }, Size{ Size }, Offset{ Offset } {}
	BasicArena(const BasicArena&) = delete;
	BasicArena& operator=(const BasicArena&) = delete;
	~BasicArena() = default;

	void* allocate(size_t n, size_t Alignment = 1)
	{
		Offset &= ~(Alignment - 1);

		if (Offset + n > Size)
		{
			return FallbackAllocator.allocate(n);
		}

		auto ret = Data + Offset;
		Offset += n;
		return ret;
	}

	void deallocate(void* ptr, size_t n)
	{
		if (ptr == Data + Offset)
			Offset -= n;
	}

	unsigned char* Data;
	size_t Size;
	size_t Offset;
	FallbackAllocatorType FallbackAllocator;
};

using Arena = BasicArena<std::allocator<char>>;

template <typename T, typename ArenaType = Arena>
struct ArenaAllocator
{
	using value_type = T;
	using pointer = T*;

	using propagate_on_container_copy_assignment = std::true_type;
	using propagate_on_container_move_assignment = std::true_type;
	using propagate_on_container_swap = std::true_type;

	ArenaAllocator(ArenaType* arena) : arena{ arena } {}

	template <typename U>
	ArenaAllocator(const ArenaAllocator<U>& rhs) : arena(rhs.arena) {}

	pointer allocate(size_t n)
	{
		return static_cast<pointer>(arena->allocate(n * sizeof(T), alignof(T)));
	}

	void deallocate(pointer p, size_t n)
	{
		arena->deallocate(p, n * sizeof(T));
	}

	template <typename U>
	bool operator==(const ArenaAllocator<U>& rhs) const
	{
		return arena == rhs.arena;
	}

	template <typename U>
	bool operator!=(const ArenaAllocator<U>& rhs) const
	{
		return arena != rhs.arena;
	}

	ArenaType* arena;
};