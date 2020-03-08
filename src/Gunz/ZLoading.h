#pragma once

#include "ZInterface.h"
#include "MPicture.h"

enum LoadingState
{
	LOADING_BEGIN,
	LOADING_ANIMATION,
	LOADING_BSP,
	LOADING_END,
	NUM_LOADING_STATE,
};

class ZLoading : public ZInterface
{
protected:
	MPicture*		m_pBackGround;
	MBitmapR2*		m_pBitmap;

	int				m_iPercentage;

public:
	ZLoading(const char* szName=NULL, MWidget* pParent=NULL, MListener* pListener=NULL);
	virtual ~ZLoading();
	virtual bool OnCreate();
	virtual void OnDestroy();
	virtual void OnDraw(MDrawContext* pDC);
	virtual bool OnEvent(MEvent* pEvent, MListener* pListener);

	void	Progress(LoadingState state);
	int		GetProgress() const;
};