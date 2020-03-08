#pragma once

#include "MUtil.h"

// A blob array is a serialized version of an array of structs,
// suitable for transmitting over the network.
//
// It contains...
// - A 4 byte integer, giving the size of a single element (OneBlobSize).
// - A 4 byte integer, giving the number of elements (BlobCount).
// - BlobCount amount of OneBlobSize-byte large elements.

// Construction and destruction
void* MMakeBlobArray(int nOneBlobSize, int nBlobCount);
void MEraseBlobArray(void* pBlob);

const void* MGetBlobArrayElement(const void* pBlob, int i);
inline void* MGetBlobArrayElement(void* pBlob, int i) {
	return const_cast<void*>(MGetBlobArrayElement(static_cast<const void*>(pBlob), i)); }

int MGetBlobArrayCount(const void* pBlob);
int MGetBlobArraySize(const void* pBlob);

const void* MGetBlobArrayPointer(const void* pBlob);
inline void* MGetBlobArrayPointer(void* pBlob) {
	return const_cast<void*>(MGetBlobArrayPointer(static_cast<const void*>(pBlob))); }

size_t MGetBlobArrayInfoSize();
size_t MGetBlobArrayElementSize(const void* pBlob);

template <typename T>
auto MGetBlobArrayRange(void* pBlob)
{
	auto* ArrayPtr = static_cast<u8*>(MGetBlobArrayPointer(pBlob));
	const auto ArraySize = MGetBlobArrayCount(pBlob);
	return Range<T*>{ ArrayPtr, ArrayPtr + ArraySize };
}

// Validates that the blob array's count info values
// exist and match the total blob array size.
bool MValidateBlobArraySize(const void* pBlob, size_t Size);

template <typename T, size_t N, size_t Alignment = sizeof(void*)>
struct StaticBlobArray
{
	static_assert(std::is_trivially_copyable<T>::value, "Illegal type");

	StaticBlobArray()
	{
		int OneBlobSize = sizeof(T);
		int BlobCount = N;
		memcpy(Buffer, &OneBlobSize, sizeof(OneBlobSize));
		memcpy(Buffer + sizeof(OneBlobSize), &BlobCount, sizeof(BlobCount));
	}

	void* Data() { return Buffer; }
	const void* Data() const { return Buffer; }
	size_t Size() const { return BufferSize; }

	T& Get(size_t Index)
	{
		return *reinterpret_cast<T*>(Buffer + sizeof(int) * 2 + sizeof(T) * Index);
	}

	const T& Get(size_t Index) const
	{
		return static_cast<const StaticBlobArray*>(this)->Get(Index);
	}

private:
	static constexpr auto BufferSize = sizeof(int) * 2 + sizeof(T) * N;
	alignas(Alignment) unsigned char Buffer[BufferSize];
};

struct BlobDeleter
{
	void operator()(void* Blob) const
	{
		MEraseBlobArray(Blob);
	}
};

using BlobPtr = std::unique_ptr<void, BlobDeleter>;

inline BlobPtr MMakeBlobArrayPtr(int OneBlobSize, int BlobCount)
{
	return BlobPtr{MMakeBlobArray(OneBlobSize, BlobCount)};
}