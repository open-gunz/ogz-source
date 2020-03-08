#include "stdafx.h"
#include "MSharedCommandTable.h"
#include "MErrorTable.h"
#include "MBlobArray.h"
#include "MObject.h"
#include "MMatchObject.h"
#include "MAgentObject.h"
#include "MMatchNotify.h"
#include "MMatchStage.h"
#include "MCommandCommunicator.h"
#include "MMatchTransDataType.h"
#include "MDebug.h"
#include "RTypes.h"
#include "MFile.h"
#include "MMatchStatus.h"
#include "MMatchSchedule.h"
#include "MTypes.h"
#include "MMatchUtil.h"
#include "sodium.h"
#include "MMatchServer.h"

#define _STATUS_CMD_START	auto nStatusStartTime = GetGlobalTimeMS();
#define _STATUS_CMD_END		MGetServerStatusSingleton()->AddCmd(pCommand->GetID(), 0, GetGlobalTimeMS()-nStatusStartTime);

bool MMatchServer::OnCommand(MCommand* pCommand)
{
	MGetServerStatusSingleton()->AddCmd(pCommand->GetID());
	if (MServer::OnCommand(pCommand) == true)
		return true;

_STATUS_CMD_START;

	switch(pCommand->GetID()){
		case MC_MATCH_LOGIN:
			{
				char szUserID[64];
				int nCommandVersion = 0;
				u32 nChecksumPack = 0;
				int nVersion = -1;
				if (pCommand->GetParameter(szUserID, 0, MPT_STR, sizeof(szUserID) )==false) break;

				auto Param = static_cast<MCmdParamBlob*>(pCommand->GetParameter(1));
				if (!Param || Param->GetType() != MPT_BLOB)
					break;

				unsigned char *HashedPassword = (unsigned char *)Param->GetPointer();
				int HashLength = Param->GetPayloadSize();

				if (pCommand->GetParameter(&nCommandVersion, 2, MPT_INT)==false) break;
				if (pCommand->GetParameter(&nChecksumPack, 3, MPT_UINT) == false) break;

				u32 Major, Minor, Patch, Revision;
				if (pCommand->GetParameter(&Major, 4, MPT_UINT) == false) break;
				if (pCommand->GetParameter(&Minor, 5, MPT_UINT) == false) break;
				if (pCommand->GetParameter(&Patch, 6, MPT_UINT) == false) break;
				if (pCommand->GetParameter(&Revision, 7, MPT_UINT) == false) break;

				OnMatchLogin(pCommand->GetSenderUID(),
					szUserID, HashedPassword, HashLength,
					nCommandVersion, nChecksumPack,
					Major, Minor, Patch, Revision);
			}
			break;
		case MC_MATCH_REQUEST_CREATE_ACCOUNT:
		{
			char szUserID[64];
			char szEmail[64];
			if (pCommand->GetParameter(szUserID, 0, MPT_STR, sizeof(szUserID)) == false) break;

			auto Param = pCommand->GetParameter(1);
			if (!Param || Param->GetType() != MPT_BLOB)
				break;

			unsigned char *HashedPassword = (unsigned char *)Param->GetPointer();
			int HashLength = Param->GetSize() - sizeof(int);

			if (pCommand->GetParameter(szEmail, 2, MPT_STR, sizeof(szEmail)) == false) break;

			CreateAccount(pCommand->GetSenderUID(), szUserID, HashedPassword, HashLength, szEmail);
		}
		break;
		case MC_MATCH_SEND_VOICE_CHAT:
		{
			auto Param = pCommand->GetParameter(0);
			if (!Param || Param->GetType() != MPT_BLOB)
				break;

			unsigned char *Data = (unsigned char *)Param->GetPointer();
			auto Length = Param->GetSize() - sizeof(int);

			OnVoiceChat(pCommand->GetSenderUID(), Data, Length);
		}
		break;
		case MC_MATCH_P2P_COMMAND:
		{
			auto Sender = pCommand->GetSenderUID();
			MUID Receiver;
			if (!pCommand->GetParameter(&Receiver, 0, MPT_UID)) break;
			auto Param = pCommand->GetParameter(1);
			if (Param->GetType() != MPT_BLOB) break;
			auto Blob = (MCmdParamBlob*)Param;
			auto BlobPtr = Blob->GetPointer();

			OnTunnelledP2PCommand(Sender, Receiver, (char*)BlobPtr, Blob->GetPayloadSize());
		}
		break;
		case MC_MATCH_UPDATE_CLIENT_SETTINGS:
		{
			auto Param = pCommand->GetParameter(0);
			if (Param->GetType() != MPT_BLOB) break;
			auto Blob = (MCmdParamBlob*)Param;
			auto BlobPtr = Blob->GetPointer();
			auto BlobSize = Blob->GetPayloadSize();
			if (BlobSize != sizeof(MTD_ClientSettings))
				break;
			auto Obj = GetObject(pCommand->GetSenderUID());
			if (!Obj)
				break;

			Obj->clientSettings = { reinterpret_cast<MTD_ClientSettings*>(BlobPtr)->DebugOutput };
		}
		break;
		case MC_MATCH_REQUEST_CREATE_BOT:
		{
			OnRequestCreateBot(pCommand->GetSenderUID());
		}
		break;
		case MC_MATCH_REQUEST_SPEC:
		{
			bool Value;
			if (!pCommand->GetParameter(&Value, 0, MPT_BOOL)) break;
			OnRequestSpec(pCommand->GetSenderUID(), Value);
		}
		break;
		case MC_MATCH_LOGIN_FROM_DBAGENT:
			{
				MUID CommUID;
				char szLoginID[256];
				char szName[256];
				int nSex;
				bool bFreeLoginIP;
				u32 nChecksumPack;

				if (pCommand->GetParameter(&CommUID,		0, MPT_UID)==false) break;
				if (pCommand->GetParameter(szLoginID,		1, MPT_STR, sizeof(szLoginID) )==false) break;
				if (pCommand->GetParameter(szName,			2, MPT_STR, sizeof(szName) )==false) break;
				if (pCommand->GetParameter(&nSex,			3, MPT_INT)==false) break;
				if (pCommand->GetParameter(&bFreeLoginIP,	4, MPT_BOOL)==false) break;
				if (pCommand->GetParameter(&nChecksumPack,	5, MPT_UINT)==false) break;

				OnMatchLoginFromDBAgent(CommUID, szLoginID, szName, nSex, bFreeLoginIP, nChecksumPack);
			}
			break;
		case MC_MATCH_LOGIN_FROM_DBAGENT_FAILED:
			{
				/*MUID CommUID;
				int nResult;

				if (pCommand->GetParameter(&CommUID,	0, MPT_UID)==false) break;
				if (pCommand->GetParameter(&nResult,	1, MPT_INT)==false) break;

				OnMatchLoginFailedFromDBAgent(CommUID, nResult);*/
			}
			break;

		case MC_MATCH_BRIDGEPEER:
			{
				MUID uidChar;
				u32 dwIP, nPort;

				pCommand->GetParameter(&uidChar,	0, MPT_UID);
				pCommand->GetParameter(&dwIP,		1, MPT_UINT);
				pCommand->GetParameter(&nPort,		2, MPT_UINT);
				OnBridgePeer(uidChar, dwIP, nPort);
			}
			break;
		case MC_MATCH_REQUEST_RECOMMANDED_CHANNEL:
			{
				OnRequestRecommendedChannel(pCommand->GetSenderUID());
			}
			break;
		case MC_MATCH_CHANNEL_REQUEST_JOIN:
			{
				MUID uidPlayer, uidChannel;

				uidPlayer = pCommand->GetSenderUID();
				pCommand->GetParameter(&uidChannel, 1, MPT_UID);

				OnRequestChannelJoin(uidPlayer, uidChannel);
			}
			break;
		case MC_MATCH_CHANNEL_REQUEST_JOIN_FROM_NAME:
			{
				MUID uidPlayer = pCommand->GetSenderUID();
				int nChannelType;
				char szChannelName[256];

				pCommand->GetParameter(&nChannelType,	1, MPT_INT);
				pCommand->GetParameter(szChannelName,	2, MPT_STR, sizeof(szChannelName) );

				OnRequestChannelJoin(uidPlayer, MCHANNEL_TYPE(nChannelType), szChannelName);
			}
			break;
		case MC_MATCH_CHANNEL_LIST_START:
			{
				MUID uidPlayer = pCommand->GetSenderUID();
				int nChannelType;

				pCommand->GetParameter(&nChannelType,	1, MPT_INT);

				OnStartChannelList(uidPlayer, nChannelType);
			}
			break;
		case MC_MATCH_CHANNEL_LIST_STOP:
			{
				MUID uidPlayer = pCommand->GetSenderUID();

				OnStopChannelList(uidPlayer);
			}
			break;
		case MC_MATCH_CHANNEL_REQUEST_CHAT:
			{
				MUID uidPlayer, uidChannel;
				static char szChat[1024];
				uidPlayer = pCommand->GetSenderUID();

				pCommand->GetParameter(&uidChannel, 1, MPT_UID);
				pCommand->GetParameter(szChat, 2, MPT_STR, sizeof(szChat) );

				OnChannelChat(uidPlayer, uidChannel, szChat);
			}
			break;
		case MC_MATCH_STAGE_TEAM:
			{
				MUID uidPlayer, uidStage;
				MMatchTeam nTeam;
				uidPlayer = pCommand->GetSenderUID();

				pCommand->GetParameter(&uidStage, 1, MPT_UID);
				pCommand->GetParameter(&nTeam, 2, MPT_UINT);

				OnStageTeam(uidPlayer, uidStage, nTeam);
			}
			break;
		case MC_MATCH_STAGE_PLAYER_STATE:
			{
				MUID uidPlayer, uidStage;
				int nStageState;
				uidPlayer = pCommand->GetSenderUID();

				pCommand->GetParameter(&uidStage, 1, MPT_UID);
				pCommand->GetParameter(&nStageState, 2, MPT_INT);


				OnStagePlayerState(uidPlayer, uidStage, MMatchObjectStageState(nStageState));

			}
			break;
		case MC_MATCH_STAGE_CREATE:
			{
				MUID uidChar;
				char szStageName[128], szStagePassword[128];
				bool bPrivate;

				memset(szStagePassword, 0, sizeof(szStagePassword));
				pCommand->GetParameter(&uidChar, 0, MPT_UID);
				pCommand->GetParameter(szStageName, 1, MPT_STR, sizeof(szStageName) );
				pCommand->GetParameter(&bPrivate, 2, MPT_BOOL);
				pCommand->GetParameter(szStagePassword, 3, MPT_STR, sizeof(szStagePassword) );

				OnStageCreate(uidChar, szStageName, bPrivate, szStagePassword);
			}
			break;
		case MC_MATCH_REQUEST_STAGE_JOIN:
			{
				MUID uidPlayer, uidStage;
				uidPlayer = pCommand->GetSenderUID();

				pCommand->GetParameter(&uidStage, 1, MPT_UID);

				OnStageJoin(uidPlayer, uidStage);
			}
			break;
		case MC_MATCH_REQUEST_PRIVATE_STAGE_JOIN:
			{
				MUID uidPlayer, uidStage;
				char szPassword[256];
				uidPlayer = pCommand->GetSenderUID();

				pCommand->GetParameter(&uidStage, 1, MPT_UID);
				pCommand->GetParameter(szPassword, 2, MPT_STR, sizeof(szPassword) );

				OnPrivateStageJoin(uidPlayer, uidStage, szPassword);
			}
			break;
		case MC_MATCH_STAGE_FOLLOW:
			{
				char szTargetName[MATCHOBJECT_NAME_LENGTH+1];

				pCommand->GetParameter(szTargetName, 0, MPT_STR, sizeof(szTargetName) );

				OnStageFollow(pCommand->GetSenderUID(), szTargetName);
			}
			break;
		case MC_MATCH_STAGE_LEAVE:
			{
				MUID uidPlayer, uidStage;
				uidPlayer = pCommand->GetSenderUID();

				pCommand->GetParameter(&uidStage, 1, MPT_UID);

				OnStageLeave(uidPlayer, uidStage);
			}
			break;
		case MC_MATCH_STAGE_REQUEST_PLAYERLIST:
			{
				MUID uidStage;
				pCommand->GetParameter(&uidStage, 0, MPT_UID);
				OnStageRequestPlayerList(pCommand->GetSenderUID(), uidStage);
			}
			break;
		case MC_MATCH_STAGE_REQUEST_ENTERBATTLE:
			{
				MUID uidPlayer, uidStage;
				uidPlayer = pCommand->GetSenderUID();

				pCommand->GetParameter(&uidStage, 1, MPT_UID);
				OnStageEnterBattle(uidPlayer, uidStage);
			}
			break;
		case MC_MATCH_STAGE_LEAVEBATTLE:
			{
				MUID uidPlayer, uidStage, uidTarget;
				uidPlayer = pCommand->GetSenderUID();

				pCommand->GetParameter(&uidTarget, 0, MPT_UID);
				pCommand->GetParameter(&uidStage, 1, MPT_UID);
				OnStageLeaveBattle(uidPlayer, uidStage, uidTarget);
			}
			break;
		case MC_MATCH_STAGE_START:
			{
				MUID uidPlayer, uidStage;
				int nCountdown;
				uidPlayer = pCommand->GetSenderUID();

				pCommand->GetParameter(&uidStage, 1, MPT_UID);
				pCommand->GetParameter(&nCountdown, 2, MPT_INT);

				OnStageStart(uidPlayer, uidStage, nCountdown);
			}
			break;
		case MC_MATCH_GAME_ROUNDSTATE:
			{
				MUID uidStage;
				int nState, nRound;

				pCommand->GetParameter(&uidStage, 0, MPT_UID);
				pCommand->GetParameter(&nState, 1, MPT_INT);
				pCommand->GetParameter(&nRound, 2, MPT_INT);

				OnGameRoundState(uidStage, nState, nRound);
			}
			break;
		case MC_MATCH_GAME_KILL:
			{
				MUID uidAttacker, uidVictim;
				pCommand->GetParameter(&uidAttacker, 0, MPT_UID);
				uidVictim = pCommand->GetSenderUID();

				OnGameKill(uidAttacker, uidVictim);
			}
			break;
		case MC_MATCH_GAME_REQUEST_SPAWN:
			{
				MUID uidChar;
				MVector pos, dir;

				pCommand->GetParameter(&uidChar, 0, MPT_UID);
				pCommand->GetParameter(&pos, 1, MPT_POS);
				pCommand->GetParameter(&dir, 2, MPT_DIR);

				OnRequestSpawn(pCommand->GetSenderUID(), pos, dir);
			}
			break;
		case MC_MATCH_GAME_REQUEST_TIMESYNC:
			{
				unsigned int nLocalTimeStamp;
				pCommand->GetParameter(&nLocalTimeStamp, 0, MPT_UINT);

				OnGameRequestTimeSync(pCommand->GetSenderUID(), nLocalTimeStamp);
			}
			break;
		case MC_MATCH_GAME_REPORT_TIMESYNC:
			{
				unsigned int nLocalTimeStamp, nDataChecksum;
				pCommand->GetParameter(&nLocalTimeStamp, 0, MPT_UINT);
				pCommand->GetParameter(&nDataChecksum, 1, MPT_UINT);

				OnGameReportTimeSync(pCommand->GetSenderUID(), nLocalTimeStamp, nDataChecksum);
			}
			break;
		case MC_MATCH_STAGE_CHAT:
			{
				MUID uidPlayer, uidStage;
				static char szChat[1024];
				uidPlayer = pCommand->GetSenderUID();

				pCommand->GetParameter(&uidStage, 1, MPT_UID);
				pCommand->GetParameter(szChat, 2, MPT_STR, sizeof(szChat) );

				OnStageChat(uidPlayer, uidStage, szChat);
			}
			break;
		case MC_MATCH_STAGE_REQUEST_QUICKJOIN:
			{
				MUID uidPlayer = pCommand->GetSenderUID();

				MCommandParameter* pQuickJoinParam = pCommand->GetParameter(1);
				if(pQuickJoinParam->GetType()!=MPT_BLOB) break;

				void* pQuickJoinBlob = pQuickJoinParam->GetPointer();

				OnRequestQuickJoin(uidPlayer, pQuickJoinBlob);

			}
			break;
		case MC_MATCH_STAGE_GO:
			{
				unsigned int nRoomNo;

				pCommand->GetParameter(&nRoomNo, 0, MPT_UINT);
				OnStageGo(pCommand->GetSenderUID(), nRoomNo);
			}
			break;
		case MC_MATCH_STAGE_LIST_START:
			{
				OnStartStageList(pCommand->GetSenderUID());

			}
			break;
		case MC_MATCH_STAGE_LIST_STOP:
			{
				OnStopStageList(pCommand->GetSenderUID());
			}
			break;
		case MC_MATCH_REQUEST_STAGE_LIST:
			{
				MUID uidPlayer, uidChannel;
				int nStageCursor;
				uidPlayer = pCommand->GetSenderUID();

				pCommand->GetParameter(&uidChannel, 1, MPT_UID);
				pCommand->GetParameter(&nStageCursor, 2, MPT_INT);

				OnStageRequestStageList(uidPlayer, uidChannel, nStageCursor);
			}
			break;
		case MC_MATCH_CHANNEL_REQUEST_PLAYER_LIST:
			{
				MUID /*uidPlayer, */uidChannel;
				int nPage;

				//pCommand->GetParameter(&uidPlayer, 0, MPT_UID);
				pCommand->GetParameter(&uidChannel, 1, MPT_UID);
				pCommand->GetParameter(&nPage, 2, MPT_INT);

				OnChannelRequestPlayerList(pCommand->GetSenderUID(), uidChannel, nPage);
			}
			break;
		case MC_MATCH_STAGE_MAP:
			{
				MUID uidStage;
				pCommand->GetParameter(&uidStage, 0, MPT_UID);
				char szMapName[MFile::MaxPath];
				pCommand->GetParameter(szMapName, 1, MPT_STR, sizeof(szMapName) );
				OnStageMap(uidStage, szMapName);
			}
			break;
		case MC_MATCH_STAGESETTING:
			{
				MUID uidPlayer, uidStage;
				uidPlayer = pCommand->GetSenderUID();

				pCommand->GetParameter(&uidStage, 1, MPT_UID);

				MCommandParameter* pStageParam = pCommand->GetParameter(2);
				if(pStageParam->GetType()!=MPT_BLOB) break;
				void* pStageBlob = pStageParam->GetPointer();
				int nStageCount = MGetBlobArrayCount(pStageBlob);

				// Verify size
				auto BlobParam = static_cast<MCmdParamBlob*>(pStageParam);
				auto BlobSize = BlobParam->GetPayloadSize();

				auto BlobInfoSize = MGetBlobArrayInfoSize();

				if (BlobSize - BlobInfoSize != sizeof(MSTAGE_SETTING_NODE))
					break;

				OnStageSetting(uidPlayer, uidStage, pStageBlob, nStageCount);
			}
			break;
		case MC_MATCH_REQUEST_STAGESETTING:
			{
				MUID uidStage;
				pCommand->GetParameter(&uidStage, 0, MPT_UID);
				OnRequestStageSetting(pCommand->GetSenderUID(), uidStage);
			}
			break;
		case MC_MATCH_REQUEST_PEERLIST:
			{
				MUID uidChar, uidStage;
				pCommand->GetParameter(&uidChar, 0, MPT_UID);
				pCommand->GetParameter(&uidStage, 1, MPT_UID);
				OnRequestPeerList(uidChar, uidStage);
			}
			break;
		case MC_MATCH_REQUEST_GAME_INFO:
			{
				MUID uidChar, uidStage;
				pCommand->GetParameter(&uidChar, 0, MPT_UID);
				pCommand->GetParameter(&uidStage, 1, MPT_UID);
				OnRequestGameInfo(uidChar, uidStage);
			}
			break;
		case MC_MATCH_LOADING_COMPLETE:
			{
				MUID uidChar;
				int nPercent;
				pCommand->GetParameter(&uidChar, 0, MPT_UID);
				pCommand->GetParameter(&nPercent, 1, MPT_INT);
				OnMatchLoadingComplete(uidChar, nPercent);
			}
			break;
		case MC_MATCH_REQUEST_PEER_RELAY:
			{
				MUID uidChar, uidPeer;

				if (pCommand->GetParameter(&uidChar, 0, MPT_UID) == false) break;
				if (pCommand->GetParameter(&uidPeer, 1, MPT_UID) == false) break;

				OnRequestRelayPeer(uidChar, uidPeer);
			}
			break;
		case MC_AGENT_PEER_READY:
			{
				MUID uidChar, uidPeer;

				if (pCommand->GetParameter(&uidChar, 0, MPT_UID) == false) break;
				if (pCommand->GetParameter(&uidPeer, 1, MPT_UID) == false) break;

				OnPeerReady(uidChar, uidPeer);
			}
			break;
		case MC_MATCH_REGISTERAGENT:
			{
				char szIP[128];
				int nTCPPort, nUDPPort;

				if (pCommand->GetParameter(&szIP, 0, MPT_STR, sizeof(szIP) ) == false) break;
				if (pCommand->GetParameter(&nTCPPort, 1, MPT_INT) == false) break;
				if (pCommand->GetParameter(&nUDPPort, 2, MPT_INT) == false) break;

				// Not the best way to patch, but working for now
				if (strstr(szIP, "%")) {
					break;
				}

				OnRegisterAgent(pCommand->GetSenderUID(), szIP, nTCPPort, nUDPPort);
			}
			break;
		case MC_MATCH_UNREGISTERAGENT:
			{
				OnUnRegisterAgent(pCommand->GetSenderUID());
			}
			break;
		case MC_MATCH_AGENT_REQUEST_LIVECHECK:
			{
				u32 nTimeStamp;
				u32 nStageCount;
				u32 nUserCount;

				if (pCommand->GetParameter(&nTimeStamp, 0, MPT_UINT) == false) break;
				if (pCommand->GetParameter(&nStageCount, 1, MPT_UINT) == false) break;
				if (pCommand->GetParameter(&nUserCount, 2, MPT_UINT) == false) break;

				OnRequestLiveCheck(pCommand->GetSenderUID(), nTimeStamp, nStageCount, nUserCount);
			}
			break;
		case MC_AGENT_STAGE_READY:
			{
				MUID uidStage;
				pCommand->GetParameter(&uidStage, 0, MPT_UID);
				OnAgentStageReady(pCommand->GetSenderUID(), uidStage);
			}
			break;
		case MC_MATCH_REQUEST_ACCOUNT_CHARLIST:
			{
				MUID uidPlayer = pCommand->GetSenderUID();
				char szSerialKey[256];

				pCommand->GetParameter(szSerialKey, 0, MPT_STR, sizeof(szSerialKey) );
				MCommandParameter* pParam = pCommand->GetParameter(1);
				if(pParam->GetType() != MPT_BLOB) 	break;
				void* pBlob = pParam->GetPointer();
				int nCount = MGetBlobArrayCount(pBlob);

				unsigned char* pbyGuidAckMsg = (unsigned char*)MGetBlobArrayElement(pBlob, 0);

				OnRequestAccountCharList(uidPlayer);
			}
			break;
		case MC_MATCH_REQUEST_ACCOUNT_CHARINFO:
			{
				MUID uidPlayer = pCommand->GetSenderUID();
				char nCharNum;
				pCommand->GetParameter(&nCharNum, 0, MPT_CHAR);


				OnRequestAccountCharInfo(uidPlayer, nCharNum);
			}
			break;
		case MC_MATCH_REQUEST_SELECT_CHAR:
			{
				MUID uidPlayer = pCommand->GetSenderUID();
				u32 nCharIndex;

				pCommand->GetParameter(&nCharIndex, 1, MPT_UINT);

				OnRequestSelectChar(uidPlayer, nCharIndex);
			}
			break;
		case MC_MATCH_REQUEST_DELETE_CHAR:
			{
				MUID uidPlayer = pCommand->GetSenderUID();
				u32 nCharIndex;
				char szCharName[256];

				pCommand->GetParameter(&nCharIndex, 1, MPT_UINT);
				pCommand->GetParameter(szCharName, 2, MPT_STR, sizeof(szCharName) );

				OnRequestDeleteChar(uidPlayer, nCharIndex, szCharName);
			}
			break;
		case MC_MATCH_REQUEST_CREATE_CHAR:
			{
				MUID uidPlayer = pCommand->GetSenderUID();
				u32 nCharIndex;
				u32 nSex, nHair, nFace, nCostume;

				char szCharName[128];

				pCommand->GetParameter(&nCharIndex, 1, MPT_UINT);
				if (pCommand->GetParameter(szCharName, 2, MPT_STR, sizeof(szCharName) )==false) break;
				pCommand->GetParameter(&nSex, 3, MPT_UINT);
				pCommand->GetParameter(&nHair, 4, MPT_UINT);
				pCommand->GetParameter(&nFace, 5, MPT_UINT);
				pCommand->GetParameter(&nCostume, 6, MPT_UINT);

				OnRequestCreateChar(uidPlayer, nCharIndex, szCharName, nSex, nHair, nFace, nCostume);
			}
			break;
		case MC_MATCH_ROUND_FINISHINFO:
			{
				MUID uidStage, uidChar;

				pCommand->GetParameter(&uidStage, 0, MPT_UID);
				pCommand->GetParameter(&uidChar, 1, MPT_UID);
				OnUpdateFinishedRound(uidStage, uidChar, NULL, NULL);
			}
			break;
		case MC_MATCH_REQUEST_BUY_ITEM:
			{
				MUID uidPlayer = pCommand->GetSenderUID();
				u32 nItemID;
				pCommand->GetParameter(&nItemID, 1, MPT_UINT);

				OnRequestBuyItem(uidPlayer, nItemID);
			}
			break;
		case MC_MATCH_REQUEST_SELL_ITEM:
			{
				MUID uidPlayer, uidItem;
				uidPlayer = pCommand->GetSenderUID();

				pCommand->GetParameter(&uidItem, 1, MPT_UID);

				OnRequestSellItem(uidPlayer, uidItem);
			}
			break;
		case MC_MATCH_STAGE_REQUEST_FORCED_ENTRY:
			{
				MUID uidPlayer, uidStage;
				uidPlayer = pCommand->GetSenderUID();

				pCommand->GetParameter(&uidStage, 1, MPT_UID);

				OnRequestForcedEntry(uidStage, uidPlayer);
			}
			break;
		case MC_MATCH_FRIEND_ADD:
			{
				char szArg[1024];
				pCommand->GetParameter(szArg, 0, MPT_STR, sizeof(szArg) );

				OnFriendAdd(pCommand->GetSenderUID(), szArg);
			}
			break;
		case MC_MATCH_FRIEND_REMOVE:
			{
				char szArg[1024];
				pCommand->GetParameter(szArg, 0, MPT_STR, sizeof(szArg) );

				OnFriendRemove(pCommand->GetSenderUID(), szArg);
			}
			break;
		case MC_MATCH_FRIEND_LIST:
			{
				OnFriendList(pCommand->GetSenderUID());
			}
			break;
		case MC_MATCH_FRIEND_MSG:
			{
				char szArg[1024];
				pCommand->GetParameter(szArg, 0, MPT_STR, sizeof(szArg) );

				OnFriendMsg(pCommand->GetSenderUID(), szArg);
			}
			break;
		case MC_ADMIN_ANNOUNCE:
			{
				MUID uidAdmin;
				static char szChat[1024];
				u32 nMsgType = 0;

				pCommand->GetParameter(&uidAdmin, 0, MPT_UID);
				pCommand->GetParameter(szChat, 1, MPT_STR, sizeof(szChat) );
				pCommand->GetParameter(&nMsgType, 2, MPT_UINT);

				OnAdminAnnounce(uidAdmin, szChat, nMsgType);
			}
			break;
		case MC_ADMIN_TERMINAL:
			{
				MUID uidAdmin;
				char szText[1024];

				pCommand->GetParameter(&uidAdmin, 0, MPT_UID);
				pCommand->GetParameter(szText, 1, MPT_STR, sizeof(szText) );

				OnAdminTerminal(uidAdmin, szText);

			}
			break;
		case MC_ADMIN_REQUEST_SERVER_INFO:
			{
				MUID uidAdmin;
				pCommand->GetParameter(&uidAdmin, 0, MPT_UID);

				OnAdminRequestServerInfo(uidAdmin);
			}
			break;
		case MC_ADMIN_SERVER_HALT:
			{
				MUID uidAdmin;
				pCommand->GetParameter(&uidAdmin, 0, MPT_UID);

				OnAdminServerHalt(uidAdmin);
			}
			break;
		case MC_ADMIN_REQUEST_BAN_PLAYER:
			{
				MUID uidAdmin;
				char szPlayer[512];

				pCommand->GetParameter(&uidAdmin, 0, MPT_UID);
				pCommand->GetParameter(szPlayer, 1, MPT_STR, sizeof(szPlayer) );

				OnAdminRequestBanPlayer(uidAdmin, szPlayer);
			}
			break;
		case MC_ADMIN_REQUEST_UPDATE_ACCOUNT_UGRADE:
			{
				MUID uidAdmin;
				char szPlayer[512];

				pCommand->GetParameter(&uidAdmin, 0, MPT_UID);
				pCommand->GetParameter(szPlayer, 1, MPT_STR, sizeof(szPlayer) );

				OnAdminRequestUpdateAccountUGrade(uidAdmin, szPlayer);
			}
			break;
		case MC_ADMIN_REQUEST_SWITCH_LADDER_GAME:
			{
				MUID uidAdmin;
				bool bEnabled;

				pCommand->GetParameter(&uidAdmin, 0, MPT_UID);
				pCommand->GetParameter(&bEnabled, 1, MPT_BOOL);

				OnAdminRequestSwitchLadderGame(uidAdmin, bEnabled);
			}
			break;
		case MC_ADMIN_HIDE:
			{
				OnAdminHide(pCommand->GetSenderUID());
			}
			break;
		case MC_ADMIN_RELOAD_CLIENT_HASH:
			{
			}
			break;
		case MC_ADMIN_PING_TO_ALL:
			{
				OnAdminPingToAll(pCommand->GetSenderUID());
			}
			break;
		case MC_ADMIN_RESET_ALL_HACKING_BLOCK :
			{
				OnAdminResetAllHackingBlock( pCommand->GetSenderUID() );
			}
			break;
		case MC_ADMIN_TELEPORT :
			{
				char szAdminName[MAX_CHARNAME_LENGTH] = "";
				char szTargetName[MAX_CHARNAME_LENGTH] = "";


				if (pCommand->GetParameter(szAdminName, 0, MPT_STR, MAX_CHARNAME_LENGTH) == false) break;
				if (pCommand->GetParameter(szTargetName, 1, MPT_STR, MAX_CHARNAME_LENGTH) == false) break;


				MMatchObject* pObj = GetObject(pCommand->GetSenderUID());

				if (pObj == NULL) break;
				if (!IsAdminGrade(pObj)) break;




				MMatchObject* pTargetObj = GetPlayerByName(szTargetName);
				if (pTargetObj == NULL)
				{
					NotifyMessage(pObj->GetUID(), MATCHNOTIFY_GENERAL_USER_NOTFOUND);
					break;
				}


				MCommand* pCmd = CreateCommand(MC_ADMIN_TELEPORT, MUID(0, 0));
				pCmd->AddParameter(new MCmdParamStr(pObj->GetName()));
				pCmd->AddParameter(new MCmdParamStr(szTargetName));
				RouteToListener(pTargetObj, pCmd);
			}

			break;
		case MC_EVENT_CHANGE_MASTER:
			{
				OnEventChangeMaster(pCommand->GetSenderUID());
			}
			break;
		case MC_EVENT_CHANGE_PASSWORD:
			{
				char szPassword[128]="";
				pCommand->GetParameter(szPassword, 0, MPT_STR, sizeof(szPassword) );

				OnEventChangePassword(pCommand->GetSenderUID() ,szPassword);
			}
			break;
		case MC_EVENT_REQUEST_JJANG:
			{
				char szTargetName[128]="";
				pCommand->GetParameter(szTargetName, 0, MPT_STR, sizeof(szTargetName) );
				OnEventRequestJjang(pCommand->GetSenderUID(), szTargetName);
			}
			break;
		case MC_EVENT_REMOVE_JJANG:
			{
				char szTargetName[128]="";
				pCommand->GetParameter(szTargetName, 0, MPT_STR, sizeof(szTargetName) );
				OnEventRemoveJjang(pCommand->GetSenderUID(), szTargetName);
			}
			break;
		case MC_MATCH_REQUEST_SHOP_ITEMLIST:
			{
				MUID uidPlayer = pCommand->GetSenderUID();
				int nFirstItemIndex = 0, nItemCount = 0;

				pCommand->GetParameter(&nFirstItemIndex, 1, MPT_INT);
				pCommand->GetParameter(&nItemCount, 2, MPT_INT);

				OnRequestShopItemList(uidPlayer, nFirstItemIndex, nItemCount);
			}
			break;
		case MC_MATCH_REQUEST_CHARACTER_ITEMLIST:
			{
				MUID uidPlayer = pCommand->GetSenderUID();

				OnRequestCharacterItemList(uidPlayer);
			}
			break;
		case MC_MATCH_REQUEST_ACCOUNT_ITEMLIST:
			{
				MUID uidPlayer = pCommand->GetSenderUID();
				OnRequestAccountItemList(uidPlayer);
			}
			break;
		case MC_MATCH_REQUEST_BRING_ACCOUNTITEM:
			{
				MUID uidPlayer = pCommand->GetSenderUID();
				int nAIID;
				pCommand->GetParameter(&nAIID, 1, MPT_INT);

				OnRequestBringAccountItem(uidPlayer, nAIID);
			}
			break;
		case MC_MATCH_REQUEST_BRING_BACK_ACCOUNTITEM:
			{
				MUID uidPlayer, uidItem;
				uidPlayer = pCommand->GetSenderUID();

				pCommand->GetParameter(&uidItem, 1, MPT_UID);

				OnRequestBringBackAccountItem(uidPlayer, uidItem);
			}
			break;
		case MC_MATCH_REQUEST_EQUIP_ITEM:
			{
				MUID uidPlayer, uidItem;
				uidPlayer = pCommand->GetSenderUID();
				u32 nEquipmentSlot = 0;

				pCommand->GetParameter(&uidItem, 1, MPT_UID);
				pCommand->GetParameter(&nEquipmentSlot, 2, MPT_UINT);

				OnRequestEquipItem(uidPlayer, uidItem, nEquipmentSlot);
			}
			break;
		case MC_MATCH_REQUEST_TAKEOFF_ITEM:
			{
				MUID uidPlayer = pCommand->GetSenderUID();
				u32 nEquipmentSlot = 0;
				pCommand->GetParameter(&nEquipmentSlot, 1, MPT_UINT);

				OnRequestTakeoffItem(uidPlayer, nEquipmentSlot);
			}
			break;
		case MC_MATCH_REQUEST_SUICIDE:
			{
				MUID uidPlayer = pCommand->GetSenderUID();

				OnRequestSuicide(uidPlayer);
			}
			break;
		case MC_MATCH_REQUEST_OBTAIN_WORLDITEM:
			{
				MUID uidPlayer = pCommand->GetSenderUID();
				int nItemUID = 0;

				pCommand->GetParameter(&nItemUID, 1, MPT_INT);

				OnRequestObtainWorldItem(uidPlayer, nItemUID);
			}
			break;
		case MC_MATCH_REQUEST_SPAWN_WORLDITEM:
			{
				MUID uidPlayer = pCommand->GetSenderUID();
				int nItemID = 0;
				rvector pos;

				pCommand->GetParameter(&nItemID, 1, MPT_INT);
				pCommand->GetParameter(&pos, 2, MPT_POS);

				OnRequestSpawnWorldItem(uidPlayer, nItemID, pos.x, pos.y, pos.z);
			}
			break;
		case MC_MATCH_USER_WHISPER:
			{
				char szSenderName[1024]="";
				char szTargetName[1024]="";
				char szMessage[1024]="";

				if (pCommand->GetParameter(szSenderName, 0, MPT_STR, sizeof(szSenderName) ) == false) break;
				if (pCommand->GetParameter(szTargetName, 1, MPT_STR, sizeof(szTargetName) ) == false) break;
				if (pCommand->GetParameter(szMessage, 2, MPT_STR, sizeof(szMessage) ) == false) break;

				OnUserWhisper(pCommand->GetSenderUID(), szSenderName, szTargetName, szMessage);
			}
			break;
		case MC_MATCH_USER_WHERE:
			{
				char szTargetName[128]="";
				pCommand->GetParameter(szTargetName, 0, MPT_STR, sizeof(szTargetName) );

				OnUserWhere(pCommand->GetSenderUID(), szTargetName);
			}
			break;
		case MC_MATCH_USER_OPTION:
			{
				u32 nOptionFlags=0;
				pCommand->GetParameter(&nOptionFlags, 0, MPT_UINT);

				OnUserOption(pCommand->GetSenderUID(), nOptionFlags);
			}
			break;
		case MC_MATCH_CHATROOM_CREATE:
			{
				MUID uidPlayer = pCommand->GetSenderUID();
				char szChatRoomName[128];

				pCommand->GetParameter(szChatRoomName, 1, MPT_STR, sizeof(szChatRoomName) );

				OnChatRoomCreate(uidPlayer, szChatRoomName);
			}
			break;
		case MC_MATCH_CHATROOM_JOIN:
			{
				char szPlayerName[64];
				char szChatRoomName[128];

				pCommand->GetParameter(szPlayerName, 0, MPT_STR, sizeof(szPlayerName) );
				pCommand->GetParameter(szChatRoomName, 1, MPT_STR, sizeof(szChatRoomName) );

				OnChatRoomJoin(pCommand->GetSenderUID(), szPlayerName, szChatRoomName);
			}
			break;
		case MC_MATCH_CHATROOM_LEAVE:
			{
				char szPlayerName[64];
				char szChatRoomName[128];

				pCommand->GetParameter(szPlayerName, 0, MPT_STR, sizeof(szPlayerName) );
				pCommand->GetParameter(szChatRoomName, 1, MPT_STR, sizeof(szChatRoomName) );

				OnChatRoomLeave(pCommand->GetSenderUID(), szPlayerName, szChatRoomName);
			}
			break;
		case MC_MATCH_CHATROOM_SELECT_WRITE:
			{
				char szChatRoomName[128];

				pCommand->GetParameter(szChatRoomName, 0, MPT_STR, sizeof(szChatRoomName) );

				OnChatRoomSelectWrite(pCommand->GetSenderUID(), szChatRoomName);
			}
			break;
		case MC_MATCH_CHATROOM_INVITE:
			{
				char szSenderName[64]="";
				char szTargetName[64]="";
				char szRoomName[128]="";

				pCommand->GetParameter(szSenderName, 0, MPT_STR, sizeof(szSenderName) );
				pCommand->GetParameter(szTargetName, 1, MPT_STR, sizeof(szTargetName) );
				pCommand->GetParameter(szRoomName, 2, MPT_STR, sizeof(szRoomName) );

				OnChatRoomInvite(pCommand->GetSenderUID(), szTargetName);
			}
			break;
		case MC_MATCH_CHATROOM_CHAT:
			{
				char szRoomName[64];
				char szPlayerName[64];
				char szMessage[256];

				pCommand->GetParameter(szRoomName, 0, MPT_STR, sizeof(szRoomName) );
				pCommand->GetParameter(szPlayerName, 1, MPT_STR, sizeof(szPlayerName) );
				pCommand->GetParameter(szMessage, 2, MPT_STR, sizeof(szMessage));

				OnChatRoomChat(pCommand->GetSenderUID(), szMessage);
			}
			break;
		case MC_MATCH_REQUEST_MY_SIMPLE_CHARINFO:
			{
				//MUID uidPlayer;
				//pCommand->GetParameter(&uidPlayer, 0, MPT_UID);
				OnRequestMySimpleCharInfo(pCommand->GetSenderUID());
			}
			break;
		case MC_MATCH_REQUEST_COPY_TO_TESTSERVER:
			{
				MUID uidPlayer = pCommand->GetSenderUID();
				OnRequestCopyToTestServer(uidPlayer);
			}
			break;
		case MC_MATCH_CLAN_REQUEST_CREATE_CLAN:
			{
				MUID uidPlayer = pCommand->GetSenderUID();
				char szClanName[256];
				int nRequestID;

				pCommand->GetParameter(&nRequestID,			1, MPT_INT);
				pCommand->GetParameter(szClanName,			2, MPT_STR, sizeof(szClanName) );

#if CLAN_SPONSORS_COUNT > 0
				char szSponsorNames[CLAN_SPONSORS_COUNT][256];
				char* sncv[CLAN_SPONSORS_COUNT];

				for (int i = 0; i < CLAN_SPONSORS_COUNT; i++)
				{
					pCommand->GetParameter(szSponsorNames[i],	3+i, MPT_STR, sizeof(szSponsorNames[i]));

					sncv[i] = szSponsorNames[i];
				}
#else
				char** sncv = nullptr;
#endif

				OnClanRequestCreateClan(uidPlayer, nRequestID, szClanName, sncv);
			}
			break;
		case MC_MATCH_CLAN_ANSWER_SPONSOR_AGREEMENT:
			{
				MUID uidClanMaster;
				int nRequestID;
				bool bAnswer;
				char szCharName[256];

				pCommand->GetParameter(&nRequestID,		0, MPT_INT);
				pCommand->GetParameter(&uidClanMaster,	1, MPT_UID);
				pCommand->GetParameter(szCharName,		2, MPT_STR, sizeof(szCharName) );
				pCommand->GetParameter(&bAnswer,		3, MPT_BOOL);

				OnClanAnswerSponsorAgreement(nRequestID, uidClanMaster, szCharName, bAnswer);
			}
			break;
		case MC_MATCH_CLAN_REQUEST_AGREED_CREATE_CLAN:
			{
				MUID uidPlayer = pCommand->GetSenderUID();
				char szClanName[256];

				pCommand->GetParameter(szClanName, 1, MPT_STR, sizeof(szClanName));

#if CLAN_SPONSORS_COUNT > 0
				char szSponsorNames[CLAN_SPONSORS_COUNT][256];
				char* sncv[CLAN_SPONSORS_COUNT];

				for (int i = 0; i < CLAN_SPONSORS_COUNT; i++)
				{
					pCommand->GetParameter(szSponsorNames[i],	2+i, MPT_STR, sizeof(szSponsorNames[i]) );

					sncv[i] = szSponsorNames[i];
				}

				OnClanRequestAgreedCreateClan(uidPlayer, szClanName, sncv);
#else
				OnClanRequestAgreedCreateClan(uidPlayer, szClanName, nullptr);
#endif
			}
			break;
		case MC_MATCH_CLAN_REQUEST_CLOSE_CLAN:
			{
				MUID uidClanMaster;
				char szClanName[256];

				pCommand->GetParameter(&uidClanMaster,		0, MPT_UID);
				pCommand->GetParameter(szClanName,			1, MPT_STR, sizeof(szClanName) );

				OnClanRequestCloseClan(uidClanMaster, szClanName);
			}
			break;
		case MC_MATCH_CLAN_REQUEST_JOIN_CLAN:
			{
				MUID uidClanAdmin;
				char szClanName[256], szJoiner[256];

				pCommand->GetParameter(&uidClanAdmin,	0, MPT_UID);
				pCommand->GetParameter(szClanName,		1, MPT_STR, sizeof(szClanName) );
				pCommand->GetParameter(szJoiner,		2, MPT_STR, sizeof(szJoiner) );

				OnClanRequestJoinClan(uidClanAdmin, szClanName, szJoiner);
			}
			break;
		case MC_MATCH_CLAN_ANSWER_JOIN_AGREEMENT:
			{
				MUID uidClanAdmin;
				bool bAnswer;
				char szJoiner[256];

				pCommand->GetParameter(&uidClanAdmin,	0, MPT_UID);
				pCommand->GetParameter(szJoiner,		1, MPT_STR, sizeof(szJoiner) );
				pCommand->GetParameter(&bAnswer,		2, MPT_BOOL);

				OnClanAnswerJoinAgreement(uidClanAdmin, szJoiner, bAnswer);
			}
			break;
		case MC_MATCH_CLAN_REQUEST_AGREED_JOIN_CLAN:
			{
				MUID uidClanAdmin;
				char szClanName[256], szJoiner[256];

				pCommand->GetParameter(&uidClanAdmin,	0, MPT_UID);
				pCommand->GetParameter(szClanName,		1, MPT_STR, sizeof(szClanName) );
				pCommand->GetParameter(szJoiner,		2, MPT_STR, sizeof(szJoiner) );

				OnClanRequestAgreedJoinClan(uidClanAdmin, szClanName, szJoiner);
			}
			break;
		case MC_MATCH_CLAN_REQUEST_LEAVE_CLAN:
			{
				MUID uidPlayer = pCommand->GetSenderUID();

				OnClanRequestLeaveClan(uidPlayer);
			}
			break;
		case MC_MATCH_CLAN_MASTER_REQUEST_CHANGE_GRADE:
			{
				MUID uidClanMaster;
				char szMember[256];
				int nClanGrade;

				pCommand->GetParameter(&uidClanMaster,	0, MPT_UID);
				pCommand->GetParameter(szMember,		1, MPT_STR, sizeof(szMember) );
				pCommand->GetParameter(&nClanGrade,		2, MPT_INT);

				OnClanRequestChangeClanGrade(uidClanMaster, szMember, nClanGrade);
			}
			break;
		case MC_MATCH_CLAN_ADMIN_REQUEST_EXPEL_MEMBER:
			{
				MUID uidClanAdmin;
				char szMember[256];

				pCommand->GetParameter(&uidClanAdmin,	0, MPT_UID);
				pCommand->GetParameter(szMember,		1, MPT_STR, sizeof(szMember) );

				OnClanRequestExpelMember(uidClanAdmin, szMember);
			}
			break;
		case MC_MATCH_CLAN_REQUEST_MSG:
			{
				MUID uidSender;
				char szMsg[512];

				pCommand->GetParameter(&uidSender,	0, MPT_UID);
				pCommand->GetParameter(szMsg,		1, MPT_STR, sizeof(szMsg) );

				OnClanRequestMsg(uidSender, szMsg);
			}
			break;
		case MC_MATCH_CLAN_REQUEST_MEMBER_LIST:
			{
				MUID uidChar;

				pCommand->GetParameter(&uidChar,	0, MPT_UID);

				OnClanRequestMemberList(uidChar);
			}
			break;
		case MC_MATCH_CLAN_REQUEST_CLAN_INFO:
			{
				MUID uidChar;
				char szClanName[256] = "";

				pCommand->GetParameter(&uidChar,	0, MPT_UID);
				pCommand->GetParameter(szClanName,	1, MPT_STR, sizeof(szClanName) );

				OnClanRequestClanInfo(uidChar, szClanName);

			}
			break;
		case MC_MATCH_CHANNEL_REQUEST_ALL_PLAYER_LIST:
			{
				MUID uidPlayer, uidChannel;
				u32 nPlaceFilter;
				u32 nOptions;
				uidPlayer = pCommand->GetSenderUID();

				pCommand->GetParameter(&uidChannel,		1, MPT_UID);
				pCommand->GetParameter(&nPlaceFilter,	2, MPT_UINT);
				pCommand->GetParameter(&nOptions,		3, MPT_UINT);

				OnChannelRequestAllPlayerList(uidPlayer, uidChannel, nPlaceFilter, nOptions);
			}
			break;
		case MC_MATCH_REQUEST_CHARINFO_DETAIL:
			{
				MUID uidChar;
				char szCharName[256];

				pCommand->GetParameter(&uidChar,	0, MPT_UID);
				pCommand->GetParameter(szCharName,	1, MPT_STR, sizeof(szCharName) );

				OnRequestCharInfoDetail(uidChar, szCharName);
			}
			break;


		case MC_MATCH_LADDER_REQUEST_CHALLENGE:
			{
				int nMemberCount;
				u32 nOptions;

				pCommand->GetParameter(&nMemberCount,		0, MPT_INT);
				pCommand->GetParameter(&nOptions,			1, MPT_UINT);

				MCommandParameter* pMemberNamesBlobParam = pCommand->GetParameter(2);
				if(pMemberNamesBlobParam->GetType()!=MPT_BLOB) break;
				void* pMemberNamesBlob = pMemberNamesBlobParam->GetPointer();

				OnLadderRequestChallenge(pCommand->GetSenderUID(), pMemberNamesBlob, nOptions);
			}
			break;
		case MC_MATCH_LADDER_REQUEST_CANCEL_CHALLENGE:
			{
				OnLadderRequestCancelChallenge(pCommand->GetSenderUID());
			}
			break;
		case MC_MATCH_REQUEST_PROPOSAL:
			{
				MUID uidChar;
				int nProposalMode, nRequestID, nReplierCount;

				pCommand->GetParameter(&uidChar,			0, MPT_UID);
				pCommand->GetParameter(&nProposalMode,		1, MPT_INT);
				pCommand->GetParameter(&nRequestID,			2, MPT_INT);
				pCommand->GetParameter(&nReplierCount,		3, MPT_INT);

				MCommandParameter* pReplierNamesParam = pCommand->GetParameter(4);
				if(pReplierNamesParam->GetType()!=MPT_BLOB) break;

				void* pReplierNamesBlob = pReplierNamesParam->GetPointer();

				OnRequestProposal(uidChar, nProposalMode, nRequestID, nReplierCount, pReplierNamesBlob);
			}
			break;
		case MC_MATCH_REPLY_AGREEMENT:
			{
				MUID uidProposer, uidReplier;
				char szReplierName[256];
				int nProposalMode, nRequestID;
				bool bAgreement;

				pCommand->GetParameter(&uidProposer,	0, MPT_UID);
				pCommand->GetParameter(&uidReplier,		1, MPT_UID);
				pCommand->GetParameter(szReplierName,	2, MPT_STR, sizeof(szReplierName) );
				pCommand->GetParameter(&nProposalMode,	3, MPT_INT);
				pCommand->GetParameter(&nRequestID,		4, MPT_INT);
				pCommand->GetParameter(&bAgreement,		5, MPT_BOOL);

				OnReplyAgreement(uidProposer, uidReplier, szReplierName, nProposalMode, nRequestID, bAgreement);
			}
			break;
		case MC_MATCH_CALLVOTE:
			{
				char szDiscuss[128]="";
				char szArg[256]="";

				pCommand->GetParameter(szDiscuss, 0, MPT_STR, sizeof(szDiscuss) );
				pCommand->GetParameter(szArg, 1, MPT_STR, sizeof(szArg) );
				OnVoteCallVote(pCommand->GetSenderUID(), szDiscuss, szArg);
			}
			break;
		case MC_MATCH_VOTE_YES:
			{
				OnVoteYes(pCommand->GetSenderUID());
			}
			break;
		case MC_MATCH_VOTE_NO:
			{
				OnVoteNo(pCommand->GetSenderUID());
			}
			break;
		case MC_MATCH_CLAN_REQUEST_EMBLEMURL:
			{
				int nCLID = 0;

				pCommand->GetParameter(&nCLID, 0, MPT_INT);
				OnClanRequestEmblemURL(pCommand->GetSenderUID(), nCLID);
			}
			break;

		case MC_QUEST_REQUEST_NPC_DEAD:
			{
				MUID uidKiller, uidNPC;
				MShortVector s_pos;
				pCommand->GetParameter(&uidKiller,	0, MPT_UID);
				pCommand->GetParameter(&uidNPC,		1, MPT_UID);
				pCommand->GetParameter(&s_pos,		2, MPT_SVECTOR);

				MUID uidSender = pCommand->GetSenderUID();
				MVector pos = MVector((float)s_pos.x, (float)s_pos.y, (float)s_pos.z);

				OnRequestNPCDead(uidSender, uidKiller, uidNPC, pos);
			}
			break;
		case MC_MATCH_QUEST_REQUEST_DEAD:
			{
				MUID uidVictim = pCommand->GetSenderUID();
				OnQuestRequestDead(uidVictim);
			}
			break;
		case MC_QUEST_TEST_REQUEST_NPC_SPAWN:
			{
				int nNPCType, nNPCCount;

				pCommand->GetParameter(&nNPCType,	0, MPT_INT);
				pCommand->GetParameter(&nNPCCount,	1, MPT_INT);
				MUID uidPlayer = pCommand->GetSenderUID();

				OnQuestTestRequestNPCSpawn(uidPlayer, nNPCType, nNPCCount);
			}
			break;
		case MC_QUEST_TEST_REQUEST_CLEAR_NPC:
			{
				MUID uidPlayer = pCommand->GetSenderUID();

				OnQuestTestRequestClearNPC(uidPlayer);
			}
			break;
		case MC_QUEST_TEST_REQUEST_SECTOR_CLEAR:
			{
				MUID uidPlayer = pCommand->GetSenderUID();

				OnQuestTestRequestSectorClear(uidPlayer);
			}
			break;
		case MC_QUEST_TEST_REQUEST_FINISH:
			{
				MUID uidPlayer = pCommand->GetSenderUID();

				OnQuestTestRequestQuestFinish(uidPlayer);
			}
			break;
		case MC_QUEST_REQUEST_MOVETO_PORTAL:
			{
				MUID uidPlayer = pCommand->GetSenderUID();

				OnQuestRequestMovetoPortal(uidPlayer);
			}
			break;
		case MC_QUEST_READYTO_NEWSECTOR:
			{
				MUID uidPlayer = pCommand->GetSenderUID();

				OnQuestReadyToNewSector(uidPlayer);
			}
			break;

		case MC_QUEST_PONG:
			{
				MUID uidPlayer = pCommand->GetSenderUID();

				OnQuestPong(uidPlayer);
			}
			break;

#ifdef _QUEST_ITEM
		case MC_MATCH_REQUEST_CHAR_QUEST_ITEM_LIST :
			{
				MUID uidChar;

				pCommand->GetParameter( &uidChar, 0, MPT_UID );

				OnRequestCharQuestItemList( uidChar );
			}
			break;

		case MC_MATCH_REQUEST_DROP_SACRIFICE_ITEM :
			{
				MUID	uidQuestItemOwner;
				int		nSlotIndex;
				int		nItemID;

				pCommand->GetParameter( &uidQuestItemOwner, 0, MPT_UID );
				pCommand->GetParameter( &nSlotIndex, 1, MPT_INT );
				pCommand->GetParameter( &nItemID, 2, MPT_INT );

				OnRequestDropSacrificeItemOnSlot( uidQuestItemOwner, nSlotIndex, nItemID );
			}
			break;

		case MC_MATCH_REQUEST_CALLBACK_SACRIFICE_ITEM :
			{
				MUID uidPlayer = pCommand->GetSenderUID();
				int		nSlotIndex;
				int		nItemID;

				pCommand->GetParameter( &nSlotIndex, 1, MPT_INT );
				pCommand->GetParameter( &nItemID, 2, MPT_INT );

				OnRequestCallbackSacrificeItem( uidPlayer, nSlotIndex, nItemID );
			}
			break;

		case MC_MATCH_REQUEST_BUY_QUEST_ITEM :
			{
				MUID	uidSender;
				int	 	nItemID;

				pCommand->GetParameter( &uidSender, 0, MPT_UID );
				pCommand->GetParameter( &nItemID, 1, MPT_INT );

				OnRequestBuyQuestItem( uidSender, nItemID );
			}
			break;

		case MC_MATCH_REQUEST_SELL_QUEST_ITEM :
			{
				MUID	uidSender;
				int		nItemID;
				int		nCount;

				pCommand->GetParameter( &uidSender, 0, MPT_UID );
				pCommand->GetParameter( &nItemID, 1, MPT_INT );
				pCommand->GetParameter( &nCount, 2, MPT_INT );

				OnRequestSellQuestItem( uidSender, nItemID, nCount );
			}
			break;

		case MC_QUEST_REQUEST_QL :
			{
				MUID uidSender;

				pCommand->GetParameter( &uidSender, 0, MPT_UID );

				OnRequestQL( uidSender );
			}
			break;

		case MC_MATCH_REQUEST_SLOT_INFO :
			{
				MUID uidSender;

				pCommand->GetParameter( &uidSender, 0, MPT_UID );

				OnRequestSacrificeSlotInfo( uidSender );
			}
			break;

		case MC_QUEST_STAGE_MAPSET :
			{
				MUID uidStage;
				char nMapsetID;

				pCommand->GetParameter( &uidStage,	0, MPT_UID );
				pCommand->GetParameter( &nMapsetID, 1, MPT_CHAR );

				OnQuestStageMapset(uidStage, (int)nMapsetID );
			}
			break;
		case MC_MATCH_REQUEST_MONSTER_BIBLE_INFO :
			{
				MUID uidSender;

				pCommand->GetParameter( &uidSender, 0, MPT_UID );

				OnRequestMonsterBibleInfo( uidSender );
			}
			break;

#endif

		default:
			return false;
	}

_STATUS_CMD_END;


	return true;
}
