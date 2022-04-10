#include "stdafx.h"

#include "ZGameInterface.h"
#include "ZApplication.h"
#include "ZPost.h"
#include "ZConsole.h"
#include "MCommandLogFrame.h"
#include "ZConfiguration.h"
#include "FileInfo.h"
#include "ZInterfaceItem.h"
#include "MPicture.h"
#include "ZInterfaceListener.h"
#include "ZEffectSmoke.h"
#include "ZEffectLightTracer.h"
#include "MProfiler.h"
#include "ZActionDef.h"
#include "MSlider.h"
#include "ZMsgBox.h"
#include "MDebug.h"
#include "MMatchTransDataType.h"
#include "MBlobArray.h"
#include "MListBox.h"
#include "MTextArea.h"
#include "MTabCtrl.h"
#include "MComboBox.h"
#include "ZInterfaceBackground.h"
#include "ZCharacterSelectView.h"
#include "ZCharacterViewList.h"
#include "ZCharacterView.h"
#include "ZScreenEffectManager.h"
#include "RShaderMgr.h"
#include "ZEquipmentListBox.h"
#include "ZShop.h"
#include "ZMyItemList.h"
#include "ZMyInfo.h"
#include "ZStageSetting.h"
#include "RealSoundEffect.h"
#include "ZInitialLoading.h"
#include "RShaderMgr.h"
#include "zeffectflashbang.h"
#include "MToolTip.h"
#include "ZRoomListbox.h"
#include "ZPlayerListBox.h"
#include "MMatchNotify.h"
#include "ZMapListBox.h"
#include "ZToolTip.h"
#include "ZCanvas.h"
#include "ZCrossHair.h"
#include "ZPlayerMenu.h"
#include "ZItemMenu.h"
#include "MPanel.h"
#include "ZNetRepository.h"
#include "ZStencilLight.h"
#include "MUtil.h"
#include "ZMap.h"
#include "ZBmNumLabel.h"
#include "ZItemSlotView.h"
#include "ZMapDesc.h"
#include "MStringTable.h"

#include "ZReplay.h"
#include "MFileDialog.h"
#include "ZServerView.h"

#include "ZLocatorList.h"
#include "ZSecurity.h"
#include "ZInput.h"
#include "ZActionKey.h"
#include "ZMonsterBookInterface.h"
#include "ZGameInput.h"
#include "ZOptionInterface.h"

#include "NewChat.h"
#include "ReplayControl.h"
#include "RGMain.h"
#include "defer.h"
#include "MFile.h"

extern MCommandLogFrame* m_pLogFrame;

void ZChangeGameState(GunzState state)
{
	PostMessage(g_hWnd, WM_CHANGE_GAMESTATE, int(state), 0);
}

void ZEmptyBitmap()
{
	MBitmapManager::Destroy();
	MBitmapManager::DestroyAniBitmap();
}

static StringView GetAliasName(const StringView& Path, bool AddDir)
{
	if (AddDir)
		return Path;

	auto SlashIndex = Path.find_last_of("/\\");
	if (SlashIndex == Path.npos)
		return Path;

	return Path.substr(SlashIndex + 1);
}

static void AddBitmap(const StringView& Path, bool AddDirToAliasName)
{
#if defined(_PUBLISH) && defined(ONLY_LOAD_MRS_FILES)
	MZFile::SetReadMode(MZIPREADFLAG_ZIP | MZIPREADFLAG_MRS | MZIPREADFLAG_MRS2 | MZIPREADFLAG_FILE);
#endif

	auto MBitmapR2Create = MBeginProfile("ZGameInterface::LoadBitmaps - MBitmapR2::Create");

	MBitmapR2* pBitmap = new MBitmapR2;
	if (pBitmap->Create(GetAliasName(Path, AddDirToAliasName), RGetDevice(), Path) == true)
		MBitmapManager::Add(pBitmap);
	else
		delete pBitmap;

	MEndProfile(MBitmapR2Create);

#if defined(_PUBLISH) && defined(ONLY_LOAD_MRS_FILES)
	MZFile::SetReadMode(MZIPREADFLAG_MRS2);
#endif
}

void ZGameInterface::LoadBitmaps(const char* szDir, ZLoadingProgress *pLoadingProgress)
{
	mlog("START ZGameInterface::LoadBitmaps\n");

	static const StringView BitmapExtensions[3] = { ".png", ".bmp", ".tga" };

	auto HasBitmapExtension = [&](auto&& Path) {
		auto ExtIndex = Path.find_last_of('.');
		if (ExtIndex == Path.npos)
			return false;

		const auto Ext = Path.substr(ExtIndex);
		auto EqualsExt = [&](auto&& x) { return x == Ext; };
		return std::any_of(std::begin(BitmapExtensions), std::end(BitmapExtensions), EqualsExt);
	};

	auto& FS = *ZGetFileSystem();

	auto AddBitmapsIn = [&](auto&& Path, bool AddDirToAliasName)
	{
		auto DirNode = FS.GetDirectory(Path);
		if (!DirNode)
			return;

		for (auto&& File : FilesInDirRecursive(*DirNode))
		{
			if (HasBitmapExtension(File.Path))
				AddBitmap(File.Path, AddDirToAliasName);
		}
	};

	AddBitmapsIn(szDir, false);
	AddBitmapsIn(PATH_CUSTOM_CROSSHAIR, true);

	mlog("END ZGameInterface::LoadBitmaps\n");
}

void AddListItem(MListBox* pList, MBitmap* pBitmap, const char* szString, const char* szItemString)
{
	class MDragableListItem : public MDefaultListItem {
		char m_szDragItemString[256];
	public:
		MDragableListItem(MBitmap* pBitmap, const char* szText, const char* szItemString)
			: MDefaultListItem(pBitmap, szText) {
			if (szItemString != NULL) strcpy_safe(m_szDragItemString, szItemString);
			else m_szDragItemString[0] = 0;
		}
		virtual bool GetDragItem(MBitmap** ppDragBitmap,
			char* szDragString, char* szDragItemString) override
		{
			*ppDragBitmap = GetBitmap(0);
			if (GetString(1) != NULL) strcpy_unsafe(szDragString, GetString(1));
			else szDragString[0] = 0;
			strcpy_unsafe(szDragItemString, m_szDragItemString);
			return true;
		};
	};
	MDefaultListItem* pNew = new MDragableListItem(pBitmap, szString, szItemString);
	pList->Add(pNew);
}

bool InitSkillList(MWidget* pWidget)
{
	if (pWidget == NULL) return false;

	if (strcmp(pWidget->GetClassName(), MINT_LISTBOX) != 0) return false;
	MListBox* pList = (MListBox*)pWidget;

	pList->SetItemHeight(32);
	pList->SetVisibleHeader(false);

	pList->AddField("Icon", 32);
	pList->AddField("Name", 600);
	AddListItem(pList, MBitmapManager::Get("skill000.png"), "Fire-Ball", "Object.Skill $player $target 0");
	AddListItem(pList, MBitmapManager::Get("skill001.png"), "Bull-Shit", "Object.Skill $player $target 1");
	return true;
}

bool InitItemList(MWidget* pWidget)
{
	if (pWidget == NULL) return false;

	if (strcmp(pWidget->GetClassName(), MINT_LISTBOX) != 0) return false;
	MListBox* pList = (MListBox*)pWidget;

	pList->SetVisibleHeader(false);
	pList->SetItemHeight(40);

	pList->AddField("Icon", 42);
	pList->AddField("Name", 600);

	for (int i = 0; i < 30; i++)
	{
		char szName[256], szItem[256];
		int d = i % 6;
		sprintf_safe(szItem, "item%03d.png", d);
		sprintf_safe(szName, "나무블레이드%d", i);
		AddListItem(pList, MBitmapManager::Get(szItem), szName, "Command Something");
	}

	return true;
}

#define DEFAULT_SLIDER_MAX			10000

ZGameInterface::ZGameInterface(const char* szName, MWidget* pParent, MListener* pListener)
	: ZInterface(szName, pParent, pListener)
{
	m_bShowInterface = true;
	m_bViewUI = true;
	m_bWaitingArrangedGame = false;

	m_pMyCharacter = NULL;

	SetBounds(0, 0, MGetWorkspaceWidth(), MGetWorkspaceHeight());

	m_pGame = NULL;
	m_pCombatInterface = NULL;
	m_pLoadingInterface = NULL;

	m_dwFrameMoveClock = 0;

	m_nInitialState = GUNZ_LOGIN;
	m_nPreviousState = GUNZ_LOGIN;

	m_nState = GUNZ_NA;

	m_bCursor = true;
	m_bLogin = false;
	m_bLoading = false;

	m_pRoomListFrame = NULL;
	m_pBottomFrame = NULL;
	m_pClanInfo = NULL;

	MSetString(1, ZMsg(MSG_MENUITEM_OK));
	MSetString(2, ZMsg(MSG_MENUITEM_CANCEL));
	MSetString(3, ZMsg(MSG_MENUITEM_YES));
	MSetString(4, ZMsg(MSG_MENUITEM_NO));
	MSetString(5, ZMsg(MSG_MENUITEM_MESSAGE));
	m_pMsgBox = new ZMsgBox("", Mint::GetInstance()->GetMainFrame(), this, MT_OK);
	m_pConfirmMsgBox = new ZMsgBox("", Mint::GetInstance()->GetMainFrame(), this, MT_YESNO);

	m_pMonsterBookInterface = new ZMonsterBookInterface();

	m_pThumbnailBitmap = NULL;

	m_pCharacterSelectView = NULL;
	m_pBackground = new ZInterfaceBackground();

	m_pCursorSurface = NULL;
	g_pGameClient = NULL;

	m_nDrawCount = 0;

	m_bTeenVersion = true;

	m_pScreenEffectManager = NULL;
	m_pEffectManager = NULL;
	m_pGameInput = NULL;

	m_bReservedWeapon = false;
	m_ReservedWeapon = ZCWT_NONE;

	m_bLeaveBattleReserved = false;
	m_bLeaveStageReserved = false;

	Mint::GetInstance()->SetGlobalEvent(ZGameInterface::OnGlobalEvent);
	ZGetInput()->SetEventListener(ZGameInterface::OnGlobalEvent);

	m_pPlayerMenu = NULL;
	m_pMiniMap = NULL;

	m_bOnEndOfReplay = false;
	m_nLevelPercentCache = 0;

	m_nShopTabNum = 0;
	m_nEquipTabNum = 0;

	m_pLoginBG = NULL;
	m_pLoginPanel = NULL;

	m_dwLoginTimer = 0;

	m_nLocServ = 0;

	m_dwHourCount = 0;
	m_dwTimeCount = GetGlobalTimeMS() + 3600000;

	// Lobby Bitmap
	if (m_pRoomListFrame != NULL)
	{
		delete m_pRoomListFrame;
		m_pRoomListFrame = NULL;
	}
	if (m_pBottomFrame != NULL)
	{
		delete m_pBottomFrame;
		m_pBottomFrame = NULL;
	}
	if (m_pClanInfo != NULL)
	{
		delete m_pClanInfo;
		m_pClanInfo = NULL;
	}

	// Login Bitmap
	if (m_pLoginBG != NULL)
	{
		delete m_pLoginBG;
		m_pLoginBG = NULL;
	}

	if (m_pLoginPanel != NULL)
	{
		delete m_pLoginPanel;
		m_pLoginPanel = NULL;
	}

	m_pLocatorList = ZGetConfiguration()->GetLocatorList();
	m_pTLocatorList = ZGetConfiguration()->GetTLocatorList();

	m_dwVoiceTime = 0;
	m_szCurrVoice[ 0] = 0;
	m_szNextVoice[ 0] = 0;
	m_dwNextVoiceTime = 0;
}

ZGameInterface::~ZGameInterface()
{
	ZEmptyBitmap();

	OnDestroy();

	SAFE_DELETE(m_pMiniMap);
	SAFE_RELEASE(m_pCursorSurface);
	SAFE_DELETE(m_pScreenEffectManager);
	SAFE_DELETE(m_pEffectManager);
	SAFE_DELETE(m_pCharacterSelectView);
	SAFE_DELETE(m_pBackground);
	SAFE_DELETE(m_pMsgBox);
	SAFE_DELETE(m_pConfirmMsgBox);
	SAFE_DELETE(m_pMonsterBookInterface);

	if (m_pRoomListFrame != NULL)
		delete m_pRoomListFrame;

	if (m_pBottomFrame != NULL)
		delete m_pBottomFrame;

	if (m_pClanInfo != NULL)
		delete m_pClanInfo;

	SAFE_DELETE(m_pLoginPanel);
}

bool ZGameInterface::InitInterface(const char* szSkinName, ZLoadingProgress *pLoadingProgress)
{
	struct {
		u64 BeginTime = 0;
		u64 EndTime = 0;
	} TimingState;
	auto BeginTimingSection = [&] {
		TimingState.BeginTime = GetGlobalTimeMS();
	};
	auto EndTimingSection = [&](const char* Name) {
		TimingState.EndTime = GetGlobalTimeMS();
		auto Delta = (TimingState.EndTime - TimingState.BeginTime) / 1000.f;
		MLog("%s took %f seconds\n", Name, Delta);
	};

	SetObjectTextureLevel(ZGetConfiguration()->GetVideo()->nCharTexLevel);
	SetMapTextureLevel(ZGetConfiguration()->GetVideo()->nMapTexLevel);
	SetEffectLevel(ZGetConfiguration()->GetVideo()->nEffectLevel);
	SetTextureFormat(ZGetConfiguration()->GetVideo()->nTextureFormat);

	bool bRet = true;
	char szPath[256];
	char szFileName[256];
	char a_szSkinName[256];
	strcpy_safe(a_szSkinName, szSkinName);

	ZGetInterfaceSkinPath(szPath, a_szSkinName);
	sprintf_safe(szFileName, "%s%s", szPath, FILENAME_INTERFACE_MAIN);

	ZEmptyBitmap();

	// Pictures
	ZLoadingProgress pictureProgress("pictures", pLoadingProgress, .7f);
	auto LoadingPictures = MBeginProfile("ZGameInterface::InitInterface - LoadBitmaps");
	BeginTimingSection();

	LoadBitmaps(szPath, &pictureProgress);

	EndTimingSection("loading pictures");
	MEndProfile(LoadingPictures);

	// IDLResource loading
	auto IDLRsrc = MBeginProfile("ZGameInterface::InitInterface - MIDLResource::LoadFromFile");
	BeginTimingSection();
	if (!m_IDLResource.LoadFromFile(szFileName, this, ZGetFileSystem()))
	{
		strcpy_safe(a_szSkinName, DEFAULT_INTERFACE_SKIN);
		ZGetInterfaceSkinPath(szPath, a_szSkinName);
		sprintf_safe(szFileName, "%s%s", szPath, FILENAME_INTERFACE_MAIN);

		LoadBitmaps(szPath, &pictureProgress);

		if (m_IDLResource.LoadFromFile(szFileName, this, ZGetFileSystem()))
		{
			mlog("IDLResource Loading Success!!\n");
		}
		else
		{
			mlog("IDLResource Loading Failed!!\n");
		}
		bRet = false;
	}
	else
	{
		mlog("IDLResource Loading Success!!\n");
	}
	EndTimingSection("IDL resources");
	MEndProfile(IDLRsrc);

	// Etc.
	auto Etc = MBeginProfile("ZGameInterface::InitInterface - Etc.");
	GetRGMain().OnInitInterface(m_IDLResource);

	auto* pFrameLook = (MBFrameLook*)m_IDLResource.FindFrameLook("DefaultFrameLook");
	if (pFrameLook != nullptr)
	{
		m_pMsgBox->ChangeCustomLook((MFrameLook*)pFrameLook);
		m_pConfirmMsgBox->ChangeCustomLook((MFrameLook*)pFrameLook);
	}
	else
	{
		assert(!"DefaultFrameLook is nullptr");
	}

	ZStageSetting::InitStageSettingDialog();

	ZEquipmentListBox* pEquipmentListBox = (ZEquipmentListBox*)m_IDLResource.FindWidget("AllEquipmentList");
	if (pEquipmentListBox)
	{
		pEquipmentListBox->SetOnDropCallback(ShopPurchaseItemListBoxOnDrop);

		MWidget *pFrame = ZGetGameInterface()->GetIDLResource()->FindWidget("Shop_ItemDescriptionFrame");
		if (pFrame) {
			pFrame->Show(false);
			pEquipmentListBox->SetDescriptionWidget(pFrame);
		}
	}
	pEquipmentListBox = (ZEquipmentListBox*)m_IDLResource.FindWidget("MyAllEquipmentList");
	if (pEquipmentListBox)
	{
		pEquipmentListBox->SetOnDropCallback(ShopSaleItemListBoxOnDrop);
		MWidget *pFrame = ZGetGameInterface()->GetIDLResource()->FindWidget("Shop_ItemDescriptionFrame");
		if (pFrame) {
			pFrame->Show(false);
			pEquipmentListBox->SetDescriptionWidget(pFrame);
		}
	}

	pEquipmentListBox = (ZEquipmentListBox*)m_IDLResource.FindWidget("EquipmentList");
	if (pEquipmentListBox)
	{
		pEquipmentListBox->SetOnDropCallback(CharacterEquipmentItemListBoxOnDrop);
		MWidget *pFrame = ZGetGameInterface()->GetIDLResource()->FindWidget("Equip_ItemDescriptionFrame");
		if (pFrame) {
			pFrame->Show(false);
			pEquipmentListBox->SetDescriptionWidget(pFrame);
		}
	}

	pEquipmentListBox = (ZEquipmentListBox*)m_IDLResource.FindWidget("AccountItemList");
	if (pEquipmentListBox)
	{
		MWidget *pFrame = ZGetGameInterface()->GetIDLResource()->FindWidget("Equip_ItemDescriptionFrame");
		if (pFrame) {
			pFrame->Show(false);
			pEquipmentListBox->SetDescriptionWidget(pFrame);
		}
	}

	InitInterfaceListener();

#define CENTERMESSAGE	"CenterMessage"
	BEGIN_WIDGETLIST(CENTERMESSAGE, &m_IDLResource, MLabel*, pWidget);
	pWidget->SetAlignment(MAM_HCENTER);
	END_WIDGETLIST();

	ZGetOptionInterface()->InitInterfaceOption();

	InitMapSelectionWidget();

	_ASSERT(m_pPlayerMenu == NULL);

	m_pPlayerMenu = new ZPlayerMenu("PlayerMenu", this, this, MPMT_VERTICAL);
	m_pPlayerMenu->Show(false);
	MEndProfile(Etc);

	return true;
}

bool ZGameInterface::InitInterfaceListener()
{
	m_pMsgBox->SetListener(ZGetMsgBoxListener());
	m_pConfirmMsgBox->SetListener(ZGetConfirmMsgBoxListener());

	SetListenerWidget("VideoGamma", ZGetOptionGammaSliderChangeListener());
	SetListenerWidget("ResizeCancel", ZGetCancelResizeConfirmListener());
	SetListenerWidget("ResizeRequest", ZGetRequestResizeListener());
	SetListenerWidget("ResizeReject", ZGetViewConfirmCancelListener());
	SetListenerWidget("ResizeAccept", ZGetViewConfrimAcceptListener());
	SetListenerWidget("16BitSound", ZGet16BitSoundListener());
	SetListenerWidget("8BitSound", ZGet8BitSoundListener());

	SetListenerWidget("NetworkPortChangeRestart", ZGetNetworkPortChangeRestartListener());
	SetListenerWidget("NetworkPortChangeCancel", ZGetNetworkPortChangeCancelListener());

	SetListenerWidget("Exit", ZGetExitListener());
	SetListenerWidget("LoginOK", ZGetLoginListener());
	SetListenerWidget("CreateAccountFrameCaller", ZGetCreateAccountFrameCallerListener());
	SetListenerWidget("CreateAccountBtn", ZGetCreateAccountBtnListener());
	SetListenerWidget("OptionFrame", ZGetOptionFrameButtonListener());
	SetListenerWidget("Register", ZGetRegisterListener());
	SetListenerWidget("Stage_OptionFrame", ZGetOptionFrameButtonListener());
	SetListenerWidget("LobbyOptionFrame", ZGetOptionFrameButtonListener());

	SetListenerWidget("WarningExit", ZGetExitListener());

	SetListenerWidget("Logout", ZGetLogoutListener());
	SetListenerWidget("ChannelChattingInput", ZGetChannelChatInputListener());
	SetListenerWidget("GameStart", ZGetGameStartListener());
	SetListenerWidget("ChatInput", ZGetChatInputListener());
	SetListenerWidget("ParentClose", ZGetParentCloseListener());
	SetListenerWidget("ChannelListFrameCaller", ZGetChannelListFrameCallerListener());
	SetListenerWidget("MapList", ZGetMapListListener());
	SetListenerWidget("Lobby_StageExit", ZGetLobbyListener());
	SetListenerWidget("LoginState", ZGetLoginStateButtonListener());
	SetListenerWidget("GreeterState", ZGetGreeterStateButtonListener());
	SetListenerWidget("CombatMenuClose", ZGetCombatMenuCloseButtonListener());
	SetListenerWidget("PreviousState", ZGetPreviousStateButtonListener());
	SetListenerWidget("QuickJoin", ZGetQuickJoinButtonListener());
	SetListenerWidget("QuickJoin2", ZGetQuickJoinButtonListener());
	SetListenerWidget("Lobby_Charviewer_info", ZGetLobbyCharInfoCallerButtonListener());
	SetListenerWidget("StageBeforeBtn", ZGetLobbyPrevRoomListButtonListener());
	SetListenerWidget("StageAfterBtn", ZGetLobbyNextRoomListPrevButtonListener());

	SetListenerWidget("Lobby_RoomNo1", ZGetLobbyNextRoomNoButtonListener());
	SetListenerWidget("Lobby_RoomNo2", ZGetLobbyNextRoomNoButtonListener());
	SetListenerWidget("Lobby_RoomNo3", ZGetLobbyNextRoomNoButtonListener());
	SetListenerWidget("Lobby_RoomNo4", ZGetLobbyNextRoomNoButtonListener());
	SetListenerWidget("Lobby_RoomNo5", ZGetLobbyNextRoomNoButtonListener());
	SetListenerWidget("Lobby_RoomNo6", ZGetLobbyNextRoomNoButtonListener());

	SetListenerWidget("LobbyChannelPlayerListPrev", ZGetPlayerListPrevListener());
	SetListenerWidget("LobbyChannelPlayerListNext", ZGetPlayerListNextListener());

	SetListenerWidget("StagePlayerListPrev", ZGetStagePlayerListPrevListener());
	SetListenerWidget("StagePlayerListNext", ZGetStagePlayerListNextListener());

	SetListenerWidget("ArrangedTeamGame", ZGetArrangedTeamGameListener());
	SetListenerWidget("ArrangedTeamDialogOk", ZGetArrangedTeamDialogOkListener());
	SetListenerWidget("ArrangedTeamDialogClose", ZGetArrangedTeamDialogCloseListener());
	SetListenerWidget("ArrangedTeamGame_Cancel", ZGetArrangedTeamGame_CancelListener());

	SetListenerWidget("LeaveClanOK", ZGetLeaveClanOKListener());
	SetListenerWidget("LeaveClanCancel", ZGetLeaveClanCancelListener());

	// channel
	SetListenerWidget("ChannelJoin", ZGetChannelListJoinButtonListener());
	SetListenerWidget("ChannelListClose", ZGetChannelListCloseButtonListener());
	SetListenerWidget("ChannelList", ZGetChannelListListener());
	SetListenerWidget("PrivateChannelEnter", ZGetPrivateChannelEnterListener());

	SetListenerWidget("ChannelList_Normal", ZGetChannelList());
	SetListenerWidget("ChannelList_Private", ZGetChannelList());
	SetListenerWidget("ChannelList_Clan", ZGetChannelList());
	SetListenerWidget("MyClanChannel", ZGetMyClanChannel());

	// stage
	SetListenerWidget("StageCreateFrameCaller", ZGetStageCreateFrameCallerListener());

	SetListenerWidget("StageListFrameCaller", ZGetStageListFrameCallerListener());
	SetListenerWidget("StageCreateBtn", ZGetStageCreateBtnListener());
	SetListenerWidget("PrivateStageJoinBtn", ZGetPrivateStageJoinBtnListener());
	SetListenerWidget("StageJoin", ZGetStageJoinListener());
	SetListenerWidget("StageChattingInput", ZGetStageChatInputListener());
	SetListenerWidget("StageSettingCaller", ZGetStageSettingCallerListener());
	SetListenerWidget("StageType", ZGetStageSettingStageTypeListener());
	SetListenerWidget("StageMaxPlayer", ZGetStageSettingChangedComboboxListener());
	SetListenerWidget("StageRoundCount", ZGetStageSettingChangedComboboxListener());
	SetListenerWidget("StageLimitTime", ZGetStageSettingChangedComboboxListener());
	SetListenerWidget("StageIntrude", ZGetStageSettingChangedComboboxListener());
	SetListenerWidget("StageTeamRed", ZGetStageTeamRedListener());
	SetListenerWidget("StageTeamRed2", ZGetStageTeamRedListener());
	SetListenerWidget("StageTeamBlue", ZGetStageTeamBlueListener());
	SetListenerWidget("StageTeamBlue2", ZGetStageTeamBlueListener());
	SetListenerWidget("StageObserverBtn", ZGetStageObserverBtnListener());
	SetListenerWidget("StageReady", ZGetStageReadyListener());
	SetListenerWidget("StageSettingApplyBtn", ZGetStageSettingApplyBtnListener());
	SetListenerWidget("BattleExit", ZGetBattleExitButtonListener());
	SetListenerWidget("StageExit", ZGetStageExitButtonListener());

	SetListenerWidget("MapChange", ZGetMapChangeListener());
	SetListenerWidget("MapSelect", ZGetMapSelectListener());

	SetListenerWidget("MapList", ZGetStageMapListSelectionListener());

	SetListenerWidget("Stage_SacrificeItemButton0", ZStageSacrificeItem0());
	SetListenerWidget("Stage_SacrificeItemButton1", ZStageSacrificeItem1());
	SetListenerWidget("Stage_PutSacrificeItem", ZStagePutSacrificeItem());
	SetListenerWidget("Stage_SacrificeItemBoxOpen", ZStageSacrificeItemBoxOpen());
	SetListenerWidget("Stage_SacrificeItemBoxClose", ZStageSacrificeItemBoxClose());
	SetListenerWidget("Stage_SacrificeItemListbox", ZGetSacrificeItemListBoxListener());
	SetListenerWidget("MonsterBookCaller", ZGetMonsterBookCaller());

	SetListenerWidget("MapSelection", ZGetMapComboListener());
	SetListenerWidget("SaveOptionButton", ZGetSaveOptionButtonListener());
	SetListenerWidget("CancelOptionButton", ZGetCancelOptionButtonListener());

	SetListenerWidget("DefaultSettingLoad", ZGetLoadDefaultKeySettingListener());
	SetListenerWidget("Optimization", ZSetOptimizationListener());

	SetListenerWidget("LobbyShopCaller", ZGetShopCallerButtonListener());
	SetListenerWidget("StageShopCaller", ZGetShopCallerButtonListener());
	SetListenerWidget("ShopClose", ZGetShopCloseButtonListener());

	SetListenerWidget("LobbyEquipmentCaller", ZGetEquipmentCallerButtonListener());
	SetListenerWidget("StageEquipmentCaller", ZGetEquipmentCallerButtonListener());
	SetListenerWidget("EquipmentClose", ZGetEquipmentCloseButtonListener());

	SetListenerWidget("CharSelectionCaller", ZGetCharSelectionCallerButtonListener());
	SetListenerWidget("BuyConfirmCaller", ZGetBuyButtonListener());
	SetListenerWidget("BuyCashConfirmCaller", ZGetBuyCashItemButtonListener());
	SetListenerWidget("SellConfirmCaller", ZGetSellButtonListener());
	SetListenerWidget("SellQuestItemConfirmCaller", ZGetSellQuestItemConfirmOpenListener());
	SetListenerWidget("SellQuestItem_Cancel", ZGetSellQuestItemConfirmCloseListener());
	SetListenerWidget("SellQuestItem_Sell", ZGetSellQuestItemButtonListener());
	SetListenerWidget("SellQuestItem_CountUp", ZGetItemCountUpButtonListener());
	SetListenerWidget("SellQuestItem_CountDn", ZGetItemCountDnButtonListener());
	SetListenerWidget("Equip", ZGetEquipButtonListener());
	SetListenerWidget("EquipmentSearch", ZGetEquipmentSearchButtonListener());
	SetListenerWidget("ForcedEntryToGame", ZGetStageForcedEntryToGameListener());
	SetListenerWidget("ForcedEntryToGame2", ZGetStageForcedEntryToGameListener());
	SetListenerWidget("AllEquipmentListCaller", ZGetAllEquipmentListCallerButtonListener());
	SetListenerWidget("MyAllEquipmentListCaller", ZGetMyAllEquipmentListCallerButtonListener());
	SetListenerWidget("CashEquipmentListCaller", ZGetCashEquipmentListCallerButtonListener());
	SetListenerWidget("CashRecharge", ZGetShopCachRechargeButtonListener());
	SetListenerWidget("ShopSearchFrameCaller", ZGetShopSearchCallerButtonListener());

	SetListenerWidget("AllEquipmentList", ZGetShopPurchaseItemListBoxListener());
	SetListenerWidget("MyAllEquipmentList", ZGetShopSaleItemListBoxListener());
	SetListenerWidget("CashEquipmentList", ZGetCashShopItemListBoxListener());

	SetListenerWidget("Shop_AllEquipmentFilter", ZGetShopAllEquipmentFilterListener());
	SetListenerWidget("Equip_AllEquipmentFilter", ZGetEquipAllEquipmentFilterListener());
	SetListenerWidget("Shop_to_Equipment", ZGetShopEquipmentCallerButtonListener());

	SetListenerWidget("Shop_EquipListFrameCloseButton", ZShopListFrameClose());
	SetListenerWidget("Shop_EquipListFrameOpenButton", ZShopListFrameOpen());

	SetListenerWidget("Equipment_CharacterTab", ZGetEquipmentCharacterTabButtonListener());
	SetListenerWidget("Equipment_AccountTab", ZGetEquipmentAccountTabButtonListener());
	SetListenerWidget("EquipmentList", ZGetEquipmentItemListBoxListener());
	SetListenerWidget("AccountItemList", ZGetAccountItemListBoxListener());
	SetListenerWidget("Equipment_to_Shop", ZGetEquipmentShopCallerButtonListener());
	SetListenerWidget("SendAccountItemBtn", ZGetSendAccountItemButtonListener());
	SetListenerWidget("BringAccountItemBtn", ZGetBringAccountItemButtonListener());
	SetListenerWidget("Equip_EquipListFrameOpenButton", ZEquipListFrameOpen());
	SetListenerWidget("Equip_EquipListFrameCloseButton", ZEquipListFrameClose());
	SetListenerWidget("Equipment_CharacterRotate", ZEquipmetRotateBtn());

	SetListenerWidget("CS_SelectChar", ZGetSelectCharacterButtonListener());
	SetListenerWidget("CS_SelectCharDefKey", ZGetSelectCharacterButtonListener());
	SetListenerWidget("CS_CreateChar", ZGetShowCreateCharacterButtonListener());
	SetListenerWidget("CS_DeleteChar", ZGetDeleteCharacterButtonListener());
	SetListenerWidget("CS_Prev", ZGetLogoutListener());
	SetListenerWidget("CharSel_SelectBtn0", ZGetSelectCharacterButtonListener0());
	SetListenerWidget("CharSel_SelectBtn1", ZGetSelectCharacterButtonListener1());
	SetListenerWidget("CharSel_SelectBtn2", ZGetSelectCharacterButtonListener2());
	SetListenerWidget("CharSel_SelectBtn3", ZGetSelectCharacterButtonListener3());
	SetListenerWidget("ShowChar_infoGroup", ZGetShowCharInfoGroupListener());
	SetListenerWidget("ShowEquip_InfoGroup", ZGetShowEquipInfoGroupListener());

	SetListenerWidget("CC_CreateChar", ZGetCreateCharacterButtonListener());
	SetListenerWidget("CC_Cancel", ZGetCancelCreateCharacterButtonListener());
	SetListenerWidget("CC_Sex", ZChangeCreateCharInfoListener());
	SetListenerWidget("CC_Hair", ZChangeCreateCharInfoListener());
	SetListenerWidget("CC_Face", ZChangeCreateCharInfoListener());
	SetListenerWidget("CC_Face", ZChangeCreateCharInfoListener());
	SetListenerWidget("CC_Costume", ZChangeCreateCharInfoListener());
	SetListenerWidget("Charviewer_Rotate_L", ZGetCreateCharacterLeftButtonListener());
	SetListenerWidget("Charviewer_Rotate_R", ZGetCreateCharacterRightButtonListener());

	SetListenerWidget("Lobby_Charviewer_Rotate_L", ZGetSelectCameraLeftButtonListener());
	SetListenerWidget("Lobby_Charviewer_Rotate_R", ZGetSelectCameraRightButtonListener());

	SetListenerWidget("CS_DeleteCharButton", ZGetConfirmDeleteCharacterButtonListener());
	SetListenerWidget("CS_CloseConfirmDeleteCharButton", ZGetCloseConfirmDeleteCharButtonListener());
	SetListenerWidget("MonsterBook_PrevPageButton", ZGetMonsterInterfacePrevPage());
	SetListenerWidget("MonsterBook_NextPageButton", ZGetMonsterInterfaceNextPage());
	SetListenerWidget("MonsterBook_Close", ZGetMonsterInterfaceQuit());
	SetListenerWidget("MonsterBook_Close2", ZGetMonsterInterfaceQuit());

	SetListenerWidget("GameResult_ButtonQuit", ZGetGameResultQuit());

	SetListenerWidget("112_ConfirmEdit", ZGet112ConfirmEditListener());
	SetListenerWidget("112_ConfirmOKButton", ZGet112ConfirmOKButtonListener());
	SetListenerWidget("112_ConfirmCancelButton", ZGet112ConfirmCancelButtonListener());

	SetListenerWidget("ClanSponsorAgreementConfirm_OK",
		ZGetClanSponsorAgreementConfirm_OKButtonListener());
	SetListenerWidget("ClanSponsorAgreementConfirm_Cancel", ZGetClanSponsorAgreementConfirm_CancelButtonListener());
	SetListenerWidget("ClanSponsorAgreementWait_Cancel",
		ZGetClanSponsorAgreementWait_CancelButtonListener());
	SetListenerWidget("ClanJoinerAgreementConfirm_OK", ZGetClanJoinerAgreementConfirm_OKButtonListener());
	SetListenerWidget("ClanJoinerAgreementConfirm_Cancel", ZGetClanJoinerAgreementConfirm_CancelButtonListener());
	SetListenerWidget("ClanJoinerAgreementWait_Cancel", ZGetClanJoinerAgreementWait_CancelButtonListener());

	SetListenerWidget("LobbyPlayerListTabClanCreateButton", ZGetLobbyPlayerListTabClanCreateButtonListener());
	SetListenerWidget("ClanCreateDialogOk", ZGetClanCreateDialogOk());
	SetListenerWidget("ClanCreateDialogClose", ZGetClanCreateDialogClose());


	SetListenerWidget("ProposalAgreementWait_Cancel", ZGetProposalAgreementWait_CancelButtonListener());
	SetListenerWidget("ProposalAgreementConfirm_OK", ZGetProposalAgreementConfirm_OKButtonListener());
	SetListenerWidget("ProposalAgreementConfirm_Cancel",
		ZGetProposalAgreementConfirm_CancelButtonListener());

	SetListenerWidget("ReplayOkButton", ZReplayOk());
	SetListenerWidget("ReplayCaller", ZGetReplayCallerButtonListener());
	SetListenerWidget("ReplayLoginCaller", ZGetReplayCallerButtonListener());
	SetListenerWidget("Replay_View", ZGetReplayViewButtonListener());
	SetListenerWidget("ReplayClose", ZGetReplayExitButtonListener());
	SetListenerWidget("Replay_FileList", ZGetReplayFileListBoxListener());

	SetListenerWidget("BGMVolumeSlider", ZGetBGMVolumeSizeSliderListener());
	SetListenerWidget("EffectVolumeSlider", ZGetEffectVolumeSizeSliderListener());

	auto* pTab = ZFindWidgetAs<MTabCtrl>("PlayerListControl");
	if (pTab)
		pTab->UpdateListeners();

	GetRGMain().SetListeners();

	ZGetOptionInterface()->SetListeners();

	return true;
}

void ZGameInterface::FinalInterface()
{
	SAFE_DELETE(m_pPlayerMenu);
	m_IDLResource.Clear();
	mlog("m_IDLResource.Clear() End : \n");
	SetCursor(NULL);
	mlog("ZGameInterface::FinalInterface() End: \n");
}

bool ZGameInterface::ChangeInterfaceSkin(const char* szNewSkinName)
{
	char szPath[256];
	char szFileName[256];
	ZGetInterfaceSkinPath(szPath, szNewSkinName);
	sprintf_safe(szFileName, "%s%s", szPath, FILENAME_INTERFACE_MAIN);

	FinalInterface();
	bool bSuccess = InitInterface(szNewSkinName);

	if (bSuccess)
	{
		switch (m_nState)
		{
		case GUNZ_LOGIN:	ShowWidget("Login", true); break;
		case GUNZ_LOBBY:	ShowWidget("Lobby", true); 	break;
		case GUNZ_STAGE: 	ShowWidget("Stage", true); 	break;
		case GUNZ_CHARSELECTION:
			if (m_bShowInterface)
			{
				ShowWidget("CharSelection", true);
			}break;
		case GUNZ_CHARCREATION: ShowWidget("CharCreation", true); break;
		}
		ZGetOptionInterface()->Resize(MGetWorkspaceWidth(), MGetWorkspaceHeight());
	}

	return bSuccess;
}

static bool g_parts[10];
static bool g_parts_change;

bool ZGameInterface::ShowWidget(const char* szName, bool bVisible, bool bModal)
{
	MWidget* pWidget = m_IDLResource.FindWidget(szName);

	if (pWidget == NULL)
		return false;

	if (strcmp(szName, "Lobby") == 0)
	{
		pWidget->Show(bVisible, bModal);

		pWidget = m_IDLResource.FindWidget("Shop");
		pWidget->Show(false);
		pWidget = m_IDLResource.FindWidget("Equipment");
		pWidget->Show(false);
	}
	else
		pWidget->Show(bVisible, bModal);

	return true;
}

void ZGameInterface::SetListenerWidget(const char* szName, MListener* pListener)
{
	BEGIN_WIDGETLIST(szName, &m_IDLResource, MWidget*, pWidget);
	pWidget->SetListener(pListener);
	END_WIDGETLIST();
}

void ZGameInterface::EnableWidget(const char* szName, bool bEnable)
{
	MWidget* pWidget = m_IDLResource.FindWidget(szName);
	if (pWidget) pWidget->Enable(bEnable);
}

void ZGameInterface::SetTextWidget(const char* szName, const char* szText)
{
	BEGIN_WIDGETLIST(szName, &m_IDLResource, MWidget*, pWidget);
	pWidget->SetText(szText);
	END_WIDGETLIST();
}

bool ZGameInterface::OnGameCreate()
{
	m_Camera.Init();
	ClearMapThumbnail();

	g_parts[6] = true;

	ZApplication::GetSoundEngine()->CloseMusic();

	m_bLoading = true;

	ZLoadingProgress gameLoading("Game");

	ZGetInitialLoading()->Initialize(1, 0, 0, RGetScreenWidth(), RGetScreenHeight(), 0, 0, 1024, 768, true);

	char szFileName[256];
	int nBitmap = rand() % 9;
	switch (nBitmap)
	{
	case (0) :
		strcpy_safe(szFileName, "Interface/Default/LOADING/loading_dash.jpg");
		break;
	case (1) :
		strcpy_safe(szFileName, "Interface/Default/LOADING/loading_gaurd.jpg");
		break;
	case (2) :
		strcpy_safe(szFileName, "Interface/Default/LOADING/loading_ksa.jpg");
		break;
	case (3) :
		strcpy_safe(szFileName, "Interface/Default/LOADING/loading_safefall.jpg");
		break;
	case (4) :
		strcpy_safe(szFileName, "Interface/Default/LOADING/loading_tumbling.jpg");
		break;
	case (5) :
		strcpy_safe(szFileName, "Interface/Default/LOADING/loading_wallhang.jpg");
		break;
	case (6) :
		strcpy_safe(szFileName, "Interface/Default/LOADING/loading_walljump.jpg");
		break;
	case (7) :
		strcpy_safe(szFileName, "Interface/Default/LOADING/loading_wallrun01.jpg");
		break;
	case (8) :
		strcpy_safe(szFileName, "Interface/Default/LOADING/loading_wallrun02.jpg");
		break;
	default:
		strcpy_safe(szFileName, "");
		break;
	}

	if (!ZGetInitialLoading()->AddBitmap(0, szFileName))
		ZGetInitialLoading()->AddBitmap(0, "Interface/Default/LOADING/loading_teen.jpg");
	ZGetInitialLoading()->AddBitmapBar("Interface/Default/LOADING/loading.bmp");
	ZGetInitialLoading()->SetTipNum(nBitmap);

#ifndef _FASTDEBUG
	ZGetInitialLoading()->SetPercentage(0.0f);
	ZGetInitialLoading()->Draw(MODE_FADEIN, 0, true);
#else
	ZGetInitialLoading()->SetPercentage(10.f);
	ZGetInitialLoading()->Draw(MODE_DEFAULT, 0, true);
#endif

	m_pGame = new ZGame;
	g_pGame = m_pGame;
	if (!m_pGame->Create(ZApplication::GetFileSystem(), &gameLoading))
	{
		mlog("ZGame 생성 실패\n");
		SAFE_DELETE(m_pGame);
		g_pGame = NULL;
		m_bLoading = false;

		ZGetInitialLoading()->Release();

		return false;
	}

	m_pMyCharacter = (ZMyCharacter*)g_pGame->m_pMyCharacter;

	SetFocus();

	m_pGameInput = new ZGameInput();

	m_pCombatInterface = new ZCombatInterface("combatinterface", this, this);
	m_pCombatInterface->SetBounds(GetRect());
	m_pCombatInterface->OnCreate();


	MWidget *pWidget = m_IDLResource.FindWidget("SkillFrame");
	if (pWidget != NULL) pWidget->Show(true);

	InitSkillList(m_IDLResource.FindWidget("SkillList"));

	pWidget = m_IDLResource.FindWidget("InventoryFrame");
	if (pWidget != NULL) pWidget->Show(true);

	InitItemList(m_IDLResource.FindWidget("ItemList"));

	SetCursorEnable(false);

	m_bLoading = false;

#ifndef _FASTDEBUG
	ZGetInitialLoading()->SetPercentage(100.0f);
	ZGetInitialLoading()->Draw(MODE_FADEOUT, 0, true);
#endif
	ZGetInitialLoading()->Release();


	if ((ZApplication::GetInstance()->GetLaunchMode() == ZApplication::ZLAUNCH_MODE_STANDALONE_REPLAY) ||
		(ZGetGameClient()->IsLadderGame()) ||
		ZGetGameTypeManager()->IsQuestDerived(ZGetGameClient()->GetMatchStageSetting()->GetGameType()))
	{
		m_CombatMenu.EnableItem(ZCombatMenu::ZCMI_BATTLE_EXIT, false);
	}
	else
	{
		m_CombatMenu.EnableItem(ZCombatMenu::ZCMI_BATTLE_EXIT, true);
	}

	GetRGMain().OnGameCreate();

	return true;
}

void ZGameInterface::OnGameDestroy()
{
	mlog("OnGameDestroy Started\n");

	MPicture* pPicture;
	pPicture = (MPicture*)ZApplication::GetGameInterface()->GetIDLResource()->FindWidget("ClanResult_ClanBitmap1");
	if (pPicture) pPicture->SetBitmap(NULL);
	pPicture = (MPicture*)ZApplication::GetGameInterface()->GetIDLResource()->FindWidget("ClanResult_ClanBitmap2");
	if (pPicture) pPicture->SetBitmap(NULL);

	MWidget *pWidget = ZApplication::GetGameInterface()->GetIDLResource()->FindWidget("GameResult");
	if (pWidget) {
		MFrame *pFrame = (MFrame*)pWidget;
		pFrame->MFrame::Show(false);
	}

	ZGetGameClient()->RequestOnGameDestroyed();

	SAFE_DELETE(m_pMiniMap);

	if (m_pGameInput)
	{
		delete m_pGameInput; m_pGameInput = NULL;
	}

	if (m_pCombatInterface)
	{
		m_pCombatInterface->OnDestroy();
		delete m_pCombatInterface;
		m_pCombatInterface = NULL;
	}

	ShowWidget(CENTERMESSAGE, false);

	if (g_pGame != NULL) {
		g_pGame->Destroy();
		SAFE_DELETE(m_pGame);
		g_pGame = NULL;
	}

	SetCursorEnable(true);
	m_bLeaveBattleReserved = false;
	m_bLeaveStageReserved = false;

	mlog("OnGameDestroy Finished\n");
}

void ZGameInterface::OnGreeterCreate()
{
	ShowWidget("Greeter", true);

	if (m_pBackground)
		m_pBackground->SetScene(LOGIN_SCENE_FIXEDSKY);

	ZApplication::GetSoundEngine()->StopMusic();
	ZApplication::GetSoundEngine()->OpenMusic(BGMID_INTRO, ZApplication::GetFileSystem());
}

void ZGameInterface::OnGreeterDestroy()
{
	ShowWidget("Greeter", false);

	if (m_pBackground)
		m_pBackground->SetScene(LOGIN_SCENE_FIXEDCHAR);
}

void ZGameInterface::OnLoginCreate()
{
	m_bLoginTimeout = false;
	m_nLoginState = LoginState::FadeIn;
	m_dwLoginTimer = GetGlobalTimeMS();

	if (m_pLoginBG != NULL)
	{
		delete m_pLoginBG;
		m_pLoginBG = NULL;
	}
	m_pLoginBG = new MBitmapR2;
	((MBitmapR2*)m_pLoginBG)->Create("loginbg.png", RGetDevice(), "Interface/loadable/loginbg.jpg");
	if (m_pLoginBG)
	{
		MPicture* pPicture = (MPicture*)m_IDLResource.FindWidget("Login_BackgrdImg");
		if (pPicture)
			pPicture->SetBitmap(m_pLoginBG->GetSourceBitmap());
	}


	// 패널 이미지 로딩
	if (m_pLoginPanel != NULL)
	{
		delete m_pLoginPanel;
		m_pLoginPanel = NULL;
	}

	m_pLoginPanel = new MBitmapR2;
	((MBitmapR2*)m_pLoginPanel)->Create("loginpanel.png", RGetDevice(), "Interface/loadable/loginpanel.tga");
	if (m_pLoginPanel)
	{
		MPicture* pPicture = (MPicture*)m_IDLResource.FindWidget("Login_Panel");
		if (pPicture)
			pPicture->SetBitmap(m_pLoginPanel->GetSourceBitmap());
	}

	MButton* pLoginOk = (MButton*)m_IDLResource.FindWidget("LoginOK");
	if (pLoginOk)
		pLoginOk->Enable(true);
	MWidget* pLoginFrame = m_IDLResource.FindWidget("LoginFrame");
	if (pLoginFrame)
	{
		MWidget* pLoginBG = m_IDLResource.FindWidget("Login_BackgrdImg");
		if (pLoginBG)
			pLoginFrame->Show(false);
		else
			pLoginFrame->Show(true);
	}
	pLoginFrame = m_IDLResource.FindWidget("Login_ConnectingMsg");
	if (pLoginFrame)
		pLoginFrame->Show(false);

	MLabel* pErrorLabel = (MLabel*)m_IDLResource.FindWidget("LoginError");
	if (pErrorLabel)
		pErrorLabel->SetText("");

	MLabel* pPasswd = (MLabel*)m_IDLResource.FindWidget("LoginPassword");
	if (pPasswd)
		pPasswd->SetText("");

	if (ZApplication::GetInstance()->GetLaunchMode() == ZApplication::ZLAUNCH_MODE_NETMARBLE) {
		mlog("Netmarble Logout \n");
		ZApplication::Exit();
		return;
	}

	HideAllWidgets();

	ShowWidget("Login", true);

	ZServerView* pServerList = (ZServerView*)m_IDLResource.FindWidget("SelectedServer");
	if (pServerList)
	{
		pServerList->ClearServerList();
		if (strcmp(Z_LOCALE_DEFAULT_FONT, "Arial") == 0)
			pServerList->SetTextOffset(-2);

#ifdef	_DEBUG
		ShowWidget("LabelServerIP", true);
		ShowWidget("ServerAddress", true);
		ShowWidget("ServerPort", true);

		pServerList->AddServer("Debug Server", "", 0, 1, 0, 1000, true);			// Debug server
		pServerList->AddServer("", "", 0, 0, 0, 1000, false);						// Null
		pServerList->SetCurrSel(0);
#else
		if (ZIsLaunchDevelop())
		{
			ShowWidget("LabelServerIP", true);
			ShowWidget("ServerAddress", true);
			ShowWidget("ServerPort", true);

			pServerList->AddServer("Debug Server", "", 0, 1, 0, 1000, true);			// Debug server
			pServerList->AddServer("", "", 0, 0, 0, 1000, false);						// Null
			pServerList->SetCurrSel(0);
		}
#endif


#ifdef _LOCATOR

		if (ZApplication::GetInstance()->IsLaunchTest())
		{
			if (m_pTLocatorList && (m_pTLocatorList->GetSize() > 0))
				m_nLocServ = rand() % m_pTLocatorList->GetSize();
		}
		else
		{
			if (m_pLocatorList && (m_pLocatorList->GetSize() > 0))
				m_nLocServ = rand() % m_pLocatorList->GetSize();
		}

		RequestServerStatusListInfo();

#else

		for (int i = 0; i < ZGetConfiguration()->GetServerCount(); i++)
		{
			ZSERVERNODE ServerNode = ZGetConfiguration()->GetServerNode(i);

			if (ServerNode.nType != 1)
				pServerList->AddServer(ServerNode.szName, ServerNode.szAddress, ServerNode.nPort, ServerNode.nType, 0, 1000, true);
		}
#endif
	}


	MWidget* pWidget = m_IDLResource.FindWidget("LoginID");
	if (pWidget)
	{
		char buffer[256];
		if (ZGetApplication()->GetSystemValue("LoginID", buffer))
			pWidget->SetText(buffer);
	}

	pWidget = m_IDLResource.FindWidget("ServerAddress");
	if (pWidget)
	{
		pWidget->SetText(ZGetConfiguration()->GetServerIP());
	}
	pWidget = m_IDLResource.FindWidget("ServerPort");
	if (pWidget)
	{
		char szText[25];
		sprintf_safe(szText, "%d", ZGetConfiguration()->GetServerPort());
		pWidget->SetText(szText);
	}

	if (m_pBackground)
	{
		m_pBackground->LoadMesh();
		m_pBackground->SetScene(LOGIN_SCENE_FIXEDSKY);
	}

	ZApplication::GetSoundEngine()->StopMusic();
	ZApplication::GetSoundEngine()->OpenMusic(BGMID_INTRO, ZApplication::GetFileSystem());

	if (g_pGameClient->IsConnected())
	{
		ZPostDisconnect();
	}
}
void ZGameInterface::OnLoginDestroy()
{
	ShowWidget("Login", false);

	MWidget* pWidget = m_IDLResource.FindWidget("LoginID");
	if (pWidget)
	{
		ZGetApplication()->SetSystemValue("LoginID", pWidget->GetText());

		if (m_pBackground)
			m_pBackground->SetScene(LOGIN_SCENE_FIXEDCHAR);
	}

	if (m_pLoginBG != NULL)
	{
		MPicture* pPicture = (MPicture*)m_IDLResource.FindWidget("Login_BackgrdImg");
		if (pPicture)
			pPicture->SetBitmap(NULL);

		delete m_pLoginBG;
		m_pLoginBG = NULL;
	}

	if (m_pLoginPanel != NULL)
	{
		MPicture* pPicture = (MPicture*)m_IDLResource.FindWidget("Login_Panel");
		if (pPicture)
			pPicture->SetBitmap(NULL);

		delete m_pLoginPanel;
		m_pLoginPanel = NULL;
	}
}

void ZGameInterface::OnLobbyCreate()
{
	if (m_bOnEndOfReplay)
	{
		m_bOnEndOfReplay = false;
		ZGetMyInfo()->SetLevelPercent(m_nLevelPercentCache);
	}

	if (m_bOnEndOfReplay)
	{
		m_bOnEndOfReplay = false;
		ZGetMyInfo()->SetLevelPercent(m_nLevelPercentCache);
	}

	if (m_pBackground != 0)
		m_pBackground->Free();

	if (g_pGameClient)
	{
		g_pGameClient->ClearPeers();
		g_pGameClient->ClearStageSetting();
	}

	SetRoomNoLight(1);
	ZGetGameClient()->RequestOnLobbyCreated();


	ShowWidget("CombatMenuFrame", false);
	ShowWidget("Lobby", true);
	EnableLobbyInterface(true);

	MWidget* pWidget = m_IDLResource.FindWidget("StageName");
	if (pWidget) {
		char buffer[256];
		if (ZGetApplication()->GetSystemValue("StageName", buffer))
			pWidget->SetText(buffer);
	}

	ZRoomListBox* pRoomList = (ZRoomListBox*)m_IDLResource.FindWidget("Lobby_StageList");
	if (pRoomList) pRoomList->Clear();

	ShowWidget("Lobby_StageList", true);

	MPicture* pPicture = 0;
	pPicture = (MPicture*)ZApplication::GetGameInterface()->GetIDLResource()->FindWidget("Lobby_StripBottom");
	if (pPicture != NULL)	pPicture->SetAnimation(0, 1000.0f);
	pPicture = (MPicture*)ZApplication::GetGameInterface()->GetIDLResource()->FindWidget("Lobby_StripTop");
	if (pPicture != NULL)	pPicture->SetAnimation(1, 1000.0f);

	pPicture = (MPicture*)ZApplication::GetGameInterface()->GetIDLResource()->FindWidget("Lobby_RoomListBG");
	if (pPicture)
	{
		m_pRoomListFrame = new MBitmapR2;
		((MBitmapR2*)m_pRoomListFrame)->Create("gamelist_panel.png", RGetDevice(), "interface/loadable/gamelist_panel.png");

		if (m_pRoomListFrame != NULL)
			pPicture->SetBitmap(m_pRoomListFrame->GetSourceBitmap());
	}
	pPicture = (MPicture*)ZApplication::GetGameInterface()->GetIDLResource()->FindWidget("Lobby_BottomBG");
	if (pPicture)
	{
		m_pBottomFrame = new MBitmapR2;
		((MBitmapR2*)m_pBottomFrame)->Create("bottom_panel.png", RGetDevice(), "interface/loadable/bottom_panel.png");

		if (m_pBottomFrame != NULL)
			pPicture->SetBitmap(m_pBottomFrame->GetSourceBitmap());
	}
	pPicture = (MPicture*)ZApplication::GetGameInterface()->GetIDLResource()->FindWidget("Lobby_ClanInfoBG");
	if (pPicture)
	{
		m_pClanInfo = new MBitmapR2;
		((MBitmapR2*)m_pClanInfo)->Create("claninfo_panel.tga", RGetDevice(), "interface/loadable/claninfo_panel.tga");

		if (m_pClanInfo != NULL)
			pPicture->SetBitmap(m_pClanInfo->GetSourceBitmap());
	}

	// music
#ifdef _BIRDSOUND
	ZApplication::GetSoundEngine()->OpenMusic(BGMID_LOBBY);
	ZApplication::GetSoundEngine()->PlayMusic(true);
#else
	ZApplication::GetSoundEngine()->OpenMusic(BGMID_LOBBY, ZApplication::GetFileSystem());
	ZApplication::GetSoundEngine()->PlayMusic(true);
#endif

	auto* pPlayerListBox = ZFindWidgetAs<ZPlayerListBox>("LobbyChannelPlayerList");
	if (pPlayerListBox)
		pPlayerListBox->SetMode(ZPlayerListBox::PlayerListMode::Channel);

	pWidget = m_IDLResource.FindWidget("ChannelChattingInput");
	if (pWidget)
		pWidget->SetFocus();

	if (m_pBackground)
		m_pBackground->SetScene(LOGIN_SCENE_FIXEDCHAR);
}

void ZGameInterface::OnLobbyDestroy()
{
	ShowWidget("Lobby", false);

	auto* pPicture = ZFindWidgetAs<MPicture>("Lobby_RoomListBG");
	if (pPicture)
		pPicture->SetBitmap(NULL);

	pPicture = ZFindWidgetAs<MPicture>("Lobby_BottomBG");
	if (pPicture)
		pPicture->SetBitmap(NULL);

	pPicture = ZFindWidgetAs<MPicture>("Lobby_ClanInfoBG");
	if (pPicture)
		pPicture->SetBitmap(NULL);

	if (m_pRoomListFrame != NULL)
	{
		delete m_pRoomListFrame;
		m_pRoomListFrame = NULL;
	}
	if (m_pBottomFrame != NULL)
	{
		delete m_pBottomFrame;
		m_pBottomFrame = NULL;
	}
	if (m_pClanInfo != NULL)
	{
		delete m_pClanInfo;
		m_pClanInfo = NULL;
	}


	MWidget* pWidget = m_IDLResource.FindWidget("StageName");
	if (pWidget) ZGetApplication()->SetSystemValue("StageName", pWidget->GetText());
}

void ZGameInterface::OnStageCreate()
{
	mlog("StageCreated\n");

	if (g_pGameClient)
	{
		g_pGameClient->ClearPeers();
	}

	ShowWidget("Shop", false);
	ShowWidget("Equipment", false);
	ShowWidget("Stage", true);
	EnableStageInterface(true);
	MButton* pObserverBtn = (MButton*)m_IDLResource.FindWidget("StageObserverBtn");
	if (pObserverBtn)
		pObserverBtn->SetCheck(false);

	ZCharacterView* pCharView = (ZCharacterView*)m_IDLResource.FindWidget("Stage_Charviewer");

	if (pCharView != NULL)
	{
		pCharView->SetCharacter(ZGetMyUID());
	}

	MPicture* pPicture = (MPicture*)ZApplication::GetGameInterface()->GetIDLResource()->FindWidget("Stage_StripBottom");
	if (pPicture != NULL)	pPicture->SetBitmapColor(0xFFFFFFFF);
	pPicture = (MPicture*)ZApplication::GetGameInterface()->GetIDLResource()->FindWidget("Stage_StripTop");
	if (pPicture != NULL)	pPicture->SetBitmapColor(0xFFFFFFFF);

	ZPostRequestStageSetting(ZGetGameClient()->GetStageUID());
	SerializeStageInterface();

#ifdef _BIRDSOUND
	ZApplication::GetSoundEngine()->OpenMusic(BGMID_LOBBY);
	ZApplication::GetSoundEngine()->PlayMusic(true);
#else
	ZApplication::GetSoundEngine()->OpenMusic(BGMID_LOBBY, ZApplication::GetFileSystem());
	ZApplication::GetSoundEngine()->PlayMusic(true);
#endif

	ZApplication::GetStageInterface()->OnCreate();
}

void ZGameInterface::OnStageDestroy()
{
	ZApplication::GetStageInterface()->OnDestroy();
}

char* GetItemSlotName(const char* szName, int nItem)
{
	static char szTemp[256];
	strcpy_safe(szTemp, szName);

	switch (nItem)
	{
	case 0:
		strcat_safe(szTemp, "_EquipmentSlot_Head");
		break;
	case 1:
		strcat_safe(szTemp, "_EquipmentSlot_Chest");
		break;
	case 2:
		strcat_safe(szTemp, "_EquipmentSlot_Hands");
		break;
	case 3:
		strcat_safe(szTemp, "_EquipmentSlot_Legs");
		break;
	case 4:
		strcat_safe(szTemp, "_EquipmentSlot_Feet");
		break;
	case 5:
		strcat_safe(szTemp, "_EquipmentSlot_FingerL");
		break;
	case 6:
		strcat_safe(szTemp, "_EquipmentSlot_FingerR");
		break;
	case 7:
		strcat_safe(szTemp, "_EquipmentSlot_Melee");
		break;
	case 8:
		strcat_safe(szTemp, "_EquipmentSlot_Primary");
		break;
	case 9:
		strcat_safe(szTemp, "_EquipmentSlot_Secondary");
		break;
	case 10:
		strcat_safe(szTemp, "_EquipmentSlot_Custom1");
		break;
	case 11:
		strcat_safe(szTemp, "_EquipmentSlot_Custom2");
		break;
	}

	return szTemp;
}

bool ZGameInterface::OnCreate(ZLoadingProgress *pLoadingProgress)
{
	g_pGameClient = new ZGameClient();

	if (!m_Tips.Initialize(ZApplication::GetFileSystem(), ML_INVALID)) {
		mlog("Check tips.xml\n");
		return false;
	}

	auto ZGameInterfaceInitInterface = MBeginProfile("ZGameInterface::InitInterface");
	ZLoadingProgress interfaceProgress("interfaceSkin", pLoadingProgress, .7f);
	if (!InitInterface(ZGetConfiguration()->GetInterfaceSkinName(), &interfaceProgress))
	{
		mlog("ZGameInterface::OnCreate: Failed InitInterface\n");
		return false;
	}
	MEndProfile(ZGameInterfaceInitInterface);

	interfaceProgress.UpdateAndDraw(1.f);

	auto ZScreenEffectManagerCreate = MBeginProfile("ZGameInterface - ZScreenEffectManager::Create");
	m_pScreenEffectManager = new ZScreenEffectManager;
	if (!m_pScreenEffectManager->Create())
		return false;
	MEndProfile(ZScreenEffectManagerCreate);

	auto ZEffectManagerCreate = MBeginProfile("ZGameInterface - ZEffectManager::Create");
	m_pEffectManager = new ZEffectManager;
	if (!m_pEffectManager->Create())
		return false;
	MEndProfile(ZEffectManagerCreate);

	SetTeenVersion(false);

	auto ZGameClientCreate = MBeginProfile("ZGameInterface - ZGameClient::Create");
	const auto Start = ZGetConfiguration()->GetEtc()->nNetworkPort1;
	const auto End = ZGetConfiguration()->GetEtc()->nNetworkPort2;
	int nNetworkPort = RandomNumber(Start, End);

	MLog("ZGameInterface::OnCreate -- Picked %d as the network port (from a range of %d to %d)\n",
		nNetworkPort, Start, End);

	const auto CreateGameClientResult = g_pGameClient->Create(nNetworkPort);
	if (CreateGameClientResult.ErrorCode != 0)
	{
		char ErrorMessage[512];
		sprintf_safe(ErrorMessage, "Network error %d: %s",
			CreateGameClientResult.ErrorCode, CreateGameClientResult.ErrorMessage.c_str());
		ShowMessage(ErrorMessage);
	}

	g_pGameClient->SetOnCommandCallback(OnCommand);
	g_pGameClient->CreateUPnP(nNetworkPort);
	MEndProfile(ZGameClientCreate);

	auto Etc = MBeginProfile("ZGameInterface - Etc.");

	ZItemSlotView* itemSlot;
	for (int i = 0; i < MMCIP_END; i++)
	{
		itemSlot = (ZItemSlotView*)m_IDLResource.FindWidget(GetItemSlotName("Shop", i));
		if (itemSlot)
		{
			strcpy_safe(itemSlot->m_szItemSlotPlace, "Shop");

			switch (itemSlot->GetParts())
			{
			case MMCIP_HEAD:		itemSlot->SetText(ZMsg(MSG_WORD_HEAD));		break;
			case MMCIP_CHEST:		itemSlot->SetText(ZMsg(MSG_WORD_CHEST));		break;
			case MMCIP_HANDS:		itemSlot->SetText(ZMsg(MSG_WORD_HANDS));		break;
			case MMCIP_LEGS:		itemSlot->SetText(ZMsg(MSG_WORD_LEGS));		break;
			case MMCIP_FEET:		itemSlot->SetText(ZMsg(MSG_WORD_FEET));		break;
			case MMCIP_FINGERL:		itemSlot->SetText(ZMsg(MSG_WORD_LFINGER));	break;
			case MMCIP_FINGERR:		itemSlot->SetText(ZMsg(MSG_WORD_RFINGER));	break;
			case MMCIP_MELEE:		itemSlot->SetText(ZMsg(MSG_WORD_MELEE));		break;
			case MMCIP_PRIMARY:		itemSlot->SetText(ZMsg(MSG_WORD_WEAPON1));	break;
			case MMCIP_SECONDARY:	itemSlot->SetText(ZMsg(MSG_WORD_WEAPON2));	break;
			case MMCIP_CUSTOM1:		itemSlot->SetText(ZMsg(MSG_WORD_ITEM1));		break;
			case MMCIP_CUSTOM2:		itemSlot->SetText(ZMsg(MSG_WORD_ITEM2));		break;
			default:				itemSlot->SetText("");							break;
			}
		}
	}
	for (int i = 0; i < MMCIP_END; i++)
	{
		itemSlot = (ZItemSlotView*)m_IDLResource.FindWidget(GetItemSlotName("Equip", i));
		if (itemSlot)
		{
			strcpy_safe(itemSlot->m_szItemSlotPlace, "Equip");

			switch (itemSlot->GetParts())
			{
			case MMCIP_HEAD:		itemSlot->SetText(ZMsg(MSG_WORD_HEAD));		break;
			case MMCIP_CHEST:		itemSlot->SetText(ZMsg(MSG_WORD_CHEST));		break;
			case MMCIP_HANDS:		itemSlot->SetText(ZMsg(MSG_WORD_HANDS));		break;
			case MMCIP_LEGS:		itemSlot->SetText(ZMsg(MSG_WORD_LEGS));		break;
			case MMCIP_FEET:		itemSlot->SetText(ZMsg(MSG_WORD_FEET));		break;
			case MMCIP_FINGERL:		itemSlot->SetText(ZMsg(MSG_WORD_LFINGER));	break;
			case MMCIP_FINGERR:		itemSlot->SetText(ZMsg(MSG_WORD_RFINGER));	break;
			case MMCIP_MELEE:		itemSlot->SetText(ZMsg(MSG_WORD_MELEE));		break;
			case MMCIP_PRIMARY:		itemSlot->SetText(ZMsg(MSG_WORD_WEAPON1));	break;
			case MMCIP_SECONDARY:	itemSlot->SetText(ZMsg(MSG_WORD_WEAPON2));	break;
			case MMCIP_CUSTOM1:		itemSlot->SetText(ZMsg(MSG_WORD_ITEM1));		break;
			case MMCIP_CUSTOM2:		itemSlot->SetText(ZMsg(MSG_WORD_ITEM2));		break;
			default:				itemSlot->SetText("");							break;
			}
		}
	}
	ZEquipmentListBox* pItemListBox = (ZEquipmentListBox*)m_IDLResource.FindWidget("EquipmentList");
	if (pItemListBox)
	{
		pItemListBox->AttachMenu(new ZItemMenu("ItemMenu", pItemListBox, pItemListBox, MPMT_VERTICAL));
		pItemListBox->EnableDragAndDrop(true);
	}

	MListBox* pReplayBox = (MListBox*)m_IDLResource.FindWidget("Replay_FileList");
	if (pReplayBox)
	{
		pReplayBox->m_FontAlign = MAM_VCENTER;
		pReplayBox->SetVisibleHeader(false);
		pReplayBox->AddField("NAME", 200);
		pReplayBox->AddField("VERSION", 70);
	}

	// Setting Configuration about ZGameClient
	if (Z_ETC_BOOST && RIsActive())
		g_pGameClient->PriorityBoost(true);

	g_pGameClient->SetRejectNormalChat(Z_ETC_REJECT_NORMALCHAT);
	g_pGameClient->SetRejectTeamChat(Z_ETC_REJECT_TEAMCHAT);
	g_pGameClient->SetRejectClanChat(Z_ETC_REJECT_CLANCHAT);
	g_pGameClient->SetRejectWhisper(Z_ETC_REJECT_WHISPER);
	g_pGameClient->SetRejectInvite(Z_ETC_REJECT_INVITE);

	MTextArea* pTextArea = (MTextArea*)m_IDLResource.FindWidget("CombatResult_PlayerNameList");
	if (pTextArea)
	{
		pTextArea->SetFont(MFontManager::Get("FONTa10b"));
		pTextArea->SetLineHeight(18);
	}
	pTextArea = (MTextArea*)m_IDLResource.FindWidget("CombatResult_PlayerKillList");
	if (pTextArea)
	{
		pTextArea->SetFont(MFontManager::Get("FONTa10b"));
		pTextArea->SetLineHeight(18);
	}
	pTextArea = (MTextArea*)m_IDLResource.FindWidget("CombatResult_PlayerDeathList");
	if (pTextArea)
	{
		pTextArea->SetFont(MFontManager::Get("FONTa10b"));
		pTextArea->SetLineHeight(18);
	}
	pTextArea = (MTextArea*)m_IDLResource.FindWidget("ClanResult_PlayerNameList1");
	if (pTextArea)
	{
		pTextArea->SetFont(MFontManager::Get("FONTa10b"));
		pTextArea->SetLineHeight(18);
	}
	pTextArea = (MTextArea*)m_IDLResource.FindWidget("ClanResult_PlayerKillList1");
	if (pTextArea)
	{
		pTextArea->SetFont(MFontManager::Get("FONTa10b"));
		pTextArea->SetLineHeight(18);
	}
	pTextArea = (MTextArea*)m_IDLResource.FindWidget("ClanResult_PlayerDeathList1");
	if (pTextArea)
	{
		pTextArea->SetFont(MFontManager::Get("FONTa10b"));
		pTextArea->SetLineHeight(18);
	}
	pTextArea = (MTextArea*)m_IDLResource.FindWidget("ClanResult_PlayerNameList2");
	if (pTextArea)
	{
		pTextArea->SetFont(MFontManager::Get("FONTa10b"));
		pTextArea->SetLineHeight(18);
	}
	pTextArea = (MTextArea*)m_IDLResource.FindWidget("ClanResult_PlayerKillList2");
	if (pTextArea)
	{
		pTextArea->SetFont(MFontManager::Get("FONTa10b"));
		pTextArea->SetLineHeight(18);
	}
	pTextArea = (MTextArea*)m_IDLResource.FindWidget("ClanResult_PlayerDeathList2");
	if (pTextArea)
	{
		pTextArea->SetFont(MFontManager::Get("FONTa10b"));
		pTextArea->SetLineHeight(18);
	}
	MPicture* pPicture = (MPicture*)m_IDLResource.FindWidget("CombatResult_Header");
	if (pPicture)
		pPicture->SetOpacity(80);
	pPicture = (MPicture*)m_IDLResource.FindWidget("ClanResult_Header1");
	if (pPicture)
		pPicture->SetOpacity(80);
	pPicture = (MPicture*)m_IDLResource.FindWidget("ClanResult_Header2");
	if (pPicture)
		pPicture->SetOpacity(80);

	MListBox* pListBox = (MListBox*)m_IDLResource.FindWidget("Stage_SacrificeItemListbox");
	if (pListBox)
	{
		pListBox->m_FontAlign = MAM_VCENTER;
		pListBox->AddField("ICON", 32);
		pListBox->AddField("NAME", 170);
		pListBox->SetItemHeight(32);
		pListBox->SetVisibleHeader(false);
		pListBox->m_bNullFrame = true;
		pListBox->EnableDragAndDrop(true);
		pListBox->SetOnDropCallback(OnDropCallbackRemoveSacrificeItem);
	}

	itemSlot = (ZItemSlotView*)m_IDLResource.FindWidget("Stage_SacrificeItemButton0");
	if (itemSlot)
	{
		itemSlot->EnableDragAndDrop(true);
		strcpy_safe(itemSlot->m_szItemSlotPlace, "SACRIFICE0");
	}
	itemSlot = (ZItemSlotView*)m_IDLResource.FindWidget("Stage_SacrificeItemButton1");
	if (itemSlot)
	{
		itemSlot->EnableDragAndDrop(true);
		strcpy_safe(itemSlot->m_szItemSlotPlace, "SACRIFICE1");
	}

	pListBox = (MListBox*)m_IDLResource.FindWidget("QuestResult_ItemListbox");
	if (pListBox)
	{
		pListBox->m_FontAlign = MAM_VCENTER;
		pListBox->AddField("ICON", 35);
		pListBox->AddField("NAME", 300);
		pListBox->SetItemHeight(32);
		pListBox->SetVisibleHeader(false);
		pListBox->SetFont(MFontManager::Get("FONTa10b"));
		pListBox->m_FontColor = MCOLOR(0xFFFFF794);
		pListBox->m_bNullFrame = true;
	}

	int nMargin[BMNUM_NUMOFCHARSET] = { 15,15,15,15,15,15,15,15,15,15,8,10,8 };
	ZBmNumLabel* pBmNumLabel = (ZBmNumLabel*)m_IDLResource.FindWidget("Lobby_ClanInfoWinLose");
	if (pBmNumLabel)
		pBmNumLabel->SetCharMargin(nMargin);
	pBmNumLabel = (ZBmNumLabel*)m_IDLResource.FindWidget("Lobby_ClanInfoPoints");
	if (pBmNumLabel)
		pBmNumLabel->SetCharMargin(nMargin);
	pBmNumLabel = (ZBmNumLabel*)m_IDLResource.FindWidget("Lobby_ClanInfoTotalPoints");
	if (pBmNumLabel)
		pBmNumLabel->SetCharMargin(nMargin);
	pBmNumLabel = (ZBmNumLabel*)m_IDLResource.FindWidget("Lobby_ClanInfoRanking");
	if (pBmNumLabel)
		pBmNumLabel->SetCharMargin(nMargin);
	pBmNumLabel = (ZBmNumLabel*)m_IDLResource.FindWidget("QuestResult_GetPlusXP");
	if (pBmNumLabel)
		pBmNumLabel->SetCharMargin(nMargin);
	pBmNumLabel = (ZBmNumLabel*)m_IDLResource.FindWidget("QuestResult_GetMinusXP");
	if (pBmNumLabel)
	{
		pBmNumLabel->SetCharMargin(nMargin);
		pBmNumLabel->SetIndexOffset(16);
	}
	pBmNumLabel = (ZBmNumLabel*)m_IDLResource.FindWidget("QuestResult_GetTotalXP");
	if (pBmNumLabel)
		pBmNumLabel->SetCharMargin(nMargin);
	pBmNumLabel = (ZBmNumLabel*)m_IDLResource.FindWidget("QuestResult_GetBounty");
	if (pBmNumLabel)
		pBmNumLabel->SetCharMargin(nMargin);

	ZGetGameTypeManager()->SetGameTypeStr(MMATCH_GAMETYPE_DEATHMATCH_SOLO, ZMsg(MSG_MT_DEATHMATCH_SOLO));
	ZGetGameTypeManager()->SetGameTypeStr(MMATCH_GAMETYPE_DEATHMATCH_TEAM, ZMsg(MSG_MT_DEATHMATCH_TEAM));
	ZGetGameTypeManager()->SetGameTypeStr(MMATCH_GAMETYPE_GLADIATOR_SOLO, ZMsg(MSG_MT_GLADIATOR_SOLO));
	ZGetGameTypeManager()->SetGameTypeStr(MMATCH_GAMETYPE_GLADIATOR_TEAM, ZMsg(MSG_MT_GLADIATOR_TEAM));
	ZGetGameTypeManager()->SetGameTypeStr(MMATCH_GAMETYPE_ASSASSINATE, ZMsg(MSG_MT_ASSASSINATE));
	ZGetGameTypeManager()->SetGameTypeStr(MMATCH_GAMETYPE_TRAINING, ZMsg(MSG_MT_TRAINING));
	ZGetGameTypeManager()->SetGameTypeStr(MMATCH_GAMETYPE_SURVIVAL, ZMsg(MSG_MT_SURVIVAL));
	ZGetGameTypeManager()->SetGameTypeStr(MMATCH_GAMETYPE_QUEST, ZMsg(MSG_MT_QUEST));
	ZGetGameTypeManager()->SetGameTypeStr(MMATCH_GAMETYPE_BERSERKER, ZMsg(MSG_MT_BERSERKER));
	ZGetGameTypeManager()->SetGameTypeStr(MMATCH_GAMETYPE_DEATHMATCH_TEAM2, ZMsg(MSG_MT_DEATHMATCH_TEAM2));
	ZGetGameTypeManager()->SetGameTypeStr(MMATCH_GAMETYPE_DUEL, ZMsg(MSG_MT_DUEL));
	ZGetGameTypeManager()->SetGameTypeStr(MMATCH_GAMETYPE_SKILLMAP, "Skillmap");

#ifndef _DEBUG
	MWidget* pWidget = m_IDLResource.FindWidget("MonsterBookCaller");
	if (pWidget)
		pWidget->Show(false);
#endif

#ifndef _QUEST_ITEM
	MComboBox* pComboBox = (MComboBox*)m_IDLResource.FindWidget("Shop_AllEquipmentFilter");
	if (pComboBox)
		pComboBox->Remove(10);
	pComboBox = (MComboBox*)m_IDLResource.FindWidget("Equip_AllEquipmentFilter");
	if (pComboBox)
		pComboBox->Remove(10);
#endif

	MComboBox* pCombo = (MComboBox*)m_IDLResource.FindWidget("Shop_AllEquipmentFilter");
	if (pCombo)
	{
		pCombo->SetAlignment(MAM_LEFT);
		pCombo->SetListboxAlignment(MAM_LEFT);
	}
	pCombo = (MComboBox*)m_IDLResource.FindWidget("Equip_AllEquipmentFilter");
	if (pCombo)
	{
		pCombo->SetAlignment(MAM_LEFT);
		pCombo->SetListboxAlignment(MAM_LEFT);
	}

	pCombo = (MComboBox*)m_IDLResource.FindWidget("StageType");
	if (pCombo)
		pCombo->SetListboxAlignment(MAM_LEFT);

	ZCharacterView* pCharView = (ZCharacterView*)m_IDLResource.FindWidget("EquipmentInformation");
	if (pCharView)
		pCharView->EnableAutoRotate(true);
	pCharView = (ZCharacterView*)m_IDLResource.FindWidget("EquipmentInformationShop");
	if (pCharView)
		pCharView->EnableAutoRotate(true);

	MEndProfile(Etc);

	mlog("ZGameInterface::OnCreate : done \n");

	return true;
}

void ZGameInterface::OnDestroy()
{
	mlog("ZGameInterface::OnDestroy() : begin \n");

	ZCharacterView* pCharView = (ZCharacterView*)m_IDLResource.FindWidget("Stage_Charviewer");
	if (pCharView != 0) pCharView->OnInvalidate();

	SetCursorEnable(false);

	SetState(GUNZ_NA);

	SAFE_DELETE(m_pThumbnailBitmap);

	g_pGameClient->Destroy();

	mlog("ZGameInterface::OnDestroy() : g_pGameClient->Destroy() \n");

	SAFE_DELETE(g_pGameClient);

	mlog("SAFE_DELETE(g_pGameClient) : \n");

	SAFE_DELETE(m_pLoginBG);

	m_Tips.Finalize();
	FinalInterface();

	mlog("ZGameInterface::OnDestroy() : done() \n");
}

void ZGameInterface::OnShutdownState()
{
	mlog("ZGameInterface::OnShutdown() : begin \n");

	ZIDLResource* pResource = ZApplication::GetGameInterface()->GetIDLResource();
	MLabel* pLabel = (MLabel*)pResource->FindWidget("NetmarbleLoginMessage");
	pLabel->SetText(ZErrStr(MERR_CLIENT_DISCONNECTED));
	ZApplication::GetGameInterface()->ShowWidget("NetmarbleLogin", true);

	mlog("ZGameInterface::OnShutdown() : done() \n");
}

bool ZGameInterface::SetState(GunzState nState)
{
	if (m_nState == nState)
		return true;

	if (nState == GUNZ_PREVIOUS)
		nState = m_nPreviousState;

	if (nState == GUNZ_GAME)
	{
		if (ZApplication::GetStageInterface()->IsShowStartMovieOfQuest())
		{
			ZApplication::GetStageInterface()->ChangeStageEnableReady(true);
			MWidget* pWidget = ZApplication::GetGameInterface()->GetIDLResource()->FindWidget("StageReady");
			if (pWidget)
				pWidget->Enable(false);
			ZApplication::GetStageInterface()->StartMovieOfQuest();
			return true;
		}
	}

	m_nPreviousState = m_nState;

	if (m_nState == GUNZ_GAME) { OnGameDestroy(); }
	else if (m_nState == GUNZ_LOGIN) { if (!GetIDLResource()->FindWidget("Login_BackgrdImg")) { OnLoginDestroy(); } }
	else if (m_nState == GUNZ_LOBBY) { OnLobbyDestroy(); }
	else if (m_nState == GUNZ_STAGE) { OnStageDestroy(); }
	else if (m_nState == GUNZ_GREETER) { OnGreeterDestroy(); }
	else if (m_nState == GUNZ_CHARSELECTION)
	{
		OnCharSelectionDestroy();

		if (nState == GUNZ_LOBBY)
			ZPostRequestGetCharQuestItemInfo(ZGetGameClient()->GetPlayerUID());
	}
	else if (m_nState == GUNZ_CHARCREATION) { OnCharCreationDestroy(); }

	bool bStateChanged = true;
	if (nState == GUNZ_GAME) { bStateChanged = OnGameCreate(); }
	else if (nState == GUNZ_LOGIN) { OnLoginCreate(); }
	else if (nState == GUNZ_LOBBY) { OnLobbyCreate(); }
	else if (nState == GUNZ_STAGE) { OnStageCreate(); }
	else if (nState == GUNZ_GREETER) { OnGreeterCreate(); }
	else if (nState == GUNZ_CHARSELECTION)
	{
		if (m_nPreviousState == GUNZ_LOGIN)
		{
			MWidget* pWidget = m_IDLResource.FindWidget("Login_BackgrdImg");
			if (!pWidget)
				OnCharSelectionCreate();
			else
			{
				m_nLoginState = LoginState::LoginComplete;
				m_dwLoginTimer = GetGlobalTimeMS() + 1000;
				return true;
			}
		}
		else
			OnCharSelectionCreate();
	}
	else if (nState == GUNZ_CHARCREATION) { OnCharCreationCreate(); }
	else if (nState == GUNZ_SHUTDOWN) { OnShutdownState(); }

	if (bStateChanged == false) {
		m_pMsgBox->SetText("Error: Can't Create a Game!");
		m_pMsgBox->Show(true, true);
		SetState(GUNZ_PREVIOUS);
	}
	else {
		m_nState = nState;
	}

	m_nDrawCount = 0;
	return bStateChanged;
}

#ifndef _PUBLISH
#include "fmod.h"
#endif

void ZGameInterface::OnDrawStateGame(MDrawContext* pDC)
{
	if (m_pGame != NULL)
	{
		if (!IsMiniMapEnable())
			m_pGame->Draw();

		if (m_bViewUI) {

			if (m_bLeaveBattleReserved)
			{
				int nSeconds = (m_dwLeaveBattleTime - GetGlobalTimeMS() + 999) / 1000;
				m_pCombatInterface->SetDrawLeaveBattle(m_bLeaveBattleReserved, nSeconds);
			}
			else
				m_pCombatInterface->SetDrawLeaveBattle(false, 0);

			if (GetCamera()->GetLookMode() == ZCAMERA_MINIMAP) {
				_ASSERT(m_pMiniMap);
				m_pMiniMap->OnDraw(pDC);
			}

			m_pCombatInterface->OnDrawCustom(pDC);

			GetRGMain().OnDrawGameInterface(pDC);
		}

	}
	m_ScreenDebugger.DrawDebugInfo(pDC);
}

void ZGameInterface::OnDrawStateLogin(MDrawContext* pDC)
{
	MLabel* pConnectingLabel = (MLabel*)m_IDLResource.FindWidget("Login_ConnectingMsg");
	if (pConnectingLabel)
	{
		char szMsg[128];
		memset(szMsg, 0, sizeof(szMsg));
		int nCount = (GetGlobalTimeMS() / 800) % 4;
		for (int i = 0; i < nCount; i++)
			szMsg[i] = '<';
		sprintf_safe(szMsg, "%s %s ", szMsg, "Connecting");
		for (int i = 0; i < nCount; i++)
			strcat_safe(szMsg, ">");

		pConnectingLabel->SetText(szMsg);
		pConnectingLabel->SetAlignment(MAM_HCENTER | MAM_VCENTER);
	}


	MWidget* pWidget = m_IDLResource.FindWidget("LoginFrame");
	MPicture* pPicture = (MPicture*)m_IDLResource.FindWidget("Login_BackgrdImg");
	if (!pWidget || !pPicture)
		return;


	ZServerView* pServerList = (ZServerView*)m_IDLResource.FindWidget("SelectedServer");
	MEdit* pPassword = (MEdit*)m_IDLResource.FindWidget("LoginPassword");
	MWidget* pLogin = m_IDLResource.FindWidget("LoginOK");
	if (pServerList && pPassword && pLogin)
	{
		if (pServerList->IsSelected() && (int)strlen(pPassword->GetText()))
			pLogin->Enable(true);
		else
			pLogin->Enable(false);
	}


	DWORD dwCurrTime = GetGlobalTimeMS();

	// Check timeout
	if (m_bLoginTimeout && (m_dwLoginTimeout <= dwCurrTime))
	{
		m_bLoginTimeout = false;

		MLog("Login timeout!\n");
		MLabel* pLabel = (MLabel*)m_IDLResource.FindWidget("LoginError");
		if (pLabel)
			pLabel->SetText(ZErrStr(MERR_CLIENT_CONNECT_FAILED));

		MButton* pLoginOK = (MButton*)m_IDLResource.FindWidget("LoginOK");
		if (pLoginOK)
			pLoginOK->Enable(true);

		pWidget->Show(true);

		if (pConnectingLabel)
			pConnectingLabel->Show(false);
	}

	// Fade in
	if (m_nLoginState == LoginState::FadeIn)
	{
		m_bLoginTimeout = false;

		if (dwCurrTime >= m_dwLoginTimer)
		{
			int nOpacity = pPicture->GetOpacity() + 3;
			if (nOpacity > 255)
				nOpacity = 255;

			pPicture->SetOpacity(nOpacity);

			m_dwLoginTimer = dwCurrTime + 9;
		}

		if (pPicture->GetOpacity() == 255)
		{
			m_dwLoginTimer = dwCurrTime + 1000;
			m_nLoginState = LoginState::ShowLoginFrame;
		}
		else
			pWidget->Show(false);
	}
	// Show login frame
	else if (m_nLoginState == LoginState::ShowLoginFrame)
	{
		m_bLoginTimeout = false;

		if (GetGlobalTimeMS() > m_dwLoginTimer)
		{
			m_nLoginState = LoginState::Standby;
			pWidget->Show(true);
		}
	}
	// Standby
	else if (m_nLoginState == LoginState::Standby)
	{
#ifdef _LOCATOR
		// Refresh server status info
		if (GetGlobalTimeMS() > m_dwRefreshTime)
			RequestServerStatusListInfo();
#endif
	}
	// Login Complete
	else if (m_nLoginState == LoginState::LoginComplete)
	{
		m_bLoginTimeout = false;

		if (GetGlobalTimeMS() > m_dwLoginTimer)
			m_nLoginState = LoginState::Fadeout;

		if (pConnectingLabel)
			pConnectingLabel->Show(false);
	}
	// Fade out
	else if (m_nLoginState == LoginState::Fadeout)
	{
		m_bLoginTimeout = false;
		pWidget->Show(false);

		if (dwCurrTime >= m_dwLoginTimer)
		{
			int nOpacity = pPicture->GetOpacity() - 3;
			if (nOpacity < 0)
				nOpacity = 0;

			pPicture->SetOpacity(nOpacity);

			m_dwLoginTimer = dwCurrTime + 9;
		}

		if (pPicture->GetOpacity() == 0)
		{
			OnLoginDestroy();

			m_nLoginState = LoginState::Standby;
			m_nState = GUNZ_CHARSELECTION;
			OnCharSelectionCreate();
		}
	}
}

void ZGameInterface::OnDrawStateLobbyNStage(MDrawContext* pDC)
{
	ZIDLResource* pRes = ZApplication::GetGameInterface()->GetIDLResource();

	DWORD dwClock = GetGlobalTimeMS();
	if ((dwClock - m_dwFrameMoveClock) < 30)
		return;
	m_dwFrameMoveClock = dwClock;


	if (GetState() == GUNZ_LOBBY)
	{
		// Lobby
		char buf[512];
		MLabel* pLabel = (MLabel*)pRes->FindWidget("Lobby_PlayerName");
		if (pLabel)
		{
			sprintf_safe(buf, "%s", ZGetMyInfo()->GetCharName());
			pLabel->SetText(buf);
		}
		// Clan
		pLabel = (MLabel*)pRes->FindWidget("Lobby_PlayerSpecClan");
		sprintf_safe(buf, "%s : %s", ZMsg(MSG_CHARINFO_CLAN), ZGetMyInfo()->GetClanName());
		if (pLabel) pLabel->SetText(buf);
		// LV
		pLabel = (MLabel*)pRes->FindWidget("Lobby_PlayerSpecLevel");
		sprintf_safe(buf, "%s : %d %s", ZMsg(MSG_CHARINFO_LEVEL), ZGetMyInfo()->GetLevel(), ZMsg(MSG_CHARINFO_LEVELMARKER));
		if (pLabel) pLabel->SetText(buf);
		// XP
		pLabel = (MLabel*)pRes->FindWidget("Lobby_PlayerSpecXP");
		sprintf_safe(buf, "%s : %d%%", ZMsg(MSG_CHARINFO_XP), ZGetMyInfo()->GetLevelPercent());
		if (pLabel) pLabel->SetText(buf);
		// BP
		pLabel = (MLabel*)pRes->FindWidget("Lobby_PlayerSpecBP");
		sprintf_safe(buf, "%s : %d", ZMsg(MSG_CHARINFO_BOUNTY), ZGetMyInfo()->GetBP());
		if (pLabel) pLabel->SetText(buf);
		// HP
		pLabel = (MLabel*)pRes->FindWidget("Lobby_PlayerSpecHP");
		sprintf_safe(buf, "%s : %d", ZMsg(MSG_CHARINFO_HP), ZGetMyInfo()->GetHP());
		if (pLabel) pLabel->SetText(buf);
		// AP
		pLabel = (MLabel*)pRes->FindWidget("Lobby_PlayerSpecAP");
		sprintf_safe(buf, "%s : %d", ZMsg(MSG_CHARINFO_AP), ZGetMyInfo()->GetAP());
		if (pLabel) pLabel->SetText(buf);
		// WT
		pLabel = (MLabel*)pRes->FindWidget("Lobby_PlayerSpecWT");
		ZMyItemList* pItems = ZGetMyInfo()->GetItemList();
		sprintf_safe(buf, "%s : %d/%d", ZMsg(MSG_CHARINFO_WEIGHT), pItems->GetEquipedTotalWeight(), pItems->GetMaxWeight());
		if (pLabel) pLabel->SetText(buf);

		pLabel = (MLabel*)pRes->FindWidget("Lobby_ChannelName");
		sprintf_safe(buf, "%s > %s > %s", ZGetGameClient()->GetServerName(), ZMsg(MSG_WORD_LOBBY), ZGetGameClient()->GetChannelName());
		if (pLabel)
			pLabel->SetText(buf);
	}

	// Stage
	else if (GetState() == GUNZ_STAGE)
	{
		char buf[512];
		MListBox* pListBox = (MListBox*)pRes->FindWidget("StagePlayerList_");
		bool bShowMe = true;
		if (pListBox)
		{
			ZStagePlayerListItem* pItem = NULL;
			if (pListBox->GetSelIndex() < pListBox->GetCount())
			{
				if (pListBox->GetSelIndex() >= 0)
					pItem = (ZStagePlayerListItem*)pListBox->Get(pListBox->GetSelIndex());

				if ((pListBox->GetSelIndex() != -1) && (strcmp(ZGetMyInfo()->GetCharName(), pItem->GetString(2)) != 0))
					bShowMe = false;
			}

			MLabel* pLabel = (MLabel*)pRes->FindWidget("Stage_PlayerName");
			if (bShowMe)
				sprintf_safe(buf, "%s", ZGetMyInfo()->GetCharName());
			else
				sprintf_safe(buf, "%s", pItem->GetString(2));
			if (pLabel) pLabel->SetText(buf);

			pLabel = (MLabel*)pRes->FindWidget("Stage_PlayerSpecClan");
			if (bShowMe)
				sprintf_safe(buf, "%s : %s", ZMsg(MSG_CHARINFO_CLAN), ZGetMyInfo()->GetClanName());
			else
			{
				if (strcmp(pItem->GetString(4), "") == 0)
					sprintf_safe(buf, "%s :", ZMsg(MSG_CHARINFO_CLAN));
				else
					sprintf_safe(buf, "%s : %s", ZMsg(MSG_CHARINFO_CLAN), pItem->GetString(4));
			}
			if (pLabel) pLabel->SetText(buf);

			pLabel = (MLabel*)pRes->FindWidget("Stage_PlayerSpecLevel");
			if (bShowMe)
				sprintf_safe(buf, "%s : %d %s", ZMsg(MSG_CHARINFO_LEVEL), ZGetMyInfo()->GetLevel(), ZMsg(MSG_CHARINFO_LEVELMARKER));
			else
				sprintf_safe(buf, "%s : %s %s", ZMsg(MSG_CHARINFO_LEVEL), pItem->GetString(1), ZMsg(MSG_CHARINFO_LEVELMARKER));
			if (pLabel) pLabel->SetText(buf);

			// XP
			pLabel = (MLabel*)pRes->FindWidget("Stage_PlayerSpecXP");
			if (bShowMe)
				sprintf_safe(buf, "%s : %d%%", ZMsg(MSG_CHARINFO_XP), ZGetMyInfo()->GetLevelPercent());
			else
				sprintf_safe(buf, "%s : -", ZMsg(MSG_CHARINFO_XP));
			if (pLabel)
			{
				pLabel->SetText(buf);
				pLabel->Enable(bShowMe);
			}

			// BP
			pLabel = (MLabel*)pRes->FindWidget("Stage_PlayerSpecBP");
			if (bShowMe)
				sprintf_safe(buf, "%s : %d", ZMsg(MSG_CHARINFO_BOUNTY), ZGetMyInfo()->GetBP());
			else
				sprintf_safe(buf, "%s : -", ZMsg(MSG_CHARINFO_BOUNTY));
			if (pLabel)
			{
				pLabel->SetText(buf);
				pLabel->Enable(bShowMe);
			}

			// HP
			pLabel = (MLabel*)pRes->FindWidget("Stage_PlayerSpecHP");
			if (bShowMe)
				sprintf_safe(buf, "%s : %d", ZMsg(MSG_CHARINFO_HP), ZGetMyInfo()->GetHP());
			else
				sprintf_safe(buf, "%s : -", ZMsg(MSG_CHARINFO_HP));
			if (pLabel)
			{
				pLabel->SetText(buf);
				pLabel->Enable(bShowMe);
			}

			// AP
			pLabel = (MLabel*)pRes->FindWidget("Stage_PlayerSpecAP");
			if (bShowMe)
				sprintf_safe(buf, "%s : %d", ZMsg(MSG_CHARINFO_AP), ZGetMyInfo()->GetAP());
			else
				sprintf_safe(buf, "%s : -", ZMsg(MSG_CHARINFO_AP));
			if (pLabel)
			{
				pLabel->SetText(buf);
				pLabel->Enable(bShowMe);
			}

			// WT
			pLabel = (MLabel*)pRes->FindWidget("Stage_PlayerSpecWT");
			ZMyItemList* pItems = ZGetMyInfo()->GetItemList();
			if (bShowMe)
				sprintf_safe(buf, "%s : %d/%d", ZMsg(MSG_CHARINFO_WEIGHT),
					pItems->GetEquipedTotalWeight(), pItems->GetMaxWeight());
			else
				sprintf_safe(buf, "%s : -", ZMsg(MSG_CHARINFO_WEIGHT));
			if (pLabel)
			{
				pLabel->SetText(buf);
				pLabel->Enable(bShowMe);
			}
		}

		ZApplication::GetStageInterface()->OnDrawStartMovieOfQuest();

		int nOpacity = 90.0f * (sin(GetGlobalTimeMS() * 0.003f) + 1) + 75;

		MLabel* pLabel = (MLabel*)pRes->FindWidget("Stage_SenarioName");
		MPicture* pPicture = (MPicture*)pRes->FindWidget("Stage_Lights0");
		if (pPicture)
		{
			pPicture->SetOpacity(nOpacity);
		}
		pPicture = (MPicture*)pRes->FindWidget("Stage_Lights1");
		if (pPicture)
		{
			pPicture->SetOpacity(nOpacity);
		}

		MWidget* pWidget = pRes->FindWidget("Stage_ItemListView");
		if (!pWidget)
			return;

		int nEndPos = ZApplication::GetStageInterface()->m_nListFramePos;
		MRECT rect = pWidget->GetRect();
		if (rect.x == nEndPos)
			return;

		int nNewPos = rect.x + (nEndPos - rect.x) * 0.25;
		if (nNewPos == rect.x)		// not changed
			rect.x = nEndPos;
		else						// changed
			rect.x = nNewPos;

		pWidget->SetBounds(rect);

		if (rect.x == 0)
		{
			pWidget = pRes->FindWidget("Stage_CharacterInfo");
			if (pWidget)
				pWidget->Enable(false);
		}
	}
}

void ZGameInterface::OnDrawStateCharSelection(MDrawContext* pDC)
{
	if (m_pBackground && m_pCharacterSelectView)
	{
		m_pBackground->LoadMesh();
		m_pBackground->Draw();
		m_pCharacterSelectView->Draw();

		// Draw effects (smoke, cloud)
		ZGetEffectManager()->Draw(GetGlobalTimeMS());

		// Draw maiet logo effect
		ZGetScreenEffectManager()->DrawEffects();
	}
}

void ZGameInterface::OnDraw(MDrawContext *pDC)
{
	m_nDrawCount++;

	__BP(11, "ZGameInterface::OnDraw");

	if (m_bLoading)
	{
		__EP(11);
		return;
	}

#ifdef _BIRDTEST
	if (GetState() == GUNZ_BIRDTEST)
	{
		OnBirdTestDraw();
		__EP(11);
		return;
	}
#endif

	switch (GetState())
	{
	case GUNZ_GAME:
	{
		OnDrawStateGame(pDC);
	}
	break;
	case GUNZ_LOGIN:
	case GUNZ_NETMARBLELOGIN:
	{
		OnDrawStateLogin(pDC);
	}
	break;
	case GUNZ_LOBBY:
	case GUNZ_STAGE:
	{
		OnDrawStateLobbyNStage(pDC);
		if (GetState() == GUNZ_LOBBY)
			GetRGMain().OnDrawLobby(pDC);
	}
	break;
	case GUNZ_CHARSELECTION:
	case GUNZ_CHARCREATION:
	{
		OnDrawStateCharSelection(pDC);
	}
	break;
	}

	__EP(11);
}

void ZGameInterface::TestChangePartsAll()
{
}

void ZGameInterface::TestChangeParts(int mode) {

#ifndef _PUBLISH
	RMeshPartsType ptype = eq_parts_etc;

	if (mode == 0) { ptype = eq_parts_chest; }
	else if (mode == 1) { ptype = eq_parts_head; }
	else if (mode == 2) { ptype = eq_parts_hands; }
	else if (mode == 3) { ptype = eq_parts_legs; }
	else if (mode == 4) { ptype = eq_parts_feet; }
	else if (mode == 5) { ptype = eq_parts_face; }

	ZPostChangeParts(ptype, 1);

#endif

}

void ZGameInterface::TestToggleCharacter()
{
	ZPostChangeCharacter();
}

void ZGameInterface::TestChangeWeapon(RVisualMesh* pVMesh)
{
	static int nWeaponIndex = 0;

	int nItemID = 0;
	switch (nWeaponIndex)
	{
	case 0:
		nItemID = 1;		// katana
		break;
	case 1:
		nItemID = 5;		// dagger
		break;
	case 2:
		nItemID = 2;		// double pistol
		break;
	case 3:
		nItemID = 3;		// SMG
		break;
	case 4:
		nItemID = 6;		// shotgun
		break;
	case 5:
		nItemID = 7;		// Rocket
		break;
	case 6:
		nItemID = 4;		// grenade
		break;
	}


	if (GetState() == GUNZ_GAME)
	{
		if (m_pMyCharacter == NULL) return;


		switch (nWeaponIndex)
		{
		case 0:
		case 1:
			m_pMyCharacter->GetItems()->EquipItem(MMCIP_MELEE, nItemID);		// dagger
			m_pMyCharacter->ChangeWeapon(MMCIP_MELEE);
			break;
		case 2:
		case 3:
		case 4:
		case 5:
			m_pMyCharacter->GetItems()->EquipItem(MMCIP_PRIMARY, nItemID);		// Rocket
			m_pMyCharacter->ChangeWeapon(MMCIP_PRIMARY);
			break;
		case 6:
			m_pMyCharacter->GetItems()->EquipItem(MMCIP_CUSTOM1, nItemID);		// grenade
			m_pMyCharacter->ChangeWeapon(MMCIP_CUSTOM1);
			break;
		}

	}
	else if (GetState() == GUNZ_CHARSELECTION)
	{
		if (pVMesh != NULL)
		{
			ZChangeCharWeaponMesh(pVMesh, nItemID);
			pVMesh->SetAnimation("login_intro");
			pVMesh->GetFrameInfo(ani_mode_lower)->m_nFrame = 0;
		}
	}
	else if (GetState() == GUNZ_LOBBY)
	{
		if (pVMesh != NULL)
		{
			ZChangeCharWeaponMesh(pVMesh, nItemID);
		}
	}


	nWeaponIndex++;
	if (nWeaponIndex >= 7) nWeaponIndex = 0;
}

void ZGameInterface::RespawnMyCharacter()
{
	if (ZApplication::GetGame() == NULL) return;

	m_pMyCharacter->Revival();
	rvector pos = rvector(0, 0, 0), dir = rvector(0, 1, 0);

	ZMapSpawnData* pSpawnData = ZApplication::GetGame()->GetMapDesc()->GetSpawnManager()->GetSoloRandomData();
	if (pSpawnData != NULL)
	{
		pos = pSpawnData->m_Pos;
		dir = pSpawnData->m_Dir;
	}

	m_pMyCharacter->SetPosition(pos);
	m_pMyCharacter->SetDirection(dir);
}

bool ZGameInterface::OnGlobalEvent(MEvent* pEvent)
{
	if ((ZGetGameInterface()->GetState() == GUNZ_GAME))
		return ZGameInput::OnEvent(pEvent);

#ifndef _PUBLISH
	switch (pEvent->nMessage) {
	case MWM_CHAR:
	{
		switch (pEvent->nKey) {
		case '`':
			if (ZApplication::GetInstance()->GetLaunchMode() == ZApplication::ZLAUNCH_MODE_STANDALONE_DEVELOP)
			{
				ZGetConsole()->Show(!ZGetConsole()->IsVisible());
				ZGetConsole()->SetZOrder(MZ_TOP);
			}
			break;
		}
	}break;
	}
#endif
	return false;
}

bool ZGameInterface::OnDebugEvent(MEvent* pEvent, MListener* pListener)
{
	switch (pEvent->nMessage) {
	case MWM_KEYDOWN:
	{
		switch (pEvent->nKey)
		{
		case VK_F10:
			m_pLogFrame->Show(!m_pLogFrame->IsVisible());
			return true;
		case VK_NUMPAD8:
		{
			if (GetState() == GUNZ_LOBBY)
			{
				if (ZGetCharacterViewList(GUNZ_LOBBY) != NULL)
				{
					RVisualMesh* pVMesh =
						ZGetCharacterViewList(GUNZ_LOBBY)->Get(ZGetGameClient()->GetPlayerUID())->GetVisualMesh();

					TestChangeWeapon(pVMesh);
				}
			}

		}
		break;
		}
	}
	break;
	}
	return false;
}

bool ZGameInterface::OnEvent(MEvent* pEvent, MListener* pListener)
{
#ifndef _PUBLISH
	if (OnDebugEvent(pEvent, pListener)) return true;
#endif

	return false;
}

bool ZGameInterface::OnCommand(MWidget* pWidget, const char* szMessage)
{
	if (pWidget == m_pPlayerMenu) {
		MMenuItem* pItem = (MMenuItem*)pWidget;


		OutputDebugString("PLAYERMENU");
	}
	return false;
}

void ZGameInterface::ChangeParts(int key)
{
}

void ZGameInterface::ChangeWeapon(ZChangeWeaponType nType)
{
	ZMyCharacter* pChar = g_pGame->m_pMyCharacter;

	if (pChar->m_pVMesh == NULL) return;

	if (pChar->IsDead()) return;

	if (m_pGame->GetMatch()->IsRuleGladiator() && !pChar->IsAdmin())
		return;

	int nParts = -1;

	bool bWheel = false;

	auto IsOutOfAmmo = [&](int PartsInt)
	{
		auto Parts = MMatchCharItemParts(PartsInt);
		auto* Item = g_pGame->m_pMyCharacter->GetItems()->GetItem(Parts);
		if (!Item)
			return true;

		return Item->GetBulletAMagazine() <= 0;
	};

	if (nType == ZCWT_PREV || nType == ZCWT_NEXT)
	{
		bWheel = true;

		int nHasItemCount = 0;
		int nPos = -1;
		int ItemQueue[int(MMCIP_END) - int(MMCIP_MELEE)]{};

		for (int i = MMCIP_MELEE; i < MMCIP_END; i++)
		{
			if (pChar->GetItems()->GetItem((MMatchCharItemParts)i)->IsEmpty())
				continue;

			if ((i == MMCIP_CUSTOM1 || i == MMCIP_CUSTOM2) && IsOutOfAmmo(i))
				continue;

			if (pChar->GetItems()->GetSelectedWeaponParts() == i)
				nPos = nHasItemCount;

			ItemQueue[nHasItemCount++] = i;
		}

		if (nPos < 0 || nHasItemCount <= 1)
			return;

		auto Offset = nType == ZCWT_PREV ? -1 : 1;
		auto NewPos = mod(nPos + Offset, nHasItemCount);
		nParts = ItemQueue[NewPos];
	}
	else
	{
		switch (nType)
		{
		case ZCWT_MELEE:
			nParts = int(MMCIP_MELEE);
			break;
		case ZCWT_PRIMARY:
			nParts = int(MMCIP_PRIMARY);
			break;
		case ZCWT_SECONDARY:
			nParts = int(MMCIP_SECONDARY);
			break;
		case ZCWT_CUSTOM1:
			nParts = int(MMCIP_CUSTOM1);
			break;
		case ZCWT_CUSTOM2:
			nParts = int(MMCIP_CUSTOM2);
			break;
		}

		if (nParts == MMCIP_CUSTOM1 || nParts == MMCIP_CUSTOM2) {
			if (IsOutOfAmmo(nParts))
				return;
		}
	}

	if (nParts < 0) return;
	if (pChar->GetItems()->GetSelectedWeaponParts() == nParts) return;

	if (bWheel &&
		!ZGetConfiguration()->FastWeaponCycle &&
		(pChar->GetStateUpper() == ZC_STATE_UPPER_LOAD && pChar->IsUpperPlayDone() == false))
		return;

	if (pChar->m_bWallHang || pChar->m_bShot || pChar->m_bShotReturn || pChar->m_bTumble
		|| pChar->m_bSkill || pChar->m_bGuard || pChar->m_bBlast || pChar->m_bBlastFall
		|| pChar->m_bBlastDrop || pChar->m_bBlastStand || pChar->m_bBlastAirmove
		|| pChar->m_bSlash || pChar->m_bJumpSlash || pChar->m_bJumpSlashLanding
		|| (pChar->GetStateUpper() == ZC_STATE_UPPER_SHOT && pChar->IsUpperPlayDone() == false))
	{
		m_bReservedWeapon = true;
		m_ReservedWeapon = nType;
		return;
	}

	m_bReservedWeapon = false;
	ZPostChangeWeapon(nParts);

	m_pMyCharacter->m_bSpMotion = false;

	m_pMyCharacter->ChangeWeapon((MMatchCharItemParts)nParts);
}

void ZGameInterface::OnGameUpdate(float fElapsed)
{
	__BP(12, "ZGameInterface::OnGameUpdate");
	if (m_pGame == NULL) return;

	if (m_pGameInput) m_pGameInput->Update(fElapsed);

	m_pGame->Update(fElapsed);

	if (m_pCombatInterface) m_pCombatInterface->Update();

	if (m_bReservedWeapon)
		ChangeWeapon(m_ReservedWeapon);

	__EP(12);
}


void ZGameInterface::OnReplay()
{
	ShowWidget("ReplayConfirm", false);

	CreateReplayGame(nullptr);
}

bool ZGameInterface::Update(float fElapsed)
{
	if (m_pBackground && ((GetState() == GUNZ_CHARSELECTION) || (GetState() == GUNZ_CHARCREATION)))
		m_pBackground->OnUpdate(fElapsed);

	if (GetState() == GUNZ_LOBBY)
	{
		if (ZGetApplication()->GetLaunchMode() == ZApplication::ZLAUNCH_MODE_STANDALONE_REPLAY)
		{
			ShowWidget("Lobby", false);
			ShowWidget("ReplayConfirm", true);
			return false;
		}
	}

	__BP(13, "ZGameInterface::Update");

	ZGetOptionInterface()->Update();

	__BP(14, "ZGameInterface::GameClient Run & update");
	if (g_pGameClient != NULL) g_pGameClient->Run();
	g_pGameClient->Tick();
	__EP(14);

	if (!m_bLoading) {
		if (GetState() == GUNZ_GAME) {
			OnGameUpdate(fElapsed);
		}
		else {
#ifdef _BIRDTEST
			if (GetState() == GUNZ_BIRDTEST) OnBirdTestUpdate();
#endif
		}
	}

	if (GetState() == GUNZ_LOBBY && m_bWaitingArrangedGame) {
		MLabel *pLabel = (MLabel*)m_IDLResource.FindWidget("LobbyWaitingArrangedGameLabel");
		if (pLabel) {
			int nCount = (GetGlobalTimeMS() / 500) % 5;
			char dots[10];
			for (int i = 0; i < nCount; i++) {
				dots[i] = '.';
			}
			dots[nCount] = 0;

			char szBuffer[256];
			sprintf_safe(szBuffer, "%s%s", ZMsg(MSG_WORD_FINDTEAM), dots);
			pLabel->SetText(szBuffer);
		}
	}

	UpdateCursorEnable();

	if (g_pGame != NULL && m_bLeaveBattleReserved && (m_dwLeaveBattleTime < GetGlobalTimeMS()))
		LeaveBattle();

	__EP(13);

	return true;
}

void ZGameInterface::OnResetCursor()
{
	SetCursorEnable(m_bCursor);
}

void ZGameInterface::SetCursorEnable(bool bEnable)
{
	if (m_bCursor == bEnable) return;

	m_bCursor = bEnable;
	MCursorSystem::Show(bEnable);
}

void ZGameInterface::UpdateCursorEnable()
{

	if (GetState() != GUNZ_GAME ||
		(GetCombatInterface() && GetCombatInterface()->IsShowResult()) ||
		IsMenuVisible() ||
		m_pMsgBox->IsVisible() ||
		m_pConfirmMsgBox->IsVisible() ||
		GetRGMain().IsCursorEnabled()
		)
		SetCursorEnable(true);
	else
	{
		MWidget* pWidget = m_IDLResource.FindWidget("112Confirm");
		if (pWidget && pWidget->IsVisible())
			SetCursorEnable(true);

		else
			SetCursorEnable(false);
	}
}

void ZGameInterface::SetMapThumbnail(const char* szMapName)
{
	SAFE_DELETE(m_pThumbnailBitmap);

	char szThumbnail[256];
	sprintf_safe(szThumbnail, "maps/%s/%s.rs.bmp", szMapName, szMapName);

	m_pThumbnailBitmap = Mint::GetInstance()->OpenBitmap(szThumbnail);
	if (!m_pThumbnailBitmap)
	{
		sprintf_safe(szThumbnail, "maps/%s/%s.bmp", szMapName, szMapName);
		m_pThumbnailBitmap = Mint::GetInstance()->OpenBitmap(szThumbnail);
	}
}

void ZGameInterface::ClearMapThumbnail()
{
	SAFE_DELETE(m_pThumbnailBitmap);
}

void ZGameInterface::Reload()
{
	if (!g_pGame->m_pMyCharacter->GetItems()->GetSelectedWeapon()) return;
	MMatchItemDesc* pSelectedItemDesc = g_pGame->m_pMyCharacter->GetItems()->GetSelectedWeapon()->GetDesc();

	if (pSelectedItemDesc == NULL) return;

	if (pSelectedItemDesc->m_nType != MMIT_RANGE)  return;

	if (g_pGame->m_pMyCharacter->GetItems()->GetSelectedWeapon()->isReloadable() == false) return;

	ZMyCharacter* pChar = g_pGame->m_pMyCharacter;

	if (pChar->m_bBlast ||
		pChar->m_bBlastFall ||
		pChar->m_bBlastDrop ||
		pChar->m_bBlastStand ||
		pChar->m_bBlastAirmove)
		return;

	if (pChar->GetStateUpper() == ZC_STATE_UPPER_RELOAD) return;

	pChar->m_bSpMotion = false;

	ZPostReload();
}

void ZGameInterface::OnScreenshot()
{
	if (ZGetConfiguration()->AsyncScreenshots)
	{
		if (!ScreenshotPixelBuffer)
		{
			ScreenshotQueued = true;
		}
	}
	else
	{
		SaveScreenshot(true);
	}
}

void ZGameInterface::SaveScreenshotIfQueued()
{
	if (!ScreenshotQueued)
		return;

	ScreenshotQueued = false;

	SaveScreenshot(false);
}

template <size_t Size>
static int GetScreenshotFilename(char(&Output)[Size])
{
	std::string ScreenshotFolderName = GetMyDocumentsPath();
	ScreenshotFolderName += GUNZ_FOLDER;
	ScreenshotFolderName += SCREENSHOT_FOLDER;
	MFile::CreateDir(ScreenshotFolderName);
	//MakePath(ScreenshotFolderName.c_str());

	int ScreenshotCount = -1;
	char ScreenshotFilenameWithExtension[MFile::MaxPath];

	auto ExistsWithWildcard = [](const char* Spec)
	{
		auto range = MFile::Glob(Spec);
		return !range.empty();
	};

	do {
		ScreenshotCount++;
		sprintf_safe(ScreenshotFilenameWithExtension, "%s/GunZ%03d.*", ScreenshotFolderName.c_str(), ScreenshotCount);
	} while (ExistsWithWildcard(ScreenshotFilenameWithExtension));

	sprintf_safe(Output, "%s/GunZ%03d", ScreenshotFolderName.c_str(), ScreenshotCount);

	return ScreenshotCount;
}

static void NotifyFailedScreenshot()
{
	ZChatOutput(MCOLOR(ZCOLOR_CHAT_SYSTEM),
		ZMsg(MSG_SCREENSHOT_CANT_SAVE));
}

static void NotifyScreenshotStatus(bool ScreenshotSucceeded, int ScreenshotCount)
{
	ZGetGameInterface()->ScreenshotPixelBuffer.reset();

	if (ScreenshotSucceeded)
	{
		auto* Extension = GetScreenshotFormatExtension(ZGetConfiguration()->ScreenshotFormat);

		char szOutputFilename[512];
		sprintf_safe(szOutputFilename, GUNZ_FOLDER SCREENSHOT_FOLDER"/GunZ%03d.%s",
			ScreenshotCount, Extension);

		char szOutput[512];
		ZTransMsg(szOutput, MSG_SCREENSHOT_SAVED, 1, szOutputFilename);
		ZChatOutput(MCOLOR(ZCOLOR_CHAT_SYSTEM), szOutput);
	}
	else
	{
		NotifyFailedScreenshot();
	}
};

#define FAIL() do { NotifyFailedScreenshot(); return; } while (false)
#define V(expr) do { if (DXERR(expr)) FAIL(); } while (false)

void ZGameInterface::SaveScreenshot(bool Sync)
{
	char ScreenshotFilename[MFile::MaxPath];
	auto ScreenshotCount = GetScreenshotFilename(ScreenshotFilename);

	if (ScreenshotCount < 0)
	{
		MLog("ZGameInterface::SaveScreenshot -- Failed to get screenshot filename, "
			"ScreenshotCount = %d\n", ScreenshotCount);
		FAIL();
	}

	auto* pDevice = RGetDevice();

	D3DPtr<IDirect3DSurface9> Surface;

	if (Sync)
	{
		V(pDevice->CreateOffscreenPlainSurface(RGetScreenWidth(), RGetScreenHeight(),
			D3DFMT_A8R8G8B8, D3DPOOL_SCRATCH, MakeWriteProxy(Surface), NULL));
		V(pDevice->GetFrontBufferData(0, Surface.get()));
	}
	else
	{
		V(pDevice->CreateOffscreenPlainSurface(RGetScreenWidth(), RGetScreenHeight(),
			RGetPixelFormat(), D3DPOOL_SYSTEMMEM, MakeWriteProxy(Surface), NULL));

		D3DPtr<IDirect3DSurface9> RenderTarget;
		V(pDevice->GetRenderTarget(0, MakeWriteProxy(RenderTarget)));
		V(pDevice->GetRenderTargetData(RenderTarget.get(), Surface.get()));
	}

	RECT rt;
	GetWindowRect(g_hWnd, &rt);

	D3DLOCKED_RECT Rect;
	V(Surface->LockRect(&Rect, &rt, NULL));

	int nWidth = rt.right - rt.left;
	int nHeight = rt.bottom - rt.top;

	auto* TexturePtr = static_cast<u8*>(Rect.pBits);

	ScreenshotPixelBuffer = std::make_unique<u8[]>(nWidth * nHeight * 4);
	auto ScreenshotPixelBufferPtr = ScreenshotPixelBuffer.get();

	for (int i = 0; i < nHeight; i++)
	{
		memcpy(ScreenshotPixelBufferPtr + i * nWidth * 4, TexturePtr + Rect.Pitch * i, nWidth * 4);
	}

	V(Surface->UnlockRect());

	if (Sync)
	{
		bool ScreenshotSucceeded = RScreenShot(nWidth, nHeight,
			ScreenshotPixelBuffer.get(), ScreenshotFilename,
			ZGetConfiguration()->ScreenshotFormat);

		NotifyScreenshotStatus(ScreenshotSucceeded, ScreenshotCount);
	}
	else
	{
		auto AsyncProc = [nWidth, nHeight, ScreenshotCount,
			Filename = std::string{ ScreenshotFilename }]
		{
			auto* Interface = ZGetGameInterface();

			bool ScreenshotSucceeded = Interface && RScreenShot(nWidth, nHeight,
				ZGetGameInterface()->ScreenshotPixelBuffer.get(), Filename.c_str(),
				ZGetConfiguration()->ScreenshotFormat);

			auto MainThreadProc = [ScreenshotSucceeded, ScreenshotCount]
			{
				NotifyScreenshotStatus(ScreenshotSucceeded, ScreenshotCount);
			};
			GetRGMain().Invoke(MainThreadProc);
		};

		TaskManager::GetInstance().AddTask(std::move(AsyncProc));
	}
}

#undef V
#undef FAIL

void ZGameInterface::Sell()
{
	MButton* pButton = (MButton*)m_IDLResource.FindWidget("SellConfirmCaller");
	if (pButton)
	{
		pButton->Enable(false);
		pButton->Show(false);
		pButton->Show(true);
	}

	ZEquipmentListBox* pListBox = (ZEquipmentListBox*)m_IDLResource.FindWidget("MyAllEquipmentList");
	if (!pListBox)
		return;

	if (pListBox->IsSelected())
	{
		ZEquipmentListItem* pListItem = (ZEquipmentListItem*)pListBox->GetSelItem();
		if (pListItem == NULL)
		{
			ZApplication::GetGameInterface()->ShowErrorMessage(MERR_NO_SELITEM);
			return;
		}

		MUID uidItem = pListItem->GetUID();

		ZPostRequestSellItem(ZGetGameClient()->GetPlayerUID(), uidItem);
		ZPostRequestCharacterItemList(ZGetGameClient()->GetPlayerUID());
	}
	else
	{
		ZApplication::GetGameInterface()->ShowErrorMessage(MERR_NO_SELITEM);
	}
}


void ZGameInterface::SellQuestItem()
{
	ZEquipmentListBox* pEquipmentListBox = (ZEquipmentListBox*)m_IDLResource.FindWidget("MyAllEquipmentList");
	if (pEquipmentListBox)
	{
		ZEquipmentListItem* pListItem = (ZEquipmentListItem*)pEquipmentListBox->GetSelItem();
		if (pListItem)
		{
			ZPostRequestSellQuestItem(ZGetGameClient()->GetPlayerUID(), pListItem->GetItemID(), m_nSellQuestItemCount);

			MWidget* pWidget = m_IDLResource.FindWidget("SellQuestItemConfirmCaller");
			if (pWidget)
			{
				pWidget->Show(false);
				pWidget->Enable(false);
				pWidget->Show(true);
			}
		}
	}

	MWidget* pWidget = m_IDLResource.FindWidget("Shop_SellQuestItem");
	if (pWidget)
		pWidget->Show(false);
}

void ZGameInterface::SetSellQuestItemConfirmFrame()
{
	ZEquipmentListBox* pEquipmentListBox = (ZEquipmentListBox*)m_IDLResource.FindWidget("MyAllEquipmentList");
	if (pEquipmentListBox)
	{
		ZEquipmentListItem* pListItem = (ZEquipmentListItem*)pEquipmentListBox->GetSelItem();
		if (pListItem)
		{
			MPicture* pPicture = (MPicture*)m_IDLResource.FindWidget("SellQuestItem_ItemIcon");
			if (pPicture)
				pPicture->SetBitmap(pListItem->GetBitmap(0));


			ZMyQuestItemNode* pQuestItemNode = ZGetMyInfo()->GetItemList()->GetQuestItemMap().Find(pListItem->GetItemID());
			if (pQuestItemNode)
			{
				if (m_nSellQuestItemCount > pQuestItemNode->m_nCount)
					m_nSellQuestItemCount = pQuestItemNode->m_nCount;
			}

			MQuestItemDesc* pQuestItemDesc = pQuestItemNode->GetDesc();

			MLabel* pLabel = (MLabel*)m_IDLResource.FindWidget("SellQuestItem_Calculate");
			if (pLabel && pQuestItemDesc)
			{
				char szText[128];
				sprintf_safe(szText, "%s(%d bounty) x %d", pQuestItemDesc->m_szQuestItemName,
					(int)(pQuestItemDesc->m_nPrice * 0.25),
					m_nSellQuestItemCount);
				pLabel->SetText(szText);
			}
			pLabel = (MLabel*)m_IDLResource.FindWidget("SellQuestItem_Total");
			if (pLabel && pQuestItemDesc)
			{
				char szText[128];
				sprintf_safe(szText, "= %d bounty", (int)(pQuestItemDesc->m_nPrice * 0.25) * m_nSellQuestItemCount);
				pLabel->SetText(szText);
			}
		}
	}


	MLabel* pLabel = (MLabel*)m_IDLResource.FindWidget("SellQuestItem_CountNum");
	if (pLabel)
	{
		char szText[128];
		sprintf_safe(szText, "%d", m_nSellQuestItemCount);
		pLabel->SetAlignment(MAM_RIGHT | MAM_VCENTER);
		pLabel->SetText(szText);
	}
}

void ZGameInterface::OpenSellQuestItemConfirm()
{
	m_nSellQuestItemCount = 1;

	SetSellQuestItemConfirmFrame();

	MWidget* pWidget = m_IDLResource.FindWidget("Shop_SellQuestItem");
	if (pWidget)
		pWidget->Show(true, true);
}

void ZGameInterface::SellQuestItemCountUp()
{
	m_nSellQuestItemCount++;

	SetSellQuestItemConfirmFrame();
}

void ZGameInterface::SellQuestItemCountDn()
{
	if (m_nSellQuestItemCount > 1)
		m_nSellQuestItemCount--;

	SetSellQuestItemConfirmFrame();
}

void ZGameInterface::Buy()
{
	MButton* pButton = (MButton*)m_IDLResource.FindWidget("BuyConfirmCaller");
	if (pButton)
	{
		pButton->Enable(false);
		pButton->Show(false);
		pButton->Show(true);
	}
	u32 nItemID = 0;
	ZEquipmentListBox* pListBox = (ZEquipmentListBox*)m_IDLResource.FindWidget("AllEquipmentList");
	if (pListBox == NULL)
		return;

	if (pListBox->IsSelected())
	{
		ZEquipmentListItem* pListItem = (ZEquipmentListItem*)pListBox->GetSelItem();
		if (pListItem == NULL)
		{
			ZApplication::GetGameInterface()->ShowErrorMessage(MERR_NO_SELITEM);
			return;
		}

		MUID uidItem = pListItem->GetUID();
		nItemID = ZGetShop()->GetItemID(pListItem->GetUID().Low - 1);

		MMatchItemDesc* pItemDesc = MGetMatchItemDescMgr()->GetItemDesc(nItemID);

#ifdef _QUEST_ITEM
		if (0 == pItemDesc)
		{
			MQuestItemDesc* pQuestItemDesc = GetQuestItemDescMgr().FindQItemDesc(nItemID);
			if (0 == pQuestItemDesc)
			{
				return;
			}

			ZPostRequestBuyQuestItem(ZGetGameClient()->GetPlayerUID(), nItemID);
			return;
		}
#endif
		if (pItemDesc->IsCashItem())
		{
			TCHAR szURL[256];
			sprintf_safe(szURL, "explorer.exe \"%s\"", Z_LOCALE_CASHSHOP_URL);
			WinExec(szURL, SW_SHOWNORMAL);
		}
		else
		{
			ZPostRequestBuyItem(ZGetGameClient()->GetPlayerUID(), nItemID);
			ZPostRequestCharacterItemList(ZGetGameClient()->GetPlayerUID());
		}
	}
	else
	{
		ZApplication::GetGameInterface()->ShowErrorMessage(MERR_NO_SELITEM);
	}
}


void ZGameInterface::BuyCashItem()
{
	MButton* pButton = (MButton*)m_IDLResource.FindWidget("BuyCashConfirmCaller");
	if (pButton)
	{
		pButton->Enable(false);
		pButton->Show(false);
		pButton->Show(true);
	}

	TCHAR szURL[256];
	sprintf_safe(szURL, "explorer.exe \"%s\"", Z_LOCALE_CASHSHOP_URL);
	WinExec(szURL, SW_SHOWNORMAL);
}


bool ZGameInterface::Equip()
{
	MUID uidItem;
	MMatchCharItemParts parts = MMCIP_END;

	ZEquipmentListBox* pListBox = (ZEquipmentListBox*)m_IDLResource.FindWidget("EquipmentList");
	if (pListBox == NULL) return false;

	if (pListBox->IsSelected())
	{
		ZEquipmentListItem* pListItem = (ZEquipmentListItem*)pListBox->GetSelItem();
		if (pListItem == NULL)
		{
			ZApplication::GetGameInterface()->ShowErrorMessage(MERR_NO_SELITEM);
			return false;
		}

		uidItem = pListItem->GetUID();

		MMatchItemDesc* pItemDesc = MGetMatchItemDescMgr()->GetItemDesc(ZGetMyInfo()->GetItemList()->GetItemID(uidItem));

		if (pItemDesc == NULL) return false;
		if (pItemDesc->m_nSlot == MMIST_RANGE)
		{
			if (ZGetMyInfo()->GetItemList()->GetEquipedItemID(MMCIP_PRIMARY) == 0)
			{
				parts = MMCIP_PRIMARY;
			}
			else if (ZGetMyInfo()->GetItemList()->GetEquipedItemID(MMCIP_SECONDARY) == 0)
			{
				parts = MMCIP_SECONDARY;
			}
		}
		else if (pItemDesc->m_nSlot == MMIST_CUSTOM)
		{
			if (ZGetMyInfo()->GetItemList()->GetEquipedItemID(MMCIP_CUSTOM1) == 0)
			{
				parts = MMCIP_CUSTOM1;
			}
			else if (ZGetMyInfo()->GetItemList()->GetEquipedItemID(MMCIP_CUSTOM2) == 0)
			{
				parts = MMCIP_CUSTOM2;
			}
		}
		else if (pItemDesc->m_nSlot == MMIST_FINGER)
		{
			if (ZGetMyInfo()->GetItemList()->GetEquipedItemID(MMCIP_FINGERL) == 0)
			{
				parts = MMCIP_FINGERL;
			}
			else if (ZGetMyInfo()->GetItemList()->GetEquipedItemID(MMCIP_FINGERR) == 0)
			{
				parts = MMCIP_FINGERR;
			}
		}

		if (parts == MMCIP_END)
		{
			parts = GetSuitableItemParts(pItemDesc->m_nSlot);
		}

		MButton* pButton = (MButton*)m_IDLResource.FindWidget("Equip");
		if (pButton)
		{
			pButton->Enable(false);
			pButton->Show(false);
			pButton->Show(true);
		}
		pButton = (MButton*)m_IDLResource.FindWidget("SendAccountItemBtn");
		if (pButton)
		{
			pButton->Enable(false);
			pButton->Show(false);
			pButton->Show(true);
		}

		SetKindableItem(MMIST_NONE);
	}
	else
	{
		ZApplication::GetGameInterface()->ShowErrorMessage(MERR_NO_SELITEM);
		return false;
	}


	return Equip(parts, uidItem);
}

bool ZGameInterface::Equip(MMatchCharItemParts parts, MUID& uidItem)
{

	ZPostRequestEquipItem(ZGetGameClient()->GetPlayerUID(), uidItem, parts);
	// The server sends this automatically if UPDATE_STAGE_EQUIP_LOOK is defined.
#ifndef UPDATE_STAGE_EQUIP_LOOK
	ZPostRequestCharacterItemList(ZGetGameClient()->GetPlayerUID());
#endif
	return true;
}

int ZGameInterface::CheckRestrictBringAccountItem()
{
	// Return -1 : Error
	//		  0 : No restriction
	//		  1 : Sex restricted
	//		  2 : Level restricted

	ZEquipmentListBox* pListBox = (ZEquipmentListBox*)m_IDLResource.FindWidget("AccountItemList");
	if (pListBox == NULL) return -1;

	if (pListBox->IsSelected())
	{
		ZEquipmentListItem* pListItem = (ZEquipmentListItem*)pListBox->GetSelItem();
		if (pListItem == NULL) return -1;

		const auto nItemID = pListItem->GetItemID();
		MMatchItemDesc* pItemDesc = MGetMatchItemDescMgr()->GetItemDesc(nItemID);
		if (pItemDesc == NULL)
			return -1;	// Item Not Found
		if ((pItemDesc->m_nResSex != -1) && (ZGetMyInfo()->GetSex() != pItemDesc->m_nResSex))
			return 1;	// Sex restricted
		if (ZGetMyInfo()->GetLevel() < pItemDesc->m_nResLevel)
			return 2;	// Level restricted
	}

	return 0;	// No Restriction
}

void ZGameInterface::BringAccountItem()
{
	ZEquipmentListBox* pListBox = (ZEquipmentListBox*)m_IDLResource.FindWidget("AccountItemList");
	if (pListBox == NULL) return;

	if (pListBox->IsSelected())
	{
		ZEquipmentListItem* pListItem = (ZEquipmentListItem*)pListBox->GetSelItem();
		if (pListItem == NULL)
		{
			ZApplication::GetGameInterface()->ShowErrorMessage(MERR_NO_SELITEM);
			return;
		}

		const int nAIID = pListItem->GetAIID();
		if (nAIID != 0)
		{
			static u32 st_LastRequestTime = 0;
			u32 nNowTime = GetGlobalTimeMS();
			if ((nNowTime - st_LastRequestTime) >= 1000)
			{
				ZPostRequestBringAccountItem(ZGetGameClient()->GetPlayerUID(), nAIID);

				st_LastRequestTime = nNowTime;
			}
		}
	}
	else
	{
		ZApplication::GetGameInterface()->ShowErrorMessage(MERR_NO_SELITEM);
		return;
	}
}

void ZGameInterface::ShowMessage(const char* szText, MListener* pCustomListenter, int nMessageID)
{
	if (pCustomListenter)
		m_pMsgBox->SetCustomListener(pCustomListenter);

	char text[1024] = "";

	if (nMessageID != 0)
	{
		sprintf_safe(text, "%s (M%d)", szText, nMessageID);
	}
	else
	{
		strcpy_safe(text, szText);
	}

	m_pMsgBox->SetText(text);
	m_pMsgBox->Show(true, true);
}

void ZGameInterface::ShowConfirmMessage(const char* szText, MListener* pCustomListenter)
{
	if (pCustomListenter)
		m_pConfirmMsgBox->SetCustomListener(pCustomListenter);

	m_pConfirmMsgBox->SetText(szText);
	m_pConfirmMsgBox->Show(true, true);
}

void ZGameInterface::ShowMessage(int nMessageID)
{
	const char *str = ZMsg(nMessageID);
	if (str)
	{
		char text[1024];
		sprintf_safe(text, "%s (M%d)", str, nMessageID);
		ShowMessage(text);
	}
}

void ZGameInterface::ShowErrorMessage(int nErrorID)
{
	const char *str = ZErrStr(nErrorID);
	if (str)
	{
		char text[1024];
		sprintf_safe(text, "%s (E%d)", str, nErrorID);
		ShowMessage(text);
	}
}

void ZGameInterface::ShowErrorMessage(const char *szMessage)
{
	ShowMessage(szMessage);
}


void ZGameInterface::ChangeSelectedChar(int nNum)
{
	bool bRequested = false;

	if ((!ZCharacterSelectView::m_CharInfo[nNum].m_bLoaded) && (!ZCharacterSelectView::m_CharInfo[nNum].m_bRequested))
	{
		ZPostAccountCharInfo(nNum);
		GetCharacterSelectView()->UpdateInterface(nNum);
		ZCharacterSelectView::m_CharInfo[nNum].m_bRequested = true;
		bRequested = true;
	}

	if ((!bRequested) && (GetCharacterSelectView() != NULL))
	{
		GetCharacterSelectView()->SelectChar(nNum);
	}
}

void ZGameInterface::OnCharSelectionCreate()
{
	ZApplication::GetSoundEngine()->OpenMusic(BGMID_INTRO, ZApplication::GetFileSystem());

	EnableCharSelectionInterface(true);

	if (m_pCharacterSelectView != NULL) SAFE_DELETE(m_pCharacterSelectView);
	m_pCharacterSelectView = new ZCharacterSelectView();
	m_pCharacterSelectView->SetBackground(m_pBackground);
	m_pCharacterSelectView->SelectChar(ZCharacterSelectView::GetSelectedCharacter());

	ZIDLResource* pResource = ZApplication::GetGameInterface()->GetIDLResource();

	MWidget* pWidget = pResource->FindWidget("CS_SelectCharDefKey");
	if (pWidget)
		pWidget->Enable(true);

	pWidget = pResource->FindWidget("CharSel_GameRoomUser");
	if (pWidget)
	{
		if (ZGetMyInfo()->GetPGradeID() == MMPG_PREMIUM_IP)
			pWidget->Show(true);
		else
			pWidget->Show(false);
	}
}

void ZGameInterface::OnCharSelect()
{
	m_pCharacterSelectView->SelectMyCharacter();
	EnableCharSelectionInterface(false);

	if (m_pBackground)
		m_pBackground->SetScene(LOGIN_SCENE_SELECTCHAR);
}

void ZGameInterface::OnCharSelectionDestroy()
{
	ShowWidget("CharSelection", false);
	if (m_pCharacterSelectView != NULL) SAFE_DELETE(m_pCharacterSelectView);
}

void ZGameInterface::OnCharCreationCreate()
{
	ShowWidget("CharSelection", false);
	ShowWidget("CharCreation", true);

	if (m_pCharacterSelectView != NULL) SAFE_DELETE(m_pCharacterSelectView);
	m_pCharacterSelectView = new ZCharacterSelectView();
	m_pCharacterSelectView->SetBackground(m_pBackground);
	m_pCharacterSelectView->SetState(ZCharacterSelectView::ZCSVS_CREATE);
	m_pCharacterSelectView->OnChangedCharCostume();
}

void ZGameInterface::OnCharCreationDestroy()
{
	ShowWidget("CharCreation", false);
	ShowWidget("CharSelection", true);

	if (m_pCharacterSelectView != NULL) SAFE_DELETE(m_pCharacterSelectView);
}

void ZGameInterface::ChangeToCharSelection()
{
	ZCharacterSelectView::ClearCharInfo();
	ZPostAccountCharList(ZGetMyInfo()->GetSystemInfo()->szSerialKey, ZGetMyInfo()->GetSystemInfo()->pbyGuidAckMsg);
}

void ZGameInterface::OnInvalidate()
{
	ZGetWorldManager()->OnInvalidate();
	if (m_pGame)
		m_pGame->OnInvalidate();
	if (m_pBackground)
		m_pBackground->OnInvalidate();
	if (m_pCharacterSelectView)
		m_pCharacterSelectView->OnInvalidate();

	if (ZGetEffectManager())
		ZGetEffectManager()->OnInvalidate();

	ZCharacterView* pCharView = (ZCharacterView*)m_IDLResource.FindWidget("Stage_Charviewer");
	if (pCharView != 0) pCharView->OnInvalidate();
	pCharView = (ZCharacterView*)m_IDLResource.FindWidget("EquipmentInformation");
	if (pCharView != 0) pCharView->OnInvalidate();
	pCharView = (ZCharacterView*)m_IDLResource.FindWidget("EquipmentInformationShop");
	if (pCharView != 0) pCharView->OnInvalidate();
}

void ZGameInterface::OnRestore()
{
	ZGetWorldManager()->OnRestore();
	if (m_pGame)
		m_pGame->OnRestore();
	if (m_pBackground)
		m_pBackground->OnRestore();
	if (m_pCharacterSelectView)
		m_pCharacterSelectView->OnRestore();
	if (ZGetEffectManager())
		ZGetEffectManager()->OnRestore();

	ZCharacterView* pCharView = (ZCharacterView*)m_IDLResource.FindWidget("Stage_Charviewer");
	if (pCharView != 0) pCharView->OnRestore();
	pCharView = (ZCharacterView*)m_IDLResource.FindWidget("EquipmentInformation");
	if (pCharView != 0) pCharView->OnRestore();
	pCharView = (ZCharacterView*)m_IDLResource.FindWidget("EquipmentInformationShop");
	if (pCharView != 0) pCharView->OnRestore();
}


void ZGameInterface::UpdateBlueRedTeam()
{
	MButton* pBlueTeamBtn = (MButton*)m_IDLResource.FindWidget("StageTeamBlue");
	MButton* pBlueTeamBtn2 = (MButton*)m_IDLResource.FindWidget("StageTeamBlue2");
	MButton* pRedTeamBtn = (MButton*)m_IDLResource.FindWidget("StageTeamRed");
	MButton* pRedTeamBtn2 = (MButton*)m_IDLResource.FindWidget("StageTeamRed2");
	if ((pRedTeamBtn == NULL) || (pBlueTeamBtn == NULL) || (pRedTeamBtn2 == NULL) || (pBlueTeamBtn2 == NULL))
		return;

	if (m_bTeamPlay)
	{
		pRedTeamBtn->Show(true);
		pRedTeamBtn2->Show(true);
		pBlueTeamBtn->Show(true);
		pBlueTeamBtn2->Show(true);

		ZPlayerListBox* pListBox = (ZPlayerListBox*)m_IDLResource.FindWidget("StagePlayerList_");

		if (pListBox)
		{
			int nR = 0, nB = 0;

			for (int i = 0; i < pListBox->GetCount(); i++)
			{
				auto* pSItem = static_cast<ZStagePlayerListItem*>(pListBox->Get(i));
				if (pSItem->Team == MMT_RED)
					nR++;
				else if (pSItem->Team == MMT_BLUE)
					nB++;
			}

			char buffer[64];
			ZIDLResource* pRes = ZApplication::GetGameInterface()->GetIDLResource();
			ZBmNumLabel *pNumLabel;

			sprintf_safe(buffer, "%s:%d", ZMsg(MSG_WORD_BLUETEAM), nB);
			pNumLabel = (ZBmNumLabel*)pRes->FindWidget("StageNumOfBlueTeam");
			if (pNumLabel)
			{
				pNumLabel->SetText(buffer);
				pNumLabel->Show(true);
			}
			MButton* pButton = (MButton*)pRes->FindWidget("StageTeamBlue");
			if (pButton)
				pButton->SetText(buffer);

			sprintf_safe(buffer, "%s:%d", ZMsg(MSG_WORD_REDTEAM), nR);
			pNumLabel = (ZBmNumLabel*)pRes->FindWidget("StageNumOfRedTeam");
			if (pNumLabel)
			{
				pNumLabel->SetIndexOffset(16);
				pNumLabel->SetText(buffer);
				pNumLabel->Show(true);
			}
			pButton = (MButton*)pRes->FindWidget("StageTeamRed");
			if (pButton)
				pButton->SetText(buffer);

			sprintf_safe(buffer, "%s : %d", ZMsg(MSG_WORD_PLAYERS), nB + nR);
			MLabel* pLabel = (MLabel*)pRes->FindWidget("Stage_NumOfPerson");
			if (pLabel)
				pLabel->SetText(buffer);
		}
	}
	else
	{
		pRedTeamBtn->Show(false);
		pRedTeamBtn2->Show(false);
		pBlueTeamBtn->Show(false);
		pBlueTeamBtn2->Show(false);

		ZPlayerListBox* pListBox = (ZPlayerListBox*)m_IDLResource.FindWidget("StagePlayerList_");

		if (pListBox)
		{
			int nPlayerNum = 0;

			for (int i = 0; i < pListBox->GetCount(); i++)
			{
				ZStagePlayerListItem *pSItem = (ZStagePlayerListItem*)pListBox->Get(i);
				if (pSItem->Team == MMT_ALL)
					nPlayerNum++;
			}

			ZIDLResource* pRes = ZApplication::GetGameInterface()->GetIDLResource();
			ZBmNumLabel *pNumLabel;

			char buffer[64];

			pNumLabel = (ZBmNumLabel*)pRes->FindWidget("StageNumOfBlueTeam");
			if (pNumLabel)
				pNumLabel->Show(false);

			pNumLabel = (ZBmNumLabel*)pRes->FindWidget("StageNumOfRedTeam");
			if (pNumLabel)
				pNumLabel->Show(false);

			sprintf_safe(buffer, "%s : %d", ZMsg(MSG_WORD_PLAYERS), nPlayerNum);
			MLabel* pLabel = (MLabel*)pRes->FindWidget("Stage_NumOfPerson");
			if (pLabel)
				pLabel->SetText(buffer);
		}
	}
}

void ZGameInterface::ShowInterface(bool bShowInterface)
{
	m_bShowInterface = bShowInterface;

	if (m_nState != GUNZ_GAME)
	{
		SetCursorEnable(bShowInterface);
	}

	// Login
	if (m_nState == GUNZ_LOGIN)
	{
		ShowWidget("Login", m_bShowInterface);
	}
	else if (m_nState == GUNZ_CHARSELECTION)
	{
		ShowWidget("CharSelection", m_bShowInterface);
	}
	else if (m_nState == GUNZ_GAME)
	{
		bool bConsole = ZGetConsole()->IsVisible();
		bool bLogFrame = m_pLogFrame->IsVisible();

		m_pLogFrame->Show(m_bShowInterface);
		ZGetConsole()->Show(m_bShowInterface);
		ShowWidget("CombatInfo1", m_bShowInterface);
		ShowWidget("CombatInfo2", m_bShowInterface);
		ShowWidget("Time", m_bShowInterface);
		ZGetConsole()->Show(bConsole);
		m_pLogFrame->Show(bLogFrame);
	}
}

void ZGameInterface::OnResponseShopItemList(u32* nItemList, int nItemCount)
{
	ZGetShop()->SetItemsAll(nItemList, nItemCount);
	ZGetShop()->Serialize();
}

void ZGameInterface::OnResponseCharacterItemList(MUID* puidEquipItem, MTD_ItemNode* pItemNodes, int nItemCount)
{
	ZGetMyInfo()->GetItemList()->SetItemsAll(pItemNodes, nItemCount);
	ZGetMyInfo()->GetItemList()->SetEquipItemsAll(puidEquipItem);
	ZGetMyInfo()->GetItemList()->Serialize();


	MTextArea* pTextArea = (MTextArea*)m_IDLResource.FindWidget("Shop_MyInfo");
	if (pTextArea)
	{
		pTextArea->Clear();

		char szTemp[64];
		sprintf_safe(szTemp, "^9%s : %d %s", ZMsg(MSG_CHARINFO_LEVEL), ZGetMyInfo()->GetLevel(), ZMsg(MSG_CHARINFO_LEVELMARKER));
		pTextArea->AddText(szTemp);

		sprintf_safe(szTemp, "^9%s : %d", ZMsg(MSG_CHARINFO_BOUNTY), ZGetMyInfo()->GetBP());
		pTextArea->AddText(szTemp);

		sprintf_safe(szTemp, "^9%s : %d", ZMsg(MSG_CHARINFO_HP), ZGetMyInfo()->GetHP());
		pTextArea->AddText(szTemp);

		sprintf_safe(szTemp, "^9%s : %d", ZMsg(MSG_CHARINFO_AP), ZGetMyInfo()->GetAP());
		pTextArea->AddText(szTemp);

		sprintf_safe(szTemp, "^9%s : %d/%d", ZMsg(MSG_CHARINFO_WEIGHT), ZGetMyInfo()->GetItemList()->GetEquipedTotalWeight(), ZGetMyInfo()->GetItemList()->GetMaxWeight());
		pTextArea->AddText(szTemp);
	}

	pTextArea = (MTextArea*)m_IDLResource.FindWidget("Equip_MyInfo");
	if (pTextArea)
	{
		pTextArea->Clear();

		char szTemp[64];
		sprintf_safe(szTemp, "^9%s : %d %s", ZMsg(MSG_CHARINFO_LEVEL), ZGetMyInfo()->GetLevel(), ZMsg(MSG_CHARINFO_LEVELMARKER));
		pTextArea->AddText(szTemp);

		sprintf_safe(szTemp, "^9%s : %d", ZMsg(MSG_CHARINFO_BOUNTY), ZGetMyInfo()->GetBP());
		pTextArea->AddText(szTemp);

		sprintf_safe(szTemp, "^9%s : %d", ZMsg(MSG_CHARINFO_HP), ZGetMyInfo()->GetHP());
		pTextArea->AddText(szTemp);

		sprintf_safe(szTemp, "^9%s : %d", ZMsg(MSG_CHARINFO_AP), ZGetMyInfo()->GetAP());
		pTextArea->AddText(szTemp);

		sprintf_safe(szTemp, "^9%s : %d/%d", ZMsg(MSG_CHARINFO_WEIGHT), ZGetMyInfo()->GetItemList()->GetEquipedTotalWeight(), ZGetMyInfo()->GetItemList()->GetMaxWeight());
		pTextArea->AddText(szTemp);
	}
}

void ZGameInterface::FinishGame()
{
	g_pGame->StopRecording();
	ZGetGameInterface()->GetCombatInterface()->Finish();
}

void ZGameInterface::SerializeStageInterface()
{
	InitMapSelectionWidget();
	ZApplication::GetStageInterface()->OnStageInterfaceSettup();
}


void ZGameInterface::HideAllWidgets()
{
	ShowWidget("Login", false);
	ShowWidget("Lobby", false);
	ShowWidget("Stage", false);
	ShowWidget("Game", false);
	ShowWidget("Option", false);
	ShowWidget("CharSelection", false);
	ShowWidget("CharCreation", false);
	ShowWidget("Shop", false);

	// dialog
	ShowWidget("StageSettingFrame", false);
	ShowWidget("BuyConfirm", false);
	ShowWidget("Equipment", false);
	ShowWidget("StageCreateFrame", false);
	ShowWidget("PrivateStageJoinFrame", false);
}

bool SetWidgetToolTipText(char* szWidget, const char* szToolTipText) {

	ZIDLResource* pResource = ZApplication::GetGameInterface()->GetIDLResource();

	if (pResource == NULL)		return false;
	if (!szToolTipText)		return false;


	MWidget* pWidget = pResource->FindWidget(szWidget);

	if (pWidget) {
		if (!szToolTipText[0]) {
			pWidget->DetachToolTip();
		}
		else {
			pWidget->AttachToolTip(new ZToolTip(szToolTipText, pWidget));
		}
	}
	return false;
}

bool GetItemDescName(string& str, DWORD nItemID)
{
	MMatchItemDesc* pItemDesc = MGetMatchItemDescMgr()->GetItemDesc(nItemID);

	if (pItemDesc == NULL)
	{
		str.clear();
		return false;
	}

	str = (string)pItemDesc->m_szName;
	return false;
}

bool GetItemDescStr(string& str, DWORD nItemID) {

	static char temp[1024];

	MMatchItemDesc* pItemDesc = MGetMatchItemDescMgr()->GetItemDesc(nItemID);

	if (pItemDesc == NULL) {
		str.clear();
		return false;
	}

	bool bAdd = false;
	bool bLine = false;
	int	 nLine = 0;
	int  nLen = 0;

	if (pItemDesc->m_szName) {
		str += pItemDesc->m_szName;
		bAdd = true;
	}

	if (pItemDesc->m_nResLevel) {
		if (bAdd) str += " / ";
		sprintf_safe(temp, "제한레벨:%d", pItemDesc->m_nResLevel);

		nLen = (int)strlen(temp);
		if ((int)str.size() + nLen > (nLine + 1) * MAX_TOOLTIP_LINE_STRING + 3) { str += "\n"; nLine++; }

		str += temp;
		bAdd = true;
	}

	if (pItemDesc->m_nWeight) {
		if (bAdd) str += " / ";
		sprintf_safe(temp, "무게:%d", pItemDesc->m_nWeight);

		nLen = (int)strlen(temp);
		if ((int)str.size() + nLen > (nLine + 1) * MAX_TOOLTIP_LINE_STRING + 3) { str += "\n"; nLine++; }

		str += temp;
		bAdd = true;
	}

	if (pItemDesc->m_nMaxBullet) {
		if (bAdd) str += " / ";
		sprintf_safe(temp, "최대탄수 : %d", pItemDesc->m_nMaxBullet);

		nLen = (int)strlen(temp);
		if ((int)str.size() + nLen > (nLine + 1) * MAX_TOOLTIP_LINE_STRING + 3) { str += "\n"; nLine++; }

		str += temp;
		bAdd = true;
	}


	if (pItemDesc->m_nMagazine) {
		if (bAdd) str += " / ";
		sprintf_safe(temp, "탄창 : %d", pItemDesc->m_nMagazine);

		nLen = (int)strlen(temp);
		if ((int)str.size() + nLen > (nLine + 1) * MAX_TOOLTIP_LINE_STRING + 3) { str += "\n"; nLine++; }

		str += temp;
		bAdd = true;
	}

	if (pItemDesc->m_nDamage) {
		if (bAdd) str += " / ";
		sprintf_safe(temp, "공격력 : %d", pItemDesc->m_nDamage);

		nLen = (int)strlen(temp);
		if ((int)str.size() + nLen > (nLine + 1) * MAX_TOOLTIP_LINE_STRING + 3) { str += "\n"; nLine++; }

		str += temp;
		bAdd = true;
	}

	if (pItemDesc->m_nDelay) {
		if (bAdd) str += " / ";
		sprintf_safe(temp, "딜레이 : %d", pItemDesc->m_nDelay);

		nLen = (int)strlen(temp);
		if ((int)str.size() + nLen > (nLine + 1) * MAX_TOOLTIP_LINE_STRING + 3) { str += "\n"; nLine++; }

		str += temp;
		bAdd = true;
	}

	if (pItemDesc->m_nReloadTime) {
		if (bAdd) str += " / ";
		sprintf_safe(temp, "장전시간 : %d", pItemDesc->m_nReloadTime);

		nLen = (int)strlen(temp);
		if ((int)str.size() + nLen > (nLine + 1) * MAX_TOOLTIP_LINE_STRING + 3) { str += "\n"; nLine++; }

		str += temp;
		bAdd = true;
	}

	if (pItemDesc->m_nHP) {
		if (bAdd) str += " / ";
		sprintf_safe(temp, "+HP : %d", pItemDesc->m_nHP);

		nLen = (int)strlen(temp);
		if ((int)str.size() + nLen > (nLine + 1) * MAX_TOOLTIP_LINE_STRING + 3) { str += "\n"; nLine++; }

		str += temp;
		bAdd = true;
	}

	if (pItemDesc->m_nAP) {
		if (bAdd) str += " / ";
		sprintf_safe(temp, "+AP : %d", pItemDesc->m_nAP);

		nLen = (int)strlen(temp);
		if ((int)str.size() + nLen > (nLine + 1) * MAX_TOOLTIP_LINE_STRING + 3) { str += "\n"; nLine++; }

		str += temp;
		bAdd = true;
	}

	if (pItemDesc->m_nMaxWT) {
		if (bAdd) str += " / ";
		sprintf_safe(temp, "+최대무게 : %d", pItemDesc->m_nMaxWT);

		nLen = (int)strlen(temp);
		if ((int)str.size() + nLen > (nLine + 1) * MAX_TOOLTIP_LINE_STRING + 3) { str += "\n"; nLine++; }

		str += temp;
		bAdd = true;
	}

	return true;
}

void ZGameInterface::ChangeEquipPartsToolTipAll()
{
	ZMyItemList* pil = ZGetMyInfo()->GetItemList();

	MWidget* pWidget;
	MRECT r;
	string item_desc_str;

	for (int i = 0; i < MMCIP_END; i++)
	{
		GetItemDescName(item_desc_str, pil->GetEquipedItemID(MMatchCharItemParts(i)));

		// Shop
		pWidget = (MWidget*)ZApplication::GetGameInterface()->GetIDLResource()->FindWidget(GetItemSlotName("Shop", i));
		if (pWidget != NULL)
		{
			r = pWidget->GetRect();
			if (r.w < 100)
				SetWidgetToolTipText(GetItemSlotName("Shop", i), item_desc_str.c_str());
		}

		// Equipment
		pWidget = (MWidget*)ZApplication::GetGameInterface()->GetIDLResource()->FindWidget(GetItemSlotName("Equip", i));
		if (pWidget != NULL)
		{
			r = pWidget->GetRect();
			if (r.w < 100)
				SetWidgetToolTipText(GetItemSlotName("Equip", i), item_desc_str.c_str());
		}
		item_desc_str.clear();
	}
}

void ZGameInterface::ClearEquipPartsToolTipAll(const char* szName)
{
	for (int i = 0; i < MMCIP_END; i++)
		SetWidgetToolTipText(GetItemSlotName(szName, i), "");
}

static bool SetLocationLabel(bool Shop, ZIDLResource* pResource)
{
	char buf[256];
	sprintf_safe(buf, "%s_Message", Shop ? "Shop" : "Equip");

	auto* pLabel = (MLabel*)pResource->FindWidget(buf);
	if (!pLabel)
		return false;

	bool Stage = ZGetGameInterface()->GetState() == GUNZ_STAGE;
	auto Location1MsgID = Stage ? MSG_WORD_STAGE : MSG_WORD_LOBBY;
	auto Location2MsgID = Shop ? MSG_WORD_SHOP : MSG_WORD_EQUIPMENT;

	// Server name -> (Lobby|Stage) -> (Shop|Equipment)
	sprintf_safe(buf, "%s > %s > %s",
		ZGetGameClient()->GetServerName(),
		ZMsg(Location1MsgID), ZMsg(Location2MsgID));

	pLabel->SetText(buf);

	return true;
}

static bool SetStripAnimations(bool Shop, ZIDLResource* pResource)
{
	bool ret = true;

	auto f = [&](int AnimType, const char* Name)
	{
		char buf[256];
		sprintf_safe(buf, "%s_%s", Shop ? "Shop" : "Equip", Name);
		auto pPicture = (MPicture*)pResource->FindWidget(buf);
		if (!pPicture)
		{
			ret = false;
			return;
		}

		pPicture->SetAnimation(AnimType, 1000.0f);
	};

	f(0, "StripBottom");
	f(1, "StripTop");

	return ret;
}

static bool SetItemIcon(bool Shop, ZIDLResource* pResource)
{
	char buf[64];
	sprintf_safe(buf, "%s_ItemIcon", Shop ? "Shop" : "Equip");

	auto pPicture = (MPicture*)pResource->FindWidget(buf);
	if (!pPicture)
		return false;

	pPicture->SetBitmap(NULL);

	return true;
}

static bool DisableWidgets(bool Shop, ZIDLResource* pResource)
{
	bool ret = true;

	std::array<const char*, 3> WidgetsToDisable;
	if (Shop)
		WidgetsToDisable = { "BuyConfirmCaller", "BuyCashConfirmCaller", "SellConfirmCaller" };
	else
		WidgetsToDisable = { "Equip", "SendAccountItemBtn", "BringAccountItemBtn" };

	for (auto&& WidgetName : WidgetsToDisable)
	{
		auto Widget = pResource->FindWidget(WidgetName);
		if (!Widget)
		{
			ret = false;
			continue;
		}

		Widget->Enable(false);
	}

	return ret;
}

static bool InitializeItemDescriptions(bool Shop, ZIDLResource* pResource)
{
	bool ret = true;

	for (int i = 0; i < 3; ++i)
	{
		char buf[256];
		sprintf_safe(buf, "%s_ItemDescription%d", Shop ? "Shop" : "Equip", i + 1);
		auto* pTextArea = (MTextArea*)pResource->FindWidget(buf);
		if (!pTextArea)
		{
			ret = false;
			continue;
		}

		// Special case for index 3 in equipment.
		if (!Shop && i == 3)
		{
			pTextArea->SetTextColor({ 180, 180, 180 });
			pTextArea->SetText(ZMsg(MSG_SHOPMSG));
		}
		else
		{
			pTextArea->SetText("");
		}
	}

	return ret;
}

static void PostStageState(bool Shop)
{
	if (ZGetGameInterface()->GetState() == GUNZ_STAGE)
		ZPostStageState(ZGetGameClient()->GetPlayerUID(), ZGetGameClient()->GetStageUID(),
			Shop ? MOSS_SHOP : MOSS_EQUIPMENT);
}

static void InitializeEquipmentInformation(ZIDLResource* pResource)
{
	BEGIN_WIDGETLIST("EquipmentInformation", pResource, ZCharacterView*, pCharacterView);

	ZMyInfo* pmi = ZGetMyInfo();
	ZMyItemList* pil = ZGetMyInfo()->GetItemList();

	u32 nEquipedItemID[MMCIP_END];

	for (int i = 0; i < MMCIP_END; i++)
	{
		nEquipedItemID[i] = pil->GetEquipedItemID(MMatchCharItemParts(i));
	}

	pCharacterView->InitCharParts(pmi->GetSex(), pmi->GetHair(), pmi->GetFace(), nEquipedItemID);

	END_WIDGETLIST();
}

void ZGameInterface::ShowShopOrEquipmentDialog(bool Shop)
{
	auto* pResource = GetIDLResource();

	// Hide lobby and stage in case we're moving from one of those.
	ShowWidget("Lobby", false);
	ShowWidget("Stage", false);

	// Hide the other UI in case we're moving between shop and equipment.
	if (Shop)
		ShowWidget("Equipment", false);
	else
		ShowWidget("Shop", false);

	// Show this UI.
	auto* Widget = pResource->FindWidget(Shop ? "Shop" : "Equipment");
	if (Widget)
		Widget->Show(true, true);

	if (Shop)
	{
		ZGetShop()->Create();
		ZGetShop()->Serialize();
		SelectShopTab(0);
	}
	else
	{
		InitializeEquipmentInformation(pResource);
		SelectEquipmentTab(0);
		ChangeEquipPartsToolTipAll();
	}

#define V(expr) if (!(expr)) { DMLog("%s\n", #expr "failed"); }

	V(SetLocationLabel(Shop, pResource));
	V(SetStripAnimations(Shop, pResource));
	V(InitializeItemDescriptions(Shop, pResource));
	V(SetItemIcon(Shop, pResource));
	V(DisableWidgets(Shop, pResource));

#undef V

	PostStageState(Shop);
	ZPostRequestCharacterItemList(ZGetGameClient()->GetPlayerUID());
}

static void HideShopOrEquipmentDialog(bool Shop)
{
	bool InStage = ZGetGameInterface()->GetState() == GUNZ_STAGE;

	if (InStage)
		ZPostStageState(ZGetGameClient()->GetPlayerUID(), ZGetGameClient()->GetStageUID(), MOSS_NONREADY);

	// Hide the shop or equipment widget.
	MWidget* pWidget = ZGetIDLResource()->FindWidget(Shop ? "Shop" : "Equipment");
	if (pWidget)
		pWidget->Show(false);

	// Return to either the stage or the lobby UI, depending on where we were.
	ZGetGameInterface()->ShowWidget(InStage ? "Stage" : "Lobby", true);
}

void ZGameInterface::ShowEquipmentDialog(bool bShow)
{
	if (bShow)
		ShowShopOrEquipmentDialog(false);
	else
		HideShopOrEquipmentDialog(false);
}

void ZGameInterface::ShowShopDialog(bool bShow)
{
	if (bShow)
		ShowShopOrEquipmentDialog(true);
	else
		HideShopOrEquipmentDialog(true);
}

void ZGameInterface::SelectEquipmentFrameList(const char* szName, bool bOpen)
{
	char szTemp[256];

	ZIDLResource* pResource = GetIDLResource();

	// Frame open/close background image
	MPicture* pPicture;
	strcpy_safe(szTemp, szName);
	strcat_safe(szTemp, "_BGListFrameOpen");
	pPicture = (MPicture*)pResource->FindWidget(szTemp);
	if (pPicture != NULL)
		pPicture->Show(bOpen);

	strcpy_safe(szTemp, szName);
	strcat_safe(szTemp, "_BGListFrameClose");
	pPicture = (MPicture*)pResource->FindWidget(szTemp);
	if (pPicture != NULL)
		pPicture->Show(!bOpen);


	// Frame open/close image
	MButton* pButton;
	strcpy_safe(szTemp, szName);
	strcat_safe(szTemp, "_EquipListFrameCloseButton");
	pButton = (MButton*)pResource->FindWidget(szTemp);
	if (pButton != NULL)
		pButton->Show(bOpen);

	strcpy_safe(szTemp, szName);
	strcat_safe(szTemp, "_EquipListFrameOpenButton");
	pButton = (MButton*)pResource->FindWidget(szTemp);
	if (pButton != NULL)
		pButton->Show(!bOpen);


	// Resize item slot
	char szWidgetName[256];
	sprintf_safe(szWidgetName, "%s_EquipmentSlot_Head", szName);
	MWidget* itemSlot = (MWidget*)pResource->FindWidget(szWidgetName);
	if (itemSlot)
	{
		MRECT rect = itemSlot->GetRect();

		int nWidth;
		if (bOpen)
			nWidth = 220.0f * (float)RGetScreenWidth() / 800.0f;
		else
			nWidth = min(rect.w, rect.h);

		for (int i = 0; i < MMCIP_END; i++)
		{
			itemSlot = (MWidget*)pResource->FindWidget(GetItemSlotName(szName, i));

			if (itemSlot)
			{
				rect = itemSlot->GetRect();
				itemSlot->SetBounds(rect.x, rect.y, nWidth, rect.h);
			}
		}
	}

	// Set tooltip
	if (bOpen)
		ClearEquipPartsToolTipAll(szName);
	else
		ChangeEquipPartsToolTipAll();
}

void ZGameInterface::SelectShopTab(int nTabIndex)
{
	ZIDLResource* pResource = GetIDLResource();

#ifndef _DEBUG
#if defined(LOCALE_BRAZIL) || defined(LOCALE_INDIA) || defined(LOCALE_US) || defined(LOCALE_JAPAN) || defined(LOCALE_KOREA)
	{
		if (nTabIndex == 2)
			return;
	}
#endif
#endif

	MWidget* pWidget = pResource->FindWidget("AllEquipmentList");
	if (pWidget != NULL) pWidget->Show(nTabIndex == 0 ? true : false);
	pWidget = pResource->FindWidget("MyAllEquipmentList");
	if (pWidget != NULL) pWidget->Show(nTabIndex == 1 ? true : false);
	pWidget = pResource->FindWidget("CashEquipmentList");
	if (pWidget != NULL) pWidget->Show(nTabIndex == 2 ? true : false);


	// Set filter
	MComboBox* pComboBox = (MComboBox*)pResource->FindWidget("Shop_AllEquipmentFilter");
	if (pComboBox) {
		int sel = pComboBox->GetSelIndex();

		ZMyItemList* pil = ZGetMyInfo()->GetItemList();
		if (pil)
			pil->m_ListFilter = sel;
	}

	// 버튼 설정
	MButton* pButton = (MButton*)pResource->FindWidget("BuyConfirmCaller");
	if (pButton)
	{
		pButton->Show(false);
		pButton->Enable(false);
	}
	pButton = (MButton*)pResource->FindWidget("BuyCashConfirmCaller");
	if (pButton)
	{
		pButton->Show(false);
		pButton->Enable(false);
	}
	pButton = (MButton*)pResource->FindWidget("SellConfirmCaller");
	if (pButton)
	{
		pButton->Show(false);
		pButton->Enable(false);
	}
	pButton = (MButton*)pResource->FindWidget("SellQuestItemConfirmCaller");
	if (pButton)
	{
		pButton->Show(false);
		pButton->Enable(false);
	}


	if (nTabIndex == 0)
	{
		pButton = (MButton*)pResource->FindWidget("BuyConfirmCaller");
		if (pButton)
			pButton->Show(true);
	}
	else if (nTabIndex == 1)
	{
		if (pComboBox->GetSelIndex() == 10)
		{
			pButton = (MButton*)pResource->FindWidget("SellQuestItemConfirmCaller");
			if (pButton)
				pButton->Show(true);
		}
		else
		{
			pButton = (MButton*)pResource->FindWidget("SellConfirmCaller");
			if (pButton)
				pButton->Show(true);
		}
	}
	else if (nTabIndex == 2)
	{
		pButton = (MButton*)pResource->FindWidget("BuyCashConfirmCaller");
		if (pButton)
			pButton->Show(true);
	}


	pButton = (MButton*)pResource->FindWidget("AllEquipmentListCaller");
	if (pButton != NULL)
		pButton->Show(nTabIndex != 0 ? true : false);
	pButton = (MButton*)pResource->FindWidget("MyAllEquipmentListCaller");
	if (pButton != NULL)
		pButton->Show(nTabIndex != 1 ? true : false);
	pButton = (MButton*)pResource->FindWidget("CashEquipmentListCaller");
	if (pButton != NULL)
		pButton->Show(nTabIndex != 2 ? true : false);

	MPicture* pPicture;
	MBitmap* pBitmap;
	pPicture = (MPicture*)pResource->FindWidget("Shop_FrameTabLabel1");
	if (pPicture)
		pPicture->Show(nTabIndex == 0 ? true : false);
	pPicture = (MPicture*)pResource->FindWidget("Shop_FrameTabLabel2");
	if (pPicture)
		pPicture->Show(nTabIndex == 1 ? true : false);
	pPicture = (MPicture*)pResource->FindWidget("Shop_FrameTabLabel3");
	if (pPicture)
		pPicture->Show(nTabIndex == 2 ? true : false);

	pPicture = (MPicture*)pResource->FindWidget("Shop_TabLabel");
	if (pPicture)
	{
		if (nTabIndex == 0)
			pBitmap = MBitmapManager::Get("framepaneltab1.tga");
		else if (nTabIndex == 1)
			pBitmap = MBitmapManager::Get("framepaneltab2.tga");
		else if (nTabIndex == 2)
			pBitmap = MBitmapManager::Get("framepaneltab3.tga");

		if (pBitmap)
			pPicture->SetBitmap(pBitmap);
	}

#ifndef _DEBUG
#if defined(LOCALE_BRAZIL) || defined(LOCALE_INDIA) || defined(LOCALE_US) || defined(LOCALE_JAPAN) || defined(LOCALE_KOREA)
	{
		pWidget = pResource->FindWidget("Shop_TabLabelBg");
		if (pWidget)  pWidget->Show(false);

		pWidget = pResource->FindWidget("CashEquipmentListCaller");
		if (pWidget)  pWidget->Show(false);

		pWidget = pResource->FindWidget("Shop_FrameTabLabel3");
		if (pWidget)  pWidget->Show(false);
	}
#endif
#endif

	m_nShopTabNum = nTabIndex;
}

void ZGameInterface::SelectEquipmentTab(int nTabIndex)
{
	ZIDLResource* pResource = GetIDLResource();

	SetKindableItem(MMIST_NONE);

	// Set filter
	MComboBox* pComboBox = (MComboBox*)pResource->FindWidget("Equip_AllEquipmentFilter");
	if (pComboBox) {
		int sel = pComboBox->GetSelIndex();

		ZMyItemList* pil = ZGetMyInfo()->GetItemList();
		if (pil)
			pil->m_ListFilter = sel;
	}

	// EQUIPMENTLISTBOX
	MWidget* pWidget = pResource->FindWidget("EquipmentList");
	if (pWidget != NULL) pWidget->Show(nTabIndex == 0 ? true : false);
	pWidget = pResource->FindWidget("AccountItemList");
	if (pWidget != NULL) pWidget->Show(nTabIndex == 0 ? false : true);

	MButton* pButton = (MButton*)pResource->FindWidget("Equip");
	if (pButton)
	{
		pButton->Show(false);
		pButton->Enable(false);
	}
	pButton = (MButton*)pResource->FindWidget("SendAccountItemBtn");
	if (pButton)
	{
		pButton->Show(false);
		pButton->Enable(false);
	}
	pButton = (MButton*)pResource->FindWidget("BringAccountItemBtn");
	if (pButton)
	{
		pButton->Show(false);
		pButton->Enable(false);
	}
	if (nTabIndex == 0)
	{
		pButton = (MButton*)pResource->FindWidget("Equip");
		if (pButton)
			pButton->Show(true);
		pButton = (MButton*)pResource->FindWidget("SendAccountItemBtn");
		if (pButton)
			pButton->Show(true);
	}
	else if (nTabIndex == 1)
	{
		pButton = (MButton*)pResource->FindWidget("BringAccountItemBtn");
		if (pButton)
			pButton->Show(true);
	}

	pButton = (MButton*)pResource->FindWidget("Equipment_CharacterTab");
	if (pButton)
		pButton->Show(nTabIndex == 0 ? false : true);
	pButton = (MButton*)pResource->FindWidget("Equipment_AccountTab");
	if (pButton)
		pButton->Show(nTabIndex == 1 ? false : true);

	MLabel* pLabel;
	pLabel = (MLabel*)pResource->FindWidget("Equipment_FrameTabLabel1");
	if (pLabel)
		pLabel->Show(nTabIndex == 0 ? true : false);
	pLabel = (MLabel*)pResource->FindWidget("Equipment_FrameTabLabel2");
	if (pLabel)
		pLabel->Show(nTabIndex == 1 ? true : false);

	MPicture* pPicture;
	pPicture = (MPicture*)pResource->FindWidget("Equip_ListLabel1");
	if (pPicture)
		pPicture->Show(nTabIndex == 0 ? true : false);
	pPicture = (MPicture*)pResource->FindWidget("Equip_ListLabel2");
	if (pPicture)
		pPicture->Show(nTabIndex == 1 ? true : false);

	pPicture = (MPicture*)pResource->FindWidget("Equip_TabLabel");
	MBitmap* pBitmap;
	if (pPicture)
	{
		if (nTabIndex == 0)
			pBitmap = MBitmapManager::Get("framepaneltab1.tga");
		else
			pBitmap = MBitmapManager::Get("framepaneltab2.tga");

		if (pBitmap)
			pPicture->SetBitmap(pBitmap);
	}

	if (nTabIndex == 1)
	{
		ZGetMyInfo()->GetItemList()->ClearAccountItems();
		ZGetMyInfo()->GetItemList()->SerializeAccountItem();
	}

	for (int i = 0; i < MMCIP_END; i++)
	{
		ZItemSlotView* itemSlot = (ZItemSlotView*)m_IDLResource.FindWidget(GetItemSlotName("Equip", i));
		itemSlot->EnableDragAndDrop(nTabIndex == 0 ? true : false);
	}

	m_nEquipTabNum = nTabIndex;
}


void ZGameInterface::EnableCharSelectionInterface(bool bEnable)
{
	MWidget* pWidget;

	pWidget = m_IDLResource.FindWidget("CS_SelectChar");
	if (pWidget) pWidget->Enable(bEnable);
	pWidget = m_IDLResource.FindWidget("CS_DeleteChar");
	if (pWidget) pWidget->Enable(bEnable);
	pWidget = m_IDLResource.FindWidget("CS_CreateChar");
	if (pWidget) pWidget->Enable(bEnable);
	pWidget = m_IDLResource.FindWidget("CS_Prev");
	if (pWidget) pWidget->Enable(bEnable);
	pWidget = m_IDLResource.FindWidget("CS_Name_Pre");
	if (pWidget) pWidget->Enable(bEnable);
	pWidget = m_IDLResource.FindWidget("CS_Name_Next");
	if (pWidget) pWidget->Enable(bEnable);
	pWidget = m_IDLResource.FindWidget("CS_Name");
	if (pWidget) pWidget->Enable(bEnable);

	if (!bEnable)
	{
		for (int i = 0; i < MAX_CHAR_COUNT; i++)
		{
			char szName[256];
			sprintf_safe(szName, "CharSel_SelectBtn%d", i);
			pWidget = m_IDLResource.FindWidget(szName);
			if (pWidget)
				pWidget->Show(false);
		}
	}

	if (bEnable && (ZCharacterSelectView::GetNumOfCharacter() == 0))
	{
		pWidget = m_IDLResource.FindWidget("CS_SelectChar");
		if (pWidget) pWidget->Enable(false);
		pWidget = m_IDLResource.FindWidget("CS_DeleteChar");
		if (pWidget) pWidget->Enable(false);
	}

	if (bEnable && (ZCharacterSelectView::GetNumOfCharacter() >= MAX_CHAR_COUNT))
	{
		pWidget = m_IDLResource.FindWidget("CS_CreateChar");
		if (pWidget) pWidget->Enable(false);
	}
}

void ZGameInterface::EnableLobbyInterface(bool bEnable)
{

	EnableWidget("LobbyOptionFrame", bEnable);
	EnableWidget("Lobby_Charviewer_info", bEnable);
	EnableWidget("StageJoin", bEnable);
	EnableWidget("StageCreateFrameCaller", bEnable);
	EnableWidget("LobbyShopCaller", bEnable);
	EnableWidget("LobbyEquipmentCaller", bEnable);
	EnableWidget("ReplayCaller", bEnable);
	EnableWidget("CharSelectionCaller", bEnable);
	EnableWidget("Logout", bEnable);
	EnableWidget("QuickJoin", bEnable);
	EnableWidget("QuickJoin2", bEnable);
	EnableWidget("ChannelListFrameCaller", bEnable);
	EnableWidget("StageList", bEnable);
	EnableWidget("Lobby_StageList", bEnable);
	EnableWidget("LobbyChannelPlayerList", bEnable);
	EnableWidget("ChannelChattingOutput", bEnable);
	EnableWidget("ChannelChattingInput", bEnable);
}

void ZGameInterface::EnableStageInterface(bool bEnable)
{
	EnableWidget("Stage_Charviewer_info", bEnable);
	EnableWidget("StagePlayerNameInput_combo", bEnable);
	EnableWidget("GameStart", bEnable);
	MButton* pButton = (MButton*)m_IDLResource.FindWidget("StageReady");
	if (pButton)
	{
		pButton->Enable(bEnable);
		pButton->SetCheck(false);
	}
	EnableWidget("ForcedEntryToGame", bEnable);
	EnableWidget("ForcedEntryToGame2", bEnable);
	EnableWidget("StageTeamBlue", bEnable);
	EnableWidget("StageTeamBlue2", bEnable);
	EnableWidget("StageTeamRed", bEnable);
	EnableWidget("StageTeamRed2", bEnable);
	EnableWidget("StageShopCaller", bEnable);
	EnableWidget("StageEquipmentCaller", bEnable);
	EnableWidget("StageSettingCaller", bEnable);
	EnableWidget("StageObserverBtn", bEnable);
	EnableWidget("Lobby_StageExit", bEnable);

	EnableWidget("MapSelection", bEnable);
	EnableWidget("StageType", bEnable);
	EnableWidget("StageMaxPlayer", bEnable);
	EnableWidget("StageRoundCount", bEnable);
}

void ZGameInterface::SetRoomNoLight(int d)
{
	char szBuffer[64];
	sprintf_safe(szBuffer, "Lobby_RoomNo%d", d);
	MButton* pButton = (MButton*)ZApplication::GetGameInterface()->GetIDLResource()->FindWidget(szBuffer);
	if (pButton)
		pButton->SetCheck(true);

}

void ZGameInterface::ShowPrivateStageJoinFrame(const char* szStageName)
{
	MEdit* pStageNameEdit = (MEdit*)m_IDLResource.FindWidget("PrivateStageName");
	if (pStageNameEdit)
	{
		pStageNameEdit->SetText(szStageName);
		pStageNameEdit->Enable(false);
	}
	MEdit* pPassEdit = (MEdit*)m_IDLResource.FindWidget("PrivateStagePassword");
	if (pPassEdit != NULL)
	{
		pPassEdit->SetMaxLength(STAGEPASSWD_LENGTH);
		pPassEdit->SetText("");
		pPassEdit->SetFocus();
	}

	MWidget* pPrivateStageJoinFrame = m_IDLResource.FindWidget("PrivateStageJoinFrame");
	if (pPrivateStageJoinFrame)
	{
		pPrivateStageJoinFrame->Show(true, true);
	}
}

void ZGameInterface::LeaveBattle()
{
	ZGetGameInterface()->SetCursorEnable(true);
	ShowMenu(false);
	DEFER([&] { m_bLeaveBattleReserved = m_bLeaveStageReserved = EnteredReplayFromLogin = false; });

	if (EnteredReplayFromLogin)
	{
		SetState(GUNZ_LOGIN);
		return;
	}

	ZPostStageLeaveBattle(ZGetGameClient()->GetPlayerUID(), ZGetGameClient()->GetStageUID());
	if (m_bLeaveStageReserved) {
		ZPostStageLeave(ZGetGameClient()->GetPlayerUID(), ZGetGameClient()->GetStageUID());
		SetState(GUNZ_LOBBY);
	}
	else {
		SetState(GUNZ_STAGE);
	}
}

void ZGameInterface::ReserveLeaveStage()
{
	m_bLeaveStageReserved = true;
	ReserveLeaveBattle();
}

void ZGameInterface::ReserveLeaveBattle()
{
	if (!m_pGame) return;

	if (ZGetGame()->GetTime() - ZGetGame()->m_pMyCharacter->LastDamagedTime > 5
		|| !ZGetGame()->m_pMyCharacter->IsAlive()
		|| ZGetGame()->IsReplay())
	{
		LeaveBattle();
		return;
	}

	m_bLeaveBattleReserved = true;
	m_dwLeaveBattleTime = GetGlobalTimeMS() + 5000;
}

void ZGameInterface::ShowMenu(bool bEnable)
{
	if (!bEnable && IsMenuVisible())
	{
		ZGetInput()->ResetRotation();
	}

	m_CombatMenu.ShowModal(bEnable);
	ZGetGameInterface()->SetCursorEnable(bEnable);
}

bool ZGameInterface::IsMenuVisible()
{
	return m_CombatMenu.IsVisible();
}

void ZGameInterface::Show112Dialog(bool bShow)
{
	MWidget* pWidget = m_IDLResource.FindWidget("112Confirm");
	if (!pWidget)
		return;

	if (pWidget->IsVisible() == bShow)
		return;

	pWidget->Show(bShow, true);
	pWidget->SetFocus();

	if (!bShow)
		return;


	MComboBox* pCombo1 = (MComboBox*)m_IDLResource.FindWidget("112_ConfirmID");
	MComboBox* pCombo2 = (MComboBox*)m_IDLResource.FindWidget("112_ConfirmReason");

	if (!pCombo1 || !pCombo2)
		return;

	pCombo1->RemoveAll();
	pCombo2->SetSelIndex(0);


	switch (m_nState)
	{
	case GUNZ_LOBBY:
	{
		ZPlayerListBox *pPlayerListBox = (ZPlayerListBox*)m_IDLResource.FindWidget("LobbyChannelPlayerList");
		if (pPlayerListBox)
		{
			for (int i = 0; i < pPlayerListBox->GetCount(); i++)
				pCombo1->Add(pPlayerListBox->GetPlayerName(i));
		}
	}
	break;

	case GUNZ_STAGE:
	{
		ZPlayerListBox *pPlayerListBox = (ZPlayerListBox*)m_IDLResource.FindWidget("StagePlayerList_");
		if (pPlayerListBox)
		{
			for (int i = 0; i < pPlayerListBox->GetCount(); i++)
				pCombo1->Add(pPlayerListBox->GetPlayerName(i));
		}
	}
	break;

	case GUNZ_GAME:
	{
		for (ZCharacterManager::iterator itor = ZGetCharacterManager()->begin(); itor != ZGetCharacterManager()->end(); itor++)
			pCombo1->Add((*itor).second->GetUserName());
	}
	break;
	}

	pCombo1->SetSelIndex(0);
}


void ZGameInterface::RequestQuickJoin()
{
	MTD_QuickJoinParam	quick_join_param;

	quick_join_param.nMapEnum = 0xFFFFFFFF;

	quick_join_param.nModeEnum = 0;
	SetBitSet(quick_join_param.nModeEnum, MMATCH_GAMETYPE_DEATHMATCH_SOLO);
	SetBitSet(quick_join_param.nModeEnum, MMATCH_GAMETYPE_DEATHMATCH_TEAM);
	SetBitSet(quick_join_param.nModeEnum, MMATCH_GAMETYPE_ASSASSINATE);
	SetBitSet(quick_join_param.nModeEnum, MMATCH_GAMETYPE_GUNGAME);

	ZPostRequestQuickJoin(ZGetGameClient()->GetPlayerUID(), &quick_join_param);
}

void ZGameInterface::InitClanLobbyUI(bool bClanBattleEnable)
{
	OnArrangedTeamGameUI(false);

	MWidget *pWidget;

	pWidget = m_IDLResource.FindWidget("StageJoin");
	if (pWidget) pWidget->Show(!bClanBattleEnable);
	pWidget = m_IDLResource.FindWidget("StageCreateFrameCaller");
	if (pWidget) pWidget->Show(!bClanBattleEnable);

	pWidget = m_IDLResource.FindWidget("ArrangedTeamGame");
	if (pWidget) pWidget->Show(bClanBattleEnable);

	m_CombatMenu.EnableItem(ZCombatMenu::ZCMI_BATTLE_EXIT, !bClanBattleEnable);

	pWidget = m_IDLResource.FindWidget("QuickJoin");
	if (pWidget) pWidget->Show(!bClanBattleEnable);

	pWidget = m_IDLResource.FindWidget("QuickJoin2");
	if (pWidget) pWidget->Show(!bClanBattleEnable);

	bool bClanServer = ( (ZGetGameClient()->GetServerMode() == MSM_CLAN) || (ZGetGameClient()->GetServerMode() == MSM_TEST) );

	pWidget = m_IDLResource.FindWidget("PrivateChannelInput");
	if (pWidget) pWidget->Show(bClanServer);

	pWidget = m_IDLResource.FindWidget("PrivateChannelEnter");
	if (pWidget) pWidget->Show(bClanServer);

	pWidget = m_IDLResource.FindWidget("Lobby_ClanInfoBG");
	if (pWidget) pWidget->Show(bClanBattleEnable);

	pWidget = m_IDLResource.FindWidget("Lobby_ClanList");
	if (pWidget) pWidget->Show(bClanBattleEnable);
}

void ZGameInterface::InitChannelFrame(MCHANNEL_TYPE nChannelType)
{
	MWidget* pWidget;

	pWidget = m_IDLResource.FindWidget("PrivateChannelInput");
	if (pWidget) pWidget->Show(nChannelType == MCHANNEL_TYPE_USER);
	pWidget = m_IDLResource.FindWidget("PrivateChannelEnter");
	if (pWidget) pWidget->Show(nChannelType == MCHANNEL_TYPE_USER);
	pWidget = m_IDLResource.FindWidget("MyClanChannel");
	if (pWidget) pWidget->Show(nChannelType == MCHANNEL_TYPE_CLAN);

	MListBox* pListBox = (MListBox*)m_IDLResource.FindWidget("ChannelList");
	if (pListBox) pListBox->RemoveAll();
}

void ZGameInterface::InitLadderUI(bool bLadderEnable)
{
	OnArrangedTeamGameUI(false);

	MWidget *pWidget;

	pWidget = m_IDLResource.FindWidget("StageJoin");
	if (pWidget) pWidget->Show(!bLadderEnable);
	pWidget = m_IDLResource.FindWidget("StageCreateFrameCaller");
	if (pWidget) pWidget->Show(!bLadderEnable);

	pWidget = m_IDLResource.FindWidget("ArrangedTeamGame");
	if (pWidget) pWidget->Show(bLadderEnable);

	m_CombatMenu.EnableItem(ZCombatMenu::ZCMI_BATTLE_EXIT, !bLadderEnable);

	bool bLadderServer =
		ZGetGameClient()->GetServerMode() == MSM_CLAN ||
		ZGetGameClient()->GetServerMode() == MSM_LADDER ||
		ZGetGameClient()->GetServerMode() == MSM_EVENT ||
		ZGetGameClient()->GetServerMode() == MSM_TEST;

	pWidget = m_IDLResource.FindWidget("PrivateChannelInput");
	if (pWidget) pWidget->Show(bLadderServer);

	pWidget = m_IDLResource.FindWidget("PrivateChannelEnter");
	if (pWidget) pWidget->Show(bLadderServer);

}

void ZGameInterface::OnArrangedTeamGameUI(bool bFinding)
{
	MWidget *pWidget;

	pWidget = m_IDLResource.FindWidget("ArrangedTeamGame");
	if (pWidget) pWidget->Show(!bFinding);

	pWidget = m_IDLResource.FindWidget("LobbyFindClanTeam");
	if (pWidget != NULL) pWidget->Show(bFinding);

#define SAFE_ENABLE(x) { pWidget= m_IDLResource.FindWidget( x ); if(pWidget) pWidget->Enable(!bFinding); }

	SAFE_ENABLE("LobbyChannelPlayerList");
	SAFE_ENABLE("LobbyShopCaller");
	SAFE_ENABLE("LobbyEquipmentCaller");
	SAFE_ENABLE("ChannelListFrameCaller");
	SAFE_ENABLE("LobbyOptionFrame");
	SAFE_ENABLE("Logout");
	SAFE_ENABLE("ReplayCaller");
	SAFE_ENABLE("CharSelectionCaller");
	SAFE_ENABLE("QuickJoin");
	SAFE_ENABLE("QuickJoin2");

	m_bWaitingArrangedGame = bFinding;
}

bool ZGameInterface::IsReadyToPropose()
{
	if (GetState() != GUNZ_LOBBY)
		return false;

	if (m_bWaitingArrangedGame)
		return false;

	if (GetLatestExclusive() != NULL)
		return false;

	if (m_pMsgBox->IsVisible())
		return false;

	return true;
}

bool ZGameInterface::IsMiniMapEnable()
{
	return GetCamera()->GetLookMode() == ZCAMERA_MINIMAP;
}

bool ZGameInterface::OpenMiniMap()
{
	if (!m_pMiniMap) {
		m_pMiniMap = new ZMiniMap;
		if (!m_pMiniMap->Create(ZGetGameClient()->GetMatchStageSetting()->GetMapName()))
		{
			SAFE_DELETE(m_pMiniMap);
			return false;
		}
	}

	return true;
}

string GetRestrictionString(MMatchItemDesc* pItemDesc)
{
	string str;

	char temp[1024];

	bool bAdd = false;

	if (pItemDesc->m_nResSex != -1)
	{
		if (pItemDesc->m_nResSex == 0)
			sprintf_safe(temp, "%s\n", ZMsg(MSG_WORD_FORMEN));
		else if (pItemDesc->m_nResSex == 1)
			sprintf_safe(temp, "%s\n", ZMsg(MSG_WORD_FORWOMEN));
		str += temp;
	}

	if (pItemDesc->m_nResLevel)
	{
		if (pItemDesc->m_nResLevel > ZGetMyInfo()->GetLevel())
			sprintf_safe(temp, "%s : ^1%d ^0%s\n", ZMsg(MSG_WORD_LIMITEDLEVEL), pItemDesc->m_nResLevel, ZMsg(MSG_CHARINFO_LEVELMARKER));
		else
			sprintf_safe(temp, "%s : %d %s\n", ZMsg(MSG_WORD_LIMITEDLEVEL), pItemDesc->m_nResLevel, ZMsg(MSG_CHARINFO_LEVELMARKER));
		str += temp;
		bAdd = true;
	}

	if (pItemDesc->m_nWeight)
	{
		sprintf_safe(temp, "%s : %d Wt.", ZMsg(MSG_WORD_WEIGHT), pItemDesc->m_nWeight);
		str += temp;
		bAdd = true;
	}

	return str;
}

string GetItemSpecString(MMatchItemDesc* pItemDesc)
{
	string str;
	char temp[1024];
	bool bAdd = false;

	if (pItemDesc->IsEnchantItem())
	{
		switch (pItemDesc->m_nWeaponType)
		{
		case MWT_ENCHANT_FIRE:
			sprintf_safe(temp, "<%s>\n", ZMsg(MSG_WORD_ATTRIBUTE_FIRE));
			str += temp;
			sprintf_safe(temp, "%s : %d%s\n", ZMsg(MSG_WORD_RUNTIME), pItemDesc->m_nDelay / 1000, ZMsg(MSG_CHARINFO_SECOND));
			str += temp;
			if (pItemDesc->m_nDamage)
			{
				sprintf_safe(temp, "%s : %d dmg/%s\n", ZMsg(MSG_WORD_DAMAGE), pItemDesc->m_nDamage, ZMsg(MSG_CHARINFO_SECOND));
				str += temp;
			}
			break;

		case MWT_ENCHANT_COLD:
			sprintf_safe(temp, "<%s>\n", ZMsg(MSG_WORD_ATTRIBUTE_COLD));
			str += temp;
			sprintf_safe(temp, "%s : %d%s\n", ZMsg(MSG_WORD_RUNTIME), pItemDesc->m_nDelay / 1000, ZMsg(MSG_CHARINFO_SECOND));
			str += temp;
			if (pItemDesc->m_nLimitSpeed)
			{
				sprintf_safe(temp, "%s -%d%%\n", ZMsg(MSG_WORD_RUNSPEED), 100 - pItemDesc->m_nLimitSpeed);
				str += temp;
			}
			sprintf_safe(temp, "%s\n", ZMsg(MSG_WORD_DONOTDASH));
			str += temp;
			sprintf_safe(temp, "%s\n", ZMsg(MSG_WORD_DONOTHANGWALL));
			str += temp;
			break;

		case MWT_ENCHANT_LIGHTNING:
			sprintf_safe(temp, "<%s>\n", ZMsg(MSG_WORD_ATTRIBUTE_LIGHTNING));
			str += temp;
			sprintf_safe(temp, "%s : %d%s\n", ZMsg(MSG_WORD_RUNTIME), pItemDesc->m_nDelay / 1000, ZMsg(MSG_CHARINFO_SECOND));
			str += temp;
			if (pItemDesc->m_nDamage)
			{
				sprintf_safe(temp, "%s : %d dmg.\n", ZMsg(MSG_WORD_ATTACK), pItemDesc->m_nDamage);
				str += temp;
			}
			break;

		case MWT_ENCHANT_POISON:
			sprintf_safe(temp, "<%s>\n", ZMsg(MSG_WORD_ATTRIBUTE_POISON));
			str += temp;
			sprintf_safe(temp, "%s : %d%s\n", ZMsg(MSG_WORD_RUNTIME), pItemDesc->m_nDelay / 1000, ZMsg(MSG_CHARINFO_SECOND));
			str += temp;
			if (pItemDesc->m_nDamage)
			{
				sprintf_safe(temp, "%s : %d dmg/%s\n", ZMsg(MSG_WORD_DAMAGE), pItemDesc->m_nDamage, ZMsg(MSG_CHARINFO_SECOND));
				str += temp;
			}
			break;
		}
	}
	else
	{
		if (pItemDesc->m_nMaxBullet) {
			sprintf_safe(temp, "%s : %d", ZMsg(MSG_WORD_BULLET), pItemDesc->m_nMagazine);

			if ((pItemDesc->m_nMaxBullet / pItemDesc->m_nMagazine) > 1)
				sprintf_safe(temp, "%s x %d", temp, pItemDesc->m_nMaxBullet / pItemDesc->m_nMagazine);

			str += temp;
			str += "\n";
			bAdd = true;
		}

		if (pItemDesc->m_nDamage) {
			sprintf_safe(temp, "%s : %d dmg.\n", ZMsg(MSG_WORD_ATTACK), pItemDesc->m_nDamage);
			str += temp;
			bAdd = true;
		}

		if (pItemDesc->m_nDelay) {
			sprintf_safe(temp, "%s : %d\n", ZMsg(MSG_WORD_DELAY), pItemDesc->m_nDelay);
			str += temp;
			bAdd = true;
		}

		// HP
		if (pItemDesc->m_nHP) {
			sprintf_safe(temp, "%s +%d\n", ZMsg(MSG_CHARINFO_HP), pItemDesc->m_nHP);
			str += temp;
			bAdd = true;
		}


		// AP
		if (pItemDesc->m_nAP) {
			sprintf_safe(temp, "%s +%d\n", ZMsg(MSG_CHARINFO_AP), pItemDesc->m_nAP);
			str += temp;
			bAdd = true;
		}


		if (pItemDesc->m_nMaxWT) {
			sprintf_safe(temp, "%s +%d\n", ZMsg(MSG_WORD_MAXWEIGHT), pItemDesc->m_nMaxWT);
			str += temp;
			bAdd = true;
		}

		if (pItemDesc->m_nSF) {
			sprintf_safe(temp, "SF +%d\n", pItemDesc->m_nSF);
			str += temp;
			bAdd = true;
		}

		if (pItemDesc->m_nFR) {
			sprintf_safe(temp, "FR +%d\n", pItemDesc->m_nFR);
			str += temp;
			bAdd = true;
		}

		if (pItemDesc->m_nCR) {
			sprintf_safe(temp, "CR +%d\n", pItemDesc->m_nCR);
			str += temp;
			bAdd = true;
		}

		if (pItemDesc->m_nPR) {
			sprintf_safe(temp, "PR +%d\n", pItemDesc->m_nPR);
			str += temp;
			bAdd = true;
		}

		if (pItemDesc->m_nLR) {
			sprintf_safe(temp, "LR +%d\n", pItemDesc->m_nLR);
			str += temp;
			bAdd = true;
		}

		str += "\n";

		if (pItemDesc->m_nLimitSpeed != 100) {
			sprintf_safe(temp, "%s -%d%%\n", ZMsg(MSG_WORD_RUNSPEED), 100 - pItemDesc->m_nLimitSpeed);
			str += temp;
			bAdd = true;
		}

		if (pItemDesc->m_nLimitJump) {
			sprintf_safe(temp, "%s\n", ZMsg(MSG_WORD_DONOTJUMP));
			str += temp;
			bAdd = true;
		}

		if (pItemDesc->m_nLimitTumble) {
			sprintf_safe(temp, "%s\n", ZMsg(MSG_WORD_DONOTDASH));
			str += temp;
			bAdd = true;
		}

		if (pItemDesc->m_nLimitWall) {
			sprintf_safe(temp, "%s\n", ZMsg(MSG_WORD_DONOTHANGWALL));
			str += temp;
			bAdd = true;
		}
	}

	return str;
}

string GetPeriodItemString(ZMyItemNode* pRentalNode)
{
	string str = "";
	char temp[1024];

	if (pRentalNode != NULL)
	{
		if ((pRentalNode->GetRentMinutePeriodRemainder() < RENT_MINUTE_PERIOD_UNLIMITED))
		{
			DWORD dwRemaind = pRentalNode->GetRentMinutePeriodRemainder();
			DWORD dwRecievedClock = (GetGlobalTimeMS() - pRentalNode->GetWhenReceivedClock()) / 60000;

			if (dwRemaind < dwRecievedClock)
			{
				str = ZMsg(MSG_EXPIRED);
			}
			else
			{
				dwRemaind -= dwRecievedClock;

				if (dwRemaind > 1440)
				{
					sprintf_safe(temp, "%d%s ", dwRemaind / 1440, ZMsg(MSG_CHARINFO_DAY));
					str += temp;
					dwRemaind %= 1440;
				}

				sprintf_safe(temp, "%d%s ", dwRemaind / 60, ZMsg(MSG_CHARINFO_HOUR));
				str += temp;
				dwRemaind %= 60;

				sprintf_safe(temp, "%d%s", dwRemaind, ZMsg(MSG_CHARINFO_MINUTE));
				str += temp;

				str += "  ";
				str += ZMsg(MSG_REMAIND_PERIOD);
			}
		}
	}

	return str;
}

void ZGameInterface::SetupItemDescription(MMatchItemDesc* pItemDesc, const char *szTextArea1, const char *szTextArea2, const char *szTextArea3, const char *szIcon, ZMyItemNode* pRentalNode)
{
	if (!pItemDesc) return;

	MTextArea* pTextArea1 = (MTextArea*)ZGetGameInterface()->GetIDLResource()->FindWidget(szTextArea1);
	MTextArea* pTextArea2 = (MTextArea*)ZGetGameInterface()->GetIDLResource()->FindWidget(szTextArea2);
	MTextArea* pTextArea3 = (MTextArea*)ZGetGameInterface()->GetIDLResource()->FindWidget(szTextArea3);
	if (pTextArea1)
	{
		pTextArea1->SetTextColor(MCOLOR(255, 255, 255));
		pTextArea1->SetText(pItemDesc->m_szName);

		pTextArea1->AddText("\n\n\n");
		pTextArea1->AddText(GetRestrictionString(pItemDesc).c_str(), MCOLOR(170, 170, 170));

		pTextArea1->AddText("\n");
		pTextArea1->AddText(GetItemSpecString(pItemDesc).c_str(), MCOLOR(170, 170, 170));
	}

	if (pTextArea2)
	{
		pTextArea2->SetTextColor(MCOLOR(200, 0, 0));
		pTextArea2->SetText(GetPeriodItemString(pRentalNode).c_str());
	}

	if (pTextArea3)
	{
		pTextArea3->SetTextColor(MCOLOR(120, 120, 120));
		pTextArea3->SetText(pItemDesc->m_szDesc);
	}

	MPicture* pItemIcon = (MPicture*)ZGetGameInterface()->GetIDLResource()->FindWidget(szIcon);
	if (pItemIcon)
		pItemIcon->SetBitmap(GetItemIconBitmap(pItemDesc, true));

	int PrevHP = 0;
	int PrevAP = 0;
	int PrevWeight = 0;
	int PrevMaxWeight = 0;
	[&]()
	{
		ZMyItemNode* pMyItemNode = ZGetMyInfo()->GetItemList()->GetEquipedItem(GetSuitableItemParts(pItemDesc->m_nSlot));
		if (!pMyItemNode)
			return;

		auto MyItemDesc = MGetMatchItemDescMgr()->GetItemDesc(pMyItemNode->GetItemID());
		if (!MyItemDesc)
			return;

		PrevHP = MyItemDesc->m_nHP;
		PrevAP = MyItemDesc->m_nAP;
		PrevWeight = MyItemDesc->m_nWeight;
		PrevMaxWeight = MyItemDesc->m_nMaxWT;
	}();
	;
	char szName[128], szBuff[128];
	strcpy_safe(szName, ((strncmp(szTextArea1, "Equip", 5) == 0) ? "Equip_MyInfo" : "Shop_MyInfo"));
	MTextArea* pTextArea = (MTextArea*)ZGetGameInterface()->GetIDLResource()->FindWidget(szName);
	if (pTextArea)
	{
		pTextArea->Clear();

		if (pItemDesc->m_nResLevel > ZGetMyInfo()->GetLevel())
			sprintf_safe(szBuff, "^9%s : ^1%d ^9%s", ZMsg(MSG_CHARINFO_LEVEL),
				ZGetMyInfo()->GetLevel(), ZMsg(MSG_CHARINFO_LEVELMARKER));
		else
			sprintf_safe(szBuff, "^9%s : %d %s", ZMsg(MSG_CHARINFO_LEVEL),
				ZGetMyInfo()->GetLevel(), ZMsg(MSG_CHARINFO_LEVELMARKER));
		pTextArea->AddText(szBuff);


		if (pItemDesc->m_nBountyPrice > ZGetMyInfo()->GetBP())
			sprintf_safe(szBuff, "^9%s : ^1%d", ZMsg(MSG_CHARINFO_BOUNTY), ZGetMyInfo()->GetBP());
		else
			sprintf_safe(szBuff, "^9%s : %d", ZMsg(MSG_CHARINFO_BOUNTY), ZGetMyInfo()->GetBP());
		pTextArea->AddText(szBuff);


		if (pItemDesc->m_nHP > PrevHP)
			sprintf_safe(szBuff, "^9%s : %d ^2+%d", ZMsg(MSG_CHARINFO_HP),
				ZGetMyInfo()->GetHP(), pItemDesc->m_nHP - PrevHP);
		else if (pItemDesc->m_nHP < PrevHP)
			sprintf_safe(szBuff, "^9%s : %d ^1-%d",
				ZMsg(MSG_CHARINFO_HP), ZGetMyInfo()->GetHP(), PrevHP - pItemDesc->m_nHP);
		else
			sprintf_safe(szBuff, "^9%s : %d", ZMsg(MSG_CHARINFO_HP), ZGetMyInfo()->GetHP());
		pTextArea->AddText(szBuff);


		if (pItemDesc->m_nAP > PrevAP)
			sprintf_safe(szBuff, "^9%s : %d ^2+%d", ZMsg(MSG_CHARINFO_AP),
				ZGetMyInfo()->GetAP(), pItemDesc->m_nAP - PrevAP);
		else if (pItemDesc->m_nAP < PrevAP)
			sprintf_safe(szBuff, "^9%s : %d ^1-%d", ZMsg(MSG_CHARINFO_AP),
				ZGetMyInfo()->GetAP(), PrevAP - pItemDesc->m_nAP);
		else
			sprintf_safe(szBuff, "^9%s : %d", ZMsg(MSG_CHARINFO_AP), ZGetMyInfo()->GetAP());
		pTextArea->AddText(szBuff);


		if ((ZGetMyInfo()->GetItemList()->GetEquipedTotalWeight() - PrevWeight + pItemDesc->m_nWeight)
		> ZGetMyInfo()->GetItemList()->GetMaxWeight())
			sprintf_safe(szBuff, "^9%s : ^1%d^9/", ZMsg(MSG_CHARINFO_WEIGHT),
				ZGetMyInfo()->GetItemList()->GetEquipedTotalWeight());
		else
			sprintf_safe(szBuff, "^9%s : %d/", ZMsg(MSG_CHARINFO_WEIGHT),
				ZGetMyInfo()->GetItemList()->GetEquipedTotalWeight());

		char chTmp[32];
		if (pItemDesc->m_nMaxWT > PrevMaxWeight)
			sprintf_safe(chTmp, "%d ^2+%d", ZGetMyInfo()->GetItemList()->GetMaxWeight(),
				pItemDesc->m_nMaxWT - PrevMaxWeight);
		else if (pItemDesc->m_nMaxWT < PrevMaxWeight)
			sprintf_safe(chTmp, "%d ^1-%d", ZGetMyInfo()->GetItemList()->GetMaxWeight(),
				PrevMaxWeight - pItemDesc->m_nMaxWT);
		else
			sprintf_safe(chTmp, "%d", ZGetMyInfo()->GetItemList()->GetMaxWeight());
		strcat_safe(szBuff, chTmp);
		pTextArea->AddText(szBuff);
	}
}

void ZGameInterface::SetupItemDescription(MQuestItemDesc* pItemDesc, const char *szTextArea1, const char *szTextArea2, const char *szTextArea3, const char *szIcon)
{
	if (!pItemDesc) return;

	MTextArea* pTextArea1 = (MTextArea*)ZGetGameInterface()->GetIDLResource()->FindWidget(szTextArea1);
	MTextArea* pTextArea2 = (MTextArea*)ZGetGameInterface()->GetIDLResource()->FindWidget(szTextArea2);
	MTextArea* pTextArea3 = (MTextArea*)ZGetGameInterface()->GetIDLResource()->FindWidget(szTextArea3);
	if (pTextArea1)
	{
		pTextArea1->SetTextColor(MCOLOR(255, 255, 255));
		pTextArea1->SetText(pItemDesc->m_szQuestItemName);
		pTextArea1->AddText("\n");
		char szText[256];
		sprintf_safe(szText, "(%s %s)", ZMsg(MSG_WORD_QUEST), ZMsg(MSG_WORD_ITEM));
		pTextArea1->AddText(szText, MCOLOR(170, 170, 170));
		pTextArea1->AddText("\n\n");

		if (pItemDesc->m_bSecrifice)
		{
			sprintf_safe(szText, "%s %s", ZMsg(MSG_WORD_SACRIFICE), ZMsg(MSG_WORD_ITEM));
			pTextArea1->AddText(szText, MCOLOR(170, 170, 170));
		}
	}

	if (pTextArea2)
	{
		pTextArea2->SetText("");
	}

	if (pTextArea3)
	{
		pTextArea3->SetTextColor(MCOLOR(120, 120, 120));
		pTextArea3->SetText(pItemDesc->m_szDesc);
	}

	MPicture* pItemIcon = (MPicture*)ZGetGameInterface()->GetIDLResource()->FindWidget(szIcon);
	if (pItemIcon)
		pItemIcon->SetBitmap(GetQuestItemIcon(pItemDesc->m_nItemID, true));
	MTextArea* pTextArea = (MTextArea*)m_IDLResource.FindWidget("Shop_MyInfo");
	if (pTextArea)
	{
		pTextArea->Clear();

		char szTemp[64];
		sprintf_safe(szTemp, "^9%s : %d %s", ZMsg(MSG_CHARINFO_LEVEL), ZGetMyInfo()->GetLevel(), ZMsg(MSG_CHARINFO_LEVELMARKER));
		pTextArea->AddText(szTemp);

		sprintf_safe(szTemp, "^9%s : %d", ZMsg(MSG_CHARINFO_BOUNTY), ZGetMyInfo()->GetBP());
		pTextArea->AddText(szTemp);

		sprintf_safe(szTemp, "^9%s : %d", ZMsg(MSG_CHARINFO_HP), ZGetMyInfo()->GetHP());
		pTextArea->AddText(szTemp);

		sprintf_safe(szTemp, "^9%s : %d", ZMsg(MSG_CHARINFO_AP), ZGetMyInfo()->GetAP());
		pTextArea->AddText(szTemp);

		sprintf_safe(szTemp, "^9%s : %d/%d", ZMsg(MSG_CHARINFO_WEIGHT), ZGetMyInfo()->GetItemList()->GetEquipedTotalWeight(), ZGetMyInfo()->GetItemList()->GetMaxWeight());
		pTextArea->AddText(szTemp);
	}

	pTextArea = (MTextArea*)m_IDLResource.FindWidget("Equip_MyInfo");
	if (pTextArea)
	{
		pTextArea->Clear();

		char szTemp[64];
		sprintf_safe(szTemp, "^9%s : %d %s", ZMsg(MSG_CHARINFO_LEVEL), ZGetMyInfo()->GetLevel(), ZMsg(MSG_CHARINFO_LEVELMARKER));
		pTextArea->AddText(szTemp);

		sprintf_safe(szTemp, "^9%s : %d", ZMsg(MSG_CHARINFO_BOUNTY), ZGetMyInfo()->GetBP());
		pTextArea->AddText(szTemp);

		sprintf_safe(szTemp, "^9%s : %d", ZMsg(MSG_CHARINFO_HP), ZGetMyInfo()->GetHP());
		pTextArea->AddText(szTemp);

		sprintf_safe(szTemp, "^9%s : %d", ZMsg(MSG_CHARINFO_AP), ZGetMyInfo()->GetAP());
		pTextArea->AddText(szTemp);

		sprintf_safe(szTemp, "^9%s : %d/%d", ZMsg(MSG_CHARINFO_WEIGHT), ZGetMyInfo()->GetItemList()->GetEquipedTotalWeight(), ZGetMyInfo()->GetItemList()->GetMaxWeight());
		pTextArea->AddText(szTemp);
	}
}


void ZGameInterface::GetBringAccountItem()
{
	int nResult = CheckRestrictBringAccountItem();
	if (nResult == -1)		// Error
		ShowErrorMessage(MERR_NO_SELITEM);

	else if (nResult == 0)	// Restriction Passed
	{
		BringAccountItem();

		MButton* pButton = (MButton*)m_IDLResource.FindWidget("BringAccountItemBtn");
		if (pButton)
		{
			pButton->Enable(false);
			pButton->Show(false);
			pButton->Show(true);
		}
	}

	else if (nResult == 1)	// Sex Restrict
		ShowErrorMessage(MERR_BRING_ACCOUNTITEM_BECAUSEOF_SEX);

	else if (nResult == 2)	// Level Restrict
		ShowConfirmMessage(ZErrStr(MERR_BRING_CONFIRM_ACCOUNTITEM_BECAUSEOF_LEVEL),
			ZGetLevelConfirmListenter());
	else
		_ASSERT(FALSE);	// Unknown Restriction
}

class ReplayListBoxItem : public MListItem
{
public:
	ReplayListBoxItem(const StringView& szName, const StringView& szVersion)
	{
		strcpy_safe(m_szName, szName);
		strcpy_safe(m_szVersion, szVersion);
	}

	virtual const char* GetString()
	{
		return m_szName;
	}
	virtual const char* GetString(int i)
	{
		if (i == 0)
			return m_szName;
		else if (i == 1)
			return m_szVersion;

		return NULL;
	}
	virtual MBitmap* GetBitmap(int i)
	{
		return NULL;
	}

protected:
	char		m_szName[128];
	char		m_szVersion[10];
};

void ZGameInterface::ShowReplayDialog(bool bShow)
{
	if (bShow)
	{
		MWidget* pWidget;
		pWidget = (MWidget*)m_IDLResource.FindWidget("Replay");
		if (pWidget)
			pWidget->Show(true, true);

		pWidget = (MWidget*)m_IDLResource.FindWidget("Replay_View");
		if (pWidget)
			pWidget->Enable(false);

		MListBox* pListBox = (MListBox*)m_IDLResource.FindWidget("Replay_FileList");
		if (pListBox)
		{
			pListBox->RemoveAll();

			std::string szPath = GetMyDocumentsPath();
			szPath += GUNZ_FOLDER;
			szPath += REPLAY_FOLDER;
			szPath += "/*.gzr";
			MakePath(szPath.c_str());

			for (auto&& FileData : MFile::Glob(szPath.c_str()))
				pListBox->Add(new ReplayListBoxItem(FileData.Name, "")); // Add to listbox

			pListBox->Sort();
		}
	}
	else
	{
		ShowWidget("Replay", false);
	}
}

void ZGameInterface::ViewReplay()
{
	ShowReplayDialog(false);

	MListBox* pListBox = (MListBox*)m_IDLResource.FindWidget("Replay_FileList");
	if (!pListBox)
		return;

	if (pListBox->GetSelItemString() == NULL)
		return;

	std::string szName = GetMyDocumentsPath();
	szName += GUNZ_FOLDER;
	szName += REPLAY_FOLDER;
	szName += "/";
	szName += pListBox->GetSelItemString();

	EnteredReplayFromLogin = GetState() == GUNZ_LOGIN;
	if (EnteredReplayFromLogin)
		OnLoginDestroy();

	m_bOnEndOfReplay = true;
	m_nLevelPercentCache = ZGetMyInfo()->GetLevelPercent();

	if (!CreateReplayGame(szName.c_str()))
		ZApplication::GetGameInterface()->ShowMessage("Can't Open Replay File");

	m_CombatMenu.EnableItem(ZCombatMenu::ZCMI_BATTLE_EXIT, false);
}

void ZGameInterface::SetKindableItem(MMatchItemSlotType nSlotType)
{
	ZItemSlotView* itemSlot;
	for (int i = 0; i < MMCIP_END; i++)
	{
		itemSlot = (ZItemSlotView*)m_IDLResource.FindWidget(GetItemSlotName("Equip", i));
		if (itemSlot)
		{
			bool bKindable = false;
			switch (nSlotType)
			{
			case MMIST_MELEE:
				if (itemSlot->GetParts() == MMCIP_MELEE)
					bKindable = true;
				break;
			case MMIST_RANGE:
				if ((itemSlot->GetParts() == MMCIP_PRIMARY) || (itemSlot->GetParts() == MMCIP_SECONDARY))
					bKindable = true;
				break;
			case MMIST_CUSTOM:
				if ((itemSlot->GetParts() == MMCIP_CUSTOM1) || (itemSlot->GetParts() == MMCIP_CUSTOM2))
					bKindable = true;
				break;
			case MMIST_HEAD:
				if (itemSlot->GetParts() == MMCIP_HEAD)
					bKindable = true;
				break;
			case MMIST_CHEST:
				if (itemSlot->GetParts() == MMCIP_CHEST)
					bKindable = true;
				break;
			case MMIST_HANDS:
				if (itemSlot->GetParts() == MMCIP_HANDS)
					bKindable = true;
				break;
			case MMIST_LEGS:
				if (itemSlot->GetParts() == MMCIP_LEGS)
					bKindable = true;
				break;
			case MMIST_FEET:
				if (itemSlot->GetParts() == MMCIP_FEET)
					bKindable = true;
				break;
			case MMIST_FINGER:
				if ((itemSlot->GetParts() == MMCIP_FINGERL) || (itemSlot->GetParts() == MMCIP_FINGERR))
					bKindable = true;
				break;
			}

			itemSlot->SetKindable(bKindable);
		}
	}
}

#ifdef _QUEST_ITEM
void ZGameInterface::OnResponseCharacterItemList_QuestItem(MTD_QuestItemNode* pQuestItemNode, int nQuestItemCount)
{
	if (0 == pQuestItemNode)
		return;

	ZGetMyInfo()->GetItemList()->SetQuestItemsAll(pQuestItemNode, nQuestItemCount);

	ZApplication::GetStageInterface()->SerializeSacrificeItemListBox();
	ZGetMyInfo()->GetItemList()->Serialize();
}

void ZGameInterface::OnResponseBuyQuestItem(const int nResult, const int nBP)
{
	if (MOK == nResult)
	{
		ZApplication::GetGameInterface()->ShowMessage(MSG_GAME_BUYITEM);

		ZGetMyInfo()->SetBP(nBP);
	}
	else if (MERR_TOO_MANY_ITEM == nResult)
	{
		ZApplication::GetGameInterface()->ShowErrorMessage(nResult);
	}
	else if (MERR_TOO_EXPENSIVE_BOUNTY == nResult)
	{
		ZApplication::GetGameInterface()->ShowErrorMessage(nResult);
	}
	else
	{
		mlog("ZGameInterface::OnCommand::MC_MATCH_RESPONSE_BUY_QUEST_ITEM - 정의되지 않은 결과처리.\n");
		ASSERT(0);
	}
}

void ZGameInterface::OnResponseSellQuestItem(const int nResult, const int nBP)
{
	if (MOK == nResult)
	{
		ZApplication::GetGameInterface()->ShowMessage(MSG_GAME_SELLITEM);

		ZGetMyInfo()->SetBP(nBP);
	}
	else
	{
	}
}
#endif

MBitmap* ZGameInterface::GetQuestItemIcon(int nItemID, bool bSmallIcon)
{
	char szFileName[64] = "";
	switch (nItemID)
	{
		// Page
	case 200001:	strcpy_safe(szFileName, "slot_icon_page"); break; // 13
	case 200002:	strcpy_safe(szFileName, "slot_icon_page"); break; // 25
	case 200003:	strcpy_safe(szFileName, "slot_icon_page"); break; // 41
	case 200004:	strcpy_safe(szFileName, "slot_icon_page"); break; // 65

	// Skull
	case 200005:	strcpy_safe(szFileName, "slot_icon_skull"); break;
	case 200006:	strcpy_safe(szFileName, "slot_icon_skull"); break;
	case 200007:	strcpy_safe(szFileName, "slot_icon_skull"); break;
	case 200008:	strcpy_safe(szFileName, "slot_icon_skull"); break;
	case 200009:	strcpy_safe(szFileName, "slot_icon_skull"); break;
	case 200010:	strcpy_safe(szFileName, "slot_icon_skull"); break;

		// Fresh
	case 200011:	strcpy_safe(szFileName, "slot_icon_fresh"); break;
	case 200012:	strcpy_safe(szFileName, "slot_icon_fresh"); break;
	case 200013:	strcpy_safe(szFileName, "slot_icon_fresh"); break;

		// Ring
	case 200014:	strcpy_safe(szFileName, "slot_icon_ring"); break;
	case 200015:	strcpy_safe(szFileName, "slot_icon_ring"); break;
	case 200016:	strcpy_safe(szFileName, "slot_icon_ring"); break;
	case 200017:	strcpy_safe(szFileName, "slot_icon_ring"); break;

		// Necklace
	case 200018:	strcpy_safe(szFileName, "slot_icon_neck"); break;

		// Doll
	case 200019:	strcpy_safe(szFileName, "slot_icon_doll"); break;
	case 200020:	strcpy_safe(szFileName, "slot_icon_doll"); break;
	case 200021:	strcpy_safe(szFileName, "slot_icon_doll"); break;
	case 200022:	strcpy_safe(szFileName, "slot_icon_doll"); break;
	case 200023:	strcpy_safe(szFileName, "slot_icon_doll"); break;
	case 200024:	strcpy_safe(szFileName, "slot_icon_doll"); break;
	case 200025:	strcpy_safe(szFileName, "slot_icon_doll"); break;
	case 200026:	strcpy_safe(szFileName, "slot_icon_doll"); break;
	case 200027:	strcpy_safe(szFileName, "slot_icon_doll"); break;

		// Book
	case 200028:	strcpy_safe(szFileName, "slot_icon_book"); break;
	case 200029:	strcpy_safe(szFileName, "slot_icon_book"); break;
	case 200030:	strcpy_safe(szFileName, "slot_icon_book"); break;

		// Object
	case 200031:	strcpy_safe(szFileName, "slot_icon_object"); break;
	case 200032:	strcpy_safe(szFileName, "slot_icon_object"); break;
	case 200033:	strcpy_safe(szFileName, "slot_icon_object"); break;
	case 200034:	strcpy_safe(szFileName, "slot_icon_object"); break;
	case 200035:	strcpy_safe(szFileName, "slot_icon_object"); break;
	case 200036:	strcpy_safe(szFileName, "slot_icon_object"); break;
	case 200037:	strcpy_safe(szFileName, "slot_icon_object"); break;

		// Sword
	case 200038:	strcpy_safe(szFileName, "slot_icon_qsword"); break;
	}

	strcat_safe(szFileName, ".tga");

	return MBitmapManager::Get(szFileName);
}


void ZGameInterface::OnResponseServerStatusInfoList(const int nListCount, void* pBlob)
{
	DMLog("OnResponseServerStatusInfoList\n");
	ZServerView* pServerList = (ZServerView*)m_IDLResource.FindWidget("SelectedServer");
	if (!pServerList)
		return;

	int nCurrSel = pServerList->GetCurrSel2();
	pServerList->ClearServerList();


#ifdef	_DEBUG
	pServerList->AddServer(ZMsg(MSG_SERVER_DEBUG), "", 0, 1, 0, 1000, true);
#else
	if (ZIsLaunchDevelop())
	{
		pServerList->AddServer(ZMsg(MSG_SERVER_DEBUG), "", 0, 1, 0, 1000, true);
	}
#endif



	if ((0 < nListCount) && (0 != pBlob))
	{
		for (int i = 0; i < nListCount; ++i)
		{
			MTD_ServerStatusInfo* pss = (MTD_ServerStatusInfo*)MGetBlobArrayElement(pBlob, i);
			if (0 == pss)
			{
				mlog("ZGameInterface::OnResponseServerStatusInfoList - %d번째에서 NULL포인터 발생.", i);
				continue;
			}

			const char* Addr = nullptr;
			std::string str;
			if (pss->m_dwIP == 0)
			{
				Addr = m_pLocatorList->GetIPByPos(0).c_str();
				static_assert(std::is_lvalue_reference<decltype(m_pLocatorList->GetIPByPos(0))>::value,
					"Fix me");
			}
			else
			{
				in_addr inaddr;
				inaddr.S_un.S_addr = pss->m_dwIP;

				str = GetIPv4String(inaddr);
				Addr = str.c_str();
			}

			char szServName[128] = { 0, };

			if (pss->m_nType == 1)					// Debug server
				sprintf_safe(szServName, "%s %d", ZMsg(MSG_SERVER_DEBUG), pss->m_nServerID);
			else if (pss->m_nType == 2)			// Match server
				sprintf_safe(szServName, "%s %d", ZMsg(MSG_SERVER_MATCH), pss->m_nServerID);
			else if (pss->m_nType == 3)			// Clan server
				sprintf_safe(szServName, "%s %d", ZMsg(MSG_SERVER_CLAN), pss->m_nServerID);
			else if (pss->m_nType == 4)			// Quest server
				sprintf_safe(szServName, "%s %d", ZMsg(MSG_SERVER_IGUNZ), pss->m_nServerID);
			else if (pss->m_nType == 5)			// Event server
				sprintf_safe(szServName, "%s %d", ZMsg(MSG_SERVER_EVENT), pss->m_nServerID);
			else if (pss->m_nType == 6)			// Test server
				sprintf_safe(szServName, "Test Server %d", pss->m_nServerID);
			else
				continue;

			pServerList->AddServer(szServName, Addr, pss->m_nPort, pss->m_nType, pss->m_nCurPlayer, pss->m_nMaxPlayer, pss->m_bIsLive);
#ifdef _DEBUG
			mlog("ServerList - Name:%s, IP:%s, Port:%d, Type:%d, (%d/%d)\n",
				szServName, Addr, pss->m_nPort, pss->m_nType, pss->m_nCurPlayer, pss->m_nMaxPlayer);
#endif
		}
	}

	if (nListCount)
		pServerList->SetCurrSel(0);
	else
		pServerList->SetCurrSel(-1);

	m_dwRefreshTime = GetGlobalTimeMS() + 10000;
}


void ZGameInterface::OnResponseBlockCountryCodeIP(const char* pszBlockCountryCode, const char* pszRoutingURL)
{
	if (0 != pszBlockCountryCode)
		ShowMessage(pszRoutingURL);
}


void ZGameInterface::RequestServerStatusListInfo()
{
	ZLocatorList*	pLocatorList;

	if (ZApplication::GetInstance()->IsLaunchTest())
		pLocatorList = m_pTLocatorList;
	else
		pLocatorList = m_pLocatorList;


	if (0 == pLocatorList)
		return;

	if (pLocatorList->GetSize() < 1)
		return;

	MCommand* pCmd = ZNewCmd(MC_REQUEST_SERVER_LIST_INFO);
	if (pCmd)
	{
		const string strIP = pLocatorList->GetIPByPos(m_nLocServ++);
		m_nLocServ %= pLocatorList->GetSize();

		GetGameClient()->SendCommandByUDP(pCmd, const_cast<char*>(strIP.c_str()), LOCATOR_PORT);
		delete pCmd;
	}

	m_dwRefreshTime = GetGlobalTimeMS() + 1500;
}
void ZGameInterface::OnDisconnectMsg(const DWORD dwMsgID)
{
	auto pListBox = static_cast<MListBox*>(ZGetGameInterface()->
		GetIDLResource()->
		FindWidget("QuestResult_ItemListbox"));

	if (0 != pListBox) {
		ShowMessage(ZErrStr(dwMsgID));
	}
}


void ZGameInterface::OnAnnounceDeleteClan(const string& strAnnounce)
{
	char szMsg[128];

	sprintf_safe(szMsg, MGetStringResManager()->GetErrorStr(MERR_CLAN_ANNOUNCE_DELETE), strAnnounce.c_str());
	ShowMessage(szMsg);
}

void ZGameInterface::OnVoiceSound()
{
	DWORD dwCurrTime = GetGlobalTimeMS();

	if ( dwCurrTime < m_dwVoiceTime)
		return;

	if ( m_szNextVoice[ 0] == 0)
		return;

	ZApplication::GetSoundEngine()->PlaySound( m_szNextVoice);
	m_dwVoiceTime = dwCurrTime + m_dwNextVoiceTime;

	strcpy( m_szCurrVoice, m_szNextVoice);
	m_szNextVoice[ 0] = 0;
	m_dwNextVoiceTime = 0;
	m_szCurrVoice[0] = 0;
}

void ZGameInterface::PlayVoiceSound(const char* pszSoundName, u32 time)
{
	if (!Z_AUDIO_NARRATIONSOUND)
		return;

	if (!equals(pszSoundName, m_szCurrVoice))
	{
		sprintf_safe(m_szNextVoice, pszSoundName);
		m_dwNextVoiceTime = time;
	}

	if (GetGlobalTimeMS() > m_dwVoiceTime)
		OnVoiceSound();
}
