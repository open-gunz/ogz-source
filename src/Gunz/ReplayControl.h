#pragma once
#include "MEvent.h"

class ReplayControl
{
public:
	void OnDraw(MDrawContext* pDC);
	bool OnEvent(MEvent *pEvent);
private:
};

extern ReplayControl g_ReplayControl;