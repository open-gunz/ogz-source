// ObjectSelectDialog.cpp : implementation file
//


#include "stdafx.h"
#include "MZFileSystem.h"
#include "WorldEdit.h"
#include "ObjectSelectDialog.h"
#include "FileInfo.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif



/////////////////////////////////////////////////////////////////////////////
// CObjectSelectDialog dialog


CObjectSelectDialog::CObjectSelectDialog(CWnd* pParent /*=NULL*/)
	: CDialog(CObjectSelectDialog::IDD, pParent)
{
	//{{AFX_DATA_INIT(CObjectSelectDialog)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT

}


void CObjectSelectDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CObjectSelectDialog)
	DDX_Control(pDX, IDC_LIST1, m_ObjectList);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CObjectSelectDialog, CDialog)
	//{{AFX_MSG_MAP(CObjectSelectDialog)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CObjectSelectDialog message handlers

void CObjectSelectDialog::Initilize()
{
	MZFileSystem mfs;
	mfs.Create("./");

	for(int i=0;i<mfs.GetFileCount();i++)
	{
		const char *filename=mfs.GetFileName(i);
		char ext[_MAX_EXT];
		GetPureExtension(ext,filename);
		if(stricmp(ext,".elu")==0)
			m_ObjectList.AddString(filename);
	}
}
