#pragma once

const int FRAME_RING_BUFFER_SIZE =	500;

struct ZFrameData
{
	DWORD dwTime;
	DWORD dwElapsed;
};

class ZProfiler
{
	ZFrameData m_dwRingBuffer[FRAME_RING_BUFFER_SIZE];
	DWORD m_dwLastTime;
	int	m_nRingHead;

	int GetRelativeIndex(int nIndex);

public:
	ZProfiler(void);
	virtual ~ZProfiler(void);

	void Update();
	void Render();
};