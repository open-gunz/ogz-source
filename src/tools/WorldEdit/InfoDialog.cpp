// InfoDialog.cpp : implementation file
//

#include "stdafx.h"
#include "WorldEdit.h"
#include "InfoDialog.h"
#include "MainFrm.h"
#include "WorldEditDoc.h"
#include "RBspObject.h"
#include "RMaterialList.h"
#include "RBaseTexture.h"

// CInfoDialog dialog

IMPLEMENT_DYNAMIC(CInfoDialog, CDialog)
CInfoDialog::CInfoDialog(CWnd* pParent /*=NULL*/)
	: CDialog(CInfoDialog::IDD, pParent)
{
}

CInfoDialog::~CInfoDialog()
{
}

void CInfoDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CInfoDialog, CDialog)
END_MESSAGE_MAP()


// CInfoDialog message handlers

BOOL CInfoDialog::OnInitDialog()
{
	CDialog::OnInitDialog();

	// TODO:  Add extra initialization here

	CWnd *wnd=GetDlgItem(IDC_EDIT1);

	CMainFrame *pMainFrame=(CMainFrame*)AfxGetApp()->GetMainWnd();

	auto& pBsp=((CWorldEditDoc*)pMainFrame->GetActiveDocument())->m_pBspObject;
	if(!pBsp)
		return FALSE;

	CString info,string;

	// global information
	rvector diff=pBsp->GetDimension();

	string.Format("Dimension : ( %3.1f , %3.1f , %3.1f )\r\n",diff.x,diff.y,diff.z);
	info+=string;

	string.Format("%d convex polygons\r\n",pBsp->GetConvexPolygonCount());
	info+=string;

	string.Format("%d polygons & %d nodes\r\n",pBsp->GetPolygonCount(),pBsp->GetNodeCount());
	info+=string;

	string.Format("%d bsp polygons & %d bsp nodes\r\n",pBsp->GetBspPolygonCount(),pBsp->GetBspNodeCount());
	info+=string;

	int nVBSize=pBsp->GetVertexCount()*sizeof(BSPVERTEX);
	string.Format("%d vertices , vertex buffer size : %d bytes ( %d kbytes )\r\n",pBsp->GetVertexCount(),nVBSize,nVBSize/1024);
	info+=string;

	string.Format("%d lightmap textures \r\n",pBsp->GetLightmapCount());
	info+=string;
	/*
	string.Format("%d lightmap texture ( ",pBsp->GetLightmapCount());
	info+=string;

	for(int i=0;i<pBsp->GetLightmapCount();i++) {
		string.Format("%3.1f ",pBsp->GetUnusedLightmapSize(i));
		info+=string;
	}
	info+=" unused )\r\n";
	*/


	// material 관련 출력한다.
	{
		string.Format("\r\n%d materials\r\n",pBsp->GetMaterialCount());
		info+=string;

		int nCount=0,nTotalBytes=0;
		for(int i=0;i!=pBsp->GetMaterialCount();i++)
		{
			RMATERIAL *pmat=pBsp->GetMaterial(i);
			string.Format("   %d %s : ( %s ) ",nCount,pmat->Name.c_str(),pmat->DiffuseMap.c_str());
			info+=string;

			RBaseTexture *pbt=pBsp->GetBaseTexture(nCount);
			if(pbt && pbt->GetWidth())
			{
				char *szFormat;
				D3DFORMAT fm=pbt->GetFormat();
				switch(fm)
				{
					case D3DFMT_A8R8G8B8	: szFormat="argb32";break;
					case D3DFMT_R8G8B8		: szFormat="rgb32";break;
					default : szFormat="unknown";
				}

				int bpp=4;
				int nSize=pbt->GetWidth()*pbt->GetHeight()*bpp;
				string.Format(" ( %d x %d %s ) %d bytes",pbt->GetWidth(),pbt->GetHeight(),szFormat,nSize);

				nTotalBytes+=nSize;

				info+=string;
			}
			info+="\r\n";
			nCount++;
		}

		string.Format("total %d bytes (%d kb ) texture memory used.\r\n",nTotalBytes,nTotalBytes/1024);
		info+=string;
	}

	string.Format("%d objects\r\n%d map lights\r\n%d object lights\r\n",
		pBsp->GetMapObjectList()->size(), pBsp->GetMapLightList().size(), pBsp->GetObjectLightList().size());
	info+=string;

	// object 관련 출력
	{
		int nCount=0;
		RMapObjectList *pol=pBsp->GetMapObjectList();
		string.Format("\r\n%d map objects\r\n",pol->size());
		info+=string;

		for(RMapObjectList::iterator i=pol->begin();i!=pol->end();i++)
		{
			ROBJECTINFO& poi=*i;
			string.Format("   %d %s\r\n",nCount++,poi.name.c_str());
			info+=string;
		}
	}

	wnd->SetWindowText(info);


	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}
