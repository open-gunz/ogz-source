#pragma once

typedef void(ZCOUNTDOWNEVENTCALLBACK)(void);

struct ZCOUNTDOWN {
	int nSeconds;
	const char *szLabelWidget;
	const char *szTargetWidget;
	ZCOUNTDOWNEVENTCALLBACK *pCallBack;
};

void SetCountdown(const ZCOUNTDOWN& Countdown);