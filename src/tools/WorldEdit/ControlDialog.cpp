// ControlDialog.cpp : implementation file
//

#include "stdafx.h"
#include "WorldEdit.h"
#include "ControlDialog.h"
#include "MainFrm.h"
#include "WorldEditDoc.h"
#include "WorldEditView.h"


// CControlDialog dialog

IMPLEMENT_DYNAMIC(CControlDialog, CDialog)
CControlDialog::CControlDialog(CWnd* pParent /*=NULL*/)
	: CDialog(CControlDialog::IDD, pParent)
{
}

CControlDialog::~CControlDialog()
{
}

BEGIN_MESSAGE_MAP(CControlDialog, CDialog)
//ON_WM_HSCROLL()
ON_STN_CLICKED(ID_MINIMAP, OnStnClickedMinimap)
ON_BN_CLICKED(IDC_BUTTON_RESETCAMERA, OnBnClickedButtonResetcamera)
END_MESSAGE_MAP()

void CControlDialog::OnStnClickedMinimap()
{
	// TODO: Add your control notification handler code here

}

//BOOL CControlDialog::OnCommand(WPARAM wParam, LPARAM lParam)
//{
//	// TODO: Add your specialized code here and/or call the base class
//
//	return CDialog::OnCommand(wParam, lParam);
//}

void CControlDialog::OnBnClickedButtonResetcamera()
{
	// TODO: Add your control notification handler code here
	((CWorldEditView*)((CMainFrame*)GetParent())->GetActiveView())->OnResetCamera();
}
