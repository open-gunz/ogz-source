#include "stdafx.h"
#include "MErrorTable.h"
#include "ZConfiguration.h"
#include "ZGameClient.h"
#include "MSharedCommandTable.h"
#include "ZConsole.h"
#include "MCommandLogFrame.h"
#include "ZIDLResource.h"
#include "MBlobArray.h"
#include "ZInterface.h"
#include "ZApplication.h"
#include "ZGameInterface.h"
#include "MMatchGlobal.h"
#include "ZCommandTable.h"
#include "ZPost.h"
#include "ZPostLocal.h"
#include "ZMatch.h"
#include "MComboBox.h"
#include "MTextArea.h"
#include "ZCharacterViewList.h"
#include "ZCharacterView.h"
#include "MDebug.h"
#include "ZScreenEffectManager.h"
#include "ZRoomListBox.h"
#include "ZPlayerListBox.h"
#include "ZChat.h"
#include "ZWorldItem.h"
#include "ZChannelRule.h"
#include "ZNetRepository.h"
#include "ZMyInfo.h"
#include "MToolTip.h"
#include "ZColorTable.h"
#include "ZClan.h"
#include "ZSecurity.h"
#include "ZItemDesc.h"
#include "ZCharacterSelectView.h"
#include "ZChannelListItem.h"
#include "ZCombatInterface.h"
#include "ZMap.h"
#include "UPnP.h"
#include "MMatchNotify.h"
#include "MListBox.h"
#include "ZMsgBox.h"
#include "sodium.h"

MCommand* ZNewCmd(int nID)
{
	MCommandDesc* pCmdDesc = ZGetGameClient()->GetCommandManager()->GetCommandDescByID(nID);

	MUID uidTarget;
	if (pCmdDesc->IsFlag(MCDT_PEER2PEER) == true)
		uidTarget = MUID(0, 0);
	else
		uidTarget = ZGetGameClient()->GetServerUID();

	MCommand* pCmd = new MCommand(nID,
		ZGetGameClient()->GetUID(),
		uidTarget,
		ZGetGameClient()->GetCommandManager());
	return pCmd;
}


bool GetUserInfoUID(MUID uid, MCOLOR& _color, char* sp_name, size_t maxlen, MMatchUserGradeID& gid)
{
	if (ZGetGameClient() == NULL)
		return false;

	MMatchObjCache* pObjCache = ZGetGameClient()->FindObjCache(uid);

	if (pObjCache) {
		gid = pObjCache->GetUGrade();
	}

	return GetUserGradeIDColor(gid, _color, sp_name, maxlen);
}


extern MCommandLogFrame* m_pLogFrame;
extern ZIDLResource	g_IDLResource;

MUID ZGetMyUID()
{
	if (!ZGetGameClient()) return MUID(0, 0);
	else return ZGetGameClient()->GetPlayerUID();
}

bool ZPostCommand(MCommand* pCmd)
{
	// Replay
	if (ZGetGame() && ZGetGame()->IsReplay()) {
		switch (pCmd->GetID()) {
		case MC_CLOCK_SYNCHRONIZE:
		case MC_MATCH_USER_WHISPER:
		case MC_MATCH_CHATROOM_JOIN:
		case MC_MATCH_CHATROOM_LEAVE:
		case MC_MATCH_CHATROOM_SELECT_WRITE:
		case MC_MATCH_CHATROOM_INVITE:
		case MC_MATCH_CHATROOM_CHAT:
		case MC_MATCH_CLAN_MSG:
			ZGetGameClient()->Post(pCmd);
			return true;
		default:
			delete pCmd;
			return false;
		};
	}
	else {
		return ZGetGameClient()->Post(pCmd);
	}
}

ZGameClient::ZGameClient() : MMatchClient(), m_pUPnP(NULL)
{
	m_pUPnP = new UPnP;

	m_uidPlayer = MUID(0, 0);
	m_nClockDistance = 0;
	m_fnOnCommandCallback = NULL;
	m_nPrevClockRequestAttribute = 0;
	m_nBridgePeerCount = 0;
	m_tmLastBridgePeer = 0;
	m_bForcedEntry = false;

	m_szChannel[0] = NULL;
	m_szStageName[0] = NULL;
	m_szChatRoomInvited[0] = NULL;
	SetChannelRuleName("");

	m_nRoomNo = 0;
	m_nStageCursor = 0;

	m_nCountdown = 0;
	m_tmLastCountdown = 0;
	m_nRequestID = 0;
	m_uidRequestPlayer = MUID(0, 0);
	m_nProposalMode = MPROPOSAL_NONE;
	m_bLadderGame = false;

	m_CurrentChannelType = MCHANNEL_TYPE_PRESET;

	SetRejectWhisper(true);
	SetRejectInvite(true);

	SetVoteInProgress(false);
	SetCanVote(false);


	m_EmblemMgr.Create();
	m_EmblemMgr.PrepareCache();

#ifdef _LOCATOR
	m_This = MUID(0, 1);
#endif
}

ZGameClient::~ZGameClient()
{
	DestroyUPnP();
	m_EmblemMgr.Destroy();
	ZGetMyInfo()->Clear();
}

void ZGameClient::PriorityBoost(bool bBoost)
{
	if (bBoost)
	{
		SetPriorityClass(GetCurrentProcess(), ABOVE_NORMAL_PRIORITY_CLASS);
		m_bPriorityBoost = true;
		OutputDebugString("<<<<  BOOST ON  >>>>\n");
	}
	else
	{
		SetPriorityClass(GetCurrentProcess(), NORMAL_PRIORITY_CLASS);
		m_bPriorityBoost = false;
		OutputDebugString("<<<<  BOOST OFF  >>>>\n");
	}
}


void ZGameClient::OnRegisterCommand(MCommandManager* pCommandManager)
{
	MMatchClient::OnRegisterCommand(pCommandManager);
	ZAddCommandTable(pCommandManager);
}

void ZGameClient::OnPrepareCommand(MCommand* pCommand)
{
#ifndef _PUBLISH
	m_pLogFrame->AddCommand(GetGlobalClockCount(), pCommand);
#endif
}

int ZGameClient::OnResponseMatchLogin(const MUID& uidServer,
	int nResult,
	const char* szServerName,
	const MMatchServerMode nServerMode,
	const char* szAccountID,
	const MMatchUserGradeID nUGradeID,
	const MMatchPremiumGradeID nPGradeID,
	const MUID& uidPlayer,
	const char* szRandomValue)
{
	int nRet = MMatchClient::OnResponseMatchLogin(uidServer, nResult, szServerName, nServerMode,
		szAccountID, nUGradeID, nPGradeID, uidPlayer, szRandomValue);

	ZGetMyInfo()->InitAccountInfo(szAccountID, nUGradeID, nPGradeID);

	if (nResult == 0 && nRet == MOK) {	// Login successful
		mlog("Login Successful. \n");

		ZApplication::GetGameInterface()->ChangeToCharSelection();
	}
	else
	{								// Login failed
		mlog("Login Failed.(ErrCode=%d) \n", nResult);

		ZPostDisconnect();

		if (nResult != MOK)
		{
			ZApplication::GetGameInterface()->ShowErrorMessage(nResult);
		}
		return MOK;
	}

	ZApplication::GetGameInterface()->ShowWidget("NetmarbleLogin", false);

	StartBridgePeer();

	return MOK;
}

void ZGameClient::OnAnnounce(unsigned int nType, char* szMsg)
{
	ZIDLResource* pResource = ZApplication::GetGameInterface()->GetIDLResource();

	ZChatOutput(szMsg, ZChat::CMT_SYSTEM);
}

void ZGameClient::OnBridgePeerACK(const MUID& uidChar, int nCode)
{
	SetBridgePeerFlag(true);
}

void ZGameClient::OnObjectCache(unsigned int nType, void* pBlob, int nCount)
{
	MMatchClient::OnObjectCache(nType, pBlob, nCount);

	auto* pResource = ZGetGameInterface()->GetIDLResource();
	auto* pList = static_cast<ZPlayerListBox*>(pResource->FindWidget("StagePlayerList_"));

	if (!pList)
		return;

	switch (nType)
	{
	case MATCHCACHEMODE_UPDATE:
		pList->RemoveAll();
		// Fallthrough
	case MATCHCACHEMODE_ADD:
		for (int i = 0; i < nCount; i++)
		{
			MMatchObjCache* pCache = (MMatchObjCache*)MGetBlobArrayElement(pBlob, i);
			if (pCache->CheckFlag(MTD_PlayerFlags_AdminHide)) //  Skip on AdminHide
				continue;

			pList->AddPlayerStage(
				pCache->GetUID(),
				MOSS_NONREADY,
				pCache->GetLevel(),
				pCache->GetName(),
				pCache->GetClanName(), pCache->GetCLID(),
				false, MMT_ALL);

			// Emblem
			ProcessEmblem(pCache->GetCLID(), pCache->GetEmblemChecksum());
		}
		break;
	case MATCHCACHEMODE_REMOVE:
		for (int i = 0; i < nCount; i++) {
			MMatchObjCache* pCache = (MMatchObjCache*)MGetBlobArrayElement(pBlob, i);
			pList->DelPlayer(pCache->GetUID());
		}

		ZApplication::GetGameInterface()->UpdateBlueRedTeam();
		break;
	}
}

void ZGameClient::OnChannelResponseJoin(const MUID& uidChannel, MCHANNEL_TYPE nChannelType,
	const char* szChannelName)
{
	ZApplication::GetGameInterface()->SetState(GUNZ_LOBBY);

	m_uidChannel = uidChannel;
	strcpy_safe(m_szChannel, szChannelName);
	m_CurrentChannelType = nChannelType;

	char szText[256];

	ZGetGameInterface()->GetChat()->Clear(ZChat::CL_LOBBY);
	ZTransMsg(szText, MSG_LOBBY_JOIN_CHANNEL, 1, szChannelName);

	ZChatOutput(szText, ZChat::CMT_SYSTEM, ZChat::CL_LOBBY);

	switch (GetServerMode())
	{
	case MSM_NORMAL_:
	{
		wsprintf(szText,
			ZMsg(MSG_LOBBY_LIMIT_LEVEL));
		ZChatOutput(szText, ZChat::CMT_SYSTEM, ZChat::CL_LOBBY);
	}
	break;
	case MSM_LADDER:
	{
		wsprintf(szText,
			ZMsg(MSG_LOBBY_LEAGUE));
		ZChatOutput(szText, ZChat::CMT_SYSTEM, ZChat::CL_LOBBY);
	}
	break;
	case MSM_CLAN:
	{
		if (nChannelType == MCHANNEL_TYPE_CLAN)
		{
			ZPostRequestClanInfo(GetPlayerUID(), szChannelName);
		}
	}
	break;
	case MSM_TEST:
	{
		if (nChannelType == MCHANNEL_TYPE_CLAN)
		{
			ZPostRequestClanInfo(GetPlayerUID(), szChannelName);
		}
	}
	break;
	}

	{
		auto* IDLResource = ZGetGameInterface()->GetIDLResource();
		auto* pRoomList = static_cast<ZRoomListBox*>(IDLResource->FindWidget("Lobby_StageList"));
		if (pRoomList)
			pRoomList->Clear();
	}

	ZApplication::GetGameInterface()->SetRoomNoLight(1);

	bool bClanBattleUI = ( (GetServerMode() == MSM_CLAN || GetServerMode() == MSM_TEST) && (nChannelType == MCHANNEL_TYPE_CLAN));
	ZGetGameInterface()->InitClanLobbyUI(bClanBattleUI);

#ifdef LIMIT_ACTIONLEAGUE
	bool bActionLeague = (strstr(szChannelName, "¾×¼Ç") != NULL) || (nChannelType == MCHANNEL_TYPE_USER);

	ZGetGameInterface()->InitLadderUI(bActionLeague);
#endif
}

void ZGameClient::OnChannelChat(const MUID& uidChannel, const char* szName, const char* szChat, int nGrade)
{
	if (GetChannelUID() != uidChannel)		return;
	if ((szChat[0] == 0) || (szName[0] == 0))	return;

	MCOLOR _color = MCOLOR(0, 0, 0);

	MMatchUserGradeID gid = (MMatchUserGradeID)nGrade;

	char sp_name[256];

	bool bSpUser = GetUserGradeIDColor(gid, _color, sp_name);

	char szText[512];

	if (bSpUser)
	{
		wsprintf(szText, "%s: %s", szName, szChat);
		ZChatOutput(szText, ZChat::CMT_NORMAL, ZChat::CL_LOBBY, _color);
	}
	else if (!ZGetGameClient()->GetRejectNormalChat() ||
		strcmp(szName, ZGetMyInfo()->GetCharName()) == 0)
	{
		wsprintf(szText, "^4%s^9: %s", szName, szChat);
		ZChatOutput(szText, ZChat::CMT_NORMAL, ZChat::CL_LOBBY);
	}
}

void ZGameClient::OnChannelList(void* pBlob, int nCount)
{
	ZIDLResource* pResource = ZApplication::GetGameInterface()->GetIDLResource();
	MListBox* pWidget = (MListBox*)pResource->FindWidget("ChannelList");
	if (pWidget == NULL) {
		ZGetGameClient()->StopChannelList();
		return;
	}

	int nStartIndex = pWidget->GetStartItem();
	int nSelIndex = pWidget->GetSelIndex();
	pWidget->RemoveAll();
	for (int i = 0; i < nCount; i++) {
		MCHANNELLISTNODE* pNode = (MCHANNELLISTNODE*)MGetBlobArrayElement(pBlob, i);

		pWidget->Add(
			new ZChannelListItem(pNode->uidChannel, (int)pNode->nNo, pNode->szChannelName,
				pNode->nChannelType, (int)pNode->nPlayers, (int)pNode->nMaxPlayers)
			);
	}
	pWidget->SetStartItem(nStartIndex);
	pWidget->SetSelIndex(nSelIndex);
}

void ZGameClient::OnChannelResponseRule(const MUID& uidchannel, const char* pszRuleName)
{
	MChannelRule* pRule = ZGetChannelRuleMgr()->GetRule(pszRuleName);
	if (pRule == NULL)
		return;

	SetChannelRuleName(pszRuleName);

	auto MapSelection = ZFindWidgetAs<MComboBox>("MapSelection");
	if (MapSelection != NULL)
	{
		InitMapSelectionWidget();
		auto MapList = ZFindWidgetAs<MListBox>("MapList");
		if (MapList != NULL)
		{
			MapList->RemoveAll();
			for (int i = 0; i < MapSelection->GetCount(); ++i)
			{
				MapList->Add(MapSelection->GetString(i));
			}
		}
	}
}

void ZGameClient::OnStageEnterBattle(const MUID& uidChar, MCmdEnterBattleParam nParam,
	MTD_PeerListNode* pPeerNode)
{
	DMLog("ZGameClient::OnStageEnterBattle -- Netcode: %d\n", GetMatchStageSetting()->GetNetcode());

	if (uidChar == GetPlayerUID())
	{
		if (GetMatchStageSetting()->GetNetcode() == NetcodeType::ServerBased)
			PeerToPeer = false;
		else
			PeerToPeer = true;

		ZPostRequestGameInfo(ZGetGameClient()->GetPlayerUID(), ZGetGameClient()->GetStageUID());

		ClientSettings.DebugOutput = ZGetConfiguration()->GetShowHitRegDebugOutput();
		ZPostClientSettings(ClientSettings);

		// Unready every player
		for (auto&& CharNode : m_MatchStageSetting.m_CharSettingList)
			CharNode.nState = MOSS_NONREADY;
	}

	if (GetMatchStageSetting()->GetNetcode() != NetcodeType::ServerBased)
		StartUDPTest(uidChar);
}

void ZGameClient::OnStageJoin(const MUID& uidChar, const MUID& uidStage, unsigned int nRoomNo,
	const char* szStageName)
{
	if (uidChar == GetPlayerUID()) {
		JustJoinedStage = true;

		m_nStageCursor = 0;
		m_uidStage = uidStage;
		m_nRoomNo = nRoomNo;

		memset(m_szStageName, 0, sizeof(m_szStageName));
		strcpy_safe(m_szStageName, szStageName); // Save StageName

		unsigned int nStageNameChecksum = m_szStageName[0] + m_szStageName[1] + m_szStageName[2] + m_szStageName[3];
		InitPeerCrypt(uidStage, nStageNameChecksum);
		CastStageBridgePeer(uidChar, uidStage);
	}

	MCommand* pCmd = new MCommand(m_CommandManager.GetCommandDescByID(MC_MATCH_REQUEST_STAGESETTING), GetServerUID(), m_This);
	pCmd->AddParameter(new MCommandParameterUID(GetStageUID()));
	Post(pCmd);

	if (uidChar == GetPlayerUID())
	{
		ZChangeGameState(GUNZ_STAGE);
	}

	string name = GetObjName(uidChar);
	char szText[256];
	if (uidChar == GetPlayerUID())
	{
		ZGetGameInterface()->GetChat()->Clear(ZChat::CL_STAGE);

		char szTmp[256];
		sprintf_safe(szTmp, "(%03d)%s", nRoomNo, szStageName);

		ZTransMsg(szText, MSG_JOINED_STAGE, 1, szTmp);
		ZChatOutput(szText, ZChat::CMT_SYSTEM, ZChat::CL_STAGE);
	}
	else if (GetStageUID() == uidStage)
	{
		char sp_name[256];
		MCOLOR _color;
		MMatchUserGradeID gid = MMUG_FREE;

		if (GetUserInfoUID(uidChar, _color, sp_name, gid))
		{
			MMatchObjCache* pObjCache = ZGetGameClient()->FindObjCache(uidChar);
			if (pObjCache && pObjCache->CheckFlag(MTD_PlayerFlags_AdminHide))
				return;	// Skip on AdminHide

			ZTransMsg(szText, MSG_JOINED_STAGE2, 2, sp_name, szStageName);
			ZChatOutput(szText, ZChat::CMT_SYSTEM, ZChat::CL_STAGE);
		}
		else
		{
			ZTransMsg(szText, MSG_JOINED_STAGE2, 2, name.c_str(), szStageName);
			ZChatOutput(szText, ZChat::CMT_SYSTEM, ZChat::CL_STAGE);
		}
	}
}

void ZGameClient::OnStageLeave(const MUID& uidChar, const MUID& uidStage)
{
	if (uidChar == GetPlayerUID()) {
		m_uidStage = MUID(0, 0);
		m_nRoomNo = 0;
	}


	if (uidChar == GetPlayerUID())
	{
		ZChangeGameState(GUNZ_LOBBY);
	}

	ZGetGameClient()->SetVoteInProgress(false);
	ZGetGameClient()->SetCanVote(false);

	AgentDisconnect();
}

void ZGameClient::OnStageStart(const MUID& uidChar, const MUID& uidStage, int nCountdown)
{
	SetCountdown(nCountdown);
}

void ZGameClient::OnStageLaunch(const MUID& uidStage, const char* pszMapName)
{
	m_bLadderGame = false;

	SetAllowTunneling(false);

	m_MatchStageSetting.SetMapName(const_cast<char*>(pszMapName));

	if (ZApplication::GetGameInterface()->GetState() != GUNZ_GAME)
	{
		ZChangeGameState(GUNZ_GAME);
	}
}

void ZGameClient::OnStageFinishGame(const MUID& uidStage)
{
	if (ZApplication::GetGameInterface()->GetState() == GUNZ_GAME)
	{
		ZApplication::GetGameInterface()->FinishGame();
	}
	ZPostRequestStageSetting(ZGetGameClient()->GetStageUID());
}

void ZGameClient::OnStageMap(const MUID& uidStage, char* szMapName)
{
	if (uidStage != GetStageUID()) return;

	m_MatchStageSetting.SetMapName(szMapName);

	ZApplication::GetGameInterface()->SerializeStageInterface();
}

void ZGameClient::OnStageTeam(const MUID& uidChar, const MUID& uidStage, unsigned int nTeam)
{
	MMatchObjectStageState nStageState = MOSS_NONREADY;
	MSTAGE_CHAR_SETTING_NODE* pCharNode = m_MatchStageSetting.FindCharSetting(uidChar);
	if (pCharNode)
	{
		nStageState = pCharNode->nState;
	}

	m_MatchStageSetting.UpdateCharSetting(uidChar, nTeam, nStageState);
	ZApplication::GetGameInterface()->SerializeStageInterface();
}

void ZGameClient::OnStagePlayerState(const MUID& uidChar, const MUID& uidStage, MMatchObjectStageState nStageState)
{
	int nTeam = MMT_SPECTATOR;
	MSTAGE_CHAR_SETTING_NODE* pCharNode = m_MatchStageSetting.FindCharSetting(uidChar);
	if (pCharNode) nTeam = pCharNode->nTeam;

	m_MatchStageSetting.UpdateCharSetting(uidChar, nTeam, nStageState);
	ZApplication::GetGameInterface()->SerializeStageInterface();
}

void ZGameClient::OnStageMaster(const MUID& uidStage, const MUID& uidChar)
{
	int nTeam = MMT_SPECTATOR;
	MMatchObjectStageState nStageState = MOSS_NONREADY;
	MSTAGE_CHAR_SETTING_NODE* pCharNode = m_MatchStageSetting.FindCharSetting(uidChar);
	if (pCharNode)
	{
		nTeam = pCharNode->nTeam;
		nStageState = pCharNode->nState;
	}

	m_MatchStageSetting.SetMasterUID(uidChar);
	m_MatchStageSetting.UpdateCharSetting(uidChar, nTeam, nStageState);

	ZApplication::GetGameInterface()->SerializeStageInterface();
}

void ZGameClient::OnStageChat(const MUID& uidChar, const MUID& uidStage, const char* szChat)
{
	if (GetStageUID() != uidStage) return;
	if (szChat[0] == 0) return;

	string name = GetObjName(uidChar);

	MCOLOR _color = MCOLOR(0, 0, 0);

	MMatchUserGradeID gid = MMUG_FREE;

	MMatchObjCache* pObjCache = FindObjCache(uidChar);

	if (pObjCache) {
		gid = pObjCache->GetUGrade();
	}

	char sp_name[256];

	bool bSpUser = GetUserGradeIDColor(gid, _color, sp_name);

	char szText[512];

	if (bSpUser)
	{
		wsprintf(szText, "%s: %s", name.c_str(), szChat);
		ZChatOutput(szText, ZChat::CMT_NORMAL, ZChat::CL_STAGE, _color);
	}
	else if (!ZGetGameClient()->GetRejectNormalChat() ||
		(strcmp(pObjCache->GetName(), ZGetMyInfo()->GetCharName()) == 0))
	{
		wsprintf(szText, "^4%s^9: %s", name.c_str(), szChat);
		ZChatOutput(szText, ZChat::CMT_NORMAL, ZChat::CL_STAGE);
	}
}

void ZGameClient::OnStageList(int nPrevStageCount, int nNextStageCount, void* pBlob, int nCount)
{
#ifdef _DEBUG
	char szTemp[256];
	sprintf_safe(szTemp, "OnStageList (nPrevStageCount = %d , nNextStageCount = %d , nCount = %d\n",
		nPrevStageCount, nNextStageCount, nCount);
	OutputDebugString(szTemp);
#endif
	ZIDLResource* pResource = ZApplication::GetGameInterface()->GetIDLResource();
	ZRoomListBox* pRoomListBox = (ZRoomListBox*)pResource->FindWidget("Lobby_StageList");
	if (pRoomListBox == NULL)
	{
		ZGetGameClient()->StopStageList();
		return;
	}

	pRoomListBox->Clear();
	for (int i = 0; i < nCount; i++) {

		MTD_StageListNode* pNode = (MTD_StageListNode*)MGetBlobArrayElement(pBlob, i);

		// log debug
		if (pNode)
		{
			bool bForcedEntry = false, bPrivate = false, bLimitLevel = false;
			int nLimitLevel = 0;
			if (pNode->nSettingFlag & MSTAGENODE_FLAG_FORCEDENTRY_ENABLED) bForcedEntry = true;
			if (pNode->nSettingFlag & MSTAGENODE_FLAG_PRIVATE) bPrivate = true;
			if (pNode->nSettingFlag & MSTAGENODE_FLAG_LIMITLEVEL) bLimitLevel = true;

			char szMapName[256] = "";
			for (int tt = 0; tt < MMATCH_MAP_COUNT; tt++)
			{
				if (g_MapDesc[tt].nMapID == pNode->nMapIndex)
				{
					strcpy_safe(szMapName, g_MapDesc[tt].szMapName);
					break;
				}
			}

			ZRoomListBox::_RoomInfoArg roominfo;
			roominfo.nIndex = i;
			roominfo.nRoomNumber = (int)pNode->nNo;
			roominfo.uidStage = pNode->uidStage;
			roominfo.szRoomName = pNode->szStageName;
			roominfo.szMapName = szMapName;
			roominfo.nMaxPlayers = pNode->nMaxPlayers;
			roominfo.nCurrPlayers = pNode->nPlayers;
			roominfo.bPrivate = bPrivate;
			roominfo.bForcedEntry = bForcedEntry;
			roominfo.bLimitLevel = bLimitLevel;
			roominfo.nMasterLevel = pNode->nMasterLevel;
			roominfo.nLimitLevel = pNode->nLimitLevel;
			roominfo.nGameType = pNode->nGameType;
			roominfo.nStageState = pNode->nState;
			pRoomListBox->SetRoom(&roominfo);
		}
	}
	pRoomListBox->SetScroll(nPrevStageCount, nNextStageCount);

	MWidget* pBtn = pResource->FindWidget("StageBeforeBtn");
	if (nPrevStageCount != -1)
	{
		if (nPrevStageCount == 0)
		{
			if (pBtn) pBtn->Enable(false);
		}
		else
		{
			if (pBtn) pBtn->Enable(true);
		}
	}

	pBtn = pResource->FindWidget("StageAfterBtn");
	if (nNextStageCount != -1)
	{
		if (nNextStageCount == 0)
		{
			if (pBtn) pBtn->Enable(false);
		}
		else
		{
			if (pBtn) pBtn->Enable(true);
		}
	}

}

ZPlayerListBox* GetProperFriendListOutput()
{
	ZIDLResource* pIDLResource = ZApplication::GetGameInterface()->GetIDLResource();

	GunzState nState = ZApplication::GetGameInterface()->GetState();
	switch (nState) {
	case GUNZ_LOBBY:
	{
		ZPlayerListBox* pList = (ZPlayerListBox*)pIDLResource->FindWidget("LobbyChannelPlayerList");
		if (pList && pList->GetMode() == ZPlayerListBox::PlayerListMode::ChannelFriend)
			return pList;
		else
			return NULL;
	}
	break;
	case GUNZ_STAGE:
	{
		ZPlayerListBox* pList = (ZPlayerListBox*)pIDLResource->FindWidget("StagePlayerList_");
		if (pList && pList->GetMode() == ZPlayerListBox::PlayerListMode::StageFriend)
			return pList;
		else
			return NULL;
	}
	break;
	};
	return NULL;
}

void ZGameClient::OnResponseFriendList(void* pBlob, int nCount)
{
	ZPlayerListBox* pList = GetProperFriendListOutput();
	if (pList)
		pList->RemoveAll();

	char szBuf[128];
	for (int i = 0; i < nCount; i++) {
		MFRIENDLISTNODE* pNode = (MFRIENDLISTNODE*)MGetBlobArrayElement(pBlob, i);

		ePlayerState state;
		switch (pNode->nState)
		{
		case MMP_LOBBY: state = PS_LOBBY; break;
		case MMP_STAGE: state = PS_WAIT; break;
		case MMP_BATTLE: state = PS_FIGHT; break;
		default: state = PS_LOGOUT;
		};

		if (pList) {
			pList->AddPlayerFriend(state, pNode->szName, pNode->szDescription);
		}
		else {
			if (ZApplication::GetGameInterface()->GetState() != GUNZ_LOBBY)
			{
				sprintf_safe(szBuf, "    %s (%s)", pNode->szName, pNode->szDescription);
				ZChatOutput(szBuf, ZChat::CMT_NORMAL, ZChat::CL_CURRENT);
			}
		}
	}
}

void ZGameClient::OnChannelPlayerList(int nTotalPlayerCount, int nPage, void* pBlob, int nCount)
{
	ZIDLResource* pResource = ZApplication::GetGameInterface()->GetIDLResource();
	ZPlayerListBox* pPlayerListBox = (ZPlayerListBox*)pResource->FindWidget("LobbyChannelPlayerList");

	if (!pPlayerListBox) return;
	if (pPlayerListBox->GetMode() != ZPlayerListBox::PlayerListMode::Channel) return;

	MUID selUID = pPlayerListBox->GetSelectedPlayerUID();

	int nStartIndex = pPlayerListBox->GetStartItem();

	if (nCount) {
		pPlayerListBox->RemoveAll();
	}
	else {
		return;
	}

	pPlayerListBox->m_nTotalPlayerCount = nTotalPlayerCount;
	pPlayerListBox->m_nPage = nPage;

	ZLobbyPlayerListItem* pItem = NULL;

	for (int i = 0; i < nCount; i++)
	{
		MTD_ChannelPlayerListNode* pNode = (MTD_ChannelPlayerListNode*)MGetBlobArrayElement(pBlob, i);
		if (pNode)
		{
			ePlayerState state;
			switch (pNode->nPlace)
			{
			case MMP_LOBBY: state = PS_LOBBY; break;
			case MMP_STAGE: state = PS_WAIT; break;
			case MMP_BATTLE: state = PS_FIGHT; break;
			default: state = PS_LOBBY;
			};

			if ((pNode->nPlayerFlags & MTD_PlayerFlags_AdminHide) == true) {
				//  Skip on AdminHide
			}
			else {
				pPlayerListBox->AddPlayerChannel(pNode->uidPlayer, state, pNode->nLevel, pNode->szName,
					pNode->szClanName, pNode->nCLID, (MMatchUserGradeID)pNode->nGrade);
			}

			// Emblem
			ProcessEmblem(pNode->nCLID, pNode->nEmblemChecksum);
		}
	}

	pPlayerListBox->SetStartItem(nStartIndex);
	pPlayerListBox->SelectPlayer(selUID);
}

void ZGameClient::OnChannelAllPlayerList(const MUID& uidChannel, void* pBlob, int nBlobCount)
{
	ZIDLResource* pResource = ZApplication::GetGameInterface()->GetIDLResource();

	MListBox* pListBox = NULL;

	MWidget *pDialog = pResource->FindWidget("ClanCreateDialog");
	if (pDialog && pDialog->IsVisible())
		pListBox = (MListBox*)pResource->FindWidget("ClanSponsorSelect");

	pDialog = pResource->FindWidget("ArrangedTeamGameDialog");
	if (pDialog && pDialog->IsVisible())
		pListBox = (MListBox*)pResource->FindWidget("ArrangedTeamSelect");

	if (pListBox && pListBox->IsVisible()) {
		pListBox->RemoveAll();
		for (int i = 0; i < nBlobCount; i++)
		{
			MTD_ChannelPlayerListNode* pNode = (MTD_ChannelPlayerListNode*)MGetBlobArrayElement(pBlob, i);
			if (pNode)
			{
				if (pNode->uidPlayer != GetPlayerUID())
					pListBox->Add(pNode->szName);
			}
		}
	}
}

static const char* GetNetcodeString(NetcodeType Netcode)
{
	switch (Netcode)
	{
	case NetcodeType::P2PLead:
		return "Peer to Peer Lead";
		break;
	case NetcodeType::P2PAntilead:
		return "Peer to Peer Antilead";
		break;
	case NetcodeType::ServerBased:
		return "Server-based";
	default:
		return "Unknown";
	}
}

void ZGameClient::UpdateStageSetting(MSTAGE_SETTING_NODE* pSetting, STAGE_STATE nStageState, const MUID& uidMaster)
{
	m_MatchStageSetting.UpdateStageSetting(pSetting);
	m_MatchStageSetting.SetMasterUID(uidMaster);
	m_MatchStageSetting.SetStageState(nStageState);

	bool bForceEntry = false;
	if (nStageState != STAGE_STATE_STANDBY)
	{
		bForceEntry = true;
	}
	m_bForcedEntry = bForceEntry;

	char buf[256];
	bool Changed = false;
	auto CheckSetting = [&](auto Old, auto New, auto Default)
	{
		if (JustJoinedStage)
		{
			Changed = false;

			if (New == Default)
				return false;

			return true;
		}

		if (Old != New)
		{
			Changed = true;
			return true;
		}

		Changed = false;
		return false;
	};

	bool CurSwordsOnly = IsSwordsOnly(pSetting->nGameType) || pSetting->SwordsOnly;
	bool LastSwordsOnly = IsSwordsOnly(LastStageSetting.nGameType) || LastStageSetting.SwordsOnly;

#define CHECK_SETTING(member, def) CheckSetting(LastStageSetting.member, pSetting->member, def)

	if (CHECK_SETTING(Netcode, NetcodeType::ServerBased) && !CurSwordsOnly
		&& !(LastSwordsOnly && pSetting->Netcode == NetcodeType::ServerBased))
	{
		sprintf_safe(buf, "Netcode%s%s", Changed ? " changed to " : ": ",
			GetNetcodeString(pSetting->Netcode));
		ZChatOutput(buf, ZChat::CMT_SYSTEM);
	}
	if (CHECK_SETTING(ForceHPAP, true))
	{
		sprintf_safe(buf, "Force HP/AP%s%s", Changed ? " changed to " : ": ",
			pSetting->ForceHPAP ? "true" : "false");
		ZChatOutput(buf, ZChat::CMT_SYSTEM);
	}
	if (CHECK_SETTING(HP, 100))
	{
		sprintf_safe(buf, "Forced HP%s%d", Changed ? " changed to " : ": ",
			pSetting->HP);
		ZChatOutput(buf, ZChat::CMT_SYSTEM);
	}
	if (CHECK_SETTING(AP, 50))
	{
		sprintf_safe(buf, "Forced AP%s%d", Changed ? " changed to " : ": ",
			pSetting->AP);
		ZChatOutput(buf, ZChat::CMT_SYSTEM);
	}
	if (CHECK_SETTING(NoFlip, false))
	{
		sprintf_safe(buf, "No flip%s%s", Changed ? " changed to " : ": ",
			pSetting->NoFlip ? "true" : "false");
		ZChatOutput(buf, ZChat::CMT_SYSTEM);
	}
	if (CHECK_SETTING(SwordsOnly, false))
	{
		sprintf_safe(buf, "Swords only%s%s", Changed ? " changed to " : ": ",
			pSetting->SwordsOnly ? "true" : "false");
		ZChatOutput(buf, ZChat::CMT_SYSTEM);
	}
	if (CHECK_SETTING(VanillaMode, false))
	{
		sprintf_safe(buf, "Vanilla mode%s%s", Changed ? " changed to " : ": ",
			pSetting->VanillaMode ? "true" : "false");
		ZChatOutput(buf, ZChat::CMT_SYSTEM);
	}
	if (CHECK_SETTING(InvulnerabilityStates, false))
	{
		sprintf_safe(buf, "Invulnerability states%s%s", Changed ? " changed to " : ": ",
			pSetting->InvulnerabilityStates ? "true" : "false");
		ZChatOutput(buf, ZChat::CMT_SYSTEM);
	}

#undef CHECK_SETTING

	JustJoinedStage = false;
	CreatedStage = false;
	LastStageSetting = *pSetting;

	ZApplication::GetGameInterface()->SerializeStageInterface();
}


void ZGameClient::OnResponseStageSetting(const MUID& uidStage, const void* pStageBlob,
	int nStageCount, const void* pCharBlob, int nCharCount, STAGE_STATE nStageState, const MUID& uidMaster)
{
	if (GetStageUID() != uidStage) return;
	if (nStageCount <= 0 || nCharCount <= 0) return;

	// Stage setting
	MSTAGE_SETTING_NODE* pNode = (MSTAGE_SETTING_NODE*)MGetBlobArrayElement(pStageBlob, 0);

	DMLog("ZGameClient::OnResponseStageSetting - Netcode = %d\n", pNode->Netcode);
	PeerToPeer = pNode->Netcode != NetcodeType::ServerBased;

	if (pNode->Netcode != NetcodeType::ServerBased
		&& GetMatchStageSetting()->GetNetcode() == NetcodeType::ServerBased)
		StartUDPTest(GetUID());

	UpdateStageSetting(pNode, nStageState, uidMaster);

	// Character setting
	m_MatchStageSetting.ResetCharSetting();
	for (int i = 0; i < nCharCount; i++) {
		MSTAGE_CHAR_SETTING_NODE* pCharSetting = (MSTAGE_CHAR_SETTING_NODE*)MGetBlobArrayElement(pCharBlob, i);
		m_MatchStageSetting.UpdateCharSetting(pCharSetting->uidChar, pCharSetting->nTeam, pCharSetting->nState);
	}

	ZApplication::GetGameInterface()->SerializeStageInterface();
}

void ZGameClient::OnAgentError(int nError)
{
	if (g_pGame) {
		const MCOLOR ChatColor = MCOLOR(0xffffffff);
		char Msg[256];
		sprintf_safe(Msg, "Agent error: Agent not available", nError);
		ZChatOutput(ChatColor, Msg);
	}
}

void ZGameClient::OnMatchNotify(unsigned int nMsgID)
{
	string strMsg;
	NotifyMessage(nMsgID, &strMsg);

	if (nMsgID == MATCHNOTIFY_GAME_SPEEDHACK ||
		nMsgID == MATCHNOTIFY_GAME_MEMORYHACK)
	{
		ZGetGameInterface()->ShowMessage(strMsg.c_str());
	}

	ZChatOutput(MCOLOR(255, 70, 70), strMsg.data());
}

void ZGameClient::OutputMessage(const char* szMessage, MZMOMType nType)
{
	DMLog("%s\n", szMessage);
	OutputToConsole("%s", szMessage);
	ZChatOutput(MCOLOR(0xFFFFC600), szMessage);
}

// Awk
bool g_bConnected = false;
std::function<void()> g_OnConnectCallback = []() {};

int ZGameClient::OnConnected(SOCKET sock, MUID* pTargetUID, MUID* pAllocUID, unsigned int nTimeStamp)
{
	mlog("Server Connected\n");

	int ret = MMatchClient::OnConnected(sock, pTargetUID, pAllocUID, nTimeStamp);

	if (sock == m_ClientSocket.GetSocket()) {
		g_OnConnectCallback();
		g_bConnected = true;
	}

	return ret;
}

bool ZGameClient::OnSockConnect(SOCKET sock)
{
	ZPOSTCMD0(MC_NET_ONCONNECT);
	return MMatchClient::OnSockConnect(sock);
}

bool ZGameClient::OnSockDisconnect(SOCKET sock)
{
	if (sock == m_ClientSocket.GetSocket()) {
		AgentDisconnect();

		if (ZApplication::GetInstance()->GetLaunchMode() == ZApplication::ZLAUNCH_MODE_NETMARBLE) {
			ZChangeGameState(GUNZ_SHUTDOWN);
			ZPOSTCMD0(MC_NET_ONDISCONNECT);
		}
		else {
			ZChangeGameState(GUNZ_LOGIN);
			ZPOSTCMD0(MC_NET_ONDISCONNECT);

			ZIDLResource* pResource = ZApplication::GetGameInterface()->GetIDLResource();
			MButton* pWidget = (MButton*)pResource->FindWidget("LoginOK");
			if (pWidget) pWidget->Enable(true);
			MWidget* pLogin = pResource->FindWidget("LoginFrame");
			if (pLogin) pLogin->Show(true);
			pLogin = pResource->FindWidget("Login_ConnectingMsg");
			if (pLogin) pLogin->Show(false);

			ZGetGameInterface()->m_bLoginTimeout = false;

			g_bConnected = false;
		}
	}
	else if (sock == m_AgentSocket.GetSocket()) {
	}

	return true;
}

void ZGameClient::OnSockError(SOCKET sock, SOCKET_ERROR_EVENT ErrorEvent, int &ErrorCode)
{
	MMatchClient::OnSockError(sock, ErrorEvent, ErrorCode);
	ZPOSTCMD1(MC_NET_ONERROR, MCmdParamInt(ErrorCode));

	if (ZApplication::GetInstance()->GetLaunchMode() == ZApplication::ZLAUNCH_MODE_NETMARBLE) {
		ZIDLResource* pResource = ZApplication::GetGameInterface()->GetIDLResource();
		MLabel* pLabel = (MLabel*)pResource->FindWidget("NetmarbleLoginMessage");
		if (pLabel) {
			pLabel->SetText(
				ZErrStr(MERR_CLIENT_CONNECT_FAILED));
			pLabel->Show();
		}
	}
	else {
		ZIDLResource* pResource = ZApplication::GetGameInterface()->GetIDLResource();
		MButton* pWidget = (MButton*)pResource->FindWidget("LoginOK");
		if (pWidget) pWidget->Enable(true);
		MWidget* pLogin = pResource->FindWidget("LoginFrame");
		if (pLogin) pLogin->Show(true);
		pLogin = pResource->FindWidget("Login_ConnectingMsg");
		if (pLogin) pLogin->Show(false);

		MLabel* pLabel = (MLabel*)pResource->FindWidget("LoginError");
		if (pLabel) {
			pLabel->SetText(ZErrStr(MERR_CLIENT_CONNECT_FAILED));

		}

		ZGetGameInterface()->m_bLoginTimeout = false;
	}
}

class MCharListItem : public MListItem {
	MUID m_uid;
	char m_szName[32];
public:
	MCharListItem(const MUID& uid, const char* szName) {
		m_uid = uid;
		strcpy_safe(m_szName, szName);
	}
	virtual ~MCharListItem() override final {}
	virtual const char* GetString() override final { return m_szName; }
	MUID GetUID() const { return m_uid; }
	const char* GetName() const { return m_szName; }
};

int ZGameClient::FindListItem(MListBox* pListBox, const MUID& uid)
{
	for (int i = 0; i < pListBox->GetCount(); i++) {
		auto* pItem = static_cast<MCharListItem*>(pListBox->Get(i));
		if (pItem->GetUID() == uid) return i;
	}
	return -1;
}

u64 ZGameClient::GetGlobalClockCount() const
{
	auto nLocalClock = GetClockCount();

	if (m_bIsBigGlobalClock)
		return (nLocalClock + m_nClockDistance);

	return (nLocalClock - m_nClockDistance);
}

u32 ZGetClockDistance(u32 nGlobalClock, u32 nLocalClock)
{
	if (nGlobalClock > nLocalClock)
		return nGlobalClock - nLocalClock;

	return nLocalClock + (UINT_MAX - nGlobalClock + 1);
}

void ZGameClient::StartBridgePeer()
{
	SetBridgePeerFlag(false);
	SetBridgePeerCount(10);

	UpdateBridgePeerTime(0);
}

void ZGameClient::Tick()
{
	auto nClock = GetGlobalClockCount();

	m_EmblemMgr.Tick(nClock);

	if ((GetBridgePeerCount() > 0) && (GetBridgePeerFlag() == false)) {
#define CLOCK_BRIDGE_PEER	200
		if (nClock - m_tmLastBridgePeer > CLOCK_BRIDGE_PEER) {
			SetBridgePeerCount(GetBridgePeerCount() - 1);
			UpdateBridgePeerTime(nClock);
			CastStageBridgePeer(GetPlayerUID(), GetStageUID());
		}
	}

	if (GetUDPTestProcess()) {
#define CLOCK_UDPTEST	500
		static u64 nUDPTestTimer = 0;
		if (nClock - nUDPTestTimer > CLOCK_UDPTEST) {
			nUDPTestTimer = nClock;

			auto* PeerList = GetPeers();
			for (auto* pPeer : MakePairValueAdapter(PeerList->MUIDMap))
			{
				if (pPeer->GetProcess()) {
					MCommand* pCmd = CreateCommand(MC_PEER_UDPTEST, pPeer->uidChar);
					SendCommandByUDP(pCmd, pPeer->szIP, pPeer->nPort);
					delete pCmd;
				}
			}

			UpdateUDPTestProcess();
		}
	}

	if ((GetAgentPeerCount() > 0) && (GetAgentPeerFlag() == false)) {
		static u64 tmLastAgentPeer;
#define CLOCK_AGENT_PEER	200
		if (nClock - tmLastAgentPeer > CLOCK_AGENT_PEER) {
			SetAgentPeerCount(GetAgentPeerCount() - 1);
			CastAgentPeerConnect();
			tmLastAgentPeer = nClock;
		}
	}
}

void ZGameClient::OnResponseRecommandedChannel(const MUID& uidChannel)
{
	RequestChannelJoin(uidChannel);
}

void ZGameClient::OnForcedEntryToGame()
{
	m_bLadderGame = false;
	m_bForcedEntry = true;
	SetAllowTunneling(false);
	ZChangeGameState(GUNZ_GAME);
}

void ZGameClient::ClearStageSetting()
{
	m_bForcedEntry = false;

	m_MatchStageSetting.Clear();
}

void ZGameClient::OnLoadingComplete(const MUID& uidChar, int nPercent)
{
	if (ZApplication::GetGame())
	{
		ZCharacter* pCharacter = ZApplication::GetGame()->m_CharacterManager.Find(uidChar);
		if (pCharacter != NULL)
		{
			pCharacter->GetStatus()->nLoadingPercent = nPercent;
		}
	}
}


void ZGameClient::OnResponsePeerRelay(const MUID& uidPeer)
{
	string strNotify = "Unknown Notify";
	NotifyMessage(MATCHNOTIFY_NETWORK_NAT_ESTABLISH, &strNotify);

	char* pszName = "UnknownPlayer";
	MMatchPeerInfo* pPeer = FindPeer(uidPeer);
	if (pPeer) pszName = pPeer->CharInfo.szName;

	char szMsg[128];
	sprintf_safe(szMsg, "%s : from %s", strNotify.c_str(), pszName);


	ZCharacter* pChar = ZGetCharacterManager()->Find(uidPeer);
	if (pChar && pChar->IsAdminHide())
		return;

	ZChatOutput(szMsg, ZChat::CMT_SYSTEM);
}

void ZGameClient::StartStageList()
{
	MCommand* pCmd = new MCommand(m_CommandManager.GetCommandDescByID(MC_MATCH_STAGE_LIST_START), GetServerUID(), m_This);
	Post(pCmd);
}

void ZGameClient::StopStageList()
{
	MCommand* pCmd = new MCommand(m_CommandManager.GetCommandDescByID(MC_MATCH_STAGE_LIST_STOP), GetServerUID(), m_This);
	Post(pCmd);
}

void ZGameClient::StartChannelList(MCHANNEL_TYPE nChannelType)
{
	ZPostStartChannelList(GetPlayerUID(), (int)nChannelType);
}

void ZGameClient::StopChannelList()
{
	ZPostStopChannelList(GetPlayerUID());
}

void ZGameClient::ReleaseForcedEntry()
{
	m_bForcedEntry = false;
}

void ZGameClient::OnAdminAnnounce(const char* szMsg, const ZAdminAnnounceType nType)
{
	switch (nType)
	{
	case ZAAT_CHAT:
	{
		char szText[512];
		ZTransMsg(szText, MSG_ADMIN_ANNOUNCE, 1, szMsg);
		ZChatOutput(szText, ZChat::CMT_SYSTEM);
	}
	break;
	case ZAAT_MSGBOX:
	{
		if (ZApplication::GetGameInterface()->GetState() != GUNZ_GAME)
		{
			ZApplication::GetGameInterface()->ShowMessage(szMsg);
		}
		else
		{
			ZChatOutput(szMsg);
		}
	}
	break;
	}
}

void ZGameClient::OnGameLevelUp(const MUID& uidChar)
{
	if (g_pGame)
	{
		ZCharacter* pCharacter = g_pGame->m_CharacterManager.Find(uidChar);
		if (pCharacter) {
			pCharacter->LevelUp();

			char temp[256] = "";
			ZTransMsg(temp, MSG_GAME_LEVEL_UP, 1, pCharacter->GetUserAndClanName());
			ZChatOutput(MCOLOR(ZCOLOR_GAME_INFO), temp);
		}
	}
}

void ZGameClient::OnGameLevelDown(const MUID& uidChar)
{
	if (g_pGame)
	{
		ZCharacter* pCharacter = g_pGame->m_CharacterManager.Find(uidChar);
		if (pCharacter) {
			pCharacter->LevelDown();

			char temp[256] = "";
			ZTransMsg(temp, MSG_GAME_LEVEL_DOWN, 1, pCharacter->GetUserAndClanName());
			ZChatOutput(MCOLOR(ZCOLOR_GAME_INFO), temp);
		}
	}
}

void ZGameClient::OnResponseGameInfo(const MUID& uidStage,
	const void* pGameInfoBlob, const void* pRuleInfoBlob, const void* pPlayerInfoBlob)
{
	if (g_pGame == NULL) return;

	// Game Info
	int nGameInfoCount = MGetBlobArrayCount(pGameInfoBlob);
	if (nGameInfoCount > 0) {
		MTD_GameInfo* pGameInfo = (MTD_GameInfo*)MGetBlobArrayElement(pGameInfoBlob, 0);
		g_pGame->GetMatch()->SetTeamScore(MMT_RED, pGameInfo->nRedTeamScore);
		g_pGame->GetMatch()->SetTeamScore(MMT_BLUE, pGameInfo->nBlueTeamScore);
		g_pGame->GetMatch()->SetTeamKills(MMT_RED, pGameInfo->nRedTeamKills);
		g_pGame->GetMatch()->SetTeamKills(MMT_BLUE, pGameInfo->nBlueTeamKills);
	}

	// Player Info
	int nPlayerCount = MGetBlobArrayCount(pPlayerInfoBlob);

	for (int i = 0; i < nPlayerCount; i++)
	{
		MTD_GameInfoPlayerItem* pPlayerInfo = (MTD_GameInfoPlayerItem*)MGetBlobArrayElement(pPlayerInfoBlob, i);
		ZCharacter* pCharacter = g_pGame->m_CharacterManager.Find(pPlayerInfo->uidPlayer);
		if (pCharacter == NULL) continue;

		if (pPlayerInfo->bAlive == true)
		{
			pCharacter->Revival();
		}
		else
		{
			if ((g_pGame->GetMatch()->IsTeamPlay()) && (g_pGame->GetMatch()->GetRoundState() != MMATCH_ROUNDSTATE_FREE))
			{
				pCharacter->ForceDie();
				pCharacter->SetVisible(false);
			}
		}

		pCharacter->GetStatus()->nKills = pPlayerInfo->nKillCount;
		pCharacter->GetStatus()->nDeaths = pPlayerInfo->nDeathCount;
	}

	int nRuleCount = MGetBlobArrayCount(pRuleInfoBlob);
	if (nRuleCount > 0) {
		MTD_RuleInfo* pRuleInfoHeader = (MTD_RuleInfo*)MGetBlobArrayElement(pRuleInfoBlob, 0);

		g_pGame->GetMatch()->OnResponseRuleInfo(pRuleInfoHeader);
	}
}

void ZGameClient::OnObtainWorldItem(const MUID& uidChar, const int nItemUID)
{
	if (g_pGame == NULL) return;

	ZCharacter* pCharacter = g_pGame->m_CharacterManager.Find(uidChar);
	if (pCharacter)
	{
		ZGetWorldItemManager()->ApplyWorldItem(nItemUID, pCharacter);

		ZWeapon* pWeapon = g_pGame->m_WeaponManager.GetWorldItem(nItemUID);
		ZWeaponItemkit* pItemkit = MDynamicCast(ZWeaponItemkit, pWeapon);

		if (pItemkit) {
			pItemkit->m_bDeath = true;
		}

	}
}

void ZGameClient::OnSpawnWorldItem(void* pBlob)
{
	if (g_pGame == NULL) return;

	int nWorldItemCount = MGetBlobArrayCount(pBlob);

	ZWeaponItemkit* pItemkit = NULL;
	ZMovingWeapon* pMWeapon = NULL;
	ZWorldItem* pWorldItem = NULL;

	for (int i = 0; i < nWorldItemCount; i++)
	{
		MTD_WorldItem* pWorldItemNode = (MTD_WorldItem*)MGetBlobArrayElement(pBlob, i);

		pWorldItem = ZGetWorldItemManager()->AddWorldItem(
			pWorldItemNode->nUID,
			pWorldItemNode->nItemID,
			(MTD_WorldItemSubType)pWorldItemNode->nItemSubType,
			rvector((float)pWorldItemNode->x, (float)pWorldItemNode->y, (float)pWorldItemNode->z));

		pMWeapon = g_pGame->m_WeaponManager.UpdateWorldItem(pWorldItemNode->nItemID, rvector(pWorldItemNode->x, pWorldItemNode->y, pWorldItemNode->z));
		pItemkit = MDynamicCast(ZWeaponItemkit, pMWeapon);

		if (pWorldItem && pItemkit) {
			pItemkit->SetItemUID(pWorldItemNode->nUID);
			pWorldItem->m_bisDraw = false;
		}
	}
}

void ZGameClient::OnRemoveWorldItem(const int nItemUID)
{
	if (g_pGame == NULL) return;

	ZGetWorldItemManager()->DeleteWorldItem(nItemUID, true);

	ZWeapon* pWeapon = g_pGame->m_WeaponManager.GetWorldItem(nItemUID);
	ZWeaponItemkit* pItemkit = MDynamicCast(ZWeaponItemkit, pWeapon);

	if (pItemkit) {
		pItemkit->m_bDeath = true;
	}
}

void ZGameClient::OnUserWhisper(char* pszSenderName, char* pszTargetName, char* pszMessage)
{
	char szText[256];
	ZTransMsg(szText, MSG_GAME_WHISPER, 2, pszSenderName, pszMessage);
	ZChatOutput(MCOLOR(ZCOLOR_CHAT_WHISPER), szText, ZChat::CL_CURRENT);

	ZGetGameInterface()->GetChat()->SetWhisperLastSender(pszSenderName);

	if ((ZApplication::GetGameInterface()->GetState() == GUNZ_GAME) && (g_pGame))
	{
		if (ZApplication::GetGameInterface()->GetCombatInterface())
		{
			if (!ZGetConfiguration()->GetViewGameChat())
			{
				ZApplication::GetGameInterface()->GetCombatInterface()->ShowChatOutput(true);
			}
		}
	}

#ifdef FLASH_WINDOW_ON_WHISPER
	FlashWindow(g_hWnd, FALSE);
#endif
}

void ZGameClient::OnChatRoomJoin(char* pszPlayerName, char* pszChatRoomName)
{
	char szText[256];
	ZTransMsg(szText, MSG_LOBBY_WHO_CHAT_ROMM_JOIN, 2, pszChatRoomName, pszPlayerName);
	ZChatOutput(szText, ZChat::CMT_NORMAL, ZChat::CL_CURRENT);
}

void ZGameClient::OnChatRoomLeave(char* pszPlayerName, char* pszChatRoomName)
{
	char szText[256];
	ZTransMsg(szText, MSG_LOBBY_WHO_CHAT_ROOM_EXIT, 2, pszChatRoomName, pszPlayerName);
	ZChatOutput(szText, ZChat::CMT_NORMAL, ZChat::CL_CURRENT);
}

void ZGameClient::OnChatRoomSelectWrite(char* pszChatRoomName)
{
	char szText[256];
	ZTransMsg(szText, MSG_LOBBY_CHAT_ROOM_CHANGE, 1, pszChatRoomName);
	ZChatOutput(szText, ZChat::CMT_NORMAL, ZChat::CL_CURRENT);
}

void ZGameClient::OnChatRoomInvite(char* pszSenderName, char* pszRoomName)
{
	char szLog[256];
	ZTransMsg(szLog, MSG_LOBBY_WHO_INVITATION, 2, pszSenderName, pszRoomName);
	ZChatOutput(szLog, ZChat::CMT_NORMAL, ZChat::CL_CURRENT);

	SetChatRoomInvited(pszRoomName);
}

void ZGameClient::OnChatRoomChat(char* pszChatRoomName, char* pszPlayerName, char* pszChat)
{
	char szText[256];
	ZTransMsg(szText, MRESULT_CHAT_ROOM, 3, pszChatRoomName, pszPlayerName, pszChat);
	ZChatOutput(MCOLOR(ZCOLOR_CHAT_ROOMCHAT), szText, ZChat::CL_CURRENT);
}

void ZGameClient::RequestPrevStageList()
{
	int nStageCursor;
	ZRoomListBox* pRoomList =
		(ZRoomListBox*)ZApplication::GetGameInterface()->GetIDLResource()->FindWidget("Lobby_StageList");
	if (!pRoomList) return;

	nStageCursor = pRoomList->GetFirstStageCursor() - NUM_DISPLAY_ROOM;
	if (nStageCursor < 0) nStageCursor = 0;

	ZPostRequestStageList(m_uidPlayer, m_uidChannel, nStageCursor);

	int nPage = (nStageCursor / TRANS_STAGELIST_NODE_COUNT) + 1;
	ZApplication::GetGameInterface()->SetRoomNoLight(nPage);
}

void ZGameClient::RequestNextStageList()
{
	int nStageCursor;
	ZRoomListBox* pRoomList =
		(ZRoomListBox*)ZApplication::GetGameInterface()->GetIDLResource()->FindWidget("Lobby_StageList");
	if (!pRoomList) return;

	nStageCursor = pRoomList->GetLastStageCursor() + 1;
	if (nStageCursor > 100) nStageCursor = 100;

	ZPostRequestStageList(m_uidPlayer, m_uidChannel, nStageCursor);

	int nPage = (nStageCursor / TRANS_STAGELIST_NODE_COUNT) + 1;
	ZApplication::GetGameInterface()->SetRoomNoLight(nPage);
}

void ZGameClient::RequestStageList(int nPage)
{
	int nStageCursor;

	nStageCursor = (nPage - 1) * TRANS_STAGELIST_NODE_COUNT;
	if (nStageCursor < 0) nStageCursor = 0;
	else if (nStageCursor > 100) nStageCursor = 100;

	ZPostRequestStageList(m_uidPlayer, m_uidChannel, nStageCursor);
}

void ZGameClient::OnLocalReport119()
{
	ZApplication::GetGameInterface()->Show112Dialog(true);
}

int ZGameClient::ValidateRequestDeleteChar()
{
	ZIDLResource* pResource = ZApplication::GetGameInterface()->GetIDLResource();

	int nCharIndex = ZCharacterSelectView::GetSelectedCharacter();
	if ((nCharIndex < 0) || (nCharIndex >= MAX_CHAR_COUNT)) return ZERR_UNKNOWN;

	ZSelectCharacterInfo* pSelectCharInfo = &ZCharacterSelectView::m_CharInfo[nCharIndex];
	MTD_AccountCharInfo* pAccountCharInfo = &pSelectCharInfo->m_AccountCharInfo;
	MTD_CharInfo* pCharInfo = &pSelectCharInfo->m_CharInfo;

	if (!pSelectCharInfo->m_bLoaded) return ZERR_UNKNOWN;

	if (pCharInfo->szClanName[0] != 0)
		return MSG_CLAN_PLEASE_LEAVE_FROM_CHAR_DELETE;

	for (int i = 0; i < MMCIP_END; i++)
	{
		MMatchItemDesc* pItemDesc = MGetMatchItemDescMgr()->GetItemDesc(pCharInfo->nEquipedItemDesc[i]);
		if (pItemDesc)
		{
			if (pItemDesc->IsCashItem()) return MSG_CANNOT_DELETE_CHAR_FOR_CASHITEM;
		}
	}

	return ZOK;
}

void ZGameClient::RequestChannelJoin(const MUID& uidChannel)
{
	ZPostChannelRequestJoin(GetPlayerUID(), uidChannel);
}

void ZGameClient::RequestChannelJoin(const MCHANNEL_TYPE nChannelType, char* szChannelName)
{
	ZPostChannelRequestJoinFromChannelName(GetPlayerUID(), (int)nChannelType, szChannelName);
}

void ZGameClient::RequestGameSuicide()
{
	ZGame* pGame = ZGetGameInterface()->GetGame();
	if (!pGame) return;

	ZMyCharacter* pMyCharacter = pGame->m_pMyCharacter;
	if (!pMyCharacter) return;

	if ((!pMyCharacter->IsDead()) && (pGame->GetMatch()->GetRoundState() == MMATCH_ROUNDSTATE_PLAY))
	{
		pMyCharacter->SetLastDamageType(ZD_NONE);

		ZPostRequestSuicide(ZGetGameClient()->GetPlayerUID());
	}
}

void ZGameClient::OnResponseResult(const int nResult)
{
	if (nResult == MOK)
		return;

	if (ZApplication::GetGameInterface()->GetState() == GUNZ_GAME)
	{
		ZChatOutput(MCOLOR(ZCOLOR_CHAT_SYSTEM), ZErrStr(nResult));
	}
	else
	{
		ZApplication::GetGameInterface()->ShowErrorMessage(nResult);
	}
}

static void blog(const char *pFormat, ...)
{
	char szBuf[256];

	va_list args;
	va_start(args, pFormat);
	vsprintf_safe(szBuf, pFormat, args);
	va_end(args);

	strcat_safe(szBuf, "\n");

	if (ZApplication::GetGameInterface()->GetState() == GUNZ_LOBBY)
		ZChatOutput(szBuf, ZChat::CMT_SYSTEM, ZChat::CL_LOBBY);
	else if (ZApplication::GetGameInterface()->GetState() == GUNZ_STAGE)
		ZChatOutput(szBuf, ZChat::CMT_SYSTEM, ZChat::CL_STAGE);
}

void ZGameClient::OnResponseCharInfoDetail(const void* pBlob)
{
#ifndef _DEBUG
	return;
#endif

	MWidget* pWidget = ZApplication::GetGameInterface()->GetIDLResource()->FindWidget("Characterinfo");
	if (pWidget)
		pWidget->Show();

	int nCount = MGetBlobArrayCount(pBlob);
	if (nCount != 1) return;

	MTD_CharInfo_Detail* pCharInfoDetail = (MTD_CharInfo_Detail*)MGetBlobArrayElement(pBlob, 0);

	blog("^9%s", ZMsg(MSG_CHARINFO_TITLE));
	blog("^9%s : ^1%s^9(%s)", ZMsg(MSG_CHARINFO_NAME),
		pCharInfoDetail->szName,
		ZGetSexStr(MMatchSex(pCharInfoDetail->nSex), true));
	char sztemp[256];
	if (strcmp(pCharInfoDetail->szClanName, "") == 0)
		strcpy_safe(sztemp, "---");
	else
		sprintf_safe(sztemp, "%s(%s)", pCharInfoDetail->szClanName, ZGetClanGradeStr(pCharInfoDetail->nClanGrade));
	blog("^9%s : %s", ZMsg(MSG_CHARINFO_CLAN), sztemp);
	blog("^9%s : %d %s", ZMsg(MSG_CHARINFO_LEVEL), pCharInfoDetail->nLevel, ZMsg(MSG_CHARINFO_LEVELMARKER));
	int nWinPercent = (int)((float)pCharInfoDetail->nKillCount / (float)(pCharInfoDetail->nKillCount + pCharInfoDetail->nDeathCount) * 100.0f);
	blog("^9%s : %d%s/%d%s(%d%%)", ZMsg(MSG_CHARINFO_WINPERCENT),
		pCharInfoDetail->nKillCount,
		ZMsg(MSG_CHARINFO_WIN),
		pCharInfoDetail->nDeathCount,
		ZMsg(MSG_CHARINFO_LOSE),
		nWinPercent);
	ZGetTimeStrFromSec(sztemp, pCharInfoDetail->nConnPlayTimeSec);
	blog("^9%s : %s", ZMsg(MSG_CHARINFO_CONNTIME), sztemp);
	blog("");
}

void ZGameClient::OnNotifyCallVote(const char* pszDiscuss, const char* pszArg)
{
	SetVoteInProgress(true);
	SetCanVote(true);

	char szText[256] = "";
	if (_stricmp(pszDiscuss, "joke") == 0) {
		ZTransMsg(szText, MSG_VOTE_START, 1, pszArg);
		ZChatOutput(szText, ZChat::CMT_SYSTEM, ZChat::CL_CURRENT);
	}
	else if (_stricmp(pszDiscuss, "kick") == 0) {
		sprintf_safe(m_szVoteText, ZMsg(MSG_VOTE_KICK), pszArg);
		ZChatOutput(szText, ZChat::CMT_SYSTEM, ZChat::CL_CURRENT);
	}
}

void ZGameClient::OnNotifyVoteResult(const char* pszDiscuss, int nResult)
{
	if (ZGetGameInterface()->GetCombatInterface() == NULL)
		return;

	ZGetGameInterface()->GetCombatInterface()->GetVoteInterface()->ShowTargetList(false);

	SetVoteInProgress(false);
	SetCanVote(false);

	if (nResult == 0) {
		ZChatOutput(ZMsg(MSG_VOTE_REJECTED), ZChat::CMT_SYSTEM, ZChat::CL_CURRENT);
	}
	else if (nResult == 1) {
		ZChatOutput(ZMsg(MSG_VOTE_PASSED), ZChat::CMT_SYSTEM, ZChat::CL_CURRENT);
	}
}

void ZGameClient::OnVoteAbort(const int nMsgCode)
{
	ZChatOutput(ZMsg(nMsgCode), ZChat::CMT_SYSTEM, ZChat::CL_CURRENT);
}

void ZGameClient::RequestOnLobbyCreated()
{
	ZPostRequestStageList(GetPlayerUID(), GetChannelUID(), 0);
	ZPostRequestChannelPlayerList(GetPlayerUID(), GetChannelUID(), 0);
}

void ZGameClient::RequestOnGameDestroyed()
{
	ZPostRequestMySimpleCharInfo(ZGetGameClient()->GetPlayerUID());

	if ( (GetServerMode() ==  MSM_CLAN || GetServerMode() == MSM_TEST ) && (GetChannelType() == MCHANNEL_TYPE_CLAN) )
	{
		ZPostRequestClanInfo(GetPlayerUID(), m_szChannel);
	}
}

void ZGameClient::OnFollowResponse(const int nMsgID)
{
	ZGetGameInterface()->GetChat()->Clear(ZChat::CL_LOBBY);
	const char* pszMsg = ZErrStr(nMsgID);
	if (!pszMsg)
		return;

	ZChatOutput(pszMsg, ZChat::CMT_SYSTEM, ZChat::CL_LOBBY);
}

void ZGameClient::ProcessEmblem(unsigned int nCLID, unsigned int nChecksum)
{
	if (!m_EmblemMgr.CheckEmblem(nCLID, nChecksum) && nChecksum != 0)
		ZPostRequestEmblemURL(nCLID);
}

void ZGameClient::RequestEmblemURL(unsigned int nCLID)
{
	ZPostRequestEmblemURL(nCLID);
}

void ZGameClient::OnClanResponseEmblemURL(unsigned int nCLID, unsigned int nEmblemChecksum, const char* szEmblemURL)
{
	char szFullURL[2048] = "";
	sprintf_safe(szFullURL, "%s%s", Z_LOCALE_EMBLEM_URL, szEmblemURL);

	m_EmblemMgr.ProcessEmblem(nCLID, szFullURL, nEmblemChecksum);
}

void ZGameClient::OnClanEmblemReady(unsigned int nCLID, const char* szURL)
{
	mlog("EMBLEM READY!! (%d)%s\n", nCLID, szURL);
	ZGetEmblemInterface()->ReloadClanInfo(nCLID);

	if (ZGetNetRepository()->GetClanInfo()->nCLID == nCLID) {
		ZIDLResource* pRes = ZApplication::GetGameInterface()->GetIDLResource();
		MPicture* pPicture = (MPicture*)pRes->FindWidget("Lobby_ClanInfoEmblem");
		if (pPicture)
			pPicture->SetBitmap(ZGetEmblemInterface()->GetClanEmblem(nCLID));
	}
}


void ZGameClient::OnExpiredRentItem(void* pBlob)
{
	int nBlobSize = MGetBlobArrayCount(pBlob);

	char szText[1024];
	sprintf_safe(szText, "%s", ZMsg(MSG_EXPIRED));

	for (int i = 0; i < nBlobSize; i++)
	{
		u32* pExpiredItemID = (u32*)MGetBlobArrayElement(pBlob, i);

		char szItemText[256];

		MMatchItemDesc* pItemDesc = MGetMatchItemDescMgr()->GetItemDesc(*pExpiredItemID);
		if (pItemDesc)
		{
			sprintf_safe(szItemText, "[%d] %s\n", i + 1, pItemDesc->m_szName);
			if ((strlen(szText) + strlen(szItemText)) <= 1022) strcat_safe(szText, szItemText);
		}
	}

	ZApplication::GetGameInterface()->ShowMessage(szText);
}


bool ZGameClient::CreateUPnP(unsigned short nUDPPort)
{
	if (!m_pUPnP)
		m_pUPnP = new UPnP;

	//////////////////////////////////////////////////////////////////////////
	/// UPnP Enable
	// Activate Port
	if (m_pUPnP->Create(nUDPPort))
	{
#ifdef MFC
		TRACE("UPnP: Port: %d\n", nUDPPort);
#endif
		mlog("%d upnp port forward initialized.\n", nUDPPort);
		return true;
	}
	else
	{
		// Failed: Use Default Port
#ifdef MFC
		TRACE("UPnP: Failed to forward port\n");
#endif
	}
	return false;
}

bool ZGameClient::DestroyUPnP()
{
	if (m_pUPnP)
	{
		m_pUPnP->Destroy();
		delete m_pUPnP;
	}

	return true;
}

void ZGameClient::OnStopUDPTest(const MUID & uid)
{
	auto* Char = ZGetGame()->m_CharacterManager.Find(uid);
	if (!Char)
		return;

	ZChatOutputF("Failed to establish direct connection to %s.", Char->GetUserName());
}

void ZGameClient::OnUDPTestReply(const MUID& uid)
{
	MMatchClient::OnUDPTestReply(uid);

	auto* Char = ZGetGame()->m_CharacterManager.Find(uid);
	if (!Char)
		return;

#ifdef _DEBUG
	ZChatOutputF("Established direct connection to %s.", Char->GetUserName());
#endif
}

void ZGameClient::OnBroadcastDuelRenewVictories(const char* pszChampionName, const char* pszChannelName, int nRoomno, int nVictories)
{
	char szText[256];
	char szVic[32], szRoomno[32];

	sprintf_safe(szVic, "%d", nVictories);
	sprintf_safe(szRoomno, "%d", nRoomno);

	ZTransMsg(szText, MSG_DUEL_BROADCAST_RENEW_VICTORIES, 4, pszChampionName, pszChannelName, szRoomno, szVic);

	ZChatOutput(szText, ZChat::CMT_BROADCAST);
}

void ZGameClient::OnBroadcastDuelInterruptVictories(const char* pszChampionName, const char* pszInterrupterName, int nVictories)
{
	char szText[256];
	char szVic[32];
	sprintf_safe(szVic, "%d", nVictories);
	ZTransMsg(szText,
		MSG_DUEL_BROADCAST_INTERRUPT_VICTORIES, 3,
		pszChampionName, pszInterrupterName, szVic);

	ZChatOutput(szText, ZChat::CMT_BROADCAST);
}

void ZGameClient::OnResponseUpdateStageEquipLook(const MUID& uidPlayer, int nParts, int nItemID)
{
	MMatchObjCacheMap::iterator itFind = m_ObjCacheMap.find(uidPlayer);
	if (m_ObjCacheMap.end() == itFind)
	{
		return;
	}

	MMatchObjCache* pObjCache = itFind->second;

	pObjCache->GetCostume()->nEquipedItemID[nParts] = nItemID;

#ifdef UPDATE_STAGE_CHARVIEWER
	auto StageCharViewer = ZFindWidgetAs<ZCharacterView>("Stage_Charviewer");
	if (StageCharViewer && StageCharViewer->m_Info.UID == uidPlayer)
	{
		StageCharViewer->SetParts(MMatchCharItemParts(nParts), nItemID);
	}
#endif

#ifdef _DEBUG
	mlog("update stage look : parts(%d), itemid(%d)\n"
		, nParts
		, nItemID);
#endif
}
