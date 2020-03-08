#include "stdafx.h"
#include "MEdit.h"
#include "MColorTable.h"
#include <stdio.h>
#include "Mint.h"
#include "MDebug.h"
#include "MClipboard.h"

#define MEDIT_DEFAULT_WIDTH				100

IMPLEMENT_LOOK(MEdit, MEditLook)

bool InsertString(char* szTarget, const char* szInsert, int nPos, int nMaxTargetLen)
{
	int nTargetLen = strlen(szTarget);
	int nInsertLen = strlen(szInsert);
	if(nPos>nTargetLen) return false;
	if(nMaxTargetLen>0 && nTargetLen+nInsertLen>=nMaxTargetLen) return false;

	int len = nTargetLen - nPos + 2;
	char* temp = new char[len];
	strcpy_safe(temp, len, szTarget+nPos);
	strcpy_safe(szTarget+nPos, nMaxTargetLen - nPos, szInsert);
	strcpy_safe(szTarget+nPos+nInsertLen, nMaxTargetLen - nPos - nInsertLen, temp);
	delete[] temp;

	return true;
}

int DeleteChar(char* szTarget, int nPos)
{
	int nTargetLen = strlen(szTarget);
	if(nPos>=nTargetLen || nPos<0) return 0;

	int nCount = (IsHangul(szTarget[nPos])==true)?2:1;
	if(nPos+nCount>nTargetLen) nCount = 1;

	int len = nTargetLen + 2;
	char* temp = new char[len];
	strcpy_safe(temp, len, szTarget);
	strcpy_safe(szTarget+nPos, len - nPos, temp+nPos+nCount);
	delete[] temp;

	return nCount;
}

int NextPos(const char* szText, int nPos)
{
	int nLen = strlen(szText);

	if(nPos>=nLen) return nLen;

	if(IsHangul(szText[nPos])==true && nPos<nLen) return (nPos+2);

	return (nPos+1);
}

int PrevPos(char* szText, int nPos)
{
	int nLen = strlen(szText);
	if(nPos<=1) return 0;

	int nCurPos=0;
	while(1)
	{
		int nNext = nCurPos + (IsHangul(szText[nCurPos]) ? 2 : 1);
		if(nNext>=nPos) return nCurPos;
		nCurPos=nNext;
	}
}

void MEdit::OnHide()
{
	if (GetTabHandler()) {
		GetTabHandler()->Show(false);
	}
}

static bool g_bFocus = false;

void MEdit::OnSetFocus()
{
	Mint::GetInstance()->EnableIME(true);
}

void MEdit::OnReleaseFocus()
{
	Mint::GetInstance()->EnableIME(false);
}

bool MEdit::OnEvent(MEvent* pEvent, MListener* pListener)
{
	static bool bMyEvent = false;

	if(MWidget::m_pFocusedWidget!=this) return false;
	switch(pEvent->nMessage){
	case MWM_KEYDOWN:
		{
			bool ret= InputFilterKey(pEvent->nKey);

			MListener* pListener = GetListener();
			if(pListener!=NULL) pListener->OnCommand(this, MEDIT_KEYDOWN_MSG);

			return ret;
		}
		break;
	case MWM_CHAR:
		{
			bool ret = false;
			if(InputFilterChar(pEvent->nKey)==false)
			{
				int nLen = strlen(m_pBuffer);
				if(nLen<m_nMaxLength-1)
				{
					char temp[2] = {(char)pEvent->nKey, 0};
					if(InsertString(m_pBuffer, temp, m_nCaretPos, m_nMaxLength)==true)
						m_nCaretPos++;
					_ASSERT(m_nCaretPos>=0 && m_nCaretPos<=(int)strlen(m_pBuffer));
				}
				ret=true;
			}
			MListener* pListener = GetListener();
			if(pListener!=NULL) pListener->OnCommand(this, MEDIT_CHAR_MSG);

			return ret;
		}
		break;
	case MWM_IMECONVERSION:
		{
			if(g_bFocus) OnSetFocus();
		}return false;
	case MWM_IMECOMPOSE:
		strcpy_safe(m_szIMECompositionString, pEvent->szIMECompositionString);
		if(pEvent->szIMECompositionResultString[0]!=NULL){
			if(InsertString(m_pBuffer, pEvent->szIMECompositionResultString, m_nCaretPos, m_nMaxLength)==true){
				m_nCaretPos += strlen(pEvent->szIMECompositionResultString);
				_ASSERT(m_nCaretPos>=0 && m_nCaretPos<=(int)strlen(m_pBuffer));
			}
			MListener* pListener = GetListener();
			if(pListener!=NULL) pListener->OnCommand(this, MEDIT_CHAR_MSG);

		}
		return true;
	case MWM_LBUTTONDOWN:
		{
			MRECT r = GetClientRect();
			if(r.InPoint(pEvent->Pos)==true){
				int nPos = GetPosByScreen(pEvent->Pos.x);
				if(nPos<0) return false;
				m_nCaretPos = nPos;
				bMyEvent = true;
				return true;
			}
			return false;
		}

	case MWM_MOUSEMOVE:
		{			
			if( !bMyEvent ) return false;
		
			m_nSelectionRange = 0;
			int nCaretPos = 0;
			MRECT r = GetClientRect();

			if(r.InPoint(pEvent->Pos)==true){
				int nPos = GetPosByScreen(pEvent->Pos.x);
				if(m_nCaretPos==nPos) return false;
				m_nSelectionRange = nPos - m_nCaretPos;
			}
			return true;
		}
	case MWM_LBUTTONUP:
		{
			if(!bMyEvent) return false;
            bMyEvent = false;
			return true;
		}
	}
	return false;
}

bool MEdit::InputFilterKey(int nKey)
{
	if(nKey==VK_DELETE){
		int nCount = DeleteChar(m_pBuffer, m_nCaretPos);
		return true;
	}
	else if(nKey==VK_LEFT){
		MoveCaretPrev();
		return true;
	}
	else if(nKey==VK_RIGHT){
		MoveCaretNext();
		return true;
	}
	else if(nKey==VK_UP){
		if(m_bSupportHistory==false) return true;

		if(m_nCurrentHistory==m_History.end()){
			m_nCurrentHistory = m_History.end();
			m_nCurrentHistory--;
		}
		else if(m_nCurrentHistory!=m_History.begin()){
			m_nCurrentHistory--;
		}

		if(m_nCurrentHistory!=m_History.end())
			SetText(*m_nCurrentHistory);

		return true;
	}
	else if(nKey==VK_DOWN){
		if(m_bSupportHistory==false) return true;

		if(m_nCurrentHistory!=m_History.end()){
			m_nCurrentHistory++;
		}

		if(m_nCurrentHistory!=m_History.end())
			SetText(*m_nCurrentHistory);
		else
			SetText("");

		return true;
	}
	else if(nKey==VK_HOME){
		MoveCaretHome();
		return true;
	}
	else if(nKey==VK_END){
		MoveCaretEnd();
		return true;
	}
	else if(nKey==VK_RETURN){
		if(m_bSupportHistory==true) AddHistory(GetText());
		MListener* pListener = GetListener();
		if(pListener!=NULL) return pListener->OnCommand(this, MEDIT_ENTER_VALUE);
		return false;
	}
	else if(nKey==VK_TAB){
		if (GetTabHandler()) {
			if (GetTabHandler()->IsVisible())
				GetTabHandler()->Show(false);
			else
				GetTabHandler()->Show(true,true);
			return true;
		} else {
			return false;
		}			
	}

	return false;
}

bool MEdit::InputFilterChar(int nKey)
{
	if(nKey==VK_BACK){
		int nCaretPos = PrevPos(m_pBuffer, m_nCaretPos);
		if(nCaretPos!=m_nCaretPos){
			int nCount = DeleteChar(m_pBuffer, nCaretPos);
			m_nCaretPos = nCaretPos;
			_ASSERT(m_nCaretPos>=0 && m_nCaretPos<=(int)strlen(m_pBuffer));
		}
		if(m_nCaretPos<m_nStartPos) m_nStartPos = m_nCaretPos;

		return true;
	}
	else if(nKey==VK_ESCAPE){
		MListener* pListener = GetListener();
		if(pListener!=NULL) pListener->OnCommand(this, MEDIT_ESC_VALUE);
		return true;
	}
	else if(nKey==22){	// Ctrl+'V'
		char* temp = new char[m_nMaxLength];
		memset(temp, 0, m_nMaxLength);
		if ( GetClipboard(temp, m_nMaxLength)==true)
		{
			char *pFirst = strstr(temp,"\r");
			if(pFirst!=NULL)
			{
				*pFirst = 0;
				*(pFirst+1) = 0;
			}
			pFirst = strstr(temp,"\n");
			if(pFirst!=NULL)
			{
				*pFirst = 0;
				*(pFirst+1) = 0;
			}
			AddText(temp);
		}
		delete temp;
		return true;
	}
	else if(nKey==3){	// Ctrl+'C'
		SetClipboard(GetText());
		return true;
	}

	switch(nKey){
	case VK_TAB:
	case VK_RETURN:
	case VK_NONCONVERT:		// ctrl+]
		return true;
	}

	// ctrl+a ~ z
	if(nKey<27) return true;

	return false;
}

void MEdit::Initialize(int nMaxLength, const char* szName)
{
	m_nMaxLength = nMaxLength+2;
	m_bMouseOver = false;

	MFont* pFont = GetFont();
	int w = MEDIT_DEFAULT_WIDTH;
	int h = pFont->GetHeight()+2;

	m_pBuffer = new char[m_nMaxLength];
	m_pBuffer[0] = NULL;
	if(szName!=NULL) SetText(szName);

	m_szIMECompositionString[0] = NULL;

	m_bPassword = false;

	m_nCaretPos = 0;
	m_nStartPos = 0;
	m_nSelectionRange = 0;

	m_nCurrentHistory = m_History.end();
	m_bSupportHistory = true;

	SetFocusEnable(true);

	SetTabHandler(NULL);
}

MEdit::MEdit(int nMaxLength, const char* szName, MWidget* pParent, MListener* pListener )
 : MWidget(szName, pParent, pListener)
{
	Initialize(nMaxLength, szName);
}

MEdit::MEdit(const char* szName, MWidget* pParent, MListener* pListener)
 : MWidget(szName, pParent, pListener)
{
	Initialize(200, szName);
}

MEdit::~MEdit()
{
	if(m_pBuffer){
		delete []m_pBuffer;
		m_pBuffer = NULL;
	}

	while(m_History.empty()==false){
		char* szText = *m_History.begin();
		delete szText;
		m_History.erase(m_History.begin());
	}
}

void MEdit::SetText(const char* szText)
{
	if(m_pBuffer) {
		strcpy_safe(m_pBuffer, m_nMaxLength, szText);
	}

	MoveCaretEnd();
	if(m_nStartPos>=m_nCaretPos)
		m_nStartPos=m_nCaretPos;
}

const char* MEdit::GetText()
{
	return m_pBuffer;
}


void MEdit::AddText(const char* szText)
{
	char temp[1024];
	sprintf_safe(temp, "%s%s", GetText(), szText);
	SetText(temp);
}

int MEdit::MoveCaretHome()
{
	m_nCaretPos = 0;
	return m_nCaretPos;
}

int MEdit::MoveCaretEnd()
{
	m_nCaretPos = strlen(m_pBuffer);
	return m_nCaretPos;
}

int MEdit::MoveCaretPrev()
{
	m_nCaretPos = PrevPos(m_pBuffer, m_nCaretPos);
	if(m_nCaretPos<m_nStartPos)
	{
		m_nStartPos=m_nCaretPos;
	}
	_ASSERT(m_nCaretPos>=0 && m_nCaretPos<=(int)strlen(m_pBuffer));
	return m_nCaretPos;
}

int MEdit::MoveCaretNext()
{
	m_nCaretPos = NextPos(m_pBuffer, m_nCaretPos);
	_ASSERT(m_nCaretPos>=0 && m_nCaretPos<=(int)strlen(m_pBuffer));
	return m_nCaretPos;
}

int MEdit::GetMaxLength()
{
	return m_nMaxLength;
}

const char* MEdit::GetCompositionString()
{
	return m_szIMECompositionString;
}

int MEdit::GetCarretPos()
{
	return m_nCaretPos;
}

bool MEdit::SetStartPos(int nStartPos)
{
	int nTextLen = strlen(GetText());
	if (nTextLen <= 0) {
		m_nStartPos = 0;
		return false;
	}
	if(nStartPos<0 || nStartPos>=nTextLen) return false;
	m_nStartPos = nStartPos;

	return true;
}

int MEdit::GetPosByScreen(int x)
{
	int nLen = strlen(m_pBuffer+GetStartPos());
	if(nLen==0) return -1;

	MFont* pFont = GetFont();
	MRECT r = GetClientRect();
	int i;
	for(i=nLen; i>0; i=PrevPos(m_pBuffer+GetStartPos(), i)){
		int nWidth = MMGetWidth(pFont, m_pBuffer+GetStartPos(),i);
		if(x>r.x+nWidth){
			return i+GetStartPos();
		}
	}
	if(i==0) return GetStartPos();

	return -1;
}

void MEdit::SetPasswordField(bool bPassword)
{
	m_bPassword = bPassword;
}

bool MEdit::IsPasswordField()
{
	return m_bPassword;
}

bool MEdit::GetClipboard(char* szText, int nSize)
{
	if (m_bPassword)
		return false;

	return MClipboard::Get(Mint::GetInstance()->GetHWND(), szText, static_cast<size_t>(nSize));
}

bool MEdit::SetClipboard(const char* szText)
{
	return MClipboard::Set(Mint::GetInstance()->GetHWND(), szText);
}

void MEdit::AddHistory(const char* szText)
{
	if(szText[0]==0) return;
	int len = strlen(szText) + 2;
	char* szNew = new char[len];
	strcpy_safe(szNew, len, szText);
	m_History.insert(m_History.end(), szNew);
	m_nCurrentHistory = m_History.end();
}

void MEdit::SetMaxLength(int nMaxLength)
{
	nMaxLength += 1;
	char *pNewBuffer = new char[nMaxLength] {};

	if (m_pBuffer != NULL) {
		strcpy_safe(pNewBuffer, nMaxLength, m_pBuffer);
		delete []m_pBuffer;
		m_pBuffer = pNewBuffer;
	}

	m_nMaxLength = nMaxLength;

	MoveCaretEnd();
	if(m_nStartPos>=m_nCaretPos)
		m_nStartPos=m_nCaretPos;
}

void MEditLook::OnFrameDraw(MEdit* pEdit, MDrawContext* pDC)
{
	MRECT r = pEdit->GetInitialClientRect();
	pDC->SetColor(MCOLOR(DEFCOLOR_MEDIT_PLANE));
	pDC->FillRectangle(r);
	pDC->SetColor(MCOLOR(DEFCOLOR_MEDIT_OUTLINE));
	pDC->Rectangle(r);
}

void MEditLook::OnTextDraw(MEdit* pEdit, MDrawContext* pDC, bool bShowLanguageTab )
{
#define BUFFERSIZE	1024

	char szBuffer[BUFFERSIZE];
	_ASSERT(sizeof(szBuffer)>pEdit->GetMaxLength()+2);
	if(pEdit->GetMaxLength()+2>BUFFERSIZE) return;

	if(pEdit->IsPasswordField()==false){
		sprintf_safe(szBuffer, "%s", pEdit->GetText());
	}
	else{
		memset(szBuffer, '*', strlen(pEdit->GetText()));
		szBuffer[strlen(pEdit->GetText())] = '\0';
	}

	// IME Composition String
	if(pEdit->IsFocus()==true && pEdit->GetCompositionString()[0]!=NULL){
		InsertString(szBuffer, pEdit->GetCompositionString(), pEdit->GetCarretPos(), sizeof(szBuffer));
	}

	pDC->SetColor(MCOLOR(DEFCOLOR_MEDIT_TEXT));

	MFont* pFont = pDC->GetFont();
	MRECT r = pEdit->GetClientRect();
	r.x-=2;
	MRECT scr=MClientToScreen(pEdit,r);
	pDC->SetClipRect(scr);

	if( bShowLanguageTab )
		r.w-=pFont->GetHeight();

	if(pEdit->GetCarretPos()<pEdit->GetStartPos())
		pEdit->SetStartPos(pEdit->GetCarretPos());

	_ASSERT(pEdit->GetCarretPos()>=pEdit->GetStartPos());

	int nCompositionStringLength = strlen(pEdit->GetCompositionString());

	char* szTemp = NULL;
	szTemp = szBuffer+pEdit->GetStartPos();
	int nTestLen = pEdit->GetCarretPos()-pEdit->GetStartPos()+nCompositionStringLength;

#define INDICATOR_WIDTH	10
	while(szTemp[0]!=NULL && r.w-INDICATOR_WIDTH < pFont->GetWidth(szTemp, nTestLen)){
		int nStartPos = NextPos(pEdit->GetText(), pEdit->GetStartPos());
		if(pEdit->SetStartPos(nStartPos)==false) break;
		szTemp = szBuffer+pEdit->GetStartPos();
		nTestLen = pEdit->GetCarretPos()-pEdit->GetStartPos()+nCompositionStringLength;
	}

	char* szStartText = szBuffer+pEdit->GetStartPos();

	if(pEdit->IsFocus()==true){
		int nFontHeight = pFont->GetHeight();
		Mint* pMint = Mint::GetInstance();

		int nInsertPosInWidget = pFont->GetWidth(szStartText, pEdit->GetCarretPos()-pEdit->GetStartPos());
		int nCaretPosInWidget = pFont->GetWidth(szStartText, pEdit->GetCarretPos()+pMint->m_nCompositionCaretPosition-pEdit->GetStartPos());

		MRECT r = pEdit->GetClientRect();

		MPOINT cp = MClientToScreen(pEdit, MPOINT(r.x+nInsertPosInWidget, r.y));
		pMint->SetCandidateListPosition(cp, r.h);

		// Caret
		int nSelStartPos=0;
		int nSelEndPos=0;
		
		{
			auto nCurrTime = GetGlobalTimeMS();
			if((nCurrTime%(MEDIT_BLINK_TIME*2))>MEDIT_BLINK_TIME){
				r.x+=nCaretPosInWidget;
				pDC->Text(r, "|", MAM_LEFT);
			}
		}

		r = pEdit->GetClientRect();

		MPOINT p;
		pDC->GetPositionOfAlignment(&p, r, szStartText, MAM_LEFT);
		p.x += nInsertPosInWidget;
		pMint->DrawCompositionAttributes(pDC, p, pEdit->GetCompositionString());
	}


	r = pEdit->GetClientRect();

	pDC->Text(r, szStartText, MAM_LEFT);

	if(pEdit->IsFocus()==true){
		if(bShowLanguageTab){
			Mint* pMint = Mint::GetInstance();
			pMint->DrawIndicator(pDC, pEdit->GetClientRect());
		}
	}
}

void MEditLook::OnDraw(MEdit* pEdit, MDrawContext* pDC, bool bShowLanguageTab)
{
	OnFrameDraw(pEdit, pDC);
	OnTextDraw(pEdit, pDC, bShowLanguageTab);
}

MRECT MEditLook::GetClientRect(MEdit* pEdit, const MRECT& r)
{
	return MRECT(r.x, r.y, r.w, r.h);
}