#include "stdafx.h"
#include "MTabCtrl.h"
#include "MColorTable.h"
#include "MButton.h"


//// MTabCtrlLook ///////////////////////////////////////////////////
IMPLEMENT_LOOK(MTabCtrl, MTabCtrlLook)
void MTabCtrlLook::OnDraw(MTabCtrl* pTabCtrl, MDrawContext* pDC)
{
	MRECT r = pTabCtrl->GetInitialClientRect();
	const int nButtonHeight = 24;
	r.y += nButtonHeight;
	r.h -= nButtonHeight;
	pDC->SetColor(128,128,128,255);
	pDC->Rectangle(r);
}

MRECT MTabCtrlLook::GetClientRect(MTabCtrl* pTabCtrl, const MRECT& r)
{
	MRECT t = r;
	t.EnLarge(-1);
	return t;
}


//// MTabItem ///////////////////////////////////////////////////////
MTabItem::MTabItem(MWidget *pFrame,MButton *pButton,MListener *pListener)
{
	MTabItem::pFrame = pFrame;
	MTabItem::pButton = pButton;
	MTabItem::pButtonListener= pListener;
}


//// MTabCtrl ///////////////////////////////////////////////////////
MTabCtrl::MTabCtrl(const char* szName, MWidget* pParent, MListener* pListener) : MWidget(szName, pParent, pListener)
{
}

MTabCtrl::MTabCtrl(MWidget* pParent, MListener* pListener) : MWidget("TabCtrl", pParent, pListener)
{
}

MTabCtrl::~MTabCtrl()
{
	RemoveAll();
}

void MTabCtrl::Add(MButton *pButton, MWidget *pFrame)
{
	MTabItem *pItem = new MTabItem(pFrame,pButton,pButton->GetListener());
	pButton->SetListener(this);
	m_TabList.push_back(pItem);
}

void MTabCtrl::RemoveAll()
{
	while(m_TabList.size() > 0) {
		MTabItem* pItem = *m_TabList.begin();
		delete pItem;
		m_TabList.pop_front();
	}
}

int MTabCtrl::GetCount()
{
	return m_TabList.size();
}

int MTabCtrl::GetSelIndex()
{
	return m_nCurrentSel;
}

bool MTabCtrl::SetSelIndex(int nIndex)
{
	MTabItem *pSelItem = NULL;
	int nCurIndex = 0;
	for(MTabList::iterator i=m_TabList.begin();i!=m_TabList.end();i++)
	{
		MTabItem *pItem = *i;
		if(nIndex==nCurIndex) 
		{
			pSelItem = pItem;
			m_nCurrentSel = nCurIndex;
		}
		else {
			pItem->pButton->SetCheck(false);
			if(pItem->pFrame)
				pItem->pFrame->Show(false);
		}
		nCurIndex++;
	}

	if(pSelItem)
	{
		pSelItem->pButton->SetCheck(true);
		if(pSelItem->pFrame)
			pSelItem->pFrame->Show(true);
	}
	return true;
}

bool MTabCtrl::OnCommand(MWidget* pWidget, const char* szMessage)
{
	int nIndex = 0;
	for(MTabList::iterator i=m_TabList.begin();i!=m_TabList.end();i++)
	{
		MTabItem *pItem = *i;
		if(pItem->pButton == pWidget) {
			if(strcmp(szMessage, MBTN_CLK_MSG)==0) {
				SetSelIndex(nIndex);
			}
			return pItem->pButtonListener->OnCommand(pWidget,szMessage);
		}
		nIndex++;
	}
	return false;
}

void MTabCtrl::UpdateListeners()
{
	int nIndex = 0;
	for(MTabList::iterator i=m_TabList.begin();i!=m_TabList.end();i++)
	{
		MTabItem *pItem = *i;
		if (pItem->pButton->GetListener() != this)
		{
			pItem->pButtonListener = pItem->pButton->GetListener();
			pItem->pButton->SetListener(this);
		}
	}
}