#include "MMatchServer.h"
#include "ODBCRecordset.h"
#include "MSSQLDatabase.h"
#include "MDatabase.h"
#include "MMatchDBFilter.h"
#include "MMatchObject.h"
#include "MMatchTransDataType.h"
#include "MMatchFriendInfo.h"
#include <stdio.h>
#include "MErrorTable.h"
#include "MDebug.h"
#include "MMatchStatus.h"
#include "MMatchFriendInfo.h"
#include "MMatchClan.h"
#include "MInetUtil.h"
#include "MCRC32.h"
#include "MQuestConst.h"
#include "MQuestItem.h"
#include "MMatchConfig.h"
#include <thread>
#include <sstream>

static char g_szDB_CREATE_ACCOUNT[] = "{CALL spInsertAccount ('%s', '%s', %d, '%s', %d, %d)}";
static char g_szDB_GET_LOGININFO[] = "{CALL spGetLoginInfo ('%s')}";
static char g_szDB_GET_ACCOUNTINFO[] = "{CALL spGetAccountInfo (%d)}";
static char g_szDB_CREATE_CHAR[] = "{CALL spInsertChar (%d, %d, '%s', %d, %d, %d, %d)}";
static char g_szDB_DELETE_CHAR[] = "{CALL spDeleteChar (%d, %d, '%s')}";
static char g_szDB_GET_CHARLIST[] = "{CALL spGetCharList (%d)}";
static char g_szDB_GET_ACCOUNT_CHARINFO[] = "{CALL spGetAccountCharInfo (%d, %d)}";
static char g_szDB_GET_CHARINFO_BY_AID[] = "{CALL spGetCharInfoByCharNum (%d, %d)}";
static char g_szDB_SIMPLE_UPDATE_CHARINFO[] = "{CALL spSimpleUpdateChar (%d, '%s', %d, %d, %d)}";
static char g_szDB_INSERT_CHAR_ITEM[] = "{CALL spInsertCharItem (%d, %d)}";
static char g_szDB_DELETE_CHAR_ITEM[] = "{CALL spDeleteCharItem (%d, %d)}";
static char g_szDB_SELECT_CHAR_ITEM[] = "{CALL spSelectCharItem (%d)}";
static char g_szDB_SELECT_ACCOUNT_ITEM[] = "{CALL spSelectAccountItem (%d)}";
static char g_szDB_BUY_BOUNTY_ITEM[] = "{CALL spBuyBountyItem (%d, %d, %d)}";
static char g_szDB_SELL_BOUNTY_ITEM[] = "{CALL spSellBountyItem (%d, %d, %d, %d, %d)}";
static char g_szDB_UPDATE_EQUIPITEM[] = "{CALL spUpdateEquipItem (%d, %d, %d, %d)}";
static char g_szDB_INSERT_CONN_LOG[] = "{CALL spInsertConnLog (%d, %d, %d, %d, %d, '%s')}";
static char g_szDB_INSERT_GAME_LOG[] = "{CALL spInsertGameLog ('%s', '%s', '%s', %d, %d, %d, '%s')}";
static char g_szDB_INSERT_KILL_LOG[] = "{CALL spInsertKillLog (%d, %d)}";
static char g_szDB_INSERT_ITEM_PURCHASE_BY_BOUNTY_LOG[] = "{CALL spInsertItemPurchaseLogByBounty (%d, %d, %d, %d, '%s')}";
static char g_szDB_INSERT_CHAR_MAKING_LOG[] = "{CALL spInsertCharMakingLog (%d, '%s', '%s')}";
static char g_szDB_INSERT_SERVER_LOG[] = "{CALL spInsertServerLog (%d, %d, %d, %u, %u)}";
static char g_szDB_UPDATE_SERVER_STATUS[] = "{CALL spUpdateServerStatus (%d, %d)}";
static char g_szDB_UPDATE_SERVER_INFO[] = "UPDATE ServerStatus Set MaxPlayer=%d, ServerName='%s' WHERE ServerID=%d";
static char g_szDB_INSERT_PLAYER_LOG[] = "{CALL spInsertPlayerLog (%d, %d, %d, %d, %d, %d)}";
static char g_szDB_INSERT_LEVELUP_LOG[] = "{CALL spInsertLevelUpLog (%d, %d, %d, %d, %d, %d)}";
static char g_szDB_UPDATE_CHAR_LEVEL[] = "{CALL spUpdateCharLevel (%d, %d)}";
static char g_szDB_UPDATE_CHAR_BP[] = "{CALL spUpdateCharBP (%d, %d)}";
static char g_szDB_UPDATE_CHAR_INFO_DATA[] = "{CALL spUpdateCharInfoData (%d, %d, %d, %d, %d)}";
static char g_szDB_UPDATE_CHAR_PLAYTIME[] = "{CALL	spUpdateCharPlayTime (%d, %d)}";
static char g_szDB_UPDATE_LAST_CONNDATE[] = "{CALL	spUpdateLastConnDate ('%s', '%s')}";
static char g_szDB_BRING_ACCOUNTITEM[] = "{CALL spBringAccountItem (%d, %d, %d)}";
static char g_szDB_BRING_BACK_ACCOUNTITEM[] = "{CALL spBringBackAccountItem (%d, %d, %d)}";
static char g_szDB_CLEARALL_EQUIPEDITEM[] = "{CALL spClearAllEquipedItem (%d)}";
static char g_szDB_ADD_FRIEND[] = "{CALL spAddFriend (%d, %d, %d)}";
static char g_szDB_REMOVE_FRIEND[] = "{CALL spRemoveFriend (%d, %d)}";
static char g_szDB_GET_FRIEND_LIST[] = "{CALL spGetFriendList (%d)}";
static char g_szDB_GET_CHAR_CLAN[] = "{CALL spGetCharClan (%d)}";
static char g_szDB_GET_CLID_FROM_CLANNAME[] = "{CALL spGetCLIDFromClanName ('%s')}";
static char g_szDB_CREATE_CLAN[] = "{CALL spCreateClan ('%s', %d, %d, %d, %d, %d)}";
static char g_szDB_CREATE_CLAN2[] = "{CALL spCreateClan ('%s', %d)}";
static char g_szDB_RESERVE_CLOSE_CLAN[] = "{CALL spReserveCloseClan (%d, '%s', %d, '%s')}";
static char g_szDB_CLOSE_CLAN[] = "{CALL spCloseClan (%d, '%s', %d)}";
static char g_szDB_DELETE_CLAN[] = "{CALL spDeleteClan (%d, '%s')}";
static char g_szDB_ADD_CLAN_MEMBER[] = "{CALL spAddClanMember (%d, %d, %d)}";
static char g_szDB_REMOVE_CLAN_MEMBER[] = "{CALL spRemoveClanMember (%d, %d)}";
static char g_szDB_UPDATE_CLAN_GRADE[] = "{CALL spUpdateClanGrade (%d, %d, %d)}";
static char g_szDB_EXPEL_CLAN_MEMBER[] = "{CALL spRemoveClanMemberFromCharName (%d, %d, '%s')}";
static char g_szDB_GET_CLAN_INFO[] = "{CALL spGetClanInfo (%d)}";
static char g_szDB_GET_LADDER_TEAM4_ID[] = "{CALL spGetTeamID4 (%d, %d, %d, %d)}";
static char g_szDB_TEAM4_WIN_THE_GAME[] = "{CALL spTeam4WinTheGame (%d, %d, %d, %d, %d, %d)}";
static char g_szDB_GET_LADDER_TEAM_MEMBERLIST[] = "{CALL spGetLadderTeamMemberByCID (%d)}";
static char g_szDB_WIN_THE_CLAN_GAME[] = "{CALL spWinTheClanGame (%d, %d, %d, %d, %d, '%s', '%s', %d, %d, %d, %d, '%s', '%s')}";
static char g_szDB_UPDATE_CHAR_CLAN_CONTPOINT[] = "{CALL spUpdateCharClanContPoint (%d, %d, %d)}";
static char g_szDB_EVENT_JJANG_UPDATE[] = "UPDATE Account SET UGradeID=%d WHERE AID=%d";
static char g_szDB_DELETE_EXPIRED_ACCOUNT_ITEM[] = "{CALL spDeleteExpiredAccountItem (%d)}";
static char g_szDB_SELECT_QUEST_ITEM_INFO_BY_CID[] = "{CALL spSelectCharQuestItemInfoByCID (%d)}";
static char g_szDB_INSERT_QUEST_GAME_LOG[] = "{CALL spInsertQuestGameLog ('%s', %d, %d, %d, %d, %d, %d, %d)}";
static char g_szDB_INSERT_QUINQUEITEMLOG[] = "{CALL spInsertQUniqueItemLog (%d, %d, %d)}";
static char g_szDB_CHECK_PREMIUM_IP[] = "{CALL spCheckPremiumIP ('%s')}";
static char g_szInsertEvent[] = "{CALL spInsertEvent (%u, %u, '%s')}";
static char g_szSetBlockAccount[] = "{CALL spSetBlockAccount (%u, %u, %u, '%s', '%s', '%s')}";
static char g_szResetAccountBlock[] = "{CALL spResetAccountBlock (%u, %u)}";
static char g_szInsertBlockLog[] = "{CALL spInsertBlockLog (%u, %u, %u,'%s', '%s')}";
static char g_szSetClanDeleteDate[] = "{CALL spSetClanDeleteDate (%u, %u, '%s')}";
static char g_szDeleteExpiredClan[] = "{CALL spDeleteExpiredClan (%u, %u, '%s', %u)}";
static char g_szAdminResetAllHackingBlock[] = "{CALL spResetHackingBlock}";

static std::string FilterSQL(std::string str)
{
	erase_remove(str, '\'');
	return str;
}

MSSQLDatabase::MSSQLDatabase()
	: MSSQLDatabase(MGetServerConfig()->GetDBConnectionDetails())
{}

MSSQLDatabase::MSSQLDatabase(const MDatabase::ConnectionDetails& Details)
{
	m_DB.SetLogCallback(LogCallback);

	std::ostringstream ss;
	ss << std::this_thread::get_id();
	auto Str = ss.str();
	auto ThreadID = Str.c_str();
	if (m_DB.Connect(Details))
	{
		Log("DBMS connected @ thread ID %s\n", ThreadID);
	}
	else
	{
		Log("DBMS failed to connect @ thread ID %s\n", ThreadID);
	}
}

MSSQLDatabase::~MSSQLDatabase() {}

bool MSSQLDatabase::DeleteAllRows()
{
	auto Query =
		"EXEC sp_MSForEachTable 'DISABLE TRIGGER ALL ON ?'\n"
		"EXEC sp_MSForEachTable 'ALTER TABLE ? NOCHECK CONSTRAINT ALL'\n"
		"EXEC sp_MSForEachTable 'DELETE FROM ?'\n"
		"EXEC sp_MSForEachTable 'ALTER TABLE ? CHECK CONSTRAINT ALL'\n"
		"EXEC sp_MSForEachTable 'ENABLE TRIGGER ALL ON ?'"_sv;
	try
	{
		m_DB.ExecuteSQL(Query);
	}
	catch (CDBException* e)
	{
		Log("MSSQLDatabase::DeleteAllRows - %s\n", e->m_strError);
		return false;
	}
	return true;
}

bool MSSQLDatabase::CheckOpen()
{
	return m_DB.CheckOpen();
}

void MSSQLDatabase::Log(const char* pFormat, ...)
{
	va_list args;
	char msg[1024];

	va_start(args, pFormat);
	vsprintf_safe(msg, pFormat, args);

	mlog(msg);

	va_end(args);
}

void MSSQLDatabase::LogCallback(const std::string& strLog)
{
	mlog(strLog.c_str());
}

#define _STATUS_DB_START	auto nStatusStartTime = GetGlobalTimeMS();
#define _STATUS_DB_END(nID) MGetServerStatusSingleton()->AddDBQuery(nID, GetGlobalTimeMS()-nStatusStartTime);

bool MSSQLDatabase::GetLoginInfo(const char* szUserID, unsigned int* poutnAID, char* poutPassword, size_t maxlen)
{
	_STATUS_DB_START
		if (!CheckOpen()) return false;

	auto temp = FilterSQL(szUserID);

	szUserID = temp.c_str();

	CStringSQL strSQL;
	strSQL.Format(g_szDB_GET_LOGININFO, szUserID);

	CODBCRecordset rs(&m_DB);

	bool bException = false;
	try
	{
		rs.Open(strSQL, CRecordset::forwardOnly, CRecordset::readOnly);
	}
	catch (CDBException* e)
	{
		bException = true;

		Log("MSSQLDatabase::GetLoginInfo - %s\n", e->m_strError);

		return false;
	}

	if ((rs.IsOpen() == false) || (rs.GetRecordCount() <= 0) || (rs.IsBOF() == true))
	{
		return false;
	}

	_STATUS_DB_END(0);

	*poutnAID = rs.Field("AID").AsInt();
	strcpy_safe(poutPassword, maxlen, rs.Field("PasswordData").AsString());

	return true;
}

bool MSSQLDatabase::CreateAccount(const char* szUserID,
	const char* szPassword,
	const int nCert,
	const char* szName,
	const int nAge,
	const int nSex)

{
	_STATUS_DB_START
		if (!CheckOpen()) return false;

	try
	{
		auto temp1 = FilterSQL(szUserID);
		auto temp2 = FilterSQL(szPassword);
		CStringSQL strSQL;
		strSQL.Format(g_szDB_CREATE_ACCOUNT, temp1.c_str(), temp2.c_str(), nCert, szName, nAge, nSex);

		m_DB.ExecuteSQL(strSQL);
	}
	catch (CDBException* e)
	{
		Log("MSSQLDatabase::CreateAccount - %s\n", e->m_strError);
		return false;
	}

	_STATUS_DB_END(1);

	return true;
}

AccountCreationResult MSSQLDatabase::CreateAccountNew(const char* szUsername,
	const char* szPasswordData, size_t PasswordSize, const char* szEmail)
{
	_STATUS_DB_START
		if (!CheckOpen()) return AccountCreationResult::DBError;

	CODBCRecordset rs(&m_DB);

	std::string Username = FilterSQL(szUsername), Email = FilterSQL(szEmail);

	szUsername = Username.c_str();
	szEmail = Email.c_str();

	try
	{
		char sql[1024];

		sprintf_safe(sql, "SELECT AID FROM Account WHERE UserID = '%s'",
			szUsername);

		rs.Open(sql, CRecordset::forwardOnly, CRecordset::readOnly);

		if (rs.IsOpen() == false)
			return AccountCreationResult::DBError;

		if (rs.GetRecordCount() > 0)
			return AccountCreationResult::UsernameAlreadyExists;

		sprintf_safe(sql, "INSERT INTO Account (UserID, UGradeID, PGradeID, RegDate, Email, Age, Name) VALUES ('%s', '0', '0', GETDATE(), '%s', %d, '%s')",
			szUsername, szEmail, 0, "");

		m_DB.ExecuteSQL(sql);

		rs.Close();

		sprintf_safe(sql, "SELECT AID FROM Account WHERE UserID = '%s'",
			szUsername);

		rs.Open(sql, CRecordset::forwardOnly, CRecordset::readOnly);

		if (rs.IsOpen() == false || rs.GetRecordCount() <= 0 || rs.IsBOF() == true)
			return AccountCreationResult::DBError;

		int AID = rs.Field("AID").AsInt();

		rs.Close();

		sprintf_safe(sql, "INSERT INTO Login(UserID, AID, PasswordData) VALUES('%s', %d, '%s')", szUsername, AID, szPasswordData);

		m_DB.ExecuteSQL(sql);
	}
	catch (CDBException* e)
	{
		Log("MSSQLDatabase::CreateAccountNew - CDBException: %s\n", e->m_strError);
		return AccountCreationResult::DBError;
	}
	catch (CException* e)
	{
		char szError[128];
		e->GetErrorMessage(szError, sizeof(szError));
		Log("MSSQLDatabase::CreateAccountNew - CException: %s\n", szError);
		return AccountCreationResult::DBError;
	}
	catch (...)
	{
		Log("MSSQLDatabase::CreateAccountNew - Unknown exception\n");
		return AccountCreationResult::DBError;
	}

	_STATUS_DB_END(1);

	return AccountCreationResult::Success;
}

bool MSSQLDatabase::BanPlayer(int nAID, const char* szReason, const time_t& UnbanTime)
{
	auto temp = FilterSQL(szReason);

	char szTime[128];

	tm Tm = *localtime(&UnbanTime);

	strftime(szTime, sizeof(szTime), "%c", &Tm);

	try
	{
		char sql[1024];

		sprintf_safe(sql, "UPDATE Account SET UGradeID = %d WHERE AID = %d", MMUG_BLOCKED, nAID);

		m_DB.ExecuteSQL(sql);

		sprintf_safe(sql, "INSERT INTO Blocks (AID, Type, Reason, EndDate) VALUES (%d, %d, '%s', '%s')",
			nAID, MMBT_BANNED, temp.c_str(), szTime);

		m_DB.ExecuteSQL(sql);
	}
	catch (CDBException* e)
	{
		Log("MSSQLDatabase::BanPlayer - %s\n", e->m_strError);
		return false;
	}

	return true;
}

int MSSQLDatabase::CreateCharacter(int nAID, const char* szNewName, int nCharIndex, int nSex,
	int nHair, int nFace, int nCostume)
{
	_STATUS_DB_START
		if (!CheckOpen()) return false;

	CODBCRecordset rs(&m_DB);

	try
	{
		auto temp = FilterSQL(szNewName);

		CStringSQL strSQL;
		strSQL.Format("SELECT COUNT(*) AS NUM FROM Character WHERE Name='%s'", temp.c_str());
		rs.Open(strSQL, CRecordset::forwardOnly, CRecordset::readOnly);
	}
	catch (CDBException* e)
	{
		Log("MSSQLDatabase::CreateCharacter - %s\n", e->m_strError);

		return MERR_UNKNOWN;
	}

	if ((rs.IsOpen() == false) || (rs.GetRecordCount() <= 0) || (rs.IsBOF() == true))
	{
		return MERR_UNKNOWN;
	}
	if (rs.Field("NUM").AsInt() > 0)
	{
		return MERR_CLIENT_EXIST_CHARNAME;
	}

	try
	{
		CStringSQL strSQL;

		auto temp = FilterSQL(szNewName);

		strSQL.Format(g_szDB_CREATE_CHAR, nAID, nCharIndex, temp.c_str(), nSex, nHair, nFace, nCostume);
		m_DB.ExecuteSQL(strSQL);
	}
	catch (CDBException* e)
	{
		Log("MSSQLDatabase::CreateCharacter - %s\n", e->m_strError);
		return MERR_UNKNOWN;
	}

	_STATUS_DB_END(2);

	return MOK;
}

bool MSSQLDatabase::GetAccountCharList(const int nAID, MTD_AccountCharInfo* poutCharList, int* noutCharCount)
{
	_STATUS_DB_START
		if (!CheckOpen()) return false;

	CStringSQL strSQL;
	strSQL.Format(g_szDB_GET_CHARLIST, nAID);
	CODBCRecordset rs(&m_DB);

	bool bException = false;
	try
	{
		rs.Open(strSQL, CRecordset::forwardOnly, CRecordset::readOnly);
	}
	catch (CDBException* e)
	{
		bException = true;
		Log("MSSQLDatabase::GetAccountCharList - %s\n", e->m_strError);
		return false;
	}

	if (rs.IsOpen() == false)
	{
		return false;
	}

	int t = 0;
	for (; !rs.IsEOF(); rs.MoveNext())
	{
		if (t >= 4) break;

		memset(&poutCharList[t], 0, sizeof(MTD_AccountCharInfo));

		strcpy_safe(poutCharList[t].szName, (const char*)rs.Field("Name").AsString());
		poutCharList[t].nCharNum = rs.Field("CharNum").AsInt();
		poutCharList[t].nLevel = rs.Field("Level").AsInt();
		t++;
	}

	*noutCharCount = t;

	_STATUS_DB_END(3);

	return true;
}

bool MSSQLDatabase::GetAccountCharInfo(const int nAID, const int nCharIndex, MTD_CharInfo* poutCharInfo)
{
	_STATUS_DB_START
		if (!CheckOpen()) return false;

	CStringSQL strSQL;

	strSQL.Format(g_szDB_GET_ACCOUNT_CHARINFO, nAID, nCharIndex);
	CODBCRecordset rs(&m_DB);

	bool bException = false;
	try
	{
		rs.Open(strSQL, CRecordset::forwardOnly, CRecordset::readOnly);
	}
	catch (CDBException* e)
	{
		bException = true;
		Log("MSSQLDatabase::GetAccountCharInfo - %s\n", e->m_strError);
		return false;
	}

	if ((rs.IsOpen() == false) || (rs.GetRecordCount() <= 0) || (rs.IsBOF() == true))
	{
		return false;
	}

	int t = 0;

	memset(poutCharInfo, 0, sizeof(MTD_CharInfo));

	strcpy_safe(poutCharInfo->szName, (const char*)rs.Field("Name").AsString());
	strcpy_safe(poutCharInfo->szClanName, (const char*)rs.Field("ClanName").AsString());

	poutCharInfo->nCharNum = rs.Field("CharNum").AsInt();
	poutCharInfo->nLevel = rs.Field("Level").AsInt();
	poutCharInfo->nSex = rs.Field("Sex").AsInt();
	poutCharInfo->nHair = rs.Field("Hair").AsInt();
	poutCharInfo->nFace = rs.Field("Face").AsInt();
	poutCharInfo->nXP = (u32)rs.Field("XP").AsLong();
	poutCharInfo->nBP = (u32)rs.Field("BP").AsLong();

	poutCharInfo->nEquipedItemDesc[MMCIP_HEAD] = rs.Field("head_itemid").AsInt();
	poutCharInfo->nEquipedItemDesc[MMCIP_CHEST] = rs.Field("chest_itemid").AsInt();
	poutCharInfo->nEquipedItemDesc[MMCIP_HANDS] = rs.Field("hands_itemid").AsInt();
	poutCharInfo->nEquipedItemDesc[MMCIP_LEGS] = rs.Field("legs_itemid").AsInt();
	poutCharInfo->nEquipedItemDesc[MMCIP_FEET] = rs.Field("feet_itemid").AsInt();
	poutCharInfo->nEquipedItemDesc[MMCIP_FINGERL] = rs.Field("fingerl_itemid").AsInt();
	poutCharInfo->nEquipedItemDesc[MMCIP_FINGERR] = rs.Field("fingerr_itemid").AsInt();
	poutCharInfo->nEquipedItemDesc[MMCIP_MELEE] = rs.Field("melee_itemid").AsInt();
	poutCharInfo->nEquipedItemDesc[MMCIP_PRIMARY] = rs.Field("primary_itemid").AsInt();
	poutCharInfo->nEquipedItemDesc[MMCIP_SECONDARY] = rs.Field("secondary_itemid").AsInt();
	poutCharInfo->nEquipedItemDesc[MMCIP_CUSTOM1] = rs.Field("custom1_itemid").AsInt();
	poutCharInfo->nEquipedItemDesc[MMCIP_CUSTOM2] = rs.Field("custom2_itemid").AsInt();

	_STATUS_DB_END(50);

	return true;
}

bool MSSQLDatabase::GetCharInfoByAID(const int nAID, const int nCharIndex, MMatchCharInfo* poutCharInfo, int& nWaitHourDiff)
{
	_STATUS_DB_START
		if (!CheckOpen()) return false;

	CStringSQL strSQL;

	strSQL.Format(g_szDB_GET_CHARINFO_BY_AID, nAID, nCharIndex);
	CODBCRecordset rs(&m_DB);

	bool bException = false;
	try
	{
		rs.Open(strSQL, CRecordset::forwardOnly, CRecordset::readOnly);
	}
	catch (CDBException* e)
	{
		bException = true;
		Log("MSSQLDatabase::GetCharInfoByAID - %s\n", e->m_strError);
		return false;
	}

	if ((rs.IsOpen() == false) || (rs.GetRecordCount() <= 0) || (rs.IsBOF() == true))
	{
		return false;
	}
	poutCharInfo->m_nCID = (u32)rs.Field("CID").AsLong();
	CStringSQL db_strName = rs.Field("Name").AsString();
	strcpy_safe(poutCharInfo->m_szName, (const char*)db_strName);
	poutCharInfo->m_nLevel = (int)rs.Field("Level").AsInt();
	poutCharInfo->m_nSex = (MMatchSex)rs.Field("Sex").AsInt();
	poutCharInfo->m_nHair = rs.Field("Hair").AsInt();
	poutCharInfo->m_nFace = rs.Field("Face").AsInt();
	poutCharInfo->m_nXP = (u32)rs.Field("XP").AsLong();
	poutCharInfo->m_nBP = (u32)rs.Field("BP").AsLong();
	poutCharInfo->m_nHP = (int)rs.Field("HP").AsInt();
	poutCharInfo->m_nAP = (int)rs.Field("AP").AsInt();
	poutCharInfo->m_nFR = (int)rs.Field("FR").AsInt();
	poutCharInfo->m_nCR = (int)rs.Field("CR").AsInt();
	poutCharInfo->m_nER = (int)rs.Field("ER").AsInt();
	poutCharInfo->m_nWR = (int)rs.Field("WR").AsInt();
	poutCharInfo->m_nCharNum = (int)rs.Field("CharNum").AsInt();

	poutCharInfo->m_nTotalKillCount = rs.Field("KillCount").AsInt();
	poutCharInfo->m_nTotalDeathCount = rs.Field("DeathCount").AsInt();
	poutCharInfo->m_nTotalPlayTimeSec = rs.Field("PlayTime").AsInt();

	poutCharInfo->m_ClanInfo.m_nClanID = (int)rs.Field("CLID").AsInt();

	CStringSQL db_strClanName = rs.Field("ClanName").AsString();
	strcpy_safe(poutCharInfo->m_ClanInfo.m_szClanName, (const char*)db_strClanName);

	poutCharInfo->m_ClanInfo.m_nGrade = (MMatchClanGrade)rs.Field("ClanGrade").AsInt();
	poutCharInfo->m_ClanInfo.m_nContPoint = (MMatchClanGrade)rs.Field("ClanContPoint").AsInt();

	if (!rs.Field("DeleteDate").IsNull())
		poutCharInfo->m_ClanInfo.m_strDeleteDate = rs.Field("DeleteDate").AsString().GetBuffer();

	if (!rs.Field("WaitHourDiff").IsNull())
		nWaitHourDiff = (int)rs.Field("WaitHourDiff").AsInt();

	memset(&poutCharInfo->m_nEquipedItemCIID, 0, sizeof(poutCharInfo->m_nEquipedItemCIID));

	poutCharInfo->m_nEquipedItemCIID[MMCIP_HEAD] = (u32)rs.Field("head_slot").AsLong();
	poutCharInfo->m_nEquipedItemCIID[MMCIP_CHEST] = (u32)rs.Field("chest_slot").AsLong();
	poutCharInfo->m_nEquipedItemCIID[MMCIP_HANDS] = (u32)rs.Field("hands_slot").AsLong();
	poutCharInfo->m_nEquipedItemCIID[MMCIP_LEGS] = (u32)rs.Field("legs_slot").AsLong();
	poutCharInfo->m_nEquipedItemCIID[MMCIP_FEET] = (u32)rs.Field("feet_slot").AsLong();
	poutCharInfo->m_nEquipedItemCIID[MMCIP_FINGERL] = (u32)rs.Field("fingerl_slot").AsLong();
	poutCharInfo->m_nEquipedItemCIID[MMCIP_FINGERR] = (u32)rs.Field("fingerr_slot").AsLong();
	poutCharInfo->m_nEquipedItemCIID[MMCIP_MELEE] = (u32)rs.Field("melee_slot").AsLong();
	poutCharInfo->m_nEquipedItemCIID[MMCIP_PRIMARY] = (u32)rs.Field("primary_slot").AsLong();
	poutCharInfo->m_nEquipedItemCIID[MMCIP_SECONDARY] = (u32)rs.Field("secondary_slot").AsLong();
	poutCharInfo->m_nEquipedItemCIID[MMCIP_CUSTOM1] = (u32)rs.Field("custom1_slot").AsLong();
	poutCharInfo->m_nEquipedItemCIID[MMCIP_CUSTOM2] = (u32)rs.Field("custom2_slot").AsLong();

	_STATUS_DB_END(4);

	return true;
}

bool MSSQLDatabase::GetAccountInfo(int nAID, MMatchAccountInfo* poutAccountInfo)
{
	_STATUS_DB_START
		if (!CheckOpen()) return false;

	CStringSQL strSQL;
	strSQL.Format(g_szDB_GET_ACCOUNTINFO, nAID);

	CODBCRecordset rs(&m_DB);

	bool bException = false;
	try
	{
		rs.Open(strSQL, CRecordset::forwardOnly, CRecordset::readOnly);
	}
	catch (CDBException* e)
	{
		bException = true;
		Log("MSSQLDatabase::GetAccountInfo - %s\n", e->m_strError);
		return false;
	}

	if ((rs.IsOpen() == false) || (rs.GetRecordCount() <= 0) || (rs.IsBOF() == true))
	{
		return false;
	}

	poutAccountInfo->m_nAID = (u32)rs.Field("AID").AsLong();
	poutAccountInfo->m_nUGrade = (MMatchUserGradeID)rs.Field("UGradeID").AsInt();
	poutAccountInfo->m_nPGrade = MMPG_FREE;
	strcpy_safe(poutAccountInfo->m_szUserID, (const char*)rs.Field("UserID").AsString());

#ifdef _BLOCK_HACKER
	if (rs.Field("BlockType").IsNull())
		poutAccountInfo->m_BlockType = MMBT_NO;
	else
	{
		poutAccountInfo->m_BlockType = static_cast<MMatchBlockType>(rs.Field("BlockType").AsInt());
		if (MMBT_NO != poutAccountInfo->m_BlockType)
		{
			poutAccountInfo->m_EndBlockDate.wYear = static_cast<WORD>(rs.Field("year").AsShort());
			poutAccountInfo->m_EndBlockDate.wMonth = static_cast<WORD>(rs.Field("month").AsShort());
			poutAccountInfo->m_EndBlockDate.wDay = static_cast<WORD>(rs.Field("day").AsShort());
			poutAccountInfo->m_EndBlockDate.wHour = static_cast<WORD>(rs.Field("hour").AsShort());
			poutAccountInfo->m_EndBlockDate.wMinute = static_cast<WORD>(rs.Field("min").AsShort());
		}
	}
#endif

	_STATUS_DB_END(5);

	return true;
}

bool MSSQLDatabase::DeleteCharacter(const int nAID, const int nCharIndex, const char* szCharName)
{
	_STATUS_DB_START

		if (!CheckOpen()) return false;

	CStringSQL strSQL;
	strSQL.Format(g_szDB_DELETE_CHAR, nAID, nCharIndex, FilterSQL(szCharName).c_str());

	CODBCRecordset rs(&m_DB);

	try
	{
		rs.Open(strSQL, CRecordset::forwardOnly, CRecordset::readOnly);
	}
	catch (CDBException* e)
	{
		Log("MSSQLDatabase::DeleteCharacter - %s\n", e->m_strError);
		return false;
	}

	if ((rs.IsOpen() == false) || (rs.GetRecordCount() <= 0) || (rs.IsBOF() == true))
	{
		return false;
	}

	int nRet = 0;
	nRet = rs.Field("Ret").AsInt();
	if (nRet != 1) return false;

	_STATUS_DB_END(6);
	return true;
}

bool MSSQLDatabase::SimpleUpdateCharInfo(const MMatchCharInfo& CharInfo)
{
	_STATUS_DB_START
		if (!CheckOpen()) return false;

	try
	{
		CStringSQL strSQL;
		strSQL.Format(g_szDB_SIMPLE_UPDATE_CHARINFO, CharInfo.m_nCID, FilterSQL(CharInfo.m_szName).c_str(),
			CharInfo.m_nLevel, CharInfo.m_nXP, CharInfo.m_nBP);

		m_DB.ExecuteSQL(strSQL);
	}
	catch (CDBException* e)
	{
		Log("MSSQLDatabase::SimpleUpdateCharInfo - %s\n", e->m_strError);
		return false;
	}

	_STATUS_DB_END(7);
	return true;
}

bool MSSQLDatabase::InsertCharItem(const unsigned int nCID, int nItemDescID, bool bRentItem, int nRentPeriodHour, u32* poutCIID)
{
	_STATUS_DB_START
		if (!CheckOpen()) return false;

	if (bRentItem == false) nRentPeriodHour = 0;

	CStringSQL strSQL;
	strSQL.Format(g_szDB_INSERT_CHAR_ITEM, nCID, nItemDescID);
	CODBCRecordset rs(&m_DB);

	bool bException = false;
	try
	{
		rs.Open(strSQL, CRecordset::forwardOnly, CRecordset::readOnly);
	}
	catch (CDBException* e)
	{
		bException = true;
		Log("MSSQLDatabase::InsertCharItem - %s\n", e->m_strError);
		Log("MSSQLDatabase::InsertCharItem - Err Query : %s\n", strSQL.GetBuffer());
		return false;
	}

	if ((rs.IsOpen() == false) || (rs.GetRecordCount() <= 0) || (rs.IsBOF() == true))
	{
		return false;
	}

	*poutCIID = rs.Field("ORDERCIID").AsLong();

	_STATUS_DB_END(8);
	return true;
}

bool MSSQLDatabase::BuyBountyItem(const unsigned int nCID, int nItemID, int nPrice, u32* poutCIID)
{
	_STATUS_DB_START
		if (!CheckOpen()) return false;

	CStringSQL strSQL;
	strSQL.Format(g_szDB_BUY_BOUNTY_ITEM, nCID, nItemID, nPrice);
	CODBCRecordset rs(&m_DB);

	bool bException = false;
	try
	{
		rs.Open(strSQL, CRecordset::forwardOnly, CRecordset::readOnly);
	}
	catch (CDBException* e)
	{
		bException = true;
		Log("MSSQLDatabase::BuyBountyItem - %s , Query: %s\n", e->m_strError, strSQL.GetBuffer());
		return false;
	}

	if ((rs.IsOpen() == false) || (rs.GetRecordCount() <= 0) || (rs.IsBOF() == true))
	{
		return false;
	}

	*poutCIID = rs.Field("ORDERCIID").AsLong();

	_STATUS_DB_END(55);
	return true;
}

bool MSSQLDatabase::SellBountyItem(const unsigned int nCID, unsigned int nItemID, unsigned int nCIID, int nPrice, int nCharBP)
{
	_STATUS_DB_START
		if (!CheckOpen()) return false;

	CStringSQL strSQL;
	strSQL.Format(g_szDB_SELL_BOUNTY_ITEM, nCID, nItemID, nCIID, nPrice, nCharBP);
	CODBCRecordset rs(&m_DB);

	bool bException = false;
	try
	{
		rs.Open(strSQL, CRecordset::forwardOnly, CRecordset::readOnly);
	}
	catch (CDBException* e)
	{
		bException = true;
		Log("MSSQLDatabase::SellBountyItem - %s , Query: %s\n", e->m_strError, strSQL.GetBuffer());
		return false;
	}

	if ((rs.IsOpen() == false) || (rs.GetRecordCount() <= 0) || (rs.IsBOF() == true))
	{
		return false;
	}

	int nRet = rs.Field("Ret").AsLong();

	_STATUS_DB_END(56);

	if (nRet == 1) return true;

	return false;
}

bool MSSQLDatabase::DeleteCharItem(const unsigned int nCID, int nCIID)
{
	_STATUS_DB_START
		if (!CheckOpen()) return false;

	try
	{
		CStringSQL strSQL;
		strSQL.Format(g_szDB_DELETE_CHAR_ITEM, nCID, nCIID);
		m_DB.ExecuteSQL(strSQL);
	}
	catch (CDBException* e)
	{
		Log("MSSQLDatabase::DeleteCharItem - %s\n", e->m_strError);
		return false;
	}

	_STATUS_DB_END(9);
	return true;
}

bool MSSQLDatabase::GetCharItemInfo(MMatchCharInfo& CharInfo)
{
	_STATUS_DB_START
		if (!CheckOpen()) return false;

	CStringSQL strSQL;
	strSQL.Format(g_szDB_SELECT_CHAR_ITEM, CharInfo.m_nCID);
	CODBCRecordset rs(&m_DB);

	bool bException = false;
	try
	{
		rs.Open(strSQL, CRecordset::forwardOnly, CRecordset::readOnly);
	}
	catch (CDBException* e)
	{
		bException = true;
		Log("MSSQLDatabase::GetCharItemInfo - %s\n", e->m_strError);
		return false;
	}

	if (rs.IsOpen() == false)
	{
		return false;
	}

	int t = 0;
	int nItemDescID = 0;
	u32 nCIID = 0;

	for (; !rs.IsEOF(); rs.MoveNext())
	{
		nItemDescID = 0; nCIID = 0;
		nItemDescID = (int)rs.Field("ItemID").AsInt();
		nCIID = (u32)rs.Field("CIID").AsLong();

		bool bIsRentItem = !rs.Field("RentPeriodRemainder").IsNull();
		int nRentMinutePeriodRemainder = RENT_MINUTE_PERIOD_UNLIMITED;
		bool bSpendingItem = false;
		int nCnt = -1;

		if (bIsRentItem)
		{
			nRentMinutePeriodRemainder = rs.Field("RentPeriodRemainder").AsInt();
		}

		MUID uidNew = MMatchItemMap::UseUID();
		CharInfo.m_ItemList.CreateItem(uidNew, nCIID, nItemDescID, bIsRentItem, nRentMinutePeriodRemainder);

		t++;
	}

	CharInfo.m_ItemList.SetDbAccess();

	_STATUS_DB_END(10);
	return true;
}

bool MSSQLDatabase::GetAccountItemInfo(const int nAID, MAccountItemNode* pOut, int* poutNodeCount, int nMaxNodeCount,
	MAccountItemNode* pOutExpiredItemList, int* poutExpiredItemCount, int nMaxExpiredItemCount)
{
	_STATUS_DB_START
		if (!CheckOpen()) return false;

	CStringSQL strSQL;
	strSQL.Format(g_szDB_SELECT_ACCOUNT_ITEM, nAID);
	CODBCRecordset rs(&m_DB);

	bool bException = false;
	try
	{
		rs.Open(strSQL, CRecordset::forwardOnly, CRecordset::readOnly);
	}
	catch (CDBException* e)
	{
		bException = true;
		Log("MSSQLDatabase::GetAccountItemInfo - %s\n", e->m_strError);
		return false;
	}

	if (rs.IsOpen() == false)
	{
		return false;
	}

	int aiid = 0, itemid = 0;
	int nodecount = 0;

	int nExpiredItemCount = 0;

	for (; !rs.IsEOF(); rs.MoveNext())
	{
		itemid = (int)rs.Field("ItemID").AsInt();
		aiid = (int)rs.Field("AIID").AsInt();

		bool bIsRentItem = !rs.Field("RentPeriodRemainder").IsNull();
		int nRentMinutePeriodRemainder = RENT_MINUTE_PERIOD_UNLIMITED;
		bool bSpendingItem = false;
		int nCnt = -1;

		if (bIsRentItem)
		{
			nRentMinutePeriodRemainder = rs.Field("RentPeriodRemainder").AsInt();
		}

		if ((bIsRentItem) && (nRentMinutePeriodRemainder <= 0))
		{
			if (nExpiredItemCount < nMaxExpiredItemCount)
			{
				pOutExpiredItemList[nExpiredItemCount].nAIID = aiid;
				pOutExpiredItemList[nExpiredItemCount].nItemID = (u32)itemid;
				nExpiredItemCount++;
			}
		}
		else
		{
			pOut[nodecount].nAIID = aiid;
			pOut[nodecount].nItemID = (u32)itemid;
			pOut[nodecount].nRentMinutePeriodRemainder = nRentMinutePeriodRemainder;

			nodecount++;
			if (nodecount >= nMaxNodeCount) break;
		}
	}

	*poutNodeCount = nodecount;
	*poutExpiredItemCount = nExpiredItemCount;

	_STATUS_DB_END(25);
	return true;
}

bool MSSQLDatabase::UpdateEquipedItem(const u32 nCID, MMatchCharItemParts parts,
	u32 nCIID, const u32 nItemID)
{
	_STATUS_DB_START
		if (!CheckOpen()) return false;

	CStringSQL strSQL;
	strSQL.Format(g_szDB_UPDATE_EQUIPITEM, (int)nCID, (int)parts, (int)nCIID, (int)nItemID);
	CODBCRecordset rs(&m_DB);

	int nRet = 0;
	try
	{
		rs.Open(strSQL, CRecordset::forwardOnly, CRecordset::readOnly);
	}
	catch (CDBException* e)
	{
		Log("MSSQLDatabase::UpdateEquipedItem - %s\n", e->m_strError);
		return false;
	}

	if ((rs.IsOpen() == false) || (rs.GetRecordCount() <= 0) || (rs.IsBOF() == true))
	{
		return false;
	}

	_STATUS_DB_END(11);

	return rs.Field("Ret").AsInt() != 0;
}

static bool SplitStrIP(int(&out)[4], const char* szIP)
{
	if (sscanf(szIP, "%d.%d.%d.%d", &out[0], &out[1], &out[2], &out[3]) != 4)
		return false;

	for (auto val : out)
		if (val < 0 || val > 255)
			return false;

	return true;
}

bool MSSQLDatabase::InsertConnLog(const int nAID, const char* szIP, const string& strCountryCode3)
{
	_STATUS_DB_START
		if (!CheckOpen()) return false;

	if (0 == szIP) return false;

	int nIP[4];
	if (!SplitStrIP(nIP, szIP))
	{
		mlog("MSSQLDatabase::InsertConnLog - 문자열 IP(%s)분해 실패. AID(%u)\n",
			nAID, szIP);
		return false;
	}

	try
	{
		auto temp = FilterSQL(strCountryCode3);

		CStringSQL strSQL;
		strSQL.Format(g_szDB_INSERT_CONN_LOG, nAID, nIP[0], nIP[1], nIP[2], nIP[3], temp.c_str());

		m_DB.ExecuteSQL(strSQL);
	}
	catch (CDBException* e)
	{
		Log("MSSQLDatabase::InsertConnLog - %s\n", e->m_strError);
		return false;
	}

	_STATUS_DB_END(12);
	return true;
}

bool MSSQLDatabase::InsertGameLog(const char* szGameName, const char* szMap, const char* szGameType,
	const int nRound, const unsigned int nMasterCID,
	const int nPlayerCount, const char* szPlayers)
{
	_STATUS_DB_START
		if (!CheckOpen()) return false;

	CStringSQL strSQL;
	try
	{
		string strStageName = FilterSQL(string(szGameName));

		strSQL.Format(g_szDB_INSERT_GAME_LOG, &strStageName[0], szMap, szGameType, nRound, nMasterCID,
			nPlayerCount, szPlayers);

		m_DB.ExecuteSQL(strSQL);
	}
	catch (CDBException* e)
	{
		Log("MSSQLDatabase::InsertGameLog - %s Query:%s\n", e->m_strError, strSQL.GetBuffer());

		return false;
	}

	_STATUS_DB_END(13);
	return true;
}

bool MSSQLDatabase::InsertKillLog(const unsigned int nAttackerCID, const unsigned int nVictimCID)
{
	if (!CheckOpen()) return false;

	try
	{
		CStringSQL strSQL;
		strSQL.Format(g_szDB_INSERT_KILL_LOG, nAttackerCID, nVictimCID);

		m_DB.ExecuteSQL(strSQL);
	}
	catch (CDBException* e)
	{
		Log("MSSQLDatabase::InsertKillLog - %s\n", e->m_strError);
		return false;
	}

	return true;
}

bool MSSQLDatabase::InsertChatLog(const u32 nCID, const char* szMsg, u64 nTime)
{
	return true;

	if (!CheckOpen()) return false;

	try
	{
		auto temp = FilterSQL(szMsg);
		CStringSQL strSQL;
		strSQL.Format("INSERT Into ChatLog (CID, Msg, Time) Values (%u, '%s', GETDATE())", nCID, temp.c_str());
		m_DB.ExecuteSQL(strSQL);
	}
	catch (CDBException* e)
	{
		Log("MSSQLDatabase::InsertChatLog - %s\n", e->m_strError);
		return false;
	}

	return true;
}

bool MSSQLDatabase::UpdateCharLevel(const int nCID, const int nLevel)
{
	_STATUS_DB_START
		if (!CheckOpen()) return false;
	try
	{
		CStringSQL strSQL;
		strSQL.Format(g_szDB_UPDATE_CHAR_LEVEL, nLevel, nCID);

		m_DB.ExecuteSQL(strSQL);
	}
	catch (CDBException* e)
	{
		Log("MSSQLDatabase::UpdateCharLevel - %s\n", e->m_strError);
		return false;
	}

	_STATUS_DB_END(14);
	return true;
}

bool MSSQLDatabase::UpdateCharBP(const int nCID, const int nBPInc)
{
	_STATUS_DB_START
		if (!CheckOpen()) return false;

#ifdef _DEBUG
	mlog("MSSQLDatabase::UpdateCharBP - Added bounty price:%d\n", nBPInc);
#endif

	try
	{
		CStringSQL strSQL;
		strSQL.Format(g_szDB_UPDATE_CHAR_BP, nBPInc, nCID);

		m_DB.ExecuteSQL(strSQL);
	}
	catch (CDBException* e)
	{
		Log("MSSQLDatabase::UpdateCharBP - %s\n", e->m_strError);
		return false;
	}

	_STATUS_DB_END(15);
	return true;
}

bool MSSQLDatabase::InsertItemPurchaseLogByBounty(const u32 nItemID, const u32 nCID,
	const int nBounty, const int nCharBP, const ItemPurchaseType nType)
{
	_STATUS_DB_START
		if (!CheckOpen()) return false;

	char szType[8] = "";

	switch (nType)
	{
	case ItemPurchaseType::Buy: strcpy_safe(szType, "구입"); break;
	case ItemPurchaseType::Sell: strcpy_safe(szType, "판매"); break;
	}

	try
	{
		CStringSQL strSQL;
		strSQL.Format(g_szDB_INSERT_ITEM_PURCHASE_BY_BOUNTY_LOG, nItemID, nCID, nBounty, nCharBP, szType);

		m_DB.ExecuteSQL(strSQL);
	}
	catch (CDBException* e)
	{
		Log("MSSQLDatabase::InsertItemPurchaseLogByBounty - %s\n", e->m_strError);
		return false;
	}

	_STATUS_DB_END(16);
	return true;
}

bool MSSQLDatabase::InsertCharMakingLog(const unsigned int nAID, const char* szCharName,
	const CharMakingType nType)
{
	_STATUS_DB_START
		if (!CheckOpen()) return false;

	char szType[8] = "";

	switch (nType)
	{
	case CharMakingType::Create: strcpy_safe(szType, "생성"); break;
	case CharMakingType::Delete: strcpy_safe(szType, "삭제"); break;
	}

	try
	{
		auto temp = FilterSQL(szCharName);

		CStringSQL strSQL;
		strSQL.Format(g_szDB_INSERT_CHAR_MAKING_LOG, nAID, temp.c_str(), szType);

		m_DB.ExecuteSQL(strSQL);
	}
	catch (CDBException* e)
	{
		Log("MSSQLDatabase::InsertCharMakingLog - %s\n", e->m_strError);
		return false;
	}

	_STATUS_DB_END(17);
	return true;
}

bool MSSQLDatabase::InsertServerLog(const int nServerID, const int nPlayerCount, const int nGameCount,
	const uint32_t dwBlockCount, const uint32_t dwNonBlockCount)
{
	_STATUS_DB_START
		if (!CheckOpen()) return false;

	try
	{
		CStringSQL strSQL;
		strSQL.Format(g_szDB_INSERT_SERVER_LOG, nServerID, nPlayerCount, nGameCount, dwBlockCount, dwNonBlockCount);

		m_DB.ExecuteSQL(strSQL);
	}
	catch (CDBException* e)
	{
		Log("MSSQLDatabase::InsertServerLog - %s\n", e->m_strError);
		return false;
	}

	_STATUS_DB_END(18);
	return true;
}

bool MSSQLDatabase::UpdateServerStatus(const int nServerID, const int nCurrPlayer)
{
	_STATUS_DB_START
		if (!CheckOpen()) return false;

	try
	{
		CStringSQL strSQL;
		strSQL.Format(g_szDB_UPDATE_SERVER_STATUS, nCurrPlayer, nServerID);

		m_DB.ExecuteSQL(strSQL);
	}
	catch (CDBException* e)
	{
		Log("MSSQLDatabase::UpdateServerStatus - %s\n", e->m_strError);
		return false;
	}

	_STATUS_DB_END(19);
	return true;
}

bool MSSQLDatabase::UpdateServerInfo(const int nServerID, const int nMaxPlayer, const char* szServerName)
{
	_STATUS_DB_START
		if (!CheckOpen()) return false;

	try
	{
		auto temp = FilterSQL(szServerName);

		CStringSQL strSQL;
		strSQL.Format(g_szDB_UPDATE_SERVER_INFO, nMaxPlayer, temp.c_str(), nServerID);

		m_DB.ExecuteSQL(strSQL);
	}
	catch (CDBException* e)
	{
		Log("MSSQLDatabase::UpdateServerInfo - %s\n", e->m_strError);
		return false;
	}

	_STATUS_DB_END(20);
	return true;
}

bool MSSQLDatabase::InsertPlayerLog(const u32 nCID,
	const int nPlayTime, const int nKillCount, const int nDeathCount, const int nXP, const int nTotalXP)
{
	_STATUS_DB_START
		if (!CheckOpen()) return false;

	try
	{
		CStringSQL strSQL;
		strSQL.Format(g_szDB_INSERT_PLAYER_LOG, nCID, nPlayTime, nKillCount, nDeathCount, nXP, nTotalXP);

		m_DB.ExecuteSQL(strSQL);
	}
	catch (CDBException* e)
	{
		Log("MSSQLDatabase::InsertPlayerLog - %s\n", e->m_strError);
		return false;
	}

	_STATUS_DB_END(21);
	return true;
}

bool MSSQLDatabase::InsertLevelUpLog(const int nCID, const int nLevel, const int nBP,
	const int nKillCount, const int nDeathCount, const int nPlayTime)
{
	_STATUS_DB_START
		if (!CheckOpen()) return false;

	try
	{
		CStringSQL strSQL;
		strSQL.Format(g_szDB_INSERT_LEVELUP_LOG, nCID, nLevel, nBP, nKillCount, nDeathCount, nPlayTime);

		m_DB.ExecuteSQL(strSQL);
	}
	catch (CDBException* e)
	{
		Log("MSSQLDatabase::InsertLevelUpLog - %s\n", e->m_strError);
		return false;
	}

	_STATUS_DB_END(41);
	return true;
}

bool MSSQLDatabase::UpdateCharPlayTime(const u32 nCID, const u32 nPlayTime)
{
	_STATUS_DB_START
		if (!CheckOpen()) return false;

	try
	{
		CStringSQL strSQL;
		strSQL.Format(g_szDB_UPDATE_CHAR_PLAYTIME, nPlayTime, nCID);

		m_DB.ExecuteSQL(strSQL);
	}
	catch (CDBException* e)
	{
		Log("MSSQLDatabase::UpdateCharPlayTime - %s\n", e->m_strError);
		return false;
	}

	_STATUS_DB_END(22);
	return true;
}

bool MSSQLDatabase::UpdateCharInfoData(const int nCID, const int nAddedXP, const int nAddedBP,
	const int nAddedKillCount, const int nAddedDeathCount)
{
	_STATUS_DB_START
		if (!CheckOpen()) return false;

	try
	{
		CStringSQL strSQL;
		strSQL.Format(g_szDB_UPDATE_CHAR_INFO_DATA, nAddedXP, nAddedBP,
			nAddedKillCount, nAddedDeathCount, nCID);

		m_DB.ExecuteSQL(strSQL);
	}
	catch (CDBException* e)
	{
		Log("MSSQLDatabase::UpdateCharInfoData - %s\n", e->m_strError);
		return false;
	}

	_STATUS_DB_END(23);
	return true;
}

bool MSSQLDatabase::UpdateLastConnDate(const char* szUserID, const char* szIP)
{
	_STATUS_DB_START
		if (!CheckOpen()) return false;

	try
	{
		auto temp = FilterSQL(szIP);

		CStringSQL strSQL;
		strSQL.Format(g_szDB_UPDATE_LAST_CONNDATE, temp.c_str(), szUserID);

		m_DB.ExecuteSQL(strSQL);
	}
	catch (CDBException* e)
	{
		Log("MSSQLDatabase::UpdateLastConnDate - %s\n", e->m_strError);
		return false;
	}

	_STATUS_DB_END(24);
	return true;
}

bool MSSQLDatabase::BringAccountItem(const int nAID, const int nCID, const int nAIID,
	unsigned int* poutCIID, u32* poutItemID,
	bool* poutIsRentItem, int* poutRentMinutePeriodRemainder)
{
	_STATUS_DB_START
		if (!CheckOpen()) return false;

	CStringSQL strSQL;
	strSQL.Format(g_szDB_BRING_ACCOUNTITEM, nAID, nCID, nAIID);
	CODBCRecordset rs(&m_DB);

	try
	{
		rs.Open(strSQL, CRecordset::forwardOnly, CRecordset::readOnly);
	}
	catch (CDBException* e)
	{
		Log("MSSQLDatabase::BringAccountItem - %s\n", e->m_strError);
		return false;
	}

	if ((rs.IsOpen() == false) || (rs.GetRecordCount() <= 0) || (rs.IsBOF() == true))
	{
		return false;
	}

	*poutCIID = rs.Field("ORDERCIID").AsLong();
	*poutItemID = rs.Field("ItemID").AsLong();

	bool bIsRentItem = !rs.Field("RentPeriodRemainder").IsNull();
	int nRentMinutePeriodRemainder = RENT_MINUTE_PERIOD_UNLIMITED;

	if (bIsRentItem)
	{
		nRentMinutePeriodRemainder = rs.Field("RentPeriodRemainder").AsInt();
	}

	*poutIsRentItem = bIsRentItem;
	*poutRentMinutePeriodRemainder = nRentMinutePeriodRemainder;

	_STATUS_DB_END(26);
	return true;
}

bool MSSQLDatabase::BringBackAccountItem(const int nAID, const int nCID, const int nCIID)
{
	_STATUS_DB_START
		if (!CheckOpen()) return false;

	try
	{
		CStringSQL strSQL;
		strSQL.Format(g_szDB_BRING_BACK_ACCOUNTITEM, nAID, nCID, nCIID);

		m_DB.ExecuteSQL(strSQL);
	}
	catch (CDBException* e)
	{
		Log("MSSQLDatabase::BringBackAccountItem - %s\n", e->m_strError);
		return false;
	}

	_STATUS_DB_END(32);
	return true;
}

bool MSSQLDatabase::ClearAllEquipedItem(const u32 nCID)
{
	_STATUS_DB_START
		if (!CheckOpen()) return false;

	try
	{
		CStringSQL strSQL;
		strSQL.Format(g_szDB_CLEARALL_EQUIPEDITEM, nCID);

		m_DB.ExecuteSQL(strSQL);
	}
	catch (CDBException* e)
	{
		Log("MSSQLDatabase::ClearAllEquipedItem - %s\n", e->m_strError);
		return false;
	}

	_STATUS_DB_END(27);
	return true;
}

bool MSSQLDatabase::GetCharCID(const char* pszName, int* poutCID)
{
	_STATUS_DB_START

		if (!CheckOpen()) return false;
	CODBCRecordset rs(&m_DB);

	try
	{
		auto temp = FilterSQL(pszName);

		CStringSQL strSQL;
		strSQL.Format("SELECT CID, Name FROM Character(NOLOCK) WHERE Name='%s'", temp.c_str());
		rs.Open(strSQL, CRecordset::forwardOnly, CRecordset::readOnly);
	}
	catch (CDBException* e)
	{
		Log("MSSQLDatabase::GetCharCID - %s\n", e->m_strError);
		return false;
	}

	if ((rs.IsOpen() == false) || (rs.GetRecordCount() <= 0) || (rs.IsBOF() == true))
	{
		return false;
	}
	*poutCID = rs.Field("CID").AsInt();

	_STATUS_DB_END(28);
	return true;
}

bool MSSQLDatabase::FriendAdd(const int nCID, const int nFriendCID, const int nFavorite)
{
	_STATUS_DB_START

		if (!CheckOpen()) return false;
	CODBCRecordset rs(&m_DB);

	CStringSQL strSQL;
	strSQL.Format(g_szDB_ADD_FRIEND, nCID, nFriendCID, nFavorite);

	bool bException = false;
	try
	{
		rs.Open(strSQL, CRecordset::forwardOnly, CRecordset::readOnly);
	}
	catch (CDBException* e)
	{
		bException = true;
		Log("MSSQLDatabase::FriendAdd - %s\n", e->m_strError);
		return false;
	}

	_STATUS_DB_END(29);
	return true;
}

bool MSSQLDatabase::FriendRemove(const int nCID, const int nFriendCID)
{
	_STATUS_DB_START
		if (!CheckOpen()) return false;

	try
	{
		CStringSQL strSQL;
		strSQL.Format(g_szDB_REMOVE_FRIEND, nCID, nFriendCID);
		m_DB.ExecuteSQL(strSQL);
	}
	catch (CDBException* e)
	{
		Log("MSSQLDatabase::FriendRemove - %s\n", e->m_strError);
		return false;
	}

	_STATUS_DB_END(30);
	return true;
}

bool MSSQLDatabase::FriendGetList(const int nCID, MMatchFriendInfo* pFriendInfo)
{
#define MAX_FRIEND_LISTCOUNT 30

	_STATUS_DB_START
		if (!CheckOpen()) return false;

	CStringSQL strSQL;
	strSQL.Format(g_szDB_GET_FRIEND_LIST, nCID);
	CODBCRecordset rs(&m_DB);

	bool bException = false;
	try
	{
		rs.Open(strSQL, CRecordset::forwardOnly, CRecordset::readOnly);
	}
	catch (CDBException* e)
	{
		bException = true;
		Log("MSSQLDatabase::FriendGetList - %s\n", e->m_strError);
		return false;
	}

	if (rs.IsOpen() == false)
	{
		return false;
	}

	int nFriendCID = 0, nFavorite = 0;
	int nodecount = 0;
	CStringSQL strName;

	for (; !rs.IsEOF(); rs.MoveNext())
	{
		nFriendCID = (int)rs.Field("FriendCID").AsInt();
		nFavorite = (int)rs.Field("Favorite").AsInt();
		strName = rs.Field("Name").AsString();

		pFriendInfo->Add(nFriendCID, nFavorite, (const char*)strName);
	}

	_STATUS_DB_END(31);
	return true;
}

bool MSSQLDatabase::GetCharClan(const int nCID, int* poutClanID, char* poutClanName, int maxlen)
{
	_STATUS_DB_START
		if (!CheckOpen()) return false;

	CStringSQL strSQL;
	strSQL.Format(g_szDB_GET_CHAR_CLAN, nCID);

	CODBCRecordset rs(&m_DB);

	bool bException = false;
	try
	{
		rs.Open(strSQL, CRecordset::forwardOnly, CRecordset::readOnly);
	}
	catch (CDBException* e)
	{
		bException = true;
		Log("MSSQLDatabase::GetCharClan - %s\n", e->m_strError);
		return false;
	}

	if ((rs.IsOpen() == false) || (rs.GetRecordCount() <= 0) || (rs.IsBOF() == true))
	{
		return false;
	}

	*poutClanID = rs.Field("CLID").AsInt();
	strcpy_safe(poutClanName, maxlen, (const char*)rs.Field("ClanName").AsString());

	_STATUS_DB_END(33);

	return true;
}

bool MSSQLDatabase::GetClanIDFromName(const char* szClanName, int* poutCLID)
{
	_STATUS_DB_START
		if (!CheckOpen()) return false;

	auto temp = FilterSQL(szClanName);

	CStringSQL strSQL;
	strSQL.Format(g_szDB_GET_CLID_FROM_CLANNAME, temp.c_str());

	CODBCRecordset rs(&m_DB);

	bool bException = false;
	try
	{
		rs.Open(strSQL, CRecordset::forwardOnly, CRecordset::readOnly);
	}
	catch (CDBException* e)
	{
		bException = true;
		Log("MSSQLDatabase::GetClanIDFromName - %s\n", e->m_strError);

		*poutCLID = 0;
		return false;
	}

	if ((rs.IsOpen() == false) || (rs.GetRecordCount() <= 0) || (rs.IsBOF() == true))
	{
		*poutCLID = 0;
		return false;
	}

	*poutCLID = rs.Field("CLID").AsInt();

	_STATUS_DB_END(34);

	return true;
}

bool MSSQLDatabase::CreateClan(const char* szClanName, const int nMasterCID, const int nMember1CID, const int nMember2CID,
	const int nMember3CID, const int nMember4CID, bool* boutRet, int* noutNewCLID)
{
	_STATUS_DB_START
		if (!CheckOpen()) return false;

	auto temp = FilterSQL(szClanName);

	CStringSQL strSQL;
	strSQL.Format(g_szDB_CREATE_CLAN, temp.c_str(), nMasterCID, nMember1CID, nMember2CID, nMember3CID, nMember4CID);

	CODBCRecordset rs(&m_DB);

	bool bException = false;
	try
	{
		rs.Open(strSQL, CRecordset::forwardOnly, CRecordset::readOnly);
	}
	catch (CDBException* e)
	{
		bException = true;
		Log("MSSQLDatabase::CreateClan - %s\n", e->m_strError);

		*boutRet = false;
		return false;
	}

	if ((rs.IsOpen() == false) || (rs.GetRecordCount() <= 0) || (rs.IsBOF() == true))
	{
		*boutRet = false;
		return false;
	}

	*boutRet = rs.Field("Ret").AsBool();
	*noutNewCLID = rs.Field("NewCLID").AsInt();

	_STATUS_DB_END(35);
	return true;
}

bool MSSQLDatabase::CreateClan(const char* szClanName, const int nMasterCID, bool* boutRet, int* noutNewCLID)
{
	_STATUS_DB_START
		if (!CheckOpen()) return false;

	auto temp = FilterSQL(szClanName);

	CStringSQL strSQL;
	strSQL.Format(g_szDB_CREATE_CLAN2, temp.c_str(), nMasterCID);

	CODBCRecordset rs(&m_DB);

	bool bException = false;
	try
	{
		rs.Open(strSQL, CRecordset::forwardOnly, CRecordset::readOnly);
	}
	catch (CDBException* e)
	{
		bException = true;
		Log("MSSQLDatabase::CreateClan - %s\n", e->m_strError);

		*boutRet = false;
		return false;
	}

	if ((rs.IsOpen() == false) || (rs.GetRecordCount() <= 0) || (rs.IsBOF() == true))
	{
		*boutRet = false;
		return false;
	}

	*boutRet = rs.Field("Ret").AsBool();
	*noutNewCLID = rs.Field("NewCLID").AsInt();

	_STATUS_DB_END(35);
	return true;
}

bool MSSQLDatabase::ReserveCloseClan(const int nCLID, const char* szClanName, const int nMasterCID, const string& strDeleteDate)
{
	_STATUS_DB_START
		if (!CheckOpen()) return false;

	try
	{
		auto temp1 = FilterSQL(szClanName);
		auto temp2 = FilterSQL(strDeleteDate);

		CStringSQL strSQL;
		strSQL.Format(g_szDB_RESERVE_CLOSE_CLAN, nCLID, temp1.c_str(), nMasterCID, temp2.c_str());
		m_DB.ExecuteSQL(strSQL);
	}
	catch (CDBException* e)
	{
		Log("MSSQLDatabase::ReserveCloseClan - %s\n", e->m_strError);
		return false;
	}

	_STATUS_DB_END(36);
	return true;
}

bool MSSQLDatabase::CloseClan(int nCLID, const char* szClanName, int nMasterCID)
{
	_STATUS_DB_START
		if (!CheckOpen()) return false;

	try
	{
		auto temp1 = FilterSQL(szClanName);

		CStringSQL strSQL;
		strSQL.Format(g_szDB_DELETE_CLAN, nCLID, temp1.c_str());
		m_DB.ExecuteSQL(strSQL);
	}
	catch (CDBException* e)
	{
		Log("MSSQLDatabase::CloseClan - %s\n", e->m_strError);
		return false;
	}

	_STATUS_DB_END(36);
	return true;
}

bool MSSQLDatabase::AddClanMember(const int nCLID, const int nJoinerCID, const int nClanGrade, bool* boutRet)
{
	_STATUS_DB_START
		if (!CheckOpen()) return false;

	CStringSQL strSQL;
	strSQL.Format(g_szDB_ADD_CLAN_MEMBER, nCLID, nJoinerCID, nClanGrade);

	CODBCRecordset rs(&m_DB);

	bool bException = false;
	try
	{
		rs.Open(strSQL, CRecordset::forwardOnly, CRecordset::readOnly);
	}
	catch (CDBException* e)
	{
		bException = true;
		Log("MSSQLDatabase::AddClanMember - %s\n", e->m_strError);

		*boutRet = false;
		return false;
	}

	if ((rs.IsOpen() == false) || (rs.GetRecordCount() <= 0) || (rs.IsBOF() == true))
	{
		*boutRet = false;
		return false;
	}

	*boutRet = rs.Field("Ret").AsBool();

	_STATUS_DB_END(37);
	return true;
}

bool MSSQLDatabase::RemoveClanMember(const int nCLID, const int nLeaverCID)
{
	_STATUS_DB_START
		if (!CheckOpen()) return false;

	try
	{
		CStringSQL strSQL;
		strSQL.Format(g_szDB_REMOVE_CLAN_MEMBER, nCLID, nLeaverCID);
		m_DB.ExecuteSQL(strSQL);
	}
	catch (CDBException* e)
	{
		Log("MSSQLDatabase::RemoveClanMember - %s\n", e->m_strError);
		return false;
	}

	_STATUS_DB_END(38);
	return true;
}

bool MSSQLDatabase::UpdateClanGrade(const int nCLID, const int nMemberCID, const int nClanGrade)
{
	_STATUS_DB_START
		if (!CheckOpen()) return false;

	try
	{
		CStringSQL strSQL;
		strSQL.Format(g_szDB_UPDATE_CLAN_GRADE, nCLID, nMemberCID, nClanGrade);
		m_DB.ExecuteSQL(strSQL);
	}
	catch (CDBException* e)
	{
		Log("MSSQLDatabase::UpdateClanGrade - %s\n", e->m_strError);
		return false;
	}

	_STATUS_DB_END(39);
	return true;
}

ExpelResult MSSQLDatabase::ExpelClanMember(const int nCLID, const int nAdminGrade, const char* szMember)
{
	_STATUS_DB_START
		if (!CheckOpen()) return ExpelResult::DBError;

	auto temp = FilterSQL(szMember);

	CStringSQL strSQL;
	strSQL.Format(g_szDB_EXPEL_CLAN_MEMBER, nCLID, nAdminGrade, temp.c_str());

	CODBCRecordset rs(&m_DB);

	bool bException = false;
	try
	{
		rs.Open(strSQL, CRecordset::forwardOnly, CRecordset::readOnly);
	}
	catch (CDBException* e)
	{
		bException = true;
		Log("MSSQLDatabase::ExpelClanMember - %s\n", e->m_strError);

		return ExpelResult::DBError;
	}

	if ((rs.IsOpen() == false) || (rs.GetRecordCount() <= 0) || (rs.IsBOF() == true))
		return ExpelResult::NoSuchMember;

	_STATUS_DB_END(40);

	ExpelResult DBReturnMap[] = { ExpelResult::NoSuchMember, ExpelResult::OK,
		ExpelResult::TooLowGrade, ExpelResult::DBError };

	auto DBReturn = rs.Field("Ret").AsInt();

	if (DBReturn < 0 || DBReturn >= static_cast<int>(std::size(DBReturnMap)))
	{
		Log("MSSQLDatabase::ExpelClanMember - unknown return value %d from DB\n", DBReturn);
		return ExpelResult::DBError;
	}

	return DBReturnMap[DBReturn];
}

bool MSSQLDatabase::GetLadderTeamID(const int nTeamTableIndex, const int* pnMemberCIDArray, const int nMemberCount, int* pnoutTID)
{
	_STATUS_DB_START

		if (!CheckOpen()) return false;

	CStringSQL strSQL;

	if ((nTeamTableIndex == 4) && (nMemberCount == 4))
	{
		strSQL.Format(g_szDB_GET_LADDER_TEAM4_ID, pnMemberCIDArray[0], pnMemberCIDArray[1], pnMemberCIDArray[2], pnMemberCIDArray[3]);
	}

#ifdef _DEBUG
	else if ((nTeamTableIndex == 1) && (nMemberCount == 1))
	{
		strSQL.Format(g_szDB_GET_LADDER_TEAM4_ID, pnMemberCIDArray[0], 1, 1, 1);
	}
	else if ((nTeamTableIndex == 2) && (nMemberCount == 2))
	{
		strSQL.Format(g_szDB_GET_LADDER_TEAM4_ID, pnMemberCIDArray[0], pnMemberCIDArray[1], 1, 1);
	}
#endif
	else
	{
		return false;
	}

	CODBCRecordset rs(&m_DB);

	try
	{
		rs.Open(strSQL, CRecordset::forwardOnly, CRecordset::readOnly);
	}
	catch (CDBException* e)
	{
		Log("MSSQLDatabase::GetLadderTeamID - %s\n", e->m_strError);
		*pnoutTID = 0;
		return false;
	}

	if ((rs.IsOpen() == false) || (rs.GetRecordCount() <= 0) || (rs.IsBOF() == true))
	{
		*pnoutTID = 0;
		return false;
	}

	char szFieldName[64] = "";

	strcpy_safe(szFieldName, "TID4");
	*pnoutTID = rs.Field(szFieldName).AsInt();

	_STATUS_DB_END(42);
	return true;
}

bool MSSQLDatabase::LadderTeamWinTheGame(const int nTeamTableIndex, const int nWinnerTID, const int nLoserTID, const bool bIsDrawGame,
	const int nWinnerPoint, const int nLoserPoint, const int nDrawPoint)
{
	_STATUS_DB_START
		if (!CheckOpen()) return false;

	int nDrawGame = 0;
	if (bIsDrawGame) nDrawGame = 1;

	try
	{
		CStringSQL strSQL;

		strSQL.Format(g_szDB_TEAM4_WIN_THE_GAME, nWinnerTID, nLoserTID, nDrawGame, nWinnerPoint, nLoserPoint, nDrawPoint);
		m_DB.ExecuteSQL(strSQL);
	}
	catch (CDBException* e)
	{
		Log("MSSQLDatabase::Team4WinTheGame - %s\n", e->m_strError);
		return false;
	}

	_STATUS_DB_END(43);
	return true;
}

bool MSSQLDatabase::GetLadderTeamMemberByCID(const int nCID, int* poutTeamID, char** ppoutCharArray, int maxlen, int nCount)
{
	_STATUS_DB_START
		if (!CheckOpen()) return false;

	CODBCRecordset rs(&m_DB);

	try
	{
		CStringSQL strSQL;
		strSQL.Format(g_szDB_GET_LADDER_TEAM_MEMBERLIST, nCID);
		rs.Open(strSQL, CRecordset::forwardOnly, CRecordset::readOnly);
	}
	catch (CDBException* e)
	{
		Log("MSSQLDatabase::GetLadderTeamMemberByCID - %s\n", e->m_strError);
		return false;
	}

	if ((rs.IsOpen() == false) || (rs.GetRecordCount() <= 0) || (rs.IsBOF() == true))
	{
		return false;
	}

	int nTeamID = rs.Field("TID4").AsInt();
	if (poutTeamID)
		*poutTeamID = nTeamID;

	int nIndex = 0;
	for (; !rs.IsEOF(); rs.MoveNext())
	{
		if (nIndex >= nCount) break;

		CStringSQL strName = rs.Field("Name").AsString();
		if (ppoutCharArray)
			strcpy_safe(ppoutCharArray[nIndex], maxlen, (const char*)strName);
		nIndex++;
	}

	_STATUS_DB_END(44);
	return true;
}

bool MSSQLDatabase::UpdateCharLevel(const int nCID, const int nNewLevel, const int nBP, const int nKillCount,
	const int nDeathCount, const int nPlayTime, bool bIsLevelUp)
{
	bool ret = UpdateCharLevel(nCID, nNewLevel);

	if ((ret == true) && (bIsLevelUp) && (nNewLevel >= 10))
	{
		InsertLevelUpLog(nCID, nNewLevel, nBP, nKillCount, nDeathCount, nPlayTime);
	}

	return ret;
}

bool MSSQLDatabase::GetClanInfo(const int nCLID, MDB_ClanInfo* poutClanInfo)
{
	_STATUS_DB_START

		if (!CheckOpen()) return false;
	CODBCRecordset rs(&m_DB);

	try
	{
		CStringSQL strSQL;
		strSQL.Format(g_szDB_GET_CLAN_INFO, nCLID);
		rs.Open(strSQL, CRecordset::forwardOnly, CRecordset::readOnly);
	}
	catch (CDBException* e)
	{
		Log("MSSQLDatabase::GetClanInfo - %s\n", e->m_strError);
		return false;
	}

	if ((rs.IsOpen() == false) || (rs.GetRecordCount() <= 0) || (rs.IsBOF() == true))
	{
		return false;
	}

	poutClanInfo->nCLID = rs.Field("CLID").AsInt();
	poutClanInfo->nLevel = rs.Field("Level").AsInt();
	strcpy_safe(poutClanInfo->szClanName, rs.Field("Name").AsString());
	strcpy_safe(poutClanInfo->szMasterName, rs.Field("ClanMaster").AsString());
	strcpy_safe(poutClanInfo->szEmblemUrl, rs.Field("EmblemUrl").AsString());
	poutClanInfo->nTotalPoint = rs.Field("TotalPoint").AsInt();
	poutClanInfo->nPoint = rs.Field("Point").AsInt();
	poutClanInfo->nWins = rs.Field("Wins").AsInt();
	poutClanInfo->nLosses = rs.Field("Losses").AsInt();
	poutClanInfo->nTotalMemberCount = rs.Field("MemberCount").AsInt();
	poutClanInfo->nRanking = rs.Field("Ranking").AsInt();
	poutClanInfo->nEmblemChecksum = rs.Field("EmblemChecksum").AsInt();

	_STATUS_DB_END(45);
	return true;
}

bool MSSQLDatabase::WinTheClanGame(const int nWinnerCLID, const int nLoserCLID, const bool bIsDrawGame,
	const int nWinnerPoint, const int nLoserPoint, const char* szWinnerClanName,
	const char* szLoserClanName, const int nRoundWins, const int nRoundLosses,
	const int nMapID, const int nGameType,
	const char* szWinnerMembers, const char* szLoserMembers)
{
	_STATUS_DB_START
		if (!CheckOpen()) return false;

	int nDrawGame = 0;
	if (bIsDrawGame) nDrawGame = 1;

	try
	{
		std::string temp[] = { FilterSQL(szWinnerClanName),
			FilterSQL(szLoserClanName),
			FilterSQL(szWinnerMembers),
			FilterSQL(szLoserMembers),
		};

		CStringSQL strSQL;

		strSQL.Format(g_szDB_WIN_THE_CLAN_GAME, nWinnerCLID, nLoserCLID, nDrawGame, nWinnerPoint, nLoserPoint,
			temp[0].c_str(), temp[1].c_str(), nRoundWins, nRoundLosses, nMapID, nGameType,
			temp[2].c_str(), temp[3].c_str());

		m_DB.ExecuteSQL(strSQL);
	}
	catch (CDBException* e)
	{
		Log("MSSQLDatabase::WinTheClanGame - %s\n", e->m_strError);
		return false;
	}

	_STATUS_DB_END(46);
	return true;
}

bool MSSQLDatabase::UpdateCharClanContPoint(const int nCID, const int nCLID, const int nAddedContPoint)
{
	_STATUS_DB_START
		if (!CheckOpen()) return false;

	try
	{
		CStringSQL strSQL;

		strSQL.Format(g_szDB_UPDATE_CHAR_CLAN_CONTPOINT, nCID, nCLID, nAddedContPoint);
		m_DB.ExecuteSQL(strSQL);
	}
	catch (CDBException* e)
	{
		Log("MSSQLDatabase::UpdateCharClanContPoint - %s\n", e->m_strError);
		return false;
	}

	_STATUS_DB_END(47);
	return true;
}

bool MSSQLDatabase::EventJjangUpdate(const int nAID, bool bJjang)
{
	_STATUS_DB_START
		if (!CheckOpen()) return false;

	try
	{
		CStringSQL strSQL;

		MMatchUserGradeID nGrade = bJjang ? MMUG_STAR : MMUG_FREE;
		strSQL.Format(g_szDB_EVENT_JJANG_UPDATE, nGrade, nAID);
		m_DB.ExecuteSQL(strSQL);
	}
	catch (CDBException* e)
	{
		Log("MSSQLDatabase::EventJjangUpdate - %s\n", e->m_strError);
		return false;
	}

	_STATUS_DB_END(48);
	return true;
}

bool MSSQLDatabase::DeleteExpiredAccountItem(const int nAIID)
{
	_STATUS_DB_START
		if (!CheckOpen()) return false;

	try
	{
		CStringSQL strSQL;

		strSQL.Format(g_szDB_DELETE_EXPIRED_ACCOUNT_ITEM, nAIID);
		m_DB.ExecuteSQL(strSQL);
	}
	catch (CDBException* e)
	{
		Log("MSSQLDatabase::DeleteExpiredAccountItem - %s\n", e->m_strError);
		return false;
	}

	_STATUS_DB_END(49);
	return true;
}

#define QUEST_ITEM_NUM_SIZE			sizeof( int )
#define SIZE_OF_SIMPLE_QUEST_ITEM	sizeof(SimpleQuestItem)

bool MSSQLDatabase::UpdateQuestItem(int nCID, MQuestItemMap& rfQuestIteMap, MQuestMonsterBible& rfQuestMonster)
{
	_STATUS_DB_START
		try
	{
		if (QUEST_DATA < MCRC32::SIZE + MAX_DB_QUEST_ITEM_SIZE + MAX_DB_MONSTERBIBLE_SIZE)
		{
			assert(0);
			Log("MSSQLDatabase::UpdateQuestItem - 아이템의 크기가 %d보다 큼.", QUEST_DATA);
			return false;
		}

		unsigned char			szData[QUEST_DATA];
		unsigned char			szMonData[MAX_DB_MONSTERBIBLE_SIZE];
		MCRC32::crc_t			Crc32;
		int						i;
		MQuestItemMap::iterator	it, end;

		memset(szData, 0, QUEST_DATA);
		memset(szMonData, 0, MAX_DB_MONSTERBIBLE_SIZE);

		end = rfQuestIteMap.end();
		for (i = 0, it = rfQuestIteMap.begin(); it != end; ++it)
		{
			MQuestItem* pQuestItem = it->second;

			if (GetQuestItemDescMgr().IsQItemID(it->first))
			{
				int nIndex = MCRC32::SIZE + pQuestItem->GetItemID() - MIN_QUEST_ITEM_ID;
				unsigned char nValue = 0;
				if (pQuestItem->GetCount() > 0)
				{
					nValue = pQuestItem->GetCount() + MIN_QUEST_DB_ITEM_COUNT;
				}
				else if (pQuestItem->IsKnown())
				{
					nValue = MIN_QUEST_DB_ITEM_COUNT;
				}
				else nValue = 0;

				szData[nIndex] =
					static_cast<unsigned char>(nValue);
			}
		}

		memcpy(szData + MCRC32::SIZE + MAX_DB_QUEST_ITEM_SIZE, &rfQuestMonster, MAX_DB_MONSTERBIBLE_SIZE);

		Crc32 = MCRC32::BuildCRC32(reinterpret_cast<BYTE*>(szData + MCRC32::SIZE), MAX_DB_QUEST_ITEM_SIZE + MAX_DB_MONSTERBIBLE_SIZE);
		if (0 == Crc32)
		{
			mlog("MSSQLDatabase::UpdateQuestItem - CRC32 is 0 CID:%d\n", nCID);
		}

		memcpy(szData, &Crc32, MCRC32::CRC::SIZE);

		CStringSQL strQuery;
		strQuery.Format("UPDATE Character SET QuestItemInfo = (?) WHERE CID = %d", nCID);

		CODBCRecordset rs(&m_DB);
		if (!rs.InsertBinary(strQuery, szData, MCRC32::CRC::SIZE + MAX_DB_QUEST_ITEM_SIZE + MAX_DB_MONSTERBIBLE_SIZE))
		{
			Log("MSSQLDatabase::UpdateQuestItem - Querry fail.");
			return false;
		}
	}
	catch (CDBException* e)
	{
		Log("MSSQLDatabase::InsertQuestItem - %s\n", e->m_strError);
		return false;
	}

	_STATUS_DB_END(51);

	return true;
}

bool MSSQLDatabase::GetCharQuestItemInfo(MMatchCharInfo* pCharInfo)
{
	_STATUS_DB_START

		try
	{
		if (!CheckOpen()) return false;

		if (0 == pCharInfo)
			return false;

		CODBCRecordset rs(&m_DB);

		CStringSQL strQuery;
		strQuery.Format(g_szDB_SELECT_QUEST_ITEM_INFO_BY_CID, pCharInfo->m_nCID);

		CDBBinary bn = rs.SelectBinary(strQuery);
		if (-1 == bn.GetCurIndex())
		{
			mlog("MSSQLDatabase::GetCharQuestItemInfo - 퀘스트 아이템 SELECT쿼리 실패. UserName:%s CID:%d\n",
				pCharInfo->m_szName,
				pCharInfo->m_nCID);
			return false;
		}

		unsigned char	szData[QUEST_DATA];
		MCRC32::crc_t	Crc32;

		bn.Begin();
		int nDataSize = bn.GetNextData(szData, QUEST_DATA);
		if (-1 == nDataSize)
		{
			Log("MMatchDMBgr::GetQuestItem - 데이터를 복사하는데 오류.");
			return false;
		}

		memcpy(&Crc32, szData, MCRC32::CRC::SIZE);

		if (0 == nDataSize)
		{
			pCharInfo->m_QuestItemList.Clear();
			memset(&pCharInfo->m_QMonsterBible, 0, sizeof(MQuestMonsterBible));
			pCharInfo->m_QuestItemList.SetDBAccess(true);

			_STATUS_DB_END(52);
			return true;
		}

		if (Crc32 != MCRC32::BuildCRC32(reinterpret_cast<BYTE*>(szData + MCRC32::CRC::SIZE), nDataSize - MCRC32::CRC::SIZE))
		{
			Log("MSSQLDatabase::GetQuestItem - crc check error.");
			return false;
		}

		pCharInfo->m_QuestItemList.Clear();

		for (int i = 0; i < MAX_DB_QUEST_ITEM_SIZE; ++i)
		{
			if (MIN_QUEST_DB_ITEM_COUNT > static_cast<int>(szData[MCRC32::CRC::SIZE + i]))
				continue;

			u32 nItemID = i + MIN_QUEST_ITEM_ID;
			int nQuestItemCount = static_cast<int>(szData[MCRC32::CRC::SIZE + i]);
			bool bKnownItem;

			if (nQuestItemCount > 0)
			{
				bKnownItem = true;
				nQuestItemCount--;
			}
			else
			{
				bKnownItem = false;
			}

			if (!pCharInfo->m_QuestItemList.CreateQuestItem(nItemID, nQuestItemCount, bKnownItem))
				mlog("MSSQLDatabase::GetCharQuestItemInfo - 새로운 퀘스트 아이템 생성 실패.\n");
		}

		memcpy(&pCharInfo->m_QMonsterBible, szData + MCRC32::SIZE + MAX_DB_QUEST_ITEM_SIZE, MAX_DB_MONSTERBIBLE_SIZE);

		pCharInfo->m_QuestItemList.SetDBAccess(true);
	}
	catch (CDBException* e)
	{
		Log("MSSQLDatabase::SelectQuestItem - %s\n", e->m_strError);
		return false;
	}

	_STATUS_DB_END(52);

	return true;
}

bool MSSQLDatabase::InsertQuestGameLog(const char* pszStageName,
	const int nScenarioID,
	const int nMasterCID, const int nPlayer1, const int nPlayer2, const int nPlayer3,
	const int nTotalRewardQItemCount,
	const int nElapsedPlayTime,
	int& outQGLID)
{
	_STATUS_DB_START
		if (!CheckOpen())
			return false;

	CStringSQL strQuery;
	CODBCRecordset rs(&m_DB);

	try
	{
		string strStageName = FilterSQL(string(pszStageName));

		strQuery.Format(g_szDB_INSERT_QUEST_GAME_LOG,
			&strStageName[0], nMasterCID, nPlayer1, nPlayer2, nPlayer3, nTotalRewardQItemCount, nScenarioID, nElapsedPlayTime);
		rs.Open(strQuery, CRecordset::forwardOnly, CRecordset::readOnly);
	}
	catch (CDBException* e)
	{
		mlog("MSSQLDatabase::InsertQuestGameLog - %s\n", e->m_strError);
		return false;
	}

	if ((rs.IsOpen() == false) || (rs.GetRecordCount() <= 0) || (rs.IsBOF() == true))
	{
		return false;
	}

	outQGLID = rs.Field("ORDERQGLID").AsLong();

	_STATUS_DB_END(53);
	return true;
}

bool MSSQLDatabase::InsertQUniqueGameLog(const int nQGLID, const int nCID, const int nQIID)
{
	_STATUS_DB_START
		if (!CheckOpen())
			return false;

	if ((0 == nCID) || (0 == nQIID))
		return false;

	CStringSQL strQuery;
	try
	{
		strQuery.Format(g_szDB_INSERT_QUINQUEITEMLOG, nQGLID, nCID, nQIID);
		m_DB.ExecuteSQL(strQuery);
	}
	catch (CDBException* e)
	{
		mlog("MSSQLDatabase::InsertQUniqueGameLog - %s\n", e->m_strError);
		return false;
	}
	_STATUS_DB_END(54);
	return true;
}

bool MSSQLDatabase::GetCID(const char* pszCharName, int& outCID)
{
	if (0 == pszCharName)
		return false;

	CStringSQL strQuery;
	strQuery.Format("SELECT CID FROM Character (UPDLOCK) WHERE Name LIKE '%s'",
		pszCharName);

	CODBCRecordset rs(&m_DB);
	try
	{
		rs.Open(strQuery, CRecordset::forwardOnly, CRecordset::readOnly);
	}
	catch (CDBException* e)
	{
		mlog("MSSQLDatabase::GetCID - %s\n", e->m_strError);
		return false;
	}

	outCID = rs.Field("CID").AsLong();

	return true;
}

bool MSSQLDatabase::GetCharName(const int nCID, string& outCharName)
{
	if (!CheckOpen())
		return false;

	CStringSQL strQuery;
	strQuery.Format("SELECT Name FROM Character (NOLOCK) WHERE CID = %u", nCID);

	CODBCRecordset rs(&m_DB);
	try
	{
		rs.Open(strQuery, CRecordset::forwardOnly, CRecordset::readOnly);
	}
	catch (CDBException* e)
	{
		mlog("MSSQLDatabase::GetCharName - %s\n", e->m_strError);
		return false;
	}

	outCharName = rs.Field("Name").AsString().GetBuffer();

	return true;
}

bool MSSQLDatabase::CheckPremiumIP(const char* szIP, bool& outbResult)
{
	_STATUS_DB_START
		if (!CheckOpen())
			return false;

	auto temp = FilterSQL(szIP);

	CStringSQL strSQL;
	strSQL.Format(g_szDB_CHECK_PREMIUM_IP, temp.c_str());

	CODBCRecordset rs(&m_DB);

	bool bException = false;
	try
	{
		rs.Open(strSQL, CRecordset::forwardOnly, CRecordset::readOnly);
	}
	catch (CDBException* e)
	{
		Log("MSSQLDatabase::CheckPremiumIP - %s\n", e->m_strError);
		return false;
	}

	if (rs.IsOpen() == false)
	{
		return false;
	}

	const bool IsGameRoom = rs.Field("IsGameRoom").AsBool();
	const int nEtc = rs.Field("etc").AsInt();

	if (IsGameRoom)
	{
		outbResult = IsGameRoom;
	}
	else
	{
		outbResult = false;
	}

	_STATUS_DB_END(57);
	return true;
}

bool MSSQLDatabase::InsertEvent(const uint32_t dwAID, const uint32_t dwCID, const std::string& strEventName)
{
	_STATUS_DB_START
		if (!CheckOpen())
			return false;

	auto temp = FilterSQL(strEventName);

	CStringSQL strSQL;
	strSQL.Format(g_szInsertEvent, dwAID, dwCID, temp.c_str());
	try
	{
		m_DB.ExecuteSQL(strSQL);
	}
	catch (CDBException* e)
	{
		Log("MSSQLDatabase::InsertEvent - %s\n", e->m_strError);
		return false;
	}
	_STATUS_DB_END(63);

	return true;
}

bool MSSQLDatabase::SetBlockAccount(const uint32_t dwAID, const uint32_t dwCID, const uint8_t btBlockType,
	const string& strComment, const string& strIP, const string& strEndHackBlockerDate)
{
#ifdef _BLOCK_HACKER
	_STATUS_DB_START
		if (!CheckOpen())
			return false;

	CStringSQL strSQL;
	strSQL.Format(g_szSetBlockAccount, dwAID, dwCID, btBlockType, strComment.c_str(), strIP.c_str(), strEndHackBlockerDate.c_str());
	try
	{
		m_DB.ExecuteSQL(strSQL);
	}
	catch (CDBException* e)
	{
		Log("MSSQLDatabase::InsertBlockLog - %s.\n", e->m_strError);
		return false;
	}

	_STATUS_DB_END(64);
#endif
	return true;
}

bool MSSQLDatabase::ResetAccountBlock(const uint32_t dwAID, const uint8_t btBlockType)
{
#ifdef _BLOCK_HACKER
	_STATUS_DB_START
		if (!CheckOpen())
			return false;

	CStringSQL strSQL;
	strSQL.Format(g_szResetAccountBlock, dwAID, btBlockType);
	try
	{
		m_DB.ExecuteSQL(strSQL);
	}
	catch (CDBException* e)
	{
		Log("MSSQLDatabase::ResetAccountBlock - %s.\n", e->m_strError);
		return false;
	}
	_STATUS_DB_END(65);
#endif
	return true;
}

bool MSSQLDatabase::InsertBlockLog(const uint32_t dwAID, const uint32_t dwCID, const BYTE btBlockType,
	const string& strComment, const string& strIP)
{
#ifdef _BLOCK_HACKER
	_STATUS_DB_START
		if (!CheckOpen())
			return false;

	CStringSQL strSQL;
	strSQL.Format(g_szInsertBlockLog, dwAID, dwCID, btBlockType, strComment.c_str(), strIP.c_str());
	try
	{
		m_DB.ExecuteSQL(strSQL);
	}
	catch (CDBException* e)
	{
		Log("MSSQLDatabase::InsertBlockLog - %s.\n", e->m_strError);
		return false;
	}
	_STATUS_DB_END(66);
#endif
	return true;
}

bool MSSQLDatabase::DeleteExpiredClan(const uint32_t dwCID, const uint32_t dwCLID,
	const string& strDeleteName, const uint32_t dwWaitHour)
{
#ifdef _DELETE_CLAN
	_STATUS_DB_START
		if (!CheckOpen())
			return false;

	CStringSQL strSQL;
	strSQL.Format(g_szDeleteExpiredClan, dwCLID, dwCID, strDeleteName.c_str(), dwWaitHour);
	try
	{
		m_DB.ExecuteSQL(strSQL);
	}
	catch (CDBException* e)
	{
		Log("MSSQLDatabase::DeleteExpiredClan - %s.\n", e->m_strError);
		return false;
	}
	_STATUS_DB_END(67);
#endif
	return true;
}

bool MSSQLDatabase::SetDeleteTime(const uint32_t dwMasterCID, const uint32_t dwCLID, const string& strDeleteDate)
{
#ifdef _DELETE_CLAN
	_STATUS_DB_START
		if (!CheckOpen())
			return false;

	if (strDeleteDate.empty())
		return false;

	CStringSQL strSQL;
	strSQL.Format(g_szSetClanDeleteDate, dwMasterCID, dwCLID, strDeleteDate.c_str());
	try
	{
		m_DB.ExecuteSQL(strSQL);
	}
	catch (CDBException* e)
	{
		Log("MSSQLDatabase::SetDeleteTime - %s.\n", e->m_strError);
		return false;
	}
	_STATUS_DB_END(68);
#endif

	return true;
}

bool MSSQLDatabase::AdminResetAllHackingBlock()
{
	_STATUS_DB_START
		if (!CheckOpen())
			return false;

	try
	{
		m_DB.ExecuteSQL(g_szAdminResetAllHackingBlock);
	}
	catch (CDBException* e)
	{
		Log("MSSQLDatabase::AdminResetAllHackingBlock - %s\n", e->m_strError);
		return false;
	}

	_STATUS_DB_END(69);

	return true;
}