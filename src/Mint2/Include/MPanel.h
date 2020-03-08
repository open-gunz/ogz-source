#pragma once

#include "MWidget.h"
#include "MDrawContext.h"
#include "MLookNFeel.h"

class MPanel;

class MPanelLook{
protected:
	virtual void OnFrameDraw(MPanel* pPanel, MDrawContext* pDC);
public:
	virtual void OnDraw(MPanel* pLabel, MDrawContext* pDC);
	virtual MRECT GetClientRect(MPanel* pLabel, const MRECT& r);
};

enum MBorderStyle {
	MBS_NONE = 0,
	MBS_SINGLE,
};

class MPanel : public MWidget{
protected:
	DECLARE_LOOK(MPanelLook)
	DECLARE_LOOK_CLIENT()
protected:
	MBorderStyle	m_nBorderStyle;
	MCOLOR			m_BorderColor;
	MCOLOR			m_BackgroundColor;
public:
	MPanel(const char* szName=NULL, MWidget* pParent=NULL, MListener* pListener=NULL);

	void SetBackgroundColor(MCOLOR color);
	MCOLOR GetBackgroundColor();

	void SetBorderColor(MCOLOR color);
	MCOLOR GetBorderColor();
	void SetBorderStyle(MBorderStyle style);
	MBorderStyle GetBorderStyle();


#define MINT_PANEL	"Panel"
	virtual const char* GetClassName() override { return MINT_PANEL; }
};
