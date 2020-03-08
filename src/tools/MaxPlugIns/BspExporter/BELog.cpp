#include "stdafx.h"
#include "Windows.h"
#include "windowsx.h"
#include "resource.h"
#include "stdio.h"

#define LOGFILENAME "BeLog.txt"

static HWND hWnd,hWnd_Edit;
bool breadytodestroy;

BOOL CALLBACK LogDlgProc ( HWND hWnd, UINT iMsg, WPARAM wParam, LPARAM lParam)
{
	switch (iMsg)
	{
	case WM_INITDIALOG :
		hWnd_Edit=GetDlgItem(hWnd,IDC_EDIT_LOG);
		breadytodestroy=false;
		EnableWindow(GetDlgItem(hWnd,IDC_BUTTON_LOG),false);
		return FALSE;
	case WM_COMMAND:
		switch (LOWORD (wParam))
		{
		case IDC_BUTTON_LOG:
			{
				if(breadytodestroy)
				{
					DestroyWindow(hWnd);
					hWnd=NULL;
					hWnd_Edit=NULL;
				}
			}	
			break;
		}
		return TRUE;
		break;
	}
	return FALSE;
}

bool BEInitLog(HWND hWnd,HINSTANCE hInstance)
{
	::hWnd=CreateDialog(hInstance,MAKEINTRESOURCE(IDD_DIALOG_LOG),hWnd,LogDlgProc);
	if(::hWnd)
	{
		ShowWindow(::hWnd,SW_SHOW);
		UpdateWindow(::hWnd);
	}

	FILE *pFile=fopen(LOGFILENAME,"w");
	fclose(pFile);
	return (::hWnd)?true:false;
}

bool BEInitLog(HWND hWnd)
{
	::hWnd=CreateDialog((HINSTANCE)GetWindowLong(hWnd,GWL_HINSTANCE),MAKEINTRESOURCE(IDD_DIALOG_LOG),hWnd,LogDlgProc);
	if(::hWnd)
	{
		ShowWindow(::hWnd,SW_SHOW);
		UpdateWindow(::hWnd);
	}
	FILE *pFile=fopen(LOGFILENAME,"w");
	fclose(pFile);
	return (::hWnd)?true:false;
}

void __cdecl log(const char *pFormat,...)
{
	char temp[256];

	va_list args;

	va_start(args,pFormat);
	vsprintf(temp,pFormat,args);
	va_end(args);

//#ifdef _DEBUG
	OutputDebugString(temp);
//#endif

	FILE *pFile;
	pFile = fopen( LOGFILENAME, "a" );
	if( !pFile ) pFile=fopen(LOGFILENAME,"w");
	if( pFile==NULL ) return;
	fprintf(pFile,temp);
	fclose(pFile);
	
	if(hWnd_Edit)
	{
		char temp2[256];
		char *s=temp,*d=temp2;
			while(*s) {
				if(*s=='\n') {*d='\r';d++;}
				*d=*s;
				s++;d++;
			}
			*d=0;
		Edit_ReplaceSel(hWnd_Edit,temp2);
	}
}

void BECloseLog()
{
	breadytodestroy=true;
	if(hWnd) {
		EnableWindow(GetDlgItem(hWnd,IDC_BUTTON_LOG),true);
		SetFocus(GetDlgItem(hWnd,IDC_BUTTON_LOG));
	}
}