#include "stdafx.h"
#include "Mcv.h"

#include "McvDoc.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CMcvDoc

IMPLEMENT_DYNCREATE(CMcvDoc, CDocument)

BEGIN_MESSAGE_MAP(CMcvDoc, CDocument)
	//{{AFX_MSG_MAP(CMcvDoc)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CMcvDoc construction/destruction

CMcvDoc::CMcvDoc()
{
}

CMcvDoc::~CMcvDoc()
{
}

BOOL CMcvDoc::OnNewDocument()
{
	if (!CDocument::OnNewDocument())
		return FALSE;

	return TRUE;
}



/////////////////////////////////////////////////////////////////////////////
// CMcvDoc serialization

void CMcvDoc::Serialize(CArchive& ar)
{
	if (ar.IsStoring())	{

	}
	else {

	}
}

/////////////////////////////////////////////////////////////////////////////
// CMcvDoc diagnostics

#ifdef _DEBUG
void CMcvDoc::AssertValid() const
{
	CDocument::AssertValid();
}

void CMcvDoc::Dump(CDumpContext& dc) const
{
	CDocument::Dump(dc);
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CMcvDoc commands
