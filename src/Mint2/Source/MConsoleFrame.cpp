#include "stdafx.h"
#include "MConsoleFrame.h"

#define FRAME_CONSOLE_X 10
#define FRAME_CONSOLE_Y 10
#define FRAME_CONSOLE_WIDTH 400
#define FRAME_CONSOLE_HEIGHT 400

///////////////////////////////////////////////////////////////////////////////////////////
// MConsoleEdit ///////////////////////////////////////////////////////////////////////////
MConsoleEdit::MConsoleEdit(MConsoleFrame* pConsoleFrame, int nMaxLength, const char* szName, MWidget* pParent, MListener* pListener)
				: MEdit(nMaxLength, szName, pParent, pListener)
{
	m_pConsoleFrame = pConsoleFrame;
	m_pfnKeyDown = NULL;
	m_pfnInput = NULL;
}

MConsoleEdit::MConsoleEdit(MConsoleFrame* pConsoleFrame, const char* szName, MWidget* pParent, MListener* pListener)
				: MEdit(szName, pParent, pListener)
{
	m_pConsoleFrame = pConsoleFrame;
	m_pfnKeyDown = NULL;
	m_pfnInput = NULL;
}

MConsoleEdit::~MConsoleEdit()
{

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
				if(toupper(shared[i])!=toupper(compared[i])){
					shared[i] = NULL;
					break;
				}
			}
		}
	}

	*pShared = shared;
}

bool MConsoleEdit::InputFilterKey(int nKey)
{
	bool bReturn = (MEdit::InputFilterKey(nKey));

	if (nKey == VK_RETURN)
	{
		const char* cp = GetText();
		char szBuf[1024];
		strcpy_literal(szBuf, ">");
		strcat_safe(szBuf, cp);

		m_pConsoleFrame->OutputMessage(szBuf);

		if (m_pfnInput)
		{
			m_pfnInput(cp);
		}

		SetText("");
		return true;
	}

	return bReturn;
}

bool MConsoleEdit::InputFilterChar(int nKey)
{
	if (nKey == '`')
	{
		m_pConsoleFrame->Show(false);
		return false;
	}

	return MEdit::InputFilterChar(nKey);
}

bool MConsoleEdit::OnTab(bool bForward)
{
	const char* szCommand = GetText();
	list<string>	RecommededCommands;
	int nCommandLen = strlen(szCommand);
	for(list<string>::iterator i=m_pConsoleFrame->m_Commands.begin(); i!=m_pConsoleFrame->m_Commands.end(); i++){
		const char* szThisCommand = (*i).c_str();
		if(_strnicmp(szThisCommand, szCommand, nCommandLen)==0){
			RecommededCommands.insert(RecommededCommands.end(), *i);
		}
	}
	if(RecommededCommands.size()==1){
		string s = *RecommededCommands.begin();
		SetText(s.c_str());
	}
	else if(RecommededCommands.size()>1){
		string s;
		GetSharedString(&s, &RecommededCommands);

		for(list<string>::iterator it=RecommededCommands.begin(); it!=RecommededCommands.end(); it++){
			m_pConsoleFrame->m_pListBox->Add((*it).c_str());
		}
		int t = m_pConsoleFrame->m_pListBox->GetCount() - m_pConsoleFrame->m_pListBox->GetShowItemCount();
		m_pConsoleFrame->m_pListBox->SetStartItem(t);

		SetText(s.c_str());
	}
	return true;
}


///////////////////////////////////////////////////////////////////////////////////////////
// MConsoleFrame //////////////////////////////////////////////////////////////////////////
MConsoleFrame::MConsoleFrame(const char* szName, MWidget* pParent, MListener* pListener)
	: MFrame(szName, pParent, pListener)
{
//	m_nLineCount = 10;
//	m_LinesColor = MCOLOR(128, 128, 255);

	m_pListBox = new MListBox(this, this);
	m_pListBox->SetVisibleHeader(false);
	m_pListBox->m_Anchors.m_bLeft = true;
	m_pListBox->m_Anchors.m_bRight = true;
	m_pListBox->m_Anchors.m_bTop = true;
	m_pListBox->m_Anchors.m_bBottom = true;

	m_pInputEdit = new MConsoleEdit(this, 255, "", this, this);
	m_pInputEdit->m_bSupportHistory = true;
	m_pInputEdit->m_Anchors.m_bLeft = true;
	m_pInputEdit->m_Anchors.m_bRight = true;
	m_pInputEdit->m_Anchors.m_bTop = false;
	m_pInputEdit->m_Anchors.m_bBottom = true;
	
	SetBounds(MRECT(FRAME_CONSOLE_X, FRAME_CONSOLE_Y, FRAME_CONSOLE_WIDTH, FRAME_CONSOLE_HEIGHT));
}

MConsoleFrame::~MConsoleFrame()
{
	ReleaseExclusive();

	if (m_pListBox != NULL) { delete m_pListBox; m_pListBox = NULL;	}
	if (m_pInputEdit != NULL) { delete m_pInputEdit; m_pInputEdit = NULL; }

}

bool MConsoleFrame::OnShow(void)
{
	m_pInputEdit->SetText("");
	m_pInputEdit->SetFocus();
	return true;
}

void MConsoleFrame::OnDraw(MDrawContext* pDC)
{
	MFrame::OnDraw(pDC);

/*
	MRECT r = GetClientRect();
	static int nRaw = 0;
	pDC->SetColor(MCOLOR(128,128,255));

	int nRawFirst, nRawLast;
	
	nRawFirst = m_Lines.size() - m_nLineCount;
	if (nRawFirst < 0) nRawFirst = 0;
	nRawLast = m_Lines.size();

	for (int i = 0; i < nRawLast - nRawFirst; i++)
	{
		pDC->Text(r.x + 3, r.y+i*(GetFont()->GetHeight()+2), m_Lines[nRawFirst+i].c_str());
	}
*/
}
bool MConsoleFrame::OnCommand(MWidget* pWidget, const char* szMessage)
{
	return false;
}

void MConsoleFrame::OnBrowseCommand(void)
{
}

void MConsoleFrame::OutputMessage(const char* sStr)
{
	m_pListBox->Add(sStr);
	int t = m_pListBox->GetCount() - m_pListBox->GetShowItemCount();
	m_pListBox->SetStartItem(t);

/*
	if (m_Lines.size() >= CONSOLE_LINES_MAX) m_Lines.pop_front();

	string str = sStr;
	string separators = "\n";

	int n = str.length();
	int start, stop;

	start = str.find_first_not_of(separators);
	while ((start >= 0) && (start < n))
	{
		stop = str.find_first_of(separators, start);
		if ((stop < 0) || (stop > n)) stop = n;
		m_Lines.push_back(str.substr(start, stop - start));
		start = str.find_first_not_of(separators, stop+1);
	}
*/
}

void MConsoleFrame::ClearMessage()
{
	m_pListBox->RemoveAll();
//	m_Lines.clear();
}

void MConsoleFrame::SetBounds(MRECT& r)
{
	SetBounds(r.x, r.y, r.w, r.h);
}

void MConsoleFrame::SetBounds(int x, int y, int w, int h)
{
	MFrame::SetBounds(x, y, w, h);
	
	m_pListBox->SetBounds(MRECT(3, 22, w - 6, h - 50));
	m_pInputEdit->SetBounds(MRECT(3, h - 24, w - 6, 20));
//	m_nLineCount = (h - 40) / (GetFont()->GetHeight()+2);
	
}

void MConsoleFrame::AddCommand(const char* szCommand)
{
	m_Commands.push_back(szCommand);
}
