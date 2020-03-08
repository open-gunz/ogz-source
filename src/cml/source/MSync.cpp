#include "stdafx.h"
#include "MSync.h"
#include "MSocket.h"

#ifdef WIN32

#include <Windows.h>

MCriticalSection::MCriticalSection() { InitializeCriticalSection(&cs()); }
MCriticalSection::~MCriticalSection() { DeleteCriticalSection(&cs()); }
void MCriticalSection::lock() { EnterCriticalSection(&cs()); }
void MCriticalSection::unlock() { LeaveCriticalSection(&cs()); }

CRITICAL_SECTION& MCriticalSection::cs() {
	static_assert(sizeof(buf) == sizeof(CRITICAL_SECTION) &&
		alignof(decltype(buf)) == alignof(CRITICAL_SECTION), "");
	return reinterpret_cast<CRITICAL_SECTION&>(*(char*)buf);
}

static_assert(WAIT_OBJECT_0 == 0, "");

MSignalEvent::MSignalEvent(bool ManualReset) : EventHandle{ CreateEventA(NULL, ManualReset, FALSE, NULL) } {}

MSignalEvent::~MSignalEvent() {
	if (EventHandle) {
		CloseHandle(EventHandle);
		EventHandle = NULL;
	}
}

bool MSignalEvent::SetEvent() {
	return ::SetEvent(EventHandle) != FALSE; }
bool MSignalEvent::ResetEvent() {
	return ::ResetEvent(EventHandle) != FALSE; }

u32 WaitForMultipleEvents(size_t NumEvents, MSignalEvent* const * EventArray, u32 Timeout)
{
	HANDLE Handles[MAXIMUM_WAIT_OBJECTS];
	for (size_t i = 0; i < NumEvents; ++i)
		Handles[i] = EventArray[i]->GetEventHandle();

	auto ret = WaitForMultipleObjects(NumEvents, Handles, FALSE, Timeout);

	switch (ret)
	{
	case WAIT_FAILED:
		return MSync::WaitFailed;
	case WAIT_TIMEOUT:
		return MSync::WaitTimeout;
	default:
		return ret;
	}
}

#else

#include <unistd.h>
#include <sys/select.h>
#include <sys/eventfd.h>

MSignalEvent::MSignalEvent(bool ManualReset) : EventHandle{ eventfd(0, 0) } {}
MSignalEvent::~MSignalEvent() { close(EventHandle); }

bool MSignalEvent::SetEvent()
{
	u64 buf = 1;
	return write(EventHandle, &buf, sizeof(buf)) == sizeof(buf);
}

bool MSignalEvent::ResetEvent()
{
	u64 buf = 0;
	return read(EventHandle, &buf, sizeof(buf)) == sizeof(buf);
}

u32 WaitForMultipleEvents(size_t NumEvents, MSignalEvent* const * EventArray, u32 Timeout)
{
	fd_set rfds, wfds, efds;
	for (auto fds : {&rfds, &wfds, &efds})
		FD_ZERO(fds);

	int max_fd = INT_MIN;
	for (int i = 0; i < int(NumEvents); ++i) {
		auto& e = EventArray[i];
		const auto fd = e->EventHandle;
		e->SetEvents = 0;
		auto ev = e->Events;
		if (ev & (MSignalEvent::Read | MSignalEvent::Close))
			FD_SET(fd, &rfds);
		if ((ev & (MSignalEvent::Write | MSignalEvent::Connect)) && !e->ReadyForWriting)
			FD_SET(fd, &wfds);
		if (fd + 1 > max_fd)
			max_fd = fd + 1;
	}

	if (max_fd == INT_MIN)
		return MSync::WaitFailed;

	timeval tv;
	const bool BlockIndefinitely = Timeout == MSync::Infinite;
	if (!BlockIndefinitely)
	{
		tv.tv_sec = Timeout / 1000;
		tv.tv_usec = (Timeout % 1000) * 1000;
	}

	auto ret = select(max_fd, &rfds, &wfds, &efds, BlockIndefinitely ? nullptr : &tv);
	if (ret == 0)
		return MSync::WaitTimeout;
	if (ret < 0)
		return MSync::WaitFailed;

	for (int i = 0; i < int(NumEvents); ++i) {
		auto& e = EventArray[i];
		const auto fd = e->EventHandle;
		if (FD_ISSET(fd, &rfds)) {
			e->SetEvents |= MSignalEvent::Read;
		}
		bool wset = FD_ISSET(fd, &wfds);
		if (wset && !e->ReadyForWriting)
			e->SetEvents |= MSignalEvent::Write;
		e->ReadyForWriting = wset;
		if (e->SetEvents)
			return static_cast<u32>(i);
	}

	MLog("WaitForMultipleEvents(%d, MSignalEvent[], %d) - select() returned before timeout but no events were tagged!\n", NumEvents, Timeout);
	return MSync::WaitTimeout;
}

#endif

u32 MSignalEvent::Await(u32 Timeout) {
	auto this_lvalue = this;
	return WaitForMultipleEvents(1, &this_lvalue, Timeout);
}