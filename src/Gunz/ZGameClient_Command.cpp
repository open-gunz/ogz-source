#include "stdafx.h"
#include "ZGameClient.h"
#include "MBlobArray.h"
#include "ZCommandTable.h"
#include "ZConsole.h"
#include "ZConfiguration.h"

bool ZGameClient::OnCommand(MCommand* pCommand)
{
	bool ret = MMatchClient::OnCommand(pCommand);

	switch (pCommand->GetID()) {
	case MC_MATCH_PING_LIST:
	{
		auto Param = pCommand->GetParameter(0);
		if (Param->GetType() != MPT_BLOB) break;
		void* Blob = Param->GetPointer();
		int Count = MGetBlobArrayCount(Blob);
		for (int i = 0; i < Count; i++)
		{
			auto& Ping = *static_cast<MTD_PingInfo*>(MGetBlobArrayElement(Blob, i));
			if (Ping.UID == GetPlayerUID())
			{
				PingToServer = Ping.Ping;
				continue;
			}

			auto Peer = GetPeers()->Find(Ping.UID);
			if (Peer)
				Peer->UpdatePing(GetGlobalTimeMS(), Ping.Ping);

			auto Char = ZGetCharacterManager()->Find(Ping.UID);
			if (Char)
				Char->Ping = Ping.Ping;
		}
	}
	break;
	case MC_MATCH_RESPONSE_LOGIN_FAILED:
	{
		char szReason[4096];
		if (!pCommand->GetParameter(szReason, 0, MPT_STR, sizeof(szReason)))
			break;

		ZGetGameInterface()->ShowErrorMessage(szReason);

		ZPostDisconnect();
	}
	break;
	case MC_MATCH_RESPONSE_CREATE_ACCOUNT:
	{
		char szMessage[128];
		if (!pCommand->GetParameter(szMessage, 0, MPT_STR, sizeof(szMessage)))
			break;

		ZGetGameInterface()->ShowErrorMessage(szMessage);
	}
	break;
	case MC_NET_ONDISCONNECT: break;
	case MC_NET_ONERROR: break;
	case ZC_CHANGESKIN:
	{
		char szSkinName[256];
		pCommand->GetParameter(szSkinName, 0, MPT_STR, sizeof(szSkinName));
		if (ZApplication::GetGameInterface()->ChangeInterfaceSkin(szSkinName))
		{
			MClient::OutputMessage(MZMOM_LOCALREPLY, "Change Skin To %s", szSkinName);
		}
		else
		{
			MClient::OutputMessage(MZMOM_LOCALREPLY, "Change Skin Failed");
		}
	}
	break;
	case MC_ADMIN_TERMINAL:
	{
#ifndef _PUBLISH
		char szText[65535]; szText[0] = 0;
		MUID uidChar;

		pCommand->GetParameter(&uidChar, 0, MPT_UID);
		pCommand->GetParameter(szText, 1, MPT_STR, sizeof(szText));
		OutputToConsole(szText);
#endif
	}
	break;
	case MC_NET_CHECKPING:
	{
		MUID uid;
		if (pCommand->GetParameter(&uid, 0, MPT_UID) == false) break;
		MCommand* pNew = new MCommand(m_CommandManager.GetCommandDescByID(MC_NET_PING), uid, m_This);
		pNew->AddParameter(new MCommandParameterUInt(GetGlobalTimeMS()));
		Post(pNew);
		return true;
	}
	case MC_NET_PING:
	{
		unsigned int nTimeStamp;
		if (pCommand->GetParameter(&nTimeStamp, 0, MPT_UINT) == false) break;
		MCommand* pNew = new MCommand(m_CommandManager.GetCommandDescByID(MC_NET_PONG), pCommand->m_Sender, m_This);
		pNew->AddParameter(new MCommandParameterUInt(nTimeStamp));
		Post(pNew);
		return true;
	}
	case MC_NET_PONG:
	{
		int nTimeStamp;
		pCommand->GetParameter(&nTimeStamp, 0, MPT_UINT);

		MClient::OutputMessage(MZMOM_LOCALREPLY, "Ping from (%u:%u) = %d",
			pCommand->GetSenderUID().High, pCommand->GetSenderUID().Low,
			GetGlobalTimeMS() - nTimeStamp);
	}
	break;
	case ZC_CON_CONNECT:
	{
		char szBuf[256];
		sprintf_safe(szBuf, "Net.Connect %s:%d", ZGetConfiguration()->GetServerIP(),
			ZGetConfiguration()->GetServerPort());
		ConsoleInputEvent(szBuf);
		MLog("%s %d\n", ZGetConfiguration()->GetServerIP(), ZGetConfiguration()->GetServerPort());
		SetServerAddr(ZGetConfiguration()->GetServerIP(), ZGetConfiguration()->GetServerPort());
	}
	break;
	case ZC_CON_DISCONNECT:
	{
		ConsoleInputEvent("Net.Disconnect");
	}
	break;
	case ZC_CON_CLEAR:
	{
		if (ZGetConsole()) ZGetConsole()->ClearMessage();
	}
	break;
	case ZC_CON_HIDE:
	{
		if (ZGetConsole()) ZGetConsole()->Show(false);
	}
	break;
	case ZC_CON_SIZE:
	{
		if (ZGetConsole())
		{
			int iConWidth, iConHeight;
			pCommand->GetParameter(&iConWidth, 0, MPT_INT);
			pCommand->GetParameter(&iConHeight, 1, MPT_INT);
			if ((iConWidth > 30) && (iConHeight > 30))
			{
				MPOINT point = ZGetConsole()->GetPosition();
				ZGetConsole()->SetBounds(point.x, point.y, iConWidth, iConHeight);
			}
		}
	}
	break;
	case MC_CLOCK_SYNCHRONIZE:
	{
		u32 nGlobalClock;
		pCommand->GetParameter(&nGlobalClock, 0, MPT_UINT);


		u32 nLocalClock = GetClockCount();

		if (nGlobalClock > nLocalClock) m_bIsBigGlobalClock = true;
		else m_bIsBigGlobalClock = false;
		m_nClockDistance = ZGetClockDistance(nGlobalClock, nLocalClock);
	}
	break;
	case MC_ADMIN_TELEPORT:
	{
		char szAdminName[128] = "";
		char szTargetName[128] = "";


		if (pCommand->GetSenderUID() != GetServerUID()) //Mark idea
			break;

		pCommand->GetParameter(szAdminName, 0, MPT_STR, sizeof(szAdminName));
		pCommand->GetParameter(szTargetName, 1, MPT_STR, sizeof(szTargetName));


		ZCharacterManager *pZCharacterManager = ZGetCharacterManager();

		if (pZCharacterManager != NULL)
		{
			for (ZCharacterManager::iterator itor = pZCharacterManager->begin(); itor != pZCharacterManager->end(); ++itor)
			{
				ZCharacter* pCharacter = (*itor).second;


				if (strcmp(pCharacter->GetProperty()->GetName(), szAdminName) == 0)
					ZGetGame()->m_pMyCharacter->SetPosition(pCharacter->GetPosition());
			}
		}
	}
	break;
	case MC_MATCH_NOTIFY:
	{
		unsigned int nMsgID = 0;
		if (pCommand->GetParameter(&nMsgID, 0, MPT_UINT) == false) break;

		OnMatchNotify(nMsgID);
	}
	break;
	case MC_MATCH_BRIDGEPEER_ACK:
	{
		MUID uidChar;
		int nCode;
		pCommand->GetParameter(&uidChar, 0, MPT_UID);
		pCommand->GetParameter(&nCode, 1, MPT_INT);
		OnBridgePeerACK(uidChar, nCode);
	}
	break;
	case MC_MATCH_STAGE_RESPONSE_FORCED_ENTRY:
	{
		int nResult;
		pCommand->GetParameter(&nResult, 0, MPT_INT);
		if (nResult == MOK)
		{
			OnForcedEntryToGame();
		}
		else
		{
			ZApplication::GetGameInterface()->ShowMessage("Forced entry rejected");
		}
	}
	break;
	case MC_MATCH_STAGE_JOIN:
	{
		MUID uidChar, uidStage;
		unsigned int nRoomNo = 0;
		char szStageName[256] = "";

		pCommand->GetParameter(&uidChar, 0, MPT_UID);
		pCommand->GetParameter(&uidStage, 1, MPT_UID);
		pCommand->GetParameter(&nRoomNo, 2, MPT_UINT);
		pCommand->GetParameter(szStageName, 3, MPT_STR, sizeof(szStageName));

		OnStageJoin(uidChar, uidStage, nRoomNo, szStageName);
	}
	break;
	case MC_MATCH_STAGE_LEAVE:
	{
		MUID uidChar, uidStage;

		pCommand->GetParameter(&uidChar, 0, MPT_UID);
		pCommand->GetParameter(&uidStage, 1, MPT_UID);

		OnStageLeave(uidChar, uidStage);
	}
	break;
	case MC_MATCH_STAGE_START:
	{
		MUID uidChar, uidStage;
		int nCountdown;

		pCommand->GetParameter(&uidChar, 0, MPT_UID);
		pCommand->GetParameter(&uidStage, 1, MPT_UID);
		pCommand->GetParameter(&nCountdown, 2, MPT_INT);

		OnStageStart(uidChar, uidStage, nCountdown);
	}
	break;
	case MC_MATCH_STAGE_LAUNCH:
	{
		MUID uidStage;
		char szMapName[_MAX_DIR];

		pCommand->GetParameter(&uidStage, 0, MPT_UID);
		pCommand->GetParameter(szMapName, 1, MPT_STR, sizeof(szMapName));
		OnStageLaunch(uidStage, szMapName);
	}
	break;
	case MC_MATCH_STAGE_FINISH_GAME:
	{
		MUID uidStage;
		pCommand->GetParameter(&uidStage, 0, MPT_UID);
		OnStageFinishGame(uidStage);
	}
	break;
	case MC_MATCH_STAGE_MAP:
	{
		MUID uidStage;
		char szMapName[_MAX_DIR];
		pCommand->GetParameter(&uidStage, 0, MPT_UID);
		pCommand->GetParameter(szMapName, 1, MPT_STR, sizeof(szMapName));

		OnStageMap(uidStage, szMapName);
	}
	break;
	case MC_MATCH_STAGE_TEAM:
	{
		MUID uidChar, uidStage;
		unsigned int nTeam;
		pCommand->GetParameter(&uidChar, 0, MPT_UID);
		pCommand->GetParameter(&uidStage, 1, MPT_UID);
		pCommand->GetParameter(&nTeam, 2, MPT_UINT);
		OnStageTeam(uidChar, uidStage, nTeam);
	}
	break;
	case MC_MATCH_STAGE_PLAYER_STATE:
	{
		MUID uidChar, uidStage;
		int nObjectStageState;
		pCommand->GetParameter(&uidChar, 0, MPT_UID);
		pCommand->GetParameter(&uidStage, 1, MPT_UID);
		pCommand->GetParameter(&nObjectStageState, 2, MPT_INT);
		OnStagePlayerState(uidChar, uidStage, MMatchObjectStageState(nObjectStageState));
	}
	break;
	case MC_MATCH_STAGE_MASTER:
	{
		MUID uidChar, uidStage;
		pCommand->GetParameter(&uidStage, 0, MPT_UID);
		pCommand->GetParameter(&uidChar, 1, MPT_UID);

		OnStageMaster(uidStage, uidChar);
	}
	break;
	case MC_MATCH_STAGE_CHAT:
	{
		MUID uidStage, uidChar;
		static char szChat[512];
		pCommand->GetParameter(&uidChar, 0, MPT_UID);
		pCommand->GetParameter(&uidStage, 1, MPT_UID);
		pCommand->GetParameter(szChat, 2, MPT_STR, sizeof(szChat));
		OnStageChat(uidChar, uidStage, szChat);
	}
	break;
	case MC_MATCH_STAGE_LIST:
	{
		char nPrevStageCount, nNextStageCount;
		pCommand->GetParameter(&nPrevStageCount, 0, MPT_CHAR);
		pCommand->GetParameter(&nNextStageCount, 1, MPT_CHAR);

		MCommandParameter* pParam = pCommand->GetParameter(2);
		if (pParam->GetType() != MPT_BLOB) break;
		void* pBlob = pParam->GetPointer();
		int nCount = MGetBlobArrayCount(pBlob);

		OnStageList((int)nPrevStageCount, (int)nNextStageCount, pBlob, nCount);
	}
	break;
	case MC_MATCH_CHANNEL_RESPONSE_PLAYER_LIST:
	{
		unsigned char nTotalPlayerCount, nPage;

		pCommand->GetParameter(&nTotalPlayerCount, 0, MPT_UCHAR);
		pCommand->GetParameter(&nPage, 1, MPT_UCHAR);

		MCommandParameter* pParam = pCommand->GetParameter(2);
		if (pParam->GetType() != MPT_BLOB) break;
		void* pBlob = pParam->GetPointer();
		int nCount = MGetBlobArrayCount(pBlob);

		OnChannelPlayerList((int)nTotalPlayerCount, (int)nPage, pBlob, nCount);

	}
	break;
	case MC_MATCH_CHANNEL_RESPONSE_ALL_PLAYER_LIST:
	{
		MUID uidChannel;

		pCommand->GetParameter(&uidChannel, 0, MPT_UID);

		MCommandParameter* pParam = pCommand->GetParameter(1);
		if (pParam->GetType() != MPT_BLOB) break;
		void* pBlob = pParam->GetPointer();
		int nCount = MGetBlobArrayCount(pBlob);

		OnChannelAllPlayerList(uidChannel, pBlob, nCount);
	}
	break;
	case MC_MATCH_RESPONSE_FRIENDLIST:
	{
		MCommandParameter* pParam = pCommand->GetParameter(0);
		if (pParam->GetType() != MPT_BLOB) break;
		void* pBlob = pParam->GetPointer();
		int nCount = MGetBlobArrayCount(pBlob);

		OnResponseFriendList(pBlob, nCount);
	}
	break;
	case MC_MATCH_RESPONSE_STAGESETTING:
	{
		MUID uidStage;
		pCommand->GetParameter(&uidStage, 0, MPT_UID);

		MCommandParameter* pStageParam = pCommand->GetParameter(1);
		if (pStageParam->GetType() != MPT_BLOB) break;
		void* pStageBlob = pStageParam->GetPointer();
		int nStageCount = MGetBlobArrayCount(pStageBlob);

		MCommandParameter* pCharParam = pCommand->GetParameter(2);
		if (pCharParam->GetType() != MPT_BLOB) break;
		void* pCharBlob = pCharParam->GetPointer();
		int nCharCount = MGetBlobArrayCount(pCharBlob);

		int nStageState;
		pCommand->GetParameter(&nStageState, 3, MPT_INT);

		MUID uidMaster;
		pCommand->GetParameter(&uidMaster, 4, MPT_UID);

		OnResponseStageSetting(uidStage, pStageBlob, nStageCount, pCharBlob,
			nCharCount, static_cast<STAGE_STATE>(nStageState), uidMaster);
	}
	break;
	case MC_MATCH_RESPONSE_PEER_RELAY:
	{
		MUID uidPeer;
		if (pCommand->GetParameter(&uidPeer, 0, MPT_UID) == false) break;

		OnResponsePeerRelay(uidPeer);
	}
	break;
	case MC_MATCH_LOADING_COMPLETE:
	{
		MUID uidChar;
		int nPercent;

		if (pCommand->GetParameter(&uidChar, 0, MPT_UID) == false) break;
		if (pCommand->GetParameter(&nPercent, 1, MPT_INT) == false) break;

		OnLoadingComplete(uidChar, nPercent);
	}
	break;
	case MC_MATCH_ANNOUNCE:
	{
		unsigned int nType;
		char szMsg[256];
		pCommand->GetParameter(&nType, 0, MPT_UINT);
		pCommand->GetParameter(szMsg, 1, MPT_STR, sizeof(szMsg));
		OnAnnounce(nType, szMsg);
	}
	break;
	case MC_MATCH_CHANNEL_RESPONSE_JOIN:
	{
		MUID uidChannel;
		int nChannelType;
		char szChannelName[256];

		pCommand->GetParameter(&uidChannel, 0, MPT_UID);
		pCommand->GetParameter(&nChannelType, 1, MPT_INT);
		pCommand->GetParameter(szChannelName, 2, MPT_STR, sizeof(szChannelName));

		OnChannelResponseJoin(uidChannel, (MCHANNEL_TYPE)nChannelType, szChannelName);
	}
	break;
	case MC_MATCH_CHANNEL_CHAT:
	{
		MUID uidChannel, uidChar;
		char szChat[512];
		char szName[256];
		int nGrade;

		pCommand->GetParameter(&uidChannel, 0, MPT_UID);
		pCommand->GetParameter(szName, 1, MPT_STR, sizeof(szName));
		pCommand->GetParameter(szChat, 2, MPT_STR, sizeof(szChat));
		pCommand->GetParameter(&nGrade, 3, MPT_INT);

		OnChannelChat(uidChannel, szName, szChat, nGrade);
	}
	break;
	case MC_MATCH_CHANNEL_LIST:
	{
		MCommandParameter* pParam = pCommand->GetParameter(0);
		if (pParam->GetType() != MPT_BLOB) break;
		void* pBlob = pParam->GetPointer();
		int nCount = MGetBlobArrayCount(pBlob);
		OnChannelList(pBlob, nCount);
	}
	break;
	case MC_MATCH_CHANNEL_RESPONSE_RULE:
	{
		MUID uidChannel;
		pCommand->GetParameter(&uidChannel, 0, MPT_UID);
		char szRuleName[128];
		pCommand->GetParameter(szRuleName, 1, MPT_STR, sizeof(szRuleName));

		OnChannelResponseRule(uidChannel, szRuleName);
	}
	break;
	case MC_MATCH_RESPONSE_RECOMMANDED_CHANNEL:
	{
		MUID uidChannel;
		pCommand->GetParameter(&uidChannel, 0, MPT_UID);

		OnResponseRecommandedChannel(uidChannel);
	}
	break;
	case MC_ADMIN_ANNOUNCE:
	{
		char szChat[512];
		u32 nMsgType = 0;

		pCommand->GetParameter(szChat, 1, MPT_STR, sizeof(szChat));
		pCommand->GetParameter(&nMsgType, 2, MPT_UINT);

		OnAdminAnnounce(szChat, ZAdminAnnounceType(nMsgType));
	}
	break;
	case MC_MATCH_GAME_LEVEL_UP:
	{
		MUID uidChar;
		pCommand->GetParameter(&uidChar, 0, MPT_UID);

		OnGameLevelUp(uidChar);
	}
	break;
	case MC_MATCH_GAME_LEVEL_DOWN:
	{
		MUID uidChar;
		pCommand->GetParameter(&uidChar, 0, MPT_UID);

		OnGameLevelDown(uidChar);
	}
	break;
	case MC_MATCH_RESPONSE_GAME_INFO:
	{
		MUID uidStage;
		pCommand->GetParameter(&uidStage, 0, MPT_UID);

		MCommandParameter* pParam = pCommand->GetParameter(1);
		if (pParam->GetType() != MPT_BLOB) break;
		void* pGameInfoBlob = pParam->GetPointer();

		pParam = pCommand->GetParameter(2);
		if (pParam->GetType() != MPT_BLOB) break;
		void* pRuleInfoBlob = pParam->GetPointer();

		pParam = pCommand->GetParameter(3);
		if (pParam->GetType() != MPT_BLOB) break;
		void* pPlayerInfoBlob = pParam->GetPointer();

		OnResponseGameInfo(uidStage, pGameInfoBlob, pRuleInfoBlob, pPlayerInfoBlob);
	}
	break;
	case MC_MATCH_OBTAIN_WORLDITEM:
	{
		MUID uidPlayer;
		int nItemUID;

		pCommand->GetParameter(&uidPlayer, 0, MPT_UID);
		pCommand->GetParameter(&nItemUID, 1, MPT_INT);

		OnObtainWorldItem(uidPlayer, nItemUID);
	}
	break;
	case MC_MATCH_SPAWN_WORLDITEM:
	{
		MCommandParameter* pParam = pCommand->GetParameter(0);
		if (pParam->GetType() != MPT_BLOB) break;

		void* pSpawnInfoBlob = pParam->GetPointer();

		OnSpawnWorldItem(pSpawnInfoBlob);
	}
	break;
	case MC_MATCH_REMOVE_WORLDITEM:
	{
		int nItemUID;

		pCommand->GetParameter(&nItemUID, 0, MPT_INT);

		OnRemoveWorldItem(nItemUID);
	}
	break;

	case MC_MATCH_USER_WHISPER:
	{
		char szSenderName[128] = "";
		char szTargetName[128] = "";
		char szMessage[1024] = "";

		pCommand->GetParameter(szSenderName, 0, MPT_STR, sizeof(szSenderName));
		pCommand->GetParameter(szTargetName, 1, MPT_STR, sizeof(szTargetName));
		pCommand->GetParameter(szMessage, 2, MPT_STR, sizeof(szMessage));

		OnUserWhisper(szSenderName, szTargetName, szMessage);
	}
	break;
	case MC_MATCH_CHATROOM_JOIN:
	{
		char szPlayerName[128] = "";
		char szChatRoomName[128] = "";

		pCommand->GetParameter(szPlayerName, 0, MPT_STR, sizeof(szPlayerName));
		pCommand->GetParameter(szChatRoomName, 1, MPT_STR, sizeof(szChatRoomName));

		OnChatRoomJoin(szPlayerName, szChatRoomName);
	}
	break;
	case MC_MATCH_CHATROOM_LEAVE:
	{
		char szPlayerName[128] = "";
		char szChatRoomName[128] = "";

		pCommand->GetParameter(szPlayerName, 0, MPT_STR, sizeof(szPlayerName));
		pCommand->GetParameter(szChatRoomName, 1, MPT_STR, sizeof(szChatRoomName));

		OnChatRoomLeave(szPlayerName, szChatRoomName);
	}
	break;
	case MC_MATCH_CHATROOM_SELECT_WRITE:
	{
		char szChatRoomName[128] = "";
		pCommand->GetParameter(szChatRoomName, 0, MPT_STR, sizeof(szChatRoomName));

		OnChatRoomSelectWrite(szChatRoomName);
	}
	break;
	case MC_MATCH_CHATROOM_INVITE:
	{
		char szSenderName[64] = "";
		char szTargetName[64] = "";
		char szRoomName[128] = "";

		pCommand->GetParameter(szSenderName, 0, MPT_STR, sizeof(szSenderName));
		pCommand->GetParameter(szTargetName, 1, MPT_STR, sizeof(szTargetName));
		pCommand->GetParameter(szRoomName, 2, MPT_STR, sizeof(szRoomName));

		OnChatRoomInvite(szSenderName, szRoomName);
	}
	break;
	case MC_MATCH_CHATROOM_CHAT:
	{
		char szChatRoomName[128] = "";
		char szPlayerName[128] = "";
		char szChat[128] = "";

		pCommand->GetParameter(szChatRoomName, 0, MPT_STR, sizeof(szChatRoomName));
		pCommand->GetParameter(szPlayerName, 1, MPT_STR, sizeof(szPlayerName));
		pCommand->GetParameter(szChat, 2, MPT_STR, sizeof(szChat));

		OnChatRoomChat(szChatRoomName, szPlayerName, szChat);
	}
	break;
	case ZC_REPORT_119:
	{
		OnLocalReport119();
	}
	break;
	case ZC_MESSAGE:
	{
		int nMessageID;
		pCommand->GetParameter(&nMessageID, 0, MPT_INT);
		ZGetGameInterface()->ShowMessage(nMessageID);
	}break;
	case MC_TEST_PEERTEST_PING:
	{
		MUID uidSender = pCommand->GetSenderUID();
		char szLog[128];
		sprintf_safe(szLog, "PEERTEST_PING: from (%d%d)", uidSender.High, uidSender.Low);
		ZChatOutput(szLog, ZChat::CMT_SYSTEM);
	}
	break;
	case MC_TEST_PEERTEST_PONG:
	{
	}
	break;
	case MC_MATCH_CLAN_RESPONSE_CREATE_CLAN:
	{
		int nResult, nRequestID;
		pCommand->GetParameter(&nResult, 0, MPT_INT);
		pCommand->GetParameter(&nRequestID, 1, MPT_INT);

		OnResponseCreateClan(nResult, nRequestID);

	}
	break;
	case MC_MATCH_CLAN_RESPONSE_AGREED_CREATE_CLAN:
	{
		int nResult;
		pCommand->GetParameter(&nResult, 0, MPT_INT);

		OnResponseAgreedCreateClan(nResult);
	}
	break;
	case MC_MATCH_CLAN_ASK_SPONSOR_AGREEMENT:
	{
		int nRequestID;
		char szClanName[256];
		MUID uidMasterObject;
		char szMasterName[256];


		pCommand->GetParameter(&nRequestID, 0, MPT_INT);
		pCommand->GetParameter(szClanName, 1, MPT_STR, sizeof(szClanName));
		pCommand->GetParameter(&uidMasterObject, 2, MPT_UID);
		pCommand->GetParameter(szMasterName, 3, MPT_STR, sizeof(szMasterName));

		OnClanAskSponsorAgreement(nRequestID, szClanName, uidMasterObject, szMasterName);
	}
	break;
	case MC_MATCH_CLAN_ANSWER_SPONSOR_AGREEMENT:
	{
		MUID uidClanMaster;
		int nRequestID;
		bool bAnswer;
		char szCharName[256];

		pCommand->GetParameter(&nRequestID, 0, MPT_INT);
		pCommand->GetParameter(&uidClanMaster, 1, MPT_UID);
		pCommand->GetParameter(szCharName, 2, MPT_STR, sizeof(szCharName));
		pCommand->GetParameter(&bAnswer, 3, MPT_BOOL);

		OnClanAnswerSponsorAgreement(nRequestID, uidClanMaster, szCharName, bAnswer);
	}
	break;
	case MC_MATCH_CLAN_RESPONSE_CLOSE_CLAN:
	{
		int nResult;
		pCommand->GetParameter(&nResult, 0, MPT_INT);

		OnClanResponseCloseClan(nResult);
	}
	break;
	case MC_MATCH_CLAN_RESPONSE_JOIN_CLAN:
	{
		int nResult;
		pCommand->GetParameter(&nResult, 0, MPT_INT);
		OnClanResponseJoinClan(nResult);
	}
	break;
	case MC_MATCH_CLAN_ASK_JOIN_AGREEMENT:
	{
		char szClanName[256], szClanAdmin[256];
		MUID uidClanAdmin;

		pCommand->GetParameter(szClanName, 0, MPT_STR, sizeof(szClanName));
		pCommand->GetParameter(&uidClanAdmin, 1, MPT_UID);
		pCommand->GetParameter(szClanAdmin, 2, MPT_STR, sizeof(szClanAdmin));

		OnClanAskJoinAgreement(szClanName, uidClanAdmin, szClanAdmin);
	}
	break;
	case MC_MATCH_CLAN_ANSWER_JOIN_AGREEMENT:
	{
		MUID uidClanAdmin;
		bool bAnswer;
		char szJoiner[256];

		pCommand->GetParameter(&uidClanAdmin, 0, MPT_UID);
		pCommand->GetParameter(szJoiner, 1, MPT_STR, sizeof(szJoiner));
		pCommand->GetParameter(&bAnswer, 2, MPT_BOOL);

		OnClanAnswerJoinAgreement(uidClanAdmin, szJoiner, bAnswer);
	}
	break;
	case MC_MATCH_CLAN_RESPONSE_AGREED_JOIN_CLAN:
	{
		int nResult;
		pCommand->GetParameter(&nResult, 0, MPT_INT);
		OnClanResponseAgreedJoinClan(nResult);
	}
	break;
	case MC_MATCH_CLAN_UPDATE_CHAR_CLANINFO:
	{
		MCommandParameter* pParam = pCommand->GetParameter(0);
		if (pParam->GetType() != MPT_BLOB) break;
		void* pBlob = pParam->GetPointer();

		OnClanUpdateCharClanInfo(pBlob);
	}
	break;
	case MC_MATCH_CLAN_RESPONSE_LEAVE_CLAN:
	{
		int nResult;
		pCommand->GetParameter(&nResult, 0, MPT_INT);
		OnClanResponseLeaveClan(nResult);
	}
	break;
	case MC_MATCH_CLAN_MASTER_RESPONSE_CHANGE_GRADE:
	{
		int nResult;
		pCommand->GetParameter(&nResult, 0, MPT_INT);
		OnClanResponseChangeGrade(nResult);
	}
	break;
	case MC_MATCH_CLAN_ADMIN_RESPONSE_EXPEL_MEMBER:
	{
		int nResult;
		pCommand->GetParameter(&nResult, 0, MPT_INT);
		OnClanResponseExpelMember(nResult);
	}
	break;
	case MC_MATCH_CLAN_MSG:
	{
		char szSenderName[256];
		char szMsg[512];

		pCommand->GetParameter(szSenderName, 0, MPT_STR, sizeof(szSenderName));
		pCommand->GetParameter(szMsg, 1, MPT_STR, sizeof(szMsg));

		OnClanMsg(szSenderName, szMsg);
	}
	break;
	case MC_MATCH_CLAN_RESPONSE_MEMBER_LIST:
	{
		MCommandParameter* pParam = pCommand->GetParameter(0);
		if (pParam->GetType() != MPT_BLOB) break;
		void* pBlob = pParam->GetPointer();

		OnClanMemberList(pBlob);

	}
	break;
	case MC_MATCH_CLAN_RESPONSE_CLAN_INFO:
	{
		MCommandParameter* pParam = pCommand->GetParameter(0);
		if (pParam->GetType() != MPT_BLOB) break;
		void* pBlob = pParam->GetPointer();

		OnClanResponseClanInfo(pBlob);
	}
	break;
	case MC_MATCH_CLAN_RESPONSE_EMBLEMURL:
	{
		int nCLID = 0;
		int nEmblemChecksum = 0;
		char szURL[4096] = "";

		pCommand->GetParameter(&nCLID, 0, MPT_INT);
		pCommand->GetParameter(&nEmblemChecksum, 1, MPT_INT);
		pCommand->GetParameter(szURL, 2, MPT_STR, sizeof(szURL));

		OnClanResponseEmblemURL(nCLID, nEmblemChecksum, szURL);
	}
	break;
	case MC_MATCH_CLAN_LOCAL_EMBLEMREADY:
	{
		int nCLID = 0;
		char szURL[4096] = "";

		pCommand->GetParameter(&nCLID, 0, MPT_INT);
		pCommand->GetParameter(szURL, 1, MPT_STR, sizeof(szURL));

		OnClanEmblemReady(nCLID, szURL);
	}
	break;
	case MC_MATCH_RESPONSE_RESULT:
	{
		int nResult;
		pCommand->GetParameter(&nResult, 0, MPT_INT);
		if (nResult != MOK)
		{
			ZApplication::GetGameInterface()->ShowErrorMessage(nResult);
		}
	}
	break;
	case MC_MATCH_RESPONSE_CHARINFO_DETAIL:
	{
		MCommandParameter* pParam = pCommand->GetParameter(0);
		if (pParam->GetType() != MPT_BLOB) break;
		void* pBlob = pParam->GetPointer();

		OnResponseCharInfoDetail(pBlob);
	}
	break;
	case MC_MATCH_RESPONSE_PROPOSAL:
	{
		int nResult, nProposalMode, nRequestID;

		pCommand->GetParameter(&nResult, 0, MPT_INT);
		pCommand->GetParameter(&nProposalMode, 1, MPT_INT);
		pCommand->GetParameter(&nRequestID, 2, MPT_INT);

		OnResponseProposal(nResult, MMatchProposalMode(nProposalMode), nRequestID);
	}
	break;
	case MC_MATCH_ASK_AGREEMENT:
	{
		MUID uidProposer;
		int nProposalMode, nRequestID;

		pCommand->GetParameter(&uidProposer, 0, MPT_UID);

		MCommandParameter* pParam = pCommand->GetParameter(1);
		void* pMemberNamesBlob = pParam->GetPointer();

		pCommand->GetParameter(&nProposalMode, 2, MPT_INT);
		pCommand->GetParameter(&nRequestID, 3, MPT_INT);

		OnAskAgreement(uidProposer, pMemberNamesBlob, MMatchProposalMode(nProposalMode), nRequestID);
	}
	break;
	case MC_MATCH_REPLY_AGREEMENT:
	{
		MUID uidProposer, uidChar;
		char szReplierName[256];
		int nProposalMode, nRequestID;
		bool bAgreement;

		pCommand->GetParameter(&uidProposer, 0, MPT_UID);
		pCommand->GetParameter(&uidChar, 1, MPT_UID);
		pCommand->GetParameter(szReplierName, 2, MPT_STR, sizeof(szReplierName));
		pCommand->GetParameter(&nProposalMode, 3, MPT_INT);
		pCommand->GetParameter(&nRequestID, 4, MPT_INT);
		pCommand->GetParameter(&bAgreement, 5, MPT_BOOL);

		OnReplyAgreement(uidProposer, uidChar, szReplierName, MMatchProposalMode(nProposalMode),
			nRequestID, bAgreement);
	}
	break;
	case MC_MATCH_LADDER_SEARCH_RIVAL:
	{
		ZGetGameInterface()->OnArrangedTeamGameUI(true);
	}break;
	case MC_MATCH_LADDER_CANCEL_CHALLENGE:
	{
		ZGetGameInterface()->OnArrangedTeamGameUI(false);

		char szCharName[MATCHOBJECT_NAME_LENGTH];
		pCommand->GetParameter(szCharName, 0, MPT_STR, sizeof(szCharName));

		if (szCharName[0] != 0)
		{
			char szOutput[256];
			ZTransMsg(szOutput, MSG_LADDER_CANCEL, 1, szCharName);
			ZChatOutput(MCOLOR(ZCOLOR_CHAT_SYSTEM), szOutput);
		}
		else
		{
			ZChatOutput(MCOLOR(ZCOLOR_CHAT_SYSTEM),
				ZMsg(MSG_LADDER_FAILED));
		}
	}break;
	case MC_MATCH_LADDER_RESPONSE_CHALLENGE:
	{
		int nResult;
		pCommand->GetParameter(&nResult, 0, MPT_INT);
		OnLadderResponseChallenge(nResult);
	}
	break;
	case MC_MATCH_LADDER_PREPARE:
	{
		MUID uidStage;
		pCommand->GetParameter(&uidStage, 0, MPT_UID);
		int nTeam;
		pCommand->GetParameter(&nTeam, 1, MPT_INT);

		OnLadderPrepare(uidStage, nTeam);
	}break;
	case MC_MATCH_LADDER_LAUNCH:
	{
		MUID uidStage;
		pCommand->GetParameter(&uidStage, 0, MPT_UID);
		char szMapName[128];
		pCommand->GetParameter(szMapName, 1, MPT_STR, sizeof(szMapName));

		OnLadderLaunch(uidStage, szMapName);
	}break;
	case MC_MATCH_CLAN_STANDBY_CLAN_LIST:
	{
		int nPrevStageCount, nNextStageCount;
		pCommand->GetParameter(&nPrevStageCount, 0, MPT_INT);
		pCommand->GetParameter(&nNextStageCount, 1, MPT_INT);

		MCommandParameter* pParam = pCommand->GetParameter(2);
		if (pParam->GetType() != MPT_BLOB) break;
		void* pBlob = pParam->GetPointer();

		OnClanStandbyClanList(nPrevStageCount, nNextStageCount, pBlob);
	}
	break;
	case MC_MATCH_CLAN_MEMBER_CONNECTED:
	{
		char szMember[256];

		pCommand->GetParameter(szMember, 0, MPT_STR, sizeof(szMember));
		OnClanMemberConnected(szMember);
	}
	break;
	case MC_MATCH_NOTIFY_CALLVOTE:
	{
		char szDiscuss[128] = "";
		char szArg[256] = "";

		pCommand->GetParameter(szDiscuss, 0, MPT_STR, sizeof(szDiscuss));
		pCommand->GetParameter(szArg, 1, MPT_STR, sizeof(szArg));
		OnNotifyCallVote(szDiscuss, szArg);
	}
	break;
	case MC_MATCH_NOTIFY_VOTERESULT:
	{
		char szDiscuss[128];
		int nResult = 0;

		pCommand->GetParameter(szDiscuss, 0, MPT_STR, sizeof(szDiscuss));
		pCommand->GetParameter(&nResult, 1, MPT_INT);
		OnNotifyVoteResult(szDiscuss, nResult);
	}
	break;
	case MC_MATCH_VOTE_RESPONSE:
	{
		int nMsgCode = 0;
		pCommand->GetParameter(&nMsgCode, 0, MPT_INT);
		OnVoteAbort(nMsgCode);
	}
	break;
	case MC_MATCH_BROADCAST_CLAN_RENEW_VICTORIES:
	{
		char szWinnerClanName[256], szLoserClanName[256];
		int nVictories;

		pCommand->GetParameter(szWinnerClanName, 0, MPT_STR, sizeof(szWinnerClanName));
		pCommand->GetParameter(szLoserClanName, 1, MPT_STR, sizeof(szLoserClanName));
		pCommand->GetParameter(&nVictories, 2, MPT_INT);
		OnBroadcastClanRenewVictories(szWinnerClanName, szLoserClanName, nVictories);
	}
	break;
	case MC_MATCH_BROADCAST_CLAN_INTERRUPT_VICTORIES:
	{
		char szWinnerClanName[256], szLoserClanName[256];
		int nVictories;

		pCommand->GetParameter(szWinnerClanName, 0, MPT_STR, sizeof(szWinnerClanName));
		pCommand->GetParameter(szLoserClanName, 1, MPT_STR, sizeof(szLoserClanName));
		pCommand->GetParameter(&nVictories, 2, MPT_INT);
		OnBroadcastClanInterruptVictories(szWinnerClanName, szLoserClanName, nVictories);
	}
	break;
	case MC_MATCH_BROADCAST_DUEL_RENEW_VICTORIES:
	{
		char szChannelName[256], szChampionName[256];
		int nVictories, nRoomNo;

		pCommand->GetParameter(szChampionName, 0, MPT_STR, sizeof(szChampionName));
		pCommand->GetParameter(szChannelName, 1, MPT_STR, sizeof(szChannelName));
		pCommand->GetParameter(&nRoomNo, 2, MPT_INT);
		pCommand->GetParameter(&nVictories, 3, MPT_INT);
		OnBroadcastDuelRenewVictories(szChampionName, szChannelName, nRoomNo, nVictories);
	}
	break;
	case MC_MATCH_BROADCAST_DUEL_INTERRUPT_VICTORIES:
	{
		char szChampionName[256], szInterrupterName[256];
		int nVictories;

		pCommand->GetParameter(szChampionName, 0, MPT_STR, sizeof(szChampionName));
		pCommand->GetParameter(szInterrupterName, 1, MPT_STR, sizeof(szInterrupterName));
		pCommand->GetParameter(&nVictories, 2, MPT_INT);
		OnBroadcastDuelInterruptVictories(szChampionName, szInterrupterName, nVictories);
	}
	break;
	case MC_MATCH_RESPONSE_STAGE_FOLLOW:
	{
		int nMsgID;
		pCommand->GetParameter(&nMsgID, 0, MPT_INT);
		OnFollowResponse(nMsgID);
	}
	break;
	case MC_MATCH_SCHEDULE_ANNOUNCE_SEND:
	{
		char cAnnounce[512] = { 0 };
		pCommand->GetParameter(cAnnounce, 0, MPT_STR, sizeof(cAnnounce));
		ZChatOutput(cAnnounce);
	}
	break;
	case MC_MATCH_EXPIRED_RENT_ITEM:
	{
		MCommandParameter* pParam = pCommand->GetParameter(0);
		if (pParam->GetType() != MPT_BLOB) break;
		void* pBlob = pParam->GetPointer();

		OnExpiredRentItem(pBlob);
	}
	break;
	case MC_MATCH_FIND_HACKING:
	{
	}
	break;
	case MC_MATCH_ROUTE_UPDATE_STAGE_EQUIP_LOOK:
	{
		MUID uidPlayer;
		int nParts;
		int nItemID;

		pCommand->GetParameter(&uidPlayer, 0, MPT_UID);
		pCommand->GetParameter(&nParts, 1, MPT_INT);
		pCommand->GetParameter(&nItemID, 2, MPT_INT);

		OnResponseUpdateStageEquipLook(uidPlayer, nParts, nItemID);
	}
	break;
	default:
		if (!ret)
		{
		}
		break;
	}

	if (m_fnOnCommandCallback) ret = m_fnOnCommandCallback(pCommand);

	return ret;
}
