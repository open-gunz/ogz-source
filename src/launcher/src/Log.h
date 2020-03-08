#pragma once

#include "SafeString.h"
#include "MFile.h"
#include "defer.h"
#include <cstdarg>
#include <cassert>

// This macro is here since MSVC does not seem willing to optimize out a normal call, even though
// the first line of Logger::Debug is a return statement when _DEBUG is off.
#ifdef _DEBUG
#define LOG_DEBUG(...) Log.Debug(__VA_ARGS__)
#else
#define LOG_DEBUG(...) (void)0
#endif

enum class LogLevel
{
	Debug,
	Info,
	Warning,
	Error,
	Fatal,
};

namespace LogTo
{
enum Type : u32
{
	Stdout = 1 << 0,
	File = 1 << 1,
	Debugger = 1 << 2,
};
}

struct Logger
{
	int DebugVerbosity = -1;
	// 0 = base
	// 4 = tight loops
	// 5 = very tight loops

	LogTo::Type Dest;

	bool Init(const char* LogFilePath, u32 Dest)
	{
		this->Dest = LogTo::Type(Dest);
		if (!(Dest & LogTo::File))
			return true;
		return LogFile.open(LogFilePath, MFile::Clear);
	}

	bool Init(const char* LogFilePath)
	{
		auto DefaultDest = LogTo::Stdout | LogTo::File;
#ifdef _DEBUG
		DefaultDest |= LogTo::Debugger;
#endif
		return Init(LogFilePath, DefaultDest);
	}

#define LOGGER_LOG_FUNCTION(Level)\
	void Level(const char* Format, ...){\
		va_list va;\
		va_start(va, Format);\
		LogVA(LogLevel::Level, Format, va);\
		va_end(va);\
	}

	LOGGER_LOG_FUNCTION(Info);
	LOGGER_LOG_FUNCTION(Warning);
	LOGGER_LOG_FUNCTION(Error);
	LOGGER_LOG_FUNCTION(Fatal);

#undef LOGGER_LOG_FUNCTION

	void Debug(const char* Format, ...)
	{
#ifndef _DEBUG
		return;
#endif
		va_list va;
		va_start(va, Format);
		DebugVA(0, Format, va);
		va_end(va);
	}

	void Debug(int MinimumDebugVerbosity, const char* Format, ...)
	{
#ifndef _DEBUG
		return;
#endif
		va_list va;
		va_start(va, Format);
		DebugVA(MinimumDebugVerbosity, Format, va);
		va_end(va);
	}

	void operator()(LogLevel Level, const char* Format, ...)
	{
		va_list va;
		va_start(va, Format);
		LogVA(Level, Format, va);
		va_end(va);
	}

	void LogVA(LogLevel Level, const char* Format, va_list va)
	{
#ifndef _DEBUG
		if (Level == LogLevel::Debug)
			return;
#endif

		char FormattedString[16 * 1024];
		const auto FormattedStringLength = vsprintf_safe(FormattedString, Format, va);
		NoFormat(Level, FormattedString, FormattedStringLength);
	}

	void NoFormat(LogLevel Level, char* String, int StringSize)
	{
#ifndef _DEBUG
		if (Level == LogLevel::Debug)
			return;
#endif

		const auto Prefix = [&] {
			switch (Level)
			{
			case LogLevel::Debug:
				return "[DEBUG] ";
			case LogLevel::Info:
				return "[INFO]  ";
			case LogLevel::Warning:
				return "[WARN]  ";
			case LogLevel::Error:
				return "[ERROR] ";
			case LogLevel::Fatal:
				return "[FATAL] ";
			default:
				assert(false);
				return "[???]   ";
			}
		}();

		Print(Prefix);

		int LastNewline = -1;
		for (int i = 0; i < StringSize; ++i)
		{
			if (String[i] != '\n' && i != StringSize - 1)
				continue;

			auto StartIndex = LastNewline + 1;
			auto Size = static_cast<size_t>(i - StartIndex);
			auto StartPtr = &String[StartIndex];

			auto PrevEndChar = StartPtr[Size];
			StartPtr[Size] = 0;
			DEFER([&] { String[Size] = PrevEndChar; });

			Print({StartPtr, Size});

			if (PrevEndChar == '\n')
				PrintNewline();

			if (i == StringSize - 1)
				break;

			Print(Prefix);

			LastNewline = i;
		}

		LogFile.flush();
	}

private:
	// Must be null terminated.
	void Print(const StringView& String)
	{
		if (Dest & LogTo::Stdout)
		{
			printf("%.*s", int(String.size()), String.data());
		}

		if (Dest & LogTo::File && LogFile.is_open())
		{
			LogFile.write(String.data(), String.size());
		}

		if (Dest & LogTo::Debugger)
		{
#if defined(_WIN32)
			OutputDbgString(String.data());
#endif
		}
	}

	void PrintNewline()
	{
#ifdef _WIN32
		auto Newline = "\r\n";
#else
		auto Newline = "\n";
#endif
		Print(Newline);
	}

	void DebugVA(int MinimumDebugVerbosity, const char* Format, va_list va)
	{
#ifndef _DEBUG
		return;
#endif

		if (DebugVerbosity < MinimumDebugVerbosity)
			return;

		LogVA(LogLevel::Debug, Format, va);
	}

	// Defined in Log.cpp to not pollute the rest of the program with Windows.h.
	void OutputDbgString(const char*);

	MFile::RWFile LogFile;
};

extern Logger Log;