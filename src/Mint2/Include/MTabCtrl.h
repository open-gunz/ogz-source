#pragma once

#include <list>
#include "MWidget.h"
#include "MLookNFeel.h"

class MTabCtrl;
class MDrawContext;
class MButton;

class MTabCtrlLook{
public:
	virtual void OnDraw(MTabCtrl* pTabCtrl, MDrawContext* pDC);
	virtual MRECT GetClientRect(MTabCtrl* pTabCtrl, const MRECT& r);
};

class MTabItem {
public:
	MWidget	*pFrame;
	MButton *pButton;
	MListener *pButtonListener;
	MTabItem(MWidget *pFrame,MButton *pButton,MListener *pListener);
};

using MTabList = std::list<MTabItem*>;

class MTabCtrl : public MWidget {
protected:
	int			m_nCurrentSel;
	MTabList	m_TabList;

public:
	MTabCtrl(const char* szName, MWidget* pParent = nullptr, MListener* pListener = nullptr);
	MTabCtrl(MWidget* pParent = nullptr, MListener* pListener = nullptr);
	virtual ~MTabCtrl() override;
		
	void Add(MButton *pButton, MWidget *pFrame);
	void RemoveAll();

	int GetCount();
	int GetSelIndex();
	bool SetSelIndex(int i);

	void UpdateListeners();

	virtual bool OnCommand(MWidget* pWidget, const char* szMessage) override;
public:
	DECLARE_LOOK(MTabCtrlLook)
	DECLARE_LOOK_CLIENT()

	#define MINT_TABCTRL	"TabCtrl"
	virtual const char* GetClassName() override{ return MINT_TABCTRL; }
};
