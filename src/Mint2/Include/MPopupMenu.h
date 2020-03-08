#pragma once

#include "MWidget.h"
#include "MDrawContext.h"
#include "MLookNFeel.h"

class MPopupMenu;
class MMenuItem;

class MPopupMenuLook {
public: 
	MCOLOR		m_SelectedPlaneColor;
	MCOLOR		m_SelectedTextColor;
	MCOLOR		m_UnfocusedSelectedPlaneColor;

protected:
	virtual void OnFrameDraw(MPopupMenu* pPopupMenu, MDrawContext* pDC);
public:
	MPopupMenuLook();
	virtual void OnDraw(MPopupMenu* pPopupMenu, MDrawContext* pDC);
	virtual MRECT GetClientRect(MPopupMenu* pPopupMenu, const MRECT& r);
};

class MMenuItem : public MWidget{
private:
	bool			m_bSelected;

protected:
	virtual void OnDraw(MDrawContext* pDC) override;
	virtual void OnDrawMenuItem(MDrawContext* pDC, bool bSelected);
	virtual bool OnEvent(MEvent* pEvent, MListener* pListener) override;
public:
	MMenuItem(const char* szName=NULL);
	virtual ~MMenuItem() override;

	bool IsSelected();

	int GetWidth();
	int GetHeight();

	virtual MPopupMenu* CreateSubMenu();
	MPopupMenu* GetSubMenu();

	void Select(bool bSelect);

#define MINT_MENUITEM	"MenuItem"
	virtual const char* GetClassName() override{ return MINT_MENUITEM; }
};

enum MPopupMenuTypes{
	MPMT_VERTICAL = 0,
	MPMT_HORIZONTAL = 1
};

class MPopupMenu : public MWidget{
protected:
	MPopupMenuTypes m_nPopupMenuType;
	int				m_nSelectedMenu;

protected:
	virtual bool OnEvent(MEvent* pEvent, MListener* pListener) override;
	virtual bool OnCommand(MWidget* pWindow, const char* szMessage) override;

public:
	MPopupMenu(const char* szName = nullptr, MWidget* pParent = nullptr,
		MListener* pListener = nullptr, MPopupMenuTypes t = MPMT_VERTICAL);
	virtual ~MPopupMenu() override;

	MPopupMenuTypes GetPopupMenuType() { return m_nPopupMenuType; }
	virtual MMenuItem* AddMenuItem(const char* szMenuName);
	void AddMenuItem(MMenuItem* pMenuItem);
	void RemoveMenuItem(MMenuItem* pMenuItem);
	void RemoveAllMenuItem();

	virtual void Show(bool bVisible = true);
	virtual void Show(int x, int y, bool bVisible = true);

	void SetType(MPopupMenuTypes t);
	MPopupMenuTypes GetType();

	void Select(int idx);
	void Select(MMenuItem* pMenuItem);

#define MINT_POPUPMENU	"PopupMenu"
	virtual const char* GetClassName() override { return MINT_POPUPMENU; }

	DECLARE_LOOK(MPopupMenuLook)
	DECLARE_LOOK_CLIENT()
};
