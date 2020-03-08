#pragma once

#include <utility>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <memory>
#include "IDatabase.h"
#include "sqlite3.h"

class SQLiteStatement;

class SQLiteDatabase final : public IDatabase
{
public:
	SQLiteDatabase(const char* Filename = "GunzDB.sq3");


	//
	// Account creation and login
	//
	virtual bool CreateAccount(const char* szUserID,
		const char* szPassword,
		int nCert,
		const char* szName,
		int nAge,
		int nSex) override
	{ return false; }
	virtual AccountCreationResult CreateAccountNew(const char *szUsername,
		const char *szPasswordData, size_t PasswordSize, const char *szEmail) override;
	virtual bool GetLoginInfo(const char* szUserID, unsigned int* poutnAID, char* poutPassword, size_t maxlen) override;


	//
	// Character
	//
	virtual int CreateCharacter(int nAID,
		const char* szNewName,
		int nCharIndex,
		int nSex,
		int nHair,
		int nFace,
		int nCostume) override;
	virtual bool DeleteCharacter(const int nAID,
		const int nCharIndex,
		const char* szCharName) override;

	virtual bool GetAccountCharList(int nAID,
	struct MTD_AccountCharInfo* poutCharList,
		int* noutCharCount) override;
	virtual bool GetAccountCharInfo(int nAID, int nCharIndex, struct MTD_CharInfo* poutCharInfo) override;
	virtual bool GetAccountInfo(int AID,
	struct MMatchAccountInfo* outAccountInfo) override;

	virtual bool GetCharInfoByAID(int nAID,
		int nCharIndex,
	class MMatchCharInfo* poutCharInfo,
		int& nWaitHourDiff) override;
	virtual bool GetCharCID(const char* pszName, int* poutCID) override;

	virtual bool SimpleUpdateCharInfo(const MMatchCharInfo& CharInfo) override;
	virtual bool UpdateCharBP(int CID, int BPInc) override;
	virtual bool UpdateCharInfoData(int CID, int AddedXP, int AddedBP,
		int AddedKillCount, int AddedDeathCount) override;
	virtual bool UpdateCharLevel(int CID, int NewLevel) override;
	virtual bool UpdateCharLevel(int CID, int NewLevel, int BP, int KillCount,
		int DeathCount, int PlayTime, bool IsLevelUp) override;
	virtual bool GetCID(const char* pszCharName, int& outCID) override;
	virtual bool GetCharName(const int nCID, std::string& outCharName) override;


	//
	// Items
	//
	virtual bool InsertCharItem(unsigned int nCID, int nItemDescID, bool bRentItem,
		int nRentPeriodHour, u32* poutCIID) override;
	virtual bool DeleteCharItem(unsigned int nCID, int nCIID) override;
	virtual bool GetCharItemInfo(MMatchCharInfo& CharInfo) override;
	virtual bool GetAccountItemInfo(int nAID, struct MAccountItemNode* pOut, int* poutNodeCount,
		int nMaxNodeCount, MAccountItemNode* pOutExpiredItemList, int* poutExpiredItemCount,
		int nMaxExpiredItemCount) override;
	virtual bool UpdateEquipedItem(const u32 nCID,
	enum MMatchCharItemParts parts,
		u32 nCIID,
		u32 nItemID) override;
	virtual bool ClearAllEquipedItem(u32 nCID) override;
	virtual bool DeleteExpiredAccountItem(int nAIID) override;
	virtual bool BuyBountyItem(unsigned int CID, int ItemID, int Price, u32* outCIID) override;
	virtual bool SellBountyItem(unsigned int nCID, unsigned int nItemID, unsigned int nCIID,
		int nPrice, int nCharBP) override;


	//
	// Quest
	//
	virtual bool UpdateQuestItem(int nCID, class MQuestItemMap& rfQuestIteMap,
	class MQuestMonsterBible& rfQuestMonster) override;
	virtual bool GetCharQuestItemInfo(MMatchCharInfo* pCharInfo) override;


	//
	// Logging
	//
	virtual bool InsertQuestGameLog(const char* pszStageName,
		int nScenarioID,
		int nMasterCID, int nPlayer1, int nPlayer2, int nPlayer3,
		int nTotalRewardQItemCount,
		int nElapsedPlayTime,
		int& outQGLID) override;
	virtual bool InsertQUniqueGameLog(int nQGLID, int nCID, int nQIID) override;
	virtual bool InsertConnLog(int nAID, const char* szIP, const std::string& strCountryCode3) override;
	virtual bool InsertGameLog(const char* szGameName, const char* szMap, const char* GameType,
		int nRound, unsigned int nMasterCID,
		int nPlayerCount, const char* szPlayers) override;
	virtual bool InsertKillLog(unsigned int nAttackerCID, unsigned int nVictimCID) override;
	virtual bool InsertChatLog(u32 nCID, const char* szMsg, u64 nTime) override;
	virtual bool InsertServerLog(int nServerID, int nPlayerCount, int nGameCount,
		uint32_t dwBlockCount, uint32_t dwNonBlockCount) override;
	virtual bool InsertPlayerLog(u32 nCID,
		int nPlayTime, int nKillCount, int nDeathCount, int nXP, int nTotalXP) override;
	virtual bool InsertItemPurchaseLogByBounty(u32 nItemID, u32 nCID,
		int nBounty, int nCharBP, ItemPurchaseType nType) override
	{ return true; }
	virtual bool InsertLevelUpLog(int nCID, int nLevel, int nBP,
		int nKillCount, int nDeathCount, int nPlayTime) override;
	virtual bool UpdateLastConnDate(const char* szUserID, const char* szIP) override;


	//
	// Freestanding locator stuff. Unimplemented.
	//
	virtual bool UpdateServerStatus(int nServerID, int nPlayerCount) override { return true; }
	virtual bool UpdateMaxPlayer(int nServerID, int nMaxPlayer) override { return true; }
	virtual bool UpdateServerInfo(int nServerID, int nMaxPlayer, const char* szServerName) override { return true; }

	virtual bool UpdateCharPlayTime(u32 nCID, u32 nPlayTime) override;

	virtual bool InsertCharMakingLog(unsigned int nAID, const char* szCharName,
		CharMakingType nType) override;


	//
	// Cash items
	//
	virtual bool BringAccountItem(int nAID, int nCID, int nAIID,
		unsigned int* poutCIID, u32* poutItemID,
		bool* poutIsRentItem, int* poutRentMinutePeriodRemainder) override;

	virtual bool BringBackAccountItem(int nAID, int nCID, int nCIID) override;


	//
	// Friends
	//
	virtual bool FriendAdd(int nCID, int nFriendCID, int nFavorite) override;
	virtual bool FriendRemove(int nCID, int nFriendCID) override;
	virtual bool FriendGetList(int nCID, class MMatchFriendInfo* pFriendInfo) override;


	//
	// Clan
	//
	virtual bool GetCharClan(int nCID, int* poutClanID, char* poutClanName, int maxlen) override;
	virtual bool GetClanIDFromName(const char* szClanName, int* poutCLID) override;
	virtual bool CreateClan(const char* szClanName, int nMasterCID, int nMember1CID, int nMember2CID,
		int nMember3CID, int nMember4CID, bool* boutRet, int* noutNewCLID) override;
	virtual bool CreateClan(const char* szClanName, int nMasterCID, bool* boutRet, int* noutNewCLID) override;
	virtual bool DeleteExpiredClan(uint32_t dwCID, uint32_t dwCLID, const std::string& strDeleteName,
		uint32_t dwWaitHour = 24) override;
	virtual bool SetDeleteTime(uint32_t dwMasterCID, uint32_t dwCLID, const std::string& strDeleteDate) override;
	virtual bool ReserveCloseClan(const int nCLID, const char* szClanName, int nMasterCID,
		const std::string& strDeleteDate) override;
	virtual bool AddClanMember(int nCLID, int nJoinerCID, int nClanGrade, bool* boutRet) override;
	virtual bool RemoveClanMember(int nCLID, int nLeaverCID) override;
	virtual bool UpdateClanGrade(int nCLID, int nMemberCID, int nClanGrade) override;
	virtual ExpelResult ExpelClanMember(int CLID, int AdminGrade, const char* Member) override;
	virtual bool GetClanInfo(int nCLID, MDB_ClanInfo* poutClanInfo) override;
	virtual bool UpdateCharClanContPoint(int nCID, int nCLID, int nAddedContPoint) override;
	virtual bool CloseClan(int nCLID, const char* szClanName, int nMasterCID) override;


	//
	// Clan war
	//
	virtual bool GetLadderTeamID(const int nTeamTableIndex, const int* pnMemberCIDArray, int nMemberCount,
		int* pnoutTID) override;
	virtual bool LadderTeamWinTheGame(int nTeamTableIndex, int nWinnerTID, int nLoserTID,
		bool bIsDrawGame, int nWinnerPoint, int nLoserPoint, int nDrawPoint) override;
	virtual bool GetLadderTeamMemberByCID(const int nCID, int* poutTeamID, char** ppoutCharArray,
		int maxlen, int nCount) override;
	virtual bool WinTheClanGame(int nWinnerCLID, int nLoserCLID, bool bIsDrawGame,
		int nWinnerPoint, int nLoserPoint, const char* szWinnerClanName,
		const char* szLoserClanName, int nRoundWins, int nRoundLosses,
		int nMapID, int nGameType,
		const char* szWinnerMembers, const char* szLoserMembers) override;


	//
	// Admin
	//
	virtual bool BanPlayer(int nAID, const char *szReason, const time_t &UnbanTime) override;
	virtual bool InsertEvent(uint32_t dwAID, uint32_t dwCID, const std::string& strEventName) override;

	virtual bool SetBlockAccount(uint32_t dwAID,
		uint32_t dwCID,
		uint8_t btBlockType,
		const std::string& strComment,
		const std::string& strIP,
		const std::string& strEndHackBlockerDate) override;

	virtual bool ResetAccountBlock(uint32_t dwAID, uint8_t btBlockType) override;

	virtual bool InsertBlockLog(uint32_t dwAID, uint32_t dwCID, uint8_t btBlockType, const std::string& strComment,
		const std::string& strIP) override;

	virtual bool EventJjangUpdate(int nAID, bool bJjang) override;
	virtual bool CheckPremiumIP(const char* szIP, bool& outbResult) override;

	virtual bool AdminResetAllHackingBlock() override;

	virtual bool IsOpen() override { return true; }

private:
	friend class Transaction;

	template <typename T, int(*fn)(T*)>
	struct Deleter {
		void operator()(T* p) const {
			if (p)
				fn(p);
		}
	};
	template <typename T, int(*fn)(T*)>
	using unique_ptr_deleter = std::unique_ptr<T, Deleter<T, fn>>;

	using SQLitePtr = unique_ptr_deleter<sqlite3, sqlite3_close>;
	using SQLiteStatementPtr = unique_ptr_deleter<sqlite3_stmt, sqlite3_finalize>;

	template <size_t size>
	SQLiteStatementPtr PrepareStatement(const char(&sql)[size]);

	template <size_t size, typename... Args>
	SQLiteStatement ExecuteSQL(const char(&sql)[size], Args&&... args);

	void HandleException(const class SQLiteError& e);

	class Transaction
	{
	public:
		Transaction(SQLiteDatabase& DB) : DB(DB), Active(true) { }
		~Transaction()
		{
			if (DB.InTransaction && Active)
				DB.RollbackTransaction();
		}

		Transaction(Transaction&& Src) : DB(Src.DB) { Move(std::move(Src)); }
		Transaction operator=(Transaction&& Src) { 
		Move(std::move(Src));
		
		/// nonsense return to try to fix some linux compilation issue 
		//return Transaction(DB);
		}

	private:
		void Move(Transaction&& Src)
		{
			Active = Src.Active;
			Src.Active = false;
		}

		SQLiteDatabase& DB;
		bool Active = false;
	};

	Transaction BeginTransaction();
	void RollbackTransaction();
	void CommitTransaction();

	// Rows modified by the most recent INSERT, UPDATE or DELETE.
	int RowsModified();
	// Rows modified by any INSERT, UPDATE or DELETE in the entire lifetime of this instance.
	int TotalRowsModified();
	i64 LastInsertedRowID();

	void ReportWrongRowsModified(const char* ExpectedValue);

	void Log(const char* Format, ...);

	bool InTransaction = false;

	SQLitePtr sqlite;
	std::unordered_map<const char*, SQLiteStatementPtr> PreparedStatements;
};