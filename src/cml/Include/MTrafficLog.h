#pragma once

#include "MTime.h"
#include "RingBuffer.h"

struct MTrafficNode
{
	u32	Timestamp;
	u32	TrafficSum;
};

struct MTrafficLog
{
	static constexpr auto DefaultSamplingInterval = 100;
	static constexpr auto DefaultCount = 10;
	static constexpr auto MaxCount = 256;

	void Record(u32 TrafficSum)
	{
		auto Time = static_cast<u32>(GetGlobalTimeMS());

		if (Time - Logs[0].Timestamp > u64(SamplingInterval))
		{
			Logs.emplace_front(Time, TrafficSum);
		}
	}

	int GetTrafficSpeed() const
	{
		auto&& Current = Logs[0];
		auto&& Last = Logs[-1];

		auto ElapsedTime = Current.Timestamp - Last.Timestamp;

		if (Last.Timestamp == 0 || ElapsedTime == 0)
			return 0;

		return int((Current.TrafficSum - Last.TrafficSum) / (ElapsedTime / 1000.0f) + 0.5f);
	}

private:
	int SamplingInterval = DefaultSamplingInterval;
	RingBuffer<MTrafficNode, 256> Logs;
};