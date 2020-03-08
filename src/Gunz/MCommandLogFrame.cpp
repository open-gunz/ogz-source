#include "stdafx.h"

#include "MCommandLogFrame.h"
#include "MCommand.h"
#include "MDebug.h"

#define MAX_COMMAND_LOG		200

void MCommandLogFrame::OnSize(int w, int h)
{
	MRECT r = GetClientRect();
	m_pCommandList->SetSize(r.w, r.h);
}
MCommandLogFrame::MCommandLogFrame(const char* szName, MWidget* pParent, MListener* pListener)
: MFrame(szName, pParent, pListener)
{

	m_pCommandList = new MListBox("", this, this);
	MRECT r = GetClientRect();
	m_pCommandList->SetBounds(r);
	m_pCommandList->m_Anchors.m_bLeft = true;
	m_pCommandList->m_Anchors.m_bRight = true;
	m_pCommandList->m_Anchors.m_bTop = true;
	m_pCommandList->m_Anchors.m_bBottom = true;
}

MCommandLogFrame::~MCommandLogFrame(void)
{
	delete m_pCommandList;
}

void MCommandLogFrame::AddCommand(u32 nGlobalClock, MCommand* pCmd)
{
#ifndef _PUBLISH
	char temp[1024];
	char szParam[256];
	MUID uid;

	sprintf_safe(temp, "%u: %s", nGlobalClock, pCmd->m_pCommandDesc->GetName());
	for(int i=0; i<pCmd->GetParameterCount(); i++){
		pCmd->GetParameter(i)->GetString(szParam);
		uid = pCmd->GetSenderUID();
		sprintf_safe(temp, "%s (uid:%d) %s(%s)", temp, uid.Low , typeid(pCmd->GetParameter(i)).name(), szParam);
		
	}
	m_pCommandList->Add(temp);
//	strcat(temp, "\n");
//	MLog(temp);
	if (m_pCommandList->GetCount() > MAX_COMMAND_LOG)
	{
		m_pCommandList->Remove(0);
	}

	int t = m_pCommandList->GetCount() - m_pCommandList->GetShowItemCount();
	m_pCommandList->SetStartItem(t);

#endif
}
