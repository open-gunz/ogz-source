// ProgressDialog.cpp : implementation file
//

#include "stdafx.h"
#include "WorldEdit.h"
#include "ProgressDialog.h"


bool g_bProgress=false;

// CProgressDialog dialog

IMPLEMENT_DYNAMIC(CProgressDialog, CDialog)
CProgressDialog::CProgressDialog(CWnd* pParent /*=NULL*/)
	: CDialog(CProgressDialog::IDD, pParent)
{
	g_bProgress=true;
}

CProgressDialog::~CProgressDialog()
{
}

void CProgressDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_PROGRESS1, m_Progress);
}


BEGIN_MESSAGE_MAP(CProgressDialog, CDialog)
	ON_BN_CLICKED(IDCANCEL, OnBnClickedCancel)
END_MESSAGE_MAP()


// CProgressDialog message handlers

void CProgressDialog::OnBnClickedCancel()
{
	// TODO: Add your control notification handler code here
	g_bProgress=false;

	OnCancel();
}


void CProgressDialog::OK()
{
	g_bProgress=false;
	OnCancel();
}