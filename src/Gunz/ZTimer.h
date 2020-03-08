#pragma once

#include <list>
#include <functional>

using ZGameTimerEventCallback = void(void*);
using ZGameTimerEventCallbackMultiple = bool(void*);

struct ZTimerEvent;

class ZTimer
{
public:
	ZTimer();
	~ZTimer();

	double UpdateFrame();
	void ResetFrame();

	void SetTimerEvent(u64 DelayInMilliseconds,
		std::function<bool()> Callback);

private:
	void UpdateEvents(double DeltaTime);
	u64 GetTimeInTicks();

	std::vector<ZTimerEvent> m_EventList;

	bool UsingQPF{};
	u64 LastTimeInTicks{};
	u64 TicksPerSecond{};
};

class ZUpdateTimer
{
private:
	float		m_fUpdateTime;
	float		m_fElapsedTime;
public:
	ZUpdateTimer(float fUpdateTime) : m_fUpdateTime(fUpdateTime), m_fElapsedTime(0.0f) { }
	ZUpdateTimer() : m_fUpdateTime(0.0f), m_fElapsedTime(0.0f) { }
	bool Update(float fDelta)
	{
		m_fElapsedTime += fDelta;
		if (m_fElapsedTime < m_fUpdateTime) return false;

		m_fElapsedTime = 0.0f;
		return true;
	}
	void Init(float fUpdateTime)	{ m_fUpdateTime = fUpdateTime; m_fElapsedTime = 0.0f; }
	void Force()					{ m_fElapsedTime = m_fUpdateTime; }
	void SetUpdateTime(float fUpdateTime) { m_fUpdateTime = fUpdateTime; }
	float GetUpdateTime() const { return m_fUpdateTime; }
};