#include "stdafx.h"
#include "SQLiteDatabase.h"
#include <algorithm>
#include "MDebug.h"
#include "MMatchObject.h"
#include "MErrorTable.h"
#include "MMatchTransDataType.h"
#include <cstdarg>
#include "MCRC32.h"


//
// SQLiteError
//

class SQLiteError : public std::runtime_error
{
public:
	SQLiteError(int err_code) : err_code(err_code), std::runtime_error("") {}
	SQLiteError(int err_code, const char* err_msg) : err_code(err_code), std::runtime_error(err_msg) {}
	SQLiteError(int err_code, const std::string& err_msg) : err_code(err_code), std::runtime_error(err_msg) {}

	auto GetErrorCode() const { return err_code; }

private:
	int err_code;
};


//
// BindParameter
//

template <typename T>
struct BindFunctionMap;

using long_equivalent = std::conditional_t<sizeof(long) == 32, int, long long>;

template <> struct BindFunctionMap<int> { static constexpr auto function = sqlite3_bind_int; };
template <> struct BindFunctionMap<long long> { static constexpr auto function = sqlite3_bind_int64; };
template <> struct BindFunctionMap<unsigned int> : BindFunctionMap<int> {};
template <> struct BindFunctionMap<unsigned long long> : BindFunctionMap<long long> {};
template <> struct BindFunctionMap<long> : BindFunctionMap<long_equivalent> {};
template <> struct BindFunctionMap<unsigned long> : BindFunctionMap<std::make_unsigned_t<long_equivalent>> {};

inline void BindParameter(sqlite3_stmt* stmt, int ParamNum) { }

// Enum
template <typename... Args, typename T>
std::enable_if_t<!std::is_enum<T>::value> BindParameter(sqlite3_stmt* stmt, int ParamNum,
	T Value, Args&&... args)
{
	BindFunctionMap<T>::function(stmt, ParamNum, Value);
	BindParameter(stmt, ParamNum + 1, std::forward<Args>(args)...);
}

// Non-enum
template <typename... Args, typename T>
std::enable_if_t<std::is_enum<T>::value> BindParameter(sqlite3_stmt* stmt, int ParamNum,
	T Value, Args&&... args)
{
	sqlite3_bind_int(stmt, ParamNum, static_cast<int>(Value));
	BindParameter(stmt, ParamNum + 1, std::forward<Args>(args)...);
}

// Text
template <typename... Args>
void BindParameter(sqlite3_stmt* stmt, int ParamNum, const char *Value, Args&&... args)
{
	sqlite3_bind_text(stmt, ParamNum, Value, -1, [](void*) {});
	BindParameter(stmt, ParamNum + 1, std::forward<Args>(args)...);
}

template <typename... Args>
void BindParameter(sqlite3_stmt* stmt, int ParamNum, const StringView& Value, Args&&... args)
{
	sqlite3_bind_text(stmt, ParamNum, Value.data(), Value.size(), [](void*) {});
	BindParameter(stmt, ParamNum + 1, std::forward<Args>(args)...);
}

struct Blob
{
	const void* Ptr;
	size_t Size;
};

// Blob
template <typename... Args>
void BindParameter(sqlite3_stmt* stmt, int ParamNum, const Blob& Value, Args&&... args)
{
	sqlite3_bind_blob(stmt, ParamNum, Value.Ptr, Value.Size, [](void*) {});
	BindParameter(stmt, ParamNum + 1, std::forward<Args>(args)...);
}


//
// SQLiteStatement
//

class SQLiteStatement
{
public:
	SQLiteStatement()
		: stmt(nullptr), bHasRow(false)
	{ }
	SQLiteStatement(sqlite3_stmt* stmt)
		: stmt(stmt), bHasRow(false)
	{ }

	~SQLiteStatement()
	{
		Reset();
	}

	SQLiteStatement(const SQLiteStatement&) = delete;
	SQLiteStatement& operator=(const SQLiteStatement&) = delete;

	SQLiteStatement(SQLiteStatement&& src)
		: stmt(nullptr), col(0)
	{
		Move(std::move(src));
	}
	SQLiteStatement& operator=(SQLiteStatement&& src)
	{
		Move(std::move(src));
		return *this;
	}

	operator sqlite3_stmt*() { return stmt; }

	template <typename T>
	T Get(int Column);

	template <typename T>
	T Get()
	{
		auto ret = Get<T>(col);
		col++;
		return ret;
	}

	template <typename T>
	T Get(const char* ColumnName)
	{
		for (int i = 1; i <= Columns(); i++)
		{
			if (strcmp(ColumnName, sqlite3_column_name(stmt, i)))
				continue;

			return Get<T>(i);
		}
	}

	bool IsNull(int Column)
	{
		return sqlite3_column_type(stmt, Column) == SQLITE_NULL;
	}

	bool IsNull() { return IsNull(col); }

	sqlite3_stmt*& GetStatement() { return stmt; }

	int Columns() { return sqlite3_data_count(stmt); }
	int ExpectedColumns() { return sqlite3_column_count(stmt); }

	int Step();

	bool HasRow() const { return bHasRow; }

	int NextColumn() { col++; return col; }

private:
	void Move(SQLiteStatement&& src)
	{
		Reset();
		stmt = src.stmt;
		col = src.col;
		bHasRow = src.bHasRow;
		src.stmt = nullptr;
	}

	void Reset()
	{
		if (stmt)
			sqlite3_reset(stmt);
	}

	sqlite3_stmt* stmt = nullptr;
	int col = 0;
	bool bHasRow = false;
};

template <>
int SQLiteStatement::Get<int>(int Column)
{
	return sqlite3_column_int(stmt, Column);
}

template <>
StringView SQLiteStatement::Get<StringView>(int Column)
{
	return{
		reinterpret_cast<const char*>(sqlite3_column_text(stmt, Column)),
		static_cast<size_t>(sqlite3_column_bytes(stmt, Column))
	};
}

template <>
Blob SQLiteStatement::Get<Blob>(int Column)
{
	Blob ret;
	ret.Ptr = sqlite3_column_blob(stmt, Column);
	ret.Size = sqlite3_column_bytes(stmt, Column);
	return ret;
}

int SQLiteStatement::Step()
{
	auto ret = sqlite3_step(stmt);
	bHasRow = ret == SQLITE_ROW;
	if (ret != SQLITE_ROW && ret != SQLITE_DONE)
		throw SQLiteError(ret);
	col = 0;
	return ret;
}

template <size_t size>
SQLiteDatabase::SQLiteStatementPtr SQLiteDatabase::PrepareStatement(const char(&sql)[size])
{
	SQLiteDatabase::SQLiteStatementPtr stmt;
	const char* tail = nullptr;

	sqlite3_stmt* temp_ptr = nullptr;
	auto err_code = sqlite3_prepare_v2(sqlite.get(), sql, size, &temp_ptr, &tail);
	if (err_code != SQLITE_OK)
		throw SQLiteError(err_code, std::string("Prepare threw ") + std::to_string(err_code) + ": "
			+ sqlite3_errmsg(sqlite.get()) + " on " + sql);

	stmt = decltype(stmt)(temp_ptr);

	return stmt;
}

template <size_t size, typename... Args>
SQLiteStatement SQLiteDatabase::ExecuteSQL(const char(&sql)[size], Args&&... args)
{
	auto it = PreparedStatements.find(sql);
	if (it == PreparedStatements.end())
		it = PreparedStatements.emplace(sql, PrepareStatement(sql)).first;

	auto stmt = SQLiteStatement{ it->second.get() };

	BindParameter(stmt, 1, std::forward<Args>(args)...);
	auto err_code = stmt.Step();
	if (err_code != SQLITE_DONE && err_code != SQLITE_ROW)
		throw SQLiteError(err_code, sqlite3_errmsg(sqlite.get()));

	return stmt;
}

struct ItemBlob
{
	int32_t CIIDs[MMCIP_END];
	int32_t ItemIDs[MMCIP_END];
};


//
// SQLiteDatabase definitions
//

static sqlite3* OpenSQLite(const char* Filename, int Timeout)
{
	sqlite3* ret;
	auto err_code = sqlite3_open(Filename, &ret);
	if (err_code != SQLITE_OK)
	{
		auto err_msg = std::string("sqlite3_open failed: error code: ")
			+ std::to_string(err_code) + ", error message: " + sqlite3_errmsg(ret);
		MLog("%s\n", err_msg.c_str());
		throw std::runtime_error(std::move(err_msg));
	}

	sqlite3_busy_timeout(ret, Timeout);

	return ret;
}

SQLiteDatabase::SQLiteDatabase(const char* Filename)
	: sqlite(OpenSQLite(Filename, 5000))
{

	auto exec = [&](const char* sql)
	{
		char *err_msg = nullptr;
		auto err_code = sqlite3_exec(sqlite.get(), sql, nullptr, nullptr, &err_msg);
		if (err_code != SQLITE_OK && err_msg)
			Log("Error during database construction: error code %d, error message: %s\n", err_code, err_msg);
	};

	exec("CREATE TABLE IF NOT EXISTS Login(AID integer NOT NULL, "
		"UserID text UNIQUE, "
		"PasswordData text, "
		"LastConnDate text, "
		"LastIP text)");

	exec("CREATE TABLE IF NOT EXISTS Account(AID integer PRIMARY KEY NOT NULL, "
		"UserID text UNIQUE, "
		"UGradeID integer, "
		"PGradeID integer, "
		"Email text, "
		"RegDate datetime)");

	exec("CREATE TABLE IF NOT EXISTS Character( "
		"CID integer PRIMARY KEY NOT NULL, "
		"AID integer NOT NULL, "
		"Name text NOT NULL, "
		"Level integer NOT NULL, "
		"Sex integer NOT NULL, "
		"CharNum integer NOT NULL, "
		"Hair integer NULL, "
		"Face integer NULL, "
		"XP integer NOT NULL, "
		"BP integer NOT NULL, "
		"Items blob NULL, "
		"RegDate text NULL, "
		"LastTime text NULL, "
		"PlayTime integer NULL, "
		"GameCount integer NULL, "
		"KillCount integer NULL, "
		"DeathCount integer NULL, "
		"DeleteFlag integer NULL, "
		"DeleteName text NULL, "
		"QuestItemInfo blob NULL)");

	exec("CREATE TABLE IF NOT EXISTS CharacterMakingLog( "
		"id integer PRIMARY KEY NOT NULL, "
		"AID integer NULL, "
		"CharName text NULL, "
		"Type text NULL, "
		"Date text NULL)");

	// clan win log
	exec("CREATE TABLE IF NOT EXISTS ClanGameLog( "
		"id integer PRIMARY KEY NOT NULL, "
		"WinnerCLID integer NULL, "
		"LoserCLID integer NULL, "
		"WinnerClanName text NULL, "
		"LoserClanName text NULL, "
		"RoundWins integer NULL, "
		"RoundLosses integer NULL, "
		"MapID text NULL, "
		"GameType integer NULL, "
		"RegDate datetime NULL, "
		"WinnerMembers text NULL, "
		"LoserMembers text NULL, "
		"WinnerPoint integer NULL, "
		"LoserPoint integer NULL)");

	exec("CREATE TABLE IF NOT EXISTS CharacterItem( "
		"CIID integer PRIMARY KEY NOT NULL, "
		"CID integer NULL, "
		"ItemID integer NOT NULL, "
		"RegDate integer NULL, "
		"RentDate integer NULL, "
		"RentHourPeriod integer NULL, "
		"Cnt integer NULL)");

	exec("CREATE TABLE IF NOT EXISTS Clan( "
		"CLID integer PRIMARY KEY NOT NULL, "
		"Name text NULL, "
		"Exp integer NOT NULL, "
		"Level integer NOT NULL, "
		"Point integer NOT NULL, "
		"MasterCID integer NULL, "
		"Wins integer NOT NULL, "
		"MarkWebImg text NULL, "
		"Introduction text NULL, "
		"RegDate text NOT NULL, "
		"DeleteFlag text NULL, "
		"DeleteName text NULL, "
		"Homepage text NULL, "
		"Losses integer NOT NULL, "
		"Draws integer NOT NULL, "
		"Ranking integer NOT NULL, "
		"TotalPoint integer NOT NULL, "
		"Cafe_Url text NULL, "
		"Email text NULL, "
		"EmblemUrl text NULL, "
		"RankIncrease integer NOT NULL, "
		"EmblemChecksum integer NOT NULL, "
		"LastDayRanking integer NOT NULL, "
		"LastMonthRanking integer NOT NULL)");

	exec("CREATE TABLE IF NOT EXISTS ClanMember( "
		"CMID integer PRIMARY KEY NOT NULL, "
		"CLID integer NULL, "
		"CID integer NULL, "
		"Grade integer NOT NULL, "
		"RegDate text NOT NULL, "
		"ContPoint integer NOT NULL)");

	exec("CREATE TABLE IF NOT EXISTS Friend( "
		"id integer PRIMARY KEY NOT NULL, "
		"CID integer NOT NULL, "
		"FriendCID integer NOT NULL, "
		"Type integer NOT NULL, "
		"Favorite integer NULL, "
		"DeleteFlag integer NULL)");
}

void SQLiteDatabase::HandleException(const SQLiteError & e)
{
	if (InTransaction)
		RollbackTransaction();

	Log("Caught SQLiteError: error code: %d, error message: %s\n", e.GetErrorCode(), e.what());
}

SQLiteDatabase::Transaction SQLiteDatabase::BeginTransaction()
{
	assert(!InTransaction);
	ExecuteSQL("BEGIN TRANSACTION");
	InTransaction = true;
	return Transaction(*this);
}

void SQLiteDatabase::RollbackTransaction()
{
	assert(InTransaction);
	ExecuteSQL("ROLLBACK TRANSACTION");
	InTransaction = false;
}

void SQLiteDatabase::CommitTransaction()
{
	assert(InTransaction);
	ExecuteSQL("COMMIT TRANSACTION");
	InTransaction = false;
}

int SQLiteDatabase::RowsModified()
{
	return sqlite3_changes(sqlite.get());
}

int SQLiteDatabase::TotalRowsModified()
{
	return sqlite3_total_changes(sqlite.get());
}

i64 SQLiteDatabase::LastInsertedRowID()
{
	return sqlite3_last_insert_rowid(sqlite.get());
}

void SQLiteDatabase::ReportWrongRowsModified(const char* ExpectedValue)
{
	const auto ActualValue = RowsModified();
	Log("Expected rows modified to be %s, actual value is %d\n", ExpectedValue, ActualValue);
	
	assert(false);
}

#define ASSERT_ROWS_MODIFIED_NOT_ZERO() \
	if (RowsModified() == 0) { ReportWrongRowsModified("not zero"); return false; }

void SQLiteDatabase::Log(const char * Format, ...)
{
	va_list va;
	va_start(va, Format);
	char buffer[512];
	vsprintf_safe(buffer, Format, va);
	va_end(va);

	MLog("%s", buffer);
}

bool SQLiteDatabase::GetLoginInfo(const char * UserID, unsigned int * outAID, char * outPassword, size_t maxlen)
try
{
	auto stmt = ExecuteSQL("SELECT AID, PasswordData FROM Login WHERE UserID = ?", UserID);

	if (!stmt.HasRow())
		return false;

	auto AID = stmt.Get<int>();
	auto PasswordData = stmt.Get<StringView>();
	strcpy_safe(outPassword, maxlen, PasswordData);
	*outAID = AID;

	return true;
}
catch (const SQLiteError& e)
{
	HandleException(e);
	return false;
}

AccountCreationResult SQLiteDatabase::CreateAccountNew(const char * Username,
	const char * PasswordData, size_t PasswordSize, const char * Email)
try
{
	auto stmt = ExecuteSQL("SELECT AID FROM Account WHERE UserID = ?", Username);

	if (stmt.HasRow())
		return AccountCreationResult::UsernameAlreadyExists;

	stmt = ExecuteSQL("SELECT AID FROM Account WHERE Email = ?", Email);

	if (stmt.HasRow())
		return AccountCreationResult::EmailAlreadyExists;

	auto Trans = BeginTransaction();

	stmt = ExecuteSQL("INSERT INTO Account (UserID, UGradeID, PGradeID, RegDate, Email) "
		"VALUES (?, 0, 0, date('now'), ?)",
		Username, Email);
	stmt = ExecuteSQL("SELECT AID FROM Account WHERE UserID = ?",
		Username);

	if (!stmt.HasRow())
		return AccountCreationResult::DBError;

	auto AID = stmt.Get<int>(0);
	stmt = ExecuteSQL("INSERT INTO Login(UserID, AID, PasswordData) VALUES(?, ?, ?)",
		Username, AID, StringView{ PasswordData, PasswordSize });

	CommitTransaction();

	return AccountCreationResult::Success;
}
catch (const SQLiteError& e)
{
	HandleException(e);
	return AccountCreationResult::DBError;
}

bool SQLiteDatabase::UpdateCharLevel(int CID, int Level)
try
{
	auto stmt = ExecuteSQL("UPDATE Character SET Level = ? WHERE CID = ?", Level, CID);

	return true;
}
catch (const SQLiteError& e)
{
	HandleException(e);
	return false;
}

bool SQLiteDatabase::InsertLevelUpLog(int nCID, int nLevel, int nBP, int nKillCount, int nDeathCount, int nPlayTime)
{
	return true;
}

bool SQLiteDatabase::UpdateLastConnDate(const char * UserID, const char * IP)
try
{
	auto stmt = ExecuteSQL("UPDATE Login SET LastConnDate = date('now'), "
		"LastIP = ? WHERE UserID = ?", IP, UserID);

	return true;
}
catch (const SQLiteError& e)
{
	HandleException(e);
	return false;
}

bool SQLiteDatabase::BanPlayer(int nAID, const char* Reason, const time_t& UnbanTime)
try
{
	auto Trans = BeginTransaction();

	auto stmt = ExecuteSQL("UPDATE Account SET UGradeID = ? WHERE AID = ?", MMUG_BLOCKED, nAID);
	ExecuteSQL("INSERT INTO Blocks (AID, Type, Reason, EndDate) VALUES (?, ?, ?, ?)",
		nAID, MMBT_BANNED, Reason, UnbanTime);

	CommitTransaction();

	return true;
}
catch (const SQLiteError& e)
{
	HandleException(e);
	return false;
}

int SQLiteDatabase::CreateCharacter(int AID, const char * NewName, int CharIndex, int Sex, int Hair, int Face, int Costume)
try
{

	auto stmt = ExecuteSQL("SELECT COUNT(*) AS NUM FROM Character WHERE Name = ?", NewName);

	if (!stmt.HasRow())
		return MERR_UNKNOWN;

	if (stmt.Get<int>() > 0)
		return MERR_CLIENT_EXIST_CHARNAME;

	auto Trans = BeginTransaction();

	ExecuteSQL("INSERT INTO Character (AID, Name, CharNum, Level, Sex, Hair, Face, XP, BP, "
		"GameCount, KillCount, DeathCount, RegDate, PlayTime, DeleteFlag) "
		"Values(?, ?, ?, 1, ?, ?, ?, 0, 0, "
		"0, 0, 0, date('now'), 0, 0)",
		AID, NewName, CharIndex, Sex, Hair, Face);

	auto CID = LastInsertedRowID();

	std::pair<int, int> Clothes[2][2] = { { { MMCIP_CHEST, 21001 },{ MMCIP_LEGS, 23001 }},
	{{ MMCIP_CHEST, 21501 },{ MMCIP_LEGS, 23501 }} };
	std::pair<int, int> Weapons[] = { { MMCIP_MELEE, 2 }, { MMCIP_PRIMARY, 5002 } };

	ItemBlob Items{};

	auto SetItem = [&](auto& Parts)
	{
		size_t Index = Parts.first;
		auto ItemID = Parts.second;
		Items.ItemIDs[Index] = ItemID;
		ExecuteSQL("INSERT INTO CharacterItem (CID, ItemID) VALUES (?, ?)", CID, ItemID);
		auto CIID = static_cast<i32>(LastInsertedRowID());
		Items.CIIDs[Index] = CIID;
	};

	for (auto& Pair : Clothes[Sex])
		SetItem(Pair);
	for (auto& Pair : Weapons)
		SetItem(Pair);

	ExecuteSQL("UPDATE Character SET Items = ? WHERE AID = ? AND CID = ?",
		Blob{ &Items, sizeof(Items) }, AID, CID);

	CommitTransaction();

	return MOK;
}
catch (const SQLiteError& e)
{
	HandleException(e);
	return false;
}

bool SQLiteDatabase::DeleteCharacter(int AID, int CharIndex, const char * CharName)
try
{
	auto stmt = ExecuteSQL("SELECT CID FROM Character WHERE AID = ? AND CharNum = ?", AID, CharIndex);

	if (!stmt.HasRow())
		return false;

	auto CID = stmt.Get<int>();

	stmt = ExecuteSQL("SELECT COUNT(*) AS CashItemCount FROM CharacterItem "
		"WHERE CID = ? AND ItemID >= 500000", CID);

	if (stmt.HasRow())
	{
		auto CashItemCount = stmt.Get<int>();

		if (CashItemCount > 0)
			return false;
	}

	stmt = ExecuteSQL("UPDATE Character SET CharNum = -1, DeleteFlag = 1, Name = '', DeleteName = ?"
		"WHERE AID = ? AND CharNum = ?",
		CharName, AID, CharIndex);

	InsertCharMakingLog(AID, CharName, CharMakingType::Delete);

	return true;
}
catch (const SQLiteError& e)
{
	HandleException(e);
	return false;
}

bool SQLiteDatabase::InsertCharMakingLog(unsigned int AID, const char * CharName, CharMakingType Type)
try
{
	ExecuteSQL("INSERT INTO CharacterMakingLog(AID, CharName, Type, Date) "
		"VALUES(?, ?, ?, date('now'))",
		AID, CharName, Type);

	return true;
}
catch (const SQLiteError& e)
{
	HandleException(e);
	return false;
}

bool SQLiteDatabase::GetAccountCharList(int AID, MTD_AccountCharInfo * outCharList, int * outCharCount)
try
{
	auto stmt = ExecuteSQL("SELECT Name, CharNum, Level "
		"FROM Character "
		"WHERE AID = ? AND DeleteFlag = 0",
		AID);

	int i = 0;
	while (stmt.HasRow() && i < MAX_CHAR_COUNT)
	{
		strcpy_safe(outCharList[i].szName, stmt.Get<StringView>());
		outCharList[i].nCharNum = stmt.Get<int>();
		outCharList[i].nLevel = stmt.Get<int>();
		stmt.Step();
		i++;
	}

	*outCharCount = i;

	return true;
}
catch (const SQLiteError& e)
{
	HandleException(e);
	return false;
}

bool SQLiteDatabase::GetAccountCharInfo(int AID, int CharIndex, MTD_CharInfo * outCharInfo)
try
{
	auto stmt = ExecuteSQL("SELECT CID FROM Character WHERE AID = ? and CharNum = ?",
		AID, CharIndex);

	if (!stmt.HasRow())
		return false;

	auto CID = stmt.Get<int>();

	stmt = ExecuteSQL("SELECT Name, CharNum, Level, Sex, Hair, Face, XP, BP, "
		"(SELECT cl.Name FROM Clan cl, ClanMember cm WHERE cm.cid = ?1 AND cm.CLID = cl.CLID) AS ClanName, "
		"Items "
		"FROM Character "
		"WHERE CID = ?1",
		CID);

	if (!stmt.HasRow())
		return false;

	strcpy_safe(outCharInfo->szName, stmt.Get<StringView>());
	outCharInfo->nCharNum = stmt.Get<int>();
	outCharInfo->nLevel = stmt.Get<int>();
	outCharInfo->nSex = stmt.Get<int>();
	outCharInfo->nHair = stmt.Get<int>();
	outCharInfo->nFace = stmt.Get<int>();
	outCharInfo->nXP = stmt.Get<int>();
	outCharInfo->nBP = stmt.Get<int>();

	if (!stmt.IsNull())
	{
		strcpy_safe(outCharInfo->szClanName, stmt.Get<StringView>());
	}
	else
	{
		std::fill(std::begin(outCharInfo->szClanName), std::end(outCharInfo->szClanName), 0);

		stmt.NextColumn();
	}

	if (!stmt.IsNull())
	{
		auto ItemsBlob = stmt.Get<Blob>();
		auto& Items = *static_cast<const ItemBlob*>(ItemsBlob.Ptr);
		std::copy(std::begin(Items.ItemIDs), std::end(Items.ItemIDs), outCharInfo->nEquipedItemDesc);
	}
	else
	{
		std::fill(std::begin(outCharInfo->nEquipedItemDesc), std::end(outCharInfo->nEquipedItemDesc), 0);

		stmt.NextColumn();
	}

	return true;
}
catch (const SQLiteError& e)
{
	HandleException(e);
	return false;
}

bool SQLiteDatabase::GetAccountInfo(int AID, MMatchAccountInfo * outAccountInfo)
try
{
	auto stmt = ExecuteSQL("SELECT UserID, UGradeID "
		"FROM Account WHERE AID = ?",
		AID);

	if (!stmt.HasRow())
		return false;

	outAccountInfo->m_nAID = AID;
	strcpy_safe(outAccountInfo->m_szUserID, stmt.Get<StringView>());
	outAccountInfo->m_nUGrade = static_cast<MMatchUserGradeID>(stmt.Get<int>());

	return true;
}
catch (const SQLiteError& e)
{
	HandleException(e);
	return false;
}

bool SQLiteDatabase::GetCharInfoByAID(int AID, int CharIndex, MMatchCharInfo * outCharInfo, int & nWaitHourDiff)
try
{
	auto stmt = ExecuteSQL("SELECT CID FROM Character WHERE AID = ? and CharNum = ?",
		AID, CharIndex);

	if (!stmt.HasRow())
		return false;

	auto CID = stmt.Get<int>();

	stmt = ExecuteSQL("SELECT Name, Level, Sex, CharNum, Hair, Face, "
		"XP, BP, GameCount, KillCount, DeathCount, PlayTime, Items "
		"FROM Character "
		"WHERE CID = ?",
		CID);

	if (!stmt.HasRow())
		return false;

	outCharInfo->m_nCID = CID;
	if (!stmt.IsNull())
		strcpy_safe(outCharInfo->m_szName, stmt.Get<StringView>());
	else
		stmt.NextColumn();
	outCharInfo->m_nLevel = stmt.Get<int>();
	outCharInfo->m_nSex = static_cast<MMatchSex>(stmt.Get<int>());
	outCharInfo->m_nCharNum = stmt.Get<int>();
	outCharInfo->m_nHair = stmt.Get<int>();
	outCharInfo->m_nFace = stmt.Get<int>();
	outCharInfo->m_nXP = stmt.Get<int>();
	outCharInfo->m_nBP = stmt.Get<int>();
	stmt.NextColumn();
	outCharInfo->m_nTotalKillCount = stmt.Get<int>();
	outCharInfo->m_nTotalDeathCount = stmt.Get<int>();
	outCharInfo->m_nTotalPlayTimeSec = stmt.Get<int>();
	if (!stmt.IsNull())
	{
		auto ItemsBlob = stmt.Get<Blob>();
		auto& Items = *reinterpret_cast<const ItemBlob*>(ItemsBlob.Ptr);
		std::copy(std::begin(Items.CIIDs), std::end(Items.CIIDs), outCharInfo->m_nEquipedItemCIID);
	}
	else
	{
		std::fill(std::begin(outCharInfo->m_nEquipedItemCIID), std::end(outCharInfo->m_nEquipedItemCIID), 0);

		stmt.NextColumn();
	}

	stmt = ExecuteSQL("SELECT cl.CLID, cl.Name, cm.Grade, cm.ContPoint "
		"FROM ClanMember cm, Clan cl "
		"WHERE cm.cid = ? AND cm.CLID = cl.CLID",
		CID);

	if (stmt.HasRow())
	{
		outCharInfo->m_ClanInfo.m_nClanID = stmt.Get<int>();
		if (!stmt.IsNull())
		{
			strcpy_safe(outCharInfo->m_ClanInfo.m_szClanName, stmt.Get<StringView>());
		}
		else
		{
			for (auto& e : outCharInfo->m_ClanInfo.m_szClanName)
				e = 0;
			stmt.NextColumn();
		}
		outCharInfo->m_ClanInfo.m_nGrade = static_cast<MMatchClanGrade>(stmt.Get<int>());
		outCharInfo->m_ClanInfo.m_nContPoint = stmt.Get<int>();
	}
	else
	{
		outCharInfo->m_ClanInfo.m_nClanID = 0;
		for (auto& c : outCharInfo->m_ClanInfo.m_szClanName)
			c = 0;
		outCharInfo->m_ClanInfo.m_nGrade = MCG_NONE;
		outCharInfo->m_ClanInfo.m_nContPoint = 0;
	}

	return true;
}
catch (const SQLiteError& e)
{
	HandleException(e);
	return false;
}

bool SQLiteDatabase::GetCharCID(const char * Name, int * outCID)
try
{
	auto stmt = ExecuteSQL("SELECT CID "
		"FROM Character "
		"WHERE Name = ?",
		Name);

	if (!stmt.HasRow())
		return false;

	*outCID = stmt.Get<int>();

	return true;
}
catch (const SQLiteError& e)
{
	HandleException(e);
	return false;
}

bool SQLiteDatabase::SimpleUpdateCharInfo(const MMatchCharInfo& CharInfo)
try
{
	ExecuteSQL("UPDATE Character "
		"SET Level = ?, XP = ?, BP = ? "
		"WHERE CID = ?",
		CharInfo.m_nLevel, CharInfo.m_nXP, CharInfo.m_nBP,
		CharInfo.m_nCID);

	return true;
}
catch (const SQLiteError& e)
{
	HandleException(e);
	return false;
}

bool SQLiteDatabase::UpdateCharBP(int CID, int BPInc)
try
{
	ExecuteSQL("UPDATE Character "
		"SET BP = BP + ? "
		"WHERE CID = ?",
		BPInc, CID);

	return true;
}
catch (const SQLiteError& e)
{
	HandleException(e);
	return false;
}

bool SQLiteDatabase::UpdateCharInfoData(int CID, int AddedXP, int AddedBP, int AddedKillCount, int AddedDeathCount)
try
{
	ExecuteSQL("UPDATE Character "
		"SET XP = XP + ?, BP = BP + ?, KillCount = KillCount + ?, DeathCount = DeathCount + ? "
		"WHERE CID = ?",
		AddedXP, AddedBP, AddedKillCount, AddedDeathCount,
		CID);

	return true;
}
catch (const SQLiteError& e)
{
	HandleException(e);
	return false;
}

bool SQLiteDatabase::InsertCharItem(unsigned int CID, int ItemID, bool RentItem, int RentPeriodHour,
	u32 * outCIID)
try
{
	ExecuteSQL("INSERT INTO CharacterItem (CID, ItemID, RegDate) "
		"Values (?, ?, date('now'))",
		CID, ItemID);

	*outCIID = static_cast<u32>(LastInsertedRowID());

	return true;
}
catch (const SQLiteError& e)
{
	HandleException(e);
	return false;
}

bool SQLiteDatabase::DeleteCharItem(unsigned int CID, int CIID)
try
{
	ExecuteSQL("UPDATE CharacterItem SET CID = NULL "
		"WHERE CID = ? AND CIID = ?",
		CID, CIID);

	return true;
}
catch (const SQLiteError& e)
{
	HandleException(e);
	return false;
}

bool SQLiteDatabase::GetCharItemInfo(MMatchCharInfo& CharInfo)
try
{
	auto stmt = ExecuteSQL("SELECT CIID, ItemID, "
		"(RentHourPeriod*60) - CAST((JulianDay(datetime('now')) - JulianDay(RentDate)) * 24 * 60 As Integer) "
		"AS RentPeriodRemainder "
		"FROM CharacterItem "
		"WHERE CID = ? ORDER BY CIID",
		CharInfo.m_nCID);

	while (stmt.HasRow())
	{
		auto CIID = stmt.Get<int>();
		auto ItemDescID = stmt.Get<int>();

		auto IsRentItem = !stmt.IsNull();
		auto RentMinutePeriodRemainder = 0;
		if (IsRentItem)
		{
			RentMinutePeriodRemainder = stmt.Get<int>();
		}
		else
		{
			RentMinutePeriodRemainder = RENT_MINUTE_PERIOD_UNLIMITED;
			stmt.NextColumn();
		}

		MUID uidNew = MMatchItemMap::UseUID();
		CharInfo.m_ItemList.CreateItem(uidNew, CIID, ItemDescID, IsRentItem, RentMinutePeriodRemainder);

		stmt.Step();
	}

	CharInfo.m_ItemList.SetDbAccess();

	return true;
}
catch (const SQLiteError& e)
{
	HandleException(e);
	return false;
}

bool SQLiteDatabase::GetAccountItemInfo(int AID, MAccountItemNode * outItemNode,
	int * outNodeCount, int MaxNodeCount, MAccountItemNode * outExpiredItemList,
	int * outExpiredItemCount, int MaxExpiredItemCount)
try
{
	auto stmt = ExecuteSQL("SELECT AIID, ItemID, "
		"(RentHourPeriod*60) - CAST((JulianDay(datetime('now')) - JulianDay(RentDate)) * 24 * 60 As Integer) "
		" AS RentPeriodRemainder "
		"FROM AccountItem "
		"WHERE AID = ? ORDER BY AIID",
		AID);

	int NodeCount;
	int ExpiredItemCount;

	while (stmt.HasRow())
	{
		auto aiid = stmt.Get<int>();
		u32 itemid = stmt.Get<int>();

		auto IsRentItem = !stmt.IsNull();
		auto RentMinutePeriodRemainder = 0;
		if (IsRentItem)
		{
			RentMinutePeriodRemainder = stmt.Get<int>();
		}
		else
		{
			RentMinutePeriodRemainder = RENT_MINUTE_PERIOD_UNLIMITED;
			stmt.NextColumn();
		}

		if (IsRentItem && RentMinutePeriodRemainder <= 0)
		{
			if (ExpiredItemCount < MaxExpiredItemCount)
			{
				outExpiredItemList[ExpiredItemCount].nAIID = aiid;
				outExpiredItemList[ExpiredItemCount].nItemID = itemid;
				ExpiredItemCount++;
			}
		}
		else
		{
			outItemNode[NodeCount].nAIID = aiid;
			outItemNode[NodeCount].nItemID = itemid;
			outItemNode[NodeCount].nRentMinutePeriodRemainder = RentMinutePeriodRemainder;

			NodeCount++;
			if (NodeCount >= MaxNodeCount) break;
		}

		stmt.Step();
	}

	*outNodeCount = NodeCount;
	*outExpiredItemCount = ExpiredItemCount;

	return true;
}
catch (const SQLiteError& e)
{
	HandleException(e);
	return false;
}

bool SQLiteDatabase::UpdateEquipedItem(u32 CID, MMatchCharItemParts parts, u32 CIID,
	u32 ItemID)
try
{
	auto stmt = ExecuteSQL("SELECT Items FROM Character WHERE CID = ?", CID);

	ItemBlob Items;

	if (stmt.HasRow() && !stmt.IsNull())
	{
		auto Blob = stmt.Get<::Blob>();
		if (Blob.Size != sizeof(ItemBlob))
		{
			Log("SQLiteDatabase::UpdateEquipedItem - Items blob has wrong size %d, expected %d\n",
				Blob.Size, sizeof(ItemBlob));
			return false;
		}
		memcpy(&Items, Blob.Ptr, sizeof(Items));
	}
	else
	{
		for (auto& e : Items.CIIDs)
			e = 0;
		for (auto& e : Items.ItemIDs)
			e = 0;
	}

	Items.CIIDs[parts] = CIID;
	Items.ItemIDs[parts] = ItemID;

	ExecuteSQL("UPDATE Character SET Items = ? WHERE CID = ?", Blob{ &Items, sizeof(Items) }, CID);

	return true;
}
catch (const SQLiteError& e)
{
	HandleException(e);
	return false;
}

bool SQLiteDatabase::ClearAllEquipedItem(u32 CID)
try
{
	ExecuteSQL("UPDATE Character SET Items = NULL WHERE CID = ?", CID);
	return true;
}
catch (const SQLiteError& e)
{
	HandleException(e);
	return false;
}

bool SQLiteDatabase::DeleteExpiredAccountItem(int AIID)
try
{
	ExecuteSQL("DELETE FROM AccountItem WHERE AIID = ? AND RentDate IS NOT NULL", AIID);
	return true;
}
catch (const SQLiteError& e)
{
	HandleException(e);
	return false;
}

bool SQLiteDatabase::BuyBountyItem(unsigned int CID, int ItemID, int Price, u32 * outCIID)
try
{
	auto stmt = ExecuteSQL("SELECT BP FROM Character WHERE CID = ?", CID);
	if (!stmt.HasRow() || stmt.IsNull() || stmt.Get<int>() < Price)
		return false;

	auto Trans = BeginTransaction();

	ExecuteSQL("UPDATE Character SET BP = BP - ? WHERE CID = ?", Price, CID);
	if (RowsModified() == 0)
		return false;

	ExecuteSQL("INSERT INTO CharacterItem (CID, ItemID, RegDate) Values (?, ?, date('now'))",
		CID, ItemID);
	if (RowsModified() == 0)
		return false;

	*outCIID = static_cast<u32>(LastInsertedRowID());

	CommitTransaction();

	// TODO: Insert purchase log

	return true;
}
catch (const SQLiteError& e)
{
	HandleException(e);
	return false;
}

bool SQLiteDatabase::SellBountyItem(unsigned int CID, unsigned int ItemID, unsigned int CIID, int Price, int CharBP)
try
{
	auto Trans = BeginTransaction();

	ExecuteSQL("UPDATE CharacterItem SET CID = NULL WHERE CID = ? AND CIID = ?", CID, CIID);
	if (RowsModified() == 0)
		return false;

	ExecuteSQL("UPDATE Character SET BP = BP + ? WHERE CID = ?", Price, CID);
	if (RowsModified() == 0)
		return false;

	CommitTransaction();

	// TODO: Insert sell log

	return true;
}
catch (const SQLiteError& e)
{
	HandleException(e);
	return false;
}

bool SQLiteDatabase::UpdateQuestItem(int nCID, MQuestItemMap& rfQuestIteMap, MQuestMonsterBible& rfQuestMonster)
try
{
	constexpr auto ActualQuestDataSize = MCRC32::SIZE + MAX_DB_QUEST_ITEM_SIZE + MAX_DB_MONSTERBIBLE_SIZE;
	static_assert(QUEST_DATA >= ActualQuestDataSize, "Invalid constants");

	u8 QuestData[QUEST_DATA]{};

	for (auto&& Pair : rfQuestIteMap)
	{
		auto ItemID = Pair.first;
		auto* QuestItem = Pair.second;

		if (!GetQuestItemDescMgr().IsQItemID(ItemID) || !QuestItem)
		{
			MLog("SQLiteDatabase::UpdateQuestItem -- Invalid data in quest item map. "
				"ItemID = %u, QuestItem = %p.\n", ItemID, static_cast<void*>(QuestItem));
			assert(false);
			continue;
		}

		int nIndex = MCRC32::SIZE + QuestItem->GetItemID() - MIN_QUEST_ITEM_ID;
		unsigned char nValue = 0;
		if (QuestItem->GetCount() > 0)
		{
			nValue = QuestItem->GetCount() + MIN_QUEST_DB_ITEM_COUNT;
		}
		else if (QuestItem->IsKnown())
		{
			nValue = MIN_QUEST_DB_ITEM_COUNT;
		}
		else
		{
			nValue = 0;
		}

		QuestData[nIndex] = static_cast<u8>(nValue);
	}

	memcpy(QuestData + MCRC32::SIZE + MAX_DB_QUEST_ITEM_SIZE, &rfQuestMonster, MAX_DB_MONSTERBIBLE_SIZE);

	auto CRC32 = MCRC32::BuildCRC32(QuestData + MCRC32::SIZE,
		MAX_DB_QUEST_ITEM_SIZE + MAX_DB_MONSTERBIBLE_SIZE);

	memcpy(QuestData, &CRC32, MCRC32::CRC::SIZE);

	ExecuteSQL("UPDATE Character SET QuestItemInfo = ? WHERE CID = ?",
		Blob{ QuestData, ActualQuestDataSize }, nCID);

	ASSERT_ROWS_MODIFIED_NOT_ZERO();

	return true;
}
catch (const SQLiteError& e)
{
	HandleException(e);
	return false;
}

bool SQLiteDatabase::GetCharQuestItemInfo(MMatchCharInfo * pCharInfo)
try
{
	if (!pCharInfo)
		return false;

	pCharInfo->m_QuestItemList.Clear();
	pCharInfo->m_QMonsterBible.Clear();

	auto stmt = ExecuteSQL("SELECT QuestItemInfo FROM Character WHERE CID = ?",
		pCharInfo->m_nCID);
	
	if (!stmt.HasRow())
		return false;

	auto QuestItemInfo = stmt.Get<Blob>();

	if (QuestItemInfo.Size < 4)
	{
		pCharInfo->m_QuestItemList.SetDBAccess(true);
		return true;
	}

	auto* QuestData = static_cast<const u8*>(QuestItemInfo.Ptr);

	MCRC32::crc_t ExpectedCRC32;
	memcpy(&ExpectedCRC32, QuestData, MCRC32::CRC::SIZE);

	auto ActualCRC32 = MCRC32::BuildCRC32(QuestData + MCRC32::CRC::SIZE,
		QuestItemInfo.Size - MCRC32::CRC::SIZE);
	if (ExpectedCRC32 != ActualCRC32)
	{
		Log("MSSQLDatabase::GetQuestItem - CRC check error, quest data is corrupt!\n"
		"Expected CRC32 = %08X, actual CRC32 = %08X, QuestItemInfo.Size = %zu\n",
			ExpectedCRC32, ActualCRC32, QuestItemInfo.Size);
		assert(false);
		return false;
	}

	for (int i = 0; i < MAX_DB_QUEST_ITEM_SIZE; ++i)
	{
		int QuestItemCount = static_cast<int>(QuestData[MCRC32::CRC::SIZE + i]);
		if (QuestItemCount < MIN_QUEST_DB_ITEM_COUNT)
			continue;

		u32 ItemID = static_cast<u32>(i) + MIN_QUEST_ITEM_ID;
		bool KnownItem;

		// NOTE: This is always true since MIN_QUEST_DB_ITEM_COUNT = 1.
		// Not sure if it's useful to keep it, just copied it from the corresponding MSSQL
		// implementation.
		if (QuestItemCount > 0)
		{
			KnownItem = true;
			QuestItemCount--;
		}
		else
		{
			KnownItem = false;
		}

		if (!pCharInfo->m_QuestItemList.CreateQuestItem(ItemID, QuestItemCount, KnownItem))
		{
			mlog("MSSQLDatabase::GetCharQuestItemInfo - Failed to create quest item with "
				"properties ItemID = %u, Count = %d, Known = %d\n",
				ItemID, QuestItemCount, KnownItem);
		}
	}

	{
		constexpr int Offset = MCRC32::SIZE + MAX_DB_QUEST_ITEM_SIZE;
		auto* SrcPtr = QuestData + Offset;
		int Size = static_cast<int>(QuestItemInfo.Size) - Offset;
		if (Size > 0)
		{
			memcpy(pCharInfo->m_QMonsterBible.szData, SrcPtr, static_cast<size_t>(Size));
		}
	}

	pCharInfo->m_QuestItemList.SetDBAccess(true);

	return true;
}
catch (const SQLiteError& e)
{
	HandleException(e);
	return false;
}

bool SQLiteDatabase::InsertQuestGameLog(const char * pszStageName, int nScenarioID, int nMasterCID, int nPlayer1, int nPlayer2, int nPlayer3, int nTotalRewardQItemCount, int nElapsedPlayTime, int & outQGLID)
{
	return true;
}

bool SQLiteDatabase::InsertQUniqueGameLog(int nQGLID, int nCID, int nQIID)
{
	// TODO: Implement

	return true;
}

bool SQLiteDatabase::InsertConnLog(int nAID, const char * szIP, const std::string & strCountryCode3)
{
	// TODO: Implement

	return true;
}

bool SQLiteDatabase::InsertGameLog(const char * szGameName, const char * szMap, const char * GameType, int nRound, unsigned int nMasterCID, int nPlayerCount, const char * szPlayers)
{
	// TODO: Implement

	return true;
}

bool SQLiteDatabase::InsertKillLog(unsigned int nAttackerCID, unsigned int nVictimCID)
{
	// TODO: Implement

	return true;
}

bool SQLiteDatabase::InsertChatLog(u32 nCID, const char * szMsg, u64 nTime)
{
	// TODO: Implement

	return true;
}

bool SQLiteDatabase::InsertServerLog(int nServerID, int nPlayerCount, int nGameCount, uint32_t dwBlockCount, uint32_t dwNonBlockCount)
{
	// TODO: Implement

	return true;
}

bool SQLiteDatabase::InsertPlayerLog(u32 nCID, int nPlayTime, int nKillCount, int nDeathCount, int nXP, int nTotalXP)
{
	// TODO: Implement

	return true;
}

bool SQLiteDatabase::UpdateCharPlayTime(u32 CID, u32 PlayTime)
try
{
	ExecuteSQL("UPDATE Character SET PlayTime = PlayTime + ?, LastTime = date('now') WHERE CID = ?", CID);
	return true;
}
catch (const SQLiteError& e)
{
	HandleException(e);
	return false;
}

bool SQLiteDatabase::BringAccountItem(int AID, int CID, int AIID, unsigned int * outCIID, u32 * outItemID, bool * outIsRentItem, int * outRentMinutePeriodRemainder)
try
{
	auto stmt = ExecuteSQL("SELECT ItemID, RentDate, RentHourPeriod, Cnt "
		"FROM AccountItem WHERE AIID = ?", AIID);
	if (!stmt.HasRow())
		return false;

	auto ItemID = stmt.Get<int>();
	auto RentDate = stmt.Get<StringView>();
	auto RentHourPeriod = stmt.Get<int>();
	auto Cnt = stmt.Get<int>();

	auto Trans = BeginTransaction();

	ExecuteSQL("DELETE FROM AccountItem WHERE AIID = ?", AIID);

	ExecuteSQL("INSERT INTO CharacterItem(CID, ItemID, RegDate, RentDate, RentHourPeriod, Cnt) "
		"VALUES(?, ?, date('now'), ?, ?, ?)",
		CID, ItemID, RentDate, RentHourPeriod, Cnt);


	// TODO: Log

	CommitTransaction();
	return true;
}
catch (const SQLiteError& e)
{
	HandleException(e);
	return false;
}

bool SQLiteDatabase::BringBackAccountItem(int AID, int CID, int CIID)
try
{
	auto Trans = BeginTransaction();

	// TODO: Check that the item isn't equipped

	auto stmt = ExecuteSQL("SELECT ItemID, RentDate, @RentHourPeriod=RentHourPeriod, Cnt "
		"FROM CharacterItem WHERE CIID = ? AND CID = ?", CIID, CID);

	if (!stmt.HasRow())
		return false;

	auto ItemID = stmt.Get<int>();
	auto RentDate = stmt.Get<int>();
	auto RentHourPeriod = stmt.Get<int>();
	auto Cnt = stmt.Get<int>();

	ExecuteSQL("UPDATE CharacterItem SET CID = NULL WHERE CIID = ? AND CID = ?", CIID, CID);

	ExecuteSQL("INSERT INTO AccountItem(AID, ItemID, RentDate, RentHourPeriod, Cnt) "
		"VALUES(?, ?, ?, ?, ?)",
		AID, ItemID, RentDate, RentHourPeriod, Cnt);

	CommitTransaction();
	return true;
}
catch (const SQLiteError& e)
{
	HandleException(e);
	return false;
}

bool SQLiteDatabase::FriendAdd(int CID, int FriendCID, int Favorite)
try
{
	ExecuteSQL("INSERT INTO Friend(CID, FriendCID, Favorite, DeleteFlag, Type) "
		"Values (?, ?, ?, 0, 1)",
		CID, FriendCID, Favorite);

	return true;
}
catch (const SQLiteError& e)
{
	HandleException(e);
	return false;
}

bool SQLiteDatabase::FriendRemove(int CID, int FriendCID)
try
{
	ExecuteSQL("UPDATE Friend "
		"SET DeleteFlag = 1 "
		"WHERE CID = ? AND FriendCID = ?",
		CID, FriendCID);

	return true;
}
catch (const SQLiteError& e)
{
	HandleException(e);
	return false;
}

bool SQLiteDatabase::FriendGetList(int CID, MMatchFriendInfo * FriendInfo)
try
{
	auto stmt = ExecuteSQL("SELECT f.FriendCID, f.Favorite, c.Name "
		"FROM Friend f, Character c "
		"WHERE f.CID = ? AND f.FriendCID = c.CID AND f.DeleteFlag = 0 AND f.Type = 1",
		CID);

	while (stmt.HasRow())
	{
		auto FriendCID = stmt.Get<int>();
		auto Favorite = stmt.Get<int>();
		auto Name = stmt.Get<StringView>();
		FriendInfo->Add(FriendCID, Favorite, Name);
		stmt.Step();
	};

	return true;
}
catch (const SQLiteError& e)
{
	HandleException(e);
	return false;
}

bool SQLiteDatabase::GetCharClan(int CID, int * outClanID, char * outClanName, int maxlen)
try
{
	auto stmt = ExecuteSQL("SELECT cl.CLID AS CLID, cl.Name AS ClanName "
		"FROM ClanMember cm, Clan cl "
		"WHERE cm.cid = ? AND cm.CLID = cl.CLID",
		CID);

	if (!stmt.HasRow())
		return false;

	*outClanID = stmt.Get<int>();
	strcpy_safe(outClanName, maxlen, stmt.Get<StringView>());

	return true;
}
catch (const SQLiteError& e)
{
	HandleException(e);
	return false;
}

bool SQLiteDatabase::GetClanIDFromName(const char * ClanName, int * outCLID)
try
{
	auto stmt = ExecuteSQL("SELECT CLID FROM Clan WHERE Name = ?", ClanName);

	if (!stmt.HasRow())
		return false;

	*outCLID = stmt.Get<int>();

	return true;
}
catch (const SQLiteError& e)
{
	HandleException(e);
	return false;
}

//unused?
bool SQLiteDatabase::CreateClan(const char * ClanName, int MasterCID, int Member1CID, int Member2CID, int Member3CID, int Member4CID, bool * outRet, int * outNewCLID)
try
{
	*outRet = false;
	
	auto stmt = ExecuteSQL("SELECT CLID FROM Clan WHERE Name = ?",
		ClanName);

	if (stmt.HasRow())
		return false;

	stmt = ExecuteSQL("SELECT COUNT(*) FROM ClanMember cm, Character c "
		"WHERE((cm.CID = ?) OR(cm.CID = ?) OR(cm.CID = ?) OR(cm.CID = ?) OR "
		"(cm.CID = ?)) AND cm.CID = c.CID AND c.DeleteFlag = 0",
		MasterCID, Member1CID, Member2CID, Member3CID, Member4CID);

	if (stmt.HasRow())
		return false;

	auto Trans = BeginTransaction();

	ExecuteSQL("INSERT INTO Clan(Name, MasterCID, RegDate) VALUES(?, ?, date('now'))",
		ClanName, MasterCID);

	stmt = ExecuteSQL("SELECT CLID FROM Clan WHERE MasterCID = ?",
		MasterCID);

	if (!stmt.HasRow())
		return false;

	auto CLID = stmt.Get<int>();

	ExecuteSQL("INSERT INTO ClanMember(CLID, CID, Grade, RegDate) VALUES(?, ?, 1, date('now'))",
		CLID, MasterCID);

	int MemberCIDs[] = { Member1CID, Member2CID, Member3CID, Member4CID };

	for (auto CID : MemberCIDs)
		ExecuteSQL("INSERT INTO ClanMember(CLID, CID, Grade, RegDate) VALUES(?, ?, 9, date('now'))",
			CLID, CID);

	CommitTransaction();

	*outRet = true;
	*outNewCLID = CLID;

	return true;
}
catch (const SQLiteError& e)
{
	HandleException(e);
	return false;
}

bool SQLiteDatabase::CreateClan(const char * ClanName, int MasterCID, bool * outRet, int * outNewCLID)
try
{
	*outRet = false;

	auto stmt = ExecuteSQL("SELECT CLID FROM Clan WHERE Name = ?",
		ClanName);

	if (stmt.HasRow())
		return false;

	stmt = ExecuteSQL("SELECT COUNT(*) FROM ClanMember cm, Character c "
		"WHERE cm.CID = ? AND cm.CID = c.CID AND c.DeleteFlag = 0",
		MasterCID);

	if (!stmt.HasRow() || stmt.Get<int>() > 0)
		return false;

	auto Trans = BeginTransaction();

	ExecuteSQL("INSERT INTO Clan(Name, MasterCID, RegDate, Exp, Level, Point, Wins, Losses, "
		"Draws, Ranking, TotalPoint, RankIncrease, EmblemChecksum, LastDayRanking, "
		"LastMonthRanking, EmblemUrl) "
		"VALUES(?, ?, date('now'), 0, 0, 1000, 0, 0, "
		"0, 0, 0, 0, 0, 0, "
		"0, '')",
		ClanName, MasterCID);

	stmt = ExecuteSQL("SELECT CLID FROM Clan WHERE MasterCID = ?",
		MasterCID);

	if (!stmt.HasRow())
		return false;

	auto CLID = stmt.Get<int>();

	ExecuteSQL("INSERT INTO ClanMember(CLID, CID, Grade, RegDate, ContPoint) "
		"VALUES(?, ?, 1, date('now'), 0)",
		CLID, MasterCID);

	CommitTransaction();

	*outRet = true;
	*outNewCLID = CLID;

	return true;
}
catch (const SQLiteError& e)
{
	HandleException(e);
	return false;
}

bool SQLiteDatabase::DeleteExpiredClan(uint32_t dwCID, uint32_t dwCLID, const std::string & strDeleteName, uint32_t dwWaitHour)
try
{
	// Unimplemented in MSSQL

	return true;
}
catch (const SQLiteError& e)
{
	HandleException(e);
	return false;
}

bool SQLiteDatabase::SetDeleteTime(uint32_t dwMasterCID, uint32_t dwCLID, const std::string & strDeleteDate)
try
{
	// Unimplemented in MSSQL

	return true;
}
catch (const SQLiteError& e)
{
	HandleException(e);
	return false;
}

bool SQLiteDatabase::ReserveCloseClan(int CLID, const char * ClanName, int MasterCID, const std::string & strDeleteDate)
try
{
	ExecuteSQL("UPDATE Clan SET DeleteFlag = 2 WHERE CLID = ? AND Name = ? AND MasterCID = ?",
		CLID, ClanName, MasterCID);
	return true;
}
catch (const SQLiteError& e)
{
	HandleException(e);
	return false;
}

bool SQLiteDatabase::CloseClan(int CLID, const char * ClanName, int MasterCID)
try
{
	auto Trans = BeginTransaction();

	ExecuteSQL("DELETE FROM ClanMember WHERE CLID = ?", CLID);

	ExecuteSQL("UPDATE Clan SET DeleteFlag = 1, MasterCID = NULL, DeleteName = ?, Name = NULL "
		"WHERE CLID = ? AND Name = ? AND MasterCID = ?",
		ClanName, CLID, ClanName, MasterCID);

	CommitTransaction();

	return true;
}
catch (const SQLiteError& e)
{
	HandleException(e);
	return false;
}

bool SQLiteDatabase::AddClanMember(int CLID, int JoinerCID, int ClanGrade, bool * outRet)
try
{
	ExecuteSQL("INSERT INTO ClanMember(CLID, CID, Grade, RegDate, ContPoint) VALUES(?, ?, ?, date('now'), 0)",
		CLID, JoinerCID, ClanGrade);
	*outRet = true;
	return true;
}
catch (const SQLiteError& e)
{
	HandleException(e);
	return false;
}

bool SQLiteDatabase::RemoveClanMember(int CLID, int LeaverCID)
try
{
	ExecuteSQL("DELETE FROM ClanMember WHERE CLID = ? AND CID = ? AND Grade != 1", CLID, LeaverCID);

	return true;
}
catch (const SQLiteError& e)
{
	HandleException(e);
	return false;
}

bool SQLiteDatabase::UpdateClanGrade(int CLID, int MemberCID, int ClanGrade)
try
{
	ExecuteSQL("UPDATE ClanMember SET Grade = ? WHERE CLID = ? AND CID = ?", ClanGrade, CLID, MemberCID);
	return true;
}
catch (const SQLiteError& e)
{
	HandleException(e);
	return false;
}

ExpelResult SQLiteDatabase::ExpelClanMember(int CLID, int AdminGrade, const char * MemberName)
try
{
	auto stmt = ExecuteSQL("SELECT c.cid, cm.Grade FROM Character c, ClanMember cm "
		"WHERE cm.clid = ? AND c.cid = cm.cid AND c.Name = ? AND DeleteFlag = 0",
		CLID, MemberName);

	if (!stmt.HasRow())
		return ExpelResult::NoSuchMember;

	if (stmt.IsNull())
		return ExpelResult::NoSuchMember;

	auto CID = stmt.Get<int>();
	auto Grade = stmt.Get<int>();

	if (AdminGrade >= Grade)
		return ExpelResult::TooLowGrade;

	ExecuteSQL("DELETE FROM ClanMember WHERE CLID = ? AND CID = ? AND Grade != 1", CLID, CID);

	return ExpelResult::OK;
}
catch (const SQLiteError& e)
{
	HandleException(e);
	return ExpelResult::DBError;
}

bool SQLiteDatabase::GetClanInfo(int CLID, MDB_ClanInfo * outClanInfo)
try
{
	auto stmt = ExecuteSQL("SELECT cl.Name AS Name, cl.TotalPoint AS TotalPoint, "
		"cl.Level AS Level, cl.Ranking AS Ranking, "
		"cl.Point AS Point, cl.Wins AS Wins, cl.Losses AS Losses, cl.Draws AS Draws, "
		"c.Name AS ClanMaster, "
		"(SELECT COUNT(*) FROM ClanMember WHERE CLID = ?1) AS MemberCount, "
		"cl.EmblemUrl AS EmblemUrl, cl.EmblemChecksum AS EmblemChecksum "
		"FROM Clan cl, Character c "
		"WHERE cl.CLID = ?1 and cl.MasterCID = c.CID",
		CLID);

	if (!stmt.HasRow())
		return false;

	outClanInfo->nCLID = CLID;
	strcpy_safe(outClanInfo->szClanName, stmt.Get<StringView>());
	outClanInfo->nTotalPoint = stmt.Get<int>();
	outClanInfo->nLevel = stmt.Get<int>();
	outClanInfo->nRanking = stmt.Get<int>();
	outClanInfo->nPoint = stmt.Get<int>();
	outClanInfo->nWins = stmt.Get<int>();
	outClanInfo->nLosses = stmt.Get<int>();
	outClanInfo->nDraws = stmt.Get<int>();
	strcpy_safe(outClanInfo->szMasterName, stmt.Get<StringView>());
	outClanInfo->nTotalMemberCount = stmt.Get<int>();
	strcpy_safe(outClanInfo->szEmblemUrl, stmt.Get<StringView>());
	outClanInfo->nEmblemChecksum = stmt.Get<int>();

	return true;
}
catch (const SQLiteError& e)
{
	HandleException(e);
	return false;
}

bool SQLiteDatabase::UpdateCharClanContPoint(int CID, int CLID, int AddedContPoint)
try
{
	ExecuteSQL("UPDATE ClanMember SET ContPoint = ContPoint + ? WHERE CID = ? AND CLID = ?",
		AddedContPoint, CID, CLID);

	return true;
}
catch (const SQLiteError& e)
{
	HandleException(e);
	return false;
}

bool SQLiteDatabase::GetLadderTeamID(const int nTeamTableIndex, const int * pnMemberCIDArray, int nMemberCount, int * pnoutTID)
try
{
	// Unimplemented in MSSQL

	return false;
}
catch (const SQLiteError& e)
{
	HandleException(e);
	return false;
}

bool SQLiteDatabase::LadderTeamWinTheGame(int nTeamTableIndex, int nWinnerTID, int nLoserTID, bool bIsDrawGame, int nWinnerPoint, int nLoserPoint, int nDrawPoint)
try
{
	// Unimplemented in MSSQL

	return false;
}
catch (const SQLiteError& e)
{
	HandleException(e);
	return false;
}

bool SQLiteDatabase::GetLadderTeamMemberByCID(const int nCID, int * poutTeamID, char ** ppoutCharArray, int maxlen, int nCount)
try
{
	// Unimplemented in MSSQL

	return false;
}
catch (const SQLiteError& e)
{
	HandleException(e);
	return false;
}

bool SQLiteDatabase::WinTheClanGame(int WinnerCLID, int LoserCLID, bool IsDrawGame, int WinnerPoint, int LoserPoint,
	const char * WinnerClanName, const char * LoserClanName, int RoundWins, int RoundLosses, int MapID,
	int GameType, const char * WinnerMembers, const char * LoserMembers)
	try
{
	if (IsDrawGame)
		return true;
	auto Trans = BeginTransaction();
	// update win
	ExecuteSQL("UPDATE Clan SET Wins = Wins + 1, Point = Point + ?1, TotalPoint = TotalPoint + ?1 "
		"WHERE CLID = ?2", WinnerPoint, WinnerCLID);
	// update lose
	ExecuteSQL("UPDATE Clan SET Losses = Losses + 1, Point = max(0, Point + ?1) WHERE CLID = ?2",
		LoserPoint, LoserCLID);
	
	//loging the clan game in the db
	ExecuteSQL("INSERT INTO ClanGameLog(WinnerCLID, LoserCLID, WinnerClanName, LoserClanName, "
		"RoundWins, RoundLosses, "
		"MapID, GameType, RegDate, WinnerMembers, LoserMembers, WinnerPoint, LoserPoint) "
		"VALUES(?, ?, ?, ?, ?, ?, "
		"?, ?, date('now'), ?, ?, ?, ?)",
		WinnerCLID, LoserCLID, WinnerClanName, LoserClanName, RoundWins, RoundLosses,
		MapID, GameType, WinnerMembers, LoserMembers, WinnerPoint, LoserPoint);

	CommitTransaction();
	return true;
}
catch (const SQLiteError& e)
{
	HandleException(e);
	return false;
}

bool SQLiteDatabase::UpdateCharLevel(int CID, int NewLevel, int BP, int KillCount, int DeathCount, int PlayTime, bool IsLevelUp)
try
{
	ExecuteSQL("UPDATE Character SET Level = ? WHERE CID = ?", NewLevel, CID);
	return true;
}
catch (const SQLiteError& e)
{
	HandleException(e);
	return false;
}

bool SQLiteDatabase::EventJjangUpdate(int AID, bool Jjang)
try
{
	auto UGradeID = Jjang ? MMUG_STAR : MMUG_FREE;
	ExecuteSQL("UPDATE Account SET UGradeID = ? WHERE AID = ?", UGradeID, AID);
	return true;
}
catch (const SQLiteError& e)
{
	HandleException(e);
	return false;
}

bool SQLiteDatabase::CheckPremiumIP(const char * szIP, bool & outbResult)
try
{
	// Unimplemented in MSSQL

	return true;
}
catch (const SQLiteError& e)
{
	HandleException(e);
	return false;
}

bool SQLiteDatabase::GetCID(const char * CharName, int & outCID)
try
{
	auto stmt = ExecuteSQL("SELECT CID FROM Character WHERE Name = ?", CharName);

	if (!stmt.HasRow())
		return false;

	outCID = stmt.Get<int>();

	return true;
}
catch (const SQLiteError& e)
{
	HandleException(e);
	return false;
}

bool SQLiteDatabase::GetCharName(int CID, std::string & outCharName)
try
{
	auto stmt = ExecuteSQL("SELECT Name FROM Character WHERE CID = ?", CID);

	if (!stmt.HasRow() || stmt.IsNull())
		return false;

	outCharName = stmt.Get<StringView>().str();

	return true;
}
catch (const SQLiteError& e)
{
	HandleException(e);
	return false;
}

bool SQLiteDatabase::InsertEvent(uint32_t dwAID, uint32_t dwCID, const std::string & strEventName)
try
{
	return true;
}
catch (const SQLiteError& e)
{
	HandleException(e);
	return false;
}

bool SQLiteDatabase::SetBlockAccount(uint32_t dwAID, uint32_t dwCID, uint8_t btBlockType, const std::string & strComment, const std::string & strIP, const std::string & strEndHackBlockerDate)
try
{
	return true;
}
catch (const SQLiteError& e)
{
	HandleException(e);
	return false;
}

bool SQLiteDatabase::ResetAccountBlock(uint32_t dwAID, uint8_t btBlockType)
try
{
	return true;
}
catch (const SQLiteError& e)
{
	HandleException(e);
	return false;
}

bool SQLiteDatabase::InsertBlockLog(uint32_t dwAID, uint32_t dwCID, uint8_t btBlockType, const std::string & strComment, const std::string & strIP)
try
{
	return true;
}
catch (const SQLiteError& e)
{
	HandleException(e);
	return false;
}

bool SQLiteDatabase::AdminResetAllHackingBlock()
try
{
	return true;
}
catch (const SQLiteError& e)
{
	HandleException(e);
	return false;
}