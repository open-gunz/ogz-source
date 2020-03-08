#include "stdafx.h"

#include "ZApplication.h"
#include "ZInterfaceListener.h"
#include "MWidget.h"
#include "MEdit.h"
#include "MComboBox.h"
#include "ZMapListBox.h"
#include "ZPost.h"
#include "ZConfiguration.h"
#include "MSlider.h"
#include "ZCharacterView.h"
#include "ZCharacterViewList.h"
#include "ZCharacterSelectView.h"
#include "ZShop.h"
#include "ZMyItemList.h"
#include "ZMyInfo.h"
#include "ZStageSetting.h"
#include "MChattingFilter.h"
#include "ZRoomListBox.h"
#include "ZPlayerListBox.h"
#include "MDebug.h"
#include "ZChat.h"
#include "ZMsgBox.h"
#include "ZActionKey.h"
#include "ZPlayerSelectListBox.h"
#include "ZChannelListItem.h"
#include "MTabCtrl.h"

#include "ZApplication.h"
#include "ZServerView.h"
#include "ZCharacterView.h"

#include "ZMonsterBookInterface.h"

#include "RGMain.h"

#include "sodium.h"

#include <iostream>
#include <string>

// Chat Input Listener
class MChatInputListener : public MListener{
public:
	virtual bool OnCommand(MWidget* pWidget, const char* szMessage){
		if(MWidget::IsMsg(szMessage, MEDIT_ENTER_VALUE)==true){
			const char* szCommand = pWidget->GetText();
			ZChatOutput(szCommand);

			char szChat[512];
			strcpy_safe(szChat, pWidget->GetText());
			if (ZApplication::GetGameInterface()->GetChat()->Input(szChat))
			{
			}

			pWidget->SetText("");
			return true;
		}
		else if ((MWidget::IsMsg(szMessage, MEDIT_CHAR_MSG)==true) || (MWidget::IsMsg(szMessage, MEDIT_KEYDOWN_MSG)==true))
		{
			ZApplication::GetGameInterface()->GetChat()->FilterWhisperKey(pWidget);
		}

		return false;
	}
};
MChatInputListener	g_ChatInputListener;

class MHotBarButton : public MButton{
protected:
	char	m_szCommandString[256];
protected:
	virtual bool OnDrop(MWidget* pSender, MBitmap* pBitmap, const char* szString, const char* szItemString){
		m_pIcon = pBitmap;
		AttachToolTip(szString);
		strcpy_safe(m_szCommandString, szItemString);
		return true;
	}

public:
	MHotBarButton(const char* szName=NULL, MWidget* pParent=NULL, MListener* pListener=NULL)
	: MButton(szName, pParent, pListener){
		strcpy_safe(m_szCommandString, "Command is not assigned");
	}
	virtual bool IsDropable(MWidget* pSender){
		return true;
	}
	const char* GetCommandString(void){
		return m_szCommandString;
	}
};


class MHotBarButtonListener : public MListener{
public:
	virtual bool OnCommand(MWidget* pWidget, const char* szMessage){
		if(MWidget::IsMsg(szMessage, MBTN_CLK_MSG)==true){
			MHotBarButton* pButton = (MHotBarButton*)pWidget;
			const char* szCommandString = pButton->GetCommandString();

			char szParse[256];
			g_pGame->ParseReservedWord(szParse, szCommandString);

			char szErrMsg[256];
			ZChatOutput(MCOLOR(0xFFFFFFFF), szCommandString);
			if( ZGetGameClient()->Post(szErrMsg, 256, szParse)==false ){
				ZChatOutput(MCOLOR(0xFFFFC600), szErrMsg);
			}

			return true;
		}
		return false;
	}
};
MHotBarButtonListener	g_HotBarButtonListener;

class MLoginListener : public MListener{
public:
	virtual bool OnCommand(MWidget* pWidget, const char* szMessage){
		if(MWidget::IsMsg(szMessage, MBTN_CLK_MSG)==true)
		{
			ZIDLResource* pResource = ZApplication::GetGameInterface()->GetIDLResource();

			ZServerView* pServerList = (ZServerView*)pResource->FindWidget( "SelectedServer");
			if ( !pServerList)
				return false;

			const ServerInfo *pServer = pServerList->GetSelectedServer();
			if ( pServer)
			{
				if ( pServer->nType == 0 )
					return false;

				if( !pServer->bIsLive )
					return false;

				MWidget* pWidget = pResource->FindWidget( "LoginOK");
				if ( pWidget)
					pWidget->Enable( false);

				pWidget = pResource->FindWidget( "LoginFrame");
				if ( pWidget)
					pWidget->Show( false);

				pWidget = pResource->FindWidget( "Login_ConnectingMsg");
				if ( pWidget)
					pWidget->Show( true);


				ZGetGameInterface()->m_bLoginTimeout = true;
				ZGetGameInterface()->m_dwLoginTimeout = GetGlobalTimeMS() + 60000;

				extern bool g_bConnected;
				extern std::function<void()> g_OnConnectCallback;

				g_OnConnectCallback = []()
				{
					char szID[256];
					char szPassword[256];
					ZIDLResource* pResource = ZApplication::GetGameInterface()->GetIDLResource();
					MWidget* pWidget = pResource->FindWidget("LoginID");
					if (pWidget == NULL) return;
					strcpy_safe(szID, pWidget->GetText());
					pWidget = pResource->FindWidget("LoginPassword");
					if (pWidget == NULL) return;
					strcpy_safe(szPassword, pWidget->GetText());

					unsigned char hashed_password[crypto_generichash_blake2b_BYTES];

					crypto_generichash_blake2b(hashed_password, sizeof(hashed_password),
						(const unsigned char *)szPassword, strlen(szPassword), NULL, 0);

					pWidget->SetText("");

#ifdef _BIRDTEST
					ZChangeGameState(GUNZ_BIRDTEST);
					return ret;
#endif

					const u32 Checksum = 0;

					ZPostLogin(szID, hashed_password, sizeof(hashed_password), Checksum);
					mlog("Login Posted\n");
				};


				if (!g_bConnected)
				{
					if (pServer->nType == 1)
					{
						MWidget* pAddr = pResource->FindWidget("ServerAddress");
						MWidget* pPort = pResource->FindWidget("ServerPort");

						ZPostConnect(pAddr->GetText(), atoi(pPort->GetText()));		// Debug server
						DMLog("Connecting\n");
					}
					else
						ZPostConnect(pServer->szAddress, pServer->nPort);			// Game server
				}
				else
				{
					g_OnConnectCallback();
				}

				MLabel* pLabel = (MLabel*)pResource->FindWidget( "LoginError");
				if ( pLabel)
					pLabel->SetText("");
			}
		}
		return false;
	}
};
MLoginListener	g_LoginListener;

class MCreateAccountFrameCallerListener : public MListener{
public:
	virtual bool OnCommand(MWidget* pWidget, const char* szMessage){
		if (MWidget::IsMsg(szMessage, MBTN_CLK_MSG) == true){
			ZIDLResource* pResource = ZApplication::GetGameInterface()->GetIDLResource();
			MWidget* pFindWidget = pResource->FindWidget("CreateAccountFrame");
			if (pFindWidget != NULL) pFindWidget->Show(true, true);

			return true;
		}
		return false;
	}
};
MCreateAccountFrameCallerListener	g_CreateAccountFrameCallerListener;

class MCreateAccountBtnListener : public MListener{
public:
	virtual bool OnCommand(MWidget* pWidget, const char* szMessage){
		if (MWidget::IsMsg(szMessage, MBTN_CLK_MSG) == true){
			ZIDLResource* pResource = ZApplication::GetGameInterface()->GetIDLResource();

			MEdit* pUsernameEdit = (MEdit*)pResource->FindWidget("AccountUsername");
			MEdit* pPassEdit = (MEdit*)pResource->FindWidget("AccountPassword");
			MEdit* pEmailEdit = (MEdit*)pResource->FindWidget("AccountEmail");

			if (!pUsernameEdit || !pPassEdit)
				return true;

			auto szUsername = pUsernameEdit->GetText();
			auto szPassword = pPassEdit->GetText();
			std::string szPasswrd = pPassEdit->GetText();
			std::string szEmail = pEmailEdit->GetText();

			if (strlen(szUsername) < 5)
			{
				ZGetGameInterface()->ShowErrorMessage("Username should be at least 5 characters long.");
				return true;
			}

			if (strlen(szPassword) < 8)
			{
				ZGetGameInterface()->ShowErrorMessage("Password should be at least 8 characters long.");
				return true;
			}

			// Is password valid or not declarations
			bool szPasswordVerified = false;
			int lowercase = 0;
			int uppercase = 0;
			int numbers = 0;

			// Walk through the string and collect the info
			for (unsigned int x = 0; x < szPasswrd.length(); x++)
			{
				if (islower(szPasswrd[x]))
					lowercase++;

				if (isupper(szPasswrd[x]))
					uppercase++;

				if (isdigit(szPasswrd[x]))
					numbers++;
			}

			// Check for conditions
			if (lowercase >= 1 &&
				uppercase >= 1 &&
				numbers >= 1)
			{
				szPasswordVerified = true;
			}

			// Check for error
			if (!szPasswordVerified)
			{
				ZGetGameInterface()->ShowErrorMessage("Password must have atleast 1 CAPITAL, 1 lowercase, and 1 number.");
				return true;
			}

			// Is email valid or not declarations
			bool szEmailVerified = false;
			unsigned int atIndex = 0;
			unsigned int dotIndex = 0;
			int numberOfAts = 0;
			int numberOfDomains = 0;

			// Walk through the string and collect the info
			for (unsigned int x = 0; x < szEmail.length(); x++) {

				if (szEmail[x] == '@') {
					atIndex = x;
					numberOfAts++;

					// Once the @ is found, begin testing for .
					for (unsigned z = x; z < szEmail.length(); z++) {

						if (szEmail[z] == '.') {
							dotIndex = z;
							numberOfDomains++;
						}
					}
				}
			}

			// Verify that conditions are met
			if (numberOfAts == 1 && numberOfDomains == 1) {

				if (atIndex < dotIndex) {

					szEmailVerified = true;
				}
			}

			// Display error code
			if (!szEmailVerified)
			{
				ZGetGameInterface()->ShowErrorMessage("Please provide a valid email address.");
				return true;
			}

			extern bool g_bConnected;
			extern std::function<void()> g_OnConnectCallback;

			g_OnConnectCallback = [pResource]()
			{
				MEdit* pUsernameEdit = (MEdit*)pResource->FindWidget("AccountUsername");
				MEdit* pPassEdit = (MEdit*)pResource->FindWidget("AccountPassword");
				MEdit* pEmailEdit = (MEdit*)pResource->FindWidget("AccountEmail");

				if (!pUsernameEdit || !pPassEdit || !pEmailEdit)
					return;

				auto szUsername = pUsernameEdit->GetText();
				auto szPassword = pPassEdit->GetText();
				auto szEmail = pEmailEdit->GetText();

				unsigned char hashed_password[crypto_generichash_blake2b_BYTES];

				crypto_generichash_blake2b(hashed_password, sizeof(hashed_password),
					(const unsigned char *)szPassword, strlen(szPassword), NULL, 0);

				ZPostCreateAccount(szUsername, hashed_password, szEmail);
			};

			if (!g_bConnected)
			{
				auto TryConnect = [pResource]()
				{
					auto pServerList = (ZServerView*)pResource->FindWidget("SelectedServer");

					if (!pServerList)
						return false;

					auto pServer = pServerList->GetFirstServer();

					if (!pServer)
						return false;

					if (pServer->nType == 1)
					{
						MWidget* pAddr = pResource->FindWidget("ServerAddress");
						MWidget* pPort = pResource->FindWidget("ServerPort");

						ZPostConnect(pAddr->GetText(), atoi(pPort->GetText()));		// Debug server
					}
					else
					{
						ZPostConnect(pServer->szAddress, pServer->nPort);			// Game server
					}

					return true;
				};

				if (!TryConnect())
					ZGetGameInterface()->ShowErrorMessage("Couldn't find server to register on");
			}
			else
				g_OnConnectCallback();

			return true;
		}
		return false;
	}
};
MCreateAccountBtnListener	g_CreateAccountBtnListener;

class MLogoutListener : public MListener{
public:
	virtual bool OnCommand(MWidget* pWidget, const char* szMessage){
		if(MWidget::IsMsg(szMessage, MBTN_CLK_MSG)==true){

			mlog("MLogoutListener !\n");
			ZPostDisconnect();
			ZApplication::GetGameInterface()->SetState(GUNZ_LOGIN);
			return true;
		}
		return false;
	}
};
MLogoutListener	g_LogoutListener;

class MExitListener : public MListener{
public:
	virtual bool OnCommand(MWidget* pWidget, const char* szMessage){
		if(MWidget::IsMsg(szMessage, MBTN_CLK_MSG)==true){

			mlog("MExitListener !\n");
			ZApplication::Exit();

			return true;
		}
		return false;
	}
};
MExitListener	g_ExitListener;

class MChannelChatInputListener : public MListener{
public:
	virtual bool OnCommand(MWidget* pWidget, const char* szMessage){
		if(MWidget::IsMsg(szMessage, MEDIT_ENTER_VALUE)==true){
			char szChat[512];
			if (strlen(pWidget->GetText()) < 255)
			{
				strcpy_safe(szChat, pWidget->GetText());
				if (ZApplication::GetGameInterface()->GetChat()->Input(szChat))
				{
					pWidget->SetText("");
				}
			}
			return true;
		}
		else if ((MWidget::IsMsg(szMessage, MEDIT_CHAR_MSG)==true) || (MWidget::IsMsg(szMessage, MEDIT_KEYDOWN_MSG)==true))
		{
			ZApplication::GetGameInterface()->GetChat()->FilterWhisperKey(pWidget);
		}
		return false;
	}
};
MChannelChatInputListener	g_ChannelChatInputListener;

class MStageChatInputListener : public MListener{
public:
	virtual bool OnCommand(MWidget* pWidget, const char* szMessage){
		if(MWidget::IsMsg(szMessage, MEDIT_ENTER_VALUE)==true){
			char szChat[512];
			if (strlen(pWidget->GetText()) < 255)
			{
				strcpy_safe(szChat, pWidget->GetText());
				if (ZApplication::GetGameInterface()->GetChat()->Input(szChat))
				{
					pWidget->SetText("");
				}
			}
			return true;
		}
		else if ((MWidget::IsMsg(szMessage, MEDIT_CHAR_MSG)==true) || (MWidget::IsMsg(szMessage, MEDIT_KEYDOWN_MSG)==true))
		{
			ZApplication::GetGameInterface()->GetChat()->FilterWhisperKey(pWidget);
		}

		return false;
	}
};
MStageChatInputListener	g_StageChatInputListener;


class MGameStartListener : public MListener{
public:
	virtual bool OnCommand(MWidget* pWidget, const char* szMessage){
		if(MWidget::IsMsg(szMessage, MBTN_CLK_MSG)==true){

			if(ZGetGameClient()->GetMatchStageSetting()->GetMapName()[0]!=0)
			{
				ZApplication::GetStageInterface()->ChangeStageEnableReady( true);

				ZPostStageStart(ZGetGameClient()->GetPlayerUID(), ZGetGameClient()->GetStageUID());
			}
			else
			{
				ZApplication::GetGameInterface()->ShowMessage("선택하신 맵이 없습니다. 맵을 선택해 주세요.");
			}

			return true;
		}
		return false;
	}
};
MGameStartListener	g_GameStartListener;

class MMapChangeListener : public MListener{
public:
	virtual bool OnCommand(MWidget* pWidget, const char* szMessage){
		if(MWidget::IsMsg(szMessage, MBTN_CLK_MSG)==true){
			ZApplication::GetGameInterface()->ShowWidget("MapFrame", true, true);

			return true;
		}
		return false;
	}
};
MMapChangeListener	g_MapChangeListener;

class MMapSelectListener : public MListener{
public:
	virtual bool OnCommand(MWidget* pWidget, const char* szMessage){
		if(MWidget::IsMsg(szMessage, MBTN_CLK_MSG)==true){
			ZIDLResource* pResource = ZApplication::GetGameInterface()->GetIDLResource();
			ZMapListBox* pWidget = (ZMapListBox*)pResource->FindWidget("MapList");
			char szMapName[_MAX_DIR];
			strcpy_safe(szMapName, pWidget->GetSelItemString());
			if(szMapName!=NULL){
				ZApplication::GetStageInterface()->SetMapName(szMapName);
				ZPostStageMap(ZGetGameClient()->GetStageUID(), szMapName);

				if(pWidget->GetParent()!=NULL) pWidget->GetParent()->Show(false);
			}

			return true;
		}
		return false;
	}
};
MMapSelectListener	g_MapSelectListener;



class MParentCloseListener : public MListener{
public:
	virtual bool OnCommand(MWidget* pWidget, const char* szMessage){
		if(MWidget::IsMsg(szMessage, MBTN_CLK_MSG)==true){
			if(pWidget->GetParent()!=NULL) pWidget->GetParent()->Show(false);
			return true;
		}
		return false;
	}
};
MParentCloseListener	g_ParentCloseListener;


class MStageCreateFrameCallerListener : public MListener{
public:
	virtual bool OnCommand(MWidget* pWidget, const char* szMessage){
		if(MWidget::IsMsg(szMessage, MBTN_CLK_MSG)==true){
			ZIDLResource* pResource = ZApplication::GetGameInterface()->GetIDLResource();
			MWidget* pFindWidget = pResource->FindWidget("StageCreateFrame");
			if(pFindWidget!=NULL) pFindWidget->Show(true, true);

			MEdit* pPassEdit = (MEdit*)pResource->FindWidget("StagePassword");
			if (pPassEdit!=NULL)
			{
				pPassEdit->SetMaxLength(STAGEPASSWD_LENGTH);
				pPassEdit->SetText("");
			}

			return true;
		}
		return false;
	}
};
MStageCreateFrameCallerListener	g_StageCreateFrameCallerListener;




class MSelectCharacterComboBoxListener : public MListener{
public:
	virtual bool OnCommand(MWidget* pWidget, const char* szMessage)
	{
		if(MWidget::IsMsg(szMessage, MCMBBOX_CHANGED)==true)
		{
			ZIDLResource* pResource = ZApplication::GetGameInterface()->GetIDLResource();

			if (ZApplication::GetGameInterface()->GetCharacterSelectView() != NULL)
			{
				ZApplication::GetGameInterface()->GetCharacterSelectView()->SelectChar( ZCharacterSelectView::GetSelectedCharacter() );
			}

			return true;
		}

		return false;
	}
};
MSelectCharacterComboBoxListener	g_SelectCharacterComboBoxListener;

MListener* ZGetChatInputListener(void)
{
	return &g_ChatInputListener;
}

MListener* ZGetLoginListener(void)
{
	return &g_LoginListener;
}

MListener* ZGetCreateAccountFrameCallerListener(void)
{
	return &g_CreateAccountFrameCallerListener;
}

MListener* ZGetCreateAccountBtnListener(void)
{
	return &g_CreateAccountBtnListener;
}

MListener* ZGetLogoutListener(void)
{
	return &g_LogoutListener;
}

MListener* ZGetExitListener(void)
{
	return &g_ExitListener;
}

MListener* ZGetChannelChatInputListener(void)
{
	return &g_ChannelChatInputListener;
}

MListener* ZGetStageChatInputListener(void)
{
	return &g_StageChatInputListener;
}

MListener* ZGetGameStartListener(void)
{
	return &g_GameStartListener;
}

MListener* ZGetMapChangeListener(void)
{
	return &g_MapChangeListener;
}

MListener* ZGetMapSelectListener(void)
{
	return &g_MapSelectListener;
}

MListener* ZGetParentCloseListener(void)
{
	return &g_ParentCloseListener;
}


MListener* ZGetStageCreateFrameCallerListener(void)
{
	return &g_StageCreateFrameCallerListener;
}

MListener* ZGetSelectCharacterComboBoxListener(void)
{
	return &g_SelectCharacterComboBoxListener;
}


BEGIN_IMPLEMENT_LISTENER(ZGetMapListListener, MLB_ITEM_DBLCLK)
	ZIDLResource* pResource = ZApplication::GetGameInterface()->GetIDLResource();
	ZMapListBox* pMapList = (ZMapListBox*)pResource->FindWidget("MapList");
	const char* pszSelItemString = pMapList->GetSelItemString();
	if (pszSelItemString) {
		char szMapName[_MAX_DIR];
		sprintf_safe(szMapName, pszSelItemString);
		ZApplication::GetStageInterface()->SetMapName(szMapName);
		ZPostStageMap(ZGetGameClient()->GetStageUID(), szMapName);
		if(pWidget->GetParent()!=NULL) pWidget->GetParent()->GetParent()->Show(false);
	}
END_IMPLEMENT_LISTENER()

BEGIN_IMPLEMENT_LISTENER(ZGetStageListFrameCallerListener, MBTN_CLK_MSG)
	ZIDLResource* pResource = ZApplication::GetGameInterface()->GetIDLResource();

	ZGetGameClient()->StartStageList();
END_IMPLEMENT_LISTENER()


BEGIN_IMPLEMENT_LISTENER(ZGetStageCreateBtnListener, MBTN_CLK_MSG)
	ZIDLResource* pResource = ZApplication::GetGameInterface()->GetIDLResource();
	MWidget* pNameWidget = pResource->FindWidget("StageName");
	if(pNameWidget==NULL) return true;
	char szStageName[128], szStagePassword[128];
	bool bPrivate = false;
	strcpy_safe(szStageName, pNameWidget->GetText());

	MEdit* pPassEdit = (MEdit*)pResource->FindWidget("StagePassword");
	if (pPassEdit)
	{
		if ((strlen(pPassEdit->GetText()) > 0) && (strlen(pPassEdit->GetText()) <= STAGEPASSWD_LENGTH))
			bPrivate = true;
		else
			bPrivate = false;


		if (bPrivate == true)
		{
			strcpy_safe(szStagePassword, pPassEdit->GetText());
		}
		else
		{
			memset(szStagePassword, 0, sizeof(szStagePassword));
		}
	}

	if(pWidget->GetParent()!=NULL) pWidget->GetParent()->Show(false);
	ZApplication::GetStageInterface()->ChangeStageButtons(false, true, false);

	MSTAGE_SETTING_NODE setting;
	setting.uidStage = MUID(0, 0);
	memset(setting.szMapName, 0, sizeof(setting.szMapName));
	setting.nGameType = MMATCH_GAMETYPE_DEATHMATCH_SOLO;
	setting.nRoundMax = 10;
	setting.nLimitTime = 10;
	setting.nMaxPlayers = 8;

	ZApplication::GetStageInterface()->ChangeStageGameSetting( &setting);

	if ( !MGetChattingFilter()->IsValidChatting( szStageName))
	{
		char szMsg[ 256 ];
		ZTransMsg( szMsg, MSG_WRONG_WORD_NAME, 1, MGetChattingFilter()->GetLastFilteredStr());
		ZApplication::GetGameInterface()->ShowMessage( szMsg );
	}
	else
	{
		ZApplication::GetGameInterface()->EnableLobbyInterface(false);
		ZGetGameClient()->CreatedStage = true;
		ZPostStageCreate(ZGetGameClient()->GetPlayerUID(), szStageName, bPrivate, szStagePassword);
	}
END_IMPLEMENT_LISTENER()

BEGIN_IMPLEMENT_LISTENER(ZGetPrivateStageJoinBtnListener, MBTN_CLK_MSG)
	ZIDLResource* pResource = ZApplication::GetGameInterface()->GetIDLResource();
	ZRoomListBox* pRoomListBox = (ZRoomListBox*)pResource->FindWidget("Lobby_StageList");
	if (pRoomListBox)
	{
		pRoomListBox->RequestSelPrivateStageJoin();
	}

	MWidget* pPrivateStageJoinFrame = pResource->FindWidget("PrivateStageJoinFrame");
	if (pPrivateStageJoinFrame)
	{
		pPrivateStageJoinFrame->Show(false);
	}
END_IMPLEMENT_LISTENER()


// Channel
BEGIN_IMPLEMENT_LISTENER(ZGetChannelListFrameCallerListener, MBTN_CLK_MSG)
	ZIDLResource* pResource = ZApplication::GetGameInterface()->GetIDLResource();
	MWidget* pFindWidget = pResource->FindWidget("ChannelListFrame");
	if(pFindWidget!=NULL) pFindWidget->Show(true, true);

	MButton* pButton = (MButton*)pResource->FindWidget("MyClanChannel");
	if ( pButton)
		pButton->Enable( ZGetMyInfo()->IsClanJoined());

	MCHANNEL_TYPE nCurrentChannelType = ZGetGameClient()->GetChannelType();
	ZApplication::GetGameInterface()->InitChannelFrame(nCurrentChannelType);
	ZGetGameClient()->StartChannelList(nCurrentChannelType);
END_IMPLEMENT_LISTENER()

BEGIN_IMPLEMENT_LISTENER(ZGetChannelListJoinButtonListener, MBTN_CLK_MSG)
	ZIDLResource* pResource = ZApplication::GetGameInterface()->GetIDLResource();
	MListBox* pChannelList = (MListBox*)pResource->FindWidget("ChannelList");
	ZChannelListItem* pItem = (ZChannelListItem*)pChannelList->GetSelItem();
	if (pItem) {
		ZGetGameClient()->RequestChannelJoin(pItem->GetUID());
	}
	if(pWidget->GetParent()!=NULL) pWidget->GetParent()->Show(false);
	ZGetGameClient()->StopChannelList();
END_IMPLEMENT_LISTENER()

BEGIN_IMPLEMENT_LISTENER(ZGetChannelListCloseButtonListener, MBTN_CLK_MSG)
	if(pWidget->GetParent()!=NULL) pWidget->GetParent()->Show(false);
	ZGetGameClient()->StopChannelList();
END_IMPLEMENT_LISTENER()

BEGIN_IMPLEMENT_LISTENER(ZGetChannelListListener, MLB_ITEM_DBLCLK)
	ZIDLResource* pResource = ZApplication::GetGameInterface()->GetIDLResource();
	MListBox* pChannelList = (MListBox*)pResource->FindWidget("ChannelList");
	ZChannelListItem* pItem = (ZChannelListItem*)pChannelList->GetSelItem();
	if (pItem) {
		ZGetGameClient()->RequestChannelJoin(pItem->GetUID());
	}
	if(pWidget->GetParent()!=NULL) pWidget->GetParent()->Show(false);
	ZGetGameClient()->StopChannelList();
END_IMPLEMENT_LISTENER()

BEGIN_IMPLEMENT_LISTENER(ZGetStageJoinListener, MBTN_CLK_MSG)
	ZIDLResource* pResource = ZApplication::GetGameInterface()->GetIDLResource();
	ZRoomListBox* pRoomListBox = (ZRoomListBox*)pResource->FindWidget("Lobby_StageList");
	if (pRoomListBox)
	{
		pRoomListBox->RequestSelStageJoin();

	}
END_IMPLEMENT_LISTENER()

BEGIN_IMPLEMENT_LISTENER(ZGetStageSettingCallerListener, MBTN_CLK_MSG)
	ZIDLResource* pResource = ZApplication::GetGameInterface()->GetIDLResource();
	MWidget* pWidget = pResource->FindWidget("StageSettingFrame");
	if(pWidget!=NULL)
		pWidget->Show(true, true);
END_IMPLEMENT_LISTENER()

BEGIN_IMPLEMENT_LISTENER(ZGetStageSettingStageTypeListener, MCMBBOX_CHANGED)
	{
		ZStageSetting::InitStageSettingGameFromGameType();
		ZStageSetting::PostDataToServer();
	}
END_IMPLEMENT_LISTENER()

BEGIN_IMPLEMENT_LISTENER(ZGetStageTeamRedListener, MBTN_CLK_MSG)
	MButton* pButton = (MButton*)ZApplication::GetGameInterface()->GetIDLResource()->FindWidget("StageTeamRed");
	if(pButton && !pButton->GetCheck() ) pButton->SetCheck(true);
	pButton = (MButton*)ZApplication::GetGameInterface()->GetIDLResource()->FindWidget("StageTeamBlue");
	if(pButton) pButton->SetCheck( false );
	ZPostStageTeam(ZGetGameClient()->GetPlayerUID(), ZGetGameClient()->GetStageUID(), MMT_RED);
END_IMPLEMENT_LISTENER()


BEGIN_IMPLEMENT_LISTENER(ZGetStageTeamBlueListener, MBTN_CLK_MSG)
	MButton* pButton = (MButton*)ZApplication::GetGameInterface()->GetIDLResource()->FindWidget("StageTeamBlue");
	if(pButton && !pButton->GetCheck() ) pButton->SetCheck(true);
	pButton = (MButton*)ZApplication::GetGameInterface()->GetIDLResource()->FindWidget("StageTeamRed");
	if(pButton) pButton->SetCheck( false );
	ZPostStageTeam(ZGetGameClient()->GetPlayerUID(), ZGetGameClient()->GetStageUID(), MMT_BLUE);
END_IMPLEMENT_LISTENER()

BEGIN_IMPLEMENT_LISTENER(ZGetStageReadyListener, MBTN_CLK_MSG)
	ZIDLResource* pResource = ZApplication::GetGameInterface()->GetIDLResource();

	bool bReady = false;
	MButton* pReadyBtn = (MButton*)pResource->FindWidget("StageReady");
	if(pReadyBtn) bReady=pReadyBtn->GetCheck();

	MMatchObjectStageState nStageState;
	if (bReady)
		nStageState = MOSS_READY;
	else
		nStageState = MOSS_NONREADY;

	ZPostStageState(ZGetGameClient()->GetPlayerUID(), ZGetGameClient()->GetStageUID(), nStageState);
	ZApplication::GetStageInterface()->ChangeStageEnableReady(bReady);
END_IMPLEMENT_LISTENER()

BEGIN_IMPLEMENT_LISTENER(ZGetStageObserverBtnListener, MBTN_CLK_MSG)
	ZIDLResource* pResource = ZApplication::GetGameInterface()->GetIDLResource();
	MButton* pObserverBtn = (MButton*)pResource->FindWidget("StageObserverBtn");
	if ( pObserverBtn)
	{
		if ( pObserverBtn->GetCheck())
			ZPostStageTeam(ZGetGameClient()->GetPlayerUID(), ZGetGameClient()->GetStageUID(), MMT_SPECTATOR);
		else
		{
			MButton* pBlueBtn = (MButton*)pResource->FindWidget("StageTeamBlue");

			if ( ZApplication::GetGameInterface()->m_bTeamPlay)
			{
				if ( pBlueBtn->GetCheck())
					ZPostStageTeam(ZGetGameClient()->GetPlayerUID(), ZGetGameClient()->GetStageUID(), MMT_BLUE);
				else
					ZPostStageTeam(ZGetGameClient()->GetPlayerUID(), ZGetGameClient()->GetStageUID(), MMT_RED);
			}
			else
			{
				ZPostStageTeam(ZGetGameClient()->GetPlayerUID(), ZGetGameClient()->GetStageUID(), MMT_ALL);
			}
		}
	}
END_IMPLEMENT_LISTENER()

BEGIN_IMPLEMENT_LISTENER(ZGetStageSettingChangedComboboxListener, MCMBBOX_CHANGED)
	ZStageSetting::PostDataToServer();
END_IMPLEMENT_LISTENER()

BEGIN_IMPLEMENT_LISTENER(ZGetStageSettingApplyBtnListener, MBTN_CLK_MSG)
	ZStageSetting::ApplyStageSettingDialog();
END_IMPLEMENT_LISTENER()


BEGIN_IMPLEMENT_LISTENER(ZGetLobbyListener, MBTN_CLK_MSG)
	ZPostStageLeave(ZGetGameClient()->GetPlayerUID(), ZGetGameClient()->GetStageUID());
END_IMPLEMENT_LISTENER()


BEGIN_IMPLEMENT_LISTENER(ZGetLoginStateButtonListener, MBTN_CLK_MSG)
	ZApplication::GetGameInterface()->SetState(GUNZ_LOGIN);
END_IMPLEMENT_LISTENER()


BEGIN_IMPLEMENT_LISTENER(ZGetGreeterStateButtonListener, MBTN_CLK_MSG)
	ZApplication::GetGameInterface()->SetState(GUNZ_GREETER);
END_IMPLEMENT_LISTENER()

BEGIN_IMPLEMENT_LISTENER(ZGetBattleExitButtonListener, MBTN_CLK_MSG)
	if(pWidget->GetParent()!=NULL) pWidget->GetParent()->Show(false);
	ZApplication::GetGameInterface()->ReserveLeaveBattle();
END_IMPLEMENT_LISTENER()

BEGIN_IMPLEMENT_LISTENER(ZGetStageExitButtonListener, MBTN_CLK_MSG)
	if(pWidget->GetParent()!=NULL) pWidget->GetParent()->Show(false);
	ZApplication::GetGameInterface()->ReserveLeaveStage();
END_IMPLEMENT_LISTENER()


BEGIN_IMPLEMENT_LISTENER(ZGetCombatMenuCloseButtonListener, MBTN_CLK_MSG)
	ZGetGameInterface()->ShowMenu(false);
END_IMPLEMENT_LISTENER()



BEGIN_IMPLEMENT_LISTENER(ZGetPreviousStateButtonListener, MBTN_CLK_MSG)
	ZApplication::GetGameInterface()->SetState(GUNZ_PREVIOUS);
END_IMPLEMENT_LISTENER()



BEGIN_IMPLEMENT_LISTENER(ZGetShopCallerButtonListener, MBTN_CLK_MSG)
	ZApplication::GetGameInterface()->ShowShopDialog(true);
END_IMPLEMENT_LISTENER()

BEGIN_IMPLEMENT_LISTENER(ZGetShopCloseButtonListener, MBTN_CLK_MSG)
	ZApplication::GetGameInterface()->ShowShopDialog(false);
END_IMPLEMENT_LISTENER()

BEGIN_IMPLEMENT_LISTENER(ZGetEquipmentCallerButtonListener, MBTN_CLK_MSG)
	ZApplication::GetGameInterface()->ShowEquipmentDialog(true);
END_IMPLEMENT_LISTENER()

BEGIN_IMPLEMENT_LISTENER(ZGetEquipmentCloseButtonListener, MBTN_CLK_MSG)
	ZApplication::GetGameInterface()->ShowEquipmentDialog(false);

{

	int nState = ZApplication::GetGameInterface()->GetState();

	if(nState==GUNZ_LOBBY)
	{
		ZCharacterViewList* pVLL = ZGetCharacterViewList(GUNZ_LOBBY);
		if(pVLL)
			pVLL->ChangeCharacterInfo();
	}
	else if(nState==GUNZ_STAGE)
	{
		ZCharacterViewList* pVLS = ZGetCharacterViewList(GUNZ_STAGE);
		if(pVLS)
			pVLS->ChangeCharacterInfo();
	}
}

END_IMPLEMENT_LISTENER()

BEGIN_IMPLEMENT_LISTENER(ZGetCharSelectionCallerButtonListener, MBTN_CLK_MSG)
	ZApplication::GetGameInterface()->ChangeToCharSelection();
END_IMPLEMENT_LISTENER()

BEGIN_IMPLEMENT_LISTENER(ZGetQuickJoinButtonListener, MBTN_CLK_MSG)
	ZGetGameInterface()->RequestQuickJoin();
END_IMPLEMENT_LISTENER()
BEGIN_IMPLEMENT_LISTENER(ZGetLobbyCharInfoCallerButtonListener, MBTN_CLK_MSG)
	ZApplication::GetGameInterface()->ShowErrorMessage( MERR_NOT_SUPPORT );
END_IMPLEMENT_LISTENER()

BEGIN_IMPLEMENT_LISTENER(ZGetSellButtonListener, MBTN_CLK_MSG)
	ZApplication::GetGameInterface()->Sell();
END_IMPLEMENT_LISTENER()
BEGIN_IMPLEMENT_LISTENER(ZGetSellQuestItemConfirmOpenListener, MBTN_CLK_MSG)
	ZApplication::GetGameInterface()->OpenSellQuestItemConfirm();
END_IMPLEMENT_LISTENER()
BEGIN_IMPLEMENT_LISTENER(ZGetSellQuestItemConfirmCloseListener, MBTN_CLK_MSG)
	MWidget* pWidget = ZApplication::GetGameInterface()->GetIDLResource()->FindWidget( "Shop_SellQuestItem");
	if ( pWidget)
		pWidget->Show( false);
END_IMPLEMENT_LISTENER()
BEGIN_IMPLEMENT_LISTENER(ZGetSellQuestItemButtonListener, MBTN_CLK_MSG)
	ZApplication::GetGameInterface()->SellQuestItem();
END_IMPLEMENT_LISTENER()
BEGIN_IMPLEMENT_LISTENER(ZGetBuyButtonListener, MBTN_CLK_MSG)
	ZApplication::GetGameInterface()->Buy();
END_IMPLEMENT_LISTENER()
BEGIN_IMPLEMENT_LISTENER(ZGetBuyCashItemButtonListener, MBTN_CLK_MSG)
	ZApplication::GetGameInterface()->BuyCashItem();
END_IMPLEMENT_LISTENER()

BEGIN_IMPLEMENT_LISTENER(ZGetItemCountUpButtonListener, MBTN_CLK_MSG)
	ZApplication::GetGameInterface()->SellQuestItemCountUp();
END_IMPLEMENT_LISTENER()

BEGIN_IMPLEMENT_LISTENER(ZGetItemCountDnButtonListener, MBTN_CLK_MSG)
	ZApplication::GetGameInterface()->SellQuestItemCountDn();
END_IMPLEMENT_LISTENER()


BEGIN_IMPLEMENT_LISTENER(ZGetEquipButtonListener, MBTN_CLK_MSG)
	ZApplication::GetGameInterface()->Equip();
END_IMPLEMENT_LISTENER()
BEGIN_IMPLEMENT_LISTENER(ZGetEquipmentSearchButtonListener, MBTN_CLK_MSG)

END_IMPLEMENT_LISTENER()

BEGIN_IMPLEMENT_LISTENER(ZGetSendAccountItemButtonListener, MBTN_CLK_MSG)
	ZIDLResource* pResource = ZApplication::GetGameInterface()->GetIDLResource();

	MListBox* pWidget = (MListBox*)pResource->FindWidget( "EquipmentList");
	if ( pWidget)
	{
		if (pWidget->IsSelected())
		{
			ZPostRequestBringBackAccountItem( ZGetMyUID(), ZGetMyInfo()->GetItemList()->GetItemUIDEquip( pWidget->GetSelIndex()));

			MButton* pButton = (MButton*)pResource->FindWidget( "SendAccountItemBtn");
			if ( pButton)
			{
				pButton->Enable( false);
				pButton->Show( false);
				pButton->Show( true);
			}

			pButton = (MButton*)pResource->FindWidget( "Equip");
			if ( pButton)
			{
				pButton->Enable( false);
				pButton->Show( false);
				pButton->Show( true);
			}

			ZApplication::GetGameInterface()->SetKindableItem( MMIST_NONE);
		}
	}
END_IMPLEMENT_LISTENER()

BEGIN_IMPLEMENT_LISTENER(ZGetBringAccountItemButtonListener, MBTN_CLK_MSG)
	ZApplication::GetGameInterface()->GetBringAccountItem();
END_IMPLEMENT_LISTENER()


BEGIN_IMPLEMENT_LISTENER(ZGetShopCachRechargeButtonListener, MBTN_CLK_MSG)
	ZApplication::GetGameInterface()->ShowErrorMessage( MERR_NOT_SUPPORT );
END_IMPLEMENT_LISTENER()

BEGIN_IMPLEMENT_LISTENER(ZGetShopSearchCallerButtonListener, MBTN_CLK_MSG)
	ZApplication::GetGameInterface()->ShowErrorMessage( MERR_NOT_SUPPORT );
END_IMPLEMENT_LISTENER()



void PostMapname()
{
	ZIDLResource* pResource = ZApplication::GetGameInterface()->GetIDLResource();
	MComboBox* pMapCombo = (MComboBox*)pResource->FindWidget("MapSelection");
	const char* pszSelItemString = pMapCombo->GetSelItemString();

	if (pszSelItemString) {
		char szMapName[_MAX_DIR];
		sprintf_safe(szMapName, pszSelItemString);
		ZApplication::GetStageInterface()->SetMapName(szMapName);
		ZPostStageMap(ZGetGameClient()->GetStageUID(), szMapName);
	}
}

BEGIN_IMPLEMENT_LISTENER(ZGetMapComboListener, MCMBBOX_CHANGED)
	PostMapname();
END_IMPLEMENT_LISTENER()

BEGIN_IMPLEMENT_LISTENER(ZGetSelectMapPrevButtonListener, MBTN_CLK_MSG)

	ZIDLResource* pResource = ZApplication::GetGameInterface()->GetIDLResource();
	MComboBox* pComboBox = (MComboBox*)pResource->FindWidget("MapSelection");

	if(pComboBox)
	{
		pComboBox->SetPrevSel();
		PostMapname();
	}

END_IMPLEMENT_LISTENER()

BEGIN_IMPLEMENT_LISTENER(ZGetSelectMapNextButtonListener, MBTN_CLK_MSG)

	ZIDLResource* pResource = ZApplication::GetGameInterface()->GetIDLResource();
	MComboBox* pComboBox = (MComboBox*)pResource->FindWidget("MapSelection");

	if(pComboBox)
	{
		pComboBox->SetNextSel();
		PostMapname();
	}

END_IMPLEMENT_LISTENER()


BEGIN_IMPLEMENT_LISTENER(ZGetSelectCameraLeftButtonListener, MBTN_CLK_MSG)

END_IMPLEMENT_LISTENER()

BEGIN_IMPLEMENT_LISTENER(ZGetSelectCameraRightButtonListener, MBTN_CLK_MSG)

END_IMPLEMENT_LISTENER()


BEGIN_IMPLEMENT_LISTENER(ZGetCreateCharacterLeftButtonListener, MBTN_CLK_MSG)
	if(ZGetGameInterface()->GetCharacterSelectView())
		ZGetGameInterface()->GetCharacterSelectView()->CharacterLeft();
END_IMPLEMENT_LISTENER()

BEGIN_IMPLEMENT_LISTENER(ZGetCreateCharacterRightButtonListener, MBTN_CLK_MSG)
	if(ZGetGameInterface()->GetCharacterSelectView())
		ZGetGameInterface()->GetCharacterSelectView()->CharacterRight();
END_IMPLEMENT_LISTENER()


static DWORD g_dwClockCharSelBtn = 0;
void CharacterSelect( int nNum)
{
	ZIDLResource* pResource = ZApplication::GetGameInterface()->GetIDLResource();

	if ( (ZCharacterSelectView::GetSelectedCharacter() == nNum) && ((GetGlobalTimeMS() - g_dwClockCharSelBtn ) <= 300))
	{
		ZApplication::GetGameInterface()->OnCharSelect();

		return ;
	}

	g_dwClockCharSelBtn = GetGlobalTimeMS();

	ZApplication::GetGameInterface()->ChangeSelectedChar( nNum);
}

BEGIN_IMPLEMENT_LISTENER( ZGetSelectCharacterButtonListener0, MBTN_CLK_MSG)
	CharacterSelect( 0);
END_IMPLEMENT_LISTENER()

BEGIN_IMPLEMENT_LISTENER( ZGetSelectCharacterButtonListener1, MBTN_CLK_MSG)
	CharacterSelect( 1);
END_IMPLEMENT_LISTENER()

BEGIN_IMPLEMENT_LISTENER( ZGetSelectCharacterButtonListener2, MBTN_CLK_MSG)
	CharacterSelect( 2);
END_IMPLEMENT_LISTENER()

BEGIN_IMPLEMENT_LISTENER( ZGetSelectCharacterButtonListener3, MBTN_CLK_MSG)
	CharacterSelect( 3);
END_IMPLEMENT_LISTENER()

BEGIN_IMPLEMENT_LISTENER(ZGetSelectCharacterButtonListener, MBTN_CLK_MSG)
	if ( ZCharacterSelectView::GetNumOfCharacter())
	{
		if ( ZApplication::GetGameInterface()->GetCharacterSelectView() != NULL)
		{
			ZApplication::GetGameInterface()->OnCharSelect();

			MWidget* pWidget = ZApplication::GetGameInterface()->GetIDLResource()->FindWidget( "CS_SelectCharDefKey");
			pWidget->Enable( false);
		}
	}
	else
	{
		ZApplication::GetGameInterface()->ShowMessage("해당 슬롯에 캐릭터가 없습니다.");
	}
END_IMPLEMENT_LISTENER()

BEGIN_IMPLEMENT_LISTENER(ZGetShowCreateCharacterButtonListener, MBTN_CLK_MSG)
	ZApplication::GetGameInterface()->SetState(GUNZ_CHARCREATION);
END_IMPLEMENT_LISTENER()

BEGIN_IMPLEMENT_LISTENER(ZGetDeleteCharacterButtonListener, MBTN_CLK_MSG)
	ZIDLResource* pResource = ZApplication::GetGameInterface()->GetIDLResource();

	char szName[256];
	sprintf_safe( szName, "CharSel_Name%d", ZCharacterSelectView::GetSelectedCharacter() );
	MLabel* pLabel = (MLabel*)pResource->FindWidget( szName);

	if( ZCharacterSelectView::GetNumOfCharacter())
	{
		int ret = ZGetGameClient()->ValidateRequestDeleteChar();

		if (ret != ZOK)
		{
			ZApplication::GetGameInterface()->ShowMessage( ret );

			return true;
		}

		MLabel* pCharNameLabel = (MLabel*)pResource->FindWidget("CS_DeleteCharLabel");
		if (pCharNameLabel)
			pCharNameLabel->SetText( pLabel->GetText());

		MEdit* pYesEdit = (MEdit*)pResource->FindWidget("CS_DeleteCharNameEdit");
		if (pYesEdit)
			pYesEdit->SetText("");

		MWidget* pWidget = pResource->FindWidget("CS_ConfirmDeleteChar");
		if (pWidget)
		{
			pWidget->Show(true, true);
			pWidget->SetFocus();
		}
	}
END_IMPLEMENT_LISTENER()

BEGIN_IMPLEMENT_LISTENER(ZGetConfirmDeleteCharacterButtonListener, MBTN_CLK_MSG)
	ZIDLResource* pResource = ZApplication::GetGameInterface()->GetIDLResource();

	char szName[256];
	sprintf_safe( szName, "CharSel_Name%d", ZCharacterSelectView::GetSelectedCharacter() );
	MLabel* pLabel = (MLabel*)pResource->FindWidget( szName);

	if( ZCharacterSelectView::GetNumOfCharacter())
	{
		MEdit* pYesEdit = (MEdit*)pResource->FindWidget( "CS_DeleteCharNameEdit");
		if (pYesEdit)
		{
			if ( (!_stricmp( pYesEdit->GetText(), ZMsg(MSG_MENUITEM_YES))) && (ZCharacterSelectView::GetSelectedCharacter() >= 0) )
			{
				if (ZCharacterSelectView::m_CharInfo[ZCharacterSelectView::GetSelectedCharacter()].m_bLoaded &&
					ZCharacterSelectView::m_CharInfo[ZCharacterSelectView::GetSelectedCharacter()].m_CharInfo.szClanName[0] == 0)
				{
					ZPostDeleteMyChar(ZGetGameClient()->GetPlayerUID(), ZCharacterSelectView::GetSelectedCharacter(), (char*)pLabel->GetText());

					ZCharacterSelectView::SetSelectedCharacter(-1);
				}
				else
					ZApplication::GetGameInterface()->ShowMessage( MSG_CLAN_PLEASE_LEAVE_FROM_CHAR_DELETE );
				}
			else
				ZApplication::GetGameInterface()->ShowMessage( MSG_CHARDELETE_ERROR );
		}

		MWidget* pWidget = pResource->FindWidget( "CS_ConfirmDeleteChar");
		if ( pWidget)
			pWidget->Show( false);
	}
END_IMPLEMENT_LISTENER()

BEGIN_IMPLEMENT_LISTENER(ZGetCloseConfirmDeleteCharButtonListener, MBTN_CLK_MSG)
	ZIDLResource* pResource = ZApplication::GetGameInterface()->GetIDLResource();
	MWidget* pWidget = pResource->FindWidget("CS_ConfirmDeleteChar");
	if (pWidget)
	{
		pWidget->Show(false);
	}
END_IMPLEMENT_LISTENER()



BEGIN_IMPLEMENT_LISTENER(ZGetCreateCharacterButtonListener, MBTN_CLK_MSG)
	int nEmptySlotIndex=-1;
	for(int i=0;i<4;i++)
	{
		if(ZApplication::GetGameInterface()->GetCharacterSelectView()->IsEmpty(i))
		{
			nEmptySlotIndex=i;
			break;
		}
	}

	if(nEmptySlotIndex>=0)
	{
		ZIDLResource* pResource = ZApplication::GetGameInterface()->GetIDLResource();
		MEdit* pEdit = (MEdit*)pResource->FindWidget("CC_CharName");
		MComboBox* pSexCB, *pHairCB, *pFaceCB, *pCostumeCB;
		pSexCB = (MComboBox*)pResource->FindWidget("CC_Sex");
		pHairCB = (MComboBox*)pResource->FindWidget("CC_Hair");
		pFaceCB = (MComboBox*)pResource->FindWidget("CC_Face");
		pCostumeCB = (MComboBox*)pResource->FindWidget("CC_Costume");

		if (!pSexCB || !pHairCB || !pFaceCB || !pCostumeCB || !pEdit)
			return true;

		int nNameLen = (int)strlen( pEdit->GetText());

		if ( nNameLen <= 0)
		{
			ZApplication::GetGameInterface()->ShowErrorMessage( MERR_PLZ_INPUT_CHARNAME);
			return true;
		}
		else if ( nNameLen < MIN_CHARNAME)
		{
			ZApplication::GetGameInterface()->ShowErrorMessage( MERR_TOO_SHORT_NAME);
			return true;
		}
		else if ( nNameLen > MAX_CHARNAME)
		{
			ZApplication::GetGameInterface()->ShowErrorMessage( MERR_TOO_LONG_NAME);
			return true;
		}

		bool bIsAbuse = MGetChattingFilter()->IsValidName( pEdit->GetText());

		if ( !bIsAbuse)
		{
			char szMsg[ 256];
			ZTransMsg( szMsg, MSG_WRONG_WORD_NAME, 1, MGetChattingFilter()->GetLastFilteredStr());
			ZApplication::GetGameInterface()->ShowMessage( szMsg, NULL, MSG_WRONG_WORD_NAME);

			return true;
		}

		ZPostCreateMyChar( ZGetGameClient()->GetPlayerUID(), nEmptySlotIndex, (char*)pEdit->GetText(), pSexCB->GetSelIndex(),
		                   pHairCB->GetSelIndex(), pFaceCB->GetSelIndex(), pCostumeCB->GetSelIndex());

	}
END_IMPLEMENT_LISTENER()


void SetCharacterInfoGroup(int n)
{
	ZIDLResource* pResource = ZApplication::GetGameInterface()->GetIDLResource();
	MButton* pBtn = (MButton*)pResource->FindWidget("ShowChar_infoGroup");
	if(pBtn)pBtn->SetCheck(n==0);
	pBtn = (MButton*)pResource->FindWidget("ShowEquip_InfoGroup");
	if(pBtn)pBtn->SetCheck(n==1);

	MWidget *pFrame=(MFrame*)pResource->FindWidget("Char_infoGroup");
	if(pFrame) pFrame->Show(n==0);
	pFrame=(MFrame*)pResource->FindWidget("Equip_InfoGroup");
	if(pFrame) pFrame->Show(n==1);
}

BEGIN_IMPLEMENT_LISTENER(ZGetShowCharInfoGroupListener, MBTN_CLK_MSG)
SetCharacterInfoGroup(0);
END_IMPLEMENT_LISTENER()

BEGIN_IMPLEMENT_LISTENER(ZGetShowEquipInfoGroupListener, MBTN_CLK_MSG)
SetCharacterInfoGroup(1);
END_IMPLEMENT_LISTENER()

BEGIN_IMPLEMENT_LISTENER(ZGetCancelCreateCharacterButtonListener, MBTN_CLK_MSG)
	ZApplication::GetGameInterface()->SetState(GUNZ_CHARSELECTION);
END_IMPLEMENT_LISTENER()



BEGIN_IMPLEMENT_LISTENER(ZChangeCreateCharInfoListener, MCMBBOX_CHANGED)
	if (ZApplication::GetGameInterface()->GetCharacterSelectView() != NULL)
	{
		ZApplication::GetGameInterface()->GetCharacterSelectView()->OnChangedCharCostume();
	}
END_IMPLEMENT_LISTENER()

BEGIN_IMPLEMENT_LISTENER(ZGetStageForcedEntryToGameListener, MBTN_CLK_MSG)
	if(ZGetGameClient()->GetMatchStageSetting()->GetMapName()[0]!=0)
	{
		ZApplication::GetStageInterface()->ChangeStageEnableReady( true);

		ZPostRequestForcedEntry(ZGetGameClient()->GetPlayerUID(), ZGetGameClient()->GetStageUID());
	}
END_IMPLEMENT_LISTENER()

BEGIN_IMPLEMENT_LISTENER(ZGetAllEquipmentListCallerButtonListener, MBTN_CLK_MSG)
	ZApplication::GetGameInterface()->SelectShopTab(0);
END_IMPLEMENT_LISTENER()
BEGIN_IMPLEMENT_LISTENER(ZGetMyAllEquipmentListCallerButtonListener, MBTN_CLK_MSG)
	ZApplication::GetGameInterface()->SelectShopTab(1);
END_IMPLEMENT_LISTENER()
BEGIN_IMPLEMENT_LISTENER(ZGetCashEquipmentListCallerButtonListener, MBTN_CLK_MSG)
	ZApplication::GetGameInterface()->SelectShopTab(2);
END_IMPLEMENT_LISTENER()


BEGIN_IMPLEMENT_LISTENER(ZGetEquipmentCharacterTabButtonListener, MBTN_CLK_MSG)
	ZApplication::GetGameInterface()->SelectEquipmentTab(0);
END_IMPLEMENT_LISTENER()

BEGIN_IMPLEMENT_LISTENER(ZGetEquipmentAccountTabButtonListener, MBTN_CLK_MSG)
#define MIN_ACCOUNT_ITEM_REQUEST_TIME		2000

	ZApplication::GetGameInterface()->SelectEquipmentTab(1);

	ZPostRequestAccountItemList(ZGetGameClient()->GetPlayerUID());
END_IMPLEMENT_LISTENER()

class ZLevelConfirmListener : public MListener{
public:
	virtual bool OnCommand(MWidget* pWidget, const char* szMessage){
		if(MWidget::IsMsg(szMessage, MMSGBOX_YES)==true){
			ZApplication::GetGameInterface()->BringAccountItem();
		} else {
		}
		pWidget->Show(false);
		return false;
	}
} g_LevelConfirmListener;

MListener* ZGetLevelConfirmListenter()
{
	return &g_LevelConfirmListener;
}

BEGIN_IMPLEMENT_LISTENER(ZGetPlayerListPrevListener, MBTN_CLK_MSG)
{
	auto* pWidget = ZFindWidgetAs<ZPlayerListBox>("LobbyChannelPlayerList");
	if (pWidget->GetMode() == ZPlayerListBox::PlayerListMode::ChannelFriend) {
		int iStart = pWidget->GetStartItem();
		if (iStart > 0)
			pWidget->SetStartItem(iStart-1);
		return true;
	}

	int nPage = pWidget->m_nPage;

	nPage--;

	if(nPage < 0) {
		return false;
	}

	ZPostRequestChannelPlayerList(ZGetGameClient()->GetPlayerUID(),ZGetGameClient()->GetChannelUID(),nPage);

}
END_IMPLEMENT_LISTENER()

BEGIN_IMPLEMENT_LISTENER(ZGetPlayerListNextListener, MBTN_CLK_MSG)
{
	auto* pWidget = ZFindWidgetAs<ZPlayerListBox>("LobbyChannelPlayerList");
	if (pWidget->GetMode() == ZPlayerListBox::PlayerListMode::ChannelFriend) {
		int iStart = pWidget->GetStartItem();
		pWidget->SetStartItem(iStart+1);
		return true;
	}

	int nMaxPage = 0;
	if(pWidget->m_nTotalPlayerCount)
		nMaxPage = pWidget->m_nTotalPlayerCount/8;

	int nPage = pWidget->m_nPage;

	nPage++;

	if(nPage > nMaxPage) {
		return false;
	}

	ZPostRequestChannelPlayerList(ZGetGameClient()->GetPlayerUID(),ZGetGameClient()->GetChannelUID(),nPage);

}

END_IMPLEMENT_LISTENER()


BEGIN_IMPLEMENT_LISTENER(ZGetStagePlayerListPrevListener, MBTN_CLK_MSG)
{
	auto* pWidget = ZFindWidgetAs<ZPlayerListBox>("StagePlayerList_");
	if (pWidget->GetMode() == ZPlayerListBox::PlayerListMode::StageFriend) {
		int iStart = pWidget->GetStartItem();
		if (iStart > 0)
			pWidget->SetStartItem(iStart-1);
		return true;
	}

	pWidget->SetStartItem( pWidget->GetStartItem()-1 );
}
END_IMPLEMENT_LISTENER()

BEGIN_IMPLEMENT_LISTENER(ZGetStagePlayerListNextListener, MBTN_CLK_MSG)
{
	auto* pWidget = ZFindWidgetAs<ZPlayerListBox>("StagePlayerList_");
	if (pWidget->GetMode() == ZPlayerListBox::PlayerListMode::StageFriend) {
		int iStart = pWidget->GetStartItem();
		pWidget->SetStartItem(iStart+1);
		return true;
	}

	pWidget->SetStartItem( pWidget->GetStartItem()+1 );
}

END_IMPLEMENT_LISTENER()

BEGIN_IMPLEMENT_LISTENER(ZGetRoomListListener, MLIST_VALUE_CHANGED)
	auto* pWidget = ZFindWidgetAs<ZRoomListBox>("Lobby_StageList");
	pWidget->SetPage();
END_IMPLEMENT_LISTENER()

BEGIN_IMPLEMENT_LISTENER(ZGetLobbyPrevRoomListButtonListener, MBTN_CLK_MSG)
	ZGetGameClient()->RequestPrevStageList();
END_IMPLEMENT_LISTENER()

BEGIN_IMPLEMENT_LISTENER(ZGetLobbyNextRoomListPrevButtonListener, MBTN_CLK_MSG)
	ZGetGameClient()->RequestNextStageList();
END_IMPLEMENT_LISTENER()

BEGIN_IMPLEMENT_LISTENER(ZGetLobbyNextRoomNoButtonListener, MBTN_CLK_MSG)
	MButton *pButton = (MButton*)pWidget;
	int nIndexInGroup = pButton->GetIndexInGroup();
	_ASSERT(0<=nIndexInGroup && nIndexInGroup<6);
	nIndexInGroup++;
	ZGetGameClient()->RequestStageList(nIndexInGroup);
	ZApplication::GetGameInterface()->SetRoomNoLight(nIndexInGroup);
END_IMPLEMENT_LISTENER()


BEGIN_IMPLEMENT_LISTENER( ZGetEquipmentShopCallerButtonListener, MBTN_CLK_MSG)
	ZApplication::GetGameInterface()->ShowEquipmentDialog(false);
	ZApplication::GetGameInterface()->ShowShopDialog(true);
END_IMPLEMENT_LISTENER()

BEGIN_IMPLEMENT_LISTENER( ZGetShopEquipmentCallerButtonListener, MBTN_CLK_MSG)
	ZApplication::GetGameInterface()->ShowShopDialog(false);
	ZApplication::GetGameInterface()->ShowEquipmentDialog(true);
END_IMPLEMENT_LISTENER()

class ZMapListListener : public MListener{
	public:
	virtual bool OnCommand(MWidget* pWidget, const char* szMessage)
	{
		if(MWidget::IsMsg(szMessage, MLB_ITEM_SEL)==true)
		{
			auto* pList = ZFindWidgetAs<MListBox>("MapList");
			if(pList != NULL)
			{
				MComboBox* pCombo = (MComboBox*)ZApplication::GetGameInterface()->GetIDLResource()->FindWidget("MapSelection");
				if(pCombo != NULL )
				{
					int si = pList->GetSelIndex();
					pCombo->SetSelIndex(si);
					PostMapname();
				}
			}

			return true;
		}
		if(MWidget::IsMsg(szMessage, MLB_ITEM_DBLCLK)==true)
		{
			auto* pList = ZFindWidgetAs<MListBox>("MapList");
			if(pList != NULL)
			{
				pList->Show(FALSE);
			}
			return true;
		}
		return false;
	}
} g_MapListListener;

MListener* ZGetStageMapListSelectionListener()
{
	return &g_MapListListener;
}

BEGIN_IMPLEMENT_LISTENER(ZGetStageMapListCallerListener, MBTN_CLK_MSG)
	auto* pList = ZFindWidgetAs<MListBox>("MapList");
	pList->Show(true);
END_IMPLEMENT_LISTENER();



void ZReport112FromListener()
{
	ZIDLResource* pResource = ZApplication::GetGameInterface()->GetIDLResource();

	MWidget* pWidget = pResource->FindWidget( "112Confirm");
	if ( !pWidget)
		return;

	MComboBox* pCombo1 = (MComboBox*)pResource->FindWidget( "112_ConfirmID");
	MComboBox* pCombo2 = (MComboBox*)pResource->FindWidget( "112_ConfirmReason");

	if ( !pCombo1 || !pCombo2)
		return;

	if ( ( pCombo2->GetSelIndex() < 0) || ( pCombo2->GetSelIndex() < 1))
		return;

	__time64_t long_time;
	_time64( &long_time);
	struct tm LocalTime;
	auto err = _localtime64_s(&LocalTime, &long_time);

	char szBuff[ 256];
	sprintf_safe( szBuff, "%s\n%s\n%03d:%s\n%04d-%02d-%02d %02d:%02d:%02d\n",
		ZGetMyInfo()->GetCharName(), pCombo1->GetSelItemString(),
		100+pCombo2->GetSelIndex(), pCombo2->GetSelItemString(),
		LocalTime.tm_year + 1900, LocalTime.tm_mon + 1, LocalTime.tm_mday,
		LocalTime.tm_hour, LocalTime.tm_min, LocalTime.tm_sec);

	ZApplication::GetGameInterface()->GetChat()->Report112(szBuff);


	pWidget->Show( false);
}


BEGIN_IMPLEMENT_LISTENER(ZGet112ConfirmEditListener, MEDIT_ENTER_VALUE)
	ZReport112FromListener();
END_IMPLEMENT_LISTENER();

BEGIN_IMPLEMENT_LISTENER(ZGet112ConfirmOKButtonListener, MBTN_CLK_MSG)
	ZReport112FromListener();
END_IMPLEMENT_LISTENER();


BEGIN_IMPLEMENT_LISTENER(ZGet112ConfirmCancelButtonListener, MBTN_CLK_MSG)
	ZApplication::GetGameInterface()->Show112Dialog(false);
END_IMPLEMENT_LISTENER();

BEGIN_IMPLEMENT_LISTENER(ZGetClanSponsorAgreementConfirm_OKButtonListener, MBTN_CLK_MSG)
	ZGetGameClient()->AnswerSponsorAgreement(true);

	ZIDLResource* pResource = ZApplication::GetGameInterface()->GetIDLResource();
	MWidget* pWidget = pResource->FindWidget("ClanSponsorAgreementConfirm");
	if(pWidget!=NULL)
	{
		pWidget->Show(false);
	}
END_IMPLEMENT_LISTENER();

BEGIN_IMPLEMENT_LISTENER(ZGetClanSponsorAgreementConfirm_CancelButtonListener, MBTN_CLK_MSG)
	ZGetGameClient()->AnswerSponsorAgreement(false);

	ZIDLResource* pResource = ZApplication::GetGameInterface()->GetIDLResource();
	MWidget* pWidget = pResource->FindWidget("ClanSponsorAgreementConfirm");
	if(pWidget!=NULL)
	{
		pWidget->Show(false);
	}
END_IMPLEMENT_LISTENER();

BEGIN_IMPLEMENT_LISTENER(ZGetClanSponsorAgreementWait_CancelButtonListener, MBTN_CLK_MSG)
	ZIDLResource* pResource = ZApplication::GetGameInterface()->GetIDLResource();
	MWidget* pWidget = pResource->FindWidget("ClanSponsorAgreementWait");
	if(pWidget!=NULL)
	{
		pWidget->Show(false);
	}
END_IMPLEMENT_LISTENER();

BEGIN_IMPLEMENT_LISTENER(ZGetClanJoinerAgreementConfirm_OKButtonListener, MBTN_CLK_MSG)
	ZGetGameClient()->AnswerJoinerAgreement(true);

	ZIDLResource* pResource = ZApplication::GetGameInterface()->GetIDLResource();
	MWidget* pWidget = pResource->FindWidget("ClanJoinerAgreementConfirm");
	if(pWidget!=NULL)
	{
		pWidget->Show(false);
	}
END_IMPLEMENT_LISTENER();

BEGIN_IMPLEMENT_LISTENER(ZGetClanJoinerAgreementConfirm_CancelButtonListener, MBTN_CLK_MSG)
	ZGetGameClient()->AnswerJoinerAgreement(false);

	ZIDLResource* pResource = ZApplication::GetGameInterface()->GetIDLResource();
	MWidget* pWidget = pResource->FindWidget("ClanJoinerAgreementConfirm");
	if(pWidget!=NULL)
	{
		pWidget->Show(false);
	}
END_IMPLEMENT_LISTENER();

BEGIN_IMPLEMENT_LISTENER(ZGetClanJoinerAgreementWait_CancelButtonListener, MBTN_CLK_MSG)
	ZIDLResource* pResource = ZApplication::GetGameInterface()->GetIDLResource();
	MWidget* pWidget = pResource->FindWidget("ClanJoinerAgreementWait");
	if(pWidget!=NULL)
	{
		pWidget->Show(false);
	}
END_IMPLEMENT_LISTENER();

BEGIN_IMPLEMENT_LISTENER(ZGetLobbyPlayerListTabClanCreateButtonListener, MBTN_CLK_MSG)
	ZIDLResource* pResource = ZApplication::GetGameInterface()->GetIDLResource();
	MWidget* pWidget = pResource->FindWidget("ClanCreateDialog");
	if(pWidget!=NULL)
	{
		pWidget->Show(true,true);

		u32 nPlaceFilter = 0;
		SetBitSet(nPlaceFilter, MMP_LOBBY);

		ZPostRequestChannelAllPlayerList(ZGetGameClient()->GetPlayerUID(), ZGetGameClient()->GetChannelUID(),nPlaceFilter,
			MCP_MATCH_CHANNEL_REQUEST_ALL_PLAYER_LIST_NONCLAN);
	}
END_IMPLEMENT_LISTENER();

BEGIN_IMPLEMENT_LISTENER(ZGetClanCreateDialogOk, MBTN_CLK_MSG)
	ZIDLResource* pResource = ZApplication::GetGameInterface()->GetIDLResource();
	MWidget* pWidget = pResource->FindWidget("ClanCreateDialog");
	if(pWidget!=NULL)
	{
		pWidget->Show(false);

		ZPlayerSelectListBox *pPlayerList = (ZPlayerSelectListBox*)pResource->FindWidget("ClanSponsorSelect");
		if(pPlayerList)
		{
#if CLAN_SPONSORS_COUNT > 0
			char szSponsors[CLAN_SPONSORS_COUNT][MATCHOBJECT_NAME_LENGTH];
			char *ppSponsors[CLAN_SPONSORS_COUNT];
			int nCount = 0;
			for(int i=0;i<pPlayerList->GetCount();i++)
			{
				MListItem *pItem = pPlayerList->Get(i);
				if(pItem->m_bSelected) {
					if(nCount>=CLAN_SPONSORS_COUNT) break;
					strcpy_safe(szSponsors[nCount],pItem->GetString());
					ppSponsors[nCount]=szSponsors[nCount];
					nCount ++;
				}
			}
#else
			char** ppSponsors = nullptr;
			int nCount = 0;
#endif

			if ( nCount==CLAN_SPONSORS_COUNT)
			{
				MEdit *pEditClanName= (MEdit*)pResource->FindWidget("ClanCreate_ClanName");
				if ( !pEditClanName)
					return true;


				int nNameLen = (int)strlen( pEditClanName->GetText());

				if ( nNameLen <= 0)
				{
					ZApplication::GetGameInterface()->ShowErrorMessage( MERR_PLZ_INPUT_CHARNAME);
					return true;
				}
				else if ( nNameLen < MIN_CLANNAME)
				{
					ZApplication::GetGameInterface()->ShowErrorMessage( MERR_TOO_SHORT_NAME);
					return true;
				}
				else if ( nNameLen > MAX_CLANNAME)
				{
					ZApplication::GetGameInterface()->ShowErrorMessage( MERR_TOO_LONG_NAME);
					return true;
				}

				if( !MGetChattingFilter()->IsValidName(pEditClanName->GetText()) ){
					char szMsg[ 256 ];
					ZTransMsg( szMsg, MSG_WRONG_WORD_NAME, 1, MGetChattingFilter()->GetLastFilteredStr());
					ZApplication::GetGameInterface()->ShowMessage( szMsg, NULL, MSG_WRONG_WORD_NAME );
				}
				else if(pEditClanName)
				{
					char szClanName[CLAN_NAME_LENGTH]={0,};
					strcpy_safe(szClanName,pEditClanName->GetText());
					ZGetGameClient()->RequestCreateClan(szClanName, ppSponsors);
				}
			}
			else
			{
				char szMsgBox[256];
				char szArg[20];
				itoa_safe(CLAN_SPONSORS_COUNT, szArg, 10);

				ZTransMsg(szMsgBox, MSG_CLAN_CREATE_NEED_SOME_SPONSOR, 1, szArg);
				ZApplication::GetGameInterface()->ShowMessage(szMsgBox, NULL, MSG_CLAN_CREATE_NEED_SOME_SPONSOR);
			}
		}
	}
END_IMPLEMENT_LISTENER();

BEGIN_IMPLEMENT_LISTENER(ZGetClanCreateDialogClose, MBTN_CLK_MSG)
	ZIDLResource* pResource = ZApplication::GetGameInterface()->GetIDLResource();
	MWidget* pWidget = pResource->FindWidget("ClanCreateDialog");
	if(pWidget!=NULL)
	{
		pWidget->Show(false);
	}
END_IMPLEMENT_LISTENER();

class ZClanCloseConfirmListener : public MListener{
public:
	virtual bool OnCommand(MWidget* pWidget, const char* szMessage){
		if(MWidget::IsMsg(szMessage, MMSGBOX_YES)==true){
			char szClanName[256];
			strcpy_safe(szClanName, ZGetMyInfo()->GetClanName());
			ZPostRequestCloseClan(ZGetGameClient()->GetPlayerUID(), szClanName);
		}
		pWidget->Show(false);
		return false;
	}
} g_ClanCloseConfirmListener;

MListener* ZGetClanCloseConfirmListenter()
{
	return &g_ClanCloseConfirmListener;
}

class ZClanLeaveConfirmListener : public MListener{
public:
	virtual bool OnCommand(MWidget* pWidget, const char* szMessage){
		if(MWidget::IsMsg(szMessage, MMSGBOX_YES)==true){
			ZPostRequestLeaveClan(ZGetMyUID());
		} else {
		}
		pWidget->Show(false);
		return false;
	}
} g_ClanLeaveConfirmListener;

MListener* ZGetClanLeaveConfirmListenter()
{
	return &g_ClanLeaveConfirmListener;
}

BEGIN_IMPLEMENT_LISTENER(ZGetArrangedTeamGameListener, MBTN_CLK_MSG)
	ZIDLResource* pResource = ZApplication::GetGameInterface()->GetIDLResource();
	MWidget* pWidget = pResource->FindWidget("ArrangedTeamGameDialog");
	if(pWidget!=NULL)
	{
		pWidget->Show(true,true);

		u32 nPlaceFilter = 0;
		SetBitSet(nPlaceFilter, MMP_LOBBY);

		ZPostRequestChannelAllPlayerList(ZGetGameClient()->GetPlayerUID(), ZGetGameClient()->GetChannelUID(),nPlaceFilter,
			MCP_MATCH_CHANNEL_REQUEST_ALL_PLAYER_LIST_MYCLAN);
	}
END_IMPLEMENT_LISTENER();

BEGIN_IMPLEMENT_LISTENER(ZGetArrangedTeamDialogOkListener, MBTN_CLK_MSG)
	ZIDLResource* pResource = ZApplication::GetGameInterface()->GetIDLResource();
	MWidget* pWidget = pResource->FindWidget("ArrangedTeamGameDialog");
	if(pWidget!=NULL)
		pWidget->Show(false);

	ZPlayerSelectListBox *pPlayerList = (ZPlayerSelectListBox*)pResource->FindWidget("ArrangedTeamSelect");
	if(pPlayerList)
	{
		const int nMaxInviteCount = max(MAX_LADDER_TEAM_MEMBER,MAX_CLANBATTLE_TEAM_MEMBER) - 1;

		char szNames[nMaxInviteCount][MATCHOBJECT_NAME_LENGTH];
		char *ppNames[nMaxInviteCount];
		int nCount = 0;
		for(int i=0;i<pPlayerList->GetCount();i++)
		{
			MListItem *pItem = pPlayerList->Get(i);
			if(pItem->m_bSelected) {
				if(nCount>=nMaxInviteCount) {
					nCount++;
					break;
				}
				strcpy_safe(szNames[nCount],pItem->GetString());
				ppNames[nCount]=szNames[nCount];
				nCount ++;
			}
		}

		switch (ZGetGameClient()->GetServerMode())
		{
		case MSM_LADDER:
			{
				if(0<nCount && nCount<=nMaxInviteCount) {
					ZGetGameClient()->RequestProposal(MPROPOSAL_LADDER_INVITE, ppNames, nCount);
				}else
				{
					ZChatOutput(MCOLOR(ZCOLOR_CHAT_SYSTEM),
						ZErrStr(MSG_LADDER_INVALID_COUNT) );
				}
			}
			break;
		case MSM_CLAN:
			{
				bool bRightMember = false;
				for (int i = 0; i < MLADDERTYPE_MAX; i++)
				{
					if ((g_nNeedLadderMemberCount[i]-1) == nCount)
					{
						bRightMember = true;
						break;
					}
				}

				if((0<nCount) && (bRightMember))
				{
					ZGetGameClient()->RequestProposal(MPROPOSAL_CLAN_INVITE, ppNames, nCount);
				}
#ifdef _DEBUG
				else if (nCount == 0)
				{
					char szMember[1][MATCHOBJECT_NAME_LENGTH];
					char* ppMember[1];

					ppMember[0] = szMember[0];
					strcpy_safe(szMember[0], ZGetMyInfo()->GetCharName());

					int nBalancedMatching = 0;
					ZIDLResource* pResource = ZApplication::GetGameInterface()->GetIDLResource();
					MButton* pButton = (MButton*)pResource->FindWidget("BalancedMatchingCheckBox");
					if ((pButton) && (pButton->GetCheck()))
					{
						nBalancedMatching = 1;
					}

					ZPostLadderRequestChallenge(ppMember, 1, nBalancedMatching);
				}
#endif
				else
				{
					ZChatOutput(MCOLOR(ZCOLOR_CHAT_SYSTEM),
						ZMsg(MSG_LADDER_INVALID_COUNT) );
				}
			}
			break;
		case MSM_TEST:
		{
			bool bRightMember = false;
			for (int i = 0; i < MLADDERTYPE_MAX; i++)
			{
				if ((g_nNeedLadderMemberCount[i] - 1) == nCount)
				{
					bRightMember = true;
					break;
				}
			}

			if ((0 < nCount) && (bRightMember))
			{
				ZGetGameClient()->RequestProposal(MPROPOSAL_CLAN_INVITE, ppNames, nCount);
			}
#ifdef _DEBUG
			else if (nCount == 0)
			{
				char szMember[1][MATCHOBJECT_NAME_LENGTH];
				char* ppMember[1];

				ppMember[0] = szMember[0];
				strcpy_safe(szMember[0], ZGetMyInfo()->GetCharName());

				int nBalancedMatching = 0;
				ZIDLResource* pResource = ZApplication::GetGameInterface()->GetIDLResource();
				MButton* pButton = (MButton*)pResource->FindWidget("BalancedMatchingCheckBox");
				if ((pButton) && (pButton->GetCheck()))
				{
					nBalancedMatching = 1;
				}

				ZPostLadderRequestChallenge(ppMember, 1, nBalancedMatching);
			}
#endif
			else
			{
				ZChatOutput(MCOLOR(ZCOLOR_CHAT_SYSTEM),
					ZMsg(MSG_LADDER_INVALID_COUNT));
			}
		}
		break;
		}
	}
END_IMPLEMENT_LISTENER();

BEGIN_IMPLEMENT_LISTENER(ZGetArrangedTeamDialogCloseListener, MBTN_CLK_MSG)
	ZIDLResource* pResource = ZApplication::GetGameInterface()->GetIDLResource();
	MWidget* pWidget = pResource->FindWidget("ArrangedTeamGameDialog");
	if(pWidget!=NULL)
		pWidget->Show(false);

	pWidget = pResource->FindWidget("LobbyFindClanTeam");
	if(pWidget!=NULL)
		pWidget->Show(false);
END_IMPLEMENT_LISTENER();


BEGIN_IMPLEMENT_LISTENER(ZGetProposalAgreementWait_CancelButtonListener, MBTN_CLK_MSG)
	ZIDLResource* pResource = ZApplication::GetGameInterface()->GetIDLResource();
	MWidget* pWidget = pResource->FindWidget("ProposalAgreementWait");
	if(pWidget!=NULL)
	{
		pWidget->Show(false);
	}
END_IMPLEMENT_LISTENER();

BEGIN_IMPLEMENT_LISTENER(ZGetProposalAgreementConfirm_OKButtonListener, MBTN_CLK_MSG)
	ZGetGameClient()->ReplyAgreement(true);

	ZIDLResource* pResource = ZApplication::GetGameInterface()->GetIDLResource();
	MWidget* pWidget = pResource->FindWidget("ProposalAgreementConfirm");
	if(pWidget!=NULL)
	{
		pWidget->Show(false);
	}
END_IMPLEMENT_LISTENER();

BEGIN_IMPLEMENT_LISTENER(ZGetProposalAgreementConfirm_CancelButtonListener, MBTN_CLK_MSG)
	ZGetGameClient()->ReplyAgreement(false);

	ZIDLResource* pResource = ZApplication::GetGameInterface()->GetIDLResource();
	MWidget* pWidget = pResource->FindWidget("ProposalAgreementConfirm");
	if(pWidget!=NULL)
	{
		pWidget->Show(false);
	}
END_IMPLEMENT_LISTENER();

BEGIN_IMPLEMENT_LISTENER(ZGetArrangedTeamGame_CancelListener, MBTN_CLK_MSG)
	ZIDLResource* pResource = ZApplication::GetGameInterface()->GetIDLResource();
	MWidget* pWidget = pResource->FindWidget("LobbyFindClanTeam");
	if(pWidget!=NULL)
		pWidget->Show(false);

	ZPostLadderCancel();
END_IMPLEMENT_LISTENER();



BEGIN_IMPLEMENT_LISTENER(ZGetPrivateChannelEnterListener, MBTN_CLK_MSG)
	ZIDLResource* pResource = ZApplication::GetGameInterface()->GetIDLResource();
	MEdit* pEdit = (MEdit*)pResource->FindWidget("PrivateChannelInput");
	if(pEdit!=NULL)
	{
		int nNameLen = (int)strlen( pEdit->GetText());

		if ( nNameLen <= 0)
		{
			ZApplication::GetGameInterface()->ShowErrorMessage( MERR_PLZ_INPUT_CHARNAME);
			return true;
		}
		else if ( nNameLen < MIN_CLANNAME)
		{
			ZApplication::GetGameInterface()->ShowErrorMessage( MERR_TOO_SHORT_NAME);
			return true;
		}
		else if ( nNameLen > MAX_CLANNAME)
		{
			ZApplication::GetGameInterface()->ShowErrorMessage( MERR_TOO_LONG_NAME);
			return true;
		}


		if( !MGetChattingFilter()->IsValidName(pEdit->GetText()) ){
			char szMsg[ 256 ];
			ZTransMsg( szMsg, MSG_WRONG_WORD_NAME, 1, MGetChattingFilter()->GetLastFilteredStr());
			ZApplication::GetGameInterface()->ShowMessage( szMsg, NULL, MSG_WRONG_WORD_NAME );
		}
		else
		{
			ZPostChannelRequestJoinFromChannelName(ZGetMyUID(),MCHANNEL_TYPE_USER,pEdit->GetText());

			MWidget* pFindWidget = pResource->FindWidget("ChannelListFrame");
			if(pFindWidget!=NULL) pFindWidget->Show(false);
		}
	}
END_IMPLEMENT_LISTENER();

BEGIN_IMPLEMENT_LISTENER(ZGetChannelList, MBTN_CLK_MSG)
	MButton *pButton = (MButton*)pWidget;
	int nIndexInGroup = pButton->GetIndexInGroup();
	_ASSERT(0<=nIndexInGroup && nIndexInGroup<3);

	ZIDLResource* pResource = ZApplication::GetGameInterface()->GetIDLResource();

	MCHANNEL_TYPE nChannelType = MCHANNEL_TYPE_PRESET;
	switch(nIndexInGroup) {
	case 0 : nChannelType = MCHANNEL_TYPE_PRESET; break;
	case 1 : nChannelType = MCHANNEL_TYPE_USER; break;
	case 2 : nChannelType = MCHANNEL_TYPE_CLAN; break;

	default : _ASSERT(FALSE);
	}

	ZApplication::GetGameInterface()->InitChannelFrame(nChannelType);
	ZGetGameClient()->StartChannelList(nChannelType);
END_IMPLEMENT_LISTENER()

BEGIN_IMPLEMENT_LISTENER(ZGetMyClanChannel, MBTN_CLK_MSG)
	if(ZGetMyInfo()->IsClanJoined())
	{
		if( !MGetChattingFilter()->IsValidName(ZGetMyInfo()->GetClanName()) ){
			char szMsg[ 256 ];
			ZTransMsg( szMsg, MSG_WRONG_WORD_NAME, 1, MGetChattingFilter()->GetLastFilteredStr());
			ZApplication::GetGameInterface()->ShowMessage( szMsg, NULL, MSG_WRONG_WORD_NAME );
		}
		else
		{
			ZPostChannelRequestJoinFromChannelName(ZGetMyUID(),MCHANNEL_TYPE_CLAN,ZGetMyInfo()->GetClanName());

			ZIDLResource* pResource = ZApplication::GetGameInterface()->GetIDLResource();
			MWidget* pFindWidget = pResource->FindWidget("ChannelListFrame");
			if(pFindWidget!=NULL) pFindWidget->Show(false);
		}

	}else {
		ZGetGameInterface()->ShowMessage( MSG_LOBBY_NO_CLAN );
	}
END_IMPLEMENT_LISTENER();


// Shop list frame open/close
BEGIN_IMPLEMENT_LISTENER(ZShopListFrameClose, MBTN_CLK_MSG)
	ZApplication::GetGameInterface()->SelectEquipmentFrameList( "Shop", false);
END_IMPLEMENT_LISTENER();

BEGIN_IMPLEMENT_LISTENER(ZShopListFrameOpen, MBTN_CLK_MSG)
	ZApplication::GetGameInterface()->SelectEquipmentFrameList( "Shop", true);
END_IMPLEMENT_LISTENER();

// Equipment list frame open/close
BEGIN_IMPLEMENT_LISTENER(ZEquipListFrameClose, MBTN_CLK_MSG)
	ZApplication::GetGameInterface()->SelectEquipmentFrameList( "Equip", false);
END_IMPLEMENT_LISTENER();

BEGIN_IMPLEMENT_LISTENER(ZEquipListFrameOpen, MBTN_CLK_MSG)
	ZApplication::GetGameInterface()->SelectEquipmentFrameList( "Equip", true);
END_IMPLEMENT_LISTENER();

// Equipment character view rotate
BEGIN_IMPLEMENT_LISTENER(ZEquipmetRotateBtn, MBTN_CLK_MSG)
	ZCharacterView* pCharView = (ZCharacterView*)ZApplication::GetGameInterface()->GetIDLResource()->FindWidget( "EquipmentInformation");
	if ( pCharView)
	{
		pCharView->EnableAutoRotate( !pCharView->IsAutoRotate());

		MBmButton* pButton = (MBmButton*)ZApplication::GetGameInterface()->GetIDLResource()->FindWidget( "Equipment_CharacterRotate");
		if ( pButton)
		{
			if ( pCharView->IsAutoRotate())
			{
				pButton->SetUpBitmap(   MBitmapManager::Get( "btn_rotate.tga"));
				pButton->SetDownBitmap( MBitmapManager::Get( "btn_rotate.tga"));
				pButton->SetOverBitmap( MBitmapManager::Get( "btn_rotate.tga"));
			}
			else
			{
				pButton->SetUpBitmap(   MBitmapManager::Get( "btn_stop.tga"));
				pButton->SetDownBitmap( MBitmapManager::Get( "btn_stop.tga"));
				pButton->SetOverBitmap( MBitmapManager::Get( "btn_stop.tga"));
			}
		}
	}
END_IMPLEMENT_LISTENER();


// Replay
BEGIN_IMPLEMENT_LISTENER(ZReplayOk, MBTN_CLK_MSG)
	ZApplication::GetGameInterface()->OnReplay();
END_IMPLEMENT_LISTENER();

BEGIN_IMPLEMENT_LISTENER(ZGetReplayCallerButtonListener, MBTN_CLK_MSG)
	ZApplication::GetGameInterface()->ShowReplayDialog( true);
END_IMPLEMENT_LISTENER();

BEGIN_IMPLEMENT_LISTENER(ZGetReplayViewButtonListener, MBTN_CLK_MSG)
	ZApplication::GetGameInterface()->ViewReplay();
END_IMPLEMENT_LISTENER();

BEGIN_IMPLEMENT_LISTENER(ZGetReplayExitButtonListener, MBTN_CLK_MSG)
	ZApplication::GetGameInterface()->ShowReplayDialog( false);
END_IMPLEMENT_LISTENER();

MListener* ZGetReplayFileListBoxListener( void)
{
	class ListenerClass : public MListener
	{
		public:
		virtual bool OnCommand( MWidget* pWidget, const char* szMessage)
		{
			// Item select
			if ( MWidget::IsMsg( szMessage, MLB_ITEM_SEL) == true)
			{
				MWidget* pFindWidget = ZApplication::GetGameInterface()->GetIDLResource()->FindWidget( "Replay_View");
				if (pFindWidget != NULL)
					pFindWidget->Enable(true);

				GetRGMain().OnReplaySelected(static_cast<MListBox*>(pWidget));

                return true;
			}
			// Item double click
			else if ( MWidget::IsMsg( szMessage, MLB_ITEM_DBLCLK) == true)
			{
				ZApplication::GetGameInterface()->ViewReplay();

                return true;
			}

			return false;
		}
	};
	static ListenerClass	Listener;
	return &Listener;
}


BEGIN_IMPLEMENT_LISTENER(ZGetLeaveClanOKListener, MBTN_CLK_MSG)
	MWidget* pWidget = (MWidget*)ZApplication::GetGameInterface()->GetIDLResource()->FindWidget( "ConfirmLeaveClan");
	if ( pWidget)
		pWidget->Show( false);

	ZPostRequestLeaveClan(ZGetMyUID());

	ZPlayerListBox *pPlayerListBox = (ZPlayerListBox*)ZApplication::GetGameInterface()->GetIDLResource()->FindWidget( "LobbyChannelPlayerList" );
	if(pPlayerListBox)
		pPlayerListBox->SetMode(ZPlayerListBox::PlayerListMode::ChannelClan);
END_IMPLEMENT_LISTENER();

BEGIN_IMPLEMENT_LISTENER(ZGetLeaveClanCancelListener, MBTN_CLK_MSG)
	MWidget* pWidget = (MWidget*)ZApplication::GetGameInterface()->GetIDLResource()->FindWidget( "ConfirmLeaveClan");
	if ( pWidget)
		pWidget->Show( false);
END_IMPLEMENT_LISTENER();

BEGIN_IMPLEMENT_LISTENER( ZStageSacrificeItem0, MBTN_CLK_MSG)
	ZApplication::GetStageInterface()->OnSacrificeItem0();
END_IMPLEMENT_LISTENER();

BEGIN_IMPLEMENT_LISTENER( ZStageSacrificeItem1, MBTN_CLK_MSG)
	ZApplication::GetStageInterface()->OnSacrificeItem1();
END_IMPLEMENT_LISTENER();

BEGIN_IMPLEMENT_LISTENER( ZStagePutSacrificeItem, MBTN_CLK_MSG)
	if ( !ZApplication::GetStageInterface()->m_SacrificeItem[ 0].IsExist())
		ZApplication::GetStageInterface()->OnDropSacrificeItem( 0);
	else if ( !ZApplication::GetStageInterface()->m_SacrificeItem[ 1].IsExist())
		ZApplication::GetStageInterface()->OnDropSacrificeItem( 1);
END_IMPLEMENT_LISTENER();

BEGIN_IMPLEMENT_LISTENER( ZStageSacrificeItemBoxOpen, MBTN_CLK_MSG)
	ZApplication::GetStageInterface()->OpenSacrificeItemBox();
END_IMPLEMENT_LISTENER();

BEGIN_IMPLEMENT_LISTENER( ZStageSacrificeItemBoxClose, MBTN_CLK_MSG)
	ZApplication::GetStageInterface()->CloseSacrificeItemBox();
END_IMPLEMENT_LISTENER();


BEGIN_IMPLEMENT_LISTENER( ZGetGameResultQuit, MBTN_CLK_MSG)
	if ( ZGetGameClient()->IsLadderGame())
		PostMessage(g_hWnd, WM_CHANGE_GAMESTATE, GUNZ_LOBBY, 0);
	else
		PostMessage(g_hWnd, WM_CHANGE_GAMESTATE, GUNZ_STAGE, 0);
END_IMPLEMENT_LISTENER();



BEGIN_IMPLEMENT_LISTENER( ZGetMonsterBookCaller, MBTN_CLK_MSG)
	ZApplication::GetGameInterface()->GetMonsterBookInterface()->OnCreate();
END_IMPLEMENT_LISTENER();


BEGIN_IMPLEMENT_LISTENER( ZGetMonsterInterfacePrevPage, MBTN_CLK_MSG)
	ZApplication::GetGameInterface()->GetMonsterBookInterface()->OnPrevPage();
END_IMPLEMENT_LISTENER();

BEGIN_IMPLEMENT_LISTENER( ZGetMonsterInterfaceNextPage, MBTN_CLK_MSG)
	ZApplication::GetGameInterface()->GetMonsterBookInterface()->OnNextPage();
END_IMPLEMENT_LISTENER();

BEGIN_IMPLEMENT_LISTENER( ZGetMonsterInterfaceQuit, MBTN_CLK_MSG)
	ZApplication::GetGameInterface()->GetMonsterBookInterface()->OnDestroy();
END_IMPLEMENT_LISTENER();


BEGIN_IMPLEMENT_LISTENER( ZGetRegisterListener, MBTN_CLK_MSG)
END_IMPLEMENT_LISTENER();
