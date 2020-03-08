#include "MUtil.h"
#include "Log.h"

struct Options
{
	bool IgnoreSelfUpdate{};
};

struct HandleArgumentsResult
{
	bool Success;
	std::string ErrorMessage;
};

template <typename T>
HandleArgumentsResult HandleArguments(Options& Opt, const T& Args)
{
	// Start i at 1 because argv[0] is the program name.
	auto argc = int(Args.size());
	for (int i = 1; i < argc; ++i)
	{
		static const StringView VerbosityOpt = "--verbosity=";
		static const StringView IgnoreSelfUpdateOpt = "--ignore-self-update";

		auto& Arg = Args[i];

		auto Check = [&](auto&& String) {
			return iequals(Arg, String);
		};
		
		auto CheckSubset = [&](auto&& String) {
			if (Arg.size() < String.size())
				return false;
			return strncmp(Arg.data(), String.data(), String.size()) == 0;
		};

		if (CheckSubset(VerbosityOpt))
		{
			auto VerbosityValue = Arg.substr(VerbosityOpt.size());
			auto MaybeVerbosity = StringToInt<int>(VerbosityValue);
			if (!MaybeVerbosity.has_value())
			{
				return {false, strprintf("Invalid --verbosity option value. "
					"Expected integral value, got \"%.*s\"\n",
					VerbosityValue.size(), VerbosityValue.data())};
			}

			Log.DebugVerbosity = *MaybeVerbosity;
		}
		else if (Check(IgnoreSelfUpdateOpt))
		{
			Opt.IgnoreSelfUpdate = true;
		}
		else
		{
			return {false, strprintf("Unknown option %.*s\n", Arg.size(), Arg.data())};
		}
	}

	return {true, {}};
}