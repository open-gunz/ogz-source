#include "stdafx.h"
#include "MProcess.h"

#ifdef _WIN32
#include "MWindows.h"

void MProcess::Start(const char* Name)
{
	STARTUPINFO si{};
	PROCESS_INFORMATION pi{};

	si.cb = sizeof(si);

	char MutableName[MFile::MaxPath];
	strcpy_safe(MutableName, Name);

	CreateProcessA(
		NULL,  // Application name
		MutableName, // Command line
		NULL,  // Process attributes
		NULL,  // Thread attributes
		FALSE, // Inherit handles
		0,     // Creation flags
		NULL,  // Environment
		NULL,  // Current directory
		&si,   // Startup info
		&pi);  // Process info
}
#else
#include <unistd.h>

void MProcess::Start(const char* Name)
{
	if (fork() == 0)
	{
		char* argv[] = {const_cast<char*>(Name), nullptr};
		execvp(Name, argv);
	}
}
#endif
