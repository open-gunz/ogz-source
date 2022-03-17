#include "stdafx.h"

#include "ZObserver.h"
#include "ZGame.h"
#include "ZCharacter.h"
#include "ZCharacterManager.h"
#include "ZCamera.h"
#include "ZGameInterface.h"
#include "MLabel.h"
#include "ZCombatInterface.h"
#include "ZApplication.h"
#include "ZMyInfo.h"
#include "ZGameClient.h"
#include "ZRuleDuel.h"


bool ZObserverQuickTarget::ConvertKeyToIndex(char nKey, int* nIndex) 
{
	int nVal = nKey-'0';
	if (0 <= nVal && nVal <= 9) {
		*nIndex = nVal;
		return true;
	} else {
		return false;
	}
}

bool ZObserver::OnKeyEvent(bool bCtrl, char nKey)
{
	int nIndex = 0;
	if (m_QuickTarget.ConvertKeyToIndex(nKey, &nIndex) == false)
		return false;

	if (bCtrl) {
		m_QuickTarget.StoreTarget(nIndex, GetTargetCharacter()->GetUID());
	} else {
		MUID uidTarget = m_QuickTarget.GetTarget(nIndex);

		// 태거지정 특수키 사용 체크
		if ((uidTarget == MUID(0,0)) && (nKey == OBSERVER_QUICK_TAGGER_TARGET_KEY))
		{
			for (ZCharacterManager::iterator itor = g_pGame->m_CharacterManager.begin(); 
				itor != g_pGame->m_CharacterManager.end(); ++itor)
			{
				ZCharacter* pChar = (*itor).second;
				if (pChar->IsTagger())
				{
					uidTarget = pChar->GetUID();
					break;
				}
			}
		}
		
		ZCharacter* pCharacter = g_pGame->m_CharacterManager.Find(uidTarget);
		if (pCharacter && pCharacter->IsDead() == false)
		{
			SetTarget(uidTarget);
		}
	}
	return true;
}
/////////////////////////////////////////////////


ZObserver::ZObserver()
{
	m_pTargetCharacter = NULL;
	m_bVisible = false;
	m_pIDLResource = NULL;
//	m_nLookType = ZOLM_BACKVIEW;
	m_FreeLookTarget = rvector(0.0f, 0.0f, 0.0f);
	m_nType = ZOM_ANYONE;
}
ZObserver::~ZObserver()
{
	Destroy();
}
bool ZObserver::Create(ZCamera* pCamera, ZIDLResource*	pIDLResource)
{
	m_pCamera = pCamera;
	m_pIDLResource = pIDLResource;
//	m_nLookType = ZOLM_BACKVIEW;

	return true;
}
void ZObserver::Destroy()
{
	ShowInfo(false);
}


void ZObserver::Show(bool bVisible)
{
	if (m_bVisible == bVisible) return;

	m_pCamera->SetLookMode(ZCAMERA_DEFAULT);

	if (bVisible)
	{
		if(g_pGame->GetMatch()->IsTeamPlay()) {
			if(g_pGame->m_pMyCharacter->GetTeamID()==MMT_BLUE)
				m_nType = ZOM_BLUE;
			else
				m_nType = ZOM_RED;
		}
		else
			m_nType = ZOM_ANYONE;

		if(SetFirstTarget())
		{
			m_fDelay = ZOBSERVER_DEFAULT_DELAY_TIME;
			ShowInfo(true);
			m_bVisible = true;
//			ZApplication::GetGameInterface()->SetCursorEnable(true);

			return;
		}
	}

	// 해제	
	m_pTargetCharacter = NULL;
	ShowInfo(false);
	m_bVisible = false;
//	ZApplication::GetGameInterface()->SetCursorEnable(false);

	ZApplication::GetGame()->ReleaseObserver();
}

void ZObserver::ShowInfo(bool bShow)
{
	if (m_pTargetCharacter == NULL) return;
}

void ZObserver::ChangeToNextTarget()
{
	if (g_pGame == NULL) return;
	if (m_pTargetCharacter == NULL) return;

	ZCharacterManager::iterator itor = g_pGame->m_CharacterManager.find(m_pTargetCharacter->GetUID());
	ZCharacter* pCharacter = NULL;
	bool bFlag = false;
	if (itor != g_pGame->m_CharacterManager.end())
	{
		do 
		{
			itor++;

			if (itor == g_pGame->m_CharacterManager.end())
			{
				if (bFlag) 
				{
					// This bugs replays when both players die at the same time
					//Show(false);
					return;
				}

				itor = g_pGame->m_CharacterManager.begin();
				bFlag = true;
			}
			pCharacter = (*itor).second;

		} while (!IsVisibleSetTarget(pCharacter));

		SetTarget(pCharacter);
	}
}

void ZObserver::SetTarget(ZCharacter* pCharacter)
{
	m_pTargetCharacter = pCharacter;
	if (m_pTargetCharacter)
	{
		m_FreeLookTarget = m_pTargetCharacter->GetTargetDir();
	}

	ShowInfo(true);
}

void ZObserver::SetType(ZObserverType nType)
{
	m_nType = nType;
}

bool ZObserver::SetFirstTarget()
{
	for (ZCharacterManager::iterator itor = g_pGame->m_CharacterManager.begin();
		itor != g_pGame->m_CharacterManager.end(); ++itor)
	{
		ZCharacter* pCharacter =  (*itor).second;
		if (IsVisibleSetTarget(pCharacter))
		{
			SetTarget(pCharacter);
			return true;
		}
	}

	// 타겟으로 할만한 사람이 없으면 옵져버 모드 해제
	Show(false);

	return false;
}



bool ZObserver::IsVisibleSetTarget(ZCharacter* pCharacter)
{
	if(pCharacter->IsDead()) return false;
	if (pCharacter->IsAdminHide()) return false;
	if (pCharacter->GetTeamID() == MMT_SPECTATOR) return false;

	if(g_pGame->IsReplay()) return true;

	if(ZGetGameClient()->GetMatchStageSetting()->GetGameType() == MMATCH_GAMETYPE_DUEL)
	{
		ZRuleDuel* pDuel = (ZRuleDuel*)ZGetGameInterface()->GetGame()->GetMatch()->GetRule();
		if (pDuel->GetQueueIdx(pCharacter->GetUID()) <= 1)
			return true;
		else
			return false;
	}

	if (ZGetGameTypeManager()->IsQuestDerived(ZGetGameClient()->GetMatchStageSetting()->GetGameType())) {
		if(!pCharacter->IsVisible()) return false;
	}

	if(m_nType==ZOM_ANYONE) return true;

	if(m_nType==ZOM_RED && pCharacter->GetTeamID()==MMT_RED)
		return true;

	if(m_nType==ZOM_BLUE && pCharacter->GetTeamID()==MMT_BLUE)
		return true;

	return false;

}

void ZObserver::OnDraw(MDrawContext* pDC)
{
	if ( g_pGame->IsReplay() && !g_pGame->IsShowReplayInfo())
		return;

	if ( m_pTargetCharacter == NULL)
		return;

	if ( ZGetCamera()->GetLookMode() == ZCAMERA_MINIMAP)
		return;

	if ( ZGetMyInfo()->IsAdminGrade())
	{
		MFont *pFont=MFontManager::Get("FONTb11b");
		if ( pFont == NULL)
			_ASSERT(0);
		pDC->SetFont(pFont);

		MCOLOR backgroundcolor;
		if ( m_pTargetCharacter->GetTeamID() == MMT_RED)
			backgroundcolor = MCOLOR(100,0,0, 150);
		else if ( m_pTargetCharacter->GetTeamID() == MMT_BLUE)
			backgroundcolor = MCOLOR(0,0,100, 150);
		else 
			backgroundcolor = MCOLOR(0,0,0, 150);

		pDC->SetColor(backgroundcolor);
		pDC->FillRectangle( MGetWorkspaceWidth() / 2 - 170, MGetWorkspaceHeight() * (650.0f/800.0f) - 7, 340, 30);

		backgroundcolor = MCOLOR( 255,255,255, 255);
		pDC->SetColor( backgroundcolor);

		char szName[128];
		sprintf_safe( szName, "%s (HP:%d, AP:%d)", m_pTargetCharacter->GetUserName(), m_pTargetCharacter->GetHP(), m_pTargetCharacter->GetAP());
		TextRelative(pDC, 0.5f, 650.0f/800.0f, szName, true);
	}

	else if ( ZApplication::GetGame()->GetMatch()->GetMatchType() == MMATCH_GAMETYPE_DUEL)
	{
		char	charName[3][100];
		charName[0][0] = charName[1][0] = charName[2][0] = 0;
		float	fMaxHP[ 2]={ 0.0f, 0.0f},	fMaxAP[ 2]={ 0.0f, 0.0f};
		int		nHP[ 2]={ 0, 0},			nAP[ 2]={ 0, 0};
		bool	bExistNextChallenger = false;
		bool	bIsChampOserved = false;
		bool	bIsChlngOserved = false;

		ZRuleDuel* pDuel = (ZRuleDuel*)ZGetGameInterface()->GetGame()->GetMatch()->GetRule();

		for (ZCharacterManager::iterator itor = ZGetCharacterManager()->begin(); itor != ZGetCharacterManager()->end(); ++itor)
		{
			ZCharacter* pCharacter = (*itor).second;

			// Champion
			if (pCharacter->GetUID() == pDuel->QInfo.m_uidChampion)
			{
				strcpy_safe(charName[0], pCharacter->GetUserName());
				fMaxHP[ 0] = pCharacter->GetProperty()->fMaxHP;
				fMaxAP[ 0] = pCharacter->GetProperty()->fMaxAP;
				if ( pCharacter->IsDead())
				{
					nHP[ 0] = 0;
					nAP[ 0] = 0;
				}
				else
				{
					nHP[ 0] = pCharacter->GetHP();
					nAP[ 0] = pCharacter->GetAP();
				}

				if ( m_pTargetCharacter)
				{
					if ( pCharacter->GetUID() == m_pTargetCharacter->GetUID())
						bIsChampOserved = true;
				}
			}

			// Challenger
			else if (pCharacter->GetUID() == pDuel->QInfo.m_uidChallenger)
			{
				strcpy_safe(charName[1], pCharacter->GetUserName());
				fMaxHP[ 1] = pCharacter->GetProperty()->fMaxHP;
				fMaxAP[ 1] = pCharacter->GetProperty()->fMaxAP;
				if ( pCharacter->IsDead())
				{
					nHP[ 1] = 0;
					nAP[ 1] = 0;
				}
				else
				{
					nHP[ 1] = pCharacter->GetHP();
					nAP[ 1] = pCharacter->GetAP();
				}

				if ( m_pTargetCharacter)
				{
					if ( pCharacter->GetUID() == m_pTargetCharacter->GetUID())
						bIsChlngOserved = true;
				}
			}

			// Waiting
			else if (pCharacter->GetUID() == pDuel->QInfo.m_WaitQueue[0])
			{
				strcpy_safe(charName[2], pCharacter->GetUserName());
				bExistNextChallenger = true;
			}
		}

		float fRx = (float)MGetWorkspaceWidth()  / 800.0f;
		float fRy = (float)MGetWorkspaceHeight() / 600.0f;

		int nWidth;
		float fPosy;
		float fLength;
		float fHeight;

		// HP
		fPosy = 10.0f*fRy;
		fLength = 163.0f*fRx;
		fHeight = 23.0f*fRy;

		pDC->SetColor( 255, 0, 0, 210);
		nWidth = (int)( (float)nHP[0] / fMaxHP[0] * fLength);
		pDC->FillRectangle( (193.0f+163.0f)*fRx-nWidth, fPosy, nWidth, fHeight);

		nWidth = (int)( (float)nHP[1] / fMaxHP[1] * fLength);
		pDC->FillRectangle( 444.0f*fRx, fPosy, nWidth, fHeight);


		// AP
		pDC->SetColor( 0, 50, 0, 170);
		pDC->FillRectangle( 218.0f*fRx, 37.0f*fRy, 150.0f*fRx, 5.0f*fRy);
		pDC->FillRectangle( 432.0f*fRx, 37.0f*fRy, 150.0f*fRx, 5.0f*fRy);

		pDC->SetColor( 0, 255, 0, 100);
		nWidth = (int)( (float)nAP[0] / fMaxAP[0] * 150.0f * fRx);
		pDC->FillRectangle( (218.0f+150.0f)*fRx-nWidth, 37.0f*fRy, nWidth, 5.0f*fRy);

		nWidth = (int)( (float)nAP[1] / fMaxAP[1] * 150.0f * fRx);
		pDC->FillRectangle( 432.0f*fRx, 37.0f*fRy, nWidth, 5.0f*fRy);


		// 게이지 프레임 출력
		MBitmap* pBitmap = MBitmapManager::Get( "duel_score.tga");
		if ( pBitmap)
		{
			pDC->SetBitmap( pBitmap);
			pDC->Draw( 167.0f*fRx, 0, 466.0f*fRx, 49.0f*fRx);
		}


		// 이름 출력
		MFont *pFont = MFontManager::Get("FONTa10_O2Wht");
		if ( pFont == NULL)
			_ASSERT(0);
		pDC->SetFont( pFont);
		int nTime = GetGlobalTimeMS() % 200;
		if ( bIsChampOserved && (nTime < 100))
			pDC->SetColor(MCOLOR(0xFFFFFF00));
		else
			pDC->SetColor(MCOLOR(0xFFA0A0A0));
		TextRelative(pDC, 0.34f, 0.026f, charName[0], true);

		if ( bIsChlngOserved && (nTime < 100))
			pDC->SetColor(MCOLOR(0xFFFFFF00));
		else
			pDC->SetColor(MCOLOR(0xFFA0A0A0));
		TextRelative(pDC, 0.66f, 0.026f, charName[1], true);

		if ( bExistNextChallenger)
		{
			MBitmap* pBitmap = MBitmapManager::Get( "icon_play.tga");
			if ( pBitmap)
			{
				pDC->SetBitmap( pBitmap);

				int nIcon = 20.0f*fRx;
				pDC->Draw( 646.0f*fRx, 0, nIcon, nIcon);
				pDC->Draw( 640.0f*fRx, 0, nIcon, nIcon);
			}

			pDC->SetColor( MCOLOR(0xFF808080));
			TextRelative( pDC, 0.83f, 0.01f, charName[ 2], false);
		}

		ZGetCombatInterface()->DrawVictory( pDC, 162, 20, pDuel->QInfo.m_nVictory);
	}
	

	else if ( ZApplication::GetGame()->GetMatch()->GetMatchType() != MMATCH_GAMETYPE_DUEL)
	{
		char szName[128];
		sprintf_safe(szName, "%s (HP:%d, AP:%d)", m_pTargetCharacter->GetUserName(), m_pTargetCharacter->GetHP(), m_pTargetCharacter->GetAP());
		if ( m_pTargetCharacter->IsAdmin())
			pDC->SetColor(MCOLOR(ZCOLOR_ADMIN_NAME));
		else
			pDC->SetColor(MCOLOR(0xFFFFFFFF));

		MFont *pFont = MFontManager::Get( "FONTb11b");
		if ( pFont == NULL)
			_ASSERT(0);
		pDC->SetFont( pFont);

		if ( ZApplication::GetGame()->GetMatch()->GetMatchType() == MMATCH_GAMETYPE_DEATHMATCH_TEAM2)
			TextRelative( pDC, 0.5f, 75.0f/800.0f, szName, true);
		else
			TextRelative( pDC, 0.5f, 50.0f/800.0f, szName, true);
	}

	// 카메라 표시
	if ( !ZGetMyInfo()->IsAdminGrade()) {
		ZCamera *pCamera = ZGetGameInterface()->GetCamera();

		const char *szModes[] = { "normal", "user", "free", "minimap" };
//		TextRelative(pDC, 0.9f, 50.0f/800.0f, szModes[pCamera->GetLookMode()], true);

		char szFileName[ 50];
		sprintf_safe( szFileName, "camera_%s.tga", szModes[pCamera->GetLookMode()]);
		pDC->SetBitmap( MBitmapManager::Get( szFileName));

		float fGain = (float)MGetWorkspaceWidth() / 800.0f;
		pDC->Draw( (int)(720.0f * fGain), (int)(7.0f * fGain), (int)(64.0f * fGain), (int)(64.0f * fGain));
	}



	// Admin 옵져버일 경우에 남은 인원수 표시
	if ( ZGetMyInfo()->IsAdminGrade())
	{
		// 인원수 구하기
		int nNumOfTotal=0, nNumOfRedTeam=0, nNumOfBlueTeam=0;
		ZCharacterManager::iterator itor;
		ZCharacter* pCharacter;
		for (itor = ZGetCharacterManager()->begin(); itor != ZGetCharacterManager()->end(); ++itor)
		{
			pCharacter = (*itor).second;
	
			if ( pCharacter->GetTeamID() == MMT_SPECTATOR)		// 옵저버는 뺸다
				continue;

			if(pCharacter->IsAdminHide()) continue;
		
			if ( (pCharacter->GetTeamID()==4) && ( !pCharacter->IsDead()))
				nNumOfTotal++;
			else if ( (pCharacter->GetTeamID()==MMT_RED) && ( !pCharacter->IsDead()))
				nNumOfRedTeam++;
			else if ( (pCharacter->GetTeamID()==MMT_BLUE) && ( !pCharacter->IsDead()))
				nNumOfBlueTeam++;
		}

		// 팀 이미지 표시
		float sizex = MGetWorkspaceWidth() / 800.f;
		float sizey = MGetWorkspaceHeight() / 600.f;
		char szText[128];

		// 배경 표시
		MCOLOR backgroundcolor;

		if (ZApplication::GetGame()->GetMatch()->IsTeamPlay())
		{
			backgroundcolor = MCOLOR(100,0,0, 150);
			pDC->SetColor(backgroundcolor);
			pDC->FillRectangle( 700 * sizex, 37 * sizey, 85 * sizex, 22 * sizey);
			backgroundcolor = MCOLOR(0,0,100, 150);
			pDC->SetColor(backgroundcolor);
			pDC->FillRectangle( 700 * sizex, 62 * sizey, 85 * sizex, 22 * sizey);

			// 인원수 표시
			backgroundcolor = MCOLOR(255,180,180, 255);
			pDC->SetColor(backgroundcolor);
			sprintf_safe( szText, "%s:%d", ZMsg( MSG_WORD_REDTEAM), nNumOfRedTeam); 
			TextRelative( pDC, 0.92f, 40.0f/600.0f, szText, true);
			backgroundcolor = MCOLOR(180,180,255, 255);
			pDC->SetColor(backgroundcolor);
			sprintf_safe( szText, "%s:%d", ZMsg( MSG_WORD_BLUETEAM), nNumOfBlueTeam); 
			TextRelative( pDC, 0.92f, 65.0f/600.0f, szText, true);
		}
	}

	CheckDeadTarget();
}


void ZObserver::CheckDeadTarget()
{
	static u32 nLastTime = GetGlobalTimeMS();
	static u32 st_nDeadTime = 0;

	u32 nNowTime = GetGlobalTimeMS();

	if (m_pTargetCharacter == NULL) 
	{
		st_nDeadTime = 0;
		nLastTime = nNowTime;
		return;
	}

	if (m_pTargetCharacter->IsDead())
	{
		st_nDeadTime += nNowTime - nLastTime;
	}
	else
	{
		st_nDeadTime = 0;
	}

	if (st_nDeadTime > 3000)
	{
		st_nDeadTime = 0;
		ChangeToNextTarget();
	}

	nLastTime = nNowTime;
}

void ZObserver::SetTarget(const MUID& muid)
{
	ZCharacter* pCharacter = NULL;
	pCharacter = g_pGame->m_CharacterManager.Find(muid);
	if(pCharacter)
	{
		SetTarget(pCharacter);
	}
}

void ZObserver::NextLookMode()
{
	ZCamera *pCamera = ZGetGameInterface()->GetCamera();
	pCamera->SetNextLookMode();
}
