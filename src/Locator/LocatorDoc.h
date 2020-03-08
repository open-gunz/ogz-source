// LocatorDoc.h : CLocatorDoc 클래스의 인터페이스
//


#pragma once

class MLocator;

class CLocatorDoc : public CDocument
{
protected: // serialization에서만 만들어집니다.
	CLocatorDoc();
	DECLARE_DYNCREATE(CLocatorDoc)

// 특성
public:

// 작업
public:
	void Run();

	const MLocator* GetLocator() const { return m_pLocator; }

private :
	bool CreateLocator();
	void ReleaseLocator();

private :
	MLocator*	m_pLocator;

// 재정의
	public:
	virtual BOOL OnNewDocument();
	virtual void Serialize(CArchive& ar);

// 구현
public:
	virtual ~CLocatorDoc();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// 메시지 맵 함수를 생성했습니다.
protected:
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnOutputlocatorstatusinfo();
};


