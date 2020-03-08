#pragma once

#include "ZPrerequisites.h"
#include "MMatchClient.h"
#include "MSharedCommandTable.h"
#include "ZChannelRule.h"
#include "ZGame.h"
#include "ZNetAgreementBuilder.h"
#include "MEmblemMgr.h"
#include "SafeString.h"

using ZONCOMMANDCALLBACK = bool(MCommand* pCommand);
class MListBox;
class ZCharacterViewList;
class UPnP;

class ZGameClient : public MMatchClient
{
public:
	ZGameClient();
	virtual ~ZGameClient();

	void OnSpawnWorldItem(void* pBlob);
	void OnObtainWorldItem(const MUID& uidChar, const int nItemUID);
	void OnRemoveWorldItem(const int nItemUID);

	bool CreatedStage = false;
	MTD_ClientSettings ClientSettings;

	void PriorityBoost(bool bBoost);
	bool GetPriorityBoost() { return m_bPriorityBoost; }
	bool GetRejectNormalChat() { return m_bRejectNormalChat; }
	void SetRejectNormalChat(bool bVal) { m_bRejectNormalChat = bVal; }
	bool GetRejectTeamChat() { return m_bRejectTeamChat; }
	void SetRejectTeamChat(bool bVal) { m_bRejectTeamChat = bVal; }
	bool GetRejectClanChat() { return m_bRejectClanChat; }
	void SetRejectClanChat(bool bVal) { m_bRejectClanChat = bVal; }
	bool GetRejectWhisper() { return m_bRejectWhisper; }
	void SetRejectWhisper(bool bVal) { m_bRejectWhisper = bVal; }
	bool GetRejectInvite() { return m_bRejectInvite; }
	void SetRejectInvite(bool bVal) { m_bRejectInvite = bVal; }

	auto GetClockCount() const { return GetGlobalTimeMS(); }
	u64 GetGlobalClockCount() const;

	virtual void OutputMessage(const char* szMessage, MZMOMType nType = MZMDM_GENERAL) override;

	void SetOnCommandCallback(ZONCOMMANDCALLBACK pCallback) { m_fnOnCommandCallback = pCallback; }

	void Tick();
	void Disconnect() { MMatchClient::Disconnect(m_Server); }

	auto* GetMatchStageSetting() { return &m_MatchStageSetting; }
	auto* GetMatchStageSetting() const { return &m_MatchStageSetting; }
	bool IsForcedEntry() const { return m_bForcedEntry; }
	bool IsLadderGame() const { return m_bLadderGame; }
	void ReleaseForcedEntry();
	void ClearStageSetting();

	void RequestPrevStageList();
	void RequestNextStageList();
	void RequestStageList(int nPage);
	void StartStageList();
	void StopStageList();
	void StartChannelList(MCHANNEL_TYPE nChannelType);
	void StopChannelList();

	const char*		GetChannelName() const { return m_szChannel; }
	MCHANNEL_TYPE	GetChannelType() const { return m_CurrentChannelType; }
	const char*		GetChannelRuleName() const { return m_szChannelRule; }
	const char*		GetStageName() const { return m_szStageName; }
	int				GetStageNumber() const { return m_nRoomNo; }

	const char* GetChatRoomInvited() const { return m_szChatRoomInvited; }
	void SetChatRoomInvited(const char* pszRoomName) { strcpy_safe(m_szChatRoomInvited, pszRoomName); }

	bool AmIStageMaster() const { return (m_MatchStageSetting.GetMasterUID() == GetPlayerUID()); }

	const char* GetVoteMessage() const { return m_szVoteText; }

	void AnswerSponsorAgreement(bool bAnswer);
	void AnswerJoinerAgreement(bool bAnswer);
	void RequestCreateClan(char* szClanName, char** ppMemberCharNames);

	void RequestProposal(const MMatchProposalMode nProposalMode, char** ppReplierCharNames, const int nReplierCount);
	void ReplyAgreement(bool bAgreement);

	bool IsVoteInProgress() { return m_bVoteInProgress; }
	void SetVoteInProgress(bool bVal) { m_bVoteInProgress = bVal; }
	bool CanVote() { return m_bCanVote; }
	void SetCanVote(bool bVal) { m_bCanVote = bVal; }

	void RequestGameSuicide();
	void OnStageEnterBattle(const MUID& uidChar, MCmdEnterBattleParam nParam, MTD_PeerListNode* pPeerNode);
	int ValidateRequestDeleteChar();
	void RequestChannelJoin(const MUID& uidChannel);
	void RequestChannelJoin(const MCHANNEL_TYPE nChannelType, char* szChannelName);
	void RequestOnLobbyCreated();
	void RequestOnGameDestroyed();

	MEmblemMgr *GetEmblemManager() { return &m_EmblemMgr; }

	bool CreateUPnP(unsigned short nUDPPort);
	bool DestroyUPnP();

	int GetPingToServer() const { return PingToServer; }

private:
	friend void RunNetBotTunnelledCommand(MCommand* Command);

	void SetChannelRuleName(const char* pszName) { strcpy_safe(m_szChannelRule, pszName); }
	int GetBridgePeerCount()			{ return m_nBridgePeerCount; }
	void SetBridgePeerCount(int nCount)	{ m_nBridgePeerCount = nCount; }
	void UpdateBridgePeerTime(u32 nClock)	{ m_tmLastBridgePeer = nClock; }
	void StartBridgePeer();

	bool JustJoinedStage = false;
	MSTAGE_SETTING_NODE LastStageSetting;

	void UpdateStageSetting(MSTAGE_SETTING_NODE* pSetting, STAGE_STATE nStageState, const MUID& uidMaster);
	void SetCountdown(int nCountdown)	{ m_nCountdown = nCountdown; m_tmLastCountdown = 0; }
	int GetCountdown()					{ return m_nCountdown; }
	bool Countdown(int nClock) {
		if (nClock - m_tmLastCountdown > 1000) {
			m_nCountdown--;
			m_tmLastCountdown = nClock;
			return true;
		}
		return false;
	}

	static int FindListItem(MListBox* pListBox, const MUID& uid);

	ZONCOMMANDCALLBACK*		m_fnOnCommandCallback;
	
	bool					m_bIsBigGlobalClock;
	u32						m_nClockDistance;

	MMatchStageSetting		m_MatchStageSetting;
	bool					m_bForcedEntry;

	virtual bool OnCommand(MCommand* pCommand) override final;
	virtual bool OnSockDisconnect(SOCKET sock) override final;
	virtual bool OnSockConnect(SOCKET sock) override final;
	virtual void OnSockError(SOCKET sock, SOCKET_ERROR_EVENT ErrorEvent, int &ErrorCode) override final;
	virtual int OnConnected(SOCKET sock, MUID* pTargetUID, MUID* pAllocUID,
		u32 nTimeStamp) override final;
	virtual void OnRegisterCommand(MCommandManager* pCommandManager) override final;
	virtual void OnPrepareCommand(MCommand* pCommand) override final;
	virtual int OnResponseMatchLogin(const MUID& uidServer, int nResult, const char* szServerName,
		const MMatchServerMode nServerMode,
		const char* szAccountID, const MMatchUserGradeID nUGradeID,
		const MMatchPremiumGradeID nPGradeID, const MUID& uidPlayer,
		const char* szRandomValue) override final;
	virtual void OnBridgePeerACK(const MUID& uidChar, int nCode);
	virtual void OnObjectCache(unsigned int nType, void* pBlob, int nCount) override final;
	virtual void OnAgentError(int nError) override final;

	void OnMatchNotify(unsigned int nMsgID);
	void OnAnnounce(unsigned int nType, char* szMsg);
	void OnResponseResult(const int nResult);

	void OnChannelResponseJoin(const MUID& uidChannel, MCHANNEL_TYPE nChannelType,
		const char* szChannelName);
	void OnChannelChat(const MUID& uidChannel, const char* szName, const char* szChat, int nGrade);
	void OnChannelList(void* pBlob, int nCount);
	void OnChannelResponseRule(const MUID& uidchannel, const char* pszRuleName);

	void OnLadderPrepare(const MUID& uidStage, int nTeam);
	void OnLadderLaunch(const MUID& uidStage, const char* pszMapName);
	void OnLadderResponseChallenge(int nResult);

	void OnStageJoin(const MUID& uidChar, const MUID& uidStage, unsigned int nRoomNo, const char* szStageName);
	void OnStageLeave(const MUID& uidChar, const MUID& uidStage);
	void OnStageStart(const MUID& uidChar, const MUID& uidStage, int nCountdown);
	void OnStageLaunch(const MUID& uidStage, const char* pszMapName);
	void OnStageFinishGame(const MUID& uidStage);
	void OnStageMap(const MUID& uidStage, char* szMapName);
	void OnStageTeam(const MUID& uidChar, const MUID& uidStage, unsigned int nTeam);
	void OnStagePlayerState(const MUID& uidChar, const MUID& uidStage, MMatchObjectStageState nStageState);
	void OnStageMaster(const MUID& uidStage, const MUID& uidChar);
	void OnStageChat(const MUID& uidChar, const MUID& uidStage, const char* szChat);
	void OnStageList(int nPrevStageCount, int nNextStageCount, void* pBlob, int nCount);
	void OnResponseFriendList(void* pBlob, int nCount);
	void OnChannelPlayerList(int nTotalPlayerCount, int nPage, void* pBlob, int nCount);
	void OnChannelAllPlayerList(const MUID& uidChannel, void* pBlob, int nBlobCount);
	void OnResponseStageSetting(const MUID& uidStage, const void* pStageBlob, int nStageCount,
		const void* pCharBlob, int nCharCount, STAGE_STATE nStageState, const MUID& uidMaster);
	void OnResponseRecommandedChannel(const MUID& uidChannel);
	void OnResponsePeerRelay(const MUID& uidPeer);
	void OnResponseGameInfo(const MUID& uidStage, const void* pGameInfoBlob,
		const void* pRuleInfoBlob, const void* pPlayerInfoBlob);
	void OnResponseCharInfoDetail(const void* pBlob);

	void OnLoadingComplete(const MUID& uidChar, int nPercent);
	void OnForcedEntryToGame();

	void OnUserWhisper(char* pszSenderName, char* pszTargetName, char* pszMessage);
	void OnChatRoomJoin(char* pszPlayerName, char* pszChatRoomName);
	void OnChatRoomLeave(char* pszPlayerName, char* pszChatRoomName);
	void OnChatRoomSelectWrite(char* pszChatRoomName);
	void OnChatRoomInvite(char* pszSenderName, char* pszRoomName);
	void OnChatRoomChat(char* pszChatRoomName, char* pszPlayerName, char* pszChat);

	void OnResponseUpdateStageEquipLook(const MUID& uidPlayer, int nParts, int nItemID);

	void OnFollowResponse( const int nMsgID );
	void OnExpiredRentItem(void* pBlob);

	void OnResponseCreateClan(const int nResult, const int nRequestID);
	void OnResponseAgreedCreateClan(const int nResult);
	void OnClanAskSponsorAgreement(const int nRequestID, const char* szClanName, MUID& uidMasterObject, const char* szMasterName);
	void OnClanAnswerSponsorAgreement(const int nRequestID, const MUID& uidClanMaster, char* szSponsorCharName, const bool bAnswer);
	void OnClanResponseCloseClan(const int nResult);
	void OnClanResponseJoinClan(const int nResult);
	void OnClanAskJoinAgreement(const char* szClanName, MUID& uidClanAdmin, const char* szClanAdmin);
	void OnClanAnswerJoinAgreement(const MUID& uidClanAdmin, const char* szJoiner, const bool bAnswer);
	void OnClanResponseAgreedJoinClan(const int nResult);
	void OnClanUpdateCharClanInfo(void* pBlob);
	void OnClanResponseLeaveClan(const int nResult);
	void OnClanResponseChangeGrade(const int nResult);
	void OnClanResponseExpelMember(const int nResult);
	void OnClanMsg(const char* szSenderName, const char* szMsg);
	void OnClanMemberList(void* pBlob);
	void OnClanResponseClanInfo(void* pBlob);
	void OnClanStandbyClanList(int nPrevStageCount, int nNextStageCount, void* pBlob);
	void OnClanMemberConnected(const char* szMember);

	void OnResponseProposal(int nResult, MMatchProposalMode nProposalMode,
		int nRequestID);
	void OnAskAgreement(const MUID& uidProposer, 
		                void* pMemberNamesBlob, 
						MMatchProposalMode nProposalMode, 
						int nRequestID);
	void OnReplyAgreement(const MUID& uidProposer, 
		                  const MUID& uidChar, 
						  const char* szReplierName, 
						  const MMatchProposalMode nProposalMode,
					      int nRequestID, 
						  bool bAgreement);
	void ReplyAgreement(const MUID& uidProposer, const MMatchProposalMode nProposalMode, bool bAgreement);

	void OnGameLevelUp(const MUID& uidChar);
	void OnGameLevelDown(const MUID& uidChar);

	void OnLocalReport119();
	void OnAdminAnnounce(const char* szMsg, const ZAdminAnnounceType nType);

	void OnNotifyCallVote(const char* pszDiscuss, const char* pszArg);
	void OnNotifyVoteResult(const char* pszDiscuss, int nResult);
	void OnVoteAbort( const int nMsgCode );

	void OnBroadcastClanRenewVictories(const char* pszWinnerClanName, const char* pszLoserClanName,
		int nVictories);
	void OnBroadcastClanInterruptVictories(const char* pszWinnerClanName, const char* pszLoserClanName,
		int nVictories);
	void OnBroadcastDuelRenewVictories(const char* pszChampionName, const char* pszChannelName,
		int nRoomno, int nVictories);
	void OnBroadcastDuelInterruptVictories(const char* pszChampionName, const char* pszInterrupterName,
		int nVictories);

	void ProcessEmblem(unsigned int nCLID, unsigned int nChecksum);
	void RequestEmblemURL(unsigned int nCLID);
	void OnClanResponseEmblemURL(unsigned int nCLID, unsigned int nEmblemChecksum, const char* szEmblemURL);
	void OnClanEmblemReady(unsigned int nCLID, const char* szURL);

	virtual void OnStopUDPTest(const MUID& uid) override final;
	virtual void OnUDPTestReply(const MUID& uid) override final;

	UPnP *m_pUPnP;

	char				m_szChannel[256];
	char				m_szChannelRule[128];
	char				m_szStageName[256];
	char				m_szChatRoomInvited[64];
	unsigned int		m_nRoomNo;
	int					m_nStageCursor;
	bool				m_bLadderGame;
	MCHANNEL_TYPE		m_CurrentChannelType;
	char				m_szVoteText[256];

	u32		m_nPrevClockRequestAttribute;

	int						m_nBridgePeerCount;
	u32		m_tmLastBridgePeer;

	int						m_nCountdown;
	u32		m_tmLastCountdown;

	int						m_nRequestID;
	MUID					m_uidRequestPlayer;
	ZNetAgreementBuilder	m_AgreementBuilder;
	MMatchProposalMode		m_nProposalMode;

	bool					m_bVoteInProgress;
	bool					m_bCanVote;

	MEmblemMgr				m_EmblemMgr;

	// Priority boost = Increased Gunz process priority
	bool					m_bPriorityBoost;
	bool					m_bRejectNormalChat;
	bool					m_bRejectTeamChat;
	bool					m_bRejectClanChat;
	bool					m_bRejectWhisper;
	bool					m_bRejectInvite;

	int PingToServer = 0;
};

bool ZPostCommand(MCommand* pCmd);
MUID ZGetMyUID();	

MCommand* ZNewCmd(int nID);
bool GetUserInfoUID(MUID uid, MCOLOR& _color, char* sp_name, size_t maxlen, MMatchUserGradeID& gid);
template <size_t size>
bool GetUserInfoUID(MUID uid, MCOLOR& _color, char(&sp_name)[size], MMatchUserGradeID& gid) {
	return GetUserInfoUID(uid, _color, sp_name, size, gid);
}

u32 ZGetClockDistance(u32 nGlobalClock, u32 nLocalClock);

// Post Command Macro For Convenience
#define ZPOSTCMD0(_ID)									{ MCommand* pC=ZNewCmd(_ID); ZPostCommand(pC); }
#define ZPOSTCMD1(_ID, _P0)								{ MCommand* pC=ZNewCmd(_ID); pC->AddParameter(new _P0); ZPostCommand(pC); }
#define ZPOSTCMD2(_ID, _P0, _P1)						{ MCommand* pC=ZNewCmd(_ID); pC->AddParameter(new _P0); pC->AddParameter(new _P1); ZPostCommand(pC); }
#define ZPOSTCMD3(_ID, _P0, _P1, _P2)					{ MCommand* pC=ZNewCmd(_ID); pC->AddParameter(new _P0); pC->AddParameter(new _P1); pC->AddParameter(new _P2); ZPostCommand(pC); }
#define ZPOSTCMD4(_ID, _P0, _P1, _P2, _P3)				{ MCommand* pC=ZNewCmd(_ID); pC->AddParameter(new _P0); pC->AddParameter(new _P1); pC->AddParameter(new _P2); pC->AddParameter(new _P3); ZPostCommand(pC); }
#define ZPOSTCMD5(_ID, _P0, _P1, _P2, _P3, _P4)			{ MCommand* pC=ZNewCmd(_ID); pC->AddParameter(new _P0); pC->AddParameter(new _P1); pC->AddParameter(new _P2); pC->AddParameter(new _P3); pC->AddParameter(new _P4); ZPostCommand(pC); }
#define ZPOSTCMD6(_ID, _P0, _P1, _P2, _P3, _P4, _P5)	{ MCommand* pC=ZNewCmd(_ID); pC->AddParameter(new _P0); pC->AddParameter(new _P1); pC->AddParameter(new _P2); pC->AddParameter(new _P3); pC->AddParameter(new _P4); pC->AddParameter(new _P5); ZPostCommand(pC); }
#define ZPOSTCMD7(_ID, _P0, _P1, _P2, _P3, _P4, _P5, _P6)	{ MCommand* pC=ZNewCmd(_ID); pC->AddParameter(new _P0); pC->AddParameter(new _P1); pC->AddParameter(new _P2); pC->AddParameter(new _P3); pC->AddParameter(new _P4); pC->AddParameter(new _P5); pC->AddParameter(new _P6); ZPostCommand(pC); }

static inline void ZPostCmd_AddParameters(MCommand*) {}

template <typename CurT, typename... RestT>
void ZPostCmd_AddParameters(MCommand* NewCmd, CurT&& Cur, RestT&&... Rest)
{
	NewCmd->AddParameter(std::forward<CurT>(Cur).Clone());
	ZPostCmd_AddParameters(NewCmd, std::forward<RestT>(Rest)...);
}

template <typename... T>
void ZPostCmd(int ID, T&&... Args)
{
	auto NewCmd = ZNewCmd(ID);
	ZPostCmd_AddParameters(NewCmd, std::forward<T>(Args)...);
	ZPostCommand(NewCmd);
}

#define HANDLE_COMMAND(message, fn)    \
	case (message): return fn(pCommand);