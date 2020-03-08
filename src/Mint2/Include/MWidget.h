#pragma once

#include "CMPtrList.h"
#include "MDrawContext.h"
#include "MTypes.h"
#include "MEvent.h"

class MWidget;

class MListener{
public:
	virtual bool OnCommand(MWidget* pWidget, const char* szMessage) = 0;
};

class MToolTip;
class MResourceMap;

enum MZOrder{
	MZ_TOP = 0,
	MZ_BOTTOM,
};

struct MAnchors
{
	bool m_bLeft;
	bool m_bRight;
	bool m_bTop;
	bool m_bBottom;
	MAnchors()	{ m_bLeft = true; m_bRight = false; m_bTop = true; m_bBottom = false; }
	MAnchors(bool bLeft, bool bRight, bool bTop, bool bBottom)
					{ m_bLeft = bLeft; m_bRight = bRight; m_bTop = bTop; m_bBottom = bBottom; }
};

#define MWIDGET_NAME_LENGTH		256

class MWidget : public MListener {
private:
	bool				m_bEnable;
	bool				m_bFocusEnable;

	MListener*			m_pListener;
	MToolTip*			m_pToolTip;

	int					m_nAndPos;
	int					m_nAccelerator;

protected:
	bool				m_bVisible;

	CMPtrList<MWidget>	m_Children;

	MWidget*			m_pParent;
	CMPtrList<MWidget>	m_Exclusive;

	MCursor*			m_pCursor;
	MFont*				m_pFont;

	static MWidget*		m_pCapturedWidget;
	static MWidget*		m_pFocusedWidget;

	bool				m_bZOrderChangable;
	bool				m_bResizable;

	int					m_nResizeSide;

	unsigned char		m_nOpacity;

	bool				m_bClipByParent;

	MAlignmentMode		m_BoundsAlignment;

public:
	char				m_szName[MWIDGET_NAME_LENGTH];
	char				m_szIDLName[MWIDGET_NAME_LENGTH];

	MRECT				m_Rect;
	MRECT				m_IDLRect;

	MAnchors			m_Anchors;
	int					m_nMinWidth, m_nMinHeight;

	bool				m_bisListBox;
	int					m_nDebugType;
	bool				m_bEventAcceleratorCall;
protected:
	// Only for Designer Mode
	bool				m_bEnableDesignerMode;
	int					m_nDMDragWidget;
	MPOINT				m_DMDragPoint;
	bool				m_bModifiedByDesigner;
	bool				m_bAddedByDesigner;
	int					m_nID;

private:
	void MakeLocalEvent(MEvent* pLoalEvent, const MEvent* pEvent);
	bool EventResize(MEvent* pEvent);
protected:
	void InsertChild(MWidget* pWidget);
	void AddChild(MWidget* pWidget);
	void RemoveChild(MWidget* pWidget);

	void AddExclusive(MWidget* pWidget);
	bool RemoveExclusive(MWidget* pWidget);

	virtual void OnRun();
	virtual void OnDraw(MDrawContext* pDC);

	virtual bool OnEvent(MEvent* pEvent, MListener* pListener);

	virtual bool OnHotKey(int nID){ return false; }

	virtual bool OnShow() { return true; }
	virtual void OnHide() {}

	virtual void OnSetFocus(void){}
	virtual void OnReleaseFocus(void){}

	virtual void OnSize(int w, int h) {}

	void OnShow(bool bVisible);

	virtual bool OnTab(bool bForward = true);
	virtual bool OnDrop(MWidget* pSender, MBitmap* pBitmap, const char* szString, const char* szItemString);
	virtual bool OnCommand(MWidget* pWidget, const char* szMessage){ return false; }

	void ResizeChildrenByAnchor(int w, int h);

	void GetBoundsAlignmentPosition(MPOINT* p, MAlignmentMode am, int w=-1, int h=-1);

public:
	MWidget(const char* szName=NULL, MWidget* pParent=NULL, MListener* pListener=NULL);
	virtual ~MWidget();

	void Run();
	void Draw(MDrawContext* pDC);
	void Redraw(void);
	bool Event(MEvent* pEvent);
	bool EventAccelerator(MEvent* pEvent);
	bool EventDefaultKey(MEvent* pEvent);

	virtual void Show(bool bVisible=true, bool bModal=false);
	void Hide(){ Show(false); }
	void Enable(bool bEnable=true);
	bool IsVisible();
	bool IsEnable();

	void SetResizable(bool bEnable);
	bool IsResizable();

	virtual void SetListener(MListener* pListener);
	virtual MListener* GetListener();

	int GetID();
	void SetID(int nID);

	virtual void SetText(const char* szText);
	virtual const char* GetText();

	void SetCapture();
	void ReleaseCapture();

	void SetFocusEnable(bool bEnable);
	bool IsFocusEnable();

	void SetFocus();
	void ReleaseFocus();
	bool IsFocus();

	MWidget* GetParent();
	int GetChildCount();
	MWidget* GetChild(int i);
	int GetChildIndex(MWidget* pWidget);

	void SetExclusive();
	void ReleaseExclusive();
	MWidget* GetLatestExclusive();
	bool IsExclusive(MWidget* pWidget);

	MCursor* SetCursor(MCursor* pCursor);
	MCursor* GetCursor();

	MFont* SetFont(MFont* pFont);
	MFont* GetFont();

	void SetSize(int w, int h);
	void SetSize(MSIZE& s);

	void SetPosition(int x, int y);
	void SetPosition(const MPOINT& p);
	void SetBounds(const MRECT& r);
	void SetBounds(int x, int y, int w, int h);
	MPOINT GetPosition();
	MRECT GetRect();
	MRECT GetIDLRect();
	void SetBoundsAlignment(MAlignmentMode am, int w, int h);
	MAlignmentMode GetBoundsAlignment();

	void SetOpacity(unsigned char nOpacity);
	unsigned char GetOpacity() const;

	MRECT GetScreenRect() const;

	void AttachToolTip(const char* szToolTipString = nullptr);
	void AttachToolTip(MToolTip* pToolTip);
	void DetachToolTip();
	MToolTip* GetToolTip();

	void SetAccelerator(int a);
	void SetLabelAccelerator();
	char GetLabelAccelerator();
	char GetToolTipAccelerator();

	virtual MRECT GetClientRect();
	MRECT GetInitialClientRect();

	void SetZOrder(MZOrder z);
	MWidget* FindExclusiveDescendant();

	MWidget* Find(int x, int y) { return Find(MPOINT(x, y)); }
	MWidget* Find(const MPOINT& p);
	MWidget* FindDropAble(MPOINT& p);

	virtual bool IsDropable(MWidget* pSender) { return false; }
	bool Drop(MWidget* pSender, MBitmap* pBitmap, const char* szString, const char* szItemString);

	void SetVisible(bool b) { m_bVisible = b; }
	bool GetVisible() { return m_bVisible; }

	template<size_t size>
	void GetHierarchicalName(char(&szName)[size]) {
		GetHierarchicalName(szName, size);
	}
	void GetHierarchicalName(char* szName, int maxlen);
	MWidget* FindWidgetByHierarchicalName(const char* szName);

	virtual bool DefaultCommand(){ return false; }

	static bool IsMsg(const char* szMsg1, const char* szMsg2);

	virtual void* Query(const char* szQuery);

	void SetClipByParent(bool b) {
		m_bClipByParent = b;
	}

#define MINT_WIDGET	"Widget"
#ifndef GetClassName
#undef GetClassName
#endif
	virtual const char* GetClassName(){ return MINT_WIDGET; }
};

int GetAndPos(const char* szText);
char GetAndChar(const char* szText);
int RemoveAnd(char* szText);

template<size_t size>
int RemoveAnd(char(&szRemovedText)[size], const char* szText) {
	return RemoveAnd(szRemovedText, size, szText);
}
int RemoveAnd(char* szRemovedText, int maxlen, const char* szText);

template<size_t size>
int RemoveAnd(char(&szRemovedFrontText)[size], char* cUnderLineChar,
	char* szRemovedBackText, const char* szText) {
	return RemoveAnd(szRemovedFrontText, size, cUnderLineChar, szRemovedBackText, szText);
}
int RemoveAnd(char* szRemovedFrontText, int maxlen, char* cUnderLineChar,
	char* szRemovedBackText, const char* szText);

MPOINT MClientToScreen(const MWidget* pWidget, const MPOINT& p);
MPOINT MScreenToClient(const MWidget* pWidget, const MPOINT& p);
MRECT MClientToScreen(const MWidget* pWidget, const MRECT& p);
MRECT MScreenToClient(const MWidget* pWidget, const MRECT& p);
