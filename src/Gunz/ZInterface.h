#pragma once

#include "ZIDLResource.h"
#include "RealSpace2.h"
#include "RBaseTexture.h"
#include "Mint.h"
#include "Mint4R2.h"

_USING_NAMESPACE_REALSPACE2

class ZInterface : public MWidget {
public:
	ZInterface(const char* szName = nullptr, MWidget* pParent = nullptr, MListener* pListener = nullptr);
	virtual ~ZInterface() override;

	virtual void OnDraw(MDrawContext* pDC) override;
	virtual bool OnEvent(MEvent* pEvent, MListener* pListener) override = 0;

	virtual bool IsDone();
	virtual bool OnCreate();
	virtual void OnDestroy();

protected:
	bool m_bDone;
};