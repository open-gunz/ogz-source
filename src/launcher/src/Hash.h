// # Hash.h
//
// # Strong hash
//
// BLAKE2b-256.
//
// # Rolling hash
//
// The rolling hash is the same as the one used in rsync: A 32-bit value where the lower 16 bits
// are the sum of all the bytes in the sequence, and the upper 16 bits is the sum of all the
// partial sums of the previous process.
//
// I.e.:
//
//     a(n, m) = sum(X_i for i in n to m) mod M
//     b(n, m) = sum(a(k, i) for i in n to m) mod M
//             = sum((m - i + 1) * X_i for i in n to m) mod M
//     hash(n, m) = a(k, l) + 2^16 * b(k, l)
//
// where X is the file data, M = 2^16, and hash(n, m) is the rolling hash of bytes n through m
// (inclusive and 1-indexed) in the file.
//
// This is the same as Adler-32, except that Adler-32 uses M = 65521 (the lowest prime under 2^16)
// and adds 1 to a (and therefore also to b).

#pragma once

#include "Log.h"
#include <fstream>
#include "ArrayView.h"
#include "MFile.h"
#include "SafeString.h"
#include "sodium.h"

namespace Hash
{

namespace detail
{

inline void bin2str(ArrayView<char> Output, const u8* Data, size_t Size)
{
	assert(Output.size() >= Size * 2 + 1 && "Output is too small!");

	auto* CurOutput = Output.data();
	auto RemainingSize = Output.size();
	for (size_t i = 0; i < Size; ++i)
	{
		// sprintf_safe null-terminates at every step,
		// so we don't need to do that after the loop.
		sprintf_safe(CurOutput, RemainingSize, "%02X", Data[i]);
		CurOutput += 2;
		RemainingSize -= 2;
	}
}

template <typename HashType>
inline bool HashFile(HashType& Output, const char* Filename)
{
	MFile::File File{ Filename };

	if (File.error())
	{
		Log.Error("Failed to open %s for reading\n", Filename);
		return false;
	}

	unsigned char InputBuffer[16 * 1024];

	typename HashType::Stream Stream;

	while (true)
	{
		const auto NumBytesRead = File.read(InputBuffer, sizeof(InputBuffer));

		Stream.Update(InputBuffer, NumBytesRead);

		if (NumBytesRead != sizeof(InputBuffer))
			break;
	}

	if (File.error())
	{
		Log.Error("HashFile -- fread failed with error code\n");
		return false;
	}

	Stream.Final(Output);

	return true;
}

}

// BLAKE2b-256 hash.
struct Strong
{
	// 256 bit digest length.
	static constexpr auto Size = 256 / CHAR_BIT;

	// This is binary data, NOT a string. Use ToString to get a readable form.
	u8 Value[Size];

	// Need two characters for every byte, along with a null terminator.
	static constexpr auto MinimumStringSize = Size * 2 + 1;

	void ToString(ArrayView<char> Output) const {
		detail::bin2str(Output, Value, Size);
	}

	inline void HashMemory(const void* Buffer, size_t Size)
	{
		crypto_generichash_blake2b(
			// Out
			Value, sizeof(Value),
			// In
			static_cast<const u8*>(Buffer), Size,
			// Key
			nullptr, 0);
	}

	inline bool HashFile(const char* Filename)
	{
		return detail::HashFile(*this, Filename);
	}

	struct Stream
	{
		crypto_generichash_blake2b_state state;

		Stream() {
			crypto_generichash_blake2b_init(&state,
				nullptr, 0, // Key
				Size);
		}

		void Update(const void* Buffer, size_t Size) {
			crypto_generichash_blake2b_update(&state,
				static_cast<const u8*>(Buffer), Size);
		}

		void Final(Strong& Output) {
			crypto_generichash_blake2b_final(&state,
				Output.Value, Output.Size);
		}
	};
};

inline bool operator==(const Strong& lhs, const Strong& rhs) {
	return !memcmp(lhs.Value, rhs.Value, lhs.Size);
}

inline bool operator!=(const Strong& lhs, const Strong& rhs) {
	return !(lhs == rhs);
}

// Modified Adler-32
struct Rolling
{
	u32 Value = 0;
	static constexpr auto Size = sizeof(u32);
	static constexpr auto MinimumStringSize = Size * 2 + 1;

	void ToString(ArrayView<char> Output) const {
		detail::bin2str(Output, reinterpret_cast<const u8*>(&Value), Size);
	}

	void HashMemory(const void* Buffer, size_t Size)
	{
		const auto ByteBuffer = static_cast<const u8*>(Buffer);

		u16 Lower = 0;
		u16 Upper = 0;

		for (size_t i = 0; i < Size; ++i)
		{
			Lower += ByteBuffer[i];
			Upper += Lower;
		}

		Value = (u32(Upper) << 16) | Lower;
	}

	inline bool HashFile(const char* Filename)
	{
		return detail::HashFile(*this, Filename);
	}

	void Move(u8 LastByte, u8 NextByte, size_t Size)
	{
		// The recurrence relations are:
		// a(n + 1, m + 1) = (a(n, m) - X_n + X_(l + 1)) mod M
		// b(n + 1, m + 1) = (b(n, m) - (m - n + 1) * X_n + a(n + 1, m + 1)) mod M

		const auto OldLower = Value & 0xFFFF;
		const auto OldUpper = Value >> 16;

		const auto NewLower = u16(OldLower - LastByte + NextByte);
		const auto NewUpper = u16(OldUpper - Size * LastByte + NewLower);

		Value = (u32(NewUpper) << 16) | NewLower;
	}

	struct Stream
	{
		u32 Value = 0;
		u64 CurSize = 0;

		void Update(const void* Buffer, size_t Size)
		{
			const auto ByteBuffer = static_cast<const u8*>(Buffer);

			const auto OldLower = u16(Value & 0xFFFF);
			const auto OldUpper = u16(Value >> 16);

			auto NewLower = OldLower;
			auto NewUpper = OldUpper;

			for (size_t i = 0; i < Size; ++i)
			{
				NewLower += ByteBuffer[i];
				NewUpper += NewLower;
			}

			Value = (u32(NewUpper) << 16) | NewLower;
		}

		void Move(u8 LastByte, u8 NextByte, size_t Size)
		{
			// The recurrence relations are:
			// a(n + 1, m + 1) = (a(n, m) - X_n + X_(l + 1)) mod M
			// b(n + 1, m + 1) = (b(n, m) - (m - n + 1) * X_n + a(n + 1, m + 1)) mod M

			const auto OldLower = Value & 0xFFFF;
			const auto OldUpper = Value >> 16;

			const auto NewLower = u16(OldLower - LastByte + NextByte);
			const auto NewUpper = u16(OldUpper - Size * LastByte + NewLower);

			Value = (u32(NewUpper) << 16) | NewLower;
		}

		void Final(Rolling& Output) {
			Output.Value = Value;
		}
	};
};

inline bool operator==(const Rolling& lhs, const Rolling& rhs) {
	return lhs.Value == rhs.Value;
}

inline bool operator!=(const Rolling& lhs, const Rolling& rhs) {
	return !(lhs == rhs);
}

}

// Specialize std::hash for Hash::Rolling so that we can use it in hashmaps.
namespace std {
template <>
struct hash<Hash::Rolling> {
	size_t operator()(const Hash::Rolling& x) const {
		// Since we're already dealing with a hash, we just return that, no hashing needed.
		return x.Value;
	}
};
}