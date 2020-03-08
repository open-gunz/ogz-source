#pragma once

#include "MWidget.h"
#include <list>
#include "MLookNFeel.h"

class MEdit;

class MEditLook{
protected:
	bool		m_bCustomLook = true;
public:
	virtual void OnFrameDraw(MEdit* pEdit, MDrawContext* pDC);
	virtual void OnTextDraw(MEdit* pEdit, MDrawContext* pDC, bool bShowLanguageTab);

	virtual void OnDraw(MEdit* pEdit, MDrawContext* pDC, bool bShowLanguageTab = true);
	virtual MRECT GetClientRect(MEdit* pEdit, const MRECT& r);

	void SetCustomLook(bool b) {
		m_bCustomLook = b;
	}
	bool GetCustomLook() const{
		return m_bCustomLook;
	}
};

class MEdit : public MWidget {
protected:
	bool		m_bMouseOver;
	MCOLOR		m_TextColor;
	int			m_nMaxLength;
	char*		m_pBuffer;
	char		m_szIMECompositionString[MIMECOMPOSITIONSTRING_LENGTH];
	bool		m_bPassword;
	int			m_nCaretPos;	
	int			m_nStartPos;
	MWidget*	m_pTabHandler;

public:
	int			m_nSelectionRange;
protected:
	std::list<char*>			m_History;
	std::list<char*>::iterator	m_nCurrentHistory;

	DECLARE_LOOK(MEditLook)
	DECLARE_LOOK_CLIENT()

public:
	bool		m_bSupportHistory;

protected:
	virtual bool OnEvent(MEvent* pEvent, MListener* pListener) override;
	
	virtual bool InputFilterKey(int nKey);	// MWM_KEYDOWN
	virtual bool InputFilterChar(int nKey);	// MWM_CHAR

	virtual void OnSetFocus() override;
	virtual void OnReleaseFocus() override;

	void Initialize(int nMaxLength, const char* szName);
public:
	MEdit(int nMaxLength, const char* szName = nullptr, MWidget* pParent = nullptr, MListener* pListener = nullptr);
	MEdit(const char* szName = nullptr, MWidget* pParent = nullptr, MListener* pListener = nullptr);
	virtual ~MEdit();

	virtual void OnHide() override;

	virtual void SetText(const char* szText) override;
	virtual const char* GetText() override;
	void AddText(const char* szText);

	int MoveCaretHome();
	int MoveCaretEnd();
	int MoveCaretPrev();
	int MoveCaretNext();

	void SetMaxLength(int nMaxLength);
	int GetMaxLength();
	const char* GetCompositionString();
	int GetCarretPos();
	int GetStartPos() { return m_nStartPos; }
	bool SetStartPos(int nStartPos);
	int GetPosByScreen(int x);

	void SetPasswordField(bool bPassword);
	bool IsPasswordField();

	bool GetClipboard(char* szText, int nSize);
	bool SetClipboard(const char* szText);

	void AddHistory(const char* szText);

	MWidget* GetTabHandler()				{ return m_pTabHandler; }
	void SetTabHandler(MWidget* pWidget)	{ m_pTabHandler = pWidget; }

#define MINT_EDIT	"Edit"
#ifdef GetClassName
#undef GetClassName
#endif
	virtual const char* GetClassName() override { return MINT_EDIT; }
};

#define MEDIT_KEYDOWN_MSG		"keydown"
#define MEDIT_CHAR_MSG			"char"
#define MEDIT_ENTER_VALUE		"entered"
#define MEDIT_ESC_VALUE			"esc"

#define MEDIT_BLINK_TIME		400