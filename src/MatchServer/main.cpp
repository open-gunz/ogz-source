#include "stdafx.h"
#include "MBMatchServer.h"
#include <iostream>
#include <atomic>
#include "MFile.h"
#include "MCrashDump.h"
#ifndef WIN32
	#include <systemd/sd-daemon.h>
#endif

template <size_t size>
static bool GetLogFileName(char(&pszBuf)[size])
{
	if (!MFile::IsDir("Log"))
		MFile::CreateDir("Log");

	struct tm	tmTime = *localtime(&unmove(time(0)));

	char szFileName[MFile::MaxPath];

	int nFooter = 1;
	while (true) {
		sprintf_safe(szFileName, "Log/MatchLog_%02d-%02d-%02d-%d.txt",
			tmTime.tm_year + 1900, tmTime.tm_mon + 1, tmTime.tm_mday, nFooter);

		if (!MFile::IsFile(szFileName))
			break;

		nFooter++;
		if (nFooter > 100) return false;
	}
	strcpy_safe(pszBuf, szFileName);
	return true;
}


static std::mutex InputMutex;
static std::vector<std::string> InputQueue;
static std::atomic<bool> HasInput;

static void InputThreadProc()
{
	std::string Input;

	while (true)
	{
		Input.clear();
		std::getline(std::cin, Input);
		if (Input.empty())
			continue;
		{
			std::lock_guard<std::mutex> Lock{ InputMutex };
			InputQueue.emplace_back(Input);
			HasInput = true;
		}
	}
}

static void HandleInput(MBMatchServer& MatchServer)
{
	if (!HasInput)
		return;

	std::lock_guard<std::mutex> Lock{ InputMutex };

	for (auto&& Input : InputQueue)
		MatchServer.OnInput(Input);

	InputQueue.clear();
	HasInput = false;
}

int main(int argc, char** argv)
try
{
	char LogFileName[MFile::MaxPath];
	GetLogFileName(LogFileName);
	InitLog(MLOGSTYLE_DEBUGSTRING | MLOGSTYLE_FILE, LogFileName);

	void MatchServerCustomLog(const char*);
	CustomLog = MatchServerCustomLog;

	MBMatchServer MatchServer;

	if (!MatchServer.Create(6000))
	{
		MLog("MMatchServer::Create failed\n");
		return -1;
	}

	MatchServer.InitLocator();

	//std::thread{ [&] { InputThreadProc(); } }.detach();

#ifndef WIN32
	sd_notify(0, "READY=1");
#endif

	while (true)
	{
		MatchServer.Run();
		HandleInput(MatchServer);
		std::this_thread::sleep_for(std::chrono::milliseconds(1));
	}
}
catch (std::runtime_error& e)
{
	MLog("Uncaught std::runtime_error: %s\n", e.what());
	throw;
}