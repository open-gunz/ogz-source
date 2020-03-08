#include "stdafx.h"
#include "MPopupMenu.h"
#include "MColorTable.h"
#include <algorithm>

IMPLEMENT_LOOK(MPopupMenu, MPopupMenuLook)

//// MPopupMenuLook ////
MPopupMenuLook::MPopupMenuLook()
{
	m_SelectedPlaneColor = DEFCOLOR_MLIST_SELECTEDPLANE;
	m_SelectedTextColor = DEFCOLOR_MLIST_SELECTEDTEXT;
	m_UnfocusedSelectedPlaneColor = DEFCOLOR_DARK;
}

void MPopupMenuLook::OnFrameDraw(MPopupMenu* pPopupMenu, MDrawContext* pDC)
{
	MRECT r = pPopupMenu->GetClientRect();
	pDC->SetColor(MCOLOR(DEFCOLOR_MPOPUP_PLANE));
	pDC->FillRectangle(r);
	pDC->SetColor(MCOLOR(DEFCOLOR_FRAME_OUTLINE));
	pDC->Rectangle(r);
}

void MPopupMenuLook::OnDraw(MPopupMenu* pPopupMenu, MDrawContext* pDC)
{
	OnFrameDraw(pPopupMenu, pDC);

	if(pPopupMenu->GetPopupMenuType()==MPMT_VERTICAL){
		pDC->SetColor(MCOLOR(DEFCOLOR_FRAME_OUTLINE));
		pDC->Rectangle(pPopupMenu->GetInitialClientRect());
	}
}

MRECT MPopupMenuLook::GetClientRect(MPopupMenu* pPopupMenu, const MRECT& r)
{
	if(pPopupMenu->GetPopupMenuType()==MPMT_VERTICAL)
		return MRECT(1, 1, r.w-2, r.h-2);
	else
		return pPopupMenu->GetInitialClientRect();
}

//// MMenuItem ////
void MMenuItem::OnDrawMenuItem(MDrawContext* pDC, bool bSelected)
{
	MRECT r = GetClientRect();	
	pDC->SetColor(bSelected==true?MCOLOR(DEFCOLOR_MPOPUP_SELECTEDPLANE):MCOLOR(DEFCOLOR_MPOPUP_PLANE));
	pDC->FillRectangle(r);
	pDC->SetColor(bSelected==true?MCOLOR(DEFCOLOR_MPOPUP_SELECTEDTEXT):MCOLOR(DEFCOLOR_MPOPUP_TEXT));
	pDC->Text(r, GetText(), MAM_LEFT);
}

void MMenuItem::OnDraw(MDrawContext* pDC)
{
	OnDrawMenuItem(pDC, IsSelected());
}

bool MMenuItem::OnEvent(MEvent* pEvent, MListener* pListener)
{
	MRECT r = GetClientRect();
	switch(pEvent->nMessage){
	case MWM_MOUSEMOVE:
	case MWM_LBUTTONDOWN:
	case MWM_RBUTTONDOWN:
	case MWM_LBUTTONDBLCLK:
		if(r.InPoint(pEvent->Pos)==true){
			MPopupMenu* pPopupMenu = (MPopupMenu *)GetParent();
			pPopupMenu->Select(this);
			return true;
		}
		break;
	case MWM_LBUTTONUP:
		if(r.InPoint(pEvent->Pos)==true){
			MPopupMenu* pPopupMenu = (MPopupMenu *)GetParent();
			if(GetSubMenu()!=NULL) pPopupMenu->Select(this);
			else{
				if(pListener!=NULL) pListener->OnCommand(this, GetText());
			}
			return true;
		}
		break;
	case MWM_RBUTTONUP:
		break;
	}
	return false;
}

MMenuItem::MMenuItem(const char* szName) : MWidget(szName, NULL, NULL)
{
	m_bSelected = false;
}

MMenuItem::~MMenuItem()
{
	while(m_Children.GetCount()>0){
		MWidget* pWidget = m_Children.Get(0);
		delete pWidget;
	}
}

bool MMenuItem::IsSelected()
{
	return m_bSelected;
}

#define MENUITEM_MARGIN_X	20
#define MENUITEM_MARGIN_Y	4
int MMenuItem::GetWidth()
{
	MFont* pFont = GetFont();
	const char* szText = GetText();

	return pFont->GetWidth(szText) + MENUITEM_MARGIN_X;
}

int MMenuItem::GetHeight()
{
	MFont* pFont = GetFont();
	return pFont->GetHeight() + MENUITEM_MARGIN_Y;
	return GetClientRect().h;
}

MPopupMenu* MMenuItem::CreateSubMenu()
{
	if(m_Children.GetCount()>0) return (MPopupMenu *)m_Children.Get(0);
	MPopupMenu* pSubMenu = new MPopupMenu("SubMenu", this, GetParent());
	return pSubMenu;
}

MPopupMenu* MMenuItem::GetSubMenu()
{
	if(m_Children.GetCount()>0) return (MPopupMenu *)m_Children.Get(0);
	return NULL;
}

void MMenuItem::Select(bool bSelect)
{
	m_bSelected = bSelect;

	MRECT r = GetClientRect();

	if(m_bSelected==true){
		MPopupMenu* pSubMenu = GetSubMenu();
		if(pSubMenu!=NULL && pSubMenu->IsVisible()==false){
			MPopupMenu* pPopupMenu = (MPopupMenu *)GetParent();
			if(pPopupMenu->GetType()==MPMT_VERTICAL) pSubMenu->Show(r.x+r.w, r.y, true);
			else pSubMenu->Show(r.x, r.y+r.h, true);
		}
	}
	else{
		MPopupMenu* pSubMenu = GetSubMenu();
		if(pSubMenu!=NULL) pSubMenu->Show(false);
	}
}

//// MPopupMenu ////
bool MPopupMenu::OnEvent(MEvent* pEvent, MListener* pListener)
{
	MRECT r = GetClientRect();
	switch(pEvent->nMessage){
	case MWM_LBUTTONDOWN:
	case MWM_LBUTTONUP:
	case MWM_LBUTTONDBLCLK:
	case MWM_RBUTTONDOWN:
	case MWM_RBUTTONDBLCLK:
	case MWM_MBUTTONDOWN:
	case MWM_MBUTTONUP:
	case MWM_MBUTTONDBLCLK:
		if(r.InPoint(pEvent->Pos)==false){
			if(m_nPopupMenuType==MPMT_VERTICAL) Show(false);
			else Select((MMenuItem *)NULL);
		}
		break;
	}
	return false;
}

MPopupMenu::MPopupMenu(const char* szName, MWidget* pParent, MListener* pListener, MPopupMenuTypes t) : MWidget(szName, pParent, pListener)
{
	m_nPopupMenuType = t;
}

MPopupMenu::~MPopupMenu()
{
	while(m_Children.GetCount()>0){
		MWidget* pWidget = m_Children.Get(0);
		delete pWidget;
	}
}

MMenuItem* MPopupMenu::AddMenuItem(const char* szMenuName)
{
	MMenuItem* pNewItem = new MMenuItem(szMenuName);
	AddMenuItem(pNewItem);
	return pNewItem;
}

void MPopupMenu::AddMenuItem(MMenuItem* pMenuItem)
{
	AddChild(pMenuItem);
	pMenuItem->SetListener(this);

	MRECT cr = GetClientRect();
	MRECT ir = GetInitialClientRect();
	if(m_nPopupMenuType==MPMT_VERTICAL){
		int y = 0;
		int nWidth = 0;
		for(int i=0; i<m_Children.GetCount(); i++){
			MMenuItem* pMenuItem = (MMenuItem *)m_Children.Get(i);
			pMenuItem->SetPosition(cr.x, cr.y+y);
			nWidth = std::max(nWidth, pMenuItem->GetWidth());
			y += pMenuItem->GetHeight();
		}
		for(int i=0; i<m_Children.GetCount(); i++){
			MMenuItem* pMenuItem = (MMenuItem *)m_Children.Get(i);
			pMenuItem->SetSize(nWidth, pMenuItem->GetHeight());
		}
		SetSize(nWidth+ir.w-cr.w-1, y+ir.h-cr.h-1);
	}
	else{
		int x = 0;
		int nHeight = 0;
		for(int i=0; i<m_Children.GetCount(); i++){
			MMenuItem* pMenuItem = (MMenuItem *)m_Children.Get(i);
			pMenuItem->SetPosition(cr.x+x, cr.y);
			nHeight = std::max(nHeight, pMenuItem->GetHeight());
			x += pMenuItem->GetWidth();
		}
		for(int i=0; i<m_Children.GetCount(); i++){
			MMenuItem* pMenuItem = (MMenuItem *)m_Children.Get(i);
			pMenuItem->SetSize(pMenuItem->GetWidth(), nHeight);
		}
		SetSize(x+ir.w-cr.w-1, nHeight+ir.h-cr.h-1);
	}
}

void MPopupMenu::RemoveMenuItem(MMenuItem* pMenuItem)
{
	delete pMenuItem;
}

void MPopupMenu::RemoveAllMenuItem()
{
	while (GetChildCount() > 0) {
		MWidget* pChild = m_Children.Get(0);
		delete pChild;
	}
}

void MPopupMenu::Show(int x, int y, bool bVisible)
{
	MWidget::Show(bVisible);
	if (bVisible == true) SetPosition(x, y);
}

void MPopupMenu::Show(bool bVisible)
{
	Select(nullptr);
	MWidget::Show(bVisible);
}

void MPopupMenu::SetType(MPopupMenuTypes t)
{
	m_nPopupMenuType = t;
}

MPopupMenuTypes MPopupMenu::GetType()
{
	return m_nPopupMenuType;
}

void MPopupMenu::Select(int idx)
{
	for(int i=0; i<m_Children.GetCount(); i++){
		MMenuItem* pMenuItem = (MMenuItem *)m_Children.Get(i);
		if(i==idx) pMenuItem->Select(true);
		else pMenuItem->Select(false);
	}
}

void MPopupMenu::Select(MMenuItem* pMenuItem)
{
	for(int i=0; i<m_Children.GetCount(); i++){
		MMenuItem* pThisMenuItem = (MMenuItem *)m_Children.Get(i);
		if(pThisMenuItem==pMenuItem) pThisMenuItem->Select(true);
		else pThisMenuItem->Select(false);
	}
}

bool MPopupMenu::OnCommand(MWidget* pWindow, const char* szMessage)
{
	if(GetType()==MPMT_VERTICAL) Show(false);
	else Select((MMenuItem*)NULL);
	MListener* pListener = GetListener();
	if(pListener!=NULL) pListener->OnCommand(this, szMessage);
	return true;
}
