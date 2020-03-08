// PathDialog.cpp : implementation file
//

#include "stdafx.h"
#include "WorldEdit.h"
#include "PathDialog.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CPathDialog dialog


CPathDialog::CPathDialog(CWnd* pParent /*=NULL*/)
	: CDialog(CPathDialog::IDD, pParent)
{
	//{{AFX_DATA_INIT(CPathDialog)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CPathDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CPathDialog)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CPathDialog, CDialog)
	//{{AFX_MSG_MAP(CPathDialog)
		// NOTE: the ClassWizard will add message map macros here
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CPathDialog message handlers
