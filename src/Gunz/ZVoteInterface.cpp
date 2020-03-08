#include "stdafx.h"
#include "ZVoteInterface.h"
#include "ZGameClient.h"
#include "RMesh.h"
#include "RVisualMeshMgr.h"
#include "ZCharacterManager.h"
#include "ZPost.h"
#include "ZApplication.h"
#include "ZCombatInterface.h"

#include "ZApplication.h"


bool ZVoteInterface::OnVoteRun(int nTargetIndex)
{
	if (nTargetIndex < 0 || nTargetIndex >= (int)m_TargetList.size())
		return false;

	string strTarget = m_TargetList[nTargetIndex];

	ZPOSTCMD2(MC_MATCH_CALLVOTE, MCmdParamStr("kick"), MCmdParamStr(strTarget.c_str()) );
	ZPOSTCMD0(MC_MATCH_VOTE_YES);	

	Clear();
	return true;
}

void ZVoteInterface::Clear()
{
	ShowTargetList(false);
	m_szDiscuss[0] = NULL;
	m_TargetList.clear();
}

void ZVoteInterface::CallVote(const char* pszDiscuss)
{
	if (GetShowTargetList())
		return;

	Clear();

	SetDiscuss(pszDiscuss);
	ShowTargetList(true);

	// Push Target List
	for(ZCharacterManager::iterator i=ZGetCharacterManager()->begin(); i!=ZGetCharacterManager()->end();i++)
	{
		ZCharacter *pChar = i->second;
		if(pChar->IsAdminHide()) continue;
		m_TargetList.push_back(pChar->GetProperty()->szName);
	}
}

void ZVoteInterface::DrawVoteTargetlist(MDrawContext* pDC)
{
	if(GetShowTargetList() == false) return;

	MFont *pFont=ZGetGameInterface()->GetCombatInterface()->GetGameFont();
	pDC->SetFont(pFont);

	float y = 0.3f;
	float linespace = (float)pFont->GetHeight() * 1.1 / (float)MGetWorkspaceHeight();

	pDC->SetColor(MCOLOR(0xFFFFFFFF));

	TextRelative( pDC, .05f, y, ZMsg(MSG_VOTE_SELECT_PLAYER_TO_KICK) );

	y+=2.f*linespace;

	for (int i=0; i<(int)m_TargetList.size(); i++) 
	{
		string& strName = m_TargetList[i];

		char szBuffer[256];
		sprintf_safe(szBuffer,"[%c] %s", ConvIndexToKey(i), strName.c_str());

		pDC->SetColor(MCOLOR(0xFFFFFFFF));
		TextRelative(pDC,.05f,y,szBuffer);

		y+=linespace;
	}

	y+=linespace;
	pDC->SetColor(MCOLOR(0xFFFFFFFF));
	
	TextRelative(pDC,.05f,y, ZMsg(MSG_VOTE_SELECT_PLAYER_CANCEL));
}

void TextRelative(MDrawContext* pDC,float x,float y,const char *szText,bool bCenter);
void ZVoteInterface::DrawVoteMessage(MDrawContext* pDC)		// 투표가 진행중일때 메시지
{
	// 투표 진행중일때 메시지
	if ( (ZGetGameInterface()->GetState() == GUNZ_GAME) &&
		ZGetGameClient() &&
		ZGetGameClient()->IsVoteInProgress() && 
		ZGetGameClient()->CanVote() ) 
	{
		MFont *pFont=ZGetGameInterface()->GetCombatInterface()->GetGameFont();
		pDC->SetFont(pFont);
		pDC->SetColor(MCOLOR(0x80ffffff));
		TextRelative(pDC,300.f/800.f,550/600.f,ZGetGameClient()->GetVoteMessage());

		if(GetGlobalTimeMS()/500 % 2 == 0 ) {

			TextRelative(pDC,300.f/800.f,565/600.f, ZMsg(MSG_VOTE_YESNO));
		}
	}
}

int ZVoteInterface::ConvKeyToIndex(char nChar)
{
	return (nChar >= 'A') ? nChar-'A'+10 : nChar-'0';
}

char ZVoteInterface::ConvIndexToKey(int nIndex)
{
	return (nIndex >= 10) ? 'A'+nIndex-10 : '0'+nIndex;
}

void ZVoteInterface::VoteInput(char nKey)
{
	if (GetShowTargetList()) {	
		// 1~0 , A~F 까지 - 누른키에 해당하는 플레이어 없을 수도 있음
		int nIndex = ConvKeyToIndex(nKey);
		if (OnVoteRun(nIndex)) {
			ZGetGameClient()->SetCanVote(false);
			ShowTargetList(false);
			return;
		}
	}

	if (ZGetGameClient()->CanVote()) {
		if (nKey == 'Y') {
			ZPOSTCMD0(MC_MATCH_VOTE_YES);
			ZGetGameClient()->SetCanVote(false);
			ShowTargetList(false);
		} else if (nKey == 'N') {
			ZPOSTCMD0(MC_MATCH_VOTE_NO);
			ZGetGameClient()->SetCanVote(false);
			ShowTargetList(false);
		}
		return;
	}
}

void ZVoteInterface::CancelVote()
{
	Clear();
}
