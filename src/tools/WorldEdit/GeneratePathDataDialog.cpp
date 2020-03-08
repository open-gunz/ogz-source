// GeneratePathDataDialog.cpp : implementation file
//

#include "stdafx.h"
#include "WorldEdit.h"
#include "GeneratePathDataDialog.h"


// CGeneratePathDataDialog dialog

IMPLEMENT_DYNAMIC(CGeneratePathDataDialog, CDialog)
CGeneratePathDataDialog::CGeneratePathDataDialog(CWnd* pParent /*=NULL*/)
	: CDialog(CGeneratePathDataDialog::IDD, pParent)
	, m_WalkableAngle(_T(""))
	, m_Toler(_T(""))
{
}

CGeneratePathDataDialog::~CGeneratePathDataDialog()
{
}

void CGeneratePathDataDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT1, m_WalkableAngle);
	DDX_Text(pDX, IDC_EDIT2, m_Toler);
}


BEGIN_MESSAGE_MAP(CGeneratePathDataDialog, CDialog)
END_MESSAGE_MAP()


// CGeneratePathDataDialog message handlers
