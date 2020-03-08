#pragma once

#include "MWidget.h"
#include "CMList.h"
#include "MScrollBar.h"
#include "MColorTable.h"
#include "StringView.h"
#include "SafeString.h"

enum MListViewStyle
{
	MVS_LIST = 0,
	MVS_ICON
};

class MListItem{
public:
	bool m_bSelected;

	MListItem() { m_bSelected = false; }
	virtual ~MListItem(){}

	virtual const char* GetString() = 0;
	virtual const char* GetString(int i){
		if(i==0) return GetString();
		return NULL;
	}
	virtual void SetString(const char *szText){
	}

	MBitmap* GetBitmap(){
		return GetBitmap(0);
	}
	virtual MBitmap* GetBitmap(int i){
		return NULL;
	}

	virtual const MCOLOR GetColor() { return GetColor(0); }
	virtual const MCOLOR GetColor(int i) { 
		return MCOLOR(DEFCOLOR_MLIST_TEXT);
	}

	virtual bool GetDragItem(MBitmap** ppDragBitmap, char* szDragString, char* szDragItemString){
		return false;
	}
};

class MListFieldItem{
protected:
	MCOLOR		Color{ DEFCOLOR_MLIST_TEXT };
	std::string String;
	MBitmap*	Bitmap{};
public:
	MListFieldItem(const StringView& String, MCOLOR Color) : Color{ Color }, String{ String.str() } {}
	MListFieldItem(const StringView& String) : String{ String.str() } {}
	MListFieldItem(MBitmap* Bitmap) : Bitmap{ Bitmap } {}

	const char* GetString(){
		return String.c_str();
	}

	void SetColor(MCOLOR color) { Color = color; }
	virtual const MCOLOR GetColor() { return Color; }

	MBitmap* GetBitmap(){ return Bitmap; }
	void SetBitmap(MBitmap* pBitmap){ Bitmap = pBitmap; }
};


class MDefaultListItem : public MListItem{
	CMPtrList<MListFieldItem>	m_Items;
public:
	MDefaultListItem(){
	}
	MDefaultListItem(const StringView& szText, const MCOLOR color) {
		MListFieldItem* pNew = new MListFieldItem(szText, color);
		m_Items.Add(pNew);
	}
	MDefaultListItem(const StringView& szText){
		MListFieldItem* pNew = new MListFieldItem(szText);
		m_Items.Add(pNew);
	}
	MDefaultListItem(MBitmap* pBitmap, const char* szText){
		MListFieldItem* pNew = new MListFieldItem(pBitmap);
		m_Items.Add(pNew);
		pNew = new MListFieldItem(szText);
		m_Items.Add(pNew);
	}
	virtual ~MDefaultListItem() override {
		for(int i=0; i<m_Items.GetCount(); i++){
			MListFieldItem* pItem = m_Items.Get(i);
			delete pItem;
		}
	}
	virtual const char* GetString() override {
		if(m_Items.GetCount()>0) return m_Items.Get(0)->GetString();
		return NULL;
	}
	virtual const char* GetString(int i) override {
		if(i<m_Items.GetCount()) return m_Items.Get(i)->GetString();
		return NULL;
	}
	virtual void SetString(const char *szText) override {
		if(m_Items.GetCount()){
			delete m_Items.Get(0);
			m_Items.Delete(0);
		}
		MListFieldItem* pNew = new MListFieldItem(szText);
		m_Items.MoveFirst();
		m_Items.InsertBefore(pNew);
	}

	virtual MBitmap* GetBitmap(int i) override {
		if(i<m_Items.GetCount()) return m_Items.Get(i)->GetBitmap();
		return NULL;
	}

	virtual const MCOLOR GetColor() override { 
		if (m_Items.GetCount()>0) return m_Items.Get(0)->GetColor();
		return MCOLOR(DEFCOLOR_MLIST_TEXT);
	}
};

struct MLISTFIELD{
	char			szFieldName[256];
	int				nTabSize;
};

class MListBox;
class MListBoxLook{
public:
	MCOLOR	m_SelectedPlaneColor;
	MCOLOR	m_SelectedTextColor;
	MCOLOR	m_UnfocusedSelectedPlaneColor;
	MAlignmentMode	m_ItemTextAlignmentMode;
	bool			m_bItemTextMultiLine;
	MBitmap*	m_pItemSlotBitmap;
protected:
	virtual void OnHeaderDraw(MDrawContext* pDC, MRECT& r, const char* szText);
	virtual int OnItemDraw(MDrawContext* pDC, MRECT& r, const char* szText, MCOLOR color, bool bSelected, bool bFocus, int nAdjustWidth = 0);
	virtual int OnItemDraw(MDrawContext* pDC, MRECT& r, MBitmap* pBitmap, bool bSelected, bool bFocus, int nAdjustWidth = 0);
	virtual void OnFrameDraw(MListBox* pListBox, MDrawContext* pDC);
public:
	MListBoxLook();

	virtual void OnDraw(MListBox* pListBox, MDrawContext* pDC);
	virtual MRECT GetClientRect(MListBox* pListBox, const MRECT& r);
};

typedef void (*ZCB_ONDROP)(void* pSelf, MWidget* pSender, MBitmap* pBitmap, const char* szString, const char* szItemString);

class MListBox : public MWidget{
protected:
	class SortedItemList : public CMLinkedList<MListItem>{
	public:
		bool	m_bAscend;
	public:
		SortedItemList(){
			m_bAscend = true;
		}
		virtual int Compare(MListItem *lpRecord1,MListItem *lpRecord2){
			int nCompare = _stricmp(lpRecord1->GetString(0), lpRecord2->GetString(0));
			if(m_bAscend==true) return nCompare;
			else return -nCompare;
		}
	} m_Items;
	int				m_nOverItem;
	int				m_nSelItem;
	int				m_nShowItemCount;
	int				m_nStartItemPos;
	int				m_nItemHeight;
	MScrollBar*		m_pScrollBar;

	CMLinkedList<MLISTFIELD>	m_Fields;

	bool			m_bVisibleHeader;
	bool			m_bSelected;
	MListViewStyle	m_ViewStyle;
	bool			m_bAlwaysVisibleScrollbar;
	bool			m_bDragAndDrop;
	ZCB_ONDROP		m_pOnDropFunc;

public:
	bool			m_bAbsoulteTabSpacing;
	bool			m_bHideScrollBar;
	bool			m_bNullFrame;
	bool			m_bMultiSelect;

protected:
	virtual bool OnEvent(MEvent* pEvent, MListener* pListener) override;
	virtual bool OnDrop(MWidget* pSender, MBitmap* pBitmap, const char* szString, const char* szItemString) override;
	virtual bool IsDropable(MWidget* pSender) override { return m_bDragAndDrop; }

	void RecalcList();
	void RecalcScrollBar();

	int FindNextItem(int i, char c);

	virtual void OnSize(int w, int h) override;

	virtual bool OnCommand(MWidget* pWindow, const char* szMessage) override;

	void Initialize();

public:
	MCOLOR m_FontColor;
	MAlignmentMode m_FontAlign;

public:
	MListBox(const char* szName, MWidget* pParent=NULL, MListener* pListener=NULL);
	MListBox(MWidget* pParent=NULL, MListener* pListener=NULL);
	virtual ~MListBox() override;

	void Add(const StringView& szItem);
	void Add(const StringView& szItem, MCOLOR color);
	void Add(MListItem* pItem);
	const char* GetString(int i);
	MListItem* Get(int i);
	bool Set(int i, const char* szItem);
	bool Set(int i, MListItem* pItem);
	void Remove(int i);
	void RemoveAll();
	bool Swap(int i, int j);
	int GetCount();
	int GetSelIndex();
	bool SetSelIndex(int i);
	const char* GetSelItemString();
	MListItem* GetSelItem();

	int GetSelCount();

	int FindItem(MPOINT& p);
	bool GetItemPos(MPOINT* p, int i);

	int GetItemHeight();
	void SetItemHeight(int nHeight);

	bool IsShowItem(int i);
	void ShowItem(int i);
	void SetStartItem(int i);
	int GetStartItem();
	int GetShowItemCount();

	MScrollBar* GetScrollBar();

	void Sort(bool bAscend=true);

	void AddField(const char* szFieldName, int nTabSize);
	void RemoveField(const char* szFieldName);
	MLISTFIELD* GetField(int i);
	int GetFieldCount();
	void RemoveAllField();

	bool IsVisibleHeader();
	void SetVisibleHeader(bool bVisible);

	bool IsAlwaysVisibleScrollbar();
	void SetAlwaysVisibleScrollbar(bool bVisible);

	bool IsSelected() { return m_bSelected; }
	void SetSelected(bool bSelected) { m_bSelected = bSelected; }
	MListViewStyle GetViewStyle() { return m_ViewStyle; }
	void SetViewStyle(MListViewStyle ViewStyle);
	int GetTabSize();
	void EnableDragAndDrop( bool bEnable);

	void SetOnDropCallback(ZCB_ONDROP pCallback) { m_pOnDropFunc = pCallback; }

	DECLARE_LOOK(MListBoxLook)
	DECLARE_LOOK_CLIENT()

#define MINT_LISTBOX	"ListBox"
	virtual const char* GetClassName() override { return MINT_LISTBOX; }
};

#define MLB_ITEM_SEL		"selected"
#define MLB_ITEM_SEL2		"selected2"
#define MLB_ITEM_DBLCLK		"dclk"
#define MLB_ITEM_SELLOST	"lost"
#define MLB_ITEM_DEL		"del"
#define MLB_ITEM_START		"start"
#define MLB_ITEM_CLICKOUT	"clickout"