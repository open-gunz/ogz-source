#include "stdafx.h"
#include "rbsexp.h"
#include "DialogProc.h"
#include "windowsx.h"
#include "OpenDlg.h"

int g_nTreeDepth;
bool g_bExportRBS,g_bExportXML,g_bExportObjects;

BOOL CALLBACK RBSExportDlgProc(HWND hWnd, UINT msg,WPARAM wParam, LPARAM lParam)
{
	RbsExp *exp = (RbsExp*)GetWindowLong(hWnd,GWL_USERDATA); 
	switch (msg) {
	case WM_INITDIALOG:

		Button_SetCheck(GetDlgItem(hWnd,IDC_CHECK_RBS),TRUE);
		Button_SetCheck(GetDlgItem(hWnd,IDC_CHECK_XML),TRUE);
		Button_SetCheck(GetDlgItem(hWnd,IDC_CHECK_OBJECTS),TRUE);
//		Button_SetCheck(GetDlgItem(hWnd,IDC_CHECK_NAVIGATION),FALSE);


		ComboBox_ResetContent(GetDlgItem(hWnd,IDC_COMBO_DEPTH));
		for(int i=1;i<16;i++)
		{
			char buffer[256];
			sprintf(buffer,"%d",i);
			ComboBox_AddString(GetDlgItem(hWnd,IDC_COMBO_DEPTH),buffer);
		}

//		ComboBox_SetCurSel(GetDlgItem(hWnd,IDC_COMBO_DEPTH),0);
//		g_nTreeDepth=6;	// default;
		char depth[256];
		ComboBox_SetText(GetDlgItem(hWnd,IDC_COMBO_DEPTH),itoa(g_nTreeDepth,depth,10));
		CenterWindow(hWnd, GetParent(hWnd));
		break;

	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDOK:
			{
				g_bExportRBS=Button_GetCheck(GetDlgItem(hWnd,IDC_CHECK_RBS))!=FALSE;
				g_bExportXML=Button_GetCheck(GetDlgItem(hWnd,IDC_CHECK_XML))!=FALSE;
				g_bExportObjects=Button_GetCheck(GetDlgItem(hWnd,IDC_CHECK_OBJECTS))!=FALSE;
//				]=Button_GetCheck(GetDlgItem(hWnd,IDC_CHECK_NAVIGATION))!=FALSE;

				char buffer[256];
				ComboBox_GetText(GetDlgItem(hWnd,IDC_COMBO_DEPTH),buffer,sizeof(buffer));
				g_nTreeDepth=atoi(buffer);

				EndDialog(hWnd, 1);
			}
			break;
		case IDCANCEL:
			EndDialog(hWnd, 0);
			break;
		case IDABOUT:
			DialogBoxParam(hInstance, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, AboutBoxDlgProc, 0);
			break;
		}
		break;
		default:
			return FALSE;
	}
	return TRUE;
}       

BOOL CALLBACK AboutBoxDlgProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg) {
	case WM_INITDIALOG:
		{
			char buf[256];
			CenterWindow(hWnd, GetParent(hWnd)); 
			GetWindowText(GetDlgItem(hWnd,IDC_STATIC_VERSIONINFO),buf,sizeof(buf));
			strcat(buf,__DATE__);strcat(buf,")");
			SetWindowText(GetDlgItem(hWnd,IDC_STATIC_VERSIONINFO),buf);
		}
		break;
	case WM_COMMAND:
		switch (LOWORD(wParam)) {
	case IDOK:
		EndDialog(hWnd, 1);
		break;
		}
		break;
	default:
		return FALSE;
	}
	return TRUE;
}       

