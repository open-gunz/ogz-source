#include "stdafx.h"
#include "MListBox.h"
#include "MColorTable.h"
#include "Mint.h"
#include <algorithm>

#define MLISTBOX_MARGIN_X	2
#define MLISTBOX_MARGIN_Y	2

#define MLISTBOX_DEFAULT_WIDTH	100
#define MLISTBOX_DEFAULT_HEIGHT	100
#define MLISTBOX_ITEM_MARGIN_Y	2
#define MLISTBOX_ITEM_MARGIN_X	2

#define COLORTEXT_SUPPORT

IMPLEMENT_LOOK(MListBox, MListBoxLook)

int MListBox::GetItemHeight()
{
	if(m_nItemHeight>0) return m_nItemHeight;
	return GetFont()->GetHeight()+MLISTBOX_ITEM_MARGIN_Y;
}

void MListBox::SetItemHeight(int nHeight)
{
	m_nItemHeight = nHeight;
}

int MListBox::FindItem(MPOINT& p)
{
	int nItemHeight = GetItemHeight();

	int nHeaderHeight = 0;
	if(IsVisibleHeader()==true) nHeaderHeight = nItemHeight;

	MRECT r = GetClientRect();
	if (m_ViewStyle == MVS_LIST)
	{
		for(int i=0; i<GetCount()-m_nStartItemPos; i++){
			if(!IsShowItem(i+m_nStartItemPos)) break;
			if(MRECT(r.x, r.y+nHeaderHeight+nItemHeight*i, r.w, nItemHeight).InPoint(p)==true){
				return (i+m_nStartItemPos);
			}
		}
	}
	else if (m_ViewStyle == MVS_ICON)
	{
		int nColCount = r.w / GetTabSize();
		int nColIndex, nRowIndex;
		nColIndex = p.x / GetTabSize();
		nRowIndex = p.y / GetItemHeight();
		int nItemIndex = m_nStartItemPos + (nRowIndex * nColCount) + nColIndex;
		if (nItemIndex < GetCount()) return nItemIndex;
	}
	return -1;
}

bool MListBox::GetItemPos(MPOINT* p, int i)
{
	int nFontHeight = GetItemHeight();

	int nHeaderHeight = 0;
	if(IsVisibleHeader()==true) nHeaderHeight = nFontHeight;

	MRECT r = GetClientRect();
	p->x = r.x;
	p->y = r.y + nHeaderHeight + (i-m_nStartItemPos) * nFontHeight;

	if(i>=m_nStartItemPos && i<m_nStartItemPos+m_nShowItemCount) return true;
	return false;
}

bool MListBox::OnEvent(MEvent* pEvent, MListener* pListener)
{
 	MRECT r = GetClientRect();
	if(pEvent->nMessage==MWM_MOUSEMOVE){
		if(r.InPoint(pEvent->Pos)==false) return false;
		m_nOverItem = FindItem(pEvent->Pos);
	}
	else if(pEvent->nMessage==MWM_LBUTTONDOWN){
		if(r.InPoint(pEvent->Pos)==false)
		{
			pListener->OnCommand(this,MLB_ITEM_CLICKOUT);
			return false;
		}

		if(m_nDebugType==2){
			int k =0;
		}

		int nSelItem = FindItem(pEvent->Pos);
		if(nSelItem==-1) return true;
		SetSelIndex(nSelItem);

		if(pListener!=NULL) pListener->OnCommand(this, MLB_ITEM_SEL);

		if ( m_bDragAndDrop)
		{
			MListItem* pItem = GetSelItem();
			if(pItem!=NULL)
			{
				MBitmap* pDragBitmap = NULL;
				char szDragString[256] = "";
				char szDragItemString[256] = "";
				if(pItem->GetDragItem(&pDragBitmap, szDragString, szDragItemString)==true){
					Mint::GetInstance()->SetDragObject(this, pDragBitmap, szDragString, szDragItemString);
				}
			}
		}

		return true;
	}
	else if(pEvent->nMessage==MWM_RBUTTONDOWN){
		if(r.InPoint(pEvent->Pos)==false)
		{
			pListener->OnCommand(this,MLB_ITEM_CLICKOUT);
			return false;
		}
		int nSelItem = FindItem(pEvent->Pos);
		if(nSelItem==-1) return true;
		SetSelIndex(nSelItem);

		if(m_nSelItem!=-1){
			if(pListener!=NULL) pListener->OnCommand(this, MLB_ITEM_SEL2);
			return true;
		}
	}
	else if(pEvent->nMessage==MWM_KEYDOWN){
		if(pEvent->nKey==VK_DELETE){
			if(pListener!=NULL) pListener->OnCommand(this, MLB_ITEM_DEL);
		}
		else if(pEvent->nKey==VK_UP){
			if(GetSelIndex()>0){
				SetSelIndex(GetSelIndex()-1);
				ShowItem(GetSelIndex());
				if(pListener!=NULL){
					if(m_nSelItem!=-1) pListener->OnCommand(this, MLB_ITEM_SEL);
					else pListener->OnCommand(this, MLB_ITEM_SELLOST);
				}
			}
		}
		else if(pEvent->nKey==VK_DOWN){
			if(GetSelIndex()<GetCount()-1){
				SetSelIndex(GetSelIndex()+1);
				ShowItem(GetSelIndex());
				if(pListener!=NULL){
					if(m_nSelItem!=-1) pListener->OnCommand(this, MLB_ITEM_SEL);
					else pListener->OnCommand(this, MLB_ITEM_SELLOST);
				}
			}
		}
	}
	else if(pEvent->nMessage==MWM_CHAR){
		int nIndex = FindNextItem(GetSelIndex(), pEvent->nKey);
		if(nIndex>=0){
			SetSelIndex(nIndex);
			ShowItem(nIndex);
			if(pListener!=NULL){
				if(m_nSelItem!=-1) pListener->OnCommand(this, MLB_ITEM_SEL);
				else pListener->OnCommand(this, MLB_ITEM_SELLOST);
			}
		}
	}
	else if(pEvent->nMessage==MWM_LBUTTONDBLCLK){
		if(r.InPoint(pEvent->Pos)==false) return false;
		int nSelItem = FindItem(pEvent->Pos);
		if(nSelItem==-1) return true;
		m_nSelItem = nSelItem;
		if(pListener!=NULL){
			if(m_nSelItem!=-1) pListener->OnCommand(this, MLB_ITEM_SEL);
			else pListener->OnCommand(this, MLB_ITEM_SELLOST);
			pListener->OnCommand(this, MLB_ITEM_DBLCLK);
		}
		return true;
	}
	else if(pEvent->nMessage==MWM_MOUSEWHEEL){
		if(r.InPoint(pEvent->Pos)==false) return false;
#define MAX_WHEEL_RANGE	4
		if (m_ViewStyle == MVS_LIST)
			SetStartItem(m_nStartItemPos + std::min(std::max(-pEvent->nDelta, -MAX_WHEEL_RANGE),
				MAX_WHEEL_RANGE));
		else if (m_ViewStyle == MVS_ICON)
		{
			int nTabSize = GetTabSize();
			int nColCount = r.w / nTabSize;
			int t = (m_nStartItemPos + std::min(std::max(-pEvent->nDelta, -MAX_WHEEL_RANGE),
				MAX_WHEEL_RANGE)) * nColCount;
			SetStartItem((m_nStartItemPos + std::min(std::max(-pEvent->nDelta, -MAX_WHEEL_RANGE),
				MAX_WHEEL_RANGE)) * nColCount);
		}
		return true;
	}
	return false;
}

void MListBox::RecalcList()
{
	int nItemHeight = GetItemHeight();
	MRECT r = GetClientRect();

	if (m_ViewStyle == MVS_LIST)
	{
		int nHeaderHeight = 0;
		if(IsVisibleHeader()==true) nHeaderHeight = nItemHeight;

		m_nShowItemCount = (r.h-nHeaderHeight) / nItemHeight;
	}
	else if (m_ViewStyle == MVS_ICON)
	{
		int nTabSize = GetTabSize();
		m_nShowItemCount = (r.w / nTabSize) * (r.h / nItemHeight);
	}

	RecalcScrollBar();
}

void MListBox::RecalcScrollBar()
{
	if(m_nShowItemCount<GetCount())
	{
		m_pScrollBar->SetMinMax(0, GetCount()-m_nShowItemCount);

		if( !m_bHideScrollBar ) {
			m_pScrollBar->Show(true);
		}

		if(m_bAlwaysVisibleScrollbar)
			m_pScrollBar->Enable(true);
	}
	else{
		m_pScrollBar->SetMinMax(0, 0);

		if(m_bAlwaysVisibleScrollbar)
			m_pScrollBar->Enable(false);
		else
			m_pScrollBar->Show(false);
	}

}

int MListBox::FindNextItem(int i, char c)
{
	for(int s=0; s<GetCount()-1; s++){
		int idx = (i+s+1)%GetCount();
		const char* szItem = GetString(idx);

		if (szItem != NULL)
		{
			char a = (char)towupper(szItem[0]);
			char b = (char)towupper(c);
			if(a==b) return idx;
		}
	}
	return -1;
}

void MListBox::OnSize(int w, int h)
{
	MRECT cr = GetInitialClientRect();
	if (m_pScrollBar->IsVisible() == true)
		m_pScrollBar->SetBounds(MRECT(cr.x + cr.w - m_pScrollBar->GetDefaultBreadth(), cr.y + 1,
			m_pScrollBar->GetDefaultBreadth(), cr.h - 1));
	else
		m_pScrollBar->SetBounds(MRECT(cr.x + cr.w - m_pScrollBar->GetDefaultBreadth(), cr.y + 1,
			m_pScrollBar->GetDefaultBreadth(), cr.h - 1));

	RecalcList();
}

void MListBox::Initialize()
{
	m_nOverItem = -1;
	m_nSelItem = -1;
	m_nStartItemPos = 0;
	m_nShowItemCount = 0;
	m_nItemHeight = -1;
	m_bSelected = true;
	m_pScrollBar = new MScrollBar(this, this);
	m_ViewStyle = MVS_LIST;
	m_bDragAndDrop = false;
	m_pOnDropFunc = NULL;

	SetSize(MLISTBOX_DEFAULT_WIDTH, MLISTBOX_DEFAULT_HEIGHT);

	SetFocusEnable(true);

	m_bVisibleHeader = true;

	m_bAbsoulteTabSpacing = true;
	m_bAlwaysVisibleScrollbar = false;
	m_bisListBox = true;
	m_bHideScrollBar = false;
	m_FontColor = MCOLOR(DEFCOLOR_MLIST_TEXT);
	m_FontAlign = MAM_NOTALIGN;

	m_bNullFrame = false;
	m_bMultiSelect = false;
}


MListBox::MListBox(const char* szName, MWidget* pParent, MListener* pListener)
: MWidget(szName, pParent, pListener)
{
	Initialize();
}

MListBox::MListBox(MWidget* pParent, MListener* pListener)
: MWidget("MListBox", pParent, pListener)
{
	Initialize();
}

MListBox::~MListBox()
{
	m_Items.DeleteAll();
	if(m_pScrollBar!=NULL) delete m_pScrollBar;
}

void MListBox::Add(const StringView& szItem)
{
	MListItem* pItem = new MDefaultListItem(szItem);
	m_Items.Add(pItem);
	if(m_Items.GetCount()) {
		if(m_nSelItem == -1)
			m_nSelItem = 0;
	}

	RecalcList();
	RecalcScrollBar();
}

void MListBox::Add(const StringView& szItem, MCOLOR color)
{
	MListItem* pItem = new MDefaultListItem(szItem, color);

	m_Items.Add(pItem);

	RecalcList();
	RecalcScrollBar();
}

void MListBox::Add(MListItem* pItem)
{
	m_Items.Add(pItem);
	RecalcList();
	RecalcScrollBar();
}

const char* MListBox::GetString(int i)
{
	return m_Items.Get(i)->GetString();
}

MListItem* MListBox::Get(int i)
{
	return m_Items.Get(i);
}

bool MListBox::Set(int i, const char* szItem)
{
	if(m_Items.GetCount()<=i) return false;
	MListItem* pItem = m_Items.Get(i);
	pItem->SetString(szItem);
	return true;
}

bool MListBox::Set(int i, MListItem* pItem)
{
	if(m_Items.GetCount()<=i) return false;
	m_Items.MoveRecord(i);
	m_Items.Insert(pItem);
	m_Items.Delete(i);
	return true;
}

void MListBox::Remove(int i)
{
	if(i<0 || i>=m_Items.GetCount()) return;
	m_Items.Delete(i);
	if(i==m_nSelItem) m_nSelItem = -1;

	RecalcScrollBar();
}

void MListBox::RemoveAll()
{
	while(m_Items.GetCount()>0) Remove(0);

	m_nOverItem = -1;
	m_nSelItem = -1;
	m_nStartItemPos = 0;
	m_pScrollBar->SetValue(0);
	RecalcScrollBar();
}

bool MListBox::Swap(int i, int j)
{
	if(i<0 || j<0 || i>=m_Items.GetCount() || j>=m_Items.GetCount()) return false;
	m_Items.Swap(i, j);
	return true;
}

int MListBox::GetCount()
{
	return m_Items.GetCount();
}

int MListBox::GetSelIndex()
{
	return m_nSelItem;
}

const char* MListBox::GetSelItemString()
{
	if(m_nSelItem==-1) return NULL;
	if(m_nSelItem>=m_Items.GetCount()) return NULL;
	return m_Items.Get(m_nSelItem)->GetString();
}

MListItem* MListBox::GetSelItem()
{
	if(m_nSelItem==-1) return NULL;
	return m_Items.Get(m_nSelItem);
}

bool MListBox::SetSelIndex(int i)
{
	if ((i >= m_Items.GetCount()) || (i<0))
		return false;

	if(!m_bMultiSelect) {
		if(0<=m_nSelItem && m_nSelItem<m_Items.GetCount())
			Get(m_nSelItem)->m_bSelected=false;
	}

	m_nSelItem = i;

	if(!m_bMultiSelect) {
		Get(m_nSelItem)->m_bSelected=true;
	}else
		Get(m_nSelItem)->m_bSelected=!Get(m_nSelItem)->m_bSelected;

	return true;
}

bool MListBox::IsShowItem(int i)
{
	if(i>=m_nStartItemPos && i<m_nStartItemPos+m_nShowItemCount) return true;
	return false;
}

void MListBox::ShowItem(int i)
{
	if(i<m_nStartItemPos){
		m_nStartItemPos = i;
		m_pScrollBar->SetValue(m_nStartItemPos);
	}
	else if(i>=m_nStartItemPos+m_nShowItemCount){
		m_nStartItemPos = i - m_nShowItemCount + 1;
		if(m_nStartItemPos+m_nShowItemCount>GetCount()) m_nStartItemPos = GetCount()-m_nShowItemCount;
		m_pScrollBar->SetValue(m_nStartItemPos);
	}
}

void MListBox::SetStartItem(int i)
{
	if(GetCount()<=m_nShowItemCount) return;

	if(i<0) i = 0;
	else if(i+m_nShowItemCount>GetCount()) i = GetCount()-m_nShowItemCount;
	m_nStartItemPos = i;
	m_pScrollBar->SetValue(m_nStartItemPos);
	GetListener()->OnCommand(this, MLB_ITEM_START);
}

int MListBox::GetStartItem()
{
	return m_nStartItemPos;
}

int MListBox::GetShowItemCount()
{
	return m_nShowItemCount;
}

MScrollBar* MListBox::GetScrollBar()
{
	return m_pScrollBar;
}

void MListBox::Sort(bool bAscend)
{
	m_Items.m_bAscend = bAscend;
	m_Items.Sort();
}

bool MListBox::OnCommand(MWidget* pWindow, const char* szMessage)
{
	if(pWindow==m_pScrollBar && strcmp(szMessage, MLIST_VALUE_CHANGED)==0)
	{
		if (m_ViewStyle == MVS_LIST)
		{
			m_nStartItemPos = m_pScrollBar->GetValue();
		}
		else if (m_ViewStyle == MVS_ICON)
		{
			MRECT r = GetClientRect();
			int nColCount = r.w / GetTabSize();
			int nValue = m_pScrollBar->GetValue() - (m_pScrollBar->GetValue() % nColCount);
			m_nStartItemPos = nValue;
		}
		return true;
	}
	return false;
}

void MListBox::AddField(const char* szFieldName, int nTabSize)
{
	bool bVisibleHeader = IsVisibleHeader();
	MLISTFIELD* pNew = new MLISTFIELD;
	strcpy_safe(pNew->szFieldName, szFieldName);
	pNew->nTabSize = nTabSize;
	m_Fields.Add(pNew);
	if(bVisibleHeader!=IsVisibleHeader()) RecalcList();
}

void MListBox::RemoveField(const char* szFieldName)
{
	bool bVisibleHeader = IsVisibleHeader();
	for(int i=0; i<m_Fields.GetCount(); i++){
		MLISTFIELD* pField = m_Fields.Get(i);
		if(strcmp(pField->szFieldName, szFieldName)==0){
			m_Fields.Delete(i);
			if(bVisibleHeader!=IsVisibleHeader()) RecalcList();
			return;
		}
	}
}

void MListBox::RemoveAllField()
{
	while(m_Fields.GetCount()){
		m_Fields.Delete(0);
	}
}

MLISTFIELD* MListBox::GetField(int i)
{
	return m_Fields.Get(i);
}

int MListBox::GetFieldCount()
{
	return m_Fields.GetCount();
}

bool MListBox::IsVisibleHeader()
{
	if(GetFieldCount()==0) return false;

	return m_bVisibleHeader;
}

void MListBox::SetVisibleHeader(bool bVisible)
{
	m_bVisibleHeader = bVisible;
}

void MListBox::SetViewStyle(MListViewStyle ViewStyle)
{
	m_ViewStyle = ViewStyle;
	if (ViewStyle == MVS_ICON)
	{
		SetVisibleHeader(false);
	}
	RecalcList();
	RecalcScrollBar();
}

int MListBox::GetTabSize()
{
	if (GetFieldCount() > 0)
	{
		return GetField(0)->nTabSize;
	}

	return GetItemHeight();
}

void MListBoxLook::OnHeaderDraw(MDrawContext* pDC, MRECT& r, const char* szText)
{
	if(szText==NULL) return;
	pDC->SetColor(MCOLOR(m_SelectedTextColor));
	pDC->Text( r, szText );
}

int MListBoxLook::OnItemDraw(MDrawContext* pDC, MRECT& r, const char* szText, MCOLOR color, bool bSelected, bool bFocus, int nAdjustWidth )
{
	int nLine = 0;

	if(szText==NULL) return nLine;

	if(bSelected==true){
		if(bFocus==true) pDC->SetColor(MCOLOR(m_SelectedPlaneColor));
		else pDC->SetColor(MCOLOR(m_UnfocusedSelectedPlaneColor));
		pDC->FillRectangle(r);
	}

	if(bSelected==true) pDC->SetColor(MCOLOR(m_SelectedTextColor));
	else pDC->SetColor(color);

	if( m_pItemSlotBitmap != NULL )
	{
		pDC->SetBitmap( m_pItemSlotBitmap );
		pDC->Draw(r.x, r.y, r.w, r.h );
	}

	MRECT rtemp, rtemp2;
	rtemp2 = rtemp = pDC->GetClipRect();
	rtemp2.w -= nAdjustWidth;
	pDC->SetClipRect(rtemp2);
#ifdef COLORTEXT_SUPPORT
	if( m_ItemTextAlignmentMode == MAM_NOTALIGN )
	{
		nLine = pDC->TextMultiLine(r, szText,0, m_bItemTextMultiLine );
	}
	else
	{
		nLine = pDC->TextMultiLine2( r, szText, 0, m_bItemTextMultiLine, m_ItemTextAlignmentMode );
	}
#else
	pDC->Text(r.x, r.y+(r.h-pDC->GetFont()->GetHeight())/2, szText);
	nLine	= 1;
#endif
	pDC->SetClipRect(rtemp);

	return nLine;
}

int MListBoxLook::OnItemDraw(MDrawContext* pDC, MRECT& r, MBitmap* pBitmap, bool bSelected, bool bFocus, int nAdjustWidth)
{
	int nLine = 0;

	if(pBitmap==NULL) return nLine;


	if(bSelected==true){
		if(bFocus==true) pDC->SetColor(MCOLOR(m_SelectedPlaneColor));
		else pDC->SetColor(MCOLOR(m_UnfocusedSelectedPlaneColor));
		pDC->FillRectangle(r);
	}


	MRECT rtemp, rtemp2;
	rtemp2 = rtemp = pDC->GetClipRect();
	rtemp2.w -= nAdjustWidth;
	pDC->SetClipRect(rtemp2);

	pDC->SetBitmap(pBitmap);

	int drawWidth, drawHeight, paddingX, paddingY;
	float fRatio = static_cast<float>(pBitmap->GetWidth()) / static_cast<float>(pBitmap->GetHeight());
	if (fRatio > 1.0) {
		drawWidth = nAdjustWidth;
		drawHeight = int(ceil(nAdjustWidth / fRatio));
	}
	else {
		drawWidth = int(ceil(r.h * fRatio));
		drawHeight = r.h;
	}
	paddingX = int(ceil((nAdjustWidth - drawWidth) / 2));
	paddingY = int(ceil((r.h - drawHeight) / 2));

	pDC->Draw(r.x + paddingX, r.y + paddingY, drawWidth, drawHeight);

	pDC->SetClipRect(rtemp);

	nLine = 1;

	return nLine;
}

void MListBoxLook::OnFrameDraw(MListBox* pListBox, MDrawContext* pDC)
{
	MRECT r = pListBox->GetInitialClientRect();
	pDC->SetColor(MCOLOR(DEFCOLOR_MLIST_PLANE));
	pDC->FillRectangle(r);
	pDC->SetColor(MCOLOR(DEFCOLOR_MLIST_OUTLINE));
	pDC->Rectangle(r);
}

void MListBoxLook::OnDraw(MListBox* pListBox, MDrawContext* pDC)
{
	int nLine = 0;

	if(pListBox->m_nDebugType==2){
		int k =0;
	}
	if(!pListBox->m_bNullFrame)
		OnFrameDraw(pListBox, pDC);

	int nItemHeight = pListBox->GetItemHeight();
	int nShowCount = 0;

	MRECT r = pListBox->GetClientRect();

	int nHeaderHeight = 0;

	pDC->SetFont( pListBox->GetFont() );
	m_ItemTextAlignmentMode = pListBox->m_FontAlign;

	if(pListBox->IsVisibleHeader()==true){
		int nFieldStartX = 0;
		for(int i=0; i<pListBox->GetFieldCount(); i++){
			MLISTFIELD* pField = pListBox->GetField(i);
			int nWidth = std::min(pField->nTabSize, r.w-nFieldStartX);
			if(pListBox->m_bAbsoulteTabSpacing==false) nWidth = r.w*pField->nTabSize/100;
			MRECT ir(r.x+nFieldStartX, r.y, nWidth, nItemHeight);
			OnHeaderDraw(pDC, ir, pField->szFieldName);
			nFieldStartX += (nWidth+1);
			if(nFieldStartX>=r.w) break;
		}
		nHeaderHeight = nItemHeight;
	}

	if (pListBox->GetViewStyle() == MVS_LIST)
	{
		for(int i=pListBox->GetStartItem(); i<pListBox->GetCount(); i++){
			MPOINT p;
			p.x = r.x;
			p.y = r.y+nHeaderHeight+nItemHeight*nShowCount;

			MListItem* pItem = pListBox->Get(i);
			bool bSelected = pItem->m_bSelected;
			bool bFocused = (pListBox->IsFocus());

			int nFieldStartX = 0;
			for (int j = 0; j < std::max(pListBox->GetFieldCount(), 1); j++) {

				int nTabSize = r.w;
				if (j < pListBox->GetFieldCount()) nTabSize = pListBox->GetField(j)->nTabSize;

				int nWidth = std::min(nTabSize, r.w-nFieldStartX);
				if(pListBox->m_bAbsoulteTabSpacing==false) nWidth = r.w*nTabSize/100;

				int nAdjustWidth = 0;
				if(pListBox->GetScrollBar()->IsVisible()){
					nAdjustWidth = pListBox->GetScrollBar()->GetRect().w + pListBox->GetScrollBar()->GetRect().w/2;
				}
				MRECT ir(p.x+nFieldStartX, p.y, nWidth, nItemHeight);
				const char* szText = pItem->GetString(j);
				MBitmap* pBitmap = pItem->GetBitmap(j);

				if(pBitmap!=NULL)
					OnItemDraw(pDC, ir, pBitmap,  bSelected, bFocused, nWidth);

 				MCOLOR color;
				if( (MCOLOR(pListBox->m_FontColor)).GetARGB() == DEFCOLOR_MLIST_TEXT )
					color = pItem->GetColor();
				else
					color = pListBox->m_FontColor;

				if(szText!=NULL && szText[0]!=0 )
				{
					nLine = OnItemDraw(pDC, ir, szText,color,bSelected, bFocused, nAdjustWidth);
				}

				nFieldStartX += nWidth;
				if(nFieldStartX>=r.w) break;
			}

			nShowCount += nLine;

			if(nShowCount>=pListBox->GetShowItemCount()) break;
		}
	}
	else if (pListBox->GetViewStyle() == MVS_ICON)
	{
		MPOINT p;
		p.x = r.x;
		p.y = r.y + nHeaderHeight;

		int nStartX = 0, nStartY = 0;
		MSIZE TabSize(r.w, r.h);

		if(pListBox->GetFieldCount() > 0)
		{
			TabSize.w = pListBox->GetField(0)->nTabSize;
			TabSize.h = pListBox->GetItemHeight();
		}

		for(int i=pListBox->GetStartItem(); i<pListBox->GetCount(); i++)
		{
			MListItem* pItem = pListBox->Get(i);
			bool bSelected = pItem->m_bSelected;
			bool bFocused = (pListBox->IsFocus());

			int nWidth = std::min(TabSize.w, r.w - nStartX);

			int nAdjustWidth = 0;
			if(pListBox->GetScrollBar()->IsVisible())
			{
				nAdjustWidth = pListBox->GetScrollBar()->GetRect().w + pListBox->GetScrollBar()->GetRect().w/2;
			}

			MRECT ir(p.x+nStartX, p.y+nStartY, nWidth, nItemHeight);
			const char* szText = pItem->GetString(0);
			MBitmap* pBitmap = pItem->GetBitmap(0);
			const MCOLOR color = pItem->GetColor(0);
			if(szText!=NULL)
				OnItemDraw(pDC, ir, szText, color, bSelected, bFocused, nAdjustWidth);
			else if(pBitmap!=NULL)
				OnItemDraw(pDC, ir, pBitmap,  bSelected, bFocused, nAdjustWidth);

			nStartX += TabSize.w;
			if(nStartX >= (r.w - TabSize.w ))
			{
				nStartX = 0;
				nStartY += nItemHeight + 5;

			}

			nShowCount++;
			if(nShowCount>=pListBox->GetShowItemCount()) break;
		}
	}
}

MRECT MListBoxLook::GetClientRect(MListBox* pListBox, const MRECT& r)
{
	return MRECT(r.x+1, r.y+1,
		(pListBox->GetScrollBar()->IsVisible()==true)?(r.w - pListBox->GetScrollBar()->GetClientRect().w-2):(r.w-2),
		r.h-2);
}

MListBoxLook::MListBoxLook()
{
	m_SelectedPlaneColor = DEFCOLOR_MLIST_SELECTEDPLANE;
	m_SelectedTextColor = DEFCOLOR_MLIST_SELECTEDTEXT;
	m_UnfocusedSelectedPlaneColor = DEFCOLOR_DARK;
	m_ItemTextAlignmentMode	= MAM_NOTALIGN;
	m_bItemTextMultiLine			= false;
	m_pItemSlotBitmap				= NULL;
}

bool MListBox::IsAlwaysVisibleScrollbar()
{
	return m_bAlwaysVisibleScrollbar;
}

void MListBox::SetAlwaysVisibleScrollbar(bool bVisible)
{
	m_bAlwaysVisibleScrollbar=bVisible;
	if(m_bAlwaysVisibleScrollbar)
		m_pScrollBar->Show(true);
}

void MListBox::EnableDragAndDrop( bool bEnable)
{
	m_bDragAndDrop = bEnable;
}

bool MListBox::OnDrop(MWidget* pSender, MBitmap* pBitmap, const char* szString, const char* szItemString)
{
	if ( m_pOnDropFunc != NULL)
	{
		m_pOnDropFunc(this, pSender, pBitmap, szString, szItemString);
	}

	return true;
}

