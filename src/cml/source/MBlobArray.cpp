#include "stdafx.h"
#include "MBlobArray.h"
#include <memory.h>
#include "GlobalTypes.h"
#include <cassert>

static const void* AddBytes(const void* ptr, int NumBytes) {
	return reinterpret_cast<const char*>(ptr) + NumBytes;
}
static void* AddBytes(void* ptr, int NumBytes) {
	return const_cast<void*>(AddBytes(ptr, NumBytes));
}

void* MMakeBlobArray(int nOneBlobSize, int nBlobCount)
{
	const auto Size = sizeof(nOneBlobSize) + sizeof(nBlobCount) + nOneBlobSize * nBlobCount;
	unsigned char* pBlob = new unsigned char[Size];
	memcpy(pBlob, &nOneBlobSize, sizeof(nOneBlobSize));
	memcpy(pBlob + sizeof(nBlobCount), &nBlobCount, sizeof(nOneBlobSize));
	return pBlob;
}

void MEraseBlobArray(void* pBlob)
{
	delete[] static_cast<unsigned char*>(pBlob);
}

const void* MGetBlobArrayElement(const void* pBlob, int i)
{
	int nBlobCount = 0;
	int nOneBlobSize = 0;
	memcpy(&nOneBlobSize, pBlob, sizeof(nOneBlobSize));
	memcpy(&nBlobCount, AddBytes(pBlob, sizeof(nOneBlobSize)), sizeof(nBlobCount));

	// Check if the index is within bounds
	if (i < 0 || i >= nBlobCount)
	{
		assert(false);
		return nullptr;
	}

	return AddBytes(pBlob, sizeof(int) * 2 + nOneBlobSize * i);
}

int MGetBlobArrayCount(const void* pBlob)
{
	i32 nBlobCount;
	memcpy(&nBlobCount, AddBytes(pBlob, sizeof(int)),
		sizeof(nBlobCount));
	return nBlobCount;
}

int MGetBlobArraySize(const void* pBlob)
{
	int nBlobCount, nOneBlobSize;
	memcpy(&nOneBlobSize, pBlob, sizeof(nOneBlobSize));
	memcpy(&nBlobCount, AddBytes(pBlob, sizeof(int)), sizeof(nBlobCount));

	return nOneBlobSize * nBlobCount + sizeof(int) * 2;
}

const void* MGetBlobArrayPointer(const void* pBlob)
{
	return AddBytes(pBlob, sizeof(int) * 2);
}

size_t MGetBlobArrayInfoSize()
{
	return sizeof(int) * 2;
}

size_t MGetBlobArrayElementSize(const void* pBlob)
{
	int nOneBlobSize;
	memcpy(&nOneBlobSize, pBlob, sizeof(nOneBlobSize));
	return nOneBlobSize;
}

bool MValidateBlobArraySize(const void* pBlob, size_t Size)
{
	// Size must at least contain the two info members.
	if (Size < 8)
		return false;

	const auto OneBlobSize = MGetBlobArrayElementSize(pBlob);
	const auto BlobCount = MGetBlobArrayCount(pBlob);

	return Size == MGetBlobArrayInfoSize() + OneBlobSize * BlobCount;
}