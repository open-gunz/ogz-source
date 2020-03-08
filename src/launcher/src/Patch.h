#pragma once

#ifdef _MSC_VER
#define _ENABLE_ATOMIC_ALIGNMENT_FIX
#endif
#include <atomic>
#include <thread>
#include <chrono>
#include "LauncherConfig.h"
#include "XML.h"
#include "Download.h"
#include "MFile.h"
#include "File.h"
#include "Sync.h"
#include "SafeString.h"
#include "FileCache.h"
#include "defer.h"
#include "MInetUtil.h"
#include "MProcess.h"
#include "CmdlineArgs.h"
#include "StringAllocator.h"
#include "RingBuffer.h"
#include <chrono>

enum class PatchStatus
{
	DownloadingPatchInfo,
	NoUpdate,
	DownloadingSyncFile,
	Calculating,
	Downloading,
	Done,
	FatalError,
};

struct PatchExternalState
{
	PatchStatus Status;
	StringView TargetFile;
	u32 FileIndex;
	u32 FileCount;
	u64 BytesMissing;
	u64 BytesDone;
	u64 BytesPerSecond;
	StringView ErrorMessage;
};

template <typename T>
struct relaxed_atomic
{
	relaxed_atomic() = default;
	relaxed_atomic(T x) : a{x} {}
	void operator=(T x) { a.store(x, std::memory_order_relaxed); }
	T load() const { return a.load(std::memory_order_relaxed); }
	std::atomic<T> a;
};

struct PatchInternalState
{
	template <typename T>
	using A = relaxed_atomic<T>;

	A<PatchStatus> Status = PatchStatus::DownloadingPatchInfo;
	A<StringView> TargetFile = "";
	A<u32> FileIndex = 0;
	A<u32> FileCount = 0;
	A<u64> BytesMissing = 0;
	A<u64> BytesDone = 0;
	A<double> BytesPerSecond = 0;
	A<StringView> ErrorMessage = "";

	XMLFile PatchXML;
	static constexpr auto DownloadSpeedSampleTime = std::chrono::milliseconds(100);
	static constexpr auto MaxSamples = 50;
	std::string ErrorMessageMemory;
	optional<Sync::Memory> SyncMemory;
	u32 Samples = 0;
	bool CanSync = true;

	void SetErrorMessage(std::string Str)
	{
		if (ErrorMessage.load() != "")
			abort();
		ErrorMessageMemory = std::move(Str);
		std::atomic_thread_fence(std::memory_order_release);
		ErrorMessage = ErrorMessageMemory;
	}

	PatchExternalState Load() const
	{
		PatchExternalState Ret;
		std::atomic_thread_fence(std::memory_order_acquire);
#define COPY(Name) Ret.Name = Name.load();
		COPY(Status);
		COPY(TargetFile);
		COPY(FileIndex);
		COPY(FileCount);
		COPY(BytesMissing);
		COPY(BytesDone);
		Ret.BytesPerSecond = u64(BytesPerSecond.load());
		COPY(ErrorMessage);
#undef COPY
		return Ret;
	}
};

template <size_t OutputSize>
void GetPatchFileURL(char(&Output)[OutputSize], const StringView& Path, bool Client = true)
{
	char URLEncodedPath[4096];
	URLEncode(URLEncodedPath, Path);

	sprintf_safe(Output, "%s/patch/%s%s",
		LauncherConfig::PatchDomain,
		Client ? "client/" : "",
		URLEncodedPath);
}

template <size_t OutputSize>
void GetSyncFileURL(char(&Output)[OutputSize], const StringView& Path)
{
	char URLEncodedPath[4096];
	URLEncode(URLEncodedPath, Path);

	sprintf_safe(Output, "%s/patch/sync/%s.sync",
		LauncherConfig::PatchDomain,
		URLEncodedPath);
}

inline bool GetPatchXML(XMLFile& PatchXML, const DownloadManagerType& DownloadManager)
{
	Log(LogLevel::Info, "Downloading patch.xml\n");

	const auto& PatchXMLFilename = "patch.xml";

	char URL[1024];
	GetPatchFileURL(URL, PatchXMLFilename, false);

	return PatchXML.CreateFromDownload(DownloadManager,
		URL, LauncherConfig::PatchPort,
		PatchXMLFilename);
}

inline bool DownloadWholeFile(const DownloadManagerType& DownloadManager,
	const char* Filename, const char* OutputPath = nullptr,
	function_view<ProgressCallbackType> ProgressCallback = {},
	DownloadError* ErrorOutput = nullptr,
	Hash::Strong* HashOutput = nullptr, u64* SizeOutput = nullptr)
{
	using namespace std::literals;

	if (OutputPath == nullptr)
		OutputPath = Filename;

	Log.Info("Downloading whole new %s\n", Filename);

	char URL[1024];
	GetPatchFileURL(URL, Filename);

	if (MFile::Exists(OutputPath))
	{
		if (!MFile::Delete(OutputPath))
		{
			Log.Error("Failed to delete file \"%s\"\n", OutputPath);
		}
	}

	MFile::RWFile File{OutputPath, MFile::Clear};
	if (File.error())
	{
		if (ErrorOutput)
		{
			sprintf_safe(ErrorOutput->String, "Couldn't open file \"%s\" for writing", OutputPath);
		}
		return false;
	}

	Hash::Strong::Stream Stream;
	if (SizeOutput)
	{
		*SizeOutput = 0;
	}

	const auto Callback = [&](const void* Buffer, size_t Size, DownloadInfo&)
	{
		if (HashOutput)
		{
			Stream.Update(Buffer, Size);
		}

		if (SizeOutput)
		{
			*SizeOutput += Size;
		}

		auto ret = File.write(Buffer, Size);
		return ret == Size;
	};

	auto ret = DownloadFile(DownloadManager, URL, LauncherConfig::PatchPort, std::ref(Callback),
		ProgressCallback, nullptr, ErrorOutput);

	if (HashOutput)
	{
		Stream.Final(*HashOutput);
	}

	return ret;
}

struct NeedsUpdateResult
{
	bool NeedsUpdate = true;
	bool FileExistsLocally = false;
	char Hash[Hash::Strong::MinimumStringSize];
	u64 Size = 0;
	std::string ErrorMessage;
};

inline NeedsUpdateResult NeedsUpdate(const char* Filename,
	const StringView& WantedHashString,
	u64 WantedSize,
	FileCacheType& FileCache)
{
	assert(Filename != nullptr);

	NeedsUpdateResult Ret;
	strcpy_literal(Ret.Hash, "");
	StringView ActualHashString;

	const auto CachedData = FileCache.GetCachedFileData(Filename);
	Ret.FileExistsLocally = MFile::Exists(Filename);

	if (!Ret.FileExistsLocally)
	{
		Log.Info("%s nonexistent\n", Filename);
		Ret.NeedsUpdate = true;
		return Ret;
	}
	else
	{
		bool FoundInCache = CachedData.Result == FileQueryResult::Found;
		if (FoundInCache)
		{
			strcpy_safe(Ret.Hash, CachedData.FileData->Hash);
			Ret.Size = CachedData.FileData->Size;
			ActualHashString = Ret.Hash;
			Ret.NeedsUpdate = Ret.Size != WantedSize || ActualHashString != WantedHashString;
			return Ret;
		}
		else
		{
			switch (CachedData.Result)
			{
			case FileQueryResult::NotFound:
				Log.Info("%s not found in cache\n", Filename);
			break;
			case FileQueryResult::Outdated:
				Log.Info("%s changed since last run\n", Filename);
			break;
			case FileQueryResult::Error:
				Log.Error("Error while retrieving cached data for %s\n", Filename);
			break;
			}

			{
				auto MaybeSize = MFile::Size(Filename);
				if (!MaybeSize)
				{
					Log.Error("Failed to get size of existing file %s\n", Filename);
				}
				else
				{
					Ret.Size = *MaybeSize;
					if (Ret.Size != WantedSize)
					{
						Ret.NeedsUpdate = true;
						return Ret;
					}
				}
			}

			Hash::Strong ActualHash;
			const auto HashSuccess = ActualHash.HashFile(Filename);
			if (!HashSuccess)
			{
				Ret.NeedsUpdate = false;
				Ret.ErrorMessage = "Failed to compute hash";
				return Ret;
			}

			ActualHash.ToString(Ret.Hash);
			ActualHashString = Ret.Hash;

			FileCache.Add(Filename, ActualHashString);
			
			// We don't need to check the size here since we already tried to do that earlier.
			Ret.NeedsUpdate = WantedHashString != ActualHashString;
			return Ret;
		}
	}
}

struct PatchFileResult
{
	bool Success;
	std::string ErrorMessage;
};

inline PatchFileResult PatchFile(PatchInternalState& State,
	DownloadManagerType& DownloadManager,
	const char* Filename,
	const char* OutputFilename,
	const StringView& WantedHashString,
	u64 WantedSize,
	bool FileExistsLocally,
	FileCacheType& FileCache)
{
	assert(Filename != nullptr);
	if (OutputFilename == nullptr)
		OutputFilename = Filename;

	State.TargetFile = Filename;

	MFile::CreateParentDirs(OutputFilename);

	Hash::Strong NewHash;
	u64 NewFileSize;

	char FileURL[4096];
	GetPatchFileURL(FileURL, Filename);

	auto GetTime = [] { return std::chrono::steady_clock::now(); };
	auto LastStep = GetTime();
	u64 LastStepDLNow{};
	State.BytesPerSecond = 0;

	auto UpdateProgress = [&](u64 DLTotal, u64 DLNow, bool StateChange = false) {
		State.BytesMissing = DLTotal;
		State.BytesDone = DLNow;

		if (StateChange)
		{
			LastStep = GetTime();
			LastStepDLNow = DLNow;
			State.BytesPerSecond = 0;
			State.Samples = 0;
			return;
		}

		using namespace std::chrono;
		const auto Now = GetTime();
		if (Now - LastStep < State.DownloadSpeedSampleTime)
		{
			return;
		}
		const auto StepDLDelta = DLNow - LastStepDLNow;
		const auto TimeDelta = Now - LastStep;
		const auto TimeFractionalPart = TimeDelta % State.DownloadSpeedSampleTime;
		const auto TimeIntegralPart = TimeDelta - TimeFractionalPart;
		const auto StepDLDeltaForIntegral = StepDLDelta * TimeIntegralPart / TimeDelta;
		const auto StepDLDeltaForFractional = StepDLDelta - StepDLDeltaForIntegral;
		const auto NumSamples = TimeIntegralPart / State.DownloadSpeedSampleTime;
		const auto DLDeltaPerStep = StepDLDeltaForIntegral / NumSamples;
		const auto NewSample = double(DLDeltaPerStep * (seconds(1) / State.DownloadSpeedSampleTime));
		double Avg = State.BytesPerSecond.load();
		for (int i = 0; i < int(NumSamples); ++i)
		{
			if (State.Samples < State.MaxSamples)
				++State.Samples;
			const auto f = 1.0 / State.Samples;
			Avg = f * NewSample + (1 - f) * Avg;
		}
		State.BytesPerSecond = Avg;
		LastStepDLNow = DLNow - size_t(StepDLDeltaForFractional);
		LastStep += TimeIntegralPart;
	};

	if (FileExistsLocally && State.CanSync)
	{
		char SyncFileURL[4096];
		GetSyncFileURL(SyncFileURL, Filename);

		auto ProgressCallback = [&](Sync::StatusType Status, u64 DTotal, u64 DNow)
		{
			auto OldStatus = State.Status.load();
			auto NewStatus = [&] {
				switch (Status)
				{
				case Sync::StatusType::DownloadingSyncFile:
					return PatchStatus::DownloadingSyncFile;
				case Sync::StatusType::CalculatingBlocks:
					return PatchStatus::Calculating;
				case Sync::StatusType::DownloadingFile:
					return PatchStatus::Downloading;
				}
				assert(false);
				return PatchStatus::Calculating;
			}();
			State.Status = NewStatus;
			UpdateProgress(DTotal, DNow, NewStatus != OldStatus);
		};

		if (!State.SyncMemory)
		{
			State.SyncMemory.emplace();
		}

		const auto Result = Sync::SynchronizeFile(*State.SyncMemory, Filename,
			OutputFilename,
			FileURL, SyncFileURL,
			WantedSize,
			DownloadManager, std::ref(ProgressCallback),
			&NewHash, &NewFileSize);
		if (!Result.Success)
		{
			Log.Error("Failed to synchronize file!\n"
				"Error message: \"%s\"\n"
				"Local file path: \"%s\"\n"
				"Remote file URL: \"%s\"\n"
				"Sync file URL: \"%s\"\n",
				Result.ErrorMessage.c_str(), Filename, FileURL, SyncFileURL);
			return {false, std::move(Result.ErrorMessage)};
		}
	}
	else
	{
		State.Status = PatchStatus::Downloading;

		DownloadError Error;

		const auto Success = DownloadWholeFile(DownloadManager,
			Filename, OutputFilename, std::ref(UpdateProgress),
			&Error, &NewHash, &NewFileSize);
		if (!Success)
		{
			Log.Error("Downloading new file \"%s\" from URL \"%s\" failed. Error message: \"%s\"",
				Filename, FileURL, Error.String);
			return {false, Error.String};
		}
	}

	char HashStringMemory[Hash::Strong::MinimumStringSize];
	NewHash.ToString(HashStringMemory);
	StringView ActualHashString = HashStringMemory;

	Log.Debug("NewFileSize = %llu, NewHash = %s\n",
		NewFileSize, ActualHashString.data());

	const auto WrongHash = WantedHashString != ActualHashString;
	const auto WrongSize = WantedSize != NewFileSize;
	if (WrongHash || WrongSize)
	{
		return {false, strprintf("Downloaded file \"%s\" is corrupt\n"
			"Expected hash: %.*s, size: %llu\n"
			"Actual hash:   %.*s, size: %llu",
			Filename,
			WantedHashString.size(), WantedHashString.data(), WantedSize,
			ActualHashString.size(), ActualHashString.data(), NewFileSize)};
	}

	FileCache.Add(Filename, ActualHashString);
	return {true, ""};
}

// Deletes any temporary files left over from a previous run of the program.
inline void DeleteResidualTemporaryFiles()
{
	using namespace std::chrono_literals;

	// launcher_new.exe should not normally exist at this point, but if it does, delete it anyway.
	if (MFile::Exists("launcher_new.exe"))
	{
		Log(LogLevel::Debug, "Deleting launcher_new.exe\n");

		if (!MFile::Delete("launcher_new.exe"))
			Log(LogLevel::Error, "DeleteFile failed on launcher_new.exe!\n");
	}

	if (MFile::Exists("launcher_old.exe"))
	{
		// Sleep so that the old launcher instance has a chance to close.
		std::this_thread::sleep_for(100ms);

		Log(LogLevel::Debug, "Deleting launcher_old.exe\n");

		if (!MFile::Delete("launcher_old.exe"))
			Log(LogLevel::Error, "DeleteFile failed on launcher_old.exe!\n");
	}
}

enum class SelfUpdateResultType
{
	Updated,
	NoUpdateAvailable,
	Failed,
};

struct SelfUpdateResult
{
	SelfUpdateResultType Result;
	std::string ErrorMessage;
};

inline SelfUpdateResult SelfUpdate(PatchInternalState& State,
	DownloadManagerType& DownloadManager,
	const XMLFile& PatchXML, rapidxml::xml_node<>& files_node,
	FileCacheType& FileCache)
{
	auto&& Range = GetFileNodeRange(files_node);

	// Find the node for the launcher.xml file.
	auto it = std::find_if(std::begin(Range), std::end(Range), [&](auto&& node) {
		return iequals(PatchXML.GetName(node), LauncherConfig::LauncherFilename);
	});

	if (it == std::end(Range))
	{
		return {SelfUpdateResultType::Failed, "Failed to find launcher.exe in patch.xml"};
	}

	auto&& node = *it;
	auto Filename = PatchXML.GetName(node);
	auto WantedHash = PatchXML.GetHash(node, Filename);
	auto Size = PatchXML.GetSize(node, Filename);
	{
		const bool MissingValues[] = {Filename.empty(), WantedHash.empty(), !Size.has_value()};
		const char* const MissingNames[] = {"filename", "hash", "size"};
		auto is_true = [](bool x) { return x; };
		if (std::any_of(std::begin(MissingValues), std::end(MissingValues), is_true))
		{
			std::string MissingString;
			bool First = true;
			for (size_t i = 0; i < std::size(MissingValues); ++i)
			{
				if (!MissingValues[i])
					continue;
				if (!First)
					MissingString += ", ";
				MissingString += MissingNames[i];
				First = false;
			}
			return {SelfUpdateResultType::Failed,
				strprintf("Malformed patch.xml: Missing %s for launcher.exe",
					MissingString.c_str())};
		}
	}

	auto Res = NeedsUpdate(Filename.data(), WantedHash, Size.value(), FileCache);

	if (!Res.NeedsUpdate)
		return {SelfUpdateResultType::NoUpdateAvailable, ""};

	auto TryPatch = [&] {
		return PatchFile(State, DownloadManager, Filename.data(), "launcher_new.exe",
			WantedHash, Size.value(), Res.FileExistsLocally, FileCache);
	};

	auto PatchResult = TryPatch();

	if (!PatchResult.Success)
	{
		// If synchronization failed, try again without sync.
		State.CanSync = false;
		PatchResult = TryPatch();
		if (!PatchResult.Success)
		{
			return {SelfUpdateResultType::Failed, std::move(PatchResult.ErrorMessage)};
		}
	}

#ifdef _DEBUG
	// Don't self-update under debug.
	Log(LogLevel::Debug, "Ignoring available update\n");
	return {SelfUpdateResultType::NoUpdateAvailable, ""};
#endif

	MFile::Move("launcher.exe", "launcher_old.exe");
	MFile::Move("launcher_new.exe", "launcher.exe");
	MProcess::Start("launcher.exe");
	return {SelfUpdateResultType::Updated, ""};
}

inline void Patch(PatchInternalState& State, const Options& Opt)
{
	auto StartTime = std::chrono::steady_clock::now();

	auto Fatal = [&](std::string Msg)
	{
		State.Status = PatchStatus::FatalError;
		State.SetErrorMessage(std::move(Msg));
		Log.Fatal("%s\n", State.ErrorMessageMemory.c_str());
	};

	State.Status = PatchStatus::DownloadingPatchInfo;

	auto DownloadManager = CreateDownloadManager();
	if (!DownloadManager)
	{
		Fatal("Failed to initialize download manager");
		return;
	}

	Log(LogLevel::Debug, "Created download manager!\n");

	DeleteResidualTemporaryFiles();

	// The patch.xml file's memory must be in scope for the rest of the program,
	// since references to parts of it are retained in many places.
	if (!GetPatchXML(State.PatchXML, DownloadManager))
	{
		Fatal("Failed to load patch.xml file");
		return;
	}

	std::atomic_thread_fence(std::memory_order_release);

	FileCacheType FileCache;
	FileCache.Load();

	auto* files = State.PatchXML.Doc.first_node("files");
	if (!files)
	{
		Fatal("Failed to find files node in patch.xml");
		return;
	}

	if (!Opt.IgnoreSelfUpdate)
	{
		auto SelfUpdateResult = SelfUpdate(State, DownloadManager, State.PatchXML, *files,
			FileCache);

		switch (SelfUpdateResult.Result)
		{
		case SelfUpdateResultType::Updated:
			Log(LogLevel::Info, "Self-updated! Starting new launcher and terminating\n");
			exit(0);

		case SelfUpdateResultType::NoUpdateAvailable:
			Log(LogLevel::Info, "No self-update available\n");
			break;

		case SelfUpdateResultType::Failed:
			Fatal(strprintf("Can't self-update (%s)", SelfUpdateResult.ErrorMessage.c_str()));
			return;
		}
	}

	State.Status = PatchStatus::Calculating;

	struct OutdatedFile
	{
		StringView Filename;
		bool FileExistsLocally;
		StringView WantedHash;
		StringView ActualHash;
		u64 WantedSize;
		u64 ActualSize;
		char ActualHashMemory[Hash::Strong::MinimumStringSize];
	};
	std::vector<OutdatedFile> FilesToUpdate;

	for (auto&& node : GetFileNodeRange(*files))
	{
		auto Filename = State.PatchXML.GetName(node);
		if (Filename.empty() || Filename == LauncherConfig::LauncherFilename)
			continue;

		auto ExpectedHash = State.PatchXML.GetHash(node, Filename);
		auto ExpectedSize = State.PatchXML.GetSize(node, Filename);
		if (ExpectedHash.empty() || !ExpectedSize.has_value())
			continue;

		State.TargetFile = Filename;
		auto ret = NeedsUpdate(Filename.data(), ExpectedHash, ExpectedSize.value(), FileCache);

		if (!ret.ErrorMessage.empty())
		{
			Fatal(strprintf("Failed to check if file \"%s\" needs to be updated\nError: %s",
				Filename.data(), ret.ErrorMessage.c_str()));
			return;
		}

		if (!ret.NeedsUpdate)
			continue;

		FilesToUpdate.emplace_back();
		auto&& File = FilesToUpdate.back();
		File.Filename = Filename.data();
		File.FileExistsLocally = ret.FileExistsLocally;
		strcpy_safe(File.ActualHashMemory, ret.Hash);
		File.WantedHash = ExpectedHash;
		File.ActualHash = File.ActualHashMemory;
		File.WantedSize = ExpectedSize.value();
		File.ActualSize = ret.Size;
	}
	
	State.FileCount = FilesToUpdate.size();

	for (size_t FileIndex = 0, End = FilesToUpdate.size(); FileIndex < End; ++FileIndex)
	{
		State.FileIndex = FileIndex + 1;
		auto&& File = FilesToUpdate[FileIndex];
		State.TargetFile = File.Filename;
		Log.Info("Patching file...\n"
			"Name: %s\n"
			"Wanted hash:  %.*s, size: %llu\n",
			File.Filename.data(),
			File.WantedHash.size(), File.WantedHash.data(), File.WantedSize);
		if (File.FileExistsLocally)
		{
			StringView ActualHash = File.ActualHash.empty() ?
				"(Not computed because size differs)" : File.ActualHash;
			Log.Info("Current hash: %.*s, size: %llu\n",
				ActualHash.size(), ActualHash.data(), File.ActualSize);
		}
		else
		{
			Log.Info("Current file nonexistent\n");
		}

		auto ret = PatchFile(State, DownloadManager, File.Filename.data(), nullptr,
			File.WantedHash, File.WantedSize, File.FileExistsLocally, FileCache);

		if (!ret.Success)
		{
			Fatal(strprintf("Failed to patch file \"%s\"\nError: %s", File.Filename.data(),
				ret.ErrorMessage.c_str()));
			return;
		}
	}

	FileCache.Save();

	State.Status = FilesToUpdate.size() != 0 ? PatchStatus::Done : PatchStatus::NoUpdate;

	auto EndTime = std::chrono::steady_clock::now();
	using namespace std::chrono;
	double Seconds = duration_cast<duration<double>>(EndTime - StartTime).count();
	Log.Info("Finished patching in %.16g seconds\n", Seconds);
}