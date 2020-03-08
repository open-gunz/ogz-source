#include "stdafx.h"
#include "MFileDialog.h"
#include "MColorTable.h"
#include "MStringTable.h"
#include "Mint.h"

#define GAP			0
#define INNERGAP	5

#define FILEDIALOG_W	300
#define FILEDIALOG_X	20
#define FILEDIALOG_H	(MGetWorkspaceHeight()-80)
#define FILEDIALOG_Y	40

#define BUTTON_W	80
#define BUTTON_H	26

#define OK_X		(r.x+r.w-BUTTON_W*2-5)
#define OK_Y		(r.y+r.h-BUTTON_H+3)
#define CANCEL_X	(r.x+r.w-BUTTON_W)
#define CANCEL_Y	(OK_Y)

/*
#define BUTTON_W	90
#define BUTTON_H	22

#define OK_X		(r.w-BUTTON_W*2-GAP-INNERGAP)
#define OK_Y		(r.y+r.h-BUTTON_H-GAP)
#define CANCEL_X	(r.w-BUTTON_W-GAP)
#define CANCEL_Y	(OK_Y)
*/

#define FILEBOX_X	(r.x)
#define FILEBOX_Y	(r.y)
#define FILEBOX_W	(r.w-GAP*2)
#define FILEBOX_H	(r.h-BUTTON_H*2-GAP*2-INNERGAP*2)

#define EDIT_X		(r.x)
#define EDIT_Y		(r.y+r.h-BUTTON_H*2-GAP-INNERGAP)
#define EDIT_W		FILEBOX_W
#define EDIT_H		BUTTON_H

bool MFileDialog::IsExistFile(const char* szFileName)
{
	FILE* fp = nullptr;
	fopen_s(&fp, szFileName, "rb");
	if(fp==NULL) return false;
	fclose(fp);
	return true;
}

/*
void MFileDialog::OnDraw(MDrawContext* pDC)
{
	pDC->SetColor(MCOLOR(DEFCOLOR_FRAME_PLANE));
	pDC->FillRectangle(m_Rect);
	pDC->SetColor(MCOLOR(DEFCOLOR_FRAME_OUTLINE));
	pDC->Rectangle(m_Rect);
}
*/
MFileDialog::MFileDialog(const char* szFilter, MWidget* pParent, MListener* pListener)
: MFrame(szFilter, pParent, (pListener==NULL)?pParent:pListener)
{
	SetBounds(MRECT(FILEDIALOG_X, FILEDIALOG_Y, FILEDIALOG_W, FILEDIALOG_H));

	MRECT r = GetClientRect();

	m_pOK = new MButton(MGetString(MSID_OK), this, this);
	m_pOK->SetBounds(MRECT(OK_X, OK_Y, BUTTON_W, BUTTON_H));
	m_pOK->SetLabelAccelerator();
	m_pOK->m_Anchors.m_bLeft = false;
	m_pOK->m_Anchors.m_bRight = true;
	m_pOK->m_Anchors.m_bTop = false;
	m_pOK->m_Anchors.m_bBottom = true;

	m_pCancel = new MButton(MGetString(MSID_CANCEL), this, this);
	m_pCancel->SetBounds(MRECT(CANCEL_X, CANCEL_Y, BUTTON_W, BUTTON_H));
	m_pCancel->SetAccelerator(27);
	m_pCancel->m_Anchors.m_bLeft = false;
	m_pCancel->m_Anchors.m_bRight = true;
	m_pCancel->m_Anchors.m_bTop = false;
	m_pCancel->m_Anchors.m_bBottom = true;

	m_pFileBox = new MFileBox(szFilter, this, this);
	m_pFileBox->SetBounds(MRECT(FILEBOX_X, FILEBOX_Y, FILEBOX_W, FILEBOX_H));
	m_pFileBox->m_Anchors.m_bRight = true;
	m_pFileBox->m_Anchors.m_bBottom = true;

	m_pFileName = new MEdit(_MAX_PATH, "", this, this);
	m_pFileName->SetBounds(MRECT(EDIT_X, EDIT_Y, EDIT_W, EDIT_H));
	//m_pFileName->SetTextColor(MCOLOR(0, 255, 0));
	m_pFileName->m_Anchors.m_bRight = true;
	m_pFileName->m_Anchors.m_bTop = false;
	m_pFileName->m_Anchors.m_bBottom = true;

	m_pMsgBox = new MMsgBox(MGetString(MSID_OVERWRITE), this, this, MT_YESNO);

	m_nType = MFDT_OPEN;

	Show(false);
}

MFileDialog::~MFileDialog(void)
{
	ReleaseExclusive();
	if(m_pMsgBox!=NULL) delete m_pMsgBox;
	if(m_pOK!=NULL) delete m_pOK;
	if(m_pCancel!=NULL) delete m_pCancel;
	if(m_pFileBox!=NULL) delete m_pFileBox;
	if(m_pFileName!=NULL) delete m_pFileName;
}

const char* MFileDialog::GetFileName(void)
{
	return m_pFileName->GetText();
}

const char* MFileDialog::GetPathName(void)
{
	sprintf_safe(m_szTempPathName, "%s\\%s", GetBaseDir(), GetFileName());
	return m_szTempPathName;
}

bool MFileDialog::OnCommand(MWidget* pWindow, const char* szMessage)
{
	MListener* pListener = GetListener();
	if(pListener==NULL) return false;

	if(pWindow==m_pFileBox){
		if(strcmp(szMessage, MLB_ITEM_SEL)==0){
			m_pFileName->SetText(m_pFileBox->GetSelItemString());
		}
		else if(strcmp(szMessage, MLB_ITEM_DBLCLK)==0){
			m_pFileName->SetText(m_pFileBox->GetSelItemString());
			if(m_nType==MFDT_SAVE && IsExistFile(GetPathName())==true){
				m_pMsgBox->Show(true, true);
				return true;
			}
			pListener->OnCommand(this, MFILEDIALOG_OK);
		}
		return true;
	}
	else if(pWindow==m_pOK && strcmp(szMessage, MBTN_CLK_MSG)==0){
		if(strcmp(m_pFileName->GetText(), "")!=0){
			if(m_nType==MFDT_SAVE && IsExistFile(GetPathName())==true){
				m_pMsgBox->Show(true, true);
				return true;
			}
			pListener->OnCommand(this, MFILEDIALOG_OK);
		}
		return true;
	}
	else if(pWindow==m_pCancel && strcmp(szMessage, MBTN_CLK_MSG)==0){
		pListener->OnCommand(this, MFILEDIALOG_CANCEL);
		return true;
	}
	else if(pWindow==m_pMsgBox){
		m_pMsgBox->Show(false);
		if(strcmp(szMessage, MMSGBOX_YES)==0) pListener->OnCommand(this, MFILEDIALOG_OK);
	}
	return false;
}

void MFileDialog::Refresh(const char* szFilter)
{
	m_pFileBox->Refresh(szFilter);
}

const char* MFileDialog::GetBaseDir(void)
{
	return m_pFileBox->GetBaseDir();
}

void MFileDialog::SetType(MFileDialogType fdt)
{
	m_nType = fdt;
}
