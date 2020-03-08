#pragma once


// CCommandLogView 뷰입니다.

class MCommand;

class CCommandLogView : public CListView
{
	DECLARE_DYNCREATE(CCommandLogView)

protected:
	CCommandLogView();           // 동적 만들기에 사용되는 protected 생성자입니다.
	virtual ~CCommandLogView();

public:
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:
	DECLARE_MESSAGE_MAP()
public:
	virtual void OnInitialUpdate();
protected:
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);

public:
	enum CCommandType{
		CCT_LOCAL = 0,
		CCT_SEND = 1,
		CCT_RECEIVE = 2,
	};
	void AddCommand(u32 nGlobalClock, CCommandType t, MCommand* pCmd);
};


