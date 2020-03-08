// ConsoleBar.cpp : implementation file
//

#include "stdafx.h"
#ifdef MFC
#include "MatchServer.h"
#include "ConsoleBar.h"
#include "MatchServerDoc.h"
#include "OutputView.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// CConsoleBar dialog

IMPLEMENT_DYNAMIC(CConsoleBar, CDialogBar)
CConsoleBar::CConsoleBar(CWnd* pParent /*=NULL*/)
	: CDialogBar()
{
}

CConsoleBar::~CConsoleBar()
{
}

void CConsoleBar::DoDataExchange(CDataExchange* pDX)
{
	CDialogBar::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CConsoleBar, CDialogBar)
	ON_WM_SIZE()
	ON_CBN_EDITCHANGE(IDC_COMBO_COMMAND, OnCbnEditchangeComboCommand)
	ON_CBN_SELCHANGE(IDC_COMBO_COMMAND, OnCbnSelchangeComboCommand)
	ON_CBN_SELENDOK(IDC_COMBO_COMMAND, OnCbnSelendokComboCommand)
	ON_WM_CREATE()
	ON_CBN_DROPDOWN(IDC_COMBO_COMMAND, OnCbnDropdownComboCommand)
	ON_CBN_EDITUPDATE(IDC_COMBO_COMMAND, OnCbnEditupdateComboCommand)
END_MESSAGE_MAP()


// CConsoleBar message handlers

void CConsoleBar::OnSize(UINT nType, int cx, int cy)
{
	CWnd* pWnd = GetDlgItem(IDC_COMBO_COMMAND);
	if(pWnd!=NULL){
		RECT r, er;
		GetClientRect(&r);
		pWnd->GetWindowRect(&er);
		ScreenToClient(&er);
		er.right = r.right;
		pWnd->MoveWindow(&er);
	}
}

MMonitor* CConsoleBar::GetMonitor(void)
{
	POSITION pos = AfxGetApp()->GetFirstDocTemplatePosition();
	CDocTemplate* pTemplate = AfxGetApp()->GetNextDocTemplate(pos);
	if(pTemplate==NULL) return NULL;
	pos = pTemplate->GetFirstDocPosition();
	if(pos==NULL) return NULL;
	CDocument* pDoc = pTemplate->GetNextDoc(pos);
	if(pDoc->IsKindOf(RUNTIME_CLASS(CMatchServerDoc))==TRUE){
		CMatchServerDoc* pMatchServerDoc = (CMatchServerDoc*)pDoc;
		//return pMatchServerDoc->m_pMonitor;
		return NULL;
	}
	return NULL;
}

void CConsoleBar::GetCurrentCommandString(CString& szCommand)
{
	CWnd* pComboBox = GetDlgItem(IDC_COMBO_COMMAND);
	if(pComboBox==NULL) return;

	pComboBox->GetWindowText(szCommand);
}

void CConsoleBar::PostCommand(void)
{
	CWnd* pComboBox = GetDlgItem(IDC_COMBO_COMMAND);
	if(pComboBox==NULL) return;

	POSITION pos = AfxGetApp()->GetFirstDocTemplatePosition();
	CDocTemplate* pTemplate = AfxGetApp()->GetNextDocTemplate(pos);
	if(pTemplate==NULL){
		pComboBox->SetWindowText("");
		return;
	}
	pos = pTemplate->GetFirstDocPosition();
	CDocument* pDoc = pTemplate->GetNextDoc(pos);
	if(pDoc->IsKindOf(RUNTIME_CLASS(CMatchServerDoc))==TRUE){
		CMatchServerDoc* pMatchServerDoc = (CMatchServerDoc*)pDoc;
		CString szCommand;
		pComboBox->GetWindowText(szCommand);
		pMatchServerDoc->PostCommand(szCommand);
	}

	pComboBox->SetWindowText("");

	return; 
}

void GetSharedString(string* pShared, list<string>* pList)
{
	string shared;
	for(list<string>::iterator it=pList->begin(); it!=pList->end(); it++){
		if(it==pList->begin()) shared = (*it);
		else{
			string compared = (*it);
			int nLen = (int)shared.length();
			for(int i=0; i<nLen; i++){
				if(shared.at(i)!=compared.at(i)){
					shared.at(i) = NULL;
				}
			}
		}
	}

	*pShared = shared;
}

BOOL CConsoleBar::PreTranslateMessage(MSG* pMsg)
{
	//if(pMsg->hwnd==GetDlgItem(IDC_COMBO_COMMAND)->m_hWnd && pMsg->message==WM_KEYDOWN){
	if(pMsg->message==WM_KEYDOWN){
		if(pMsg->wParam == VK_RETURN){
			PostCommand();
		}
		else if(pMsg->wParam == VK_ESCAPE){
			CComboBox* pComboBox = (CComboBox*)GetDlgItem(IDC_COMBO_COMMAND);
			if(pComboBox==NULL) return FALSE;
			pComboBox->ShowDropDown(FALSE);
			pComboBox->SetWindowText("");
			return TRUE;
		}
		else if(pMsg->wParam == VK_TAB){
			CComboBox* pComboBox = (CComboBox*)GetDlgItem(IDC_COMBO_COMMAND);
			if(pComboBox==NULL) return FALSE;
			CString szCommand;
			pComboBox->GetWindowText(szCommand);

			MMonitor* pMonitor = GetMonitor();
			if (pMonitor == NULL)
				return FALSE;
			list<string>	RecommededCommands;
			for(int i=0; i<pMonitor->GetCommandManager()->GetCommandDescCount(); i++){
				MCommandDesc* pCD = pMonitor->GetCommandManager()->GetCommandDesc(i);
				if(_strnicmp(pCD->GetName(), szCommand, szCommand.GetLength())==0){
					RecommededCommands.insert(RecommededCommands.end(), pCD->GetName());
				}
			}
			if(RecommededCommands.size()==1){
				pComboBox->ShowDropDown(FALSE);

				string s = *RecommededCommands.begin();
				pComboBox->SetWindowText(s.c_str());
				//pComboBox->SetEditSel(szCommand.GetLength(), (int)s.size());
				pComboBox->SetEditSel((int)s.size(), (int)s.size());
			}
			else if(RecommededCommands.size()>1){
				string s;
				GetSharedString(&s, &RecommededCommands);

				while(pComboBox->GetCount()>0) pComboBox->DeleteString(0);
				for(list<string>::iterator it=RecommededCommands.begin(); it!=RecommededCommands.end(); it++){
					//pMonitor->OutputMessage(MZMOM_LOCALREPLY, (*it).c_str());
					pComboBox->AddString((*it).c_str());
				}

				pComboBox->SetHorizontalExtent(50);
				pComboBox->ShowDropDown(TRUE);

				pComboBox->SetWindowText(s.c_str());
				pComboBox->SetEditSel((int)s.size(), (int)s.size());
			}
			return TRUE;
		}
	}
	return CDialogBar::PreTranslateMessage(pMsg);
}

void CConsoleBar::OnCbnEditchangeComboCommand()
{
}

void CConsoleBar::OnCbnSelchangeComboCommand()
{
	CString szCommand;
	GetCurrentCommandString(szCommand);

	CComboBox* pComboBox = (CComboBox*)GetDlgItem(IDC_COMBO_COMMAND);
	if(pComboBox==NULL) return;
	pComboBox->SetWindowText(szCommand);
}

void CConsoleBar::OnCbnSelendokComboCommand()
{
}

int CConsoleBar::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CDialogBar::OnCreate(lpCreateStruct) == -1)
		return -1;

	/*
	CComboBox* pComboBox = (CComboBox*)GetDlgItem(IDC_COMBO_COMMAND);
	if(pComboBox!=NULL){
		MMonitor* pMonitor = GetMonitor();
		for(int i=0; i<pMonitor->GetCommandManager()->GetCommandDescCount(); i++){
			MCommandDesc* pCD = pMonitor->GetCommandManager()->GetCommandDesc(i);
			pComboBox->AddString(pCD->GetName());
		}
	}
	*/

	return 0;
}

void CConsoleBar::OnCbnDropdownComboCommand()
{
	/*
	CString szCommand;
	GetCurrentCommandString(szCommand);
	CComboBox* pComboBox = (CComboBox*)GetDlgItem(IDC_COMBO_COMMAND);
	if(pComboBox==NULL) return;
	if(szCommand.IsEmpty()==true){
		while(pComboBox->GetCount()>0) pComboBox->DeleteString(0);

		MMonitor* pMonitor = GetMonitor();
		for(int i=0; i<pMonitor->GetCommandManager()->GetCommandDescCount(); i++){
			MCommandDesc* pCD = pMonitor->GetCommandManager()->GetCommandDesc(i);
			pComboBox->AddString(pCD->GetName());
		}
	}
	*/
}

void CConsoleBar::OnCbnEditupdateComboCommand()
{
	/*
	CComboBox* pComboBox = (CComboBox*)GetDlgItem(IDC_COMBO_COMMAND);
	if(pComboBox==NULL) return;

	CString szCommand;
	pComboBox->GetWindowText(szCommand);

	if(pComboBox->GetCount()==0){
		MMonitor* pMonitor = GetMonitor();
		for(int i=0; i<pMonitor->GetCommandManager()->GetCommandDescCount(); i++){
			MCommandDesc* pCD = pMonitor->GetCommandManager()->GetCommandDesc(i);
			pComboBox->AddString(pCD->GetName());
		}
	}

	pComboBox->SetHorizontalExtent(50);
	pComboBox->ShowDropDown(TRUE);

	for(int i=0; i<pComboBox->GetCount(); i++){
		CString szComboString;
		pComboBox->GetLBText(i, szComboString);
		if(strnicmp(szCommand, szComboString, szCommand.GetLength())==0){
			pComboBox->SetTopIndex(i);
			//pComboBox->SetCurSel(i);
		}
	}

	//pComboBox->SetEditSel(-1, -1);

	CString szSelCommand;
	pComboBox->GetWindowText(szSelCommand);

	pComboBox->SetEditSel(szCommand.GetLength(), szSelCommand .GetLength());
	*/



	/*
	CComboBox* pComboBox = (CComboBox*)GetDlgItem(IDC_COMBO_COMMAND);
	if(pComboBox==NULL) return;

	CString szCommand;
	GetCurrentCommandString(szCommand);

	//pComboBox->ResetContent();
	while(pComboBox->GetCount()>0) pComboBox->DeleteString(0);

	pComboBox->SetHorizontalExtent(50);
	pComboBox->ShowDropDown(TRUE);

	MMonitor* pMonitor = GetMonitor();
	for(int i=0; i<pMonitor->GetCommandManager()->GetCommandDescCount(); i++){
		MCommandDesc* pCD = pMonitor->GetCommandManager()->GetCommandDesc(i);
		if(strnicmp(pCD->GetName(), szCommand, szCommand.GetLength())==0){
			pComboBox->AddString(pCD->GetName());
		}
	}

	if(m_szPrevCommandEdit.GetLength()<szCommand.GetLength() && pComboBox->GetCount()>0){
		pComboBox->SetCurSel(0);

		CString szSelCommand;
		pComboBox->GetWindowText(szSelCommand);

		pComboBox->SetEditSel(szCommand.GetLength(), szSelCommand .GetLength());
	}

	m_szPrevCommandEdit = szCommand;
	*/
}
#endif