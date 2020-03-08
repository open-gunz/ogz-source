#include "stdafx.h"
#include "ZCombatQuestScreen.h"
#include "ZCombatInterface.h"
#include "ZCharacter.h"
#include "ZMyCharacter.h"
#include "ZGame.h"
#include "ZCharacterManager.h"
#include "ZModule_QuestStatus.h"
#include "ZActor.h"

/////////////////////////////////////////////////////////////////////////////////
ZCombatQuestScreen::ZCombatQuestScreen()
{
	
}

ZCombatQuestScreen::~ZCombatQuestScreen()
{

}

// NPC 킬수를 기준으로 소팅
bool CompareQuestScreenCharacter(ZCharacter* a, ZCharacter* b) 
{
	ZModule_QuestStatus* pAMod = (ZModule_QuestStatus*)a->GetModule(ZMID_QUESTSTATUS);
	ZModule_QuestStatus* pBMod = (ZModule_QuestStatus*)b->GetModule(ZMID_QUESTSTATUS);

	if ((pAMod) && (pBMod))
	{
		if (pAMod->GetKills() < pBMod->GetKills()) return false;
	}

	return true;
}


void ZCombatQuestScreen::OnDraw(MDrawContext* pDC)
{
	/////////////////////////////////////////////
	list<ZCharacter*>		SortedCharacterList;

	for(ZCharacterManager::iterator itor = ZApplication::GetGame()->m_CharacterManager.begin();
		itor != ZApplication::GetGame()->m_CharacterManager.end(); ++itor)
	{
		ZCharacter* pCharacter = (*itor).second;
		if (!pCharacter->IsVisible()) continue;

		SortedCharacterList.push_back(pCharacter);
	}

	SortedCharacterList.sort(CompareQuestScreenCharacter);
/*
	int cnt = 0;
	for (list<ZCharacter*>::iterator itor = SortedCharacterList.begin(); itor != SortedCharacterList.end(); ++itor)
	{
		DrawPlayer(pDC, cnt, (*itor));
		cnt++;
	}
*/


	if ( ZGetQuest()->IsRoundClear())
	{
		// 운영자 hide는 제외
		bool bEventHide = false;
		if (ZGetMyInfo()->IsAdminGrade()) 
		{
			MMatchObjCache* pCache = ZGetGameClient()->FindObjCache(ZGetMyUID());
			if (pCache && pCache->CheckFlag(MTD_PlayerFlags_AdminHide)) bEventHide = true;
		}

		DWORD dwSec;
		DWORD dwCurrTime = GetGlobalTimeMS();
		if ( ZGetQuest()->GetRemainedTime() < dwCurrTime)
			dwSec = 0;
		else
			dwSec = (ZGetQuest()->GetRemainedTime() - dwCurrTime) / 1000;

		char szSec[ 10];
		sprintf_safe( szSec, "%d", dwSec);
		char szMsg[ 128];
		ZTransMsg( szMsg, MSG_GAME_NEXT_N_MIN_AFTER, 1, szSec);

		pDC->SetFont( MFontManager::Get("FONTa10_O2Wht"));
		pDC->SetColor(MCOLOR(0xFFFFFFFF));

		if (!bEventHide)
			TextRelative(pDC,400.f/800.f,500.f/600.f, szMsg, true);
	}
}


void ZCombatQuestScreen::DrawPlayer(MDrawContext* pDC, int index, ZCharacter* pCharacter)
{
	MFont *pFont = MFontManager::Get("FONTa10b");
	pDC->SetFont( pFont );
	MCOLOR color = MCOLOR(0xFFFFFFFF);
	if (pCharacter->IsDie()) color = MCOLOR(0xFF999999);
	else if (pCharacter == ZApplication::GetGame()->m_pMyCharacter) color = MCOLOR(0xFFEEEE00);
	pDC->SetColor(color);

	char szMsg[128];
	float x, y;
	x = 10.0f / 800.0f;
	y = (200.0f/600.0f) + ((20.0f / 600.0f) * index);

	int screenx = x * MGetWorkspaceWidth();
	int screeny = y * MGetWorkspaceHeight();


	int nKills = 0;
	ZModule_QuestStatus* pMod = (ZModule_QuestStatus*)pCharacter->GetModule(ZMID_QUESTSTATUS);
	if (pMod)
	{
		nKills = pMod->GetKills();
	}

	sprintf_safe(szMsg, "%d. %s / %d K.O.", index+1, pCharacter->GetProperty()->szName, nKills);
	pDC->Text(screenx, screeny, szMsg);

/*
	// hp, ap bar
	const int BAR_HEIGHT = 3;
	const int BAR_WIDTH = 80;
	int bar_hp_width = (int)(BAR_WIDTH * ((float)pCharacter->GetHP() / pCharacter->GetProperty()->fMaxHP));
	int bar_ap_width = (int)(BAR_WIDTH * ((float)pCharacter->GetAP() / pCharacter->GetProperty()->fMaxAP));

	color = MCOLOR(0xFFD6290B);
	int bar_y = screeny + pFont->GetHeight()+2;
	pDC->SetColor(color);
	pDC->FillRectangle(screenx, bar_y, bar_hp_width, BAR_HEIGHT);
	pDC->Rectangle(screenx, bar_y, BAR_WIDTH, BAR_HEIGHT);

	color = MCOLOR(0xFF3AAF3A);
	pDC->SetColor(color);
	bar_y += (BAR_HEIGHT + 2);
	pDC->FillRectangle(screenx, bar_y, bar_ap_width, BAR_HEIGHT);
	pDC->Rectangle(screenx, bar_y, BAR_WIDTH, BAR_HEIGHT);
*/
}