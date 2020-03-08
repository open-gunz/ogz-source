// LocatorDoc.cpp : CLocatorDoc 클래스의 구현
//

#include "stdafx.h"
#ifdef MFC
#include "Locator.h"

#include "LocatorDoc.h"
#include "MLocator.h"
#include ".\locatordoc.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CLocatorDoc

IMPLEMENT_DYNCREATE(CLocatorDoc, CDocument)

BEGIN_MESSAGE_MAP(CLocatorDoc, CDocument)
	ON_COMMAND(ID_OutputLocatorStatusInfo, OnOutputlocatorstatusinfo)
END_MESSAGE_MAP()


// CLocatorDoc 생성/소멸

CLocatorDoc::CLocatorDoc() : m_pLocator( 0 ) 
{
	// TODO: 여기에 일회성 생성 코드를 추가합니다.

}

CLocatorDoc::~CLocatorDoc()
{
	ReleaseLocator();
}

BOOL CLocatorDoc::OnNewDocument()
{
	if (!CDocument::OnNewDocument())
		return FALSE;

	// TODO: 여기에 다시 초기화 코드를 추가합니다.
	// SDI 문서는 이 문서를 다시 사용합니다.

	if( !CreateLocator() )
		return FALSE;

	return TRUE;
}


bool CLocatorDoc::CreateLocator()
{
	ASSERT( 0 == m_pLocator );

	if( 0 == m_pLocator )
	{
		m_pLocator = new MLocator;
		if( 0 == m_pLocator ) 
			return false;
	}
	else
		m_pLocator->Destroy();

	return m_pLocator->Create();
}


void CLocatorDoc::ReleaseLocator()
{
	m_pLocator->Destroy();
	delete m_pLocator;
	m_pLocator = 0;
}


void CLocatorDoc::Run()
{
	if( 0 != m_pLocator )
	{
		m_pLocator->Run();
	}
}


// CLocatorDoc serialization

void CLocatorDoc::Serialize(CArchive& ar)
{
	if (ar.IsStoring())
	{
		// TODO: 여기에 저장 코드를 추가합니다.
	}
	else
	{
		// TODO: 여기에 로딩 코드를 추가합니다.
	}
}


// CLocatorDoc 진단

#ifdef _DEBUG
void CLocatorDoc::AssertValid() const
{
	CDocument::AssertValid();
}

void CLocatorDoc::Dump(CDumpContext& dc) const
{
	CDocument::Dump(dc);
}
#endif //_DEBUG


// CLocatorDoc 명령

void CLocatorDoc::OnOutputlocatorstatusinfo()
{
	// TODO: 여기에 명령 처리기 코드를 추가합니다.
	if( 0 != m_pLocator )
		m_pLocator->DumpLocatorStatusInfo();
}
#endif