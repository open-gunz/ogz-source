#include "stdafx.h"
#include "Mcv.h"
#include "RCFrameInfoDlg.h"

#include "MDebug.h"
#include "RealSpace2.h"

#include "RMtrl.h"

#include "RMeshMgr.h"
#include "RAnimationMgr.h"
#include "RVisualMeshMgr.h"

#include "RBspObject.h"
#include "RMaterialList.h"
#include ".\rcframeinfodlg.h"

// 대충하자..

extern bool g_bmtrl_dlg;

extern LPDIRECT3DDEVICE9 g_dev;

extern RMeshMgr		g_mesh_mgr;
extern RMeshMgr		g_weapon_mesh_mgr;

extern RAnimationMgr	g_ani_mgr;
extern RVisualMeshMgr	g_vmesh_mgr;

extern bool g_bframeinfo_dlg;

IMPLEMENT_DYNCREATE(CRCFrameInfoDlg, CDHtmlDialog)

CRCFrameInfoDlg::CRCFrameInfoDlg(CWnd* pParent /*=NULL*/)
	: CDHtmlDialog(CRCFrameInfoDlg::IDD, CRCFrameInfoDlg::IDH, pParent)
{
}

CRCFrameInfoDlg::~CRCFrameInfoDlg()
{
}

void CRCFrameInfoDlg::DoDataExchange(CDataExchange* pDX)
{
	CDHtmlDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_NODE_LISTBOX, m_ListBox);
}

BOOL CRCFrameInfoDlg::OnInitDialog()
{
	CDHtmlDialog::OnInitDialog();
	return TRUE;  // 포커스를 컨트롤에 설정하지 않으면 TRUE를 반환합니다.
}

BEGIN_MESSAGE_MAP(CRCFrameInfoDlg, CDHtmlDialog)
	
	ON_LBN_SELCHANGE(IDC_NODE_LISTBOX, OnLbnSelchangeNodeListbox)
END_MESSAGE_MAP()

BEGIN_DHTML_EVENT_MAP(CRCFrameInfoDlg)
	DHTML_EVENT_ONCLICK(_T("ButtonOK"), OnButtonOK)
	DHTML_EVENT_ONCLICK(_T("ButtonCancel"), OnButtonCancel)
END_DHTML_EVENT_MAP()



// CRCFrameInfoDlg 메시지 처리기입니다.

HRESULT CRCFrameInfoDlg::OnButtonOK(IHTMLElement* /*pElement*/)
{
	g_bframeinfo_dlg = !g_bframeinfo_dlg;
	OnOK();
	return S_OK;  // 포커스를 컨트롤에 설정하지 않으면 TRUE를 반환합니다.
}

HRESULT CRCFrameInfoDlg::OnButtonCancel(IHTMLElement* /*pElement*/)
{
	OnCancel();
	return S_OK;  // 포커스를 컨트롤에 설정하지 않으면 TRUE를 반환합니다.
}

void CRCFrameInfoDlg::ClearListBox()
{
	int cnt = m_ListBox.GetCount();

	for(int i=0;i<cnt;i++) {
		m_ListBox.DeleteString(0);
	}
}

void CRCFrameInfoDlg::Begin()
{
	ClearListBox();

	m_ListBox.SetSel(0);

	RVisualMesh* pVMesh = g_vmesh_mgr.GetFast(0);

	if(pVMesh) {
/*
		// 원래코드 에니메이션 정보
		AniFrameInfo* pInfo = pVMesh->GetFrameInfo(ani_mode_lower);

		if(pInfo) {

			RAnimation* pAni = pInfo->m_pAniSet;

			if( pAni ) {

				RAnimationFile*	pAniFile = pAni->m_pAniData;

				if( pAniFile ) {
					int cnt = pAniFile->m_ani_node_cnt;

					RAnimationNode* pNode = NULL;

					for(int i=0;i<cnt;i++) {
						pNode = pAniFile->m_ani_node[i];

						if(pNode)
							m_ListBox.AddString( pNode->GetName() );
					}
				}
			}
		}
*/
		RMeshNode* pMNode = NULL;

		for(int i=0;i<pVMesh->m_pMesh->m_data_num;i++)
		{
			pMNode = pVMesh->m_pMesh->m_data[i];

			if(pMNode)
				m_ListBox.AddString( pMNode->GetName() );
		}
	}
}

void CRCFrameInfoDlg::End()
{
	
}

void CRCFrameInfoDlg::OnLbnSelchangeNodeListbox()
{
	int nCnt = m_ListBox.GetCount();

	if(nCnt==0) return;

	int nSel = m_ListBox.GetCurSel();

	char	str[1024];
//	CString str;
	m_ListBox.GetText(nSel,str);

	RVisualMesh* pVMesh = g_vmesh_mgr.GetFast(0);

	RMeshNode* pMNode = NULL;

	if(pVMesh&&pVMesh->m_pMesh) {
		 pVMesh->m_pMesh->SetToolSelectNodeName( str );
	}
	
}
