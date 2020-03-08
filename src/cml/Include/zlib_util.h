#pragma once

#include "zip/zlib.h"
#include "MDebug.h"
#include "MUtil.h"

struct IOResult
{
	int ErrorCode;
	const char* ErrorMessage;
	size_t BytesWritten;
	size_t BytesRead;
};

template <size_t BufferSize = 1024 * 16>
inline IOResult InflateFile(void* OutputBuffer, size_t OutputSize,
	FILE* File, size_t InputSize,
	int WindowBits = 15)
{
	Byte Buffer[BufferSize];
	z_stream Stream{};
	int err = inflateInit2(&Stream, WindowBits);
	if (err != Z_OK)
		return{ err, Stream.msg, 0, 0 };

	Stream.next_out = (Byte*)OutputBuffer;
	Stream.avail_out = OutputSize;

	auto InputLeft = InputSize;
	while (Stream.avail_out > 0)
	{
		if (Stream.avail_in == 0)
		{
			const auto BytesToRead = std::min(InputLeft, BufferSize);
			const auto BytesRead = fread(Buffer, 1, BytesToRead, File);
			if (BytesToRead != BytesRead)
			{
				return{ Z_ERRNO, "fread failed", Stream.total_out, Stream.total_in };
			}

			InputLeft -= BytesRead;
			Stream.next_in = Buffer;
			Stream.avail_in = BytesRead;
		}

		err = inflate(&Stream, Z_NO_FLUSH);
		if (err != Z_OK)
			break;
	}

	inflateEnd(&Stream);
	return{ err, Stream.msg, Stream.total_out, Stream.total_in };
}

inline IOResult InflateMemory(void* OutputBuffer, size_t OutputSize,
	const void* InputBuffer, size_t InputSize,
	int WindowBits = 15)
{
	z_stream Stream{};
	int err = inflateInit2(&Stream, WindowBits);
	if (err != Z_OK)
		return{ err, Stream.msg, 0, 0 };

	Stream.next_out = (Byte*)OutputBuffer;
	Stream.avail_out = OutputSize;
	Stream.next_in = (Byte*)InputBuffer;
	Stream.avail_in = InputSize;
	err = inflate(&Stream, Z_FINISH);

	inflateEnd(&Stream);

	return{ err, Stream.msg, Stream.total_out, Stream.total_in };
}