#include "Log.h"

Logger Log;

#ifdef _WIN32
#include "MWindows.h"

void Logger::OutputDbgString(const char* String)
{
	OutputDebugStringA(String);
}
#endif