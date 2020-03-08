#include "stdafx.h"
#include "ZApplication.h"
#include "MMatchObjCache.h"
#include "ZPlayerListBox.h"
#include "MBitmap.h"
#include "MListBox.h"
#include "ZPost.h"
#include "ZCharacterView.h"
#include "ZPlayerMenu.h"
#include "MToolTip.h"
#include "ZButton.h"
#include "ZMyInfo.h"

#define PLAYER_SLOT_GAP 1
#define PLAYERLIST_ITEM_HEIGHT	23

void ZPlayerListBoxLook::OnItemDraw2(MDrawContext* pDC, MRECT& r, const char* szText, MCOLOR color,
	bool bSelected, bool bFocus, int nAdjustWidth)
{
	if (szText == nullptr)
		return;

	pDC->SetColor(color);

	auto PrevClipRect = pDC->GetClipRect();

	auto NewClipRect = pDC->GetClipRect();
	NewClipRect.x	+= r.x;
	NewClipRect.y	+= r.y;
	NewClipRect.w	 = r.w;
	NewClipRect.h	 = r.h;

	pDC->SetClipRect(NewClipRect);

#ifdef COLORTEXT_SUPPORT
	pDC->TextMultiLine(r, szText,0,false);
#else
	pDC->Text(r.x, r.y+(r.h-pDC->GetFont()->GetHeight())/2, szText);
#endif

	pDC->SetClipRect(PrevClipRect);
}

void ZPlayerListBoxLook::OnItemDraw2(MDrawContext* pDC, MRECT& r, MBitmap* pBitmap, bool bSelected, bool bFocus, int nAdjustWidth)
{
	if (pBitmap == nullptr) return;

	auto PrevClipRect = pDC->GetClipRect();

	auto NewClipRect = pDC->GetClipRect();
	NewClipRect.w -= nAdjustWidth;
	pDC->SetClipRect(NewClipRect);

	pDC->SetBitmap(pBitmap);
	pDC->Draw(r, MRECT(0,0,pBitmap->GetWidth(),pBitmap->GetHeight()));

	pDC->SetClipRect(PrevClipRect);
}

static float GetF(float _old, float _new) {
	return _old/_new;
}
static float GetF(float _new) {
	return _new/800.f;
}

void ZPlayerListBoxLook::OnDraw(MListBox* pListBox, MDrawContext* pDC)
{
	auto* PlayerListBox = static_cast<ZPlayerListBox*>(pListBox);

	PlayerListBox->UpdateList();

	int newW = RGetScreenWidth();
	float fA = GetF(newW);

	m_SelectedPlaneColor = MCOLOR(180,220,180);

	int nItemHeight = 23*fA;
	int nShowCount = 0;

	auto ClientRect = pListBox->GetClientRect();

	int nHeaderHeight = nItemHeight;

	bool bShowClanCreateFrame = PlayerListBox->GetMode() == ZPlayerListBox::PlayerListMode::ChannelClan &&
		!ZGetMyInfo()->IsClanJoined();

	pDC->SetColor(10,10,10);

	auto pm = PlayerListBox->GetMode();

	for (int i = pListBox->GetStartItem(); i < pListBox->GetCount(); i++) {

		MPOINT p;
		p.x = ClientRect.x;
		p.y = ClientRect.y + nHeaderHeight + nItemHeight*nShowCount;

		MListItem* pItem = pListBox->Get(i);
		bool bSelected = pItem->m_bSelected;
		bool bFocused = (pListBox->IsFocus());

		int nFieldStartX = 0;

		// Draw rectangle around selected item.
		if(bSelected) {
			pDC->SetColor(130,130,130);
			pDC->Rectangle(MRECT(p.x + 2, p.y + 5, ClientRect.x + ClientRect.w - 8, nItemHeight - 4));
		}

		for (int j = 0; j < max(pListBox->GetFieldCount(), 1); j++) {

			int nTabSize = ClientRect.w;
			if(j<pListBox->GetFieldCount()) nTabSize = pListBox->GetField(j)->nTabSize;

			int nWidth = min(nTabSize, ClientRect.w - nFieldStartX);
			if (pListBox->m_bAbsoulteTabSpacing == false) nWidth = ClientRect.w*nTabSize / 100;

			int nAdjustWidth = 0;

			if (pListBox->GetScrollBar()->IsVisible()) {
				nAdjustWidth = pListBox->GetScrollBar()->GetRect().w +
					pListBox->GetScrollBar()->GetRect().w / 2;
			}

			MRECT ir(p.x+nFieldStartX+5, p.y+7, nWidth-7, nItemHeight-7);
			MRECT irt(p.x+nFieldStartX, p.y+5, nWidth, nItemHeight);

			const char* szText = pItem->GetString(j);
			MBitmap* pBitmap = pItem->GetBitmap(j);
			MCOLOR color = pItem->GetColor();

			if (pBitmap != NULL)
				OnItemDraw2(pDC, ir, pBitmap,  bSelected, bFocused, nAdjustWidth);

			if (szText != NULL)
				OnItemDraw2(pDC, irt, szText, color, bSelected, bFocused, nAdjustWidth);

			nFieldStartX += nWidth;
			if (nFieldStartX >= ClientRect.w) break;
		}

		nShowCount++;

		if (nShowCount >= pListBox->GetShowItemCount()) break;
	}
}

IMPLEMENT_LOOK(ZPlayerListBox, ZPlayerListBoxLook)

ZPlayerListBox::ZPlayerListBox(const char* szName, MWidget* pParent, MListener* pListener)
	: MListBox(szName, pParent, pListener)
{
	SetVisibleHeader(false);

	m_bAbsoulteTabSpacing = true;

	m_nTotalPlayerCount = 0;
	m_nPage = 0;

	m_bVisible = true;
	m_bVisibleHeader = true;

	SetItemHeight(PLAYERLIST_ITEM_HEIGHT);
	
	mSelectedPlayer = 0;
	mStartToDisplay = 0;
	m_bAlwaysVisibleScrollbar = false;
	m_bHideScrollBar = true;
	m_pScrollBar->SetVisible(false);
	m_pScrollBar->Enable(false);

	m_pScrollBar->m_nDebugType = 3;
	m_nDebugType = 2;

	m_nOldW = RGetScreenWidth();

	SetListener(this);

	m_pButton = std::make_unique<ZBmButton>(nullptr, this, this);
	m_pButton->SetStretch(true);

	m_nMode = PlayerListMode::Channel;
	InitUI(m_nMode);
}

ZPlayerListBox::~ZPlayerListBox() = default;

bool ZPlayerListBox::LegalState(const char * szName, ModeCategory ExpectedCategory)
{
	if (szName[0] == 0)
	{
		assert(!"szName may not be empty");
		MLog("ZPlayerListBox::AddPlayer* -- Called with null name\n");
		return false;
	}

	auto ModeToCategory = [](auto PlayerListMode)
	{
		switch (PlayerListMode)
		{
		case PlayerListMode::Channel:
			return ModeCategory::Channel;
		case PlayerListMode::Stage:
			return ModeCategory::Stage;
		case PlayerListMode::ChannelFriend:
		case PlayerListMode::StageFriend:
			return ModeCategory::Friend;
		case PlayerListMode::ChannelClan:
		case PlayerListMode::StageClan:
			return ModeCategory::Clan;
		default:
			assert(!"Unknown mode");
			return ModeCategory::Unknown;
		};
	};

	auto ActualCategory = ModeToCategory(m_nMode);

	if (ActualCategory != ExpectedCategory)
	{
		assert(!"Wrong mode");
		MLog("ZPlayerListBox::AddPlayer* -- Called while in wrong category %d, only legal in %d\n",
			ActualCategory, ExpectedCategory);
		return false;
	}

	return true;
}

void ZPlayerListBox::SetupButton(const char *szOn, const char *szOff)
{
	m_pButton->SetUpBitmap(MBitmapManager::Get(szOff));
	m_pButton->SetDownBitmap(MBitmapManager::Get(szOn));
	m_pButton->SetOverBitmap(MBitmapManager::Get(szOff));
}

void ZPlayerListBox::InitUI(PlayerListMode nMode)
{
	int newW = RGetScreenWidth();
	float fA = GetF(newW);

	m_nMode = nMode;
	assert(static_cast<int>(nMode) >= 0 && nMode < PlayerListMode::End);

	RemoveAllField();

	const int nFields[static_cast<size_t>(PlayerListMode::End)] = {
		5, 5, // Channel, Stage
		2, 2, // ChannelFriend, StageFriend
		4, 4, // ChannelClan, StageClan
	};
	for (int i = 0; i < nFields[static_cast<size_t>(nMode)]; i++) {
		AddField("", 10);
	}
	OnSize(0, 0);

	auto GetModeString = [](auto Mode)
	{
		switch (Mode)
		{
		case PlayerListMode::Channel:
		default:
			return "lobby";
		case PlayerListMode::Stage:
			return "game";
		case PlayerListMode::ChannelFriend:
		case PlayerListMode::StageFriend:
			return "friends";
		case PlayerListMode::ChannelClan:
		case PlayerListMode::StageClan:
			return "clan";
		}
	};

	bool bShowClanCreateFrame = false;
	{
		auto ModeString = GetModeString(m_nMode);
		char on[256];
		char off[256];
		sprintf_safe(on,  "pltab_%s_on.tga",  ModeString);
		sprintf_safe(off, "pltab_%s_off.tga", ModeString);
		SetupButton(on, off);

		if (!strcmp(ModeString, "clan")) {
			bShowClanCreateFrame = !ZGetMyInfo()->IsClanJoined();
		}
	}

	auto pFrame = ZFindWidget("LobbyPlayerListClanCreateFrame");
	auto pButtonUp = ZFindWidgetAs<MButton>("LobbyChannelPlayerListPrev");
	auto pButtonDn = ZFindWidgetAs<MButton>("LobbyChannelPlayerListNext");
	if(pFrame)
	{
		pFrame->Show(bShowClanCreateFrame);
		pButtonUp->Show(!bShowClanCreateFrame);
		pButtonDn->Show(!bShowClanCreateFrame);
	}
}

void ZPlayerListBox::RefreshUI()
{
	InitUI(GetMode());
}

void ZPlayerListBox::SetMode(PlayerListMode nMode)
{
	auto* pWidget = ZFindWidgetAs<ZPlayerListBox>("LobbyChannelPlayerList");
	if(pWidget) {
		pWidget->RemoveAll();
	}

	mlog("ZPlayerListBox::SetMode %d\n", nMode);

	InitUI(nMode);

	if (!ZGetGameClient() || !ZGetGameClient()->IsConnected()) {
		return;
	}

	switch (nMode)
	{
		case PlayerListMode::Channel:
			ZPostRequestChannelPlayerList(ZGetGameClient()->GetPlayerUID(),
				ZGetGameClient()->GetChannelUID(), pWidget->m_nPage);
			break;
		case PlayerListMode::Stage:
			ZPostRequestStagePlayerList(ZGetGameClient()->GetStageUID());
			break;
		case PlayerListMode::ChannelFriend:
		case PlayerListMode::StageFriend:
			ZPostFriendList();
			break;
		case PlayerListMode::ChannelClan:
		case PlayerListMode::StageClan:
			ZPostRequestClanMemberList(ZGetGameClient()->GetPlayerUID());
			break;
	}
}

void GetRectMul(MRECT* rect,MRECT* org_rect,float f)
{
	rect->x = org_rect->x * f;
	rect->y = org_rect->y * f;
	rect->w = org_rect->w * f;
	rect->h = org_rect->h * f;
}

void ZPlayerListBox::OnSize(int w,int h)
{
	if (m_Fields.GetCount() == 0) return;

	int newW = RGetScreenWidth();
	float fA = GetF(newW);

	m_nItemHeight = PLAYERLIST_ITEM_HEIGHT * fA;

	m_pButton->SetBounds(0, 0, m_Rect.w, (int)(28.0 * fA));

	switch (m_nMode) {
	case PlayerListMode::Channel:
	case PlayerListMode::Stage:
		m_Fields.Get(0)->nTabSize = 23 * fA; //icon
		m_Fields.Get(1)->nTabSize = 16 * fA; //level
		m_Fields.Get(2)->nTabSize = 72 * fA; //name
		m_Fields.Get(3)->nTabSize = 23 * fA; //icon
		m_Fields.Get(4)->nTabSize = 80 * fA; //clan name
		break;
	case PlayerListMode::StageFriend:
	case PlayerListMode::ChannelFriend:
		m_Fields.Get(0)->nTabSize = 23 * fA; //icon
		m_Fields.Get(1)->nTabSize = 72 * fA; //name
		break;
	case PlayerListMode::ChannelClan:
	case PlayerListMode::StageClan:
		m_Fields.Get(0)->nTabSize = 23 * fA; //icon
		m_Fields.Get(1)->nTabSize = 85 * fA; //name
		m_Fields.Get(2)->nTabSize = 23 * fA; //icon
		m_Fields.Get(3)->nTabSize = 90 * fA; //clan grade
		break;
	};
	
	RecalcList();
}

void ZPlayerListBox::AddPlayerChannel(const MUID& puid, ePlayerState state, int nLevel,
	const char* szName, const char *szClanName, unsigned int nClanID, MMatchUserGradeID nGrade )
{
	if (!LegalState(szName, ModeCategory::Channel)) {
		return;
	}

	char szFileName[64] = "";
	char szLevel[64] = "";

	const char* szRefName = nullptr;

	MCOLOR _color;
	char sp_name[256];
	bool bSpUser = GetUserGradeIDColor(nGrade, _color, sp_name);

	sprintf_safe(szLevel, "%2d", nLevel);
	szRefName = szName;

	switch (state) {
		case PS_FIGHT	: strcpy_safe(szFileName, "player_status_player.tga");	break;
		case PS_WAIT	: strcpy_safe(szFileName, "player_status_game.tga");		break;
		case PS_LOBBY	: strcpy_safe(szFileName, "player_status_lobby.tga");	break;
	}

	auto* pItem = new ZLobbyPlayerListItem(puid, MBitmapManager::Get(szFileName), nClanID, szLevel, szRefName, szClanName, state,nGrade );

	if(bSpUser)
		pItem->SetColor(_color);

	MListBox::Add( pItem );
}

// Gets the flag bitmap that is displayed in the gameroom.
static MBitmap* GetStagePlayerBitmap(MMatchObjectStageState State, bool IsMaster, MMatchTeam Team)
{
	{
		const auto* pStageSetting = ZGetGameClient()->GetMatchStageSetting()->GetStageSetting();
		const auto IsTeamGameType = ZGetGameTypeManager()->IsTeamGame(pStageSetting->nGameType);

		// If they're not on the spectator team, and it's not a team gametype
		// they can only be MMT_ALL (since MMT_RED and MMT_BLUE are only for team gametypes).
		if (Team != MMT_SPECTATOR && IsTeamGameType == false) {
			Team = MMT_ALL;
		}
	}

	auto GetTeamString = [](auto Team)
	{
		switch (Team)
		{
		case MMT_RED:
			return "red";
		case MMT_BLUE:
			return "blue";
		case MMT_SPECTATOR:
			return "observer";
		case MMT_ALL:
		default:
			return "normal";
		};
	};

	// The return value includes the underscore if it's not empty.
	auto GetReadyString = [](auto StageState, auto Team)
	{
		// Spectators can't be ready.
		if (Team == MMT_SPECTATOR)
			return "";

		switch (StageState)
		{
		case MOSS_READY:
			return "_ready";
		case MOSS_EQUIPMENT:
		case MOSS_SHOP:
			return "_equip";
		case MOSS_NONREADY:
		default:
			return "";
		};
	};

	auto PlayerStatusString = IsMaster ? "master" : "member";
	auto TeamString = GetTeamString(Team);
	auto ReadyString = GetReadyString(State, Team);

	char BitmapFilename[512];
	// ReadyString includes the underscore if it's not empty.
	sprintf_safe(BitmapFilename, "stg_status_%s_%s%s.tga",
		PlayerStatusString, TeamString, ReadyString);

	return MBitmapManager::Get(BitmapFilename);
}

// Updates the checkmarks in the buttons that select your team.
static void UpdateTeamButtonChecks(MMatchTeam Team)
{
	auto SetButtonCheck = [](auto&& Name, bool Value) {
		auto* pButton = ZFindWidgetAs<MButton>(Name);
		if (pButton) {
			pButton->SetCheck(Value);
		}
	};

	SetButtonCheck("StageTeamBlue", Team == MMT_BLUE);
	SetButtonCheck("StageTeamRed", Team == MMT_RED);
}

void ZPlayerListBox::AddPlayerStage(const MUID& puid, MMatchObjectStageState state, int nLevel,
	const char* szName, const char* szClanName, unsigned int nClanID,
	bool isMaster, MMatchTeam nTeam)
{
	if (!LegalState(szName, ModeCategory::Stage)) {
		return;
	}

	MCOLOR Color;
	char sp_name[256];
	MMatchUserGradeID gid = MMUG_FREE;
	bool IsSpecialUser = GetUserInfoUID(puid, Color, sp_name, gid);
	const char* RefName = szName;

	char Level[256];
	sprintf_safe(Level, "%2d", nLevel);

	auto Bitmap = GetStagePlayerBitmap(state, isMaster, nTeam);
	auto* pItem = new ZStagePlayerListItem(puid, Bitmap, nClanID, RefName, szClanName, Level, gid);

	if (IsSpecialUser)
		pItem->SetColor(Color);

	MListBox::Add(pItem);

	if (ZGetMyUID() == puid) {
		UpdateTeamButtonChecks(nTeam);
	}
}

// Gets the bitmap displayed in the lobby.
static const char* GetPlayerStateBitmapFilename(ePlayerState State)
{
	switch (State)
	{
	case PS_FIGHT: return "player_status_player.tga";
	case PS_WAIT:  return "player_status_game.tga";
	case PS_LOBBY:
	default:       return "player_status_lobby.tga";
	}
}

static MBitmap* GetPlayerStateBitmap(ePlayerState State) {
	return MBitmapManager::Get(GetPlayerStateBitmapFilename(State));
}

void ZPlayerListBox::AddPlayerFriend(ePlayerState state, char* szName, char* szLocation)
{
	if (!LegalState(szName, ModeCategory::Friend)) {
		return;
	}

	auto* NewItem = new ZFriendPlayerListItem(MUID(0, 0), GetPlayerStateBitmap(state),
		szName, nullptr, szLocation, state, MMUG_FREE);
	MListBox::Add(NewItem);
}

void ZPlayerListBox::AddPlayerClan(const MUID& puid, ePlayerState state, char* szName, int nLevel,
	MMatchClanGrade nGrade )
{
	if (!LegalState(szName, ModeCategory::Clan)) {
		return;
	}

	auto GetClanGradeName = [](auto ClanGrade)
	{
		switch (ClanGrade)
		{
		case MCG_MASTER: return ZMsg(MSG_WORD_CLAN_MASTER);
		case MCG_ADMIN: return ZMsg(MSG_WORD_CLAN_ADMIN);
		default: return ZMsg(MSG_WORD_CLAN_MEMBER);
		}
	};

	auto IsSpecialClanGrade = [&](auto ClanGrade) {
		return ClanGrade == MCG_MASTER || ClanGrade == MCG_ADMIN;
	};

	auto* Bitmap = GetPlayerStateBitmap(state);
	auto* pItem = new ZClanPlayerListItem(puid, Bitmap,
		szName, nullptr, GetClanGradeName(nGrade),
		state, nGrade);

	MCOLOR Color{ 240, 64, 64 };
	if (IsSpecialClanGrade(nGrade)) {
		pItem->SetColor(Color);
	}

	MListBox::Add(pItem);
}

void ZPlayerListBox::DelPlayer(const MUID& puid)
{
	for (int i = 0; i < GetCount(); i++) {
		auto pItem = static_cast<ZPlayerListItem*>(Get(i));
		if (pItem->m_PlayerUID == puid) {
			Remove(i);
			return;
		}
	}
}

ZPlayerListItem* ZPlayerListBox::GetUID(MUID uid)
{
	ZPlayerListItem* pItem = NULL;

	for(int i=0;i<GetCount();i++) {
		auto pItem = static_cast<ZPlayerListItem*>(Get(i));
		if (pItem->m_PlayerUID == uid)
			return pItem;
	}
	return NULL;
}


const char* ZPlayerListBox::GetPlayerName( int nIndex)
{
	ZPlayerListItem* pItem = (ZPlayerListItem*)Get( nIndex);

	if ( !pItem)
		return NULL;

	return pItem->m_szName;
}

static DWORD g_zplayer_list_update_time = 0;

#define ZPLAYERLIST_UPDATE_TIME 2000

void ZPlayerListBox::UpdateList()
{
	if (ZGetGameClient()->IsConnected() == false) return;

	DWORD this_time = GetGlobalTimeMS();

	if (this_time < g_zplayer_list_update_time + ZPLAYERLIST_UPDATE_TIME)
		return;

	g_zplayer_list_update_time = this_time;
}

void ZPlayerListBox::UpdatePlayer(const MUID& puid, MMatchObjectStageState state,
	bool isMaster, MMatchTeam nTeam)
{
	auto* pItem = static_cast<ZStagePlayerListItem*>(GetUID(puid));

	if(pItem) {
		auto Bitmap = GetStagePlayerBitmap(state, isMaster, nTeam);
		pItem->m_pBitmap = Bitmap;
		pItem->Team = nTeam;
	}	

	auto* pCharView = static_cast<ZCharacterView*>(ZGetIDLResource()->FindWidget("Stage_Charviewer_"));
	if (pCharView != nullptr && puid == pCharView->m_Info.UID)
	{
		pCharView->m_Info.bMaster = isMaster;
		pCharView->m_Info.nTeam = nTeam;
		pCharView->m_Info.nStageState = state;
	}

	if (nTeam != MMT_SPECTATOR && ZGetMyUID() == puid)
	{
		UpdateTeamButtonChecks(nTeam);
	}
}

bool ZPlayerListBox::OnCommand(MWidget* pWidget, const char* szMessage)
{
	if (pWidget == m_pButton.get()) {
		if (strcmp(szMessage, MBTN_CLK_MSG) == 0) {
			switch (GetMode()) {
			case PlayerListMode::Channel:
				SetMode(PlayerListMode::ChannelFriend);
				break;
			case PlayerListMode::ChannelFriend:
				SetMode(PlayerListMode::ChannelClan);
				break;
			case PlayerListMode::Stage:
			case PlayerListMode::StageFriend:
			case PlayerListMode::StageClan:
				break;
			case PlayerListMode::ChannelClan: 
			default:
				SetMode(PlayerListMode::Channel);
				break;
			}
			return true;
		}
	}
	else if (strcmp(szMessage, "selected") == 0) {
		if(m_nSelItem != -1) {
			auto* pItem = static_cast<ZStagePlayerListItem*>(Get(m_nSelItem));
			if(pItem) {
				MUID uid = pItem->m_PlayerUID;

				auto* CharViewWidget = ZGetIDLResource()->FindWidget("Stage_Charviewer");
				auto* pCharView = static_cast<ZCharacterView*>(CharViewWidget);
				if (pCharView != nullptr)
					pCharView->SetCharacter(uid);
			}
		}
	}
	else if (strcmp(szMessage, "selected2") == 0) {
		if ((GetKeyState(VK_MENU) & 0x8000) != 0) {
			if (m_nSelItem != -1) {
				auto* pItem = static_cast<ZStagePlayerListItem*>(Get(m_nSelItem));
				if (pItem) {
					char temp[1024];
					sprintf_safe(temp, "/kick %s", pItem->m_szName);
					ZPostStageChat(ZGetGameClient()->GetPlayerUID(), ZGetGameClient()->GetStageUID(), temp);
				}
			}
		}
	}

	return true;
}

bool ZPlayerListBox::OnEvent(MEvent* pEvent, MListener* pListener)
{
	auto rtClient = GetClientRect();

	if (pEvent->nMessage == MWM_RBUTTONDOWN) {
		if(rtClient.InPoint(pEvent->Pos)==true) {
			int nSelItem = FindItem(pEvent->Pos);
			if (nSelItem != -1) {
				ZLobbyPlayerListItem* pItem = (ZLobbyPlayerListItem*)Get(nSelItem);

				bool bShow = true;

				MCOLOR _color;
				char sp_name[256];
				MMatchUserGradeID gid;

				if (pItem->GetUID() == ZGetMyUID() &&
					GetMode() != PlayerListMode::ChannelClan) {
					bShow = false;
				}

				GetUserInfoUID(pItem->GetUID(), _color, sp_name, gid);

				if (gid == MMUG_DEVELOPER || gid == MMUG_ADMIN) {
					bShow = false;
				}

				SetSelIndex(nSelItem);
				if (bShow) {

					ZPlayerMenu* pMenu = ZApplication::GetGameInterface()->GetPlayerMenu();
					pMenu->SetTargetName(pItem->GetString());
					pMenu->SetTargetUID(pItem->GetUID());

					switch(GetMode()) {
					case PlayerListMode::Channel:
						pMenu->SetupMenu(ZPLAYERMENU_SET_LOBBY);
						break;
					case PlayerListMode::Stage:
						pMenu->SetupMenu(ZPLAYERMENU_SET_STAGE);
						break;
					case PlayerListMode::ChannelFriend:
					case PlayerListMode::StageFriend:
						pMenu->SetupMenu(ZPLAYERMENU_SET_FRIEND);
						break;
					case PlayerListMode::ChannelClan:
					case PlayerListMode::StageClan:
						if(pItem->GetUID() == ZGetMyUID())
							pMenu->SetupMenu(ZPLAYERMENU_SET_CLAN_ME);
						else
							pMenu->SetupMenu(ZPLAYERMENU_SET_CLAN);
						break;
					default:
						assert(!"Unknown PlayerMenu Setup");
					};

					MPOINT posItem;
					posItem = pEvent->Pos;
					MPOINT posMenu = MClientToScreen(this, posItem);
					
					if ( (posMenu.x + pMenu->GetClientRect().w) > (MGetWorkspaceWidth() - 5))
						posMenu.x = MGetWorkspaceWidth() - pMenu->GetClientRect().w - 5;

					if ( (posMenu.y + pMenu->GetClientRect().h) > (MGetWorkspaceHeight() - 5))
						posMenu.y = MGetWorkspaceHeight() - pMenu->GetClientRect().h - 5;

					pMenu->Show( posMenu.x, posMenu.y);
				}
				return true;
			}
		}
	}
	else if (pEvent->nMessage == MWM_LBUTTONDBLCLK)
	{
		if (rtClient.InPoint(pEvent->Pos) == true)
		{
			int nSelItem = FindItem( pEvent->Pos);

			if (nSelItem != -1)
			{
				auto* pItem = static_cast<ZLobbyPlayerListItem*>(Get(nSelItem));
				ZPostRequestCharInfoDetail(ZGetMyUID(), pItem->m_szName);
			}
		}
	}

	return MListBox::OnEvent(pEvent, pListener);;
}

MUID ZPlayerListBox::GetSelectedPlayerUID() 
{
	auto* pItem = static_cast<ZLobbyPlayerListItem*>(GetSelItem());
	if (!pItem)
		return MUID(0, 0);
    
	return pItem->GetUID();
}

void ZPlayerListBox::SelectPlayer(MUID uid)
{
	for(int i=0;i<GetCount();i++)
	{
		auto* pItem = static_cast<ZLobbyPlayerListItem*>(Get(i));
		if(pItem->GetUID()==uid){
			SetSelIndex(i);
			return;
		}
	}
}
