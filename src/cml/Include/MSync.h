#pragma once

#include "SafeString.h"
#include "GlobalTypes.h"

#ifdef WIN32

using CRITICAL_SECTION = struct _RTL_CRITICAL_SECTION;

class MCriticalSection
{
public:
	MCriticalSection();
	MCriticalSection(const MCriticalSection&) = delete;
	const MCriticalSection& operator=(const MCriticalSection&) = delete;
	~MCriticalSection();
	void lock();
	void unlock();

private:
	void* buf[24 / sizeof(void*)];

	CRITICAL_SECTION& cs();
};

#else

#include <mutex>

class MCriticalSection {
	std::mutex mutex;

public:
	void lock() { mutex.lock(); }
	void unlock() { mutex.unlock(); }
};

#endif

namespace MSync
{
constexpr u32 Infinite = 0xFFFFFFFF;
constexpr u32 WaitFailed = 0xFFFFFFFF;
constexpr u32 WaitTimeout = 0xFFFFFFFE;
}

class MSignalEvent
{
public:
#ifdef WIN32
	using HandleType = void*; // WINAPI HANDLE
#else
	using HandleType = int; // File descriptor
#endif

	MSignalEvent(bool ManualReset = false);

	// Noncopyable
	MSignalEvent(const MSignalEvent&) = delete;
	const MSignalEvent& operator=(const MSignalEvent&) = delete;
	// Movable
	MSignalEvent(MSignalEvent&&) = default;
	MSignalEvent& operator=(MSignalEvent&&) = default;

	~MSignalEvent();

	bool SetEvent();
	bool ResetEvent();

	u32 Await(u32 Timeout = MSync::Infinite);

	HandleType GetEventHandle() const { return EventHandle; }

	HandleType EventHandle;

#ifndef _WIN32
	enum { Connect = 1 << 0, Read = 1 << 1, Write = 1 << 2, Close = 1 << 3 };
	u32 Events = Read;
	u32 SetEvents = 0;
	bool ReadyForWriting = false;
#endif
};

// Returns the index of the event that was triggered, WAIT_TIMEOUT on timeout, or WAIT_FAILED on error.
// Timeout is in milliseconds. INFINITE is no timeout (block indefinitely).
u32 WaitForMultipleEvents(size_t NumEvents, MSignalEvent* const * EventArray, u32 Timeout);

// Wrapper for arrays.
template <size_t size> int WaitForMultipleEvents(MSignalEvent* const (&EventArray)[size], u32 Timeout) {
	return WaitForMultipleEvents(size, EventArray, Timeout); }
