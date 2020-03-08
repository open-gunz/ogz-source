// # Sync.h
//
// This file contains functions implementing a file synchronization algorithm similar to that of rsync.
//
// # Algorithm
//
// In the following, the "server" is defined as the party having the correct version of the file,
// and the "client" is defined as the party with the incorrect file, wishing to synchronize their
// version with the server's.
//
// When the server is generating the patch files, it also generates a .sync file for each file that
// is larger than the block size.
//
// To generate a .sync file, the server divides a file into blocks according to a constant block
// size value (the last block may be shorter), and creates a .sync file containing a table of block
// hashes. The table has one entry for each block, where each entry consists of two hashes: A weak
// rolling hash, and a strong hash. These are stored tightly packed together as binary data.
//
// ## .sync file example
//
// Suppose the file to be synchronized is 250 bytes large and the block size is 100 bytes. The file
// will thus be divided into 3 blocks.
//
// Suppose also that the rolling hash is 4 bytes large, and the strong hash is 16 bytes large.
//
// Then, the .sync file will be (4 + 16) * 3 = 60 bytes large, and will contain:
//
// +----------+-----------------------------------------------+
// | Size     | Description                                   |
// +----------+-----------------------------------------------+
// | 4 bytes  | Rolling hash for first block (bytes 0-100)    |
// | 16 bytes | Strong hash for first block                   |
// | 4 bytes  | Rolling hash for second block (bytes 100-200) |
// | 16 bytes | Strong hash for second block                  |
// | 4 bytes  | Rolling hash for third block (bytes 200-250)  |
// | 16 bytes | Strong hash for third block                   |
// +----------+-----------------------------------------------+
//
// ## Delta computation
//
// When the client consumes the .sync file, it constructs a hash map that maps rolling hashes to
// data about the block that the hash corresponds to.
//
// The client then computes the rolling hash of every block in the file at every possible offset
// (not just multiples of the block size). That is, at every index in the list
// (0, 1, 2, ..., file size - block size), the file computes the rolling hash of the block at that
// offset in the file (the bytes from the index to the index + block size).
//
// If the rolling hash matches a hash in the remote file, the client then also computes the strong
// hash of the block, in order to confirm that it is actually the correct one, since the rolling
// hash is very weak and prone to collisions.
// If the strong hash also matches, we set the block data to indicate that it can be found in the
// local file at that offset.
//
// When the file is reconstructed, the client iterates over the list of blocks, grabbing blocks
// from the local file if they were found, or downloading new blocks if not.
//
// To download blocks, the HTTP range request header is used. Thus, the webserver serving the files
// must support it (the feature is optional).
//
// ## Hash algorithms
//
// Two hash algorithms are used: A weak but fast rolling hash, and a strong but expensive hash for
// verification.
//
// They are defined in Hash.h. Sync.h does not depend on any particular implementation.
//
// Rolling hash functions are hash functions specifically designed to be able to be cheaply updated
// on the basis of 1) the last hash, 2) the bytes removed from the window, and 3) the bytes added
// to the window. Since this hash function is very weak, it's not sufficient to verify blocks;
// instead, it is only there to rapidly discard blocks that definitely aren't correct.
// 
// ## Comparison with rsync
//
// In rsync, the client calculates the table of block hashes and sends it to the server, and the
// server is tasked with figuring out the differences.
//
// The algorithm implemented here, on the other hand, has the server produce the hash-table, and
// the client calculate differences. This lets the server serve only static files (since the
// hash-table can be computed beforehand), avoiding stuff like special daemons and letting you
// host the files anywhere static files can be placed.
//
// The main disadvantage of this is that there will usually be more data downloaded. For example,
// inserting 1 byte into the file would entail a 1 byte + metadata delta file, whereas with this,
// it must redownload the entire surrounding block.

#pragma once

#include "Sync.h"

#include "GlobalTypes.h"
#include "SafeString.h"
#include "MFile.h"
#include "MUtil.h"

#include "Hash.h"
#include "LauncherConfig.h"
#include "Log.h"
#include "Download.h"
#include "File.h"

#include <vector>
#include <unordered_map>
#include <algorithm>
using std::min;
using std::max;

namespace Sync
{

const auto BlockSize = LauncherConfig::BlockSize;

// A block in the remote file.
// Each of these are LauncherConfig::BlockSize byte large.
// If the last block is smaller than that, it's represented as a LastRemoteBlock instead of as a
// RemoteBlock.
//
// This struct doesn't contain any information about the block's position in the remote file.
// This is instead inferred from its position in the list it's in.
struct RemoteBlock
{
	Hash::Rolling RollingHash;
	Hash::Strong StrongHash;

	// The local file offset at which this block is located.
	// If it wasn't found in the local file, this value is -1.
	// This is set to -1 when the list of remote blocks is created, and set to something else only
	// when the search loop finds a match.
	u64 LocalFileOffset;
};

// The very last remote block.
// This also has a size member since the last remote block can be smaller than normal.
// If the last block is not smaller than normal, this is empty.
struct LastRemoteBlock : RemoteBlock
{
	bool Empty;
	u32 Size;
};

struct RemoteFile
{
	// A contiguous list of remote blocks.
	// Each block is located at i * LauncherConfig::BlockSize in the remote file, where i is its
	// index in this list.
	std::vector<RemoteBlock> RemoteBlocks;

	// Maps rolling hashes to indices into the above list.
	// Due to how weak the rolling hash is, each value might map to multiple blocks. Therefore,
	// the map is a multimap.
	std::unordered_multimap<Hash::Rolling, size_t> RollingHashToBlockMap;

	// The total size of the remote file.
	u64 Size;

	// The total size of the blocks that need to be downloaded to synchronize the local file.
	u64 UnmatchingSize;

	LastRemoteBlock LastBlock;

	RemoteBlock& MapValueToBlock(size_t Index)
	{
		if (Index == size_t(-1))
			return LastBlock;

		return RemoteBlocks[Index];
	}
};

// Same as ceil(float(a) / b), except without involving floats.
template <typename T1, typename T2>
constexpr auto ceildiv(T1 a, T2 b)
{
	return a / b + int(a % b > 0);
}

static bool DownloadSyncFile(RemoteFile& Remote, const char* SyncFileURL,
	DownloadManagerType& DownloadManager,
	function_view<ProgressCallbackType> ProgressCallback)
{
	// Each entry contains one rolling hash value and one strong hash value.
	static constexpr auto EntrySize = Hash::Rolling::Size + Hash::Strong::Size;

	if (ProgressCallback)
		ProgressCallback(StatusType::DownloadingSyncFile, 0, 0);

	// An incomplete entry left over from a previous call to the Callback.
	// This would occur when the content length is not a multiple of the entry size.
	u8 PartialData[EntrySize];
	size_t PartialDataSize = 0;

	size_t EntriesProcessed = 0;
	const auto NumEntries = ceildiv(Remote.Size, BlockSize);
	
	// Deserializes a remote block starting at the Begin address and adds it to Remote.
	auto AddRemoteBlock = [&](const u8* Begin)
	{
		RemoteBlock* NewBlock;
		size_t MapValue;
		if (!Remote.LastBlock.Empty && EntriesProcessed == NumEntries - 1)
		{
			NewBlock = &Remote.LastBlock;
			MapValue = size_t(-1);
		}
		else
		{
			NewBlock = &emplace_back(Remote.RemoteBlocks);
			MapValue = NewBlock - Remote.RemoteBlocks.data();
		}

		const auto RollingHashSrc = Begin;
		memcpy(&NewBlock->RollingHash.Value, RollingHashSrc, sizeof(NewBlock->RollingHash.Value));

		const auto StrongHashSrc = RollingHashSrc + Hash::Rolling::Size;
		memcpy(&NewBlock->StrongHash.Value, StrongHashSrc, sizeof(NewBlock->StrongHash.Value));

		Remote.RollingHashToBlockMap.emplace(NewBlock->RollingHash, MapValue);

		NewBlock->LocalFileOffset = -1;

		++EntriesProcessed;
	};

	auto Callback = [&](const u8* Buffer, size_t Size, DownloadInfo& Info)
	{
		LOG_DEBUG("Callback invoked -- Buffer = %p, Size = %zu\n",
			Buffer, Size);

		if (PartialDataSize > 0)
		{
			const auto MissingSize = EntrySize - PartialDataSize;
			memcpy(PartialData + PartialDataSize, Buffer, min(MissingSize, Size));
			if (Size < MissingSize)
			{
				PartialDataSize += min(MissingSize, Size);
				return true;
			}

			AddRemoteBlock(PartialData);
			Buffer += MissingSize;
			Size -= MissingSize;
		}

		PartialDataSize = Size % EntrySize;
		const auto UsableDataSize = Size - PartialDataSize;
		if (PartialDataSize > 0)
		{
			memcpy(PartialData, Buffer + UsableDataSize, PartialDataSize);
		}

		for (int i = 0; i < int(Size / EntrySize); ++i)
		{
			AddRemoteBlock(Buffer + i * EntrySize);
		}

		return true;
	};

	auto ProgressCallbackWrapper = [&](size_t DTotal, size_t DNow)
	{
		return ProgressCallback(StatusType::DownloadingSyncFile, DTotal, DNow);
	};

	function_view<::ProgressCallbackType> ProgressCallbackArg;
	if (ProgressCallback)
		ProgressCallbackArg = ProgressCallbackWrapper;

	const auto ret = DownloadFile(DownloadManager, SyncFileURL, LauncherConfig::PatchPort,
		std::ref(Callback), ProgressCallbackArg);

	if (Remote.LastBlock.Empty)
		assert(Remote.RemoteBlocks.size() == NumEntries);
	else
		assert(Remote.RemoteBlocks.size() == NumEntries - 1);

	assert(PartialDataSize == 0);

	return ret;
}

// Calculates the relation between a local file and a remote file,
// filling out a list of Blocks with the result.
static bool CalculateBlocks(Memory& memory, RemoteFile& Remote, BlockCounts& Counts,
	const char* LocalFilePath,
	function_view<ProgressCallbackType> ProgressCallback)
{
	using namespace detail;

	if (ProgressCallback)
		ProgressCallback(StatusType::CalculatingBlocks, 0, 0);

	MFile::File LocalFile{ LocalFilePath };
	if (LocalFile.error())
	{
		Log(LogLevel::Error, "Failed to open file %s\n", LocalFilePath);
		return false;
	}

	const auto LocalFileSize = LocalFile.size();
	assert(!LocalFile.error());

	if (LocalFileSize == 0)
	{
		Counts.UnmatchingBlocks = Remote.RemoteBlocks.size() + size_t(!Remote.LastBlock.Empty);
		Counts.MatchingBlocks = 0;
		Remote.UnmatchingSize = Remote.Size;
		return true;
	}

	// This is used as a circular buffer, so arithmetic on indices into it should be wrapped with
	// Wrap.
	constexpr auto FileBufferSize = Memory::FileBufferSize;
	const auto FileBuffer = memory.FileBuffer.get();

	// Wraps an index a such that it fits within the bounds of FileData.
	auto Wrap = [&](int a) {
		return mod(a, FileBufferSize);
	};

	const auto InitialReadSize = size_t(std::min(LocalFileSize, u64(FileBufferSize)));
	const auto BytesRead = LocalFile.read(FileBuffer, InitialReadSize);
	if (BytesRead != InitialReadSize)
	{
		Log(LogLevel::Error, "Failed to read bytes 0 to %zu from file %s\n",
			InitialReadSize, LocalFilePath);
		return false;
	}

	// Most types from here on just use int for index and size types, which is ok because the
	// FileData buffer couldn't reasonably be larger than that.

	// The index of the start of the window relative to FileBuffer.
	// The window is BlockSize bytes large, so the end index is Wrap(WindowStartIndex + BlockSize).
	int WindowStartIndex = 0;

	// The index of the start of the window relative to the file.
	u64 WindowStartFileIndex = 0;

	// The number of bytes skipped over by the window, that haven't yet been consumed.
	int SkippedBytes = 0;

	// Stores the last index at which data was loaded from the file.
	int LastLoadEndIndex = InitialReadSize - 1;

	size_t NumFoundBlocks = 0;

	// Attempts to read new data into the range given.
	// If the indices are the same, it refreshes the entire buffer.
	// Returns the actual number of bytes read.
	// Only used in AdvanceWindow.
	auto Read = [&](int StartIndex, int OnePastEndIndex)
	{
		LOG_DEBUG(4, "Reading %d-%d, LocalFile.tell() = %llu\n",
			StartIndex, OnePastEndIndex, LocalFile.tell());

		assert(StartIndex >= 0 && StartIndex < FileBufferSize);
		assert(OnePastEndIndex >= 0 && OnePastEndIndex <= FileBufferSize);

		const auto Size = StartIndex == OnePastEndIndex ? FileBufferSize : Wrap(OnePastEndIndex - StartIndex);

		const auto FirstReadSize = min(Size, int(FileBufferSize) - StartIndex);
		const auto ActualFirstReadSize = LocalFile.read(&FileBuffer[StartIndex], FirstReadSize);
		if (ActualFirstReadSize != FirstReadSize)
		{
			if (LocalFile.error())
			{
				Log.Error("CalculateBlocks -- File IO error on %s\n", LocalFilePath);
			}

			LastLoadEndIndex = Wrap(StartIndex + (ActualFirstReadSize - 1));

			LOG_DEBUG(4,
				"FirstReadSize = %d\n"
				"ActualFirstReadSize = %d\n"
				"LastLoadEndIndex = %d\n",
				FirstReadSize,
				ActualFirstReadSize,
				LastLoadEndIndex);

			return int(ActualFirstReadSize);
		} 

		auto ret = FirstReadSize;

		const auto SecondReadSize = int(Size) - (int(FileBufferSize) - StartIndex);
		if (SecondReadSize > 0)
		{
			const auto ActualSecondReadSize = LocalFile.read(&FileBuffer[0], SecondReadSize);
			ret += ActualSecondReadSize;
			if (ActualSecondReadSize != SecondReadSize)
			{
				if (LocalFile.error())
				{
					Log.Error("CalculateBlocks -- File IO error on %s\n", LocalFilePath);
				}

				LastLoadEndIndex = Wrap(0 + (ActualSecondReadSize - 1));

				LOG_DEBUG(4,
					"SecondReadSize = %d\n"
					"ActualSecondReadSize = %d\n"
					"ret = %d\n",
					"LastLoadEndIndex = %d\n",
					SecondReadSize,
					ActualSecondReadSize,
					ret,
					LastLoadEndIndex);

				return ret;
			}
		}

		LastLoadEndIndex = Wrap(OnePastEndIndex - 1);

		LOG_DEBUG(4,
			"ret = %d\n"
			"LastLoadEndIndex = %d\n",
			ret,
			LastLoadEndIndex);

		return ret;
	};

	// Attempts to advance the window by AdvanceOffset bytes, reading in more data if necessary.
	// Returns the actual number of bytes advanced by, which is less than requested if the end of
	// the file was reached or if there was an IO error.
	// If this happens, we can't advance again.
	auto AdvanceWindow = [&](int AdvanceOffset)
	{
		// Visual for BlockSize = 4, OldWindowStartIndex = 0,
		// LastLoadEndIndex = 5, AdvanceOffset = 2.
		//
		// [ x | x | - | - | - | - | x | x ]
		//       ^   ^       ^   ^   ^
		//       |   |       |   |   WindowEndIndex
		//       |   |       |   LastLoadEndIndex
		//       |   |       OldWindowEndIndex
		//       |   WindowStartIndex
		//       LastLoadStartIndex
		//
		// Bytes marked x should be updated.

		assert(AdvanceOffset > 0 && AdvanceOffset <= BlockSize);

		const auto OldWindowStartIndex = WindowStartIndex;
		WindowStartIndex = Wrap(WindowStartIndex + AdvanceOffset);

		const auto OldWindowEndIndex = Wrap(OldWindowStartIndex + BlockSize - 1);
		const auto WindowEndIndex = Wrap(WindowStartIndex + BlockSize - 1);

		// We want to read new data in if the window end moved past the last load end.
		// That is, if the old window end was behind or equal to the last load end, and the new
		// window end is ahead of it.

		// Move LoadEnd and WindowEnd so that they're relative to OldWindowEnd.
		const auto OldWindowEndToLoadEnd = Wrap(LastLoadEndIndex - OldWindowEndIndex);
		const auto WindowEndToOldWindowEnd = AdvanceOffset;

		int ret;

		if (OldWindowEndToLoadEnd >= 0 && OldWindowEndToLoadEnd < WindowEndToOldWindowEnd)
		{
			LOG_DEBUG(4, "AdvanceWindow -- Reading\n"
				"OldWindowStartIndex = %d, WindowStartIndex = %d\n"
				"OldWindowEndIndex = %d, WindowEndIndex = %d\n"
				"OldWindowEndToLoadEnd = %d, WindowEndToOldWindowEnd = %d\n"
				"LastLoadEndIndex = %d\n",
				OldWindowStartIndex, WindowStartIndex,
				OldWindowEndIndex, WindowEndIndex,
				OldWindowEndToLoadEnd, WindowEndToOldWindowEnd,
				LastLoadEndIndex);

			const auto NumBytesRead = Read(Wrap(LastLoadEndIndex + 1), WindowStartIndex);
			const auto MaxAdvance = NumBytesRead + OldWindowEndToLoadEnd;
			const auto ActualAdvance = min(AdvanceOffset, MaxAdvance);

			LOG_DEBUG(4, "NumBytesRead = %d\n"
				"MaxAdvance = %d\n"
				"ActualAdvance = %d\n",
				NumBytesRead,
				MaxAdvance,
				ActualAdvance);

			ret = ActualAdvance;
			if (ret < AdvanceOffset)
				WindowStartIndex -= AdvanceOffset - ret;

			if (ProgressCallback)
			{
				ProgressCallback(Sync::CalculatingBlocks, LocalFileSize,
					WindowStartFileIndex + ret);
			}
		}
		else
		{
			ret = AdvanceOffset;
		}

		WindowStartFileIndex += ret;

		return ret;
	};

	auto HashBlock = [&](auto&& Stream, auto&& HashOutput, int StartIndex, int Size)
	{
		const auto EndIndex = Wrap(StartIndex + BlockSize);

		const auto FirstSrc = &FileBuffer[StartIndex];
		const auto FirstSize = min(Size, int(FileBufferSize) - StartIndex);
		Stream.Update(FirstSrc, FirstSize);

		const auto SecondSrc = &FileBuffer[0];
		const auto SecondSize = Size - (int(FileBufferSize) - StartIndex);
		if (SecondSize > 0)
			Stream.Update(SecondSrc, SecondSize);

		Stream.Final(HashOutput);
	};

	auto RollingHashBlock = [&](auto&& HashOutput, int StartIndex, int Size = BlockSize) {
		 HashBlock(Hash::Rolling::Stream{}, HashOutput, StartIndex, Size);
	};

	auto StrongHashBlock = [&](auto&& HashOutput, int StartIndex, int Size = BlockSize) {
		HashBlock(Hash::Strong::Stream{}, HashOutput, StartIndex, Size);
	};

	// Adds the final block when the end of the file has been reached.
	auto OnReachedEnd = [&]
	{
		if (Remote.LastBlock.Empty)
			return;

		const auto LastBlockStartIndex = LastLoadEndIndex + 1 - Remote.LastBlock.Size;

		Hash::Strong StrongHash;
		StrongHashBlock(StrongHash, LastBlockStartIndex, Remote.LastBlock.Size);

		if (StrongHash != Remote.LastBlock.StrongHash)
			return;

		Remote.LastBlock.LocalFileOffset = LocalFileSize - Remote.LastBlock.Size;

		LOG_DEBUG(4, "Last block found at local file offset %llu\n", Remote.LastBlock.LocalFileOffset);
	};

	auto SetCounts = [&]
	{
		bool LastBlockFound = Remote.LastBlock.LocalFileOffset != -1;

		if (!LastBlockFound)
			LOG_DEBUG(4, "Last block was not found\n");

		Counts.MatchingBlocks = NumFoundBlocks +
			size_t(LastBlockFound && !Remote.LastBlock.Empty);
		Counts.UnmatchingBlocks = Remote.RemoteBlocks.size() - NumFoundBlocks + 
			size_t(!LastBlockFound && !Remote.LastBlock.Empty);
		Remote.UnmatchingSize = (Remote.RemoteBlocks.size() - NumFoundBlocks) * BlockSize +
			(LastBlockFound ? 0 : Remote.LastBlock.Size);
	};

	Hash::Rolling RollingHash;

	if (InitialReadSize < BlockSize)
	{
		if (InitialReadSize >= Remote.LastBlock.Size)
			OnReachedEnd();
		SetCounts();
		return true;
	}

	RollingHash.HashMemory(FileBuffer, BlockSize);

	while (true)
	{
#ifdef _DEBUG
		char RollingString[Hash::Rolling::MinimumStringSize];
		RollingHash.ToString(RollingString);
		Log.Debug(5, "WindowStartIndex = %llu -- Rolling = %s\n",
			WindowStartFileIndex, RollingString);
#endif

		// Check if the hash from the current window matches a block in the remote file.
		auto&& Map = Remote.RollingHashToBlockMap;
		auto Range = MakeRange(Map.equal_range(RollingHash));
		if (Range.first != Map.end())
		{
			LOG_DEBUG("Rolling hash matches a block\n");

			bool StrongHashed = false;
			Hash::Strong StrongHash;

			bool Found = false;

			for (auto&& Pair : Range)
			{
				auto&& Block = Remote.MapValueToBlock(Pair.second);
				if (Block.LocalFileOffset != -1)
					continue;

				if (!StrongHashed)
				{
					StrongHashBlock(StrongHash, WindowStartIndex);
					StrongHashed = true;
				}

				if (Block.StrongHash != StrongHash)
					continue;

				++NumFoundBlocks;

				Block.LocalFileOffset = WindowStartFileIndex;

				LOG_DEBUG(4, "Block value %zu found at local file offset %llu\n",
					Pair.second, Block.LocalFileOffset);

				Found = true;
			}

			if (Found)
			{
				// Jump ahead by one block on match, since we probably won't be able to find new
				// matches inside it.
				const auto Advanced = AdvanceWindow(BlockSize);
				if (Advanced != BlockSize)
				{
					OnReachedEnd();
					break;
				}
				SkippedBytes = 0;

				RollingHashBlock(RollingHash, WindowStartIndex);

				continue;
			}
			else
			{
				LOG_DEBUG(4, "False positive\n");
			}
		}

		// Advance the window.
		const auto OldByte = FileBuffer[WindowStartIndex];

		const auto Advanced = AdvanceWindow(1);
		if (Advanced != 1)
		{
			OnReachedEnd();
			break;
		}

		const auto WindowEndIndex = Wrap(WindowStartIndex + BlockSize - 1);
		const auto NewByte = FileBuffer[WindowEndIndex];

		RollingHash.Move(OldByte, NewByte, BlockSize);
	}

	SetCounts();

	return true;
}

template <size_t OutputSize>
static void MakeNewFilePath(char(&OutputFilePath)[OutputSize], const char* LocalFilePath)
{
	sprintf_safe(OutputFilePath, "%s_new", LocalFilePath);
}

static bool CreateNewFile(const char* LocalFilePath,
	const char* SynchronizedFilePath,
	const char* RemoteFileURL,
	DownloadManagerType& DownloadManager,
	const RemoteFile& Remote,
	function_view<ProgressCallbackType> ProgressCallback,
	Hash::Strong* HashOutput,
	u64* SizeOutput)
{
	if (ProgressCallback)
		ProgressCallback(StatusType::DownloadingFile, 0, 0);

	MFile::RWFile OutputFile{ SynchronizedFilePath, MFile::Clear};
	if (OutputFile.error())
	{
		Log(LogLevel::Error, "Sync::SynchronizeFile -- Could not open file %s for writing\n",
			SynchronizedFilePath);
		return false;
	}

	MFile::File InputFile{ LocalFilePath };
	if (InputFile.error())
	{
		Log(LogLevel::Error, "Sync::SynchronizeFile -- Could not open file %s for reading\n",
			LocalFilePath);
		return false;
	}

	Hash::Strong::Stream Hash;

	if (SizeOutput)
		*SizeOutput = 0;

	Log.Debug("Blocks.size() = %zu, Remote.LastBlock.Empty = %d\n",
		Remote.RemoteBlocks.size(), Remote.LastBlock.Empty);

	const u64 DLTotal = Remote.UnmatchingSize;
	u64 DLedSoFar = 0;
	u64 TotalSize = 0;

	auto HandleBlock = [&](const RemoteBlock& Block, size_t ThisBlockSize,
		u64 OutputOffset) // The offset in the remote/new file.
	{
#ifdef _DEBUG
		char RollingHashString[Hash::Rolling::MinimumStringSize];
		Block.RollingHash.ToString(RollingHashString);
		char StrongHashString[Hash::Strong::MinimumStringSize];
		Block.StrongHash.ToString(StrongHashString);

		Log.Debug(4, "Handling block\n"
			"RollingHash = %s\n"
			"StrongHash = %s\n"
			"LocalFileOffset = %llu\n"
			"Size = %zu\n",
			RollingHashString,
			StrongHashString,
			Block.LocalFileOffset,
			ThisBlockSize);
#endif

		if (Block.LocalFileOffset != -1)
		{
			InputFile.seek(Block.LocalFileOffset, MFile::Seek::Begin);

			char InputBuffer[BlockSize];
			const auto NumBytesRead = InputFile.read(InputBuffer, ThisBlockSize);

			OutputFile.write(InputBuffer, NumBytesRead);

			if (HashOutput)
				Hash.Update(InputBuffer, NumBytesRead);

			if (InputFile.error())
			{
				Log.Error("Sync::SynchronizeFile -- Failed to read %llu bytes from file %s\n",
					ThisBlockSize, LocalFilePath);
				return false;
			}
		}
		else
		{
			size_t DownloadedBlockSize = 0;
			Hash::Strong::Stream DownloadedBlockHashStream;

			auto Callback = [&](const void* Buffer, size_t Size, DownloadInfo& Info)
			{
				Log.Debug(4, "CreateNewFile -- Callback invoked with Buffer = %p, Size = %zu\n",
					Buffer, Size);

				if (!Info.IsRange())
				{
					assert(false);
					return false;
				}

				DownloadedBlockSize += Size;
				DownloadedBlockHashStream.Update(Buffer, Size);

				if (HashOutput)
					Hash.Update(Buffer, Size);

				OutputFile.write(Buffer, Size);

				return true;
			};

			auto ProgressCallbackWrapper = [&](size_t BlockDLTotal, size_t BlockDLNow)
			{
				ProgressCallback(StatusType::DownloadingFile, DLTotal, DLedSoFar + BlockDLNow);
			};

			const auto StartOffset = OutputOffset;
			// The range is inclusive so we need to subtract one.
			const auto EndOffset = StartOffset + ThisBlockSize - 1;

			char Range[64];
			sprintf_safe(Range, "%llu-%llu", StartOffset, EndOffset);

			Log.Debug(3, "CreateNewFile -- Calling DownloadFile with URL = %s, "
				"Port = %d, Range = %s\n",
				RemoteFileURL,
				LauncherConfig::PatchPort, Range);

			function_view<::ProgressCallbackType> ProgressCallbackArg;
			if (ProgressCallback)
				ProgressCallbackArg = ProgressCallbackWrapper;

			DownloadFile(DownloadManager, RemoteFileURL, LauncherConfig::PatchPort,
				std::ref(Callback), ProgressCallbackArg, Range);

			Hash::Strong DownloadedBlockHash;
			DownloadedBlockHashStream.Final(DownloadedBlockHash);

			const auto WrongSize = DownloadedBlockSize != ThisBlockSize;
			const auto WrongHash = DownloadedBlockHash != Block.StrongHash;
			if (WrongSize || WrongHash)
			{
				Log.Error("Downloaded block integrity fail\n");

				if (WrongSize)
				{
					Log.Error("Expected size %zu, got %zu\n", ThisBlockSize, DownloadedBlockSize);
				}
				if (WrongHash)
				{
					char ExpectedHashString[Hash::Strong::MinimumStringSize];
					Block.StrongHash.ToString(ExpectedHashString);
					char ActualHashString[Hash::Strong::MinimumStringSize];
					DownloadedBlockHash.ToString(ActualHashString);

					Log.Error("Expected hash %s, got %s\n", ExpectedHashString, ActualHashString);
				}

				return false;
			}

			DLedSoFar += ThisBlockSize;
		}

		TotalSize += ThisBlockSize;

		return true;
	};

	for (size_t i = 0; i < Remote.RemoteBlocks.size(); ++i)
	{
		auto&& Block = Remote.RemoteBlocks[i];

		const auto OutputOffset = i * BlockSize;
		if (!HandleBlock(Block, BlockSize, OutputOffset))
		{
			return false;
		}
	}

	if (!Remote.LastBlock.Empty)
	{
		if (!HandleBlock(Remote.LastBlock, Remote.LastBlock.Size,
			u64(Remote.RemoteBlocks.size()) * BlockSize))
		{
			return false;
		}
	}

	if (HashOutput)
		Hash.Final(*HashOutput);

	if (SizeOutput)
		*SizeOutput = TotalSize;

	return true;
}

bool MakeSyncFile(const char* OutputFilePath, const char* InputFilePath)
{
	MFile::CreateParentDirs(OutputFilePath);

	MFile::RWFile OutputFile{ OutputFilePath, MFile::Clear};
	if (OutputFile.error())
	{
		Log(LogLevel::Error, "Sync::MakeSyncFile -- Could not open file %s for writing\n",
			OutputFilePath);
		return false;
	}

	MFile::File InputFile{ InputFilePath };
	if (InputFile.error())
	{
		Log(LogLevel::Error, "Sync::MakeSyncFile -- Could not open file %s for reading\n",
			InputFilePath);
		return false;
	}

	const auto InputFileSize = InputFile.size();
	assert(!InputFile.error());

	const auto NumBlocks = ceildiv(InputFileSize, BlockSize);

	Log.Debug("NumBlocks = %llu\n", NumBlocks);

	u8 InputBuffer[BlockSize];
	for (u64 i = 0; i < NumBlocks; ++i)
	{
		// Read one block from the file.
		constexpr auto NumBytesToTryRead = sizeof(InputBuffer);
		const auto NumBytesRead = InputFile.read(InputBuffer, NumBytesToTryRead);
		if (NumBytesRead != NumBytesToTryRead && i != NumBlocks - 1)
		{
			Log(LogLevel::Error, "Sync::MakeSyncFile -- Failed to read %zu bytes from file %s\n",
				NumBytesToTryRead, InputFilePath);
			return false;
		}

		// Compute and write weak rolling hash.
		{
			Hash::Rolling RollingHash;
			RollingHash.HashMemory(InputBuffer, NumBytesRead);
			OutputFile.write(&RollingHash.Value, sizeof(RollingHash.Value));
			if (OutputFile.error())
			{
				Log(LogLevel::Error, "Sync::MakeSyncFile -- Failed to write %zu bytes to file %s\n",
					sizeof(RollingHash.Value), OutputFilePath);
				return false;
			}
		}

		// Compute and write strong hash.
		{
			Hash::Strong StrongHash;
			StrongHash.HashMemory(InputBuffer, NumBytesRead);
			OutputFile.write(&StrongHash.Value, sizeof(StrongHash.Value));
			if (OutputFile.error())
			{
				Log(LogLevel::Error, "Sync::MakeSyncFile -- Failed to write %zu bytes to file %s\n",
					sizeof(StrongHash.Value), OutputFilePath);
				return false;
			}
		}
	}

	return true;
}

SyncResult SynchronizeFile(Memory& memory,
	const char* LocalFilePath,
	const char* OutputFilePath,
	const char* RemoteFileURL,
	const char* SyncFileURL,
	u64 RemoteFileSize,
	DownloadManagerType& DownloadManager,
	function_view<ProgressCallbackType> ProgressCallback,
	Hash::Strong* HashOutput,
	u64* SizeOutput,
	BlockCounts* BlockCountsOutput)
{
	if (OutputFilePath == nullptr)
		OutputFilePath = LocalFilePath;

	using namespace detail;

	Log.Debug("Downloading sync file\n");

	RemoteFile Remote;

	Remote.Size = RemoteFileSize;
	Remote.LastBlock.Size = Remote.Size % BlockSize;
	Remote.LastBlock.Empty = Remote.LastBlock.Size == 0;
	Remote.LastBlock.LocalFileOffset = -1;

	Log.Debug("Remote.LastBlock.Size = %u\n", Remote.LastBlock.Size);

	auto Success = DownloadSyncFile(Remote, SyncFileURL, DownloadManager, ProgressCallback);
	if (!Success)
	{
		return {false, strprintf("Failed to download sync file \"%s\"", SyncFileURL)};
	}

	Log.Debug("Remote.RemoteBlocks.size() = %zu\n",
		Remote.RemoteBlocks.size());

#ifdef _DEBUG
	for (size_t i = 0; i < Remote.RemoteBlocks.size(); ++i)
	{
		auto&& Block = Remote.RemoteBlocks[i];
		char RollingString[Hash::Rolling::MinimumStringSize];
		Block.RollingHash.ToString(RollingString);
		char StrongString[Hash::Strong::MinimumStringSize];
		Block.StrongHash.ToString(StrongString);
		Log.Debug(4, "Block %zu: Rolling = %s, Strong = %s\n",
			i, RollingString, StrongString);
	}

	int i = 0;
	for (auto&& Pair : Remote.RollingHashToBlockMap)
	{
		auto&& Key = Pair.first;
		auto&& Value = Pair.second;
		auto&& Block = Remote.MapValueToBlock(Value);
		char KeyString[Hash::Rolling::MinimumStringSize];
		Key.ToString(KeyString);
		char RollingString[Hash::Rolling::MinimumStringSize];
		Block.RollingHash.ToString(RollingString);
		char StrongString[Hash::Strong::MinimumStringSize];
		Block.StrongHash.ToString(StrongString);
		Log.Debug(4, "Map block %d: Key = %s, Rolling = %s, Strong = %s\n",
			i, KeyString, RollingString, StrongString);
		++i;
	}
#endif

	Log.Debug("Calculating blocks\n");

	BlockCounts Counts;

	Success = CalculateBlocks(memory, Remote, Counts, LocalFilePath, ProgressCallback);
	if (!Success)
	{
		return {false, strprintf("Failed to calculate blocks for file \"%s\"", LocalFilePath)};
	}

	Log.Info("%zu unmatching blocks, %zu matching blocks, %zu unmatching size\n",
		Counts.UnmatchingBlocks, Counts.MatchingBlocks, Remote.UnmatchingSize);

	if (BlockCountsOutput)
		*BlockCountsOutput = Counts;

#ifdef _DEBUG
	auto PrintBlock = [](size_t i, const RemoteBlock& Block) {
		Log.Debug("Block %zu (%llu): Block.LocalFileOffset = %llu\n",
			i, u64(i) * BlockSize, Block.LocalFileOffset);
	};
	for (size_t i = 0; i < Remote.RemoteBlocks.size(); ++i)
	{
		PrintBlock(i, Remote.RemoteBlocks[i]);
	}
	PrintBlock(Remote.RemoteBlocks.size(), Remote.LastBlock);
#endif

	Log.Debug("Creating new file\n");

	char SynchronizedFilePath[MFile::MaxPath];
	MakeNewFilePath(SynchronizedFilePath, LocalFilePath);

	Success = CreateNewFile(LocalFilePath,
		SynchronizedFilePath,
		RemoteFileURL,
		DownloadManager,
		Remote, ProgressCallback,
		HashOutput, SizeOutput);
	if (!Success)
	{
		return {false, strprintf("Failed to create new file \"%s\" from "
			"remote \"%s\"", LocalFilePath, RemoteFileURL)};
	}

	// Delete the output file if it exists and move the synchronized file over the output file.
	if (MFile::Exists(OutputFilePath))
	{
		if (!MFile::Delete(OutputFilePath))
		{
			Log.Error("Sync::SynchronizeFile -- Failed to delete existing output file %s\n", LocalFilePath);
		}
	}

	if (!MFile::Move(SynchronizedFilePath, OutputFilePath))
	{
		return {false, strprintf("Failed to move synchronized file \"%s\" "
			"to output file path \"%s\"", SynchronizedFilePath, OutputFilePath)};
	}

	return {true, ""};
}

}