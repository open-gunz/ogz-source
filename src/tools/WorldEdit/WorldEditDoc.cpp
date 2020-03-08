// WorldEditDoc.cpp : implementation of the CWorldEditDoc class
//

#include "stdafx.h"
#include "WorldEdit.h"

#include "WorldEditDoc.h"

#include "RBspObject.h"
#include "FileInfo.h"
#include "RMaterialList.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CWorldEditDoc

IMPLEMENT_DYNCREATE(CWorldEditDoc, CDocument)

BEGIN_MESSAGE_MAP(CWorldEditDoc, CDocument)
	//{{AFX_MSG_MAP(CWorldEditDoc)
		// NOTE - the ClassWizard will add and remove mapping macros here.
		//    DO NOT EDIT what you see in these blocks of generated code!
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CWorldEditDoc construction/destruction

BOOL CWorldEditDoc::OnNewDocument()
{
	if (!CDocument::OnNewDocument())
		return FALSE;

	// TODO: add reinitialization code here
	// (SDI documents will reuse this document)

	return TRUE;
}



/////////////////////////////////////////////////////////////////////////////
// CWorldEditDoc serialization

void CWorldEditDoc::Serialize(CArchive& ar)
{
	if (ar.IsStoring())
	{
		// TODO: add storing code here
	}
	else
	{
		// TODO: add loading code here
	}
}

/////////////////////////////////////////////////////////////////////////////
// CWorldEditDoc diagnostics

#ifdef _DEBUG
void CWorldEditDoc::AssertValid() const
{
	CDocument::AssertValid();
}

void CWorldEditDoc::Dump(CDumpContext& dc) const
{
	CDocument::Dump(dc);
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CWorldEditDoc commands

BOOL CWorldEditDoc::OnOpenDocument(LPCTSTR lpszPathName) 
{
	if (!CDocument::OnOpenDocument(lpszPathName))
		return FALSE;

	m_pBspObject.reset();

	m_pBspObject = std::make_unique<RBspObject>();
	if(!m_pBspObject->Open(lpszPathName, RBspObject::ROpenMode::Editor))
	{
		m_pBspObject.reset();
		AfxMessageBox("Failed to open map!");
		return FALSE;
	}

	m_bLastPicked=false;
	return TRUE;
}

void CWorldEditDoc::OnCloseDocument()
{
	// TODO: Add your specialized code here and/or call the base class
	m_pBspObject.reset();

	CDocument::OnCloseDocument();
}

bool CWorldEditDoc::ReOpen()
{
	m_bLastPicked=false;

	m_pBspObject.reset();

	m_pBspObject = std::make_unique<RBspObject>();
	return m_pBspObject->Open(GetPathName(),RBspObject::ROpenMode::Editor);
}