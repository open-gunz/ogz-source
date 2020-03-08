#include "stdafx.h"
#include "MCommandRegistration.h"
#include "MMatchGlobal.h"
#include "MMatchItem.h"
#include "MBlobArray.h"
#include "reinterpret.h"

// Transmission data types for blob validation.
#include "MMatchTransDataType.h"

// MC_MATCH_OBJECT_CACHE: MMatchObjCache.
#include "MMatchObjCache.h"

// MC_PEER_BASICINFO: ZPACKEDBASICINFO.
#include "stuff.h"

// MC_MATCH_LOGIN: crypto_generichash_blake2b_BYTES.
#include "sodium.h"

void MAddSharedCommandTable(MCommandManager* CommandManager, MSharedCommandType::Type SharedType)
{
	BEGIN_CMD_DESC(CommandManager, SharedType);

	// Custom commands
	C(MC_PEER_RG_SLASH, "", "", MCDT_PEER2PEER);
		P(MPT_VECTOR, "Position");
		P(MPT_VECTOR, "Direction");
		P(MPT_INT, "Type");
	C(MC_PEER_RG_MASSIVE, "", "", MCDT_PEER2PEER);
		P(MPT_VECTOR, "Position");
		P(MPT_VECTOR, "Direction");
	C(MC_PEER_PORTAL, "", "", MCDT_PEER2PEER);
		P(MPT_INT, "Portal index");
		P(MPT_VECTOR, "Position");
		P(MPT_VECTOR, "Normal");
		P(MPT_VECTOR, "Up");
	C(MC_PEER_SPEC, "", "", MCDT_PEER2PEER);
	C(MC_PEER_MOVE_DELTA, "", "", MCDT_PEER2PEER);
		P(MPT_BLOB, "Bloooob");
	C(MC_PEER_COMPLETED_SKILLMAP, "", "", MCDT_PEER2PEER);
		P(MPT_FLOAT, "Time");
		P(MPT_STR, "Course name");
	C(MC_MATCH_REQUEST_CREATE_ACCOUNT, "", "", MCDT_MACHINE2MACHINE);
		P(MPT_STR, "Username");
		P(MPT_BLOB, "Hashed password");
		P(MPT_STR, "Email");
	C(MC_MATCH_RESPONSE_CREATE_ACCOUNT, "", "", MCDT_MACHINE2MACHINE);
		P(MPT_STR, "Message");
	C(MC_MATCH_SEND_VOICE_CHAT, "", "", MCDT_MACHINE2MACHINE);
		P(MPT_BLOB, "Encoded microphone data");
	C(MC_MATCH_RECEIVE_VOICE_CHAT, "", "", MCDT_MACHINE2MACHINE);
		P(MPT_UID, "Sender");
		P(MPT_BLOB, "Encoded microphone data");
	C(MC_PEER_SET_SWORD_COLOR, "", "", MCDT_PEER2PEER);
		P(MPT_UINT, "Color");
	C(MC_PEER_ANTILEAD_DAMAGE, "", "", MCDT_PEER2PEER);
		P(MPT_UID, "Target");
		P(MPT_USHORT, "Damage");
		P(MPT_FLOAT, "PiercingRatio");
		P(MPT_UCHAR, "DamageType");
		P(MPT_UCHAR, "WeaponType");
	C(MC_MATCH_P2P_COMMAND, "Match.P2PCommand", "Forwards Peer to Peer commands",
		MCDT_MACHINE2MACHINE | MCCT_NON_ENCRYPTED);
		// Client -> Server = Receiver
		// Server -> Client = Sender
		P(MPT_UID, "Sender/Receiver"); 
		P(MPT_BLOB, "Data");
	C(MC_MATCH_DAMAGE, "", "", MCDT_MACHINE2MACHINE);
		P(MPT_UID, "Attacker");
		P(MPT_USHORT, "Damage");
		P(MPT_FLOAT, "PiercingRatio");
		P(MPT_UCHAR, "DamageType");
		P(MPT_UCHAR, "WeaponType");
	C(MC_MATCH_UPDATE_CLIENT_SETTINGS, "", "", MCDT_MACHINE2MACHINE);
		P(MPT_BLOB, "Settings", MCPCBlobSize{sizeof(MTD_ClientSettings)});
	C(MC_MATCH_PING_LIST, "", "", MCDT_MACHINE2MACHINE);
		P(MPT_BLOB, "Ping list", MCPCBlobArraySize{ sizeof(MTD_PingInfo) });
	C(MC_PEER_BASICINFO_RG, "", "", MCDT_PEER2PEER);
		// NOTE: This is validated in the handler, by UnpackNewBasicInfo.
		P(MPT_BLOB, "BasicInfo");
	C(MC_MATCH_GUNGAME_SEND_NEW_WEAPON, "", "", MCDT_MACHINE2MACHINE);
		P(MPT_UID, "uidPlayer");
		P(MPT_UINT, "melee");
		P(MPT_UINT, "primary");
		P(MPT_UINT, "secondary");
		P(MPT_UINT, "custom1");
		P(MPT_UINT, "custom2");
	C(MC_MATCH_REQUEST_CREATE_BOT, "", "", MCDT_MACHINE2MACHINE);
	C(MC_PEER_TUNNEL_BOT_COMMAND, "", "", MCDT_PEER2PEER);
		P(MPT_UID, "Bot UID");
		P(MPT_BLOB, "Command");
	C(MC_MATCH_REQUEST_SPEC, "", "", MCDT_MACHINE2MACHINE);
		P(MPT_BOOL, "On/Off");
	C(MC_MATCH_RESPONSE_SPEC, "", "", MCDT_MACHINE2MACHINE);
		P(MPT_UID, "Target player");
		P(MPT_UINT, "Team");


	// Freestyle Gunz commands
	C(MC_MATCH_GAME_CHAT, "", "", MCDT_MACHINE2MACHINE);
		P(MPT_UID, "Sender");
		P(MPT_UINT64, "Unknown");
		P(MPT_STR, "Message");
		P(MPT_INT, "Team");


	C(MC_LOCAL_INFO, "Local.Info", "Local information", MCDT_LOCAL);
	C(MC_LOCAL_ECHO, "Local.Echo", "Local echo test", MCDT_LOCAL);
		P(MPT_STR, "Message");
	C(MC_LOCAL_LOGIN, "Local.Login", "Local Login", MCDT_LOCAL);
		P(MPT_UID, "uidComm");
		P(MPT_UID, "uidPlayer");

	C(MC_HELP, "Help", "This command", MCDT_LOCAL);
	C(MC_VERSION, "Version", "Version description", MCDT_LOCAL);

	C(MC_DEBUG_TEST, "DebugTest", "Debug Test", MCDT_MACHINE2MACHINE);

	C(MC_NET_ENUM, "Net.Enum", "Enum server list", MCDT_LOCAL);
	C(MC_NET_CONNECT, "Net.Connect", "Connect to server", MCDT_LOCAL);
		P(MPT_STR, "Address");
	C(MC_NET_DISCONNECT, "Net.Disconnect", "Disconnect to server", MCDT_LOCAL);
	C(MC_NET_CLEAR, "Net.Clear", "Clear Connection", MCDT_LOCAL);
		P(MPT_UID, "uid");
	C(MC_NET_CHECKPING, "Net.CheckPing", "Check ping time", MCDT_LOCAL);
		P(MPT_UID, "uid");
	C(MC_NET_PING, "Net.Ping", "Ping", MCDT_MACHINE2MACHINE | MCCT_NON_ENCRYPTED);
		P(MPT_UINT, "TimeStamp");
	C(MC_NET_PONG, "Net.Pong", "Pong", MCDT_MACHINE2MACHINE | MCCT_NON_ENCRYPTED);
		P(MPT_UINT, "TimeStamp");

	C(MC_NET_ONCONNECT, "Net.OnConnect", "On Connect", MCDT_LOCAL);
	C(MC_NET_ONDISCONNECT, "Net.OnDisConnect", "On Disconnect", MCDT_LOCAL);
	C(MC_NET_ONERROR, "Net.OnError", "On Error", MCDT_LOCAL);
		P(MPT_INT, "ErrorCode");

	C(MC_NET_CONNECTTOZONESERVER, "Net.ConnectToZoneServer", "Connect to zone-server", MCDT_LOCAL);
	

	C(MC_NET_REQUEST_INFO, "Net.RequestInfo", "Request Net information", MCDT_MACHINE2MACHINE);
	C(MC_NET_RESPONSE_INFO, "Net.ResponseInfo", "Response Net information", MCDT_MACHINE2MACHINE);
		P(MPT_STR, "Information");
	C(MC_NET_ECHO, "Net.Echo", "Echo test", MCDT_MACHINE2MACHINE);
		P(MPT_STR, "Message");

	C(MC_MATCH_ANNOUNCE, "Match.Announce", "Announce Server Message", MCDT_MACHINE2MACHINE | MCCT_NON_ENCRYPTED );
		P(MPT_UINT, "Type");
		P(MPT_STR, "Msg");

	if(IsTypeAnyOf(MatchServer, Client)){
		C(MC_CLOCK_SYNCHRONIZE, "Clock.Synchronize", "Synchronize Clock", MCDT_MACHINE2MACHINE);
			P(MPT_UINT, "GlobalClock(msec);");
		C(MC_MATCH_LOGIN, "Match.Login", "Login Match Server", MCDT_MACHINE2MACHINE);
			P(MPT_STR, "UserID");
			P(MPT_BLOB, "Hashed password", MCPCBlobSize{ crypto_generichash_blake2b_BYTES });
			P(MPT_INT, "CommandVersion");
			P(MPT_UINT, "nChecksumPack");
			P(MPT_UINT, "ClientVersionMajor");
			P(MPT_UINT, "ClientVersionMinor");
			P(MPT_UINT, "ClientVersionPatch");
			P(MPT_UINT, "ClientVersionRevision");
		C(MC_MATCH_RESPONSE_LOGIN_FAILED, "", "", MCDT_MACHINE2MACHINE);
			P(MPT_STR, "Reason");
		C(MC_MATCH_RESPONSE_LOGIN, "Match.ResponseLogin", "Response Login", MCDT_MACHINE2MACHINE);
			P(MPT_INT,	"Result");
			P(MPT_STR,	"ServerName");
			P(MPT_CHAR, "ServerMode");
			P(MPT_STR,	"AccountID");
			P(MPT_UCHAR, "UGradeID");
			P(MPT_UCHAR, "PGradeID");
			P(MPT_UID,	"uidPlayer");
			P(MPT_STR,	"RandomValue");
			// NOTE: Unused.
			P(MPT_BLOB,	"EncryptMsg");

		C(MC_MATCH_RESPONSE_RESULT, "Match.Response.Result", "Response Result", MCDT_MACHINE2MACHINE);
			P(MPT_INT, "Result");
		C(MC_MATCH_LOGIN_NETMARBLE, "Match.LoginNetmarble", "Login from Netmarble", MCDT_MACHINE2MACHINE);
			P(MPT_STR, "CPCookie");
			P(MPT_STR, "SpareParam");
			P(MPT_INT, "CommandVersion");
			P(MPT_UINT, "nChecksumPack");
		C(MC_MATCH_LOGIN_NETMARBLE_JP, "Match.LoginNetmarbleJP", "Login from Netmarble Japan", MCDT_MACHINE2MACHINE);
			P(MPT_STR, "LoginID");
			P(MPT_STR, "LoginPW");
			P(MPT_INT, "CommandVersion");
			P(MPT_UINT, "nChecksumPack");
		C(MC_MATCH_LOGIN_FROM_DBAGENT, "Match.LoginFromDBAgent", "Login from DBAgent", MCDT_LOCAL);
			P(MPT_UID, "CommUID");
			P(MPT_STR, "LoginID");
			P(MPT_STR, "Name");
			P(MPT_INT, "Sex");
			P(MPT_BOOL, "bFreeLoginIP");
			P(MPT_UINT, "nChecksumPack");
		C(MC_MATCH_LOGIN_FROM_DBAGENT_FAILED, "Match.LoginFailedFromDBAgent", "Login Failed from DBAgent", MCDT_LOCAL);
			P(MPT_UID, "CommUID");
			P(MPT_INT, "Result");
		C(MC_MATCH_FIND_HACKING, "Match.FinH", "FinH", MCDT_MACHINE2MACHINE);
		C( MC_MATCH_DISCONNMSG, "MC_MATCH_DISCONNMSG", "disconnect reason", MCDT_MACHINE2MACHINE );
			P(MPT_UINT, "message id");


		C(MC_MATCH_OBJECT_CACHE, "Match.ObjectCache", "Match Object Cache", MCDT_MACHINE2MACHINE);
			P(MPT_UCHAR, "Type");
			P(MPT_BLOB, "ObjectCache", MCPCBlobArraySize{ sizeof(MMatchObjCache) });
		C(MC_MATCH_BRIDGEPEER, "Match.BridgePeer", "Match BridgePeer", MCDT_MACHINE2MACHINE | MCCT_NON_ENCRYPTED);
			P(MPT_UID, "uidPlayer");
			P(MPT_UINT, "dwIP");
			P(MPT_UINT, "nPort");
		C(MC_MATCH_BRIDGEPEER_ACK, "Match.BridgePeerACK", "ACK for BridgePeer", MCDT_MACHINE2MACHINE);
			P(MPT_UID, "uidPlayer");
			P(MPT_INT, "nCode");

		C(MC_MATCH_REQUEST_RECOMMANDED_CHANNEL, "MatchServer.RequestRecommandedChannel", "Request recommanded channel", MCDT_MACHINE2MACHINE);
        C(MC_MATCH_RESPONSE_RECOMMANDED_CHANNEL, "MatchServer.ResponseRecommandedChannel", "Response recommanded channel", MCDT_MACHINE2MACHINE);
			P(MPT_UID, "uid");
		C(MC_MATCH_CHANNEL_REQUEST_JOIN, "Channel.Join", "Join a Channel", MCDT_MACHINE2MACHINE);
			P(MPT_UID, "uidPlayer");
			P(MPT_UID, "uidChannel");
		C(MC_MATCH_CHANNEL_RESPONSE_JOIN, "Channel.ResponseJoin", "Response Join a Channel", MCDT_MACHINE2MACHINE);
			P(MPT_UID, "uidChannel");
			P(MPT_INT, "ChannelType");
			P(MPT_STR, "ChannelName");
		C(MC_MATCH_CHANNEL_REQUEST_JOIN_FROM_NAME, "Channel.RequestJoinFromName", "Join a Channel From Name", MCDT_MACHINE2MACHINE);
			P(MPT_UID, "uidPlayer");
			P(MPT_INT, "nChannelType", MCPCMinMax{ 0, MCHANNEL_TYPE_MAX - 1 });
			P(MPT_STR, "ChannelName");
		C(MC_MATCH_CHANNEL_LEAVE, "Channel.Leave", "Leave Channel", MCDT_MACHINE2MACHINE);
			P(MPT_UID, "uidPlayer");
			P(MPT_UID, "uidChannel");
		C(MC_MATCH_CHANNEL_LIST_START, "Channel.ListStart", "Channel List transmit start", MCDT_MACHINE2MACHINE);
			P(MPT_UID, "uidPlayer");
			P(MPT_INT, "nChannelType", MCPCMinMax{ 0, MCHANNEL_TYPE_MAX - 1 });
		C(MC_MATCH_CHANNEL_LIST_STOP, "Channel.ListStop", "Channel List transmit stop", MCDT_MACHINE2MACHINE);
			P(MPT_UID, "uidPlayer");
		C(MC_MATCH_CHANNEL_LIST, "Channel.List", "Channel List", MCDT_MACHINE2MACHINE);
			P(MPT_BLOB, "ChannelList", MCPCBlobArraySize{ sizeof(MCHANNELLISTNODE) });

		C(MC_MATCH_CHANNEL_REQUEST_CHAT, "Channel.Request.Chat",
			"Request Chat to Channel", MCDT_MACHINE2MACHINE);
			P(MPT_UID, "uidPlayer");
			P(MPT_UID, "uidChannel");
			P(MPT_STR, "Chat");

		C(MC_MATCH_CHANNEL_CHAT, "Channel.Chat",
			"Chat to Channel", MCDT_MACHINE2MACHINE);
			P(MPT_UID, "uidChannel");
			P(MPT_STR, "PlayerName");
			P(MPT_STR, "Chat");
			P(MPT_INT, "nGrade");

		C(MC_MATCH_CHANNEL_REQUEST_RULE, "Channel.Request.Rule",
			"Request the Channel Rule", MCDT_MACHINE2MACHINE);
			P(MPT_UID, "uidChannel");
		C(MC_MATCH_CHANNEL_RESPONSE_RULE, "Channel.Response.Rule",
			"Response the Channel Rule", MCDT_MACHINE2MACHINE);
			P(MPT_UID, "uidChannel");
			P(MPT_STR, "RuleName");

		C(MC_MATCH_CHANNEL_REQUEST_ALL_PLAYER_LIST, "Channel.RequestAllPlayerList",
			"Request Channel All Player List", MCDT_MACHINE2MACHINE);
			P(MPT_UID, "uidPlayer");
			P(MPT_UID, "uidChannel");
			P(MPT_UINT, "PlaceFilter");
			P(MPT_UINT, "Options");
		C(MC_MATCH_CHANNEL_RESPONSE_ALL_PLAYER_LIST, "Channel.ResponseAllPlayerList",
			"Response Channel All Player List", MCDT_MACHINE2MACHINE);
			P(MPT_UID, "uidChannel");
			P(MPT_BLOB, "PlayerList", MCPCBlobArraySize{ sizeof(MTD_ChannelPlayerListNode) });

		C(MC_MATCH_STAGE_CREATE, "Stage.Create", "Create a Stage", MCDT_MACHINE2MACHINE);
			P(MPT_UID, "uidChar");
			P(MPT_STR, "StageName");
			P(MPT_BOOL, "IsPrivate");
			P(MPT_STR, "Password");
		C(MC_MATCH_REQUEST_STAGE_JOIN, "Stage.RequestJoin", "Request Join a Stage", MCDT_MACHINE2MACHINE);
			P(MPT_UID, "uidPlayer");
			P(MPT_UID, "uidStage");
		C(MC_MATCH_REQUEST_PRIVATE_STAGE_JOIN, "Stage.RequestPrivateJoin", "Request Join a Private Stage", MCDT_MACHINE2MACHINE);
			P(MPT_UID, "uidPlayer");
			P(MPT_UID, "uidStage");
			P(MPT_STR, "Password");

		C(MC_MATCH_STAGE_JOIN, "Stage.Join", "Join a Stage", MCDT_MACHINE2MACHINE);
			P(MPT_UID, "uidPlayer");
			P(MPT_UID, "uidStage");
			P(MPT_UINT, "nRoomNo");
			P(MPT_STR, "StageName");
		C(MC_MATCH_STAGE_LEAVE, "Stage.Leave", "Leave Stage", MCDT_MACHINE2MACHINE);
			P(MPT_UID, "uidPlayer");
			P(MPT_UID, "uidStage");
		C(MC_MATCH_STAGE_REQUEST_PLAYERLIST, "Stage.Request.PlayerList", "Requst PlayerList from the Stage", MCDT_MACHINE2MACHINE);
			P(MPT_UID, "uidStage");
		C(MC_MATCH_STAGE_FOLLOW, "Stage.Follow", "Follow User to Stage", MCDT_MACHINE2MACHINE);
			P(MPT_STR, "strTarget");
		C(MC_MATCH_RESPONSE_STAGE_FOLLOW, "Stage.Response.Follow", "Response Follow User to Stage", MCDT_MACHINE2MACHINE);
			P(MPT_INT, "Result");

		C(MC_MATCH_RESPONSE_STAGE_JOIN, "Stage.ResponseJoin", "Response Join a Stage", MCDT_MACHINE2MACHINE);
			P(MPT_INT, "Result");

		C(MC_MATCH_STAGE_REQUIRE_PASSWORD, "Stage.RequirePassword", "Require password", MCDT_MACHINE2MACHINE);
			P(MPT_UID, "uidStage");
			P(MPT_STR, "StageName");

		struct RuleInfoCondition : public MCommandParamCondition
		{
			virtual bool Check(MCommandParameter* pCP) override
			{
				assert(pCP->GetType() == MPT_BLOB);

				auto* Blob = static_cast<MCmdParamBlob*>(pCP);
				const auto* Ptr = static_cast<const u8*>(Blob->GetPointer());
				const auto Size = Blob->GetPayloadSize();

				if (!MValidateBlobArraySize(Ptr, Size))
					return false;

				const auto ElementSize = MGetBlobArrayElementSize(Ptr);
				const auto ArrayCount = MGetBlobArrayCount(Ptr);
				if (ElementSize < sizeof(MTD_RuleInfo))
					return false;

				if (ArrayCount != 1)
					return ArrayCount == 0;

				auto&& RuleInfo = reinterpret_pointee<MTD_RuleInfo>(MGetBlobArrayElement(Ptr, 0));

				if (RuleInfo.nRuleType < MMATCH_GAMETYPE_DEATHMATCH_SOLO ||
					RuleInfo.nRuleType >= MMATCH_GAMETYPE_MAX)
					return false;

				switch (MMATCH_GAMETYPE(RuleInfo.nRuleType))
				{
				case MMATCH_GAMETYPE_ASSASSINATE:
					return ElementSize == sizeof(MTD_RuleInfo_Assassinate);
				case MMATCH_GAMETYPE_BERSERKER:
					return ElementSize == sizeof(MTD_RuleInfo_Berserker);
				default:
					return true;
				}
			}
		};

		C(MC_MATCH_REQUEST_GAME_INFO, "RequestGameInfo", "Request Game Info", MCDT_MACHINE2MACHINE);
			P(MPT_UID, "uidChar");
			P(MPT_UID, "uidStage");
		C(MC_MATCH_RESPONSE_GAME_INFO, "ResponseGameInfo", "Response Game Info", MCDT_MACHINE2MACHINE);
			P(MPT_UID, "uidStage");
			P(MPT_BLOB, "GameInfo", MCPCBlobArraySize{ sizeof(MTD_GameInfo), 1, 1 });
			P(MPT_BLOB, "RuleInfo", RuleInfoCondition{});
			P(MPT_BLOB, "PlayerInfo", MCPCBlobArraySize{ sizeof(MTD_GameInfoPlayerItem) });
		C(MC_MATCH_RESPONSE_STAGE_CREATE, "Stage.ResponseCreate", "Response Create a Stage", MCDT_MACHINE2MACHINE);
			P(MPT_INT, "Result");

		C(MC_MATCH_STAGE_REQUEST_ENTERBATTLE, "Stage.Request.EnterBattle", "Request Enter Stage Battle", MCDT_MACHINE2MACHINE);
			P(MPT_UID, "uidPlayer");
			P(MPT_UID, "uidStage");
		C(MC_MATCH_STAGE_ENTERBATTLE, "Stage.EnterBattle", "Enter Stage Battle", MCDT_MACHINE2MACHINE);
			P(MPT_UCHAR, "Param");
			P(MPT_BLOB, "CharData", MCPCBlobArraySize{ sizeof(MTD_PeerListNode), 1, 1 });

		C(MC_MATCH_STAGE_LEAVEBATTLE, "Stage.LeaveBattle", "Leave Stage Battle", MCDT_MACHINE2MACHINE);
			P(MPT_UID, "uidPlayer");
			P(MPT_UID, "uidStage");
		C(MC_MATCH_STAGE_START, "Stage.Start", "Start Stage", MCDT_MACHINE2MACHINE);
			P(MPT_UID, "uidPlayer");
			P(MPT_UID, "uidStage");
			P(MPT_INT, "nCountdown");
		C(MC_MATCH_STAGE_MAP, "Stage.Map", "Change Map", MCDT_MACHINE2MACHINE);
			P(MPT_UID, "uidStage");
			P(MPT_STR, "MapName");
		C(MC_MATCH_STAGE_CHAT, "Stage.Chat", "Chat to Stage", MCDT_MACHINE2MACHINE);
			P(MPT_UID, "uidPlayer");
			P(MPT_UID, "uidStage");
			P(MPT_STR, "Chat");

		C(MC_MATCH_STAGE_REQUEST_QUICKJOIN, "Stage.RequestQuickJoin", "Stage Request QuickJoin", MCDT_MACHINE2MACHINE);
			P(MPT_UID, "uidPlayer");
			P(MPT_BLOB, "Param", MCPCBlobArraySize{ sizeof(MTD_QuickJoinParam), 1, 1 });
		C(MC_MATCH_STAGE_RESPONSE_QUICKJOIN, "Stage.ResponseQuickJoin", "Stage Response QuickJoin", MCDT_MACHINE2MACHINE);
			P(MPT_INT, "Result");
			P(MPT_UID, "uidStage");
		C(MC_MATCH_STAGE_GO, "Stage.StageGo", "Stage Go", MCDT_MACHINE2MACHINE);
			P(MPT_UINT, "RoomNo");


		C(MC_MATCH_STAGE_PLAYER_STATE, "Stage.State", "Change State In Stage", MCDT_MACHINE2MACHINE);
			P(MPT_UID, "uidPlayer");
			P(MPT_UID, "uidStage");
			P(MPT_INT, "nState", MCPCMinMax{ 0, MOSS_END - 1 });
		C(MC_MATCH_STAGE_TEAM, "Stage.Team", "Change Team", MCDT_MACHINE2MACHINE);
			P(MPT_UID, "uidPlayer");
			P(MPT_UID, "uidStage");
			P(MPT_UINT, "nTeam");
		C(MC_MATCH_STAGE_MASTER, "Stage.Master", "Set Master", MCDT_MACHINE2MACHINE);
			P(MPT_UID, "uidStage");
			P(MPT_UID, "uidPlayer");
		C(MC_MATCH_STAGE_LIST_START, "Stage.ListStart",
			"Stage List transmit start", MCDT_MACHINE2MACHINE); 
		C(MC_MATCH_STAGE_LIST_STOP, "Stage.ListStop",
			"Stage List transmit stop", MCDT_MACHINE2MACHINE); 
		C(MC_MATCH_STAGE_LIST, "Stage.List", "Stage List", MCDT_MACHINE2MACHINE); 
			P(MPT_CHAR, "PrevStageListCount");
			P(MPT_CHAR, "NextStageListCount");
			P(MPT_BLOB, "StageList", MCPCBlobArraySize{ sizeof(MTD_StageListNode) });
		C(MC_MATCH_REQUEST_STAGE_LIST, "Stage.RequestStageList",
			"Request Stage List", MCDT_MACHINE2MACHINE);
			P(MPT_UID, "uidPlayer");
			P(MPT_UID, "uidChannel");
			P(MPT_INT, "StageCursor");
			
		C(MC_MATCH_CHANNEL_REQUEST_PLAYER_LIST, "Channel.RequestPlayerList",
			"Request Channel Player List", MCDT_MACHINE2MACHINE);
			P(MPT_UID, "uidPlayer");
			P(MPT_UID, "uidChannel");
			P(MPT_INT, "PlayerListPage");
		C(MC_MATCH_CHANNEL_RESPONSE_PLAYER_LIST, "Channel.ResponsePlayerList",
			"Response Channel Player List", MCDT_MACHINE2MACHINE);
			P(MPT_UCHAR, "TotalPlayerCount");
			P(MPT_UCHAR, "PlayerListPage");
			P(MPT_BLOB, "PlayerList", MCPCBlobArraySize{ sizeof(MTD_ChannelPlayerListNode) });

		C(MC_MATCH_REQUEST_STAGESETTING, "Stage.RequestStageSetting", "Request stage setting", MCDT_MACHINE2MACHINE);
			P(MPT_UID, "uidStage");
		C(MC_MATCH_RESPONSE_STAGESETTING, "Stage.ResponseStageSetting", "Response stage setting", MCDT_MACHINE2MACHINE);
			P(MPT_UID, "uidStage");
			P(MPT_BLOB, "StageSetting", MCPCBlobArraySize{ sizeof(MSTAGE_SETTING_NODE), 1, 1 });
			P(MPT_BLOB, "CharSetting", MCPCBlobArraySize{ sizeof(MSTAGE_CHAR_SETTING_NODE) });
			P(MPT_INT, "StageState");
			P(MPT_UID, "uidMaster");

		C(MC_MATCH_STAGESETTING, "Stage.StageSetting", "Setting up Stage", MCDT_MACHINE2MACHINE);
			P(MPT_UID, "uidPlayer");
			P(MPT_UID, "uidStage");
			P(MPT_BLOB, "StageSetting", MCPCBlobArraySize{ sizeof(MSTAGE_SETTING_NODE), 1, 1 });
		C(MC_MATCH_STAGE_LAUNCH, "Stage.Launch", "Launch Stage", MCDT_MACHINE2MACHINE);
			P(MPT_UID, "uidStage");
			P(MPT_STR, "MapName");
		C(MC_MATCH_STAGE_FINISH_GAME, "Stage.Finish", "Finish Stage", MCDT_MACHINE2MACHINE);
			P(MPT_UID, "uidStage");

		C(MC_MATCH_REQUEST_PEERLIST, "Stage.RequestPeerList", "Request peer list", MCDT_MACHINE2MACHINE);
			P(MPT_UID, "uidPlayer");
			P(MPT_UID, "uidStage");
		C(MC_MATCH_RESPONSE_PEERLIST, "Stage.ResponsePeerList", "Response peer list", MCDT_MACHINE2MACHINE);
			P(MPT_UID, "uidStage");
			P(MPT_BLOB, "PeerList", MCPCBlobArraySize{ sizeof(MTD_PeerListNode) });
		C(MC_MATCH_LOADING_COMPLETE, "Loading.Complete", "Loading Complete", MCDT_MACHINE2MACHINE);
			P(MPT_UID, "ChrUID");
			P(MPT_INT, "Percent");
		C(MC_MATCH_REQUEST_PEER_RELAY, "Match.RequestPeerRelay", "Request Peer Relay", MCDT_MACHINE2MACHINE);
			P(MPT_UID, "PlayerUID");
			P(MPT_UID, "PeerCharUID");
		C(MC_MATCH_RESPONSE_PEER_RELAY, "Match.ResponsePeerRelay", "Response Peer Relay", MCDT_MACHINE2MACHINE);
			P(MPT_UID, "PeerCharUID");
		C(MC_MATCH_GAME_ROUNDSTATE, "Stage.RoundState", "Sync State of a StageRound", MCDT_MACHINE2MACHINE);
			P(MPT_UID, "uidStage");
			P(MPT_INT, "nRound");
			P(MPT_INT, "nState");
			P(MPT_INT, "nArg");
		C(MC_MATCH_GAME_KILL, "Game.Kill", "Object Die", MCDT_MACHINE2MACHINE);
			P(MPT_UID, "Attacker");
		C(MC_MATCH_GAME_REQUEST_SPAWN, "Game.Requst.Spawn", "Request Spawn", MCDT_MACHINE2MACHINE);
			P(MPT_UID, "CharUID");
			P(MPT_POS, "Position");
			P(MPT_DIR, "Direction");
		C(MC_MATCH_GAME_RESPONSE_SPAWN, "Game.Response.Spawn", "Response Spawn", MCDT_MACHINE2MACHINE);
			P(MPT_UID, "CharUID");
			P(MPT_SVECTOR, "Position");
			P(MPT_SVECTOR, "Direction");

		C(MC_MATCH_GAME_LEVEL_UP, "Game.LevelUp", "Game.LevelUp", MCDT_MACHINE2MACHINE);
			P(MPT_UID, "Player");
			P(MPT_INT, "Level");
		C(MC_MATCH_GAME_LEVEL_DOWN, "Game.LevelDown", "Game.LevelDown", MCDT_MACHINE2MACHINE);
			P(MPT_UID, "Player");
			P(MPT_INT, "Level");
		C(MC_MATCH_GAME_DEAD, "Game.Dead", "Game.Dead", MCDT_MACHINE2MACHINE);
			P(MPT_UID, "Attacker");
			P(MPT_UINT, "AttackerArg");
			P(MPT_UID, "Victim");
			P(MPT_UINT, "VictimArg");
		C(MC_MATCH_GAME_TEAMBONUS, "Game.TeamBonus", "Game.TeamBonus", MCDT_MACHINE2MACHINE);
			P(MPT_UID, "CharUID");
			P(MPT_UINT, "ExpArg");

		C(MC_MATCH_GAME_REQUEST_TIMESYNC, "Game.RequestTimeSync",
			"Request TimeSync for Game", MCDT_MACHINE2MACHINE);
			P(MPT_UINT, "LocalTimeStamp");
		C(MC_MATCH_GAME_RESPONSE_TIMESYNC, "Game.ResponseTimeSync",
			"Response TimeSync for Game", MCDT_MACHINE2MACHINE);
			P(MPT_UINT, "LocalTimeStamp");
			P(MPT_UINT, "GlobalTimeStamp");
		C(MC_MATCH_GAME_REPORT_TIMESYNC, "Game.ReportTimeSync",
			"Report TimeSync for Verify SpeedHack", MCDT_MACHINE2MACHINE);
			P(MPT_UINT, "LocalTimeStamp");
			P(MPT_UINT, "MemoryChecksum");

		C(MC_MATCH_STAGE_REQUEST_FORCED_ENTRY, "Stage.RequestForcedEntry",
			"Request Forced Entry", MCDT_MACHINE2MACHINE);
			P(MPT_UID, "uidChar");
			P(MPT_UID, "uidStage");
		C(MC_MATCH_STAGE_RESPONSE_FORCED_ENTRY, "Stage.ResponseForcedEntry",
			"Response Forced Entry", MCDT_MACHINE2MACHINE);
			P(MPT_INT, "Result");

		C(MC_MATCH_ROUND_FINISHINFO, "Stage.RoundFinishInfo",
			"Update Round Finished Info", MCDT_MACHINE2MACHINE);
			P(MPT_UID, "uidStage");
			P(MPT_UID, "uidChar");
			P(MPT_BLOB, "PeerInfo", MCPCBlobArraySize{ sizeof(MTD_RoundPeerInfo) });
			P(MPT_BLOB, "KillInfo", MCPCBlobArraySize{ sizeof(MTD_RoundKillInfo) });

		C(MC_MATCH_NOTIFY, "Match.Notify", "Notify Message", MCDT_MACHINE2MACHINE);
			P(MPT_UINT, "nMsgID");

		C(MC_MATCH_USER_WHISPER, "Match.Whisper", "Whisper Message to a User", MCDT_MACHINE2MACHINE);
			P(MPT_STR, "SenderName");
			P(MPT_STR, "TargetName");
			P(MPT_STR, "Message");
		C(MC_MATCH_USER_WHERE, "Match.Where", "Ask Player Location", MCDT_MACHINE2MACHINE);
			P(MPT_STR, "TargetName");			
		C(MC_MATCH_USER_OPTION, "Match.UserOption", "Set User Option", MCDT_MACHINE2MACHINE);
			P(MPT_UINT, "OptionFlags");
		C(MC_MATCH_CHATROOM_CREATE, "ChatRoom.Create", "Create a ChatRoom", MCDT_MACHINE2MACHINE);
			P(MPT_UID, "uidPlayer");
			P(MPT_STR, "ChatRoomName");
		C(MC_MATCH_CHATROOM_JOIN, "ChatRoom.Join", "Join a ChatRoom", MCDT_MACHINE2MACHINE);
			P(MPT_STR, "PlayerName");
			P(MPT_STR, "ChatRoomName");
		C(MC_MATCH_CHATROOM_LEAVE, "ChatRoom.Leave", "Leave a Chat Room", MCDT_MACHINE2MACHINE);
			P(MPT_STR, "PlayerName");
			P(MPT_STR, "ChatRoomName");
		C(MC_MATCH_CHATROOM_SELECT_WRITE, "ChatRoom.SelectWrite", "Select ChatRoom to Write", MCDT_MACHINE2MACHINE);
			P(MPT_STR, "ChatRoomName");
		C(MC_MATCH_CHATROOM_INVITE, "ChatRoom.Invite", "Invite a user to ChatRoom", MCDT_MACHINE2MACHINE);
			P(MPT_STR, "SenderName");
			P(MPT_STR, "TargetName");
			P(MPT_STR, "ChatRoomName");
		C(MC_MATCH_CHATROOM_CHAT, "ChatRoom.Chat", "ChatRoom Chat", MCDT_MACHINE2MACHINE);
			P(MPT_STR, "ChatRoomName");
			P(MPT_STR, "SenderName");
			P(MPT_STR, "Message");
		
#define SIZEOF_GUIDACKMSG 20
		C(MC_MATCH_REQUEST_ACCOUNT_CHARLIST, "Match.RequestAccountCharList",
			"Request Account Character List", MCDT_MACHINE2MACHINE);
			P(MPT_STR, "SKey");
			P(MPT_BLOB, "EMsg", MCPCBlobArraySize{ 1, SIZEOF_GUIDACKMSG, SIZEOF_GUIDACKMSG });
		C(MC_MATCH_RESPONSE_ACCOUNT_CHARLIST, "Match.ResponseAccountCharList",
			"Response Account Character List", MCDT_MACHINE2MACHINE);
			P(MPT_BLOB, "CharList", MCPCBlobArraySize{ sizeof(MTD_AccountCharInfo) });
		C(MC_MATCH_REQUEST_ACCOUNT_CHARINFO, "Match.RequestAccountCharInfo",
			"Request Account Character Info", MCDT_MACHINE2MACHINE);
			P(MPT_CHAR, "CharNum", MCPCMinMax{ 0, MAX_CHAR_COUNT - 1 });
		C(MC_MATCH_RESPONSE_ACCOUNT_CHARINFO, "Match.ResponseAccountCharInfo",
			"Response Account Character Info", MCDT_MACHINE2MACHINE);
			P(MPT_CHAR, "CharNum", MCPCMinMax{ 0, MAX_CHAR_COUNT - 1 });
			P(MPT_BLOB, "CharInfo", MCPCBlobArraySize{ sizeof(MTD_CharInfo), 1, 1 });
		C(MC_MATCH_REQUEST_SELECT_CHAR, "Match.RequestSelectChar",
			"Request Select Character", MCDT_MACHINE2MACHINE);
			P(MPT_UID, "uid");
			P(MPT_UINT, "CharIndex", MCPCMinMax{ 0, MAX_CHAR_COUNT - 1 });
		C(MC_MATCH_RESPONSE_SELECT_CHAR, "Match.ResponseSelectChar",
			"Response Select Character", MCDT_MACHINE2MACHINE);
			P(MPT_INT, "Result");
			P(MPT_BLOB, "CharInfo", MCPCBlobArraySize{ sizeof(MTD_CharInfo), 1, 1 });
			P(MPT_BLOB, "MyExtraCharInfo", MCPCBlobArraySize{ sizeof(MTD_MyExtraCharInfo), 1, 1 });
		C(MC_MATCH_REQUEST_MYCHARINFO, "Match.RequestCharInfo",
			"Request Character Info", MCDT_MACHINE2MACHINE);
			P(MPT_UID, "uid");
			P(MPT_UINT, "CharIndex");
		C(MC_MATCH_RESPONSE_MYCHARINFO, "Match.ResponseCharInfo",
			"Response Character Info", MCDT_MACHINE2MACHINE);
			P(MPT_UID, "uid");
			P(MPT_BLOB, "CharInfo", MCPCBlobArraySize{ sizeof(MTD_CharInfo), 1, 1 });
		C(MC_MATCH_REQUEST_DELETE_CHAR, "Match.RequestDeleteChar",
			"Request Delete Character", MCDT_MACHINE2MACHINE);
			P(MPT_UID, "uid");
			P(MPT_UINT, "CharIndex", MCPCMinMax{ 0, MAX_CHAR_COUNT - 1 });
			P(MPT_STR, "CharName");
		C(MC_MATCH_RESPONSE_DELETE_CHAR, "Match.ResponseDeleteChar",
			"Response Delete Character", MCDT_MACHINE2MACHINE);
			P(MPT_INT, "Result");
		C(MC_MATCH_REQUEST_CREATE_CHAR, "Match.RequestCreateChar",
			"Request Create Character", MCDT_MACHINE2MACHINE);
			P(MPT_UID, "uid");
			P(MPT_UINT, "CharIndex", MCPCMinMax{ 0, MAX_CHAR_COUNT - 1 });
			P(MPT_STR, "Name");
			P(MPT_UINT, "Sex");
			P(MPT_UINT, "Hair");
			P(MPT_UINT, "Face");
			P(MPT_UINT, "Costume");
		C(MC_MATCH_RESPONSE_CREATE_CHAR, "Match.ResponseCreateChar", "Response Create Character", MCDT_MACHINE2MACHINE);
			P(MPT_INT, "Result");
			P(MPT_STR, "CharName");
		C(MC_MATCH_REQUEST_COPY_TO_TESTSERVER, "Match.RequestCopyToTestServer", "Request Copy To TestServer", MCDT_MACHINE2MACHINE);
			P(MPT_UID, "uidChar");
		C(MC_MATCH_RESPONSE_COPY_TO_TESTSERVER, "Match.ResponseCopyToTestServer", "Response Copy To TestServer", MCDT_MACHINE2MACHINE);
			P(MPT_INT, "Result");
		C(MC_MATCH_REQUEST_CHARINFO_DETAIL, "Match.RequestCharInfoDetail", "Request Character Info Detail", MCDT_MACHINE2MACHINE);
			P(MPT_UID, "uid");
			P(MPT_STR, "CharName");
		C(MC_MATCH_RESPONSE_CHARINFO_DETAIL, "Match.ResponseCharInfoDetail", "Response Character Info Detail", MCDT_MACHINE2MACHINE);
			P(MPT_BLOB, "CharInfo", MCPCBlobArraySize{ sizeof(MTD_CharInfo_Detail), 1, 1 });

		// NOTE: Unused.
		C(MC_MATCH_REQUEST_SIMPLE_CHARINFO, "Match.RequestSimpleCharInfo", "Request Simple CharInfo", MCDT_MACHINE2MACHINE);
			P(MPT_UID, "uid");
		// NOTE: Unused.
		C(MC_MATCH_RESPONSE_SIMPLE_CHARINFO, "Match.ResponseSimpleCharInfo", "Response Simple CharInfo", MCDT_MACHINE2MACHINE);
			P(MPT_UID, "uid");
			P(MPT_BLOB, "SimpleCharInfo");
		C(MC_MATCH_REQUEST_MY_SIMPLE_CHARINFO, "Match.RequestMySimpleCharInfo", "Request My Simple CharInfo", MCDT_MACHINE2MACHINE);
			P(MPT_UID, "uidChar");
		C(MC_MATCH_RESPONSE_MY_SIMPLE_CHARINFO, "Match.ResponseMySimpleCharInfo", "Response My Simple CharInfo", MCDT_MACHINE2MACHINE);
			P(MPT_BLOB, "MySimpleCharInfo", MCPCBlobArraySize{ sizeof(MTD_MySimpleCharInfo), 1, 1 });


		C(MC_MATCH_REQUEST_BUY_ITEM, "Match.RequestBuyItem", "Request Buy Item", MCDT_MACHINE2MACHINE);
			P(MPT_UID, "uidChar");
			P(MPT_UINT, "ItemID");
		C(MC_MATCH_RESPONSE_BUY_ITEM, "Match.ResponseBuyItem", "Response Buy Item", MCDT_MACHINE2MACHINE);
			P(MPT_INT, "result");
		C(MC_MATCH_REQUEST_SELL_ITEM, "Match.RequestSellItem", "Request Sell Item", MCDT_MACHINE2MACHINE);
			P(MPT_UID, "uidChar");
			P(MPT_UID, "uidItem");
		C(MC_MATCH_RESPONSE_SELL_ITEM, "Match.ResponseSellItem", "Response Sell Item", MCDT_MACHINE2MACHINE);
			P(MPT_INT, "result");


		C(MC_MATCH_REQUEST_SHOP_ITEMLIST, "Match.RequestShopItemList", "Request Shop Item List", MCDT_MACHINE2MACHINE);
			P(MPT_UID, "uid");
			P(MPT_INT, "FirstItemIndex");
			P(MPT_INT, "ItemCount");
		C(MC_MATCH_RESPONSE_SHOP_ITEMLIST, "Match.ResponseShopItemList", "Response Shop Item List", MCDT_MACHINE2MACHINE);
			P(MPT_BLOB, "ItemList", MCPCBlobArraySize{ sizeof(u32) });

		C(MC_MATCH_REQUEST_CHARACTER_ITEMLIST, "Match.RequestCharacterItemList", "Request Character Item List", MCDT_MACHINE2MACHINE);
			P(MPT_UID, "uid");

		C(MC_MATCH_RESPONSE_CHARACTER_ITEMLIST, "Match.ResponseCharacterItemList", "Response Character Item List", MCDT_MACHINE2MACHINE);
			P(MPT_INT, "Bounty");
			P(MPT_BLOB, "EquipItemList", MCPCBlobArraySize{ sizeof(MUID) });
			P(MPT_BLOB, "ItemList", MCPCBlobArraySize{ sizeof(MTD_ItemNode) });
			
		C(MC_MATCH_REQUEST_EQUIP_ITEM, "MatchRequestEquipItem", "Request Equip Item", MCDT_MACHINE2MACHINE);
			P(MPT_UID, "uidChar");
			P(MPT_UID, "uidItem");
			P(MPT_UINT, "EquipmentSlot", MCPCMinMax{ 0, MMCIP_END - 1 });
		C(MC_MATCH_RESPONSE_EQUIP_ITEM, "MatchResponseEquipItem", "Response Equip Item", MCDT_MACHINE2MACHINE);
			P(MPT_INT, "Result");
		C(MC_MATCH_REQUEST_TAKEOFF_ITEM, "MatchRequestTakeoffItem", "Request Takeoff Item", MCDT_MACHINE2MACHINE);
			P(MPT_UID, "uid");
			P(MPT_UINT, "EquipmentSlot", MCPCMinMax{ 0, MMCIP_END - 1 });
		C(MC_MATCH_RESPONSE_TAKEOFF_ITEM, "MatchResponseTakeoffItem", "Response Takeoff Item", MCDT_MACHINE2MACHINE);
			P(MPT_INT, "Result");
		C(MC_MATCH_REQUEST_ACCOUNT_ITEMLIST, "Match.RequestAccountItemList", "Request Account Item List", MCDT_MACHINE2MACHINE);
			P(MPT_UID, "uid");
		C(MC_MATCH_RESPONSE_ACCOUNT_ITEMLIST, "Match.ResponseAccountItemList", "Response Account Item List" , MCDT_MACHINE2MACHINE);
			P(MPT_BLOB, "ItemList", MCPCBlobArraySize{ sizeof(MTD_AccountItemNode) });
		C(MC_MATCH_REQUEST_BRING_ACCOUNTITEM, "Match.RequestBringAccountItem", "Request Bring Account Item", MCDT_MACHINE2MACHINE);
			P(MPT_UID, "uidChar");
			P(MPT_INT, "AIID");
		C(MC_MATCH_RESPONSE_BRING_ACCOUNTITEM, "Match.ResponseBringAccountItem", "Response Bring Account Item", MCDT_MACHINE2MACHINE);
			P(MPT_INT, "Result");
		C(MC_MATCH_REQUEST_BRING_BACK_ACCOUNTITEM, "Match.RequestBringBackAccountItem", "Request BringBack Account Item", MCDT_MACHINE2MACHINE);
			P(MPT_UID, "uidChar");
			P(MPT_UID, "uidItem");
		C(MC_MATCH_RESPONSE_BRING_BACK_ACCOUNTITEM, "Match.ResponseBringBackAccountItem", "Response BringBack Account Item", MCDT_MACHINE2MACHINE);
			P(MPT_INT, "Result");
		C(MC_MATCH_EXPIRED_RENT_ITEM, "Match.ExpiredRentItem", "Match.Expired Rent Item", MCDT_MACHINE2MACHINE);
			P(MPT_BLOB, "ItemIDList", MCPCBlobArraySize{ sizeof(u32) });


		C(MC_MATCH_REQUEST_SUICIDE, "Match.Request.Suicide", "Request Suicide", MCDT_MACHINE2MACHINE);
			P(MPT_UID, "uidChar");
		C(MC_MATCH_RESPONSE_SUICIDE, "Match.Response.Suicide", "Response Suicide", MCDT_MACHINE2MACHINE);
			P(MPT_INT, "Result");
			P(MPT_UID, "uidChar");

		C(MC_MATCH_REQUEST_OBTAIN_WORLDITEM, "Match.Request.Obtain.WorldItem", "Request Obtain WorldItem", MCDT_MACHINE2MACHINE);
			P(MPT_UID, "uidChar");
			P(MPT_INT, "nItemUID");
		C(MC_MATCH_OBTAIN_WORLDITEM, "Match.WorldItem.Obtain", "Obtain WorldItem", MCDT_MACHINE2MACHINE);
			P(MPT_UID, "uidChar");
			P(MPT_INT, "nItemUID");
		C(MC_MATCH_SPAWN_WORLDITEM, "Match.WorldItem.Spawn", "Spawn WorldItem", MCDT_MACHINE2MACHINE);
			P(MPT_BLOB, "SpawnInfo", MCPCBlobArraySize{ sizeof(MTD_WorldItem) });
		C(MC_MATCH_REQUEST_SPAWN_WORLDITEM, "Match.Request.Spawn.WorldItem", "Request Spawn WorldItem", MCDT_MACHINE2MACHINE);
			P(MPT_UID, "uidChar");
			P(MPT_INT, "ItemID");
			P(MPT_POS, "ItemPos");
		C(MC_MATCH_REMOVE_WORLDITEM, "Match.Request.Spawn.WorldItem", "Request Spawn WorldItem", MCDT_MACHINE2MACHINE);
			P(MPT_INT, "nWorldItemUID");

		C(MC_MATCH_RESET_TEAM_MEMBERS, "Match.Reset.TeamMembers", "Reset Team Members", MCDT_MACHINE2MACHINE);
			P(MPT_BLOB, "TeamMemberData", MCPCBlobArraySize{ sizeof(MTD_ResetTeamMembersData) });

		C(MC_MATCH_ASSIGN_COMMANDER, "Match.Assign.Commander", "Assign Commander", MCDT_MACHINE2MACHINE);
			P(MPT_UID, "uidRedCommander");
			P(MPT_UID, "uidBlueCommander");

		C(MC_MATCH_SET_OBSERVER, "Match.Set.Observer", "Set Observer", MCDT_MACHINE2MACHINE);
			P(MPT_UID, "uidChar");

		C(MC_MATCH_LADDER_REQUEST_CHALLENGE, "Match.Ladder.Request.Challenge", "Request Challenge a Ladder", MCDT_MACHINE2MACHINE);
			P(MPT_INT, "MemberCount");
			P(MPT_UINT, "Options");
			P(MPT_BLOB, "MemberNames", MCPCBlobArraySize{ sizeof(MTD_LadderTeamMemberNode) });
			
		C(MC_MATCH_LADDER_RESPONSE_CHALLENGE, "Match.Ladder.Response.Challenge", "Response Challenge a Ladder", MCDT_MACHINE2MACHINE);
			P(MPT_INT, "Result");
		C(MC_MATCH_LADDER_SEARCH_RIVAL, "Match.Ladder.SearchRival", "Search a Ladder Rival", MCDT_MACHINE2MACHINE);
		C(MC_MATCH_LADDER_REQUEST_CANCEL_CHALLENGE, "Match.Ladder.Request.CancelChallenge", "Request Cancel Challenge", MCDT_MACHINE2MACHINE);
		C(MC_MATCH_LADDER_CANCEL_CHALLENGE, "Match.Ladder.CancelChallenge", "Cancel Ladder Challenge", MCDT_MACHINE2MACHINE);
			P(MPT_STR, "CharName");
		C(MC_MATCH_LADDER_PREPARE, "Ladder.Prepare", "Prepare Ladder Game", MCDT_MACHINE2MACHINE);
			P(MPT_UID, "uidStage");
			P(MPT_INT, "nTeam");
		C(MC_MATCH_LADDER_LAUNCH, "Ladder.Launch", "Launch Ladder Game", MCDT_MACHINE2MACHINE);
			P(MPT_UID, "uidStage");
			P(MPT_STR, "MapName");

		C(MC_MATCH_REQUEST_PROPOSAL, "Match.RequestProposal", "Request Proposal", MCDT_MACHINE2MACHINE);
			P(MPT_UID, "uidChar");
			P(MPT_INT, "ProposalMode");	// MMatchProposalMode
			P(MPT_INT, "RequestID");
			P(MPT_INT, "ReplierCount");
			P(MPT_BLOB, "ReplierCharNames", MCPCBlobArraySize{ sizeof(MTD_ReplierNode) });
		C(MC_MATCH_RESPONSE_PROPOSAL, "Match.ResponseProposal", "Response Proposal", MCDT_MACHINE2MACHINE);
			P(MPT_INT, "Result");
			P(MPT_INT, "ProposalMode");	// MMatchAgreementMode
			P(MPT_INT, "RequestID");
		C(MC_MATCH_ASK_AGREEMENT, "Match.AskAgreement", "Ask Agreement", MCDT_MACHINE2MACHINE);
			P(MPT_UID,	"uidProposer");
			P(MPT_BLOB, "MembersCharName", MCPCBlobArraySize{ sizeof(MTD_ReplierNode) });
			P(MPT_INT,	"ProposalMode"); // MMatchAgreementMode
			P(MPT_INT,	"RequestID");
		C(MC_MATCH_REPLY_AGREEMENT, "Match.ReplyAgreement", "Reply Agreement", MCDT_MACHINE2MACHINE);
			P(MPT_UID,	"uidProposer");
			P(MPT_UID,	"uidChar");
			P(MPT_STR,	"Replier");
			P(MPT_INT,	"ProposalMode");
			P(MPT_INT,	"RequestID");
			P(MPT_BOOL,	"Agreement");

			
		C(MC_MATCH_FRIEND_ADD, "Match.Friend.Add", "Add a Friend", MCDT_MACHINE2MACHINE);
			P(MPT_STR, "Name");
		C(MC_MATCH_FRIEND_REMOVE, "Match.Friend.Remove", "Remove a Friend", MCDT_MACHINE2MACHINE);
			P(MPT_STR, "Name");
		C(MC_MATCH_FRIEND_LIST, "Match.Friend.List", "List Friend", MCDT_MACHINE2MACHINE);
		C(MC_MATCH_RESPONSE_FRIENDLIST, "Match.Response.FriendList", "Response List Friend", MCDT_MACHINE2MACHINE);
			P(MPT_BLOB, "FriendList", MCPCBlobArraySize{ sizeof(MFRIENDLISTNODE) });
		C(MC_MATCH_FRIEND_MSG, "Match.Friend.Msg", "Message to Friends", MCDT_MACHINE2MACHINE);
			P(MPT_STR, "Msg");

		C(MC_MATCH_CLAN_REQUEST_CREATE_CLAN,
			"Match.Clan.RequestCreateClan", "Request Create Clan",
			MCDT_MACHINE2MACHINE);
			P(MPT_UID, "uidChar");
			P(MPT_INT, "RequestID");
			P(MPT_STR, "ClanName");
#if CLAN_SPONSORS_COUNT == 4
			P(MPT_STR, "Member1CharName");
			P(MPT_STR, "Member2CharName");
			P(MPT_STR, "Member3CharName");
			P(MPT_STR, "Member4CharName");
#elif CLAN_SPONSORS_COUNT > 0
			static_assert(false, "Invalid CLAN_SPONSORS_COUNT value");
#endif
        C(MC_MATCH_CLAN_RESPONSE_CREATE_CLAN,
			"Match.Clan.ResponseCreateClan", "Response Create Clan",
			MCDT_MACHINE2MACHINE);
			P(MPT_INT, "Result");
			P(MPT_INT, "RequestID");
		C(MC_MATCH_CLAN_ASK_SPONSOR_AGREEMENT,
			"Match.Clan.AskSponsorAgreement", "Ask Sponsor's Agreement",
			MCDT_MACHINE2MACHINE);
			P(MPT_INT, "RequestID");
			P(MPT_STR, "ClanName");
			P(MPT_UID, "uidClanMaster");
			P(MPT_STR, "szClanMaster");
		C(MC_MATCH_CLAN_ANSWER_SPONSOR_AGREEMENT,
			"Match.Clan.AnswerSponsorAgreement", "Answer Sponsor's Agreement",
			MCDT_MACHINE2MACHINE);
			P(MPT_INT,	"RequestID");
			P(MPT_UID,	"uidClanMaster");
			P(MPT_STR,	"SponsorCharName");
			P(MPT_BOOL, "Answer");
		C(MC_MATCH_CLAN_REQUEST_AGREED_CREATE_CLAN, "Match.Clan.RequestAgreedCreateClan", "Request Agreed Create Clan", MCDT_MACHINE2MACHINE);
			P(MPT_UID, "uidChar");
			P(MPT_STR, "ClanName");
			P(MPT_STR, "Member1CharName");
			P(MPT_STR, "Member2CharName");
			P(MPT_STR, "Member3CharName");
			P(MPT_STR, "Member4CharName");
        C(MC_MATCH_CLAN_RESPONSE_AGREED_CREATE_CLAN, "Match.Clan.AgreedResponseCreateClan", "Response Agreed Create Clan", MCDT_MACHINE2MACHINE);
			P(MPT_INT, "Result");
		C(MC_MATCH_CLAN_REQUEST_CLOSE_CLAN, "Match.Clan.RequestCloseClan", "Request Close Clan", MCDT_MACHINE2MACHINE);
			P(MPT_UID, "uidChar");
			P(MPT_STR, "ClanName");
        C(MC_MATCH_CLAN_RESPONSE_CLOSE_CLAN, "Match.Clan.ResponseCloseClan", "Response Close Clan", MCDT_MACHINE2MACHINE);
			P(MPT_INT, "Result");
		
		C(MC_MATCH_CLAN_REQUEST_JOIN_CLAN, "Match.Clan.RequestJoinClan", "Request Join Clan", MCDT_MACHINE2MACHINE);
			P(MPT_UID, "uidChar");
			P(MPT_STR, "ClanName");
			P(MPT_STR, "szJoiner");
		C(MC_MATCH_CLAN_RESPONSE_JOIN_CLAN, "Match.Clan.ResponseJoinClan", "Response Join Clan", MCDT_MACHINE2MACHINE);
			P(MPT_INT, "Result");
		C(MC_MATCH_CLAN_ASK_JOIN_AGREEMENT, "Match.Clan.AskJoinAgreement", "Ask Join Agreement", MCDT_MACHINE2MACHINE);
			P(MPT_STR, "ClanName");
			P(MPT_UID, "uidClanAdmin");
			P(MPT_STR, "szClanAdmin");
		C(MC_MATCH_CLAN_ANSWER_JOIN_AGREEMENT, "Match.Clan.AnswerJoinAgreement", "Answer Join Agreement", MCDT_MACHINE2MACHINE);
			P(MPT_UID, "uidClanAdmin");
			P(MPT_STR,	"JoinerCharName");
			P(MPT_BOOL, "Answer");

		C(MC_MATCH_CLAN_REQUEST_AGREED_JOIN_CLAN, "Match.Clan.RequestAgreedJoinClan", "Request Agreed Join Clan", MCDT_MACHINE2MACHINE);
			P(MPT_UID,	"uidClanAdmin");
			P(MPT_STR,	"ClanName");
			P(MPT_STR,	"szJoiner");
		C(MC_MATCH_CLAN_RESPONSE_AGREED_JOIN_CLAN, "Match.Clan.ResponseAgreedJoinClan", "Response Agreed Join Clan", MCDT_MACHINE2MACHINE);
			P(MPT_INT, "Result");

		C(MC_MATCH_CLAN_REQUEST_LEAVE_CLAN, "Match.Clan.RequestLeaveClan", "Request Leave Clan", MCDT_MACHINE2MACHINE);
			P(MPT_UID, "uidChar");
		C(MC_MATCH_CLAN_RESPONSE_LEAVE_CLAN, "Match.Clan.ResponseLeaveClan", "Response Leave Clan", MCDT_MACHINE2MACHINE);
			P(MPT_INT, "Result");

		C(MC_MATCH_CLAN_UPDATE_CHAR_CLANINFO, "Match.Clan.UpdateCharClanInfo", "Update Char ClanInfo", MCDT_MACHINE2MACHINE);
			P(MPT_BLOB, "ClanInfo", MCPCBlobArraySize{ sizeof(MTD_CharClanInfo)});

		C(MC_MATCH_CLAN_MASTER_REQUEST_CHANGE_GRADE, "Match.Clan.Master.RequestChangeGrade", "Request Change ClanGrade", MCDT_MACHINE2MACHINE);
			P(MPT_UID, "uidClanMaster");
			P(MPT_STR, "szMember");
			P(MPT_INT, "Grade", MCPCMinMax{ 0, MCG_END - 1 });
		C(MC_MATCH_CLAN_MASTER_RESPONSE_CHANGE_GRADE, "Match.Clan.Master.ResponseChangeGrade", "Response Change ClanGrade", MCDT_MACHINE2MACHINE);
			P(MPT_INT, "Result");

		C(MC_MATCH_CLAN_ADMIN_REQUEST_EXPEL_MEMBER, "Match.Clan.Admin.RequestExpelMember", "Request Expel ClanMember", MCDT_MACHINE2MACHINE);
			P(MPT_UID, "uidClanAdmin");
			P(MPT_STR, "szMember");
		C(MC_MATCH_CLAN_ADMIN_RESPONSE_EXPEL_MEMBER, "Match.Clan.Admin.ResponseLeaveMember", "Response Expel ClanMember", MCDT_MACHINE2MACHINE);
			P(MPT_INT, "Result");

		C(MC_MATCH_CLAN_REQUEST_MSG, "Match.Clan.Request.Msg", "Request Clan Msg", MCDT_MACHINE2MACHINE);
			P(MPT_UID, "uidSender");
			P(MPT_STR, "Msg");

		C(MC_MATCH_CLAN_MSG, "Match.Clan.Msg", "Clan Msg", MCDT_MACHINE2MACHINE);
			P(MPT_STR, "SenderName");
			P(MPT_STR, "Msg");

		C(MC_MATCH_CLAN_REQUEST_MEMBER_LIST, "Match.Clan.Request.ClanMemberList",
			"Request Clan Member List", MCDT_MACHINE2MACHINE);
			P(MPT_UID, "uidChar");
		C(MC_MATCH_CLAN_RESPONSE_MEMBER_LIST, "Match.Clan.Response.ClanMemberList",
			"Response Clan Member List", MCDT_MACHINE2MACHINE);
			P(MPT_BLOB, "ClanMemberList", MCPCBlobArraySize{ sizeof(MTD_ClanMemberListNode) });
		C(MC_MATCH_CLAN_REQUEST_CLAN_INFO, "Match.Clan.Request.Clan.Info",
			"Request Clan Info", MCDT_MACHINE2MACHINE);
			P(MPT_UID, "uidChar");
			P(MPT_STR, "ClanName");
		C(MC_MATCH_CLAN_RESPONSE_CLAN_INFO, "Match.Clan.Response.Clan.Info",
			"Response Clan Info", MCDT_MACHINE2MACHINE);
			P(MPT_BLOB, "ClanInfo", MCPCBlobArraySize{ sizeof(MTD_ClanInfo), 1, 1 });
		C(MC_MATCH_CLAN_STANDBY_CLAN_LIST, "Match.Clan.Standby.ClanList",
			"Standby Clan List", MCDT_MACHINE2MACHINE);
			P(MPT_INT, "PrevClanListCount");
			P(MPT_INT, "NextClanListCount");
			P(MPT_BLOB, "ClanList", MCPCBlobArraySize{ sizeof(MTD_StandbyClanList) });
		C(MC_MATCH_CLAN_MEMBER_CONNECTED, "Match.Clan.Member.Connected",
			"Member Connected", MCDT_MACHINE2MACHINE);
			P(MPT_STR, "szMember");
		C(MC_MATCH_CLAN_REQUEST_EMBLEMURL, "Match.Clan.Request.EmblemURL",
			"Request EmblemURL", MCDT_MACHINE2MACHINE);
			P(MPT_INT, "ClanCLID");
		C(MC_MATCH_CLAN_RESPONSE_EMBLEMURL, "Match.Clan.Response.EmblemURL",
			"Response EmblemURL", MCDT_MACHINE2MACHINE);
			P(MPT_INT, "ClanCLID");
			P(MPT_INT, "EmblemChecksum");
			P(MPT_STR, "EmblemURL");
		C(MC_MATCH_CLAN_LOCAL_EMBLEMREADY, "Match.Clan.Local.EmblemReady",
			"Notify Emblem Ready", MCDT_LOCAL);
			P(MPT_INT, "ClanCLID");
			P(MPT_STR, "EmblemURL");
		C(MC_MATCH_CLAN_ACCOUNCE_DELETE, "MC_MATCH_CLAN_ACCOUNCE_DELETE",
			"delete clan info announce to clan member", MCDT_MACHINE2MACHINE);
			P(MPT_STR, "delete info" );

		// Votes
		C(MC_MATCH_CALLVOTE, "Match.Callvote", "Callvote", MCDT_MACHINE2MACHINE);
			P(MPT_STR, "strDiscuss");
			P(MPT_STR, "strArg");
		C(MC_MATCH_NOTIFY_CALLVOTE, "Match.NotifyCallvote", "Notify Callvote", MCDT_MACHINE2MACHINE);
			P(MPT_STR, "strDiscuss");
			P(MPT_STR, "strArg");
		C(MC_MATCH_NOTIFY_VOTERESULT, "Match.NotifyVoteResult", "Notify Vote Result", MCDT_MACHINE2MACHINE);
			P(MPT_STR, "strDiscuss");
			P(MPT_INT, "nResult");	// 1=Passed , 0=Rejected
		C(MC_MATCH_VOTE_YES, "Match.VoteYes", "Vote", MCDT_MACHINE2MACHINE);
		C(MC_MATCH_VOTE_NO, "Match.VoteNo", "Vote", MCDT_MACHINE2MACHINE);
		C(MC_MATCH_VOTE_STOP, "Vote stop", "Vote stop", MCDT_MACHINE2MACHINE );

		// Clan broadcasting
		C(MC_MATCH_BROADCAST_CLAN_RENEW_VICTORIES, "Match.Broadcast.ClanRenewVictories",
			"Broadcast Clan Renew Victories", MCDT_MACHINE2MACHINE);
			P(MPT_STR, "strWinnerClanName");
			P(MPT_STR, "strLoserClanName");
			P(MPT_INT, "nVictories");
		C(MC_MATCH_BROADCAST_CLAN_INTERRUPT_VICTORIES, "Match.Broadcast.ClanInterruptVictories",
			"Broadcast Clan Interrupt Victories", MCDT_MACHINE2MACHINE);
			P(MPT_STR, "strWinnerClanName");
			P(MPT_STR, "strLoserClanName");
			P(MPT_INT, "nVictories");

		C(MC_MATCH_BROADCAST_DUEL_RENEW_VICTORIES, "Match.Broadcast.DuelRenewVictories",
			"Broadcast Duel Renew Victories", MCDT_MACHINE2MACHINE);
			P(MPT_STR, "strChampionName");
			P(MPT_STR, "strChannelName");
			P(MPT_INT, "nRoomNumber");
			P(MPT_INT, "nVictories");
		C(MC_MATCH_BROADCAST_DUEL_INTERRUPT_VICTORIES, "Match.Broadcast.DuelInterruptVictories",
			"Broadcast Duel Interrupt Victories", MCDT_MACHINE2MACHINE);
			P(MPT_STR, "strChampionName");
			P(MPT_STR, "strInterrupterName");
			P(MPT_INT, "nVictories");

		// Berserker
		C(MC_MATCH_ASSIGN_BERSERKER, "Match.Assign.Berserker", "Assign Berserker", MCDT_MACHINE2MACHINE);
			P(MPT_UID, "uidPlayer");

		C(MC_MATCH_DUEL_QUEUEINFO, "Match.Duel.Queue Info", "Queue Info", MCDT_MACHINE2MACHINE);
			P(MPT_BLOB, "QueueInfo", MCPCBlobSize{ sizeof(MTD_DuelQueueInfo) });

		C(MC_QUEST_PING, "Match.Quest.Ping", "QuestPing", MCDT_MACHINE2MACHINE);
			P(MPT_UINT, "nTimeStamp");

		C(MC_QUEST_PONG, "Match.Quest.Pong", "QuestPong", MCDT_MACHINE2MACHINE);
			P(MPT_UINT, "nTimeStamp");

		// Event
		C(MC_EVENT_CHANGE_MASTER, "Event.ChangeMaster", "Take out Master from Stage", MCDT_MACHINE2MACHINE);
		C(MC_EVENT_CHANGE_PASSWORD, "Event.ChangePassword", "Change Password on Stage", MCDT_MACHINE2MACHINE);
			P(MPT_STR, "strPassword");
		C(MC_EVENT_REQUEST_JJANG, "Event.RequestJJang",
			"Request JJang mark to a Player", MCDT_MACHINE2MACHINE);
			P(MPT_STR, "strTargetName");
		C(MC_EVENT_REMOVE_JJANG, "Event.RemoveJJang", "Remove JJang mark from a Player", MCDT_MACHINE2MACHINE);
			P(MPT_STR, "strTargetName");
		C(MC_EVENT_UPDATE_JJANG, "Event.UpdateJJang",
			"Update JJang Player", MCDT_MACHINE2MACHINE);
			P(MPT_UID, "uidPlayer");
			P(MPT_BOOL, "bJjang");

		// Quest
		C(MC_QUEST_NPC_SPAWN,	"Quest.NPCSpawn", "Npc Spawn", MCDT_MACHINE2MACHINE);
			P(MPT_UID,		"uidChar");
			P(MPT_UID,		"nNpcUID");
			P(MPT_UCHAR,	"nNpcType");
			P(MPT_UCHAR,	"PositionIndex");

		C(MC_QUEST_ENTRUST_NPC_CONTROL,		"Quest.Entrust.NPC.Control", "Entrust Npc Control", MCDT_MACHINE2MACHINE);
			P(MPT_UID, "uidChar");
			P(MPT_UID, "nNpcUID");
		C(MC_QUEST_CHECKSUM_NPCINFO,		"Quest.Checksum.NPCInfo",		"Checksum NPC Info", MCDT_MACHINE2MACHINE);
			P(MPT_UID, "uidChar");
			P(MPT_UINT, "checksum");
		C(MC_QUEST_REQUEST_NPC_DEAD,	"Quest.Request.NPCDead", "Request Npc Dead", MCDT_MACHINE2MACHINE);
			P(MPT_UID, "uidChar");
			P(MPT_UID, "uidNPC");
			P(MPT_SVECTOR, "NpcPosition");
		C(MC_QUEST_NPC_DEAD,	"Quest.NPCDead", "Npc Dead", MCDT_MACHINE2MACHINE);
			P(MPT_UID, "uidKillerPlayer");
			P(MPT_UID, "uidNPC");
		C(MC_QUEST_REFRESH_PLAYER_STATUS,	"Quest.RefreshPlayerStatus", "Refresh Player Status", MCDT_MACHINE2MACHINE);

		C(MC_QUEST_NPC_ALL_CLEAR,	"Quest.NPC.AllClear", "Clear All NPC", MCDT_MACHINE2MACHINE);

		C(MC_QUEST_ROUND_START,	"Quest.Round.Start", "Quest Start Round", MCDT_MACHINE2MACHINE);
			P(MPT_UCHAR, "round");
		C(MC_MATCH_QUEST_REQUEST_DEAD, "Quest.RequestDead", "Quest Request Dead", MCDT_MACHINE2MACHINE);
		C(MC_MATCH_QUEST_PLAYER_DEAD, "Quest.PlayerDead", "Quest Player Dead", MCDT_MACHINE2MACHINE);
			P(MPT_UID, "Victim");
		C(MC_QUEST_OBTAIN_QUESTITEM, "Quest.ObtainQuestItem", "Obtain QuestItem", MCDT_MACHINE2MACHINE);
			P(MPT_UINT, "QuestItemID");
		C(MC_QUEST_OBTAIN_ZITEM, "Quest.ObtainZItem", "Obtain ZItem", MCDT_MACHINE2MACHINE);
			P(MPT_UINT, "ItemID");

		C(MC_QUEST_STAGE_MAPSET, "Quest.State.Mapset", "Change Stage Quest Mapset Setting", MCDT_MACHINE2MACHINE);
			P(MPT_UID, "uidStage");
			P(MPT_CHAR, "QuestMapset");
		C(MC_QUEST_STAGE_GAME_INFO, "Quest.Stage.GameInfo", "Quest Stage GameInfo", MCDT_MACHINE2MACHINE);
			P(MPT_CHAR, "Quest level");
			P(MPT_CHAR, "Mapset ID");
			P(MPT_UINT, "QuestScenarioID");

		C(MC_QUEST_SECTOR_BONUS, "Quest.SectorBonus", "Quest Sector Bonus", MCDT_MACHINE2MACHINE);
			P(MPT_UID, "uidPlayer");
			P(MPT_UINT, "XPBonus");

		C(MC_QUEST_GAME_INFO, "Quest.GameInfo", "Quest Game Info", MCDT_MACHINE2MACHINE);
			P(MPT_BLOB, "Info", MCPCBlobArraySize{ sizeof(MTD_QuestGameInfo), 1, 1 });
		C(MC_QUEST_COMBAT_STATE, "Quest.Combat.State", "Quest Combat State", MCDT_MACHINE2MACHINE);
			P(MPT_CHAR, "CombatState");

		C(MC_QUEST_SECTOR_START, "Quest.Sector.Start", "Quest Sector Start", MCDT_MACHINE2MACHINE);
			P(MPT_CHAR, "SectorIndex");

		C(MC_QUEST_COMPLETED, "Quest.Complete", "Complete Quest", MCDT_MACHINE2MACHINE);
			P(MPT_BLOB, "RewardInfo", MCPCBlobArraySize{ sizeof(MTD_QuestReward) });
		C(MC_QUEST_FAILED, "Quest", "Quest failed", MCDT_MACHINE2MACHINE);

		C(MC_QUEST_REQUEST_MOVETO_PORTAL, "Quest.Request.Moveto.Portal", "Request Moveto Portal", MCDT_MACHINE2MACHINE);
			P(MPT_CHAR, "CurrSectorIndex");
		C(MC_QUEST_MOVETO_PORTAL, "Quest.Moveto.Portal", "Moveto Portal", MCDT_MACHINE2MACHINE);
			P(MPT_CHAR, "SectorIndex");
			P(MPT_UID, "uidPlayer");
		C(MC_QUEST_READYTO_NEWSECTOR, "Quest.Readyto.NewSector", "Ready To New Sector", MCDT_MACHINE2MACHINE);
			P(MPT_UID, "uidPlayer");

		C(MC_QUEST_PEER_NPC_BASICINFO, "Quest.Peer.NPC.BasicInfo",
			"NPC BasicInfo", MCDT_PEER2PEER);
			P(MPT_BLOB, "Info", MCPCBlobSize{ sizeof(ZACTOR_BASICINFO) });
		// NOTE: This command is not used.
		C(MC_QUEST_PEER_NPC_HPINFO, "Quest.Peer.NPC.HPInfo",
			"NPC HPInfo", MCDT_PEER2PEER);
			P(MPT_INT, "nNPCCount");
			P(MPT_BLOB, "HPTable");
		C(MC_QUEST_PEER_NPC_ATTACK_MELEE, "Quest.Peer.NPC.Attack.Melee",
			"NPC Melee Attack", MCDT_PEER2PEER);
			P(MPT_UID, "uidOwner");
		C(MC_QUEST_PEER_NPC_ATTACK_RANGE, "Quest.Peer.NPC.Attack.Range",
			"NPC Range Attack", MCDT_PEER2PEER);
			P(MPT_UID, "uidOwner");
			P(MPT_BLOB, "Info", MCPCBlobSize{ sizeof(ZPACKEDSHOTINFO) });
		C(MC_QUEST_PEER_NPC_SKILL_START, "Quest.Peer.NPC.Skill.Start",
			"NPC Skill Start", MCDT_PEER2PEER);
			P(MPT_UID, "uidOwner");
			P(MPT_INT, "nSkill");
			P(MPT_UID, "uidTarget");
			P(MPT_POS, "targetPos");
		C(MC_QUEST_PEER_NPC_SKILL_EXECUTE,	"Quest.Peer.NPC.Skill.Execute",
			"NPC Skill Start", MCDT_PEER2PEER);
			P(MPT_UID, "uidOwner");
			P(MPT_INT, "nSkill");
			P(MPT_UID, "uidTarget");
			P(MPT_POS, "targetPos");

		C(MC_QUEST_PEER_NPC_DEAD,	"Quest.Peer.NPC.Dead",	"NPC Dead", MCDT_PEER2PEER);
			P(MPT_UID, "uidKillerPlayer");
			P(MPT_UID, "uidNPC"); // NPC id

		C(MC_QUEST_TEST_REQUEST_NPC_SPAWN,	"Quest.Test.RequestNPCSpawn", "NPC Spawn", MCDT_MACHINE2MACHINE);
			P(MPT_INT,  "NPC Type");
			P(MPT_INT,	"NPC Count");
		C(MC_QUEST_TEST_REQUEST_CLEAR_NPC,		"Quest.Test.ClearNPC", "Clear NPC", MCDT_MACHINE2MACHINE);
		C(MC_QUEST_TEST_REQUEST_SECTOR_CLEAR,	"Quest.Test.SectorClear", "Sector Clear", MCDT_MACHINE2MACHINE);
		C(MC_QUEST_TEST_REQUEST_FINISH,			"Quest.Test.Finish", "Finish Quest", MCDT_MACHINE2MACHINE);

		C(MC_TEST_BIRDTEST1, "Test.BirdTest1", "BirdTest1", MCDT_MACHINE2MACHINE);
			P(MPT_INT, "Result");
			P(MPT_UID, "uidPlayer");
			P(MPT_BLOB, "Dummy");
			
		C(MC_TEST_PEERTEST_PING, "Test.PeerTest.Ping", "PeerTest Ping", MCDT_PEER2PEER | MCCT_NON_ENCRYPTED);
		C(MC_TEST_PEERTEST_PONG, "Test.PeerTest.Pong", "PeerTest Pong", MCDT_PEER2PEER | MCCT_NON_ENCRYPTED);
	}

	if(IsTypeAnyOf(MatchServer, Client, Agent)) {
		C(MC_ADMIN_ANNOUNCE, "Admin.Announce", "Announce", MCDT_MACHINE2MACHINE | MCCT_NON_ENCRYPTED );
			P(MPT_UID, "uidAdmin");
			P(MPT_STR, "Msg");
			P(MPT_UINT, "MsgType");
		C(MC_ADMIN_PING_TO_ALL, "Admin.PingToAll", "Ping to All Clients", MCDT_MACHINE2MACHINE);

		if (IsTypeAnyOf(MatchServer, Client)) {
			C(MC_ADMIN_REQUEST_SERVER_INFO, "Admin.RequestServerInfo", "Request Server Info", MCDT_MACHINE2MACHINE);
				P(MPT_UID, "uidAdmin");
			// NOTE: Unused.
			C(MC_ADMIN_RESPONSE_SERVER_INFO, "Admin.ResponseServerInfo", "Response Server Info", MCDT_MACHINE2MACHINE);
				P(MPT_BLOB, "ServerInfo");
			C(MC_ADMIN_SERVER_HALT, "Admin.Halt", "Halt Server", MCDT_MACHINE2MACHINE);
				P(MPT_UID, "uidAdmin");
			C(MC_ADMIN_TERMINAL, "Admin.Terminal", "Admin.Terminal", MCDT_MACHINE2MACHINE);
				P(MPT_UID, "uidAdmin");
				P(MPT_STR, "message");
			C(MC_ADMIN_REQUEST_UPDATE_ACCOUNT_UGRADE, "Admin.RequestUpdateAccountUGrade", "Request Update Account UGrade", MCDT_MACHINE2MACHINE);
				P(MPT_UID, "uidAdmin");
				P(MPT_STR, "uidTargetCharName");
			C(MC_ADMIN_RESPONSE_UPDATE_ACCOUNT_UGRADE, "Admin.ResponseUpdateAccountUGrade", "Response Update Account UGrade", MCDT_MACHINE2MACHINE);
				P(MPT_INT, "Result");
				P(MPT_UID, "uidChar");
			C(MC_ADMIN_REQUEST_BAN_PLAYER, "Admin.RequestBanPlayer", "Request Ban Player", MCDT_MACHINE2MACHINE);
				P(MPT_UID, "uidAdmin");
				P(MPT_STR, "uidTargetCharName");
			C(MC_ADMIN_RESPONSE_BAN_PLAYER, "Admin.ResponseBanPlayer", "Response Ban Player", MCDT_MACHINE2MACHINE);
				P(MPT_INT, "Result");
			C(MC_ADMIN_REQUEST_SWITCH_LADDER_GAME, "Admin.RequestSwitchLadderGame", "Request Switch LadderGame", MCDT_MACHINE2MACHINE);
				P(MPT_UID, "uidAdmin");	
				P(MPT_BOOL, "IsEnabled");
			C(MC_ADMIN_HIDE, "Admin.Hide", "Hide Admin Player", MCDT_MACHINE2MACHINE);
			C(MC_ADMIN_RELOAD_CLIENT_HASH, "Admin.ReloadClientHash", "Reload Client Hash", MCDT_MACHINE2MACHINE);
			C(MC_ADMIN_RESET_ALL_HACKING_BLOCK, "MC_ADMIN_RESET_ALL_HACKING_BLOCK", "reset all hacking block", MCDT_MACHINE2MACHINE);
			C(MC_ADMIN_TELEPORT, "Admin.Teleport", "Teleport player to admin position", MCDT_MACHINE2MACHINE);
				P(MPT_STR, "AdminName");
				P(MPT_STR, "TargetName");
			C(MC_ADMIN_MUTE, "", "", MCDT_MACHINE2MACHINE);
				P(MPT_STR, "Target");
				P(MPT_STR, "Reason");
				P(MPT_INT, "Seconds");
		}		
	}

	if(IsTypeAnyOf(Master)){
		C(MC_NET_REQUEST_UID, "Net.RequestUID", "Request UID", MCDT_MACHINE2MACHINE);
			P(MPT_INT, "Size");
		C(MC_NET_RESPONSE_UID, "Net.ResponseUID", "Response UID", MCDT_MACHINE2MACHINE);
			P(MPT_UID, "Start");
		P(MPT_UID, "End");
	}

	if (IsTypeAnyOf(Client)) {
		C(MC_PEER_OPENED, "Peer.Open", "Peer Connection Opened", MCDT_LOCAL);
			P(MPT_UID, "uidPlayer");

		C(MC_PEER_MOVE, "Peer.Move", "Move Object", MCDT_PEER2PEER);
			P(MPT_POS, "Position");
			P(MPT_VECTOR, "Direction");
			P(MPT_VECTOR, "Velocity");
			P(MPT_INT, "StateUpper");
			P(MPT_INT, "StateLower");

		C(MC_PEER_CHANGE_WEAPON, "Peer.ObjectChangeWeapon", "Change Object Weapon", MCDT_PEER2PEER);
			P(MPT_INT, "WeaponID");

		C(MC_PEER_CHANGE_PARTS, "Peer.ObjectChangeParts", "Change Object Parts", MCDT_PEER2PEER);
			P(MPT_INT, "PartsType");
			P(MPT_INT, "PartsID");

		C(MC_PEER_ATTACK, "Peer.ObjectAttack", "Object Attack", MCDT_PEER2PEER);
			P(MPT_INT, "AttackType");
			P(MPT_VECTOR, "Position");

		C(MC_PEER_DAMAGE, "Peer.ObjectDamage", "Object Damage", MCDT_PEER2PEER);
			P(MPT_UID, "TargetUID");
			P(MPT_INT, "Damage");

		C(MC_PEER_CHAT, "Peer.Chat", "Chat", MCDT_PEER2PEER);
			P(MPT_INT, "TeamID");
			P(MPT_STR, "Msg");

		C(MC_PEER_CHAT_ICON, "Peer.ChatIcon", "ChatIcon", MCDT_PEER2PEER);
			P(MPT_BOOL, "bStart");

		C(MC_PEER_REACTION, "Peer.Reaction", "React", MCDT_PEER2PEER);
			P(MPT_FLOAT, "Time");
			P(MPT_INT, "ReactionID");

		C(MC_PEER_ENCHANT_DAMAGE, "Peer.EnchantDamage", "EnchantDamage", MCDT_PEER2PEER);
			P(MPT_UID, "OwnerUID");
			P(MPT_UID, "TargetUID");

		C(MC_PEER_SHOT, "Peer.Shot", "Shot", MCDT_PEER2PEER);
			P(MPT_BLOB, "Info", MCPCBlobSize{ sizeof(ZPACKEDSHOTINFO) });

		C(MC_PEER_SHOT_MELEE, "Peer.Shot.Melee", "ShotMelee", MCDT_PEER2PEER);
			P(MPT_FLOAT, "Time");
			P(MPT_POS, "Position");
			P(MPT_INT, "nShot");

		C(MC_PEER_SHOT_SP, "Peer.Shot.Sp", "ShotSp", MCDT_PEER2PEER);
			P(MPT_FLOAT, "Time");
			P(MPT_POS, "Position");
			P(MPT_VECTOR, "Direction");
			P(MPT_INT, "Type");
			P(MPT_INT, "SelType");

		C(MC_PEER_RELOAD, "Peer.Reload", "Reload", MCDT_PEER2PEER);

		C(MC_PEER_SPMOTION,"Peer.ObjectSpMotion","Object SpMotion",MCDT_PEER2PEER);
			P(MPT_INT, "SelType");

		C(MC_PEER_CHANGECHARACTER, "Peer.ChangeCharacter", "ChangeCharacter", MCDT_PEER2PEER);

		C(MC_PEER_DIE, "Peer.Die", "Die", MCDT_PEER2PEER);
			P(MPT_UID, "Attacker");

		C(MC_PEER_SPAWN, "Peer.Spawn", "Spawn", MCDT_PEER2PEER);
			P(MPT_POS, "Position");
			P(MPT_DIR, "Direction");

		C(MC_PEER_DASH, "Peer.Dash", "Dash", MCDT_PEER2PEER);
			P(MPT_BLOB, "DashInfo", MCPCBlobSize{ sizeof(ZPACKEDDASHINFO) });

		C(MC_PEER_SKILL, "Peer.ObjectSkill", "Skill", MCDT_PEER2PEER);
			P(MPT_FLOAT, "Time");
			P(MPT_INT, "SkillID");
			P(MPT_INT, "SelType");

		C(MC_PEER_BASICINFO, "Peer.CharacterBasicInfo", "BasicInfo", MCDT_PEER2PEER);
			P(MPT_BLOB, "Info", MCPCBlobSize{ sizeof(ZPACKEDBASICINFO) });

		C(MC_PEER_HPINFO, "Peer.CharacterHPInfo", "HPInfo", MCDT_PEER2PEER);
			P(MPT_FLOAT, "fHP");

		C(MC_PEER_HPAPINFO, "Peer.CharacterHPAPInfo", "HPAPInfo", MCDT_PEER2PEER);
			P(MPT_FLOAT, "fHP");
			P(MPT_FLOAT, "fAP");

		C(MC_PEER_UDPTEST, "Peer.UDPTest", "UDP Test on Peer-to-Peer", MCDT_PEER2PEER | MCCT_NON_ENCRYPTED);
		C(MC_PEER_UDPTEST_REPLY, "Peer.UDPTestReply", "UDP Test Reply on Peer-to-Peer", MCDT_PEER2PEER | MCCT_NON_ENCRYPTED);

		C(MC_PEER_PING, "Peer.Ping", "Ping", MCDT_PEER2PEER | MCCT_NON_ENCRYPTED);
			P(MPT_UINT, "TimeStamp");
		C(MC_PEER_PONG, "Peer.Pong", "Pong", MCDT_PEER2PEER | MCCT_NON_ENCRYPTED);
			P(MPT_UINT, "TimeStamp");
	}

	if (IsTypeAnyOf(Agent)) {
		C(MC_AGENT_CONNECT, "Agent.Connect", "Connect Agent to MatchServer", MCDT_LOCAL);
			P(MPT_STR, "Address");
			P(MPT_INT, "Port");
		C(MC_AGENT_DISCONNECT, "Agent.Disconnect", "Unregister Agent from MatchServer", MCDT_LOCAL);
		C(MC_AGENT_LOCAL_LOGIN, "Agent.LocalLogin", "Client login", MCDT_LOCAL);
			P(MPT_UID, "uidComm");
			P(MPT_UID, "uidPlayer");
	}

	if (IsTypeAnyOf(MatchServer, Agent)) {
		C(MC_MATCH_REGISTERAGENT, "Match.RegisterAgent", "Register Agent to MatchServer", MCDT_MACHINE2MACHINE);
			P(MPT_STR, "Address");
			P(MPT_INT, "Port");
			P(MPT_INT, "UDPPort");
		C(MC_MATCH_UNREGISTERAGENT, "Match.UnRegisterAgent", "Unregister Agent from MatchServer", MCDT_MACHINE2MACHINE);
		C(MC_MATCH_AGENT_REQUEST_LIVECHECK, "Match.Agent.RequestLiveCheck", "Request LiveCheck for Agent", MCDT_MACHINE2MACHINE);
			P(MPT_UINT, "TimeStamp");
			P(MPT_UINT, "StageCount");
			P(MPT_UINT, "UserCount");
		C(MC_MATCH_AGENT_RESPONSE_LIVECHECK, "Match.Agent.ResponseLiveCheck", "Response LiveCheck for Agent", MCDT_MACHINE2MACHINE);
			P(MPT_UINT, "TimeStamp");
		C(MC_AGENT_STAGE_RESERVE, "Agent.StageReserve", "Reserve stage on AgentServer", MCDT_MACHINE2MACHINE);
			P(MPT_UID, "StageUID");
		C(MC_AGENT_STAGE_RELEASE, "Agent.StageRelease", "Release stage on AgentServer", MCDT_MACHINE2MACHINE);
			P(MPT_UID, "StageUID");
		C(MC_AGENT_STAGE_READY, "Agent.StageReady", "Ready to Handle stage", MCDT_MACHINE2MACHINE);
			P(MPT_UID, "StageUID");
		C(MC_AGENT_RELAY_PEER, "Agent.RelayPeer", "Let agent to Relay Peer", MCDT_MACHINE2MACHINE);
			P(MPT_UID, "PlayerUID");
			P(MPT_UID, "PeerCharUID");
			P(MPT_UID, "StageUID");
		C(MC_AGENT_PEER_READY, "Agent.PeerReady", "Ready to relay peer", MCDT_MACHINE2MACHINE);
			P(MPT_UID, "PlayerUID");
			P(MPT_UID, "PeerCharUID");
	}

	if (IsTypeAnyOf(MatchServer, Agent, Client)) {
		C(MC_AGENT_LOCATETO_CLIENT, "Agent.LocateToClient", "Locate Agent to Client", MCDT_MACHINE2MACHINE);
		P(MPT_UID, "AgentUID");
			P(MPT_STR, "Address");
			P(MPT_INT, "Port");
			P(MPT_INT, "UDPPort");
		C(MC_AGENT_RESPONSE_LOGIN, "Agent.ResponseLogin", "Response Login result to Client", MCDT_MACHINE2MACHINE);
		C(MC_AGENT_PEER_BINDTCP, "Agent.PeerBindTCP", "Bind Client to Peer by TCP", MCDT_MACHINE2MACHINE | MCCT_NON_ENCRYPTED);
			P(MPT_UID, "CharUID");
		C(MC_AGENT_PEER_BINDUDP, "Agent.PeerBindUDP", "Bind Client to Peer by UDP", MCDT_MACHINE2MACHINE | MCCT_NON_ENCRYPTED);
			P(MPT_UID, "CharUID");
			P(MPT_STR, "strLocalIP");
			P(MPT_UINT, "nLocalPort");
			P(MPT_STR, "strIP");
			P(MPT_UINT, "nPort");
		C(MC_AGENT_PEER_UNBIND, "Agent.PeerUnbind", "Unbind Client from Peer", MCDT_MACHINE2MACHINE | MCCT_NON_ENCRYPTED);
			P(MPT_UID, "CharUID");
		C(MC_AGENT_ERROR, "Agent.Error", "Error about Agent", MCDT_MACHINE2MACHINE | MCCT_NON_ENCRYPTED);
			P(MPT_INT, "ErrorCode");
		C(MC_AGENT_TUNNELING_TCP, "Agent.TunnelingTCP", "Tunneling TCP", MCDT_MACHINE2MACHINE | MCCT_NON_ENCRYPTED);
			P(MPT_UID, "SendUID");
			P(MPT_UID, "RecvUID");
			P(MPT_BLOB, "Data");
		C(MC_AGENT_TUNNELING_UDP, "Agent.TunnelingUDP", "Tunneling UDP", MCDT_PEER2PEER | MCCT_NON_ENCRYPTED);
			P(MPT_UID, "SendUID");
			P(MPT_UID, "RecvUID");
			P(MPT_BLOB, "Data");
		C(MC_AGENT_ALLOW_TUNNELING_TCP, "Agent.AllowTunnelingTCP", "Allow Tunneling by TCP", MCDT_MACHINE2MACHINE | MCCT_NON_ENCRYPTED);
		C(MC_AGENT_ALLOW_TUNNELING_UDP, "Agent.AllowTunnelingUDP", "Allow Tunneling by UDP", MCDT_MACHINE2MACHINE | MCCT_NON_ENCRYPTED);
		C(MC_AGENT_DEBUGPING, "Agent.DebugPing", "Debug Ping Test for Agent", MCDT_MACHINE2MACHINE | MCCT_NON_ENCRYPTED);
			P(MPT_UID, "TestUID");
		C(MC_AGENT_DEBUGTEST, "Agent.DebugTest", "Trigger Debug code for Agent", MCDT_MACHINE2MACHINE | MCCT_NON_ENCRYPTED);
			P(MPT_STR, "strMsg");
	}



	// MatchServer Schedule
	C( MC_MATCH_SCHEDULE_ANNOUNCE_MAKE, "Announce.", "make announce for scheduler.", MCDT_LOCAL );
		P( MPT_STR, "strAnnounce" );
	C( MC_MATCH_SCHEDULE_ANNOUNCE_SEND, "Announce.", "send announce of scheduler.", MCDT_MACHINE2MACHINE | MCCT_NON_ENCRYPTED );
		P( MPT_STR, "strAnnounce" );
	C( MC_MATCH_SCHEDULE_CLAN_SERVER_SWITCH_DOWN, "Switch clan server status.", "if server type is clan server, then this command is can chage server status.", MCDT_LOCAL );
	C( MC_MATCH_SCHEDULE_CLAN_SERVER_SWITCH_ON, "Switch clan server status.", "up", MCDT_LOCAL );

	C( MC_MATCH_SCHEDULE_STOP_SERVER, "MC_MATCH_SCHEDULE_STOP_SERVER", "stop server", MCDT_LOCAL );
		P( MPT_STR, "announce" );

	// Keeper Manager && Keeper, Keeper && Server.
	C( MC_KEEPER_MANAGER_CONNECT, "test", "test", MCDT_MACHINE2MACHINE | MCCT_NON_ENCRYPTED );
		P( MPT_INT, "test code" );
	C( MC_RESPONSE_KEEPER_MANAGER_CONNECT, "MC_RESPONSE_KEEPER_MANAGER_CONNECT", "connect", MCDT_MACHINE2MACHINE | MCCT_NON_ENCRYPTED );
		P( MPT_UID, "KeeperManagerUID" );

	C( MC_REQUEST_KEEPERMGR_ANNOUNCE, "MC_REQUEST_KEEPERMGR_ANNOUNCE", "keeper manager request to keeper do that announce to it's match server", MCDT_MACHINE2MACHINE | MCCT_NON_ENCRYPTED );
		P( MPT_STR, "announce" );

	C( MC_REQUEST_KEEPER_ANNOUNCE, "MC_REQUEST_KEEPER_ANNOUNCE", "keeper request to matchserver do that announce", MCDT_MACHINE2MACHINE | MCCT_NON_ENCRYPTED );
		P( MPT_STR, "announce" );

	C( MC_CHECK_KEEPER_MANAGER_PING, "Check ping", "Check ping", MCDT_MACHINE2MACHINE | MCCT_NON_ENCRYPTED );

	C( MC_REQUEST_MATCHSERVER_STATUS, "request matchserver status", "request matchserver status from keeper", MCDT_MACHINE2MACHINE | MCCT_NON_ENCRYPTED );

	C( MC_RESPONSE_MATCHSERVER_STATUS, "response matchserver ststus", "response matchserver status", MCDT_MACHINE2MACHINE | MCCT_NON_ENCRYPTED );
		P( MPT_INT, "is open DB" );
		P( MPT_STR, "server release version" );
		P( MPT_UCHAR, "connected agent count" );

	// 2005.06.02 ~ 
	C( MC_REQUEST_DOWNLOAD_SERVER_PATCH_FILE, "MC_REQUEST_DOWNLOAD_SERVER_PATCH_FILE", "request download server patch file", MCDT_MACHINE2MACHINE | MCCT_NON_ENCRYPTED );
	
	C( MC_REQUEST_STOP_SERVER, "MC_REQUEST_STOP_SERVER", "request stop server", MCDT_MACHINE2MACHINE | MCCT_NON_ENCRYPTED );
	
	C( MC_REQUEST_CONNECTION_STATE, "MC_REQUEST_CONNECTION_STATE", "request current server state", MCDT_MACHINE2MACHINE | MCCT_NON_ENCRYPTED );
	C( MC_RESPONSE_CONNECTION_STATE, "MC_RESPONSE_CONNECTION_STATE", "response current server state", MCDT_MACHINE2MACHINE | MCCT_NON_ENCRYPTED );
		P( MPT_INT, "resut" );
	C( MC_REQUEST_SERVER_HEARBEAT, "MC_REQUEST_SERVER_HEARBEAT", "request server hearbeat check", MCDT_MACHINE2MACHINE | MCCT_NON_ENCRYPTED );
	C( MC_RESPONSE_SERVER_HEARHEAT, "MC_RESPONSE_SERVER_HEARHEAT", "response server hearbeat check", MCDT_MACHINE2MACHINE | MCCT_NON_ENCRYPTED );

	C( MC_REQUEST_START_SERVER, "MC_REQUEST_START_SERVER", "request start match server", MCDT_MACHINE2MACHINE | MCCT_NON_ENCRYPTED );
	
	C( MC_REQUEST_KEEPER_CONNECT_MATCHSERVER, "MC_REQUEST_KEEPER_CONNECT_MATCHSERVER", "request keeper connect to match server", MCDT_MACHINE2MACHINE | MCCT_NON_ENCRYPTED );
	C( MC_RESPONSE_KEEPER_CONNECT_MATCHSERVER, "MC_RESPONSE_KEEPER_CONNECT_MATCHSERVER", "response keeper connect to match server", MCDT_MACHINE2MACHINE | MCCT_NON_ENCRYPTED );
		P( MPT_UID, "uid" );

	C( MC_REQUEST_REFRESH_SERVER, "MC_REQUEST_REFRESH_SERVER", "request check match server heartbeat", MCDT_MACHINE2MACHINE | MCCT_NON_ENCRYPTED );

	C( MC_REQUEST_PREPARE_SERVER_PATCH, "MC_REQUEST_PREPARE_SERVER_PATCH", "request prepare patch", MCDT_MACHINE2MACHINE | MCCT_NON_ENCRYPTED );
	
	C( MC_REQUEST_SERVER_PATCH, "MC_REQUEST_SERVER_PATCH", "request patching", MCDT_MACHINE2MACHINE | MCCT_NON_ENCRYPTED );
	
	C( MC_REQUEST_LAST_JOB_STATE, "MC_REQUEST_LAST_JOB_STATE", "request last job state", MCDT_MACHINE2MACHINE | MCCT_NON_ENCRYPTED );
	C( MC_RESPONSE_LAST_JOB_STATE, "MC_RESPONSE_LAST_JOB_STATE", "response last job state", MCDT_MACHINE2MACHINE | MCCT_NON_ENCRYPTED );
		P( MPT_INT, "job" );
		P( MPT_INT, "result" );

	C( MC_REQUEST_CONFIG_STATE, "MC_REQUEST_CONFIG_STATE", "request config state", MCDT_MACHINE2MACHINE | MCCT_NON_ENCRYPTED );
	C( MC_RESPONSE_CONFIG_STATE, "MC_RESPONSE_CONFIG_STATE", "response config state", MCDT_MACHINE2MACHINE | MCCT_NON_ENCRYPTED );
		P( MPT_INT, "server config" );
		P( MPT_INT, "agent config" );
		P( MPT_INT, "download config" );
		P( MPT_INT, "prepare config" );
		P( MPT_INT, "patch config" );

	C( MC_REQUEST_SET_ONE_CONFIG, "MC_REQUEST_SET_ONE_CONFIG", "reqeust set one config", MCDT_MACHINE2MACHINE | MCCT_NON_ENCRYPTED );
		P( MPT_INT, "config id" );
		P( MPT_INT, "confing state" );
	C( MC_RESPONSE_SET_ONE_CONFIG, "MC_RESPONSE_SET_ONE_CONFIG", "response set one config", MCDT_MACHINE2MACHINE | MCCT_NON_ENCRYPTED );
		P( MPT_INT, "config id" );
		P( MPT_INT, "confing state" );

	C( MC_REQUEST_STOP_AGENT_SERVER, "MC_REQUEST_STOP_AGENT_SERVER", "request stop agent", MCDT_MACHINE2MACHINE | MCCT_NON_ENCRYPTED );
    
    C( MC_REQUEST_START_AGENT_SERVER, "MC_REQUEST_START_AGENT_SERVER", "request start agent", MCDT_MACHINE2MACHINE | MCCT_NON_ENCRYPTED );
	
	C( MC_REQUEST_DOWNLOAD_AGENT_PATCH_FILE, "MC_REQUEST_DOWNLOAD_AGENT_PATCH_FILE", "request download agent patch file", MCDT_MACHINE2MACHINE | MCCT_NON_ENCRYPTED );
	
	C( MC_REQUEST_PREPARE_AGENT_PATCH, "MC_REQUEST_PREPARE_AGENT_PATCH", "request prepare agent patch", MCDT_MACHINE2MACHINE | MCCT_NON_ENCRYPTED );
	
    C( MC_REQUEST_AGENT_PATCH, "MC_REQUEST_AGENT_PATCH", "request agent patch", MCDT_MACHINE2MACHINE | MCCT_NON_ENCRYPTED );
	
	C( MC_REQUEST_RESET_PATCH, "MC_REQUEST_RESET_PATCH", "request reset patch job state", MCDT_MACHINE2MACHINE | MCCT_NON_ENCRYPTED );

	C( MC_REQUEST_DISCONNECT_SERVER, "MC_REQUEST_DISCONNECT_SERVER", "request disconnect match server", MCDT_MACHINE2MACHINE | MCCT_NON_ENCRYPTED );

	C( MC_REQUEST_REBOOT_WINDOWS, "MC_REQUEST_REBOOT_WINDOWS", "request restart windows", MCDT_MACHINE2MACHINE | MCCT_NON_ENCRYPTED );

	C( MC_REQUEST_ANNOUNCE_STOP_SERVER, "MC_REQUEST_ANNOUNCE_STOP_SERVER", "request stop server with administrator announce.", MCDT_MACHINE2MACHINE | MCCT_NON_ENCRYPTED );
	C( MC_RESPONSE_ANNOUNCE_STOP_SERVER, "MC_RESPONSE_ANNOUNCE_STOP_SERVER", "response stop server with administrator announce.", MCDT_MACHINE2MACHINE | MCCT_NON_ENCRYPTED );

	C( MC_REQUEST_KEEPER_MANAGER_SCHEDULE, "MC_REQUEST_KEEPER_MANAGER_SCHEDULE", "reqeust keeper manager schedule", MCDT_MACHINE2MACHINE | MCCT_NON_ENCRYPTED );
		P( MPT_INT, "type" );
		P( MPT_INT, "year" );
		P( MPT_INT, "month" );
		P( MPT_INT, "day" );
		P( MPT_INT, "hour" );
		P( MPT_INT, "min" );
		P( MPT_INT, "count" );
		P( MPT_INT, "command" );
		P( MPT_STR, "announce" );
	C( MC_RESPONSE_KEEPER_MANAGER_SCHEDULE, "MC_RESPONSE_KEEPER_MANAGER_SCHEDULE", "response keeper manager schedule", MCDT_MACHINE2MACHINE | MCCT_NON_ENCRYPTED );
		P( MPT_INT, "command type" );
		P( MPT_CHAR, "result" );

	C( MC_REQUEST_SERVER_AGENT_STATE, "MC_REQUEST_SERVER_AGENT_STATE", "request current state of server and agent", MCDT_MACHINE2MACHINE | MCCT_NON_ENCRYPTED );
	C( MC_RESPONSE_SERVER_AGENT_STATE, "MC_RESPONSE_SERVER_AGENT_STATE", "response current state of server and agent", MCDT_MACHINE2MACHINE | MCCT_NON_ENCRYPTED );
		P( MPT_INT, "server state" );
		P( MPT_INT, "agent state" );

	C( MC_REQUEST_SERVER_STATUS, "MC_REQUEST_SERVER_STATUS", "request server status", MCDT_MACHINE2MACHINE | MCCT_NON_ENCRYPTED );
	C( MC_RESPONSE_SERVER_STATUS, "MC_RESPONSE_SERVER_STATUS", "response server status resquest", MCDT_MACHINE2MACHINE | MCCT_NON_ENCRYPTED );
		P( MPT_INT, "is open DB" );
		P( MPT_STR, "server release vision" );
		P( MPT_STR, "server file was last modified" );
		P( MPT_STR, "agent file was last modified" );
		P( MPT_STR, "keeper file was last modified" );
		P( MPT_UINT64, "server file size" );
		P( MPT_UINT64, "agent file size" );
		P( MPT_UINT64, "keeper file size" );
		P( MPT_UCHAR, "agent count" );

	C( MC_REQUEST_START_SERVER_SCHEDULE, "MC_REQUEST_START_SERVER_SCHEDULE", "start server.", MCDT_LOCAL );

	C( MC_REQUEST_WRITE_CLIENT_CRC, "MC_REQUEST_WRITE_CLIENT_CRC", "request gunz client file crc32 checksum write on server.ini.", MCDT_MACHINE2MACHINE | MCCT_NON_ENCRYPTED );
		P( MPT_UINT, "gunz client file crc32 checksum" );
	C( MC_RESPONSE_WRITE_CLIENT_CRC, "MC_RESPONSE_WRITE_CLIENT_CRC", "response write client crc32 request", MCDT_MACHINE2MACHINE | MCCT_NON_ENCRYPTED );
		P( MPT_BOOL, "result" );
	C( MC_REQUEST_KEEPER_RELOAD_SERVER_CONFIG, "MC_REQUEST_KEEPER_RELOAD_SERVER_CONFIG", "keeper manager send to keeper request server reload server.ini file.", MCDT_MACHINE2MACHINE | MCCT_NON_ENCRYPTED );
		P( MPT_STR, "file list" );
	C( MC_REQUEST_RELOAD_CONFIG, "MC_REQUEST_RELOAD_CONFIG", "keeper request server reload server.ini file", MCDT_MACHINE2MACHINE | MCCT_NON_ENCRYPTED );
		P( MPT_STR, "file list" );
	C( MC_REQUEST_KEEPER_ADD_HASHMAP, "MC_REQUEST_KEEPER_ADD_HASHMAP", "", MCDT_MACHINE2MACHINE | MCCT_NON_ENCRYPTED );
		P( MPT_STR, "new hash value" );
	C( MC_RESPONSE_KEEPER_ADD_HASHMAP, "MC_RESPONSE_KEEPER_ADD_HASHMAP", "", MCDT_MACHINE2MACHINE | MCCT_NON_ENCRYPTED );
		P( MPT_BOOL, "result" );
	C( MC_REQUEST_ADD_HASHMAP, "MC_REQUEST_ADD_HASHMAP", "", MCDT_MACHINE2MACHINE | MCCT_NON_ENCRYPTED );
		P( MPT_STR, "new hash value" );
	C( MC_RESPONSE_ADD_HASHMAP, "MC_RESPONSE_ADD_HASHMAP", "", MCDT_MACHINE2MACHINE | MCCT_NON_ENCRYPTED );
		P( MPT_BOOL, "result" );


	C( MC_MATCH_REQUEST_CHAR_QUEST_ITEM_LIST, "Quest item", "Request my quest item list", MCDT_MACHINE2MACHINE );
		P( MPT_UID, "uid" );
	C( MC_MATCH_RESPONSE_CHAR_QUEST_ITEM_LIST, "Quest item", "Response my quest item list", MCDT_MACHINE2MACHINE );
		P( MPT_BLOB, "My quest item list" );

    C( MC_MATCH_REQUEST_BUY_QUEST_ITEM, "Quest item", "Request buy quest item", MCDT_MACHINE2MACHINE );
		P( MPT_UID, "uidChar" );
		P( MPT_INT, "QuestItemID" );
	C( MC_MATCH_RESPONSE_BUY_QUEST_ITEM, "Quest item", "Response buy quest item", MCDT_MACHINE2MACHINE );
		P( MPT_INT, "Result" );
		P( MPT_INT, "Player Bounty" );

	C( MC_MATCH_REQUEST_SELL_QUEST_ITEM, "Quest item", "Request sell quest item", MCDT_MACHINE2MACHINE );
		P( MPT_UID, "uidChar" );
		P( MPT_INT, "QuestItemID" );
		P( MPT_INT,	"Count" );
	C( MC_MATCH_RESPONSE_SELL_QUEST_ITEM, "Quest item", "Response sell quest item", MCDT_MACHINE2MACHINE );
		P( MPT_INT, "Result" );
		P( MPT_INT, "Player Bounty" );

	C( MC_MATCH_USER_REWARD_QUEST, "Quest", "Quest reward.", MCDT_MACHINE2MACHINE );
		P( MPT_INT, "XP" );
		P( MPT_INT, "Bounty" );
		P( MPT_BLOB, "Reward quest item" );
		P( MPT_BLOB, "Reward ZItem" );
		
	C( MC_MATCH_REQUEST_DROP_SACRIFICE_ITEM, "Quest", "Request drop sacrifice item", MCDT_MACHINE2MACHINE );
		P( MPT_UID, "Drop item owner" );
		P( MPT_INT, "Slot index" );
		P( MPT_INT, "ItemID" );
	C( MC_MATCH_RESPONSE_DROP_SACRIFICE_ITEM, "Quest", "Response drop sacrifice item", MCDT_MACHINE2MACHINE );
		P( MPT_INT, "Result" );
		P( MPT_UID, "requester of drop sacrifice item." );
		P( MPT_INT, "Slot index" );
		P( MPT_INT, "ItemID" );

	C( MC_MATCH_REQUEST_CALLBACK_SACRIFICE_ITEM, "Quest", "Request callback sacrifice item", MCDT_MACHINE2MACHINE );
		P( MPT_UID, "Callback item requester" );
		P( MPT_INT, "Slot index" );
		P( MPT_INT, "ItemID" );
	C( MC_MATCH_RESPONSE_CALLBACK_SACRIFICE_ITEM, "Quest", "Request callback sacrifice item", MCDT_MACHINE2MACHINE );
		P( MPT_INT, "Result" );
		P( MPT_UID, "requester of callback sacrifice item." );
		P( MPT_INT, "Slot index" );
		P( MPT_INT, "ItemID" );
	C( MC_MATCH_REQUEST_SLOT_INFO, "Quest", "Request slot info", MCDT_MACHINE2MACHINE );
		P( MPT_UID, "Sender" );
	C( MC_MATCH_RESPONSE_SLOT_INFO, "Quest", "Response slot info", MCDT_MACHINE2MACHINE );
		P( MPT_UID, "Owner 1" );
		P( MPT_INT, "ItemID 1" );
		P( MPT_UID, "Owner 2");
		P( MPT_INT, "ItemID 2" );

	C( MC_QUEST_REQUEST_QL, "Quest", "Request stage quest level", MCDT_MACHINE2MACHINE );
		P( MPT_UID, "sender" );
	C( MC_QUEST_RESPONSE_QL, "Quest", "Response stage quest level", MCDT_MACHINE2MACHINE );
		P( MPT_INT, "Quest level" );


	C( MC_GAME_START_FAIL, "Quest", "Failed start quest.", MCDT_MACHINE2MACHINE );
		P( MPT_INT, "Failed type." );
		P( MPT_INT, "Type state." );

	C( MC_MATCH_NEW_MONSTER_INFO, "monster info", "monster info", MCDT_MACHINE2MACHINE );
		P( MPT_CHAR, "monster db index" );

	C( MC_MATCH_REQUEST_MONSTER_BIBLE_INFO, "Quest", "request monster bible info", MCDT_MACHINE2MACHINE );
		P( MPT_UID, "requester" );

	C( MC_MATCH_RESPONSE_MONSTER_BIBLE_INFO, "Quest", "response monster bible info", MCDT_MACHINE2MACHINE );
		P( MPT_UID, "requester" );
		P( MPT_BLOB, "Monster bible info" );

	// Locator
	C( MC_REQUEST_SERVER_LIST_INFO, "MC_REQUEST_SERVER_LIST_INFO", "request connectable server list info.", MCDT_MACHINE2MACHINE | MCCT_NON_ENCRYPTED );
	C( MC_RESPONSE_SERVER_LIST_INFO, "MC_RESPONSE_SERVER_LIST_INFO", "response connectable server list info", MCDT_MACHINE2MACHINE | MCCT_NON_ENCRYPTED );
		P( MPT_BLOB, "server list" );
	C( MC_RESPONSE_BLOCK_COUNTRY_CODE_IP, "MC_RESPONSE_BLOCK_COUNTRY_CODE_IP", "response connected ip country code is blocked", MCDT_MACHINE2MACHINE | MCCT_NON_ENCRYPTED );
		P( MPT_STR, "Country code" );
		P( MPT_STR, "Routing URL" );

	// IP filter
	C( MC_RESPONSE_BLOCK_COUNTRYCODE, "MC_RESPONSE_BLOCK_COUNTRYCODE", "response block ip connected.", MCDT_MACHINE2MACHINE | MCCT_NON_ENCRYPTED );
		P( MPT_STR, "Comment" );
	C( MC_LOCAL_UPDATE_USE_COUNTRY_FILTER, "MC_LOCAL_UPDATE_USE_COUNTRY_FILTER", "update use country filter.", MCDT_LOCAL );

	C( MC_LOCAL_GET_DB_IP_TO_COUNTRY, "MC_LOCAL_GET_DB_IP_TO_COUNTRY", "get db ip to country code.", MCDT_LOCAL );
	C( MC_LOCAL_GET_DB_BLOCK_COUNTRY_CODE, "MC_LOCAL_GET_DB_BLOCK_COUNTRY_CODE", "get db block country code.", MCDT_LOCAL );
	C( MC_LOCAL_GET_DB_CUSTOM_IP, "MC_LOCAL_GET_DB_CUSTOM_IP", "get db custom ip.", MCDT_LOCAL );

	C( MC_LOCAL_UPDATE_IP_TO_COUNTRY, "MC_LOCAL_UPDAET_IP_TO_COUNTRY", "update ip to country code.", MCDT_LOCAL );
	C( MC_LOCAL_UPDATE_BLOCK_COUTRYCODE, "MC_LOCAL_UPDAET_BLOCK_COUTRYCODE", "update block country code.", MCDT_LOCAL );
	C( MC_LOCAL_UPDATE_CUSTOM_IP, "MC_LOCAL_UPDAET_CUSTOM_IP", "update custom ip.", MCDT_LOCAL );
	C( MC_LOCAL_UPDATE_ACCEPT_INVALID_IP, "MC_LOCAL_UPDATE_ACCEPT_INVALID_IP", "update accept invalid ip.", MCDT_LOCAL );

	C(MC_MATCH_ROUTE_UPDATE_STAGE_EQUIP_LOOK, "MC_MATCH_ROUTE_UPDATE_STAGE_EQUIP_LOOK",
		"route updated user equip info", MCDT_MACHINE2MACHINE);
		P(MPT_UID, "user uid");
		P(MPT_INT, "parts");
		P(MPT_INT, "itemid");
}
