#include "stdafx.h"
#include "Mcv.h"
#include "ScaleDlg.h"


IMPLEMENT_DYNAMIC(CScaleDlg, CDialog)
CScaleDlg::CScaleDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CScaleDlg::IDD, pParent)
{
}

CScaleDlg::~CScaleDlg()
{
}

void CScaleDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_SX, m_sx);
	DDX_Control(pDX, IDC_SY, m_sy);
	DDX_Control(pDX, IDC_SZ, m_sz);
}

BEGIN_MESSAGE_MAP(CScaleDlg, CDialog)
	ON_BN_CLICKED(IDOK, OnBnClickedOk)
	ON_NOTIFY(NM_THEMECHANGED, IDC_SX, OnNMThemeChangedSx)
	ON_NOTIFY(NM_RELEASEDCAPTURE, IDC_SX, OnNMReleasedcaptureSx)
	ON_NOTIFY(NM_RELEASEDCAPTURE, IDC_SY, OnNMReleasedcaptureSy)
	ON_NOTIFY(NM_RELEASEDCAPTURE, IDC_SZ, OnNMReleasedcaptureSz)
END_MESSAGE_MAP()

// 대충하자...
extern float g_scale[3];
extern bool g_bScale;

void CScaleDlg::OnBnClickedOk()
{
	int p;

	p = m_sx.GetPos();
	g_scale[0] = p*0.1f;

	p = m_sy.GetPos();
	g_scale[1] = p*0.1f;

	p = m_sz.GetPos();
	g_scale[2] = p*0.1f;

	g_bScale = !g_bScale;

	OnOK();
}

BOOL CScaleDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	m_sx.SetRange(1,50);
	m_sx.SetPos(10);

	m_sy.SetRange(1,50);
	m_sy.SetPos(10);

	m_sz.SetRange(1,50);
	m_sz.SetPos(10);
	
	return TRUE;  
}

void CScaleDlg::OnNMThemeChangedSx(NMHDR *pNMHDR, LRESULT *pResult)
{
	*pResult = 0;
}

void CScaleDlg::OnNMReleasedcaptureSx(NMHDR *pNMHDR, LRESULT *pResult)
{
	int p = m_sx.GetPos();
	g_scale[0] = p*0.1f;

	*pResult = 0;
}

void CScaleDlg::OnNMReleasedcaptureSy(NMHDR *pNMHDR, LRESULT *pResult)
{
	int p = m_sy.GetPos();
	g_scale[1] = p*0.1f;
	*pResult = 0;
}

void CScaleDlg::OnNMReleasedcaptureSz(NMHDR *pNMHDR, LRESULT *pResult)
{
	int p = m_sz.GetPos();
	g_scale[2] = p*0.1f;
	*pResult = 0;
}
