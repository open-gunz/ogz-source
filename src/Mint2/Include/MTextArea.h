#pragma once

#include <list>
#include <string>
#include "MWidget.h"
#include "MLookNFeel.h"
#include "MScrollBar.h"

class MTextArea;
class MScrollBar;

#define MTEXTAREA_DEFAULT_TEXT_COLOR		MCOLOR(224,224,224)

class MTextAreaLook {
public:
	virtual void OnDraw(MTextArea* pTextArea, MDrawContext* pDC);
	virtual MRECT GetClientRect(MTextArea* pTextArea, const MRECT& r);

private:
	virtual void OnFrameDraw(MTextArea* pTextArea, MDrawContext* pDC);
	virtual void OnTextDraw(MTextArea* pTextArea, MDrawContext* pDC);
	virtual void OnTextDraw_WordWrap(MTextArea* pTextArea, MDrawContext* pDC);
};

struct MLineItem {
	MLineItem(MCOLOR _color,string &_text) {
		color=_color;
		text=_text;
	}
	MLineItem(string &_text) {
		color=MTEXTAREA_DEFAULT_TEXT_COLOR;
		text=_text;
	}
	MCOLOR color;
	string text;
};

typedef std::list<MLineItem> MLINELIST;
typedef std::list<MLineItem>::iterator MLINELISTITERATOR;


class MTextArea : public MWidget{
	friend MTextAreaLook;
protected:
	bool		m_bScrollBarEnable;
	int			m_nIndentation;
	bool		m_bWordWrap;
	bool		m_bColorSupport;
	MPOINT		m_TextOffset;
	bool		m_bMouseOver;
	MCOLOR		m_TextColor;
	int			m_nMaxLen;
	char		m_szIMECompositionString[MIMECOMPOSITIONSTRING_LENGTH];
	bool		m_bEditable;
	int			m_nStartLine;
	int			m_nStartLineSkipLine;
	
	int			m_nCurrentSize;
	bool		m_bVerticalMoving;
	int			m_nVerticalMoveAxis;

	int			m_nLineHeight;
	
	MPOINT		m_CaretPos;
	bool		m_bCaretFirst;

	MLINELIST			m_Lines;
	MLINELISTITERATOR	m_CurrentLine;

	std::unique_ptr<MScrollBar>	m_pScrollBar;

	// Look & Feel
	DECLARE_LOOK(MTextAreaLook)
	DECLARE_LOOK_CLIENT()

protected:
	virtual void OnSize(int w, int h) override;
	virtual bool OnCommand(MWidget* pWindow, const char* szMessage) override;
	virtual bool OnEvent(MEvent* pEvent, MListener* pListener) override;
	virtual void OnSetFocus() override;
	virtual void OnReleaseFocus() override;
	
	virtual bool InputFilterKey(int nKey, bool bCtrl);	// MWM_KEYDOWN
	virtual bool InputFilterChar(int nKey);	// MWM_CHAR

	bool OnLButtonDown(MPOINT pos);
	void OnScrollBarChanged(int nPos);

	bool MoveLeft(bool bBackspace=false);
	void MoveRight();
	void MoveDown();
	void MoveUp();
	void DeleteCurrent();
	void ScrollDown();
	void ScrollUp();
	void MoveFirst();
	void MoveLast();
	void OnHome();
	void OnEnd();

	bool GetCaretPosition(MPOINT *pOut,int nSize,const char* szText,int nPos,bool bFirst);

	int GetCharPosition(const char* szText,int nX,int nLine);

	bool IsDoubleCaretPos();	

	void UpdateScrollBar(bool bAdjustStart=false);

	MLINELISTITERATOR GetIterator(int nLine);

public:
	MTextArea(int nMaxLen = 120, const char* szName=NULL, MWidget* pParent=NULL, MListener* pListener=NULL);
	virtual ~MTextArea();

#define MINT_TEXTAREA	"TextArea"
	virtual const char* GetClassName() override { return MINT_TEXTAREA; }

	MPOINT GetCaretPos() { return m_CaretPos; }
	int	GetStartLine() { return m_nStartLine; }

	bool IsScrollBarVisible() { return m_pScrollBar->IsVisible();	}
	int GetScrollBarWidth() { return m_pScrollBar->GetRect().w;	}

	int GetClientWidth();

	int GetLength() { return (int)(m_nCurrentSize+m_Lines.size()); }
	int GetLineCount() { return (int)m_Lines.size(); }

	bool GetText(char *pBuffer,int nBufferSize);
	const char* GetTextLine(int nLine);

	void SetMaxLen(int nMaxLen);
	int	GetMaxLen() { return m_nMaxLen; }

	const char* GetCompositionString();

	void SetEditable(bool editable){ m_bEditable = editable; }
	bool GetEditable() { return m_bEditable; }

	void SetScrollBarEnable(bool bEnable) { m_bScrollBarEnable = bEnable; }
	bool GetScrollBarEnable() { return m_bScrollBarEnable; }
	
	void SetTextOffset(MPOINT p);

	void SetIndentation(int nIndentation) { m_nIndentation = nIndentation; }
	
	void SetTextColor(MCOLOR color);
	MCOLOR GetTextColor();

	void Clear();

	void SetText(const char *szText);

	void AddText(const char *szText,MCOLOR color);
	void AddText(const char *szText);
	
	void DeleteFirstLine();

	int GetLineHeight( void);
	void SetLineHeight( int nHeight);
};

#define MTEXTAREA_ENTER_VALUE		"entered"
#define MTEXTAREA_ESC_VALUE			"esc"
