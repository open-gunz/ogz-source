#include "imgui.h"
#include "Patch.h"

struct GUIState
{
	bool GunzStarted{};
	std::chrono::steady_clock::time_point TimeGunzStarted;
	bool ShowSettings{};
	struct {
		bool StartGunzImmediatelyIfNoUpdate = true;
	} Settings;
};

enum class UnitType { B, KiB, MiB, GiB };

const char* UnitToString(UnitType Unit)
{
	const char* const Strings[] = { "B", "KiB", "MiB", "GiB" };
	return Strings[size_t(Unit)];
}

const u64 UnitSizes[] = {
	1,
	1024,
	1024 * 1024,
	1024 * 1024 * 1024,
};

template <size_t N>
inline void WriteNumber(char(&Output)[N], u64 Bytes, UnitType Unit)
{
	const auto ConvBytes = double(Bytes) / UnitSizes[size_t(Unit)];
	sprintf_safe(Output, "%.2f", ConvBytes);
}

inline UnitType GetUnit(u64 Bytes)
{
	size_t i;
	for (i = std::size(UnitSizes) - 1; i > 0; --i)
	{
		if (Bytes >= UnitSizes[i])
			break;
	}
	return UnitType(i);
}

struct RenderResult
{
	bool ShouldExit = false;
};

inline RenderResult Render(GUIState& State, const PatchExternalState& PatchState,
	const ImVec2& WindowSize)
{
	auto Flags = ImGuiWindowFlags_NoTitleBar  | ImGuiWindowFlags_NoResize
		       | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoFocusOnAppearing
		       | ImGuiWindowFlags_NoBringToFrontOnFocus;

	ImGui::SetNextWindowPos({0, 0});
	ImGui::SetNextWindowSize(WindowSize);
	ImGui::Begin("Main", nullptr, Flags);
	DEFER(ImGui::End);

	char TargetFileString[512];
	sprintf_safe(TargetFileString, "%.*s [%d/%d]",
		PatchState.TargetFile.size(), PatchState.TargetFile.data(),
		PatchState.FileIndex, PatchState.FileCount);

	auto ShowProgress = [&](const char* StatusName)
	{
		char BytesDone[32], BytesMissing[32], BytesPerSecond[32], TimeLeft[32];
		auto BytesMissingUnit = GetUnit(PatchState.BytesMissing);
		auto BytesPerSecondUnit = GetUnit(PatchState.BytesPerSecond);
		WriteNumber(BytesDone, PatchState.BytesDone, BytesMissingUnit);
		WriteNumber(BytesMissing, PatchState.BytesMissing, BytesMissingUnit);
		WriteNumber(BytesPerSecond, PatchState.BytesPerSecond, BytesPerSecondUnit);
		if (PatchState.BytesPerSecond == 0)
		{
			strcpy_literal(TimeLeft, "??:??");
		}
		else
		{
			auto BytesLeft = PatchState.BytesMissing - PatchState.BytesDone;
			int SecondsLeft = int(BytesLeft / PatchState.BytesPerSecond);
			int MinutesLeft = SecondsLeft / 60;
			int TruncatedSecondsLeft = SecondsLeft % 60;
			sprintf_safe(TimeLeft, "%02d:%02d", MinutesLeft, TruncatedSecondsLeft);
		}
		ImGui::Text("%s | %s\n%s/%s %s @ %s %s/s | %s left",
			TargetFileString, StatusName,
			BytesDone, BytesMissing, UnitToString(BytesMissingUnit),
			BytesPerSecond, UnitToString(BytesPerSecondUnit),
			TimeLeft);

		auto Progress = float(PatchState.BytesDone) / PatchState.BytesMissing;
		auto Size = ImVec2{ImGui::GetWindowWidth() - 100, 0};
		ImGui::ProgressBar(Progress, Size);
	};

	switch (PatchState.Status)
	{
	case PatchStatus::DownloadingPatchInfo:
	{
		ImGui::Text("Downloading patch info...");
	}
	break;
	case PatchStatus::Calculating:
	{
		if (!PatchState.TargetFile.empty())
		{
			ShowProgress("Calculating");
		}
		else
		{
			ImGui::Text("Calculating");
		}
	}
	break;
	case PatchStatus::Downloading:
	{
		ShowProgress("Downloading");
	}
	break;
	case PatchStatus::DownloadingSyncFile:
	{
		ShowProgress("Downloading metadata");
	}
	break;
	case PatchStatus::NoUpdate:
	case PatchStatus::Done:
	{
		if (!State.GunzStarted)
		{
			bool HasUpdated = PatchState.Status == PatchStatus::Done;
			if (HasUpdated)
			{
				ImGui::Text("Finished updating");
			}
			else
			{
				ImGui::Text("No update available");
			}
			
			auto StartGunz = [&] {
				Log.Info("Starting GunZ...\n");
				MProcess::Start("Gunz.exe");
				State.GunzStarted = true;
				State.TimeGunzStarted = std::chrono::steady_clock::now();
			};

			if (!HasUpdated && State.Settings.StartGunzImmediatelyIfNoUpdate)
			{
				StartGunz();
			}
			else
			{
				auto Mul = [](const ImVec4& Color, float Scale) {
					return ImVec4{
						Color.x * Scale,
						Color.y * Scale,
						Color.z * Scale,
						Color.w * Scale,
					};
				};
				ImVec4 Color = Mul(ImColor(0x40, 0xE0, 0xD0), 0.75f);
				ImGui::PushStyleColor(ImGuiCol_Button, Color);
				ImGui::PushStyleColor(ImGuiCol_ButtonHovered, Mul(Color, 1.2f));
				ImGui::PushStyleColor(ImGuiCol_ButtonActive, Mul(Color, 1.4f));
				ImVec2 Size = {150, 80};
				ImGui::PushItemWidth(150);
				ImGui::SetCursorPosX(WindowSize.x / 2 - Size.x / 2);
				if (ImGui::Button("Start GunZ", Size))
				{
					StartGunz();
				}
				ImGui::PopStyleColor(3);
			}
		}
		else
		{
			using namespace std::chrono;
			auto Delta = steady_clock::now() - State.TimeGunzStarted;
			auto TimeLeft = seconds(3) - Delta;
			auto SecondsLeft = duration_cast<duration<float>>(TimeLeft);
			if (SecondsLeft <= seconds(0))
			{
				RenderResult Res;
				Res.ShouldExit = true;
				return Res;
			}
			int IntegralSeconds = static_cast<int>(std::ceil(SecondsLeft.count()));
			ImGui::Text("GunZ started! Exiting in %d seconds...", IntegralSeconds);
		}
	}
	break;
	case PatchStatus::FatalError:
	{
		auto&& s = PatchState.ErrorMessage;
		ImGui::PushTextWrapPos();
		ImGui::Text("Fatal error: %.*s", s.size(), s.data());
		ImGui::PopTextWrapPos();
	}
	break;
	}

	return {};
}