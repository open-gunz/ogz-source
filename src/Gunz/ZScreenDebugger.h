#pragma once

class MDrawContext;

class ZScreenDebugger
{
private:
	int		m_nShowDebugInfo;
	int		m_nY;

	void OnDrawAIDebugInfo(MDrawContext *pDC);
	void DrawActorInfo(int num, MDrawContext *pDC, class ZActor* pActor);
	void PrintText(MDrawContext* pDC, const char* buffer);
public:
	ZScreenDebugger();
	~ZScreenDebugger();
	void DrawDebugInfo(MDrawContext *pDC);
	void SwitchDebugInfo();

	bool IsVisible();
};