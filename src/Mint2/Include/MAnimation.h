#pragma once

#include "MWidget.h"
#include "GlobalTypes.h"

enum MAPlayMode{
	MAPM_FORWARDONCE,
	MAPM_FORWARDNBACKWARD,
	MAPM_REPETITION,
};

class MAnimation : public MWidget{
private:
	u64				m_nStartTime;
	MAniBitmap*		m_pAniBitmap;
public:
	int				m_nDelay;
	MAPlayMode		m_nPlayMode;
protected:
	virtual void OnDraw(MDrawContext* pDC) override;
public:
	MAnimation(const char* szName=NULL, MAniBitmap* pAniBitmap=NULL, MWidget* pParent=NULL);
	void SetAniBitmap(MAniBitmap* pAniBitmap);
	void InitStartTime();
	MBitmap* GetBitmap();

#define MINT_ANIMATION	"Animation"
	virtual const char* GetClassName() override { return MINT_ANIMATION; }

protected:
	int				m_nCurrFrame;

public:
	bool			m_bRunAnimation;
	bool			GetRunAnimation()  { return m_bRunAnimation;}
	void			SetRunAnimation( bool bRun);
	int				GetCurrentFrame();
	void			SetCurrentFrame( int nFrame);
};
