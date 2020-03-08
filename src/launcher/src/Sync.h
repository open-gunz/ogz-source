#pragma once

#include <string>
#include <memory>
#include "GlobalTypes.h"
#include "Download.h"
#include "LauncherConfig.h"
#include "function_view.h"

struct PatchInternalState;
namespace Hash { struct Strong; }

namespace Sync
{

bool MakeSyncFile(const char* OutputFilePath, const char* InputFilePath);

struct BlockCounts
{
	size_t MatchingBlocks;
	size_t UnmatchingBlocks;
};

enum StatusType
{
	DownloadingSyncFile,
	CalculatingBlocks,
	DownloadingFile,
};

using ProgressCallbackType = void(StatusType Status, u64 DTotal, u64 DNow);

struct SyncResult
{
	bool Success;
	std::string ErrorMessage;
};

struct Memory
{
	static constexpr auto FileBufferSize = int(4 * LauncherConfig::BlockSize);
	std::unique_ptr<u8[]> FileBuffer{new u8[FileBufferSize]};
};

SyncResult SynchronizeFile(Memory&,
	const char* LocalFilePath,
	const char* OutputFilePath,
	const char* RemoteFileURL,
	const char* SyncFileURL,
	u64 RemoteFileSize,
	DownloadManagerType& DownloadManager,
	function_view<ProgressCallbackType> ProgressCallback = {},
	Hash::Strong* HashOutput = nullptr,
	u64* SizeOutput = nullptr,
	BlockCounts* BlockCountsOutput = nullptr);

}