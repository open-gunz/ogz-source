#include "stdafx.h"
#include "Mcv.h"
#include "ModelInfoDlg.h"

#include "MDebug.h"
#include "RealSpace2.h"

#include "RMtrl.h"

#include "RMeshMgr.h"
#include "RAnimationMgr.h"
#include "RVisualMeshMgr.h"

#include "RBspObject.h"
#include "RMaterialList.h"

#include "stdio.h"
#include ".\modelinfodlg.h"

// 대충하자..

extern bool g_bmtrl_dlg;

extern LPDIRECT3DDEVICE9 g_dev;

extern RMeshMgr		g_mesh_mgr;
extern RMeshMgr		g_weapon_mesh_mgr;

extern RAnimationMgr	g_ani_mgr;
extern RVisualMeshMgr	g_vmesh_mgr;

extern bool g_bframeinfo_dlg;


IMPLEMENT_DYNAMIC(CModelInfoDlg, CDialog)
CModelInfoDlg::CModelInfoDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CModelInfoDlg::IDD, pParent)
{
}

CModelInfoDlg::~CModelInfoDlg()
{
}

void CModelInfoDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST_MESHNODE, m_ListMeshNode);
	DDX_Control(pDX, IDC_LIST_MTRLNODE, m_ListMtrlNode);
}


BEGIN_MESSAGE_MAP(CModelInfoDlg, CDialog)
	ON_BN_CLICKED(IDC_BUTTON_MESHNODE_COLOR, OnBnClickedButtonMeshnodeColor)
	ON_BN_CLICKED(IDC_BUTTON_MTRLNODE_COLOR, OnBnClickedButtonMtrlnodeColor)
END_MESSAGE_MAP()


// CModelInfoDlg 메시지 처리기입니다.

void  CModelInfoDlg::ClearListBox()
{
	int cnt,i;

	cnt = m_ListMeshNode.GetCount();

	for( i=0;i<cnt;i++ ) {
		m_ListMeshNode.DeleteString(0);
	}

	cnt = m_ListMtrlNode.GetCount();

	for( i=0;i<cnt;i++ ) {
		m_ListMtrlNode.DeleteString(0);
	}

}

void  CModelInfoDlg::Begin()
{
	ClearListBox();

	m_ListMeshNode.SetSel(0);

	m_ListMtrlNode.SetSel(0);

	RVisualMesh* pVMesh = g_vmesh_mgr.GetFast(0);

	if(pVMesh) {

		RMeshNode* pMNode	= NULL;
		RMtrl* pMtrl		= NULL;

		static char temp[256];

		for(int i=0;i<pVMesh->m_pMesh->m_data_num;i++)
		{
			pMNode = pVMesh->m_pMesh->m_data[i];

			if(pMNode) {
				m_ListMeshNode.AddString( pMNode->GetName() );
			}
		}

		RMtrlMgr* pMtrlMgr = &pVMesh->m_pMesh->m_mtrl_list_ex;

		int mtrl_size = pMtrlMgr->size();

		for( i=0;i<mtrl_size;i++) {

			pMtrl = pMtrlMgr->m_node_table[i];

			if(pMtrl) {
				if( pMtrl->m_name[0] ){
					sprintf(temp,"%s",pMtrl->m_name);
				}
				else {
					sprintf(temp,"mtrl %d",i);
				}

				m_ListMtrlNode.AddString( temp );
			}
		}
	}
}

void  CModelInfoDlg::End()
{

}

void CModelInfoDlg::OnBnClickedButtonMeshnodeColor()
{
	CColorDialog dlg;
	COLORREF col;

	BYTE a,r,g,b;

	CString str;

	if( dlg.DoModal()==IDOK ) {

		col = dlg.GetColor();

		a = (col>>24)&0xff;
		b = (col>>16)&0xff;
		g = (col>> 8)&0xff;
		r = (col    )&0xff;

		col = D3DCOLOR_ARGB(255,r,g,b);

		RVisualMesh* pVMesh = g_vmesh_mgr.GetFast(0);

		int sel = m_ListMeshNode.GetCurSel();


		m_ListMeshNode.GetText(sel,str);

		RMeshNode* pMeshNode = pVMesh->m_pMesh->GetMeshData((char*)str.operator const char*());

		if( pMeshNode ) {
			pMeshNode->SetTColor((DWORD)col);
		}
	}
}

void CModelInfoDlg::OnBnClickedButtonMtrlnodeColor()
{
	CColorDialog dlg;
	COLORREF col;

	BYTE a,r,g,b;

	CString str;

	if( dlg.DoModal()==IDOK ) {

		col = dlg.GetColor();

		a = (col>>24)&0xff;
		b = (col>>16)&0xff;
		g = (col>> 8)&0xff;
		r = (col    )&0xff;

		col = D3DCOLOR_ARGB(255,r,g,b);

		RVisualMesh* pVMesh = g_vmesh_mgr.GetFast(0);

		if(!pVMesh) return;

		int sel = m_ListMtrlNode.GetCurSel();

		m_ListMtrlNode.GetText(sel,str);

		RMtrlMgr* pMtrlMgr = &pVMesh->m_pMesh->m_mtrl_list_ex;

		RMtrl* pMtrl = pMtrlMgr->GetMtrl((char*)str.operator const char*());

		if(pMtrl) {
			pMtrl->SetTColor((DWORD)col);
		}
	}
}
