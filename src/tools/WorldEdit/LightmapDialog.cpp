// LightmapDialog.cpp : implementation file
//

#include "stdafx.h"
#include "WorldEdit.h"
#include "LightmapDialog.h"


// CLightmapDialog dialog

IMPLEMENT_DYNAMIC(CGenerateLightmapDialog, CDialog)
CGenerateLightmapDialog::CGenerateLightmapDialog(CWnd* pParent /*=NULL*/)
	: CDialog(CGenerateLightmapDialog::IDD, pParent)
	, m_MaxSize(_T(""))
	, m_Toler(_T(""))
	, m_SuperSample(_T(""))
{
}

CGenerateLightmapDialog::~CGenerateLightmapDialog()
{
}

void CGenerateLightmapDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_CBString(pDX, IDC_COMBO1, m_MaxSize);
	DDX_Text(pDX, IDC_EDIT1, m_Toler);
	DDX_CBString(pDX, IDC_COMBO2, m_SuperSample);
}


BEGIN_MESSAGE_MAP(CGenerateLightmapDialog, CDialog)
END_MESSAGE_MAP()


// CLightmapDialog message handlers

//BOOL CLightmapDialog::Create(LPCTSTR lpszTemplateName, CWnd* pParentWnd)
//{
//	// TODO: Add your specialized code here and/or call the base class
//
//	BOOL ret=CDialog::Create(lpszTemplateName, pParentWnd);
//	m_Combo_MaxLightmap.AddString("test");
//	return ret;
//}
