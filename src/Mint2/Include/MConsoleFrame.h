#ifndef MCONSOLEFRAME_H
#define MCONSOLEFRAME_H

#include "Mint.h"
#include "MWidget.h"
#include "MFrame.h"
#include "MButton.h"
#include "MEdit.h"
#include "MListBox.h"

#include <string>
#include <list>
#include <vector>
#include <map>
#include <deque>
#include <algorithm>
using namespace std;

class MConsoleFrame;

/////////////////////////////////////////////////////////////////////////////////
#define CONSOLE_LINES_MAX 250

typedef void(MCONSOLE_KEYDOWN_CALLBACK)(int nKey);
typedef void(MCONSOLE_INPUT_CALLBACK)(const char* szInputStr);

/// 콘솔에서 사용하는 Edit
class MConsoleEdit: public MEdit
{
private:
	MConsoleFrame*		m_pConsoleFrame;
protected:
	virtual bool InputFilterKey(int nKey);	// MWM_KEYDOWN
	virtual bool InputFilterChar(int nKey);	// MWM_CHAR
	virtual bool OnTab(bool bForward=true);

	MCONSOLE_KEYDOWN_CALLBACK*		m_pfnKeyDown;
	MCONSOLE_INPUT_CALLBACK*		m_pfnInput;
public:
	MConsoleEdit(MConsoleFrame* pConsoleFrame, int nMaxLength, const char* szName=NULL, MWidget* pParent=NULL, MListener* pListener=NULL);
	MConsoleEdit(MConsoleFrame* pConsoleFrame, const char* szName=NULL, MWidget* pParent=NULL, MListener* pListener=NULL);
	virtual ~MConsoleEdit();

	void SetKeyDownCallback(MCONSOLE_KEYDOWN_CALLBACK* pfnKeyDown) { m_pfnKeyDown = pfnKeyDown; }
	void SetInputCallback(MCONSOLE_INPUT_CALLBACK* pfnInput) { m_pfnInput = pfnInput; }
};

/// 실제 콘솔 클래스
class MConsoleFrame : public MFrame
{
private:
	MListBox*		m_pListBox;
	MConsoleEdit*	m_pInputEdit;
//	deque<string>	m_Lines;
//	int				m_nLineCount;
//	MCOLOR			m_LinesColor;
	list<string>	m_Commands;
protected:
	virtual void OnDraw(MDrawContext* pDC);
	virtual bool OnCommand(MWidget* pWidget, const char* szMessage);
	friend MConsoleEdit;
	virtual void OnBrowseCommand(void);
	virtual bool OnShow(void);
public:
	MConsoleFrame(const char* szName=NULL, MWidget* pParent=NULL, MListener* pListener=NULL);
	virtual ~MConsoleFrame(void);

	void OutputMessage(const char* sStr);
	void ClearMessage();
	void SetBounds(MRECT& r);
	void SetBounds(int x, int y, int w, int h);
//	void SetLinesColor(MCOLOR aColor) { m_LinesColor = aColor; }

	void AddCommand(const char* szCommand);

	MConsoleEdit* GetInputEdit() { return m_pInputEdit; }

	void SetInputCallback(MCONSOLE_INPUT_CALLBACK* pfnInput) { m_pInputEdit->SetInputCallback(pfnInput); }
};

#endif