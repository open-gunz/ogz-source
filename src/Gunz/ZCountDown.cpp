#include "stdafx.h"
#include "ZCountDown.h"
#include "ZGameInterface.h"
#include "ZApplication.h"

void SetCountdown(const ZCOUNTDOWN& Countdown)
{
	auto Callback = [Countdown = Countdown]() mutable
	{
		ZIDLResource* pResource = ZApplication::GetGameInterface()->GetIDLResource();
		MWidget* pTargetWidget = pResource->FindWidget(Countdown.szTargetWidget);
		if (!pTargetWidget || !pTargetWidget->IsVisible()) return true;

		if (Countdown.nSeconds > 0) {

			if (Countdown.szLabelWidget != NULL)
			{
				ZIDLResource* pResource = ZApplication::GetGameInterface()->GetIDLResource();
				MWidget* pWidget = pResource->FindWidget(Countdown.szLabelWidget);
				if (pWidget)
				{
					char buffer[256];
					sprintf_safe(buffer, "%d", Countdown.nSeconds);
					pWidget->SetText(buffer);
				}
			}

			Countdown.nSeconds--;
			return false;
		}

		pTargetWidget->Show(false);

		if (Countdown.pCallBack)
			Countdown.pCallBack();

		return true;
	};

	ZApplication::GetTimer()->SetTimerEvent(1000, Callback);
}