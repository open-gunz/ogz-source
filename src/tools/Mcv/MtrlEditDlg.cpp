#include "stdafx.h"
#include "Mcv.h"
#include "MtrlEditDlg.h"
#include ".\mtrleditdlg.h"

#include "MDebug.h"
#include "RealSpace2.h"

#include "RMtrl.h"

#include "RMeshMgr.h"
#include "RAnimationMgr.h"
#include "RVisualMeshMgr.h"

#include "RBspObject.h"
#include "RMaterialList.h"

#include "stdio.h"

// 대충하자..

extern bool g_bmtrl_dlg;

extern LPDIRECT3DDEVICE9 g_dev;

extern RMeshMgr		g_mesh_mgr;
extern RMeshMgr		g_weapon_mesh_mgr;

extern RAnimationMgr	g_ani_mgr;
extern RVisualMeshMgr	g_vmesh_mgr;

extern int	g_id;

int g_sel = 0;

IMPLEMENT_DYNAMIC(CMtrlEditDlg, CDialog)
CMtrlEditDlg::CMtrlEditDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CMtrlEditDlg::IDD, pParent)
	, m_texture_name(_T(""))
	, m_alpha_texture_name(_T(""))
	, m_nAlphaRefValue(0)
{
}

CMtrlEditDlg::~CMtrlEditDlg()
{
}

void CMtrlEditDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_MTRLLIST, m_mtrl_list_box);
	DDX_Text(pDX, IDC_TEX_NAME, m_texture_name);
	DDX_Text(pDX, IDC_ALPHA_TEX_NAME, m_alpha_texture_name);
	DDX_Control(pDX, IDOKSAVE, m_SaveButton);
	DDX_Text(pDX, IDC_ALPHA_REF, m_nAlphaRefValue);
	DDV_MinMaxByte(pDX, m_nAlphaRefValue, 0, 255);
}


BEGIN_MESSAGE_MAP(CMtrlEditDlg, CDialog)
	ON_BN_CLICKED(IDOKSAVE, OnBnClickedOksave)
	ON_BN_CLICKED(IDCANCEL, OnBnClickedCancel)
	ON_LBN_SELCHANGE(IDC_MTRLLIST, OnLbnSelchangeMtrllist)
END_MESSAGE_MAP()

void CMtrlEditDlg::ClearListBox()
{
	int cnt = m_mtrl_list_box.GetCount();

	// clear listbox

	for(int i=0;i<cnt;i++) {
		m_mtrl_list_box.DeleteString(0);
	}
}

// save 한다..

extern char g_last_open_elu_file[256];


void CMtrlEditDlg::OnBnClickedOksave()
{
	g_bmtrl_dlg = !g_bmtrl_dlg;
	// model 파일을 열어서 앞부분 mtrl 만 덮어쓴다...

	UpdateName();//마지막으로 바꾼것 갱신..

	if(g_last_open_elu_file[0]) {

		// 예전버전 때문에 잠시 지원... 나중에 지우기...

		//<---------------------------------------------------

		ex_hd_t t_hd;
		
		FILE* fp = fopen(g_last_open_elu_file, "rb");

		if(fp==NULL) {
			::MessageBox(NULL,"파일을 확인하세요.","알림",MB_OK);
			return;
		}

		fread(&t_hd,sizeof(ex_hd_t),1,fp);

		fclose(fp);

		//<----------------------------------------------------

		fp = fopen(g_last_open_elu_file, "rb+");

		if(fp==NULL) {
			::MessageBox(NULL,"파일이 읽기 전용인지 확인하세요.","알림",MB_OK);
			return;
		}

		fseek(fp, 16, SEEK_SET);//헤더 skip

		RMtrlMgr* pMtrlMgr = GetMtrlMgr();

		int mtrl_cnt = pMtrlMgr->size();
		
		RMtrl* pMtrl = NULL;

		for(int i=0;i<mtrl_cnt;i++) {

			pMtrl = pMtrlMgr->m_node_table[i];
			
			if(pMtrl == NULL) continue;

			// 구조 정해질때 까지 하나씩 출력

			fwrite(&pMtrl->m_mtrl_id    ,4,1,fp);
			fwrite(&pMtrl->m_sub_mtrl_id,4,1,fp);

			fwrite(&pMtrl->m_ambient ,sizeof(D3DXCOLOR),1,fp);
			fwrite(&pMtrl->m_diffuse ,sizeof(D3DXCOLOR),1,fp);
			fwrite(&pMtrl->m_specular,sizeof(D3DXCOLOR),1,fp);

			fwrite(&pMtrl->m_power,4,1,fp);

			fwrite(&pMtrl->m_sub_mtrl_num,4,1,fp);

			// ver EXPORTER_MESH_VER7

			if(t_hd.ver < EXPORTER_MESH_VER7) {

				fwrite(pMtrl->m_name,MAX_NAME_LEN,1,fp);
				fwrite(pMtrl->m_opa_name,MAX_NAME_LEN,1,fp);

			} else {

				fwrite(pMtrl->m_name,MAX_PATH_NAME_LEN,1,fp);
				fwrite(pMtrl->m_opa_name,MAX_PATH_NAME_LEN,1,fp);

			}

			int nIn = 0;

			if(pMtrl->m_bTwoSided)	nIn = 1;
			else					nIn = 0;

			fwrite(&nIn,sizeof(int),1,fp);

			if(pMtrl->m_bAdditive)	nIn = 1;
			else					nIn = 0;

			fwrite(&nIn,sizeof(int),1,fp);

			if(t_hd.ver > EXPORTER_MESH_VER7) {

				fwrite(&pMtrl->m_nAlphaTestValue,sizeof(int),1,fp);
			}
			
		}
		
		fclose(fp);
	}
	else {
		::MessageBox(NULL,"xml 파일은 지원하지 않습니다.","알림",MB_OK);
	}

	g_sel = 0;

	::MessageBox(NULL,"저장완료","알림",MB_OK);

//	OnOK();
}

void CMtrlEditDlg::OnBnClickedCancel()
{
	g_bmtrl_dlg = !g_bmtrl_dlg;

	g_sel = 0;

	OnCancel();
}

void CMtrlEditDlg::Begin()
{
	// 보여질 때마다 초기화

	ClearListBox();

	g_sel = 0;
	m_mtrl_list_box.SetSel(0);
	RMtrl* pMtrl = GetMtrl(0);
	if(pMtrl) {
		UpdateMtrl( pMtrl );
	}

	RVisualMesh* pVMesh = g_vmesh_mgr.GetFast(0);

	if(pVMesh) {
		if(pVMesh->m_pMesh) {

			RMtrlMgr* pMtrlMgr = &pVMesh->m_pMesh->m_mtrl_list_ex;
			int mtrl_size = pMtrlMgr->size();

			static char temp[256];

			RMtrl* pMtrl = NULL;

			for(int i=0;i<mtrl_size;i++) {

				pMtrl = pMtrlMgr->m_node_table[i];

				if(pMtrl) {
					if( pMtrl->m_name[0] ){
						sprintf(temp,"%s",pMtrl->m_name);
					}
					else {
						sprintf(temp,"mtrl %d",i);
					}

					m_mtrl_list_box.AddString( temp );
				}
			}
		}
	}

}

void CMtrlEditDlg::End()
{
	
}

void CMtrlEditDlg::UpdateMtrl(RMtrl* pMtrl)
{
	if(pMtrl==NULL)
		return;

	m_texture_name = pMtrl->m_name;
	m_alpha_texture_name = pMtrl->m_opa_name;
	m_nAlphaRefValue = pMtrl->m_nAlphaTestValue;

	UpdateData(FALSE);
}

RMtrlMgr* CMtrlEditDlg::GetMtrlMgr()
{
	RVisualMesh* pVMesh = g_vmesh_mgr.GetFast(0);

	if(pVMesh) {

		if(pVMesh->m_pMesh) {

			return &pVMesh->m_pMesh->m_mtrl_list_ex;
		}
	}
	return NULL;
}

RMtrl* CMtrlEditDlg::GetMtrl(int index)
{
	RMtrlMgr* pMtrlMgr = GetMtrlMgr();

	if(pMtrlMgr)
		return pMtrlMgr->m_node_table[index];

	return NULL;
}

void CMtrlEditDlg::UpdateName()
{
	RMtrl* pMtrl = NULL;

	UpdateData(TRUE);

	pMtrl = GetMtrl(g_sel);

	if(pMtrl) {

		if( m_texture_name.operator const char*() )
			strcpy( pMtrl->m_name,m_texture_name.operator const char*() );

		if( m_alpha_texture_name.operator const char*() )
			strcpy( pMtrl->m_opa_name,m_alpha_texture_name.operator const char*() );

		pMtrl->m_nAlphaTestValue = m_nAlphaRefValue;
	}
}

void CMtrlEditDlg::OnLbnSelchangeMtrllist()
{
	RMtrl* pMtrl = NULL;
	// 이전에 선택된것

	UpdateName();

	g_sel = m_mtrl_list_box.GetCurSel();

	pMtrl = GetMtrl(g_sel);

	if(pMtrl) {
		UpdateMtrl( pMtrl );
	}
}
