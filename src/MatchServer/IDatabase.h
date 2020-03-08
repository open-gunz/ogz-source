#pragma once

#include <cstdint>
#include <string>
#include "MMatchGlobal.h"
#include "MMatchItem.h"
#include "MErrorTable.h"

enum class AccountCreationResult
{
	Success,
	UsernameAlreadyExists,
	DBError,
	EmailAlreadyExists,
};

enum class DatabaseType
{
	SQLite,
	MSSQL,
	Max,
};

inline const char* ToString(DatabaseType DBType)
{
	switch (DBType)
	{
	case DatabaseType::SQLite: return "SQLite";
	}
	assert(false);
	return nullptr;
}

enum class ItemPurchaseType
{
	Buy,
	Sell,
};
enum class CharMakingType
{
	Create,
	Delete,
};

enum class ExpelResult
{
	OK,
	NoSuchMember,
	TooLowGrade,
	DBError,
};

struct MDB_ClanInfo
{
	int		nCLID;
	char	szClanName[CLAN_NAME_LENGTH];
	int		nLevel;
	int		nRanking;
	int		nPoint;
	int		nTotalPoint;
	int		nWins;
	int		nLosses;
	int		nDraws;
	char	szMasterName[CLAN_NAME_LENGTH];
	int		nTotalMemberCount;
	char	szEmblemUrl[256];
	int		nEmblemChecksum;
};

class IDatabase
{
public:
	virtual bool IsOpen() = 0;

	template <size_t size>
	bool GetLoginInfo(const char* szUserID, unsigned int* poutnAID, char(&poutPassword)[size]) {
		return GetLoginInfo(szUserID, poutnAID, poutPassword, size); }
	virtual bool GetLoginInfo(const char* szUserID, unsigned int* poutnAID, char* poutPassword, size_t maxlen) = 0;
	virtual bool UpdateCharLevel(int nCID, int nLevel) = 0;
	virtual bool InsertLevelUpLog(int nCID, int nLevel, int nBP,
		int nKillCount, int nDeathCount, int nPlayTime) = 0;

	virtual bool UpdateLastConnDate(const char* szUserID, const char* szIP) = 0;

	virtual bool CreateAccount(const char* szUserID,
		const char* szPassword,
		int nCert,
		const char* szName,
		int nAge,
		int nSex) = 0;

	virtual AccountCreationResult CreateAccountNew(const char *Username,
		const char *PasswordData, size_t PasswordSize, const char *Email) = 0;

	virtual bool BanPlayer(int nAID, const char *szReason, const time_t &UnbanTime) = 0;

	virtual int CreateCharacter(int nAID,
		const char* szNewName,
		int nCharIndex,
		int nSex,
		int nHair,
		int nFace,
		int nCostume) = 0;
	virtual bool DeleteCharacter(const int nAID,
		const int nCharIndex,
		const char* szCharName) = 0;

	virtual bool GetAccountCharList(int nAID,
		struct MTD_AccountCharInfo* poutCharList,
		int* noutCharCount) = 0;
	virtual bool GetAccountCharInfo(int nAID, int nCharIndex, struct MTD_CharInfo* poutCharInfo) = 0;
	virtual bool GetAccountInfo(int AID,
		struct MMatchAccountInfo* outAccountInfo) = 0;

	virtual bool GetCharInfoByAID(int nAID,
		int nCharIndex,
		class MMatchCharInfo* poutCharInfo,
		int& nWaitHourDiff) = 0;

	virtual bool GetCharCID(const char* pszName, int* poutCID) = 0;

	virtual bool SimpleUpdateCharInfo(const MMatchCharInfo& CharInfo) = 0;

	virtual bool UpdateCharBP(int CID, int nBPInc) = 0;

	virtual bool UpdateCharInfoData(int CID, int AddedXP, int AddedBP,
		int AddedKillCount, int AddedDeathCount) = 0;

	virtual bool InsertCharItem(unsigned int nCID, int nItemDescID, bool bRentItem,
		int nRentPeriodHour, u32* poutCIID) = 0;
	virtual bool DeleteCharItem(unsigned int nCID, int nCIID) = 0;
	virtual bool GetCharItemInfo(MMatchCharInfo& CharInfo) = 0;
	virtual bool GetAccountItemInfo(int nAID, struct MAccountItemNode* pOut, int* poutNodeCount,
		int nMaxNodeCount, MAccountItemNode* pOutExpiredItemList, int* poutExpiredItemCount,
		int nMaxExpiredItemCount) = 0;
	virtual bool UpdateEquipedItem(const u32 nCID,
		MMatchCharItemParts parts,
		u32 nCIID,
		u32 nItemID) = 0;
	virtual bool ClearAllEquipedItem(u32 nCID) = 0;
	virtual bool DeleteExpiredAccountItem(int nAIID) = 0;
	virtual bool BuyBountyItem(unsigned int nCID, int nItemID, int nPrice, u32* poutCIID) = 0;
	virtual bool SellBountyItem(unsigned int nCID, unsigned int nItemID, unsigned int nCIID,
		int nPrice, int nCharBP) = 0;


	virtual bool UpdateQuestItem(int nCID, class MQuestItemMap& rfQuestIteMap,
	class MQuestMonsterBible& rfQuestMonster) = 0;
	virtual bool GetCharQuestItemInfo(MMatchCharInfo* pCharInfo) = 0;

	virtual bool InsertQuestGameLog(const char* pszStageName,
		int nScenarioID,
		int nMasterCID, int nPlayer1, int nPlayer2, int nPlayer3,
		int nTotalRewardQItemCount,
		int nElapsedPlayTime,
		int& outQGLID) = 0;

	virtual bool InsertQUniqueGameLog(int nQGLID, int nCID, int nQIID) = 0;


	virtual bool InsertConnLog(int nAID, const char* szIP, const std::string& strCountryCode3) = 0;
	virtual bool InsertGameLog(const char* szGameName, const char* szMap, const char* GameType,
		int nRound, unsigned int nMasterCID,
		int nPlayerCount, const char* szPlayers) = 0;
	virtual bool InsertKillLog(unsigned int nAttackerCID, unsigned int nVictimCID) = 0;
	virtual bool InsertChatLog(u32 nCID, const char* szMsg, u64 nTime) = 0;
	virtual bool InsertServerLog(int nServerID, int nPlayerCount, int nGameCount,
		uint32_t dwBlockCount, uint32_t dwNonBlockCount) = 0;
	virtual bool InsertPlayerLog(u32 nCID,
		int nPlayTime, int nKillCount, int nDeathCount, int nXP, int nTotalXP) = 0;

	virtual bool UpdateServerStatus(int nServerID, int nPlayerCount) = 0;
	virtual bool UpdateMaxPlayer(int nServerID, int nMaxPlayer) = 0;
	virtual bool UpdateServerInfo(int nServerID, int nMaxPlayer, const char* szServerName) = 0;
	virtual bool UpdateCharPlayTime(u32 nCID, u32 nPlayTime) = 0;

	virtual bool InsertItemPurchaseLogByBounty(u32 nItemID, u32 nCID,
		int nBounty, int nCharBP, ItemPurchaseType nType) = 0;

	virtual bool InsertCharMakingLog(unsigned int nAID, const char* szCharName,
		CharMakingType nType) = 0;

	virtual bool BringAccountItem(int nAID, int nCID, int nAIID,
		unsigned int* poutCIID, u32* poutItemID,
		bool* poutIsRentItem, int* poutRentMinutePeriodRemainder) = 0;

	virtual bool BringBackAccountItem(int nAID, int nCID, int nCIID) = 0;


	//// Friends ////
	virtual bool FriendAdd(int nCID, int nFriendCID, int nFavorite) = 0;
	virtual bool FriendRemove(int nCID, int nFriendCID) = 0;
	virtual bool FriendGetList(int nCID, class MMatchFriendInfo* pFriendInfo) = 0;


	//// Clan ////
	template<size_t size>
	bool GetCharClan(int nCID, int* poutClanID, char(&poutClanName)[size]) {
		return GetCharClan(nCID, poutClanID, poutClanName, size); }
	virtual bool GetCharClan(int nCID, int* poutClanID, char* poutClanName, int maxlen) = 0;
	virtual bool GetClanIDFromName(const char* szClanName, int* poutCLID) = 0;
	virtual bool CreateClan(const char* szClanName, int nMasterCID, int nMember1CID, int nMember2CID,
		int nMember3CID, int nMember4CID, bool* boutRet, int* noutNewCLID) = 0;
	virtual bool CreateClan(const char* szClanName, int nMasterCID, bool* boutRet, int* noutNewCLID) = 0;
	virtual bool DeleteExpiredClan(uint32_t dwCID, uint32_t dwCLID, const std::string& strDeleteName,
		uint32_t dwWaitHour = 24) = 0;
	virtual bool SetDeleteTime(uint32_t dwMasterCID, uint32_t dwCLID, const std::string& strDeleteDate) = 0;
	virtual bool ReserveCloseClan(const int nCLID, const char* szClanName, int nMasterCID,
		const std::string& strDeleteDate) = 0;
	virtual bool AddClanMember(int nCLID, int nJoinerCID, int nClanGrade, bool* boutRet) = 0;
	virtual bool RemoveClanMember(int nCLID, int nLeaverCID) = 0;
	virtual bool UpdateClanGrade(int nCLID, int nMemberCID, int nClanGrade) = 0;
	virtual ExpelResult ExpelClanMember(int nCLID, int nAdminGrade, const char* szMember) = 0;
	virtual bool GetClanInfo(int nCLID, MDB_ClanInfo* poutClanInfo) = 0;
	virtual bool UpdateCharClanContPoint(int nCID, int nCLID, int nAddedContPoint) = 0;
	virtual bool CloseClan(int nCLID, const char* szClanName, int nMasterCID) = 0;


	//// Ladder ////
	virtual bool GetLadderTeamID(const int nTeamTableIndex, const int* pnMemberCIDArray, int nMemberCount,
		int* pnoutTID) = 0;
	virtual bool LadderTeamWinTheGame(int nTeamTableIndex, int nWinnerTID, int nLoserTID,
		bool bIsDrawGame, int nWinnerPoint, int nLoserPoint, int nDrawPoint) = 0;
	template <size_t size>
	bool GetLadderTeamMemberByCID(const int nCID, int* poutTeamID, char *(&ppoutCharArray)[size], int nCount) {
		return GetLadderTeamMemberByCID(nCID, poutTeamID, ppoutCharArray, size, nCount) = 0; }
	virtual bool GetLadderTeamMemberByCID(const int nCID, int* poutTeamID, char** ppoutCharArray, int maxlen, int nCount) = 0;


	virtual bool WinTheClanGame(int nWinnerCLID, int nLoserCLID, bool bIsDrawGame,
		int nWinnerPoint, int nLoserPoint, const char* szWinnerClanName,
		const char* szLoserClanName, int nRoundWins, int nRoundLosses,
		int nMapID, int nGameType,
		const char* szWinnerMembers, const char* szLoserMembers) = 0;


	virtual bool UpdateCharLevel(int nCID, int nNewLevel, int nBP, int nKillCount,
		int nDeathCount, int nPlayTime, bool bIsLevelUp) = 0;

	virtual bool EventJjangUpdate(int nAID, bool bJjang) = 0;
	virtual bool CheckPremiumIP(const char* szIP, bool& outbResult) = 0;

	virtual bool GetCID(const char* pszCharName, int& outCID) = 0;
	virtual bool GetCharName(const int nCID, std::string& outCharName) = 0;

	virtual bool InsertEvent(uint32_t dwAID, uint32_t dwCID, const std::string& strEventName) = 0;

	virtual bool SetBlockAccount(uint32_t dwAID,
		uint32_t dwCID,
		uint8_t btBlockType,
		const std::string& strComment,
		const std::string& strIP,
		const std::string& strEndHackBlockerDate) = 0;

	virtual bool ResetAccountBlock(uint32_t dwAID, uint8_t btBlockType) = 0;

	virtual bool InsertBlockLog(uint32_t dwAID, uint32_t dwCID, uint8_t btBlockType, const std::string& strComment,
		const std::string& strIP) = 0;

	virtual bool AdminResetAllHackingBlock() = 0;
};
