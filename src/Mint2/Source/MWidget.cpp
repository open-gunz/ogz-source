#include "stdafx.h"
#include "MResource.h"
#include "MWidget.h"
#include "MToolTip.h"
#include "Mint.h"
#include "MButton.h"
#include "MEdit.h"

inline bool MIsSameWidgetClass(MWidget* pWidget, const char* szClassName)
{
	if (strcmp(pWidget->GetClassName(), szClassName) == 0) return true;
	return false;
}

inline bool MIsSameWidgetClass(MWidget* pWidgetA, MWidget* pWidgetB)
{
	if (strcmp(pWidgetA->GetClassName(), pWidgetB->GetClassName()) == 0) return true;
	return false;
}

MWidget* MWidget::m_pCapturedWidget = NULL;
MWidget* MWidget::m_pFocusedWidget = NULL;

static int g_nWidgetCount=0;

void MWidget::MakeLocalEvent(MEvent* pLoalEvent, const MEvent* pEvent)
{
	memcpy(pLoalEvent, pEvent, sizeof(MEvent));

	MRECT sr = GetScreenRect();
	pLoalEvent->Pos.x -= sr.x;
	pLoalEvent->Pos.y -= sr.y;
}

bool MWidget::EventResize(MEvent* pEvent)
{
	MEvent LocalEvent;
	MakeLocalEvent(&LocalEvent, pEvent);

	// Widget Resize
#define RESIZER_SIZE	4
	if(m_bResizable==true){
		MPOINT sp = pEvent->Pos;
		MWidget* pParent = GetParent();
		if(pParent==NULL) return false;

		if(pEvent->nMessage==MWM_LBUTTONDOWN){
			m_nResizeSide = 0;
			if(LocalEvent.Pos.y>=-RESIZER_SIZE && LocalEvent.Pos.y<=RESIZER_SIZE){
				if(LocalEvent.Pos.x>=-RESIZER_SIZE && LocalEvent.Pos.x<=m_Rect.w+RESIZER_SIZE)
					m_nResizeSide |= 1;
			}
			if(LocalEvent.Pos.x>=m_Rect.w-RESIZER_SIZE && LocalEvent.Pos.x<=m_Rect.w+RESIZER_SIZE){
				if(LocalEvent.Pos.y>=-RESIZER_SIZE && LocalEvent.Pos.y<=m_Rect.h+RESIZER_SIZE)
					m_nResizeSide |= 2;
			}
			if(LocalEvent.Pos.y>=m_Rect.h-RESIZER_SIZE && LocalEvent.Pos.y<=m_Rect.h+RESIZER_SIZE){
				if(LocalEvent.Pos.x>=-RESIZER_SIZE && LocalEvent.Pos.x<=m_Rect.w+RESIZER_SIZE)
					m_nResizeSide |= 4;
			}
			if(LocalEvent.Pos.x>=-RESIZER_SIZE && LocalEvent.Pos.x<=RESIZER_SIZE){
				if(LocalEvent.Pos.y>=-RESIZER_SIZE && LocalEvent.Pos.y<=m_Rect.h+RESIZER_SIZE)
					m_nResizeSide |= 8;
			}
			if(m_nResizeSide!=0){
				SetCapture();
				//
				return true;
			}
		}
		if(m_nResizeSide!=0 && pEvent->nMessage==MWM_LBUTTONUP){
			ReleaseCapture();
			m_nResizeSide = 0;
			return true;
		}
		if(pEvent->nMessage==MWM_MOUSEMOVE){
			if(m_nResizeSide!=0){
				MPOINT pp = MScreenToClient(pParent, sp);
				MPOINT tp = MScreenToClient(this, sp);
				MRECT r = m_Rect;
				if(m_nResizeSide&1){
					r.h += (r.y - pp.y);
					r.y = pp.y;
				}
				if(m_nResizeSide&2){
					r.w = tp.x;
				}
				if(m_nResizeSide&4){
					r.h = tp.y;
				}
				if(m_nResizeSide&8){
					r.w += (r.x - pp.x);
					r.x = pp.x;
				}
				SetPosition(r.x, r.y);
				if(r.w<m_nMinWidth) r.w = m_nMinWidth;
				if(r.h<m_nMinHeight) r.h = m_nMinHeight;
				SetSize(r.w, r.h);
				return true;
			}
		}
	}

	return false;
}


void MWidget::InsertChild(MWidget* pWidget)
{
	m_Children.InsertBefore(pWidget);
	pWidget->m_pParent = this;
}

void MWidget::AddChild(MWidget* pWidget)
{
	m_Children.Add(pWidget);
	pWidget->m_pParent = this;
}

void MWidget::RemoveChild(MWidget* pWidget)
{
	for(int i=0; i<m_Children.GetCount(); i++){
		MWidget* pCurWnd = m_Children.Get(i);
		if(pCurWnd==pWidget){
			pWidget->m_pParent = NULL;
			m_Children.Delete(i);
			return;
		}
	}
}

void MWidget::AddExclusive(MWidget* pWidget)
{
	m_Exclusive.Add(pWidget);
}

bool MWidget::RemoveExclusive(MWidget* pWidget)
{
	for(int i=0; i<m_Exclusive.GetCount(); i++){
		MWidget* pThis = m_Exclusive.Get(i);
		if(pThis==pWidget){
			m_Exclusive.Delete(i);
			return true;
		}
	}
	return false;
}

MWidget* MWidget::GetLatestExclusive()
{
	if(m_Exclusive.GetCount()>0) return m_Exclusive.Get(m_Exclusive.GetCount()-1);
	return NULL;
}

MWidget::MWidget(const char* szName, MWidget* pParent, MListener* pListener)
{
	g_nWidgetCount++;
	m_nID = g_nWidgetCount;

	if(szName==NULL) m_szName[0] = NULL;
	else{
		if(strlen(szName)<MWIDGET_NAME_LENGTH){
			strcpy_safe(m_szName, szName);
		}
		else{
			memcpy(m_szName, szName, MWIDGET_NAME_LENGTH-4);
			m_szName[MWIDGET_NAME_LENGTH-4] = '.';
			m_szName[MWIDGET_NAME_LENGTH-3] = '.';
			m_szName[MWIDGET_NAME_LENGTH-2] = '.';
			m_szName[MWIDGET_NAME_LENGTH-1] = 0;
		}
	}

	// Default Region
	m_Rect.x = 0;
	m_Rect.y = 0;
	m_Rect.w = 100;
	m_Rect.h = 100;

	m_pParent = pParent;
	if(m_pParent!=NULL) m_pParent->AddChild(this);

	m_pListener = pListener;

	m_pCursor = NULL;		// NULL Cursor is Default Cursor
	m_pFont = NULL;			// NULL Font is Default Font

	m_bVisible = true;
	m_bEnable = true;
	m_bFocusEnable = false;	// Default Focus Disabled

	m_pToolTip = NULL;

	SetAccelerator(0);

	m_bZOrderChangable = false;
	m_bResizable = false;
	m_nResizeSide = 0;

	m_bClipByParent = true;

	m_nOpacity = 255;

	m_bEnableDesignerMode = true;
	m_nDMDragWidget = 0;
	m_bModifiedByDesigner = false;
	m_bAddedByDesigner = false;

	m_BoundsAlignment = MAM_NOTALIGN;

	m_nMinWidth = 10;
	m_nMinHeight = 10;

	m_bisListBox = false;

	m_nDebugType = 0;
	m_bEventAcceleratorCall = false;

	m_IDLRect = MRECT(-1,-1,-1,-1);

}

MWidget::~MWidget()
{
	ReleaseExclusive();

	for(int i=0; i<m_Children.GetCount(); i++){
		MWidget* pWidget = m_Children.Get(i);
		pWidget->m_pParent = NULL;
	}

	if(m_pParent!=NULL) m_pParent->RemoveChild(this);
	if(MWidget::m_pCapturedWidget==this) MWidget::m_pCapturedWidget = NULL;
	if(MWidget::m_pFocusedWidget==this) MWidget::m_pFocusedWidget = NULL;
	DetachToolTip();
}

void MWidget::OnRun()
{
}

void MWidget::OnDraw(MDrawContext* pDC)
{
	// Draw Nothing
	pDC->SetColor(MCOLOR(196, 196, 196));
	pDC->FillRectangle(GetInitialClientRect());
}

bool MWidget::OnEvent(MEvent* pEvent, MListener* pListener)
{
	return false;
}

void MWidget::Run()
{
	OnRun();

	for(int i=0; i<m_Children.GetCount(); i++){
		MWidget* pCurWnd = m_Children.Get(i);
		pCurWnd->Run();
	}
}

void MWidget::Draw(MDrawContext* pDC)
{
	if(m_bVisible==false) return;

	unsigned char nLastOpacity;
	nLastOpacity = pDC->GetOpacity();

	MRECT sr = GetScreenRect();
	pDC->SetOrigin(MPOINT(sr.x, sr.y));

	if(m_pFont!=NULL) pDC->SetFont(m_pFont);
	else pDC->SetFont(MFontManager::Get(NULL));

	pDC->SetOpacity((unsigned char)(nLastOpacity * (float)(m_nOpacity / 255.0f)));
	if(!IsEnable())
		pDC->SetOpacity((unsigned char)(pDC->GetOpacity()*0.70));

	bool bIntersect = true;
	MRECT rectScreen(0, 0, MGetWorkspaceWidth()-1, MGetWorkspaceHeight()-1);
	MRECT PrevClipRect;
	if(GetParent()!=NULL) {
		MRECT parentClipRect = MClientToScreen(GetParent(), GetParent()->GetClientRect());
		bIntersect = rectScreen.Intersect(&PrevClipRect,parentClipRect);
	}else
		PrevClipRect = rectScreen;

	MRECT CurrClipRect = GetScreenRect();
	MRECT IntersectClipRect;

	if(m_bClipByParent==true){
		if(PrevClipRect.Intersect(&IntersectClipRect, CurrClipRect)==true){
			MRECT test = IntersectClipRect;
			if(IntersectClipRect.w>0 && IntersectClipRect.h>0) {
				pDC->SetClipRect(IntersectClipRect);
				OnDraw(pDC);
			}
		}
	}
	else{
		pDC->SetClipRect(CurrClipRect);
		OnDraw(pDC);
	}

	for(int i=0; i<m_Children.GetCount(); i++){
		MWidget* pCurWnd = m_Children.Get(i);
		if(pCurWnd==GetLatestExclusive()) continue;
		if(pCurWnd != NULL ) pCurWnd->Draw(pDC);
	}
	if(GetLatestExclusive()!=NULL) GetLatestExclusive()->Draw(pDC);

	pDC->SetOpacity(nLastOpacity);
}

void MWidget::Redraw()
{
	Mint::GetInstance()->Update();
}

bool MWidget::Event(MEvent* pEvent)
{
	if(m_bVisible==false) return false;

	MRECT r = GetRect();
	r.x = r.y = 0;

	if(pEvent->nMessage==MWM_LBUTTONDOWN)
		int k=0;

	MEvent LocalEvent;
	MakeLocalEvent(&LocalEvent, pEvent);

	if(m_nDebugType==2){
		if(pEvent->nMessage==MWM_LBUTTONDOWN )
			int k=0;
	}

	if(r.InPoint(LocalEvent.Pos)==true && (MWidget::m_pCapturedWidget==NULL ||
		MWidget::m_pCapturedWidget==this) && IsVisible()==true){
		if(m_pCursor!=NULL) MCursorSystem::Set(m_pCursor);
		else MCursorSystem::Set(MCURSOR_ARROW);

		if(m_pToolTip!=NULL){
			SetZOrder(MZ_TOP);
			m_pToolTip->Show(true);
		}
	}
	else{
		if(m_pToolTip!=NULL){
			m_pToolTip->Show(false);
		}
	}

	if(m_bEnable==false) return false;

	if(GetLatestExclusive()!=NULL){
		if(GetLatestExclusive()->Event(pEvent)==true) return true;
		if(GetLatestExclusive()!=NULL) return false;
	}

	if(pEvent->nMessage==MWM_LBUTTONDOWN)
	{
		if ( r.InPoint(LocalEvent.Pos)==true)
		{
			SetFocus();
 			if((m_bFocusEnable)&&(m_bZOrderChangable==true)) SetZOrder(MZ_TOP);
		}
	}

	if(GetLatestExclusive()==NULL){
		for(int i=m_Children.GetCount()-1; i>=0; i--){
			MWidget* pCurWnd = m_Children.Get(i);
			if(pCurWnd->Event(pEvent)==true) {
				return true;
			}
		}
	}

	if(EventResize(pEvent)==true) return true;

	if(pEvent->nMessage==MWM_HOTKEY){
		if(OnHotKey(pEvent->nKey)==true) return true;
	}

	// Focused Event
	if(pEvent->nMessage==MWM_CHAR || pEvent->nMessage==MWM_KEYDOWN || pEvent->nMessage==MWM_IMECOMPOSE ||
		pEvent->nMessage==MWM_ACTIONKEYDOWN || pEvent->nMessage==MWM_ACTIONKEYUP || pEvent->nMessage==MWM_ACTIONPRESSED || pEvent->nMessage==MWM_ACTIONRELEASED){
		if (IsFocus()) {
			if(OnEvent(&LocalEvent, GetListener())==true) 
				return true;

			// Tab Key
			if(pEvent->nMessage==MWM_KEYDOWN && pEvent->nKey==VK_TAB){
				if(OnTab(!pEvent->GetShiftState())==true) 
					return true;
			}
		}
	}
	else{
		if((MWidget::m_pCapturedWidget==NULL || MWidget::m_pCapturedWidget==this))
			if(OnEvent(&LocalEvent, GetListener())==true) 
				return true;
	}

	if(pEvent->nMessage==MWM_LBUTTONUP) ReleaseCapture();

	return false;
}

bool MWidget::EventAccelerator(MEvent* pEvent)
{
	if(m_bVisible==false) return false;
	if(m_bEnable==false) return false;

	if(!(pEvent->nMessage==MWM_KEYDOWN || pEvent->nMessage==MWM_SYSKEYDOWN)) return false;

	if(pEvent->nMessage==MWM_KEYDOWN)
		if(MWidget::m_pFocusedWidget!=NULL)
			if(dynamic_cast<MEdit*>(MWidget::m_pFocusedWidget) != nullptr) return false;

	if(GetLatestExclusive()!=NULL){
		if(GetLatestExclusive()->EventAccelerator(pEvent)==true) return true;
		if(GetLatestExclusive()!=NULL) return false;
	}

	if(GetLatestExclusive()==NULL){
		for(int i=m_Children.GetCount()-1; i>=0; i--){
			MWidget* pCurWnd = m_Children.Get(i);
			if(pCurWnd->EventAccelerator(pEvent)==true) return true;
		}
	}

	char szKey[2] = {(char)pEvent->nKey, 0};
	if(m_nAccelerator==szKey[0]) {

		m_bEventAcceleratorCall = true;

		if(DefaultCommand()==true) {
			m_bEventAcceleratorCall = false;
			return true;
		}
		m_bEventAcceleratorCall = false;
	}

	return false;
}

bool MWidget::EventDefaultKey(MEvent* pEvent)
{
	if(m_bVisible==false) return false;
	if(m_bEnable==false) return false;

	if(!(pEvent->nMessage==MWM_KEYDOWN))return false;

	if(GetLatestExclusive()!=NULL){
		if(GetLatestExclusive()->EventDefaultKey(pEvent)==true) return true;
		if(GetLatestExclusive()!=NULL) return false;
	}

	if(GetLatestExclusive()==NULL){
		for(int i=m_Children.GetCount()-1; i>=0; i--){
			MWidget* pCurWnd = m_Children.Get(i);
			if(pCurWnd->EventDefaultKey(pEvent)==true) return true;
		}
	}

	if(strcmp(GetClassName(), MINT_BUTTON)!=0) return false;

	MButton* pButton = (MButton*)this;
	if((pButton->m_nKeyAssigned==MBKA_ENTER && pEvent->nKey==VK_RETURN) ||
		(pButton->m_nKeyAssigned==MBKA_ESC && pEvent->nKey==VK_ESCAPE))
		if(DefaultCommand()==true) return true;

	return false;
}

void MWidget::OnShow(bool bVisible)
{
	for(int i=m_Children.GetCount()-1; i>=0; i--){
		MWidget* pCurWnd = m_Children.Get(i);
		if(pCurWnd->m_bVisible==true) pCurWnd->OnShow(bVisible);
	}

	if(bVisible==true) OnShow();
	else OnHide();

	if(bVisible==false && MWidget::m_pFocusedWidget==this) ReleaseFocus();
}

bool MWidget::OnTab(bool bForward)
{
	MWidget* pParent = GetParent();
	if(pParent==NULL) return false;

	if(m_pParent->GetLatestExclusive()==this) return false;

	int nThisIndex = pParent->GetChildIndex(this);
	if(nThisIndex<0) return false;

	for(int i=0; i<pParent->GetChildCount(); i++){
		int nIndex = 0;
		if(bForward==true) nIndex = (nThisIndex+i+1) % pParent->GetChildCount();
		else nIndex = (nThisIndex+pParent->GetChildCount()-1-i) % pParent->GetChildCount();

		MWidget* pSibling = pParent->GetChild( nIndex );
		if(pSibling->IsFocusEnable()==true && pSibling!=this && pSibling->IsVisible()==true && pSibling->IsEnable()==true){
			pSibling->SetFocus();
			return true;
		}
	}

	return false;
}

bool MWidget::OnDrop(MWidget* pSender, MBitmap* pBitmap, const char* szString, const char* szItemString)
{
	return false;
}

void MWidget::ResizeChildrenByAnchor(int w, int h)
{
	for(int i=0; i<m_Children.GetCount(); i++){
		MWidget* pChild = m_Children.Get(i);
		if(pChild->GetBoundsAlignment()!=0){
			pChild->SetBoundsAlignment(pChild->GetBoundsAlignment(), w, h);
			continue;
		}

		MRECT r = pChild->m_Rect;
		if(pChild->m_Anchors.m_bLeft==true && pChild->m_Anchors.m_bRight==true){
			r.w += (w-m_Rect.w);
		}
		else if(pChild->m_Anchors.m_bRight==true){
			r.x += (w-m_Rect.w);
		}
		if(pChild->m_Anchors.m_bTop==true && pChild->m_Anchors.m_bBottom==true){
			r.h += (h-m_Rect.h);
		}
		else if(pChild->m_Anchors.m_bBottom==true){
			r.y += (h-m_Rect.h);
		}
		pChild->SetBounds(r);
	}
}

void MWidget::Show(bool bVisible, bool bModal)
{
	if(m_bVisible==bVisible){
		if(bModal==true){
			if(m_pParent!=NULL && m_pParent->GetLatestExclusive()==this)
				return;
		}
		else return;
	}

	m_bVisible = bVisible;

	if(bVisible==true && bModal==true) SetExclusive();
	else if(bVisible==false) {
		ReleaseExclusive();
		if(MWidget::m_pFocusedWidget==this) ReleaseFocus();
	}

	OnShow(bVisible);
}

void MWidget::Enable(bool bEnable)
{
	m_bEnable = bEnable;
}

bool MWidget::IsVisible()
{
	return m_bVisible;
}

bool MWidget::IsEnable()
{
	return m_bEnable;
}

void MWidget::SetResizable(bool bEnable)
{
	m_bResizable = bEnable;
}

bool MWidget::IsResizable()
{
	return m_bResizable;
}

void MWidget::SetListener(MListener* pListener)
{
	m_pListener = pListener;
}

MListener* MWidget::GetListener()
{
	return m_pListener;
}

int MWidget::GetID()
{
	return m_nID;
}

void MWidget::SetID(int nID)
{
	m_nID = nID;
}

void MWidget::SetText(const char* szText)
{
	if(szText==NULL) return;
	_ASSERT(strlen(szText)<MWIDGET_NAME_LENGTH);
	if (strlen(szText) >= MWIDGET_NAME_LENGTH) return;

	strcpy_safe(m_szName, szText);
	if(m_pToolTip!=NULL) if(m_pToolTip->IsUseParentName()==true) m_pToolTip->SetBounds();

	SetLabelAccelerator();
}

const char* MWidget::GetText()
{
	return m_szName;
}

void MWidget::SetCapture()
{
	MWidget::m_pCapturedWidget = this;
}

void MWidget::ReleaseCapture()
{
	MWidget::m_pCapturedWidget = NULL;
}

void MWidget::SetFocusEnable(bool bEnable)
{
	m_bFocusEnable = bEnable;
}

bool MWidget::IsFocusEnable()
{
	return m_bFocusEnable;
}

void MWidget::SetFocus()
{
	if(m_bFocusEnable==false) return;

	MWidget* pExDes = FindExclusiveDescendant();
	if(pExDes!=NULL) return;

	if(MWidget::m_pFocusedWidget==this) return;

	if(MWidget::m_pFocusedWidget!=NULL) MWidget::m_pFocusedWidget->OnReleaseFocus();

	MWidget::m_pFocusedWidget = this;
	OnSetFocus();
}

void MWidget::ReleaseFocus()
{
	if(MWidget::m_pFocusedWidget==this) OnReleaseFocus();
	MWidget::m_pFocusedWidget = NULL;
}

bool MWidget::IsFocus()
{
	if(MWidget::m_pFocusedWidget==this) return true;
	return false;
}

MWidget* MWidget::GetParent()
{
	return m_pParent;
}

int MWidget::GetChildCount()
{
	return m_Children.GetCount();
}

MWidget* MWidget::GetChild(int i)
{
	return m_Children.Get(i);
}

int MWidget::GetChildIndex(MWidget* pWidget)
{
	for(int i=0; i<m_Children.GetCount(); i++){
		MWidget* pChild = m_Children.Get(i);
		if(pChild==pWidget) return i;
	}

	return -1;
}

void MWidget::SetExclusive()
{
	if(m_pParent!=NULL){
		m_pParent->AddExclusive(this);
		SetFocus();
	}
}

void MWidget::ReleaseExclusive()
{
	if(m_pParent!=NULL)
		m_pParent->RemoveExclusive(this);
}

MCursor* MWidget::SetCursor(MCursor* pCursor)
{
	MCursor* pTemp = m_pCursor;
	m_pCursor = pCursor;
	return pTemp;
}

MCursor* MWidget::GetCursor()
{
	return m_pCursor;
}

MFont* MWidget::SetFont(MFont* pFont)
{
	MFont* pTemp = m_pFont;
	m_pFont = pFont;
	return pTemp;
}

MFont* MWidget::GetFont()
{
	if(m_pFont==NULL) return MFontManager::Get(NULL);
	else return m_pFont;
}

void MWidget::SetSize(int w, int h)
{
	if(w<0) w = 1;
	if(h<0) h = 1;

	if(w==m_Rect.w && h==m_Rect.h) return;

	ResizeChildrenByAnchor(w, h);

	m_Rect.w = w;
	m_Rect.h = h;

	OnSize(w, h);
}

void MWidget::SetSize(MSIZE& s)
{
	SetSize(s.w, s.h);
}

void MWidget::SetPosition(int x, int y)
{
	m_Rect.x = x;
	m_Rect.y = y;
}

void MWidget::SetPosition(const MPOINT& p)
{
	SetPosition(p.x, p.y);
}

void MWidget::SetBounds(const MRECT& r)
{
	SetBounds(r.x, r.y, r.w, r.h);
}

void MWidget::SetBounds(int x, int y, int w, int h)
{
	SetPosition(x, y);

	if(w<0) w = 1;
	if(h<0) h = 1;

	SetSize(w, h);
}

MPOINT MWidget::GetPosition()
{
	return MPOINT(m_Rect.x, m_Rect.y);
}

MRECT MWidget::GetRect()
{
	return m_Rect;
}

MRECT MWidget::GetIDLRect()
{
	return m_IDLRect;
}

void MWidget::GetBoundsAlignmentPosition(MPOINT* p, MAlignmentMode am, int w, int h)
{
	p->x = m_Rect.x;
	p->y = m_Rect.y;

	MSIZE s(w, h);
	if(w==-1){
		s.w = MGetWorkspaceWidth();
		if(GetParent()!=NULL) s.w = GetParent()->m_Rect.w;
	}
	if(h==-1){
		s.h = MGetWorkspaceHeight();
		if(GetParent()!=NULL) s.h = GetParent()->m_Rect.h;
	}

	if(m_BoundsAlignment&MAM_LEFT) p->x = 0;
	else if(m_BoundsAlignment&MAM_RIGHT) p->x = s.w-m_Rect.w;
	else if(m_BoundsAlignment&MAM_HCENTER) p->x = (s.w-m_Rect.w)/2;

	if(m_BoundsAlignment&MAM_LEFT) p->y = 0;
	else if(m_BoundsAlignment&MAM_BOTTOM) p->y = s.h-m_Rect.h;
	else if(m_BoundsAlignment&MAM_VCENTER) p->y = (s.h-m_Rect.h)/2;
}

void MWidget::SetBoundsAlignment(MAlignmentMode am, int w, int h)
{
	m_BoundsAlignment = am;

	MPOINT p;
	GetBoundsAlignmentPosition(&p, m_BoundsAlignment, w, h);

	m_Rect.x = p.x;
	m_Rect.y = p.y;
}

MAlignmentMode MWidget::GetBoundsAlignment()
{
	return m_BoundsAlignment;
}


MRECT MWidget::GetScreenRect() const
{
	if(m_pParent!=NULL){
		MRECT sr = m_pParent->GetScreenRect();
		MRECT r = m_Rect;
		r.Offset(sr.x, sr.y);
		return r;
	}

	return m_Rect;
}

void MWidget::AttachToolTip(const char* szToolTipString)
{
	DetachToolTip();
	m_pToolTip = new MToolTip(szToolTipString, this);
}

void MWidget::AttachToolTip(MToolTip* pToolTip)
{
	DetachToolTip();
	m_pToolTip = pToolTip;
}

void MWidget::DetachToolTip()
{
	if(m_pToolTip!=NULL) delete m_pToolTip;
	m_pToolTip = NULL;
}

MToolTip* MWidget::GetToolTip()
{
	return m_pToolTip;
}

void MWidget::SetAccelerator(int a)
{
	if(a==0) m_nAccelerator = -1;
	else m_nAccelerator = a;
}

void MWidget::SetLabelAccelerator()
{
	SetAccelerator(GetLabelAccelerator());
}

char MWidget::GetLabelAccelerator()
{
	char szAnd[2] = {GetAndChar(m_szName), 0};
	_strupr_s(szAnd);
	if(szAnd[0]==0) return -1;
	return szAnd[0];
}

char MWidget::GetToolTipAccelerator()
{
	if(m_pToolTip!=NULL){
		char szAnd[2] = {GetAndChar(m_pToolTip->m_szName), 0};
		_strupr_s(szAnd);
		if(szAnd[0]==0) return -1;
		return szAnd[0];
	}
	else{
		return -1;
	}
}

MRECT MWidget::GetClientRect()
{
	return MRECT(0, 0, m_Rect.w, m_Rect.h);
}

MRECT MWidget::GetInitialClientRect()
{
	return MWidget::GetClientRect();
}

void MWidget::SetZOrder(MZOrder z)
{
	if(m_pParent==NULL) return;

	MWidget* pParent = m_pParent;
	pParent->RemoveChild(this);

	switch(z){
	case MZ_TOP:
		pParent->AddChild(this);
		break;
	case MZ_BOTTOM:
		pParent->InsertChild(this);
		break;
	}
}

MWidget* MWidget::FindExclusiveDescendant()
{
	if(m_Exclusive.GetCount()>0) return m_Exclusive.Get(m_Exclusive.GetCount()-1);

	for(int i=0; i<m_Children.GetCount(); i++){
		MWidget* pChild = m_Children.Get(i);
		MWidget* pExDes = pChild->FindExclusiveDescendant();
		if(pExDes!=NULL) return pExDes;
	}

	return NULL;
}

bool MWidget::IsExclusive(MWidget* pWidget)
{
	for(int i=0; i<m_Exclusive.GetCount(); i++){
		MWidget* pThis = m_Exclusive.Get(i);
		if(pThis==pWidget) return true;
	}
	return false;
}


MWidget* MWidget::Find(const MPOINT& p)
{
	if(IsVisible()==false) return NULL;

	for(int i=0; i<m_Children.GetCount(); i++){
		MWidget* pChild = m_Children.Get(m_Children.GetCount()-i-1);
		MWidget* pFind = pChild->Find(p);
		if(pFind!=NULL) return pFind;
	}

	if(GetScreenRect().InPoint(p)==true)
		return this;

	return NULL;
}

MWidget* MWidget::FindDropAble(MPOINT& p)
{
	if(IsVisible()==false) return NULL;

	for(int i=0; i<m_Children.GetCount(); i++){
		MWidget* pChild = m_Children.Get(m_Children.GetCount()-i-1);
		MWidget* pFind = pChild->FindDropAble(p);
		if(pFind!=NULL) return pFind;
	}

	if(GetScreenRect().InPoint(p)==true)
		if(m_bFocusEnable==true)
			return this;

	return NULL;
}


bool MWidget::Drop(MWidget* pSender, MBitmap* pBitmap, const char* szString, const char* szItemString)
{
	return OnDrop(pSender, pBitmap, szString, szItemString);
}

void MWidget::GetHierarchicalName(char* szName, int maxlen)
{
	if(m_pParent!=NULL){
		m_pParent->GetHierarchicalName(szName, maxlen);
		strcat_s(szName, maxlen, "/");
		strcat_s(szName, maxlen, m_szName);
	}
	else{
		strcpy_safe(szName, maxlen, m_szName);
	}
}

MWidget* MWidget::FindWidgetByHierarchicalName(const char* szName)
{
	char szHierachicalName[2048];
	GetHierarchicalName(szHierachicalName);
	if(strcmp(szName, szHierachicalName)==0){
		return this;
	}

	for(int i=0; i<GetChildCount(); i++){
		MWidget* pFind = GetChild(i)->FindWidgetByHierarchicalName(szName);
		if(pFind!=NULL) return pFind;
	}

	return NULL;
}

bool MWidget::IsMsg(const char* szMsg1, const char* szMsg2)
{
	char szTmp1[256];
	char szTmp2[256];
	strcpy_safe(szTmp1, szMsg1);
	strcpy_safe(szTmp2, szMsg2);
	_strupr_s(szTmp1);
	_strupr_s(szTmp2);
	if(strcmp(szTmp1, szTmp2)==0) return true;
	return false;
}

void* MWidget::Query(const char* szQuery)
{
	return NULL;
}

void MWidget::SetOpacity(unsigned char nOpacity)
{
	m_nOpacity = nOpacity;
}

unsigned char MWidget::GetOpacity() const
{
	return m_nOpacity;
}

int GetAndPos(const char* szText)
{
	if(szText==NULL) return -1;
	int nLen = strlen(szText);
	for(int i=0; i<nLen; i++){
		if(szText[i]=='&') return i;
	}
	return -1;
}

char GetAndChar(const char* szText)
{
	int nLen = GetAndPos(szText);
	if(nLen==-1) return 0;
	return szText[nLen+1];
}

int RemoveAnd(char* szText)
{
	if(szText==NULL) return -1;
	int nPos = GetAndPos(szText);
	if(nPos==-1) return -1;
	int nLen = strlen(szText);
	for(int i=0; i<(nLen-nPos); i++){
		szText[nPos+i] = szText[nPos+i+1];
	}
	return nPos;
}

int RemoveAnd(char* szRemovedText, int maxlen, const char* szText)
{
	if(szText==NULL) return -1;
	strcpy_safe(szRemovedText, maxlen, szText);
	return RemoveAnd(szRemovedText);
}

int RemoveAnd(char* szRemovedFrontText, int maxlen, char* cUnderLineChar, char* szRemovedBackText, const char* szText)
{
	if(szText==NULL) return -1;
	int nPos = GetAndPos(szText);
	if(nPos==-1) return -1;
	memcpy(szRemovedFrontText, szText, nPos);
	szRemovedFrontText[nPos] = 0;
	*cUnderLineChar = szText[nPos+1];
	strcpy_safe(szRemovedBackText, maxlen, szText+nPos+2);
	return nPos;
}

MPOINT MClientToScreen(const MWidget* pWidget, const MPOINT& p)
{
	MRECT r = pWidget->GetScreenRect();
	return MPOINT(p.x+r.x, p.y+r.y);
}

MPOINT MScreenToClient(const MWidget* pWidget, const MPOINT& p)
{
	MRECT r = pWidget->GetScreenRect();
	return MPOINT(p.x-r.x, p.y-r.y);
}

MRECT MClientToScreen(const MWidget* pWidget, const MRECT& p)
{
	MRECT r = pWidget->GetScreenRect();
	return MRECT(p.x+r.x, p.y+r.y, p.w, p.h);
}

MRECT MScreenToClient(const MWidget* pWidget, const MRECT& p)
{
	MRECT r = pWidget->GetScreenRect();
	return MRECT(p.x-r.x, p.y-r.y, p.w, p.h);
}