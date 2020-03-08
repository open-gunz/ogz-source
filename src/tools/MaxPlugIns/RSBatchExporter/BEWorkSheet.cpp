#include <stdio.h>
#include <windows.h>
#include <windowsx.h>
#include <commctrl.h>

#include "resource.h"
#include "BEWorkSheet.h"
#include "FileInfo.h"
#include "OpenDlg.h"

BOOL CALLBACK BEWorkSheet_DialogProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

static BECommandList	g_CommandList;
static BECommand		*g_pSelectedCommand;
static AnimationParam	*g_pSelectedAnimation;
static WNDPROC OldListProc;
static HWND	g_hWnd,g_hWnd_List_Export,g_hWnd_List_Animation;
static char g_FileName[256]={0,},g_Destination[256]={0,},g_RMLFile[256]={0,};

BECommandList	*BEWorkSheet_GetCommandList()
{
	return &g_CommandList;
}

int	BEWorkSheet_Work(HWND hWnd)
{
	return DialogBox((HINSTANCE)GetWindowLong(hWnd,GWL_HINSTANCE),
			MAKEINTRESOURCE(IDD_DIALOG_WORKSHEET),hWnd,BEWorkSheet_DialogProc);
}

void BEList_OnDropFiles(HWND hwnd, HDROP hdrop)
{
	_ASSERT((hwnd==g_hWnd_List_Export)||(hwnd==g_hWnd_List_Animation));

	char szFile[256];
	int nCount=DragQueryFile(hdrop, 0xFFFFFFFF, NULL, 0);

	for(int i=0;i<nCount;i++)
	{
		DragQueryFile(hdrop,i,szFile,sizeof(szFile));
		char *ext=_strupr(szFile+strlen(szFile)-3);
		if(stricmp(ext,"max")==0)
		{
			if(hwnd==g_hWnd_List_Export)
			{
				ListBox_AddString(hwnd,szFile);
				BECommand *pCommand=new BECommand;
				pCommand->nCommand=CEXPORT;
				strcpy(pCommand->szBuffer,szFile);
				GetPureFilename(pCommand->szBuffer2,szFile);
				strcat(pCommand->szBuffer2,".rsm");
				g_CommandList.Add(pCommand);
			}
			else
			{
				int nSel=ListBox_GetCurSel(g_hWnd_List_Export);
				if(nSel!=LB_ERR)
				{
					BECommand *pCommand=g_CommandList.Get(nSel);
					ListBox_AddString(hwnd,szFile);
					AnimationParam *pAnim=new AnimationParam;
					pAnim->iAnimationType=AM_TRANSFORM;
					pAnim->fAnimationSpeed=1.0f;
					strcpy(pAnim->szMaxFileName,szFile);
					GetPureFilename(pAnim->szAnimationName,szFile);
					pCommand->AnimationList.Add(pAnim);
				}
			}
		}
	}
}

void BEList_Enable_Export(bool bEnable)
{
	EnableWindow(GetDlgItem(g_hWnd,IDC_EDIT_EXPORT_NAME),bEnable);
	EnableWindow(GetDlgItem(g_hWnd,IDC_STATIC_EXPORT),bEnable);
	EnableWindow(GetDlgItem(g_hWnd,IDC_STATIC_EXPORT_NAME),bEnable);
}

void BEList_Enable_Animation(bool bEnable)
{
	EnableWindow(GetDlgItem(g_hWnd,IDC_EDIT_ANIMATION_NAME),bEnable);
	EnableWindow(GetDlgItem(g_hWnd,IDC_EDIT_ANIMATION_SPEED),bEnable);
	EnableWindow(GetDlgItem(g_hWnd,IDC_COMBO_ANIMATION_METHOD),bEnable);
	EnableWindow(GetDlgItem(g_hWnd,IDC_STATIC_ANIMATION),bEnable);
	EnableWindow(GetDlgItem(g_hWnd,IDC_STATIC_ANIMATION_NAME),bEnable);
	EnableWindow(GetDlgItem(g_hWnd,IDC_STATIC_ANIMATION_SPEED),bEnable);
	EnableWindow(GetDlgItem(g_hWnd,IDC_STATIC_ANIMATION_METHOD),bEnable);
}

void BEList_SyncData()
{
	// sync data
	if(g_pSelectedCommand){
		GetWindowText(GetDlgItem(g_hWnd,IDC_EDIT_EXPORT_NAME),
			g_pSelectedCommand->szBuffer2,sizeof(g_pSelectedCommand->szBuffer2));		
	}
	if(g_pSelectedAnimation)
	{
		char buf[256];
		GetWindowText(GetDlgItem(g_hWnd,IDC_EDIT_ANIMATION_NAME),
			g_pSelectedAnimation->szAnimationName,sizeof(g_pSelectedAnimation->szAnimationName));
		GetWindowText(GetDlgItem(g_hWnd,IDC_EDIT_ANIMATION_SPEED),buf,sizeof(buf));
		g_pSelectedAnimation->fAnimationSpeed=(float)atof(buf);
		
		g_pSelectedAnimation->iAnimationType=
			(ANIMATIONMETHOD)ComboBox_GetCurSel(GetDlgItem(g_hWnd,IDC_COMBO_ANIMATION_METHOD));
	}

}

void BEList_Refresh_Export()
{
	BEList_SyncData();
	int nSel=ListBox_GetCurSel(g_hWnd_List_Export);

	g_pSelectedAnimation=NULL;
	BEList_Enable_Animation(false);
	ListBox_ResetContent(g_hWnd_List_Animation);

	if(nSel!=LB_ERR)
	{
		g_pSelectedCommand=g_CommandList.Get(nSel);

		for(int i=0;i<g_pSelectedCommand->AnimationList.GetCount();i++)
		{
			ListBox_AddString(g_hWnd_List_Animation,g_pSelectedCommand->AnimationList.Get(i)->szMaxFileName);
		}
		SetWindowText(GetDlgItem(g_hWnd,IDC_EDIT_EXPORT_NAME),g_pSelectedCommand->szBuffer2);
		BEList_Enable_Export(true);
	}
	else
	{
		BEList_Enable_Export(false);
		g_pSelectedCommand=NULL;
	}
}

void BEList_Refresh_Animation()
{
	_ASSERT(g_pSelectedCommand);
	BEList_SyncData();

	int nSel=ListBox_GetCurSel(g_hWnd_List_Animation);
	if(nSel!=LB_ERR)
	{
		BEList_Enable_Animation(true);
		g_pSelectedAnimation=g_pSelectedCommand->AnimationList.Get(nSel);
		char buf[256];
		SetWindowText(GetDlgItem(g_hWnd,IDC_EDIT_ANIMATION_NAME),g_pSelectedAnimation->szAnimationName);
		sprintf(buf,"%3.3f",g_pSelectedAnimation->fAnimationSpeed);
		SetWindowText(GetDlgItem(g_hWnd,IDC_EDIT_ANIMATION_SPEED),buf);
		ComboBox_SetCurSel(GetDlgItem(g_hWnd,IDC_COMBO_ANIMATION_METHOD),
			g_pSelectedAnimation->iAnimationType);
	}
	else
	{
		BEList_Enable_Animation(false);
		g_pSelectedAnimation=NULL;
	}
}

BOOL CALLBACK ListProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg) {
		case WM_KEYDOWN:
			{
				switch((int) wParam)
				{
					case VK_DELETE:
						{
							if(hWnd==g_hWnd_List_Export)
							{
								g_pSelectedCommand=NULL;
								g_pSelectedAnimation=NULL;

								int nSel=ListBox_GetCurSel(hWnd);
								if(nSel!=LB_ERR)
								{
									ListBox_DeleteString(hWnd,nSel);
									g_CommandList.Delete(nSel);
									ListBox_SetCurSel(hWnd,nSel);
									BEList_Refresh_Export();
								}
							}
							else
							{
								g_pSelectedAnimation=NULL;

								_ASSERT(g_pSelectedCommand);
								int nSel=ListBox_GetCurSel(hWnd);
								if(nSel!=LB_ERR)
								{
									ListBox_DeleteString(hWnd,nSel);
									g_pSelectedCommand->AnimationList.Delete(nSel);
									ListBox_SetCurSel(hWnd,nSel);
									BEList_Refresh_Animation();
								}
							}
						}
						return FALSE;
					default: OldListProc(hWnd,msg,wParam,lParam);
				}
			}break;
		HANDLE_MSG(hWnd,WM_DROPFILES,BEList_OnDropFiles);
	default:
		return OldListProc(hWnd,msg,wParam,lParam);
	}
	return TRUE;
}

void BEWorkSheet_SetFileName(const char *filename)
{
	strcpy(g_FileName,filename);
}

char *BEWorkSheet_GetFileName()
{
	return g_FileName;
}

void ResetAll()
{
	g_pSelectedCommand=NULL;
	g_pSelectedAnimation=NULL;

	BEList_Enable_Export(false);
	BEList_Enable_Animation(false);

	g_CommandList.DeleteAll();

	g_Destination[0]=0;
	g_RMLFile[0]=0;
	ListBox_ResetContent(g_hWnd_List_Export);
	ListBox_ResetContent(g_hWnd_List_Animation);


	Button_SetCheck(GetDlgItem(g_hWnd,IDC_CHECK_DESTINATION),false);
	Button_SetCheck(GetDlgItem(g_hWnd,IDC_CHECK_RMLFILE),false);

	EnableWindow(GetDlgItem(g_hWnd,IDC_EDIT_DESTINATION),false);
	EnableWindow(GetDlgItem(g_hWnd,IDC_EDIT_RMLFILE),false);

	SetWindowText(GetDlgItem(g_hWnd,IDC_EDIT_DESTINATION),"");
	SetWindowText(GetDlgItem(g_hWnd,IDC_EDIT_RMLFILE),"");

	ComboBox_ResetContent(GetDlgItem(g_hWnd,IDC_COMBO_ANIMATION_METHOD));
	ComboBox_AddString(GetDlgItem(g_hWnd,IDC_COMBO_ANIMATION_METHOD),"Transform");
	ComboBox_AddString(GetDlgItem(g_hWnd,IDC_COMBO_ANIMATION_METHOD),"Vertex");
	ComboBox_AddString(GetDlgItem(g_hWnd,IDC_COMBO_ANIMATION_METHOD),"KeyFrame");
}

void LoadSheet()
{
	if(OpenDlg(g_hWnd, g_FileName, sizeof(g_FileName), 
		"Select Batch Description File to Open",
		"RealSpace Batch Description Files (*.rbe)\0*.rbe\0All Files (*.*)\0*.*\0\0", "rbe"))
	{
		BEParser parser;
		if(parser.Open(GetFileName(g_FileName,0)))
		{
			ResetAll();
			BECommandList *pCommandList=parser.GetCommandList();
			for(int i=0;i<pCommandList->GetCount();i++)
			{
				BECommand *pCommand=pCommandList->Get(i);
				switch(pCommand->nCommand)
				{
				case CDESTINATION	:	strcpy(g_Destination,pCommand->szBuffer);break;
				case CRMLFILE		:	strcpy(g_RMLFile,pCommand->szBuffer);break;
				case CEXPORT :
					{
						g_CommandList.Add(pCommand);
						pCommandList->DeleteRecord(i);
						i--;
					}break;
				}
			}

			for(i=0;i<g_CommandList.GetCount();i++)
			{
				BECommand *pCommand=g_CommandList.Get(i);
				ListBox_AddString(g_hWnd_List_Export,pCommand->szBuffer);
			}

			if(g_Destination[0])
			{
				Button_SetCheck(GetDlgItem(g_hWnd,IDC_CHECK_DESTINATION),true);
				SetWindowText(GetDlgItem(g_hWnd,IDC_EDIT_DESTINATION),g_Destination);
				EnableWindow(GetDlgItem(g_hWnd,IDC_EDIT_DESTINATION),true);
			}
			if(g_RMLFile[0])
			{
				Button_SetCheck(GetDlgItem(g_hWnd,IDC_CHECK_RMLFILE),true);
				SetWindowText(GetDlgItem(g_hWnd,IDC_EDIT_RMLFILE),g_RMLFile);
				EnableWindow(GetDlgItem(g_hWnd,IDC_EDIT_RMLFILE),true);
			}
			
		}
	}
}

void SaveGlobalInformation(FILE *file)
{
	char buf[256];
	bool bUseDest=Button_GetCheck(GetDlgItem(g_hWnd,IDC_CHECK_DESTINATION))!=FALSE;
	bool bUseRML=Button_GetCheck(GetDlgItem(g_hWnd,IDC_CHECK_RMLFILE))!=FALSE;
	if(bUseDest)
	{
		GetWindowText(GetDlgItem(g_hWnd,IDC_EDIT_DESTINATION),buf,sizeof(buf));
		fprintf(file,"destination \"%s\";\n",buf);
	}
	if(bUseRML)
	{
		GetWindowText(GetDlgItem(g_hWnd,IDC_EDIT_RMLFILE),buf,sizeof(buf));
		fprintf(file,"rmlfile \"%s\";\n",buf);
	}
}

void SaveSheet()
{
	BEList_SyncData();

	const char *animationtype[]={ "transform", "vertex", "keyframe" };

	if(OpenDlg(g_hWnd, g_FileName, sizeof(g_FileName), 
		"Select Batch Description File to Save",
		"RealSpace Batch Description Files (*.rbe)\0*.rbe\0All Files (*.*)\0*.*\0\0", "rbe",false))
	{
		FILE *file=fopen(GetFileName(g_FileName,0),"w+");
		if(file)
		{
			SaveGlobalInformation(file);
			for(int i=0;i<g_CommandList.GetCount();i++)
			{
				BECommand *pCommand=g_CommandList.Get(i);
				fprintf(file,"export \"%s\" \"%s\"",pCommand->szBuffer,pCommand->szBuffer2);
				for(int j=0;j<pCommand->AnimationList.GetCount();j++)
				{
					AnimationParam *pAnim=pCommand->AnimationList.Get(j);
					fprintf(file,"\n        animation %s \"%s\" %3.3f \"%s\"",
						animationtype[pAnim->iAnimationType],
						pAnim->szAnimationName,
						pAnim->fAnimationSpeed,
						pAnim->szMaxFileName);
				}
				fprintf(file,";\n");
			}
			fclose(file);
		}
	}
}

BOOL BEWorkSheet_OnInitDialog(HWND hwnd, HWND hwndFocus, LPARAM lParam)
{
	g_hWnd=hwnd;
	g_hWnd_List_Export=GetDlgItem(hwnd,IDC_LIST_EXPORT);
	g_hWnd_List_Animation=GetDlgItem(hwnd,IDC_LIST_ANIMATION);
	OldListProc=SubclassWindow(g_hWnd_List_Export,ListProc);
	SubclassWindow(g_hWnd_List_Animation,ListProc);

	ResetAll();
	return TRUE;
}

void BEWorkSheet_OnOk(HWND hwnd)
{
	BEList_SyncData();
	bool bUseDest=Button_GetCheck(GetDlgItem(g_hWnd,IDC_CHECK_DESTINATION))!=FALSE;
	bool bUseRML=Button_GetCheck(GetDlgItem(g_hWnd,IDC_CHECK_RMLFILE))!=FALSE;
	if(bUseDest)
	{
		BECommand *pCommand=new BECommand;
		pCommand->nCommand=CDESTINATION;
		GetWindowText(GetDlgItem(g_hWnd,IDC_EDIT_DESTINATION),g_Destination,sizeof(g_Destination));
		strcpy(pCommand->szBuffer,g_Destination);
		g_CommandList.InsertHead(pCommand);
	}
	if(bUseRML)
	{
		BECommand *pCommand=new BECommand;
		pCommand->nCommand=CRMLFILE;
		GetWindowText(GetDlgItem(g_hWnd,IDC_EDIT_RMLFILE),g_RMLFile,sizeof(g_RMLFile));
		strcpy(pCommand->szBuffer,g_RMLFile);
		g_CommandList.InsertHead(pCommand);
	}
	
	EndDialog(hwnd, BEWORKSHEET_OK);
}

void BEWorkSheet_OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify)
{
	switch (id) {
	case IDC_LIST_EXPORT:
		switch(codeNotify) {
			case LBN_SELCHANGE:	BEList_Refresh_Export();break;
		}
		break;
	case IDC_LIST_ANIMATION:
		switch(codeNotify) {
			case LBN_SELCHANGE: BEList_Refresh_Animation();break;
		}
		break;
	case IDC_CHECK_DESTINATION:
				EnableWindow(GetDlgItem(g_hWnd,IDC_EDIT_DESTINATION),
					Button_GetCheck(GetDlgItem(g_hWnd,IDC_CHECK_DESTINATION)));break;
	case IDC_CHECK_RMLFILE:
				EnableWindow(GetDlgItem(g_hWnd,IDC_EDIT_RMLFILE),
					Button_GetCheck(GetDlgItem(g_hWnd,IDC_CHECK_RMLFILE)));break;
	case ID_LOAD	:	LoadSheet();break;
	case ID_SAVE	:	SaveSheet();break;
	case IDOK		:	BEWorkSheet_OnOk(hwnd);break;
	case IDCANCEL	:	EndDialog(hwnd,BEWORKSHEET_CANCEL);break;
	}
}

BOOL CALLBACK BEWorkSheet_DialogProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg) {
		HANDLE_MSG(hWnd, WM_COMMAND,		BEWorkSheet_OnCommand);
		HANDLE_MSG(hWnd, WM_INITDIALOG,		BEWorkSheet_OnInitDialog);
	default:
		return FALSE;
	}
	return TRUE;
}       
