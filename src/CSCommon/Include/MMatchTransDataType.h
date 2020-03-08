#ifndef _MMATCHTRANSDATATYPE_H
#define _MMATCHTRANSDATATYPE_H

#include "MMatchGlobal.h"
#include "MMatchItem.h"
#include "MMatchStageSetting.h"
#include "MMatchGameType.h"

#pragma pack(push, old)
#pragma pack(1)

struct MTD_AccountCharInfo
{
	char				szName[MATCHOBJECT_NAME_LENGTH];
	char				nCharNum;
	unsigned char		nLevel;
};


struct MTD_CharInfo
{
	char				szName[32];
	char				szClanName[CLAN_NAME_LENGTH];
	MMatchClanGrade		nClanGrade;
	unsigned short		nClanContPoint;
	char				nCharNum;
	unsigned short		nLevel;
	char				nSex;
	char				nHair;
	char				nFace;
	u32	nXP;
	int					nBP;
	float				fBonusRate;
	unsigned short		nPrize;
	unsigned short		nHP;
	unsigned short		nAP;
	unsigned short		nMaxWeight;
	unsigned short		nSafeFalls;
	unsigned short		nFR;
	unsigned short		nCR;
	unsigned short		nER;
	unsigned short		nWR;

	u32					nEquipedItemDesc[MMCIP_END];

	MMatchUserGradeID	nUGradeID;

	unsigned int		nClanCLID;
};

struct MTD_MyExtraCharInfo
{
	char	nLevelPercent;
};

struct MTD_SimpleCharInfo
{
	char				szName[32];
	char				nLevel;
	char				nSex;
	char				nHair;
	char				nFace;
	u32	nEquipedItemDesc[MMCIP_END];
};


struct MTD_MySimpleCharInfo
{
	unsigned char		nLevel;
	u32	nXP;
	int					nBP;
};

struct MTD_CharLevelInfo
{
	unsigned char		nLevel;
	u32	nCurrLevelExp;
	u32	nNextLevelExp;
};

struct MTD_RoundPeerInfo
{
	MUID			uidChar;
	unsigned char	nHP;
	unsigned char	nAP;
};

struct MTD_RoundKillInfo
{
	MUID	uidAttacker;
	MUID	uidVictim;
};

struct MTD_ItemNode
{
	MUID				uidItem;
	u32	nItemID;
	int					nRentMinutePeriodRemainder;		// 기간제 아이템 사용가능시간(초단위), RENT_MINUTE_PERIOD_UNLIMITED이면 무제한
};

struct MTD_AccountItemNode
{
	int					nAIID;
	u32	nItemID;
	int					nRentMinutePeriodRemainder;		// 기간제 아이템 사용가능시간(초단위), RENT_MINUTE_PERIOD_UNLIMITED이면 무제한
};

// 게임안 상태 정보
struct MTD_GameInfoPlayerItem
{
	MUID	uidPlayer;
	bool	bAlive;
	int		nKillCount;
	int		nDeathCount;
};

struct MTD_GameInfo
{
	char	nRedTeamScore;		// 팀전에서만 사용하는 레드팀정보
	char	nBlueTeamScore;		// 팀전에서만 사용하는 블루팀정보

	short	nRedTeamKills;		// 무한팀데스매치에서만 사용하는 레드팀킬수
	short	nBlueTeamKills;		// 무한팀데스매치에서만 사용하는 블루팀킬수
};

struct MTD_RuleInfo	
{
	unsigned char	nRuleType;
};

struct MTD_RuleInfo_Assassinate : public MTD_RuleInfo
{
	MUID	uidRedCommander;
	MUID	uidBlueCommander;
};

struct MTD_RuleInfo_Berserker : public MTD_RuleInfo
{
	MUID	uidBerserker;
};


enum MTD_PlayerFlags {
	MTD_PlayerFlags_AdminHide	= 1,
	MTD_PlayerFlags_BridgePeer	= 1<<1,
	MTD_PlayerFlags_Premium		= 1<<2,
	MTD_PlayerFlags_Bot			= 1<<3,
};

struct MTD_ChannelPlayerListNode 
{
	MUID			uidPlayer;
	char			szName[MATCHOBJECT_NAME_LENGTH];
	char			szClanName[CLAN_NAME_LENGTH];
	char			nLevel;
	MMatchPlace		nPlace;
	unsigned char	nGrade;			// 로비에서는 uid 로 캐릭터의 등급을 알수가 없어서..
	unsigned char	nPlayerFlags;	// 플레이어 속성(운영자숨김등) - MTD_PlayerFlags 사용
	unsigned int	nCLID;			// ClanID
	unsigned int	nEmblemChecksum;// Emblem Checksum
};


struct MTD_ClanMemberListNode 
{
	MUID				uidPlayer;
	char				szName[MATCHOBJECT_NAME_LENGTH];
	char				nLevel;
	MMatchClanGrade		nClanGrade;
	MMatchPlace			nPlace;
};

enum MTD_WorldItemSubType
{
	MTD_Dynamic = 0,
	MTD_Static  = 1,
};

// 아이템 스폰 정보
struct MTD_WorldItem
{
	unsigned short	nUID;
	unsigned short	nItemID;
	unsigned short  nItemSubType;
	short			x;
	short			y;
	short			z;
/*
	float			x;
	float			y;
	float			z;
*/
};


// 바로게임하기 필터링 정보
struct MTD_QuickJoinParam
{
	u32	nMapEnum;		// 원하는 맵의 비트어레이
	u32	nModeEnum;		// 윈하는 게임모드의 비트어레이
};


// 캐릭터의 클랜 업데이트 정보
struct MTD_CharClanInfo
{
	char				szClanName[CLAN_NAME_LENGTH];		// 클랜 이름
	MMatchClanGrade		nGrade;
};


// 유저 정보보기
struct MTD_CharInfo_Detail
{
	char				szName[32];						// 이름
	char				szClanName[CLAN_NAME_LENGTH];	// 클랜이름
	MMatchClanGrade		nClanGrade;						// 클랜직책
	int					nClanContPoint;					// 클랜 기여도
	unsigned short		nLevel;							// 레벨
	char				nSex;							// 성별
	char				nHair;							// 머리 코스츔
	char				nFace;							// 얼굴 코스츔
	u32	nXP;							// xp
	int					nBP;							// bp

	int					nKillCount;
	int					nDeathCount;

	// 접속상황

	u32	nTotalPlayTimeSec;				// 총 플레이 시간
	u32	nConnPlayTimeSec;				// 현재 접속 시간


	u32	nEquipedItemDesc[MMCIP_END];	// 아이템 정보

	MMatchUserGradeID	nUGradeID;						// account UGrade

	// ClanCLID
	unsigned int		nClanCLID;
};


/// 방 리스트 달라고 요청할때 보내는 구조체
struct MTD_StageListNode
{
	MUID			uidStage;							///< 방 UID
	unsigned char	nNo;								///< 방번호
	char			szStageName[STAGENAME_LENGTH];		///< 방이름
	char			nPlayers;							///< 현재인원
	char			nMaxPlayers;						///< 최대인원
	STAGE_STATE		nState;								///< 현재상태
	MMATCH_GAMETYPE nGameType;							///< 게임 타입
	char			nMapIndex;							///< 맵
	int				nSettingFlag;						///< 방 세팅 플래그(난입, 비밀방, 레벨제한)
	char			nMasterLevel;						///< 방장 레벨
	char			nLimitLevel;						///< 제한레벨
};

/// 클라이언트가 알아야할 기타정보 : AdminHide 상태를 명시적으로 교환 & 녹화파일에 저장
struct MTD_ExtendInfo
{
	char			nTeam;
	unsigned char	nPlayerFlags;	// 플레이어 속성(운영자숨김등) - MTD_PlayerFlags 사용
	unsigned char	nReserved1;		// 여분
	unsigned char	nReserved2;
};

struct MTD_PeerListNode
{
	MUID			uidChar;
	u32			dwIP;
	unsigned int	nPort;
	MTD_CharInfo	CharInfo;
	MTD_ExtendInfo	ExtendInfo;
};


// 동의 답변자
struct MTD_ReplierNode
{
	char szName[MATCHOBJECT_NAME_LENGTH];
};


// 래더 게임 신청 팀 그룹
struct MTD_LadderTeamMemberNode
{
	char szName[MATCHOBJECT_NAME_LENGTH];

};

// 클랜 정보
struct MTD_ClanInfo
{
	char				szClanName[CLAN_NAME_LENGTH];		// 클랜 이름
	short				nLevel;								// 레벨
	int					nPoint;								// 포인트
	int					nTotalPoint;						// 토탈포인트
	int					nRanking;							// 랭킹
	char				szMaster[MATCHOBJECT_NAME_LENGTH];	// 클랜 마스터
	unsigned short		nWins;								// 전적 - 승수
	unsigned short		nLosses;							// 전적 - 패수
	unsigned short		nTotalMemberCount;					// 전체 클랜원수
	unsigned short		nConnedMember;						// 현재 접속된 클랜원수
	unsigned int		nCLID;								// ClanID
	unsigned int		nEmblemChecksum;					// Emblem Checksum
};

// 클랜전 대기중인 클랜 리스트
struct MTD_StandbyClanList
{
	char				szClanName[CLAN_NAME_LENGTH];		// 클랜 이름
	short				nPlayers;							// 대기중인 인원수
	short				nLevel;								// 레벨
	int					nRanking;							// 랭킹 - 0이면 unranked
	unsigned int		nCLID;								// ClanID
	unsigned int		nEmblemChecksum;					// Emblem Checksum
};


// 퀘스트, 서바이벌의 게임 정보
struct MTD_QuestGameInfo
{
	unsigned short		nQL;												// 퀘스트 레벨
	float				fNPC_TC;											// NPC 난이도 조절 계수
	unsigned short		nNPCCount;											// 섹터당 등장할 NPC개수

	unsigned char		nNPCInfoCount;										// 등장할 NPC 종류 개수
	unsigned char		nNPCInfo[MAX_QUEST_NPC_INFO_COUNT];					// 등장할 NPC 정보
	unsigned short		nMapSectorCount;									// 맵 노드 개수
	unsigned short		nMapSectorID[MAX_QUEST_MAP_SECTOR_COUNT];			// 맵 노드 ID
	char				nMapSectorLinkIndex[MAX_QUEST_MAP_SECTOR_COUNT];	// 맵 노드의 Link Index

};

// 퀘스트, 보상 내용
struct MTD_QuestReward
{
	MUID				uidPlayer;	// 해당 플레이어 UID
	int					nXP;		// 해당 플레이어가 얻은 XP
	int					nBP;		// 해당 플레이어가 얻은 BP
};

// 퀘스트 아이템 보상 내용
struct MTD_QuestItemNode
{
	int		m_nItemID;
	int		m_nCount;
};

// 퀘스트 일반 아이템 보상 내용
struct MTD_QuestZItemNode
{
	unsigned int		m_nItemID;
	int					m_nRentPeriodHour;
};


struct MTD_ServerStatusInfo
{
	u32			m_dwIP;
	int				m_nPort;
	unsigned char	m_nServerID;
	short			m_nMaxPlayer;
	short			m_nCurPlayer;
	char			m_nType;
	bool			m_bIsLive;
};

struct MTD_ResetTeamMembersData
{
	MUID			m_uidPlayer;		// 해당 플레이어
	char			nTeam;				// 팀
};


// 듀얼 큐 정보

struct MTD_DuelQueueInfo
{
	MUID			m_uidChampion;
	MUID			m_uidChallenger;
	MUID			m_WaitQueue[14];				// 팀
	char			m_nQueueLength;
	char			m_nVictory;						// 연승수
	bool			m_bIsRoundEnd;					// 라운드 끝날때인가
};

struct MTD_GunGameWeaponInfo
{
	u32 WeaponIDs[5];
};

struct MTD_ClientSettings
{
	bool DebugOutput = false;
};

struct MTD_PingInfo
{
	MUID UID;
	uint16_t Ping;
};

struct MFRIENDLISTNODE {
	unsigned char	nState;
	char			szName[MATCHOBJECT_NAME_LENGTH];
	char			szDescription[MATCH_SIMPLE_DESC_LENGTH];
};

struct ZACTOR_BASICINFO {
	float			fTime;
	MUID			uidNPC;
	short			posx, posy, posz;
	short			velx, vely, velz;
	short			dirx, diry, dirz;
	u8			anistate;
};

struct ZPACKEDSHOTINFO {
	float	fTime;
	short	posx, posy, posz;
	short	tox, toy, toz;
	u8	sel_type;
};

struct ZPACKEDDASHINFO {
	short	posx, posy, posz;
	short	dirx, diry, dirz;
	u8	seltype;
};

#pragma pack(pop, old)


// admin 전용
enum ZAdminAnnounceType
{
	ZAAT_CHAT = 0,
	ZAAT_MSGBOX
};



/////////////////////////////////////////////////////////
void Make_MTDItemNode(MTD_ItemNode* pout, const MUID& uidItem, u32 nItemID, int nRentMinutePeriodRemainder);
void Make_MTDAccountItemNode(MTD_AccountItemNode* pout, int nAIID, u32 nItemID, int nRentMinutePeriodRemainder);

void Make_MTDQuestItemNode( MTD_QuestItemNode* pOut, const u32 nItemID, const int nCount );

// 경험치, 경험치 비율을 4byte로 조합
// 상위 2바이트는 경험치, 하위 2바이트는 경험치의 퍼센트이다.
inline u32 MakeExpTransData(int nAddedXP, int nPercent)
{
	u32 ret = 0;
	ret |= (nAddedXP & 0x0000FFFF) << 16;
	ret |= nPercent & 0xFFFF;
	return ret;
}
inline int GetExpFromTransData(u32 nValue)
{
	return (int)((nValue & 0xFFFF0000) >> 16);

}
inline int GetExpPercentFromTransData(u32 nValue)
{
	return (int)(nValue & 0x0000FFFF);
}



#endif
