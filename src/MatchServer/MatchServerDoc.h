// MatchServerDoc.h : CMatchServerDoc 클래스의 인터페이스
//


#pragma once

#include "MMatchServer.h"
#include "MCommandManager.h"
#include "MMonitor.h"
#include "MMaster.h"

class COutputView;
class CMatchServerDoc;
class MCommandProcessor;
class MBMatchServer;
class CCommandLogView;


#define TIME_COLOR	RGB(128, 128, 128)


class MBMaster : public MMaster{
public:
	COutputView*	m_pView;
public:
	MBMaster(COutputView* pView=NULL);
	virtual void Log(const char* szLog);
};


class CMatchServerDoc : public CDocument
{
protected: // serialization에서만 만들어집니다.
	CMatchServerDoc();
	DECLARE_DYNCREATE(CMatchServerDoc)

// Attributes
public:
//	MBMaster*		m_pMaster;
	MBMatchServer*	m_pMatchServer;

protected:
//	COutputView*		m_pMasterView;
	COutputView*		m_pMatchServerView;
	CCommandLogView*	m_pCmdLogView;
//	CFrameWnd*		m_pFrmWndMasterView;
	CFrameWnd*		m_pFrmWndCmdLogView;

public:
	virtual BOOL OnNewDocument();
	virtual void Serialize(CArchive& ar);

	COutputView* GetOutputView(void);

	void PostCommand(const char* szCommand);
	void Run(void);

protected:
	void ProcessLocalCommand(MCommand* pCommand);

// 구현
public:
	virtual ~CMatchServerDoc();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// 메시지 맵 함수를 생성했습니다.
protected:
	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnOpenDocument(LPCTSTR lpszPathName);
	virtual void OnCloseDocument();
};


