// ObjectDialog.cpp : implementation file
//


#include "stdafx.h"
#include "MZFileSystem.h"
#include "WorldEdit.h"
#include "ObjectDialog.h"
#include "FileInfo.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif



/////////////////////////////////////////////////////////////////////////////
// CObjectDialog dialog


CObjectDialog::CObjectDialog(CWnd* pParent /*=NULL*/)
	: CDialog(CObjectDialog::IDD, pParent)
{
	//{{AFX_DATA_INIT(CObjectDialog)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT

}


void CObjectDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CObjectDialog)
	DDX_Control(pDX, IDC_LIST1, m_ObjectList);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CObjectDialog, CDialog)
	//{{AFX_MSG_MAP(CObjectDialog)
	//}}AFX_MSG_MAP
//	ON_BN_CLICKED(IDC_RADIO1, OnBnClickedRadio1)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CObjectDialog message handlers

void CObjectDialog::Initilize()
{
	MZFileSystem mfs;
	mfs.Create("./");

	for(int i=0;i<mfs.GetFileCount();i++)
	{
		const char *filename=mfs.GetFileName(i);
		char ext[_MAX_EXT];
		GetPureExtension(ext,filename);
		if(_stricmp(ext,".elu")==0)
			m_ObjectList.AddString(filename);
	}
}