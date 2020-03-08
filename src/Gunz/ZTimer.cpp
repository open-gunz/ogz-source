#include "stdafx.h"
#include "ZTimer.h"

struct ZTimerEvent
{
	// The amount of time elapsed so far.
	double ElapsedTime{};

	// The amount of time that has to pass before the event is triggered.
	double EventTime{};

	// The callback to be called once the event is triggered.
	std::function<bool()> Callback;

	// Returns true if the event is to be stopped and false to continue.
	bool UpdateTick(double Delta)
	{
		ElapsedTime += Delta;

		if (ElapsedTime >= EventTime)
		{
			auto CallbackRet = Callback();
			if (!CallbackRet)
			{
				ElapsedTime -= EventTime;
			}

			return CallbackRet;
		}

		return false;
	}
};

ZTimer::ZTimer()
{
#ifdef WIN32
	LARGE_INTEGER Frequency;
	auto qpf_ret = QueryPerformanceFrequency(&Frequency);
	if (qpf_ret != FALSE)
	{
		UsingQPF = true;
		TicksPerSecond = Frequency.QuadPart;
	}
	else
	{
		// timeGetTime resolution is in milliseconds
		TicksPerSecond = 1000;
	}
#endif

	LastTimeInTicks = GetTimeInTicks();
}

ZTimer::~ZTimer() = default;

void ZTimer::ResetFrame()
{
	LastTimeInTicks = GetTimeInTicks();
}

u64 ZTimer::GetTimeInTicks()
{
	if (UsingQPF)
		return QPC();
	
	return timeGetTime();
}

double ZTimer::UpdateFrame()
{
	u64 TimeInTicks = GetTimeInTicks();
	auto ElapsedTicks = TimeInTicks - LastTimeInTicks;
	auto ElapsedSeconds = double(ElapsedTicks) / TicksPerSecond;
	LastTimeInTicks = TimeInTicks;

	UpdateEvents(ElapsedSeconds);

	return ElapsedSeconds;
}

void ZTimer::UpdateEvents(double DeltaTime)
{
	// Update all of the events, and remove the ones that return true.
	erase_remove_if(m_EventList, [&](auto&& Event) {
		return Event.UpdateTick(DeltaTime);
	});
}

void ZTimer::SetTimerEvent(u64 DelayInMilliseconds,
	std::function<bool()> Callback)
{
	ZTimerEvent NewEvent;
	NewEvent.ElapsedTime = 0;
	NewEvent.EventTime = double(DelayInMilliseconds) / 1000.0;
	NewEvent.Callback = Callback;
	m_EventList.push_back(std::move(NewEvent));
}