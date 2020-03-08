#include "stdafx.h"
#include "ZStageInterface.h"
#include "ZStageSetting.h"
#include "ZGameInterface.h"
#include "ZPlayerListBox.h"
#include "ZCombatMenu.h"
#include "ZEquipmentListBox.h"
#include "ZMyItemList.h"
#include "ZItemSlotView.h"
#include "ZMessages.h"

ZStageInterface::ZStageInterface()
{
	m_bPrevQuest = false;
	m_bDrawStartMovieOfQuest = false;
	m_pTopBgImg = NULL;
	m_pStageFrameImg = NULL;
	m_pItemListFrameImg = NULL;
	m_nListFramePos = 0;
	m_nStateSacrificeItemBox = 0;
}

ZStageInterface::~ZStageInterface()
{
	if ( m_pTopBgImg != NULL)
	{
		delete m_pTopBgImg;
		m_pTopBgImg = NULL;
	}

	if ( m_pStageFrameImg != NULL)
	{
		delete m_pStageFrameImg;
		m_pStageFrameImg = NULL;
	}
}

void ZStageInterface::OnCreate()
{
	ZIDLResource* pResource = ZApplication::GetGameInterface()->GetIDLResource();

	m_bPrevQuest = false;
	m_bDrawStartMovieOfQuest = false;
	m_nStateSacrificeItemBox = 0;		// Hide
	m_nGameType = MMATCH_GAMETYPE_DEATHMATCH_SOLO;
	m_SacrificeItem[ SACRIFICEITEM_SLOT0].RemoveItem();
	m_SacrificeItem[ SACRIFICEITEM_SLOT1].RemoveItem();

	ReadSenarioNameXML();

	MPicture* pPicture = (MPicture*)pResource->FindWidget( "Stage_SacrificeItemImage0");
	if ( pPicture)
		pPicture->SetOpacity( 255);
	pPicture = (MPicture*)pResource->FindWidget( "Stage_SacrificeItemImage1");
	if ( pPicture)
		pPicture->SetOpacity( 255);

	pPicture = (MPicture*)pResource->FindWidget( "Stage_MainBGTop");
	if ( pPicture)
		pPicture->SetBitmap( MBitmapManager::Get( "main_bg_t.png"));
	pPicture = (MPicture*)pResource->FindWidget( "Stage_FrameBG");
	if ( pPicture)
	{
		m_pStageFrameImg = new MBitmapR2;
		((MBitmapR2*)m_pStageFrameImg)->Create( "stage_frame.png", RGetDevice(), "interface/loadable/stage_frame.png");

		if ( m_pStageFrameImg != NULL)
			pPicture->SetBitmap( m_pStageFrameImg->GetSourceBitmap());
	}
	pPicture = (MPicture*)pResource->FindWidget( "Stage_ItemListBG");
	if ( pPicture)
	{
		m_pItemListFrameImg = new MBitmapR2;
		((MBitmapR2*)m_pItemListFrameImg)->Create( "itemlistframe.tga", RGetDevice(), "interface/loadable/itemlistframe.tga");

		if ( m_pItemListFrameImg != NULL)
			pPicture->SetBitmap( m_pItemListFrameImg->GetSourceBitmap());
	}
	MWidget* pWidget = (MWidget*)pResource->FindWidget( "Stage_ItemListView");
	if ( pWidget)
	{
		MRECT rect;
		rect = pWidget->GetRect();
		rect.x = -rect.w;
		m_nListFramePos = rect.x;
		pWidget->SetBounds( rect);
	}
	MLabel* pLabel = (MLabel*)pResource->FindWidget( "Stage_SenarioName");
	if ( pLabel)
		pLabel->SetText( "");
	pWidget = ZApplication::GetGameInterface()->GetIDLResource()->FindWidget( "Stage_SenarioNameImg");
	if ( pWidget)
		pWidget->Show( false);
	MListBox* pListBox = (MListBox*)pResource->FindWidget( "Stage_SacrificeItemListbox");
	if ( pListBox)
		pListBox->RemoveAll();
	MTextArea* pDesc = (MTextArea*)pResource->FindWidget( "Stage_ItemDesc");
	if ( pDesc)
	{
		pDesc->SetTextColor( MCOLOR(0xFF808080));
		pDesc->SetText( "아이템을 화면 중앙에 있는 두개의 제단에 끌어놓음으로써 게임 레벨을 조정할 수 있습니다.");
	}

	ZApplication::GetGameInterface()->ShowWidget( "Stage_Flame0", false);
	ZApplication::GetGameInterface()->ShowWidget( "Stage_Flame1", false);

	MComboBox* pCombo = (MComboBox*)pResource->FindWidget("StageType");
	if ( pCombo)
		pCombo->CloseComboBoxList();

	pCombo = (MComboBox*)pResource->FindWidget("MapSelection");
	if ( pCombo)
		pCombo->CloseComboBoxList();


	pWidget = (MWidget*)pResource->FindWidget( "ChannelListFrame");
	if ( pWidget)
		pWidget->Show( false);


	UpdateSacrificeItem();
	SerializeSacrificeItemListBox();

	OnResponseQL( 0);
}

void ZStageInterface::OnDestroy()
{
	ZApplication::GetGameInterface()->ShowWidget( "Stage", false);

	MPicture* pPicture = (MPicture*)ZApplication::GetGameInterface()->GetIDLResource()->FindWidget( "Stage_MainBGTop");
	if ( pPicture)
		pPicture->SetBitmap( MBitmapManager::Get( "main_bg_t.png"));
	pPicture = (MPicture*)ZApplication::GetGameInterface()->GetIDLResource()->FindWidget( "Stage_FrameBG");
	if ( pPicture)
		pPicture->SetBitmap( NULL);
	pPicture = (MPicture*)ZApplication::GetGameInterface()->GetIDLResource()->FindWidget( "Stage_ItemListBG");
	if ( pPicture)
		pPicture->SetBitmap( NULL);

	if ( m_pTopBgImg != NULL)
	{
		delete m_pTopBgImg;
		m_pTopBgImg = NULL;
	}
	if ( m_pStageFrameImg != NULL)
	{
		delete m_pStageFrameImg;
		m_pStageFrameImg = NULL;
	}
	if ( m_pItemListFrameImg != NULL)
	{
		delete m_pItemListFrameImg;
		m_pItemListFrameImg = NULL;
	}

	MWidget* pWidget = ZApplication::GetGameInterface()->GetIDLResource()->FindWidget( "Stage_CharacterInfo");
	if ( pWidget)
		pWidget->Enable( true);
}

void ZStageInterface::OnStageInterfaceSettup()
{
	ZIDLResource* pResource = ZApplication::GetGameInterface()->GetIDLResource();

	ZStageSetting::InitStageSettingGameType();

	MSTAGE_CHAR_SETTING_NODE* pMyCharNode = NULL;
	bool bMyReady = false;
	for (auto&& CharNode : ZGetGameClient()->GetMatchStageSetting()->m_CharSettingList) 
	{
		if ( CharNode.uidChar == ZGetGameClient()->GetPlayerUID()) 
		{
			pMyCharNode = &CharNode;
			if (pMyCharNode->nState == MOSS_READY)
				bMyReady = true;
			else
				bMyReady = false;
		}

		ZPlayerListBox* pItem = (ZPlayerListBox*)pResource->FindWidget( "StagePlayerList_");
		if ( pItem)
		{
			bool bMaster = false;

			if ( ZGetGameClient()->GetMatchStageSetting()->GetMasterUID() == CharNode.uidChar)
				bMaster = true;
			
			pItem->UpdatePlayer(CharNode.uidChar, CharNode.nState,
				bMaster, MMatchTeam(CharNode.nTeam));
		}
	}

	ChangeStageButtons( ZGetGameClient()->IsForcedEntry(), ZGetGameClient()->AmIStageMaster(), bMyReady);

	ChangeStageGameSetting( ZGetGameClient()->GetMatchStageSetting()->GetStageSetting());
	
	if ( !ZGetGameClient()->AmIStageMaster() && ( ZGetGameClient()->IsForcedEntry()))
	{
		if ( pMyCharNode != NULL)
			ChangeStageEnableReady( bMyReady);
	}

	if ( (ZGetGameClient()->AmIStageMaster() == true) && ( ZGetGameClient()->IsForcedEntry()))
	{
		bool b = ZGetGameClient()->GetMatchStageSetting()->GetStageState() == STAGE_STATE_STANDBY;
		if (b)
		{
			ZGetGameClient()->ReleaseForcedEntry();
		}

		ZGetGameInterface()->EnableWidget( "StageSettingCaller", b);
		ZGetGameInterface()->EnableWidget( "MapSelection", b);
		ZGetGameInterface()->EnableWidget( "StageType", b);
		ZGetGameInterface()->EnableWidget( "StageMaxPlayer", b);
		ZGetGameInterface()->EnableWidget( "StageRoundCount", b);
	}

	MPicture* pPicture = 0;
	MBitmap* pBitmap = 0;
	char szMapName[256];
 	pPicture = (MPicture*)pResource->FindWidget( "Stage_MainBGTop");
	if ( pPicture)
	{
		sprintf_safe(szMapName, "interface/loadable/%s",
			MGetMapImageName( ZGetGameClient()->GetMatchStageSetting()->GetMapName()));

		if ( m_pTopBgImg != NULL)
		{
			delete m_pTopBgImg;
			m_pTopBgImg = NULL;
		}

		m_pTopBgImg = new MBitmapR2;
		((MBitmapR2*)m_pTopBgImg)->Create( "TopBgImg.png", RGetDevice(), szMapName);

		if ( m_pTopBgImg != NULL)
			pPicture->SetBitmap( m_pTopBgImg->GetSourceBitmap());
	}
	
	MLabel* pLabel = (MLabel*)pResource->FindWidget( "StageNameLabel");
	if ( pLabel != 0)
	{
		char szStr[ 256];
		sprintf_safe( szStr, "%s > %s > %03d:%s",
			ZGetGameClient()->GetServerName(), ZMsg( MSG_WORD_STAGE),
			ZGetGameClient()->GetStageNumber(), ZGetGameClient()->GetStageName());
		pLabel->SetText( szStr);
	}

#define SDM_COLOR			MCOLOR(255,0,0)
#define TDM_COLOR			MCOLOR(0,255,0)
#define SGD_COLOR			MCOLOR(0,0,255)
#define TGD_COLOR			MCOLOR(255,255,0)
#define ASSASIN_COLOR		MCOLOR(255,0,255)
#define TRAINING_COLOR		MCOLOR(0,255,255)
#define QUEST_COLOR			MCOLOR(255,255,255)
#define SURVIVAL_COLOR		MCOLOR(255,255,255)

	MCOLOR color;
	switch ( ZGetGameClient()->GetMatchStageSetting()->GetGameType() )
	{	
		case MMATCH_GAMETYPE_ASSASSINATE:
			color = ASSASIN_COLOR;
			break;

		case MMATCH_GAMETYPE_DEATHMATCH_SOLO:
		case MMATCH_GAMETYPE_GUNGAME:
			color = SDM_COLOR;
			break;

		case MMATCH_GAMETYPE_DEATHMATCH_TEAM:
		case MMATCH_GAMETYPE_DEATHMATCH_TEAM2:
			color = TDM_COLOR;
			break;

		case MMATCH_GAMETYPE_GLADIATOR_SOLO:
			color = SGD_COLOR;
			break;

		case MMATCH_GAMETYPE_GLADIATOR_TEAM:
			color = TGD_COLOR;
			break;

		case MMATCH_GAMETYPE_TRAINING:
			color = TRAINING_COLOR;
			break;

#ifdef _QUEST
		case MMATCH_GAMETYPE_SURVIVAL:
			color = QUEST_COLOR;
			break;

		case MMATCH_GAMETYPE_QUEST:
			color = SURVIVAL_COLOR;
			break;
#endif
		case MMATCH_GAMETYPE_BERSERKER:
			color = SDM_COLOR;
			break;

		case MMATCH_GAMETYPE_DUEL:
			color = SDM_COLOR;
			break;

		case MMATCH_GAMETYPE_SKILLMAP:
			color = TRAINING_COLOR;
			break;

		default:
			_ASSERT(0);
			color = MCOLOR(255,255,255,255);
	}
	pPicture = (MPicture*)ZApplication::GetGameInterface()->GetIDLResource()->FindWidget("Stage_StripBottom");
	if(pPicture != NULL && !pPicture->IsAnim())
	{		
        pPicture->SetBitmapColor( color );
		if(!(pPicture->GetBitmapColor().GetARGB() == pPicture->GetReservedBitmapColor().GetARGB()))
			pPicture->SetAnimation( 2, 700.0f);
	}
	pPicture = (MPicture*)ZApplication::GetGameInterface()->GetIDLResource()->FindWidget("Stage_StripTop");
	if(pPicture != NULL && !pPicture->IsAnim())	
	{
		pPicture->SetBitmapColor( color );
		if(!(pPicture->GetBitmapColor().GetARGB() == pPicture->GetReservedBitmapColor().GetARGB()))
			pPicture->SetAnimation( 3, 700.0f);		
	}
}

void ZStageInterface::ChangeStageGameSetting( MSTAGE_SETTING_NODE* pSetting)
{
	ZIDLResource* pResource = ZApplication::GetGameInterface()->GetIDLResource();

	m_nGameType = pSetting->nGameType;

	SetMapName( pSetting->szMapName);

	ZApplication::GetGameInterface()->m_bTeamPlay = ZGetGameTypeManager()->IsTeamGame( pSetting->nGameType);

	MComboBox* pCombo = (MComboBox*)pResource->FindWidget( "StageObserver");
	MButton* pObserverBtn = (MButton*)pResource->FindWidget( "StageObserverBtn");
	MLabel* pObserverLabel = (MLabel*)pResource->FindWidget( "StageObserverLabel");
	if ( pCombo && pObserverBtn && pObserverLabel)
	{
		if ( pCombo->GetSelIndex() == 1)
		{
			pObserverBtn->SetCheck( false);
			pObserverBtn->Enable( false);
			pObserverLabel->Enable( false);
		}
		else
		{
			pObserverBtn->Enable( true);
			pObserverLabel->Enable( true);
		}
	}

	ZApplication::GetGameInterface()->UpdateBlueRedTeam();

	MAnimation* pAniMapImg = (MAnimation*)pResource->FindWidget( "Stage_MapNameBG");
	bool bQuestUI = false;
	if ( (pSetting->nGameType == MMATCH_GAMETYPE_DEATHMATCH_SOLO) ||
		 (pSetting->nGameType == MMATCH_GAMETYPE_GLADIATOR_SOLO) ||
		 (pSetting->nGameType == MMATCH_GAMETYPE_BERSERKER) ||
		 (pSetting->nGameType == MMATCH_GAMETYPE_TRAINING) ||
		 (pSetting->nGameType == MMATCH_GAMETYPE_DUEL) ||
		 (pSetting->nGameType == MMATCH_GAMETYPE_GUNGAME))
	{
		if ( pAniMapImg)
			pAniMapImg->SetCurrentFrame( 0);

		bQuestUI = false;
	}
	else if ( (pSetting->nGameType == MMATCH_GAMETYPE_DEATHMATCH_TEAM) ||
		(pSetting->nGameType == MMATCH_GAMETYPE_DEATHMATCH_TEAM2) ||
		 (pSetting->nGameType == MMATCH_GAMETYPE_GLADIATOR_TEAM) ||
		 (pSetting->nGameType == MMATCH_GAMETYPE_ASSASSINATE))
	{
		if ( pAniMapImg)
			pAniMapImg->SetCurrentFrame( 1);

		bQuestUI = false;
	}
	else if ( pSetting->nGameType == MMATCH_GAMETYPE_SURVIVAL)
	{
		if ( pAniMapImg)
			pAniMapImg->SetCurrentFrame( 0);

		bQuestUI = false;
	}
	else if ( pSetting->nGameType == MMATCH_GAMETYPE_QUEST)
	{
		if ( pAniMapImg)
			pAniMapImg->SetCurrentFrame( 2);

		bQuestUI = true;
	}

	ZApplication::GetGameInterface()->ShowWidget( "Stage_SacrificeItemImage0", bQuestUI);
	ZApplication::GetGameInterface()->ShowWidget( "Stage_SacrificeItemImage1", bQuestUI);
	ZApplication::GetGameInterface()->ShowWidget( "Stage_Lights0", bQuestUI);
	ZApplication::GetGameInterface()->ShowWidget( "Stage_Lights1", bQuestUI);
	ZApplication::GetGameInterface()->ShowWidget( "Stage_QuestLevel", bQuestUI);
	ZApplication::GetGameInterface()->ShowWidget( "Stage_QuestLevelBG", bQuestUI);
	ZApplication::GetGameInterface()->ShowWidget( "Stage_SacrificeItemButton0", bQuestUI);
	ZApplication::GetGameInterface()->ShowWidget( "Stage_SacrificeItemButton1", bQuestUI);

	if ( m_bPrevQuest != bQuestUI)
	{
		m_SacrificeItem[ SACRIFICEITEM_SLOT0].RemoveItem();
		m_SacrificeItem[ SACRIFICEITEM_SLOT1].RemoveItem();

		UpdateSacrificeItem();

		if ( bQuestUI)
		{
			ZPostRequestSacrificeSlotInfo( ZGetGameClient()->GetPlayerUID());
			ZPostRequestQL( ZGetGameClient()->GetPlayerUID());
			OpenSacrificeItemBox();
		}
		else
		{
			MLabel* pLabel = (MLabel*)pResource->FindWidget( "Stage_SenarioName");
			if ( pLabel)
				pLabel->SetText( "");
			ZApplication::GetGameInterface()->ShowWidget( "Stage_SenarioNameImg", false);

			HideSacrificeItemBox();
		}

		m_bPrevQuest = !m_bPrevQuest;
	}

	if ( (pSetting->nGameType == MMATCH_GAMETYPE_SURVIVAL) || (pSetting->nGameType == MMATCH_GAMETYPE_QUEST))
		ZApplication::GetGameInterface()->EnableWidget( "StageSettingCaller", false);

	MWidget* pWidget = ZApplication::GetGameInterface()->GetIDLResource()->FindWidget( "StageRoundCountLabel");
	if ( pWidget)
	{
		if ((pSetting->nGameType == MMATCH_GAMETYPE_DEATHMATCH_SOLO) ||
			(pSetting->nGameType == MMATCH_GAMETYPE_GLADIATOR_SOLO) ||
			(pSetting->nGameType == MMATCH_GAMETYPE_TRAINING) ||
			(pSetting->nGameType == MMATCH_GAMETYPE_BERSERKER) ||
			(pSetting->nGameType == MMATCH_GAMETYPE_DUEL) ||
			(pSetting->nGameType == MMATCH_GAMETYPE_GUNGAME))
			pWidget->SetText( ZMsg(MSG_WORD_KILL));

		else
			pWidget->SetText( ZMsg(MSG_WORD_ROUND));
	}

	ZStageSetting::ShowStageSettingDialog( pSetting, false);

#ifdef _QUEST
	if ( ZGetGameTypeManager()->IsQuestDerived( pSetting->nGameType))
		ZApplication::GetGameInterface()->GetCombatMenu()->EnableItem( ZCombatMenu::ZCMI_BATTLE_EXIT, false);
	else
		ZApplication::GetGameInterface()->GetCombatMenu()->EnableItem( ZCombatMenu::ZCMI_BATTLE_EXIT, true);
#endif
}

void ZStageInterface::ChangeStageButtons( bool bForcedEntry, bool bMaster, bool bReady)
{
	if ( bForcedEntry)
	{
		ZApplication::GetGameInterface()->ShowWidget( "GameStart", false);
		ZApplication::GetGameInterface()->ShowWidget( "StageReady", false);

		ZApplication::GetGameInterface()->ShowWidget( "ForcedEntryToGame", true);
		ZApplication::GetGameInterface()->ShowWidget( "ForcedEntryToGame2", true);

		ChangeStageEnableReady( false);
	}
	else
	{
		ZApplication::GetGameInterface()->ShowWidget( "ForcedEntryToGame", false);
		ZApplication::GetGameInterface()->ShowWidget( "ForcedEntryToGame2", false);

		ZApplication::GetGameInterface()->ShowWidget( "GameStart", bMaster);
		ZApplication::GetGameInterface()->ShowWidget( "StageReady", !bMaster);

		if ( bMaster)
			ChangeStageEnableReady( false);
		else
			ChangeStageEnableReady( bReady);
	}
}

void ZStageInterface::ChangeStageEnableReady( bool bReady)
{
	ZIDLResource* pResource = ZApplication::GetGameInterface()->GetIDLResource();

	ZApplication::GetGameInterface()->EnableWidget( "GameStart", !bReady);

	ZApplication::GetGameInterface()->EnableWidget( "StageTeamBlue",  !bReady);
	ZApplication::GetGameInterface()->EnableWidget( "StageTeamBlue2", !bReady);
	ZApplication::GetGameInterface()->EnableWidget( "StageTeamRed",  !bReady);
	ZApplication::GetGameInterface()->EnableWidget( "StageTeamRed2", !bReady);
	ZApplication::GetGameInterface()->EnableWidget( "Lobby_StageExit", !bReady);

	if ( (m_nGameType == MMATCH_GAMETYPE_SURVIVAL) || (m_nGameType == MMATCH_GAMETYPE_QUEST))
	{
		ZApplication::GetGameInterface()->EnableWidget( "Stage_SacrificeItemListbox", !bReady);
		ZApplication::GetGameInterface()->EnableWidget( "Stage_PutSacrificeItem",     !bReady);
		ZApplication::GetGameInterface()->EnableWidget( "Stage_SacrificeItemButton0", !bReady);
		ZApplication::GetGameInterface()->EnableWidget( "Stage_SacrificeItemButton1", !bReady);
		if ( ZGetGameClient()->AmIStageMaster())
		{
			ZApplication::GetGameInterface()->EnableWidget( "MapSelection", !bReady);
			ZApplication::GetGameInterface()->EnableWidget( "StageType", !bReady);
		}
		ZApplication::GetGameInterface()->EnableWidget( "StageSettingCaller", false);
	}
	else
	{
		if ( ZGetGameClient()->AmIStageMaster())
		{
			ZApplication::GetGameInterface()->EnableWidget( "MapSelection", !bReady);
			ZApplication::GetGameInterface()->EnableWidget( "StageType", !bReady);
			ZApplication::GetGameInterface()->EnableWidget( "StageMaxPlayer", !bReady);
			ZApplication::GetGameInterface()->EnableWidget( "StageRoundCount", !bReady);
			ZApplication::GetGameInterface()->EnableWidget( "StageSettingCaller", !bReady);
		}
		else
		{
			ZApplication::GetGameInterface()->EnableWidget( "MapSelection", false);
			ZApplication::GetGameInterface()->EnableWidget( "StageType", false);
			ZApplication::GetGameInterface()->EnableWidget( "StageMaxPlayer", false);
			ZApplication::GetGameInterface()->EnableWidget( "StageRoundCount", false);
			ZApplication::GetGameInterface()->EnableWidget( "StageSettingCaller", false);
		}
	}
    
	BEGIN_WIDGETLIST( "Stage_OptionFrame", pResource, MButton*, pButton);
	pButton->Enable( !bReady);
	END_WIDGETLIST();

	BEGIN_WIDGETLIST( "StageEquipmentCaller", pResource, MButton*, pButton);
	pButton->Enable( !bReady);
	END_WIDGETLIST();
}

void ZStageInterface::SetMapName(const char* szMapName)
{
	if (auto MapCombo = ZFindWidgetAs<MComboBox>("MapSelection"))
	{
		MapCombo->SetText(szMapName);
	}
}

void ZStageInterface::OpenSacrificeItemBox()
{
	ZIDLResource* pResource = ZApplication::GetGameInterface()->GetIDLResource();

	MButton* pButton = (MButton*)pResource->FindWidget( "Stage_SacrificeItemBoxOpen");
	if ( pButton)
		pButton->Show( false);
	pButton = (MButton*)pResource->FindWidget( "Stage_SacrificeItemBoxClose");
	if ( pButton)
		pButton->Show( true);

	m_nStateSacrificeItemBox = 2;
	GetSacrificeItemBoxPos();
}

void ZStageInterface::CloseSacrificeItemBox()
{
	ZIDLResource* pResource = ZApplication::GetGameInterface()->GetIDLResource();

	MButton* pButton = (MButton*)pResource->FindWidget( "Stage_SacrificeItemBoxClose");
	if ( pButton)
		pButton->Show( false);
	pButton = (MButton*)pResource->FindWidget( "Stage_SacrificeItemBoxOpen");
	if ( pButton)
		pButton->Show( true);

	MWidget* pWidget = pResource->FindWidget( "Stage_CharacterInfo");
	if ( pWidget)
		pWidget->Enable( true);

	m_nStateSacrificeItemBox = 1;
	GetSacrificeItemBoxPos();
}

void ZStageInterface::HideSacrificeItemBox()
{
	ZIDLResource* pResource = ZApplication::GetGameInterface()->GetIDLResource();

	MButton* pButton = (MButton*)pResource->FindWidget( "Stage_SacrificeItemBoxClose");
	if ( pButton)
		pButton->Show( false);
	pButton = (MButton*)pResource->FindWidget( "Stage_SacrificeItemBoxOpen");
	if ( pButton)
		pButton->Show( true);

	MWidget* pWidget = pResource->FindWidget( "Stage_CharacterInfo");
	if ( pWidget)
		pWidget->Enable( true);

	m_nStateSacrificeItemBox = 0;
	GetSacrificeItemBoxPos();
}

void ZStageInterface::GetSacrificeItemBoxPos()
{
	MWidget* pWidget = ZApplication::GetGameInterface()->GetIDLResource()->FindWidget( "Stage_ItemListView");
	if ( pWidget)
	{
		MRECT rect;

		switch ( m_nStateSacrificeItemBox)
		{
			case 0 :		// Hide
				rect = pWidget->GetRect();
				m_nListFramePos = -rect.w;
				break;

			case 1 :		// Close
				rect = pWidget->GetRect();
				m_nListFramePos = -rect.w + ( rect.w * 0.14);
				break;

			case 2 :		// Open
				m_nListFramePos = 0;
				break;
		}
	}
}

void ZStageInterface::OnSacrificeItem0()
{
}

void ZStageInterface::OnSacrificeItem1()
{
}

void ZStageInterface::UpdateSacrificeItem()
{
	ZIDLResource* pResource = ZApplication::GetGameInterface()->GetIDLResource();

	for ( int i = SACRIFICEITEM_SLOT0;  i <= SACRIFICEITEM_SLOT1;  i++)
	{
		char szWidgetNameItem[ 128];
		sprintf_safe( szWidgetNameItem, "Stage_SacrificeItemImage%d", i);
		MPicture* pPicture = (MPicture*)pResource->FindWidget( szWidgetNameItem);
		if ( pPicture)
		{
			if ( m_SacrificeItem[ i].IsExist())
			{
				pPicture->SetBitmap( m_SacrificeItem[ i].GetIconBitmap());
				char szMsg[ 128];
				MMatchObjCache* pObjCache = ZGetGameClient()->FindObjCache( m_SacrificeItem[ i].GetUID());
				if ( pObjCache)
					sprintf_safe( szMsg, "%s (%s)", m_SacrificeItem[ i].GetName(), pObjCache->GetName());
				else
					strcpy_safe( szMsg, m_SacrificeItem[ i].GetName());
				pPicture->AttachToolTip( szMsg);
			}
			else
			{
				pPicture->SetBitmap( NULL);
				pPicture->DetachToolTip();
			}
		}
	}
}

void ZStageInterface::SerializeSacrificeItemListBox()
{
	ZIDLResource* pResource = ZApplication::GetGameInterface()->GetIDLResource();

	MListBox* pListBox = (MListBox*)pResource->FindWidget( "Stage_SacrificeItemListbox");
	if ( !pListBox)
		return;

	int nStartIndex  = pListBox->GetStartItem();
	int nSelectIndex = pListBox->GetSelIndex();
	pListBox->RemoveAll();

	for ( MQUESTITEMNODEMAP::iterator questitem_itor = ZGetMyInfo()->GetItemList()->GetQuestItemMap().begin();
		  questitem_itor != ZGetMyInfo()->GetItemList()->GetQuestItemMap().end();
		  questitem_itor++)
	{
		ZMyQuestItemNode* pItemNode = (*questitem_itor).second;
		MQuestItemDesc* pItemDesc = GetQuestItemDescMgr().FindQItemDesc( pItemNode->GetItemID());
		if ( pItemDesc)
		{
			int nCount = pItemNode->m_nCount;
			if ( m_SacrificeItem[ SACRIFICEITEM_SLOT0].IsExist() &&
				 (m_SacrificeItem[ SACRIFICEITEM_SLOT0].GetUID() == ZGetGameClient()->GetPlayerUID()) &&
				 (pItemDesc->m_nItemID == m_SacrificeItem[ SACRIFICEITEM_SLOT0].GetItemID()))
				nCount--;
			if ( m_SacrificeItem[ SACRIFICEITEM_SLOT1].IsExist() &&
				 (m_SacrificeItem[ SACRIFICEITEM_SLOT1].GetUID() == ZGetGameClient()->GetPlayerUID()) &&
				 (pItemDesc->m_nItemID == m_SacrificeItem[ SACRIFICEITEM_SLOT1].GetItemID()))
				nCount--;

			if ( pItemDesc->m_bSecrifice && (nCount > 0))
			{
				pListBox->Add( new SacrificeItemListBoxItem( pItemDesc->m_nItemID,
															 ZApplication::GetGameInterface()->GetQuestItemIcon( pItemDesc->m_nItemID, true),
															 pItemDesc->m_szQuestItemName,
															 nCount,
															 pItemDesc->m_szDesc));
			}
		}
	}

	MWidget* pWidget = pResource->FindWidget( "Stage_NoItemLabel");
	if ( pWidget)
	{
		if ( pListBox->GetCount() > 0)
			pWidget->Show( false);
		else
			pWidget->Show( true);
	}

	pListBox->SetStartItem( nStartIndex);
	pListBox->SetSelIndex( min( (pListBox->GetCount() - 1), nSelectIndex));
}

void ZStageInterface::OnDropSacrificeItem( int nSlotNum)
{
	ZIDLResource* pResource = ZApplication::GetGameInterface()->GetIDLResource();

	MListBox* pListBox = (MListBox*)pResource->FindWidget( "Stage_SacrificeItemListbox");
	if ( !pListBox || (pListBox->GetSelIndex() < 0))
		return;

	SacrificeItemListBoxItem* pItemDesc = (SacrificeItemListBoxItem*)pListBox->Get( pListBox->GetSelIndex());
	if ( pItemDesc)
	{
		MTextArea* pDesc = (MTextArea*)pResource->FindWidget( "Stage_ItemDesc");

		if ( ! m_SacrificeItem[ nSlotNum].IsExist())
		{
			ZPostRequestDropSacrificeItem( ZGetGameClient()->GetPlayerUID(), nSlotNum, pItemDesc->GetItemID());

			if ( pDesc)
				pDesc->Clear();
		}

		else
		{
			if ( (m_SacrificeItem[ nSlotNum].GetUID()    != ZGetGameClient()->GetPlayerUID()) ||
				 (m_SacrificeItem[ nSlotNum].GetItemID() != pItemDesc->GetItemID()))
				ZPostRequestDropSacrificeItem( ZGetGameClient()->GetPlayerUID(), nSlotNum, pItemDesc->GetItemID());

			if ( pDesc)
				pDesc->Clear();
		}
	}
}

void ZStageInterface::OnRemoveSacrificeItem( int nSlotNum)
{
	if ( !m_SacrificeItem[ nSlotNum].IsExist())
		return;

	ZPostRequestCallbackSacrificeItem( ZGetGameClient()->GetPlayerUID(),
									   nSlotNum,
									   m_SacrificeItem[ nSlotNum].GetItemID());

	MTextArea* pDesc = (MTextArea*)ZApplication::GetGameInterface()->GetIDLResource()->FindWidget( "Stage_ItemDesc");
	if ( pDesc)
		pDesc->Clear();
}

class MSacrificeItemListBoxListener : public MListener
{
public:
	virtual bool OnCommand(MWidget* pWidget, const char* szMessage)
	{
		// On select
		if ( MWidget::IsMsg( szMessage, MLB_ITEM_SEL) == true)
		{
			MListBox* pListBox = (MListBox*)pWidget;

			SacrificeItemListBoxItem* pItemDesc = (SacrificeItemListBoxItem*)pListBox->GetSelItem();
			MTextArea* pDesc = (MTextArea*)ZApplication::GetGameInterface()->GetIDLResource()->FindWidget( "Stage_ItemDesc");
			if ( pItemDesc && pDesc)
			{
				char szCount[ 128];
				sprintf_safe( szCount, "%s : %d", ZMsg( MSG_WORD_QUANTITY), pItemDesc->GetItemCount());
				pDesc->SetTextColor( MCOLOR( 0xFFD0D0D0));
				pDesc->SetText( szCount);
				pDesc->AddText( "\n");
				pDesc->AddText( pItemDesc->GetItemDesc(), 0xFF808080);
			}

			return true;
		}


		// On double click
		else if ( MWidget::IsMsg( szMessage, MLB_ITEM_DBLCLK) == true)
		{
			// Put item
			if ( !ZApplication::GetStageInterface()->m_SacrificeItem[ 0].IsExist())
				ZApplication::GetStageInterface()->OnDropSacrificeItem( 0);
			else if ( !ZApplication::GetStageInterface()->m_SacrificeItem[ 1].IsExist())
				ZApplication::GetStageInterface()->OnDropSacrificeItem( 1);
		
			return true;
		}

		return false;
	}
};

MSacrificeItemListBoxListener g_SacrificeItemListBoxListener;

MListener* ZGetSacrificeItemListBoxListener()
{
	return &g_SacrificeItemListBoxListener;
}

void OnDropCallbackRemoveSacrificeItem( void* pSelf, MWidget* pSender, MBitmap* pBitmap,
	const char* szString, const char* szItemString)
{
	if ( (pSender == NULL) || (strcmp(pSender->GetClassName(), MINT_ITEMSLOTVIEW)))
		return;

	ZItemSlotView* pItemSlotView = (ZItemSlotView*)pSender;
	ZApplication::GetStageInterface()->OnRemoveSacrificeItem( (strcmp( pItemSlotView->m_szItemSlotPlace, "SACRIFICE0") == 0) ? 0 : 1);
}

void ZStageInterface::StartMovieOfQuest()
{
	ZIDLResource* pResource = ZApplication::GetGameInterface()->GetIDLResource();

	m_dwClockOfStartMovie = GetGlobalTimeMS();

	MAnimation* pAnimation = (MAnimation*)pResource->FindWidget( "Stage_Flame0");
	if ( pAnimation && m_SacrificeItem[ SACRIFICEITEM_SLOT0].IsExist())
	{
		pAnimation->SetCurrentFrame( 0);
		pAnimation->Show( true);
		pAnimation->SetRunAnimation( true);
	}
	pAnimation = (MAnimation*)pResource->FindWidget( "Stage_Flame1");
	if ( pAnimation && m_SacrificeItem[ SACRIFICEITEM_SLOT1].IsExist())
	{
		pAnimation->SetCurrentFrame( 0);
		pAnimation->Show( true);
		pAnimation->SetRunAnimation( true);
	}

	m_bDrawStartMovieOfQuest = true;
}

void ZStageInterface::OnDrawStartMovieOfQuest()
{
	if ( !m_bDrawStartMovieOfQuest)
		return ;

	ZIDLResource* pResource = ZApplication::GetGameInterface()->GetIDLResource();

	DWORD dwClock = GetGlobalTimeMS() - m_dwClockOfStartMovie;

	int nOpacity = 255 - dwClock * 0.12f;
	if ( nOpacity < 0)
		nOpacity = 0;

	MPicture* pPicture = (MPicture*)pResource->FindWidget( "Stage_SacrificeItemImage0");
	if ( pPicture && m_SacrificeItem[ SACRIFICEITEM_SLOT0].IsExist())
		pPicture->SetOpacity( nOpacity);

	pPicture = (MPicture*)pResource->FindWidget( "Stage_SacrificeItemImage1");
	if ( pPicture && m_SacrificeItem[ SACRIFICEITEM_SLOT1].IsExist())
		pPicture->SetOpacity( nOpacity);

	if ( dwClock > 3200)
	{
		m_bDrawStartMovieOfQuest = false;

		ZMyQuestItemMap::iterator itMyQItem;

		if( ZGetGameClient()->GetUID() == m_SacrificeItem[ SACRIFICEITEM_SLOT0].GetUID() )
		{
			itMyQItem = ZGetMyInfo()->GetItemList()->GetQuestItemMap().find( m_SacrificeItem[ SACRIFICEITEM_SLOT0].GetItemID() );
			itMyQItem->second->Decrease();
		}
		if( ZGetGameClient()->GetUID() == m_SacrificeItem[ SACRIFICEITEM_SLOT1].GetUID() )
		{
			itMyQItem = ZGetMyInfo()->GetItemList()->GetQuestItemMap().find( m_SacrificeItem[ SACRIFICEITEM_SLOT1].GetItemID() );
			itMyQItem->second->Decrease();
		}		
		
		m_SacrificeItem[ SACRIFICEITEM_SLOT0].RemoveItem();
		m_SacrificeItem[ SACRIFICEITEM_SLOT1].RemoveItem();

		ZApplication::GetGameInterface()->SetState( GUNZ_GAME);
	}
}

bool ZStageInterface::IsShowStartMovieOfQuest()
{
	ZIDLResource* pResource = ZApplication::GetGameInterface()->GetIDLResource();

	if ( m_nGameType == MMATCH_GAMETYPE_QUEST)
	{
		if ( m_SacrificeItem[ SACRIFICEITEM_SLOT0].IsExist() || m_SacrificeItem[ SACRIFICEITEM_SLOT1].IsExist())
			return true;
	}

	return false;
}
bool ZStageInterface::OnResponseDropSacrificeItemOnSlot( const int nResult, const MUID& uidRequester,
	int nSlotIndex, int nItemID )
{
#ifdef _QUEST_ITEM
	if( MOK == nResult)
	{
		MQuestItemDesc* pItemDesc = GetQuestItemDescMgr().FindQItemDesc( nItemID);
		MBitmap* pIconBitmap = ZApplication::GetGameInterface()->GetQuestItemIcon( nItemID, false);

		m_SacrificeItem[ nSlotIndex].SetSacrificeItemSlot( uidRequester, nItemID, pIconBitmap, pItemDesc->m_szQuestItemName, pItemDesc->m_nLevel);
		SerializeSacrificeItemListBox();

		UpdateSacrificeItem();
	}
	else if( ITEM_TYPE_NOT_SACRIFICE == nResult)
	{
		return false;
	}
	else if( NEED_MORE_QUEST_ITEM == nResult )
	{
	}
	else if( MOK != nResult )
	{
		return false;
	}
	else
	{
		ASSERT( 0 );
	}

#endif

	return true;
}

bool ZStageInterface::OnResponseCallbackSacrificeItem( const int nResult, const MUID& uidRequester, const int nSlotIndex, const int nItemID )
{
#ifdef _QUEST_ITEM
	if( MOK == nResult )
	{
		m_SacrificeItem[ nSlotIndex].RemoveItem();
		SerializeSacrificeItemListBox();

		UpdateSacrificeItem();

		MTextArea* pDesc = (MTextArea*)ZApplication::GetGameInterface()->GetIDLResource()->FindWidget( "Stage_ItemDesc");
		if ( pDesc)
			pDesc->Clear();
	}
	else if( ERR_SACRIFICE_ITEM_INFO == nResult )
	{
	}

#endif

	return true;
}

#ifdef _QUEST_ITEM
bool ZStageInterface::OnResponseQL( const int nQL )
{
	ZGetQuest()->GetGameInfo()->SetQuestLevel( nQL);

	MLabel* pLabel = (MLabel*)ZApplication::GetGameInterface()->GetIDLResource()->FindWidget( "Stage_QuestLevel");
	if ( pLabel)
	{
		char szText[125];
		sprintf_safe( szText, "%s %s : %d", ZMsg( MSG_WORD_QUEST), ZMsg( MSG_CHARINFO_LEVEL), nQL);
		pLabel->SetText( szText);
	}

	return true;
}

bool ZStageInterface::OnStageGameInfo( const int nQL, const int nMapsetID, const unsigned int nScenarioID )
{
	if (nScenarioID != 0)
	{
		ZGetQuest()->GetGameInfo()->SetQuestLevel( nQL );
	}
	else
	{
		ZGetQuest()->GetGameInfo()->SetQuestLevel( 0 );
	}

	UpdateStageGameInfo(nQL, nMapsetID, nScenarioID);

	return true;
}

bool ZStageInterface::OnResponseSacrificeSlotInfo( const MUID& uidOwner1, const u32 nItemID1, 
												   const MUID& uidOwner2, const u32 nItemID2 )
{
	if ( (uidOwner1 != MUID(0,0)) && nItemID1)
	{
		MQuestItemDesc* pItemDesc = GetQuestItemDescMgr().FindQItemDesc( nItemID1);
		MBitmap* pIconBitmap = ZApplication::GetGameInterface()->GetQuestItemIcon( nItemID1, false);
		m_SacrificeItem[ SACRIFICEITEM_SLOT0].SetSacrificeItemSlot( uidOwner1, nItemID1, pIconBitmap, pItemDesc->m_szQuestItemName, pItemDesc->m_nLevel);
	}
	else
		m_SacrificeItem[ SACRIFICEITEM_SLOT0].RemoveItem();

	if ( (uidOwner2 != MUID(0,0)) && nItemID2)
	{
		MQuestItemDesc* pItemDesc = GetQuestItemDescMgr().FindQItemDesc( nItemID2);
		MBitmap* pIconBitmap = ZApplication::GetGameInterface()->GetQuestItemIcon( nItemID2, false);
		m_SacrificeItem[ SACRIFICEITEM_SLOT1].SetSacrificeItemSlot( uidOwner2, nItemID2, pIconBitmap, pItemDesc->m_szQuestItemName, pItemDesc->m_nLevel);
	}
	else
		m_SacrificeItem[ SACRIFICEITEM_SLOT1].RemoveItem();

	UpdateSacrificeItem();

	MTextArea* pDesc = (MTextArea*)ZApplication::GetGameInterface()->GetIDLResource()->FindWidget( "Stage_ItemDesc");
	if ( pDesc)
		pDesc->Clear();

	return true;
}


bool ZStageInterface::OnQuestStartFailed( const int nState )
{
	MTextArea* pTextArea = (MTextArea*)ZApplication::GetGameInterface()->GetIDLResource()->FindWidget( "StageChattingOutput");
	if ( pTextArea)
	{
		char text[256];
		sprintf_safe(text, "^1%s", ZMsg(MSG_GANE_NO_QUEST_SCENARIO));
		pTextArea->AddText( text);
	}

	return true;
}


bool ZStageInterface::OnNotAllReady()
{
	return true;
}
#endif

void ZStageInterface::UpdateStageGameInfo(const int nQL, const int nMapsetID, const int nScenarioID)
{
	if (!IsQuestDerivedGameType())
		return;

	if (auto pLabel = ZFindWidgetAs<MLabel>("Stage_QuestLevel"))
	{
		char szText[125];
		sprintf_safe(szText, "%s %s : %d", ZMsg(MSG_WORD_QUEST), ZMsg(MSG_CHARINFO_LEVEL), nQL);
		pLabel->SetText(szText);
	}

	static const MCOLOR MapSetNormal  = 0xFFFFFFFF;
	static const MCOLOR MapSetSpecial = 0xFFFFFF40;

	auto pLabel = ZFindWidgetAs<MLabel>("Stage_SenarioName");
	if (!pLabel)
		return;

	auto pWidget = ZFindWidget("Stage_SenarioNameImg");
	auto pPictureL = ZFindWidgetAs<MPicture>("Stage_Lights0");
	auto pPictureR = ZFindWidgetAs<MPicture>("Stage_Lights1");

	if (nScenarioID == 0)
	{
		pLabel->SetText("");
		MWidget* Widgets[] = {pWidget, pPictureL, pPictureR};
		for (auto Widget : Widgets)
			Widget->Show(false);
		return;
	}

	pLabel->SetAlignment( MAM_HCENTER | MAM_VCENTER);

	const char* Text = "";
	MCOLOR TextColor = 0xFFFFFFFF;
	MCOLOR BitmapColor = MapSetNormal;
	bool ShowScenarioNameImage = false;
	if (auto Scenario = GetScenarioManager().GetSpecialScenario(nScenarioID))
	{
		Text = Scenario->Title;
		TextColor = 0xFFFFFF00;
		BitmapColor = MapSetSpecial;
		ShowScenarioNameImage = true;
	}

	pLabel->SetText(Text);
	pLabel->SetTextColor(TextColor);
	if (pWidget)
		pWidget->Show(ShowScenarioNameImage);

	for (auto Picture : {pPictureL, pPictureR})
	{
		Picture->Show(true);
		Picture->SetBitmapColor(BitmapColor);
	}
}

void SacrificeItemSlotDesc::SetSacrificeItemSlot( const MUID& uidUserID, const u32 nItemID, MBitmap* pBitmap, const char* szItemName, const int nQL)
{
	m_uidUserID = uidUserID;
	m_nItemID = nItemID;
	m_pIconBitmap = pBitmap;
	strcpy_safe( m_szItemName, szItemName);
	m_nQL = nQL;
	m_bExist = true;
}

template <typename V>
static auto SearchSpecialScenarios(V& v, int ID)
{
	return std::lower_bound(v.begin(), v.end(), ID, [](const SpecialScenario& a, int b) {
		return a.ID < b;
	});
}

bool ScenarioManager::Load()
{
	if (Loaded)
		return true;
	Loaded = true;

	MXmlDocument xmlQuestItemDesc;
	if (!xmlQuestItemDesc.LoadFromFile("System/scenario.xml", ZApplication::GetFileSystem()))
	{
		return false;
	}

	for (auto&& chrElement : xmlQuestItemDesc.GetDocumentElement().Children())
	{
		auto TagName = chrElement.GetTagName();
		if (starts_with(TagName, "#"))
			continue;

		bool IsSpecialScenario = iequals(TagName, "SPECIAL_SCENARIO");
		if (!IsSpecialScenario && !iequals(TagName, "STANDARD_SCENARIO"))
		{
			mlog("Invalid scenario type %.*s\n", TagName.size(), TagName.data());
			return false;
		}

		auto MissingAttribute = [](const char* Name) {
			mlog("Scenario entry missing attribute %s\n", Name);
			return false;
		};

		auto MapSet = chrElement.GetAttribute("mapset");
		if (!MapSet)
			return MissingAttribute("mapset");

		if (IsSpecialScenario)
		{
			auto Title = chrElement.GetAttribute("title");
			if (!Title)
				return MissingAttribute("title");

			auto IDAttr = chrElement.GetAttribute("id");
			if (!IDAttr)
				return MissingAttribute("id");
			auto ID = StringToInt(*IDAttr);
			if (!ID)
			{
				mlog("Malformed special scenario ID. Expected integral value, got %.*s\n",
						IDAttr->size(), IDAttr->data());
				return false;
			}

			auto& Scenario = *SpecialScenarios.emplace(SearchSpecialScenarios(SpecialScenarios, *ID));
			Scenario.ID = *ID;
			strcpy_safe(Scenario.Title, *Title);
			strcpy_safe(Scenario.MapSet, *MapSet);
		}
		else
		{
			auto AlreadyExists = [&] {
				for (auto&& MapName : MapNames)
					if (iequals(MapName.data(), *MapSet))
						return true;
				return false;
			}();
			if (AlreadyExists)
				continue;
			auto& MapName = emplace_back(MapNames);
			strcpy_safe(MapName, *MapSet);
		}
	}

	MapNames.shrink_to_fit();
	SpecialScenarios.shrink_to_fit();

	return true;
}

const SpecialScenario* ScenarioManager::GetSpecialScenario(int ID) const
{
	auto it = SearchSpecialScenarios(SpecialScenarios, ID);
	if (it == SpecialScenarios.end() || it->ID != ID)
	   return nullptr;
	return &*it;
}

bool ZStageInterface::ReadSenarioNameXML()
{
	return GetScenarioManager().Load();
}

bool ZStageInterface::OnStopVote()
{
	ZGetGameClient()->SetVoteInProgress( false );
	ZGetGameClient()->SetCanVote( false );

#ifdef _DEBUG
	string str = ZMsg(MSG_VOTE_VOTE_STOP);
#endif

	ZChatOutput(ZMsg(MSG_VOTE_VOTE_STOP), ZChat::CMT_SYSTEM, ZChat::CL_CURRENT);
	return true;
}
