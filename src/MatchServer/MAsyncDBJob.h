#pragma once

#include "MAsyncProxy.h"
#include "MUID.h"
#include "MErrorTable.h"
#include "MQuestItem.h"
#include "MMatchGlobal.h"
#include "IDatabase.h"

class MCommand;
class MMatchCharInfo;

enum MASYNCJOB {
	MASYNCJOB_TEST=0,
	MASYNCJOB_GETACCOUNTCHARLIST,
	MASYNCJOB_GETACCOUNTCHARINFO,
	MASYNCJOB_GETCHARINFO,
	MASYNCJOB_UPDATCHARCLANCONTPOINT,
	MASYNCJOB_FRIENDLIST,
	MASYNCJOB_GETLOGININFO,
	MASYNCJOB_CREATECHAR,
	MASYNCJOB_DELETECHAR,	
	MASYNCJOB_WINTHECLANGAME,
	MASYNCJOB_UPDATECHARINFODATA,
	MASYNCJOB_CHARFINALIZE,
	MASYNCJOB_BRINGACCOUNTITEM,
	MASYNCJOB_INSERTCONNLOG,
	MASYNCJOB_INSERTGAMELOG,
	MASYNCJOB_CREATECLAN,
	MASYNCJOB_EXPELCLANMEMBER,
	MASYNCJOB_INSERTQUESTGAMELOG,
	MASYNCJOB_UPDATEQUESTITEMINFO,
	MASYNCJOB_UPDATEIPTOCOUNTRYLIST,
	MASYNCJOB_UPDATEBLOCKCOUNTRYCODELIST,
	MASYNCJOB_UPDATECUSTOMIPLIST,
	MASYNCJOB_PROBABILITYEVENTPERTIME,
	MASYNCJOB_INSERTBLOCKLOG,
	MASYNCJOB_RESETACCOUNTBLOCK,

	MASYNCJOB_MAX,
};

class MAsyncDBJob_Test : public MAsyncJob {
public:
	MAsyncDBJob_Test() : MAsyncJob(MASYNCJOB_TEST)	{}
	virtual ~MAsyncDBJob_Test()	{}
	virtual void Run(void* pContext);
};

class MAsyncDBJob_GetAccountCharList : public MAsyncJob {
protected:
	MUID			m_uid;

protected:	// Input Argument
	int				m_nAID;

protected:	// Output Result
	MCommand*		m_pResultCommand;
	int				m_nCharMaxLevel;		// newbie인지 체크하기 위함
public:
	MAsyncDBJob_GetAccountCharList(const MUID& uid, int nAID) 
		: MAsyncJob(MASYNCJOB_GETACCOUNTCHARLIST)
	{
		m_uid = uid;
		m_nAID = nAID;
		m_nCharMaxLevel = 0;
		m_pResultCommand = NULL;
	}
	virtual ~MAsyncDBJob_GetAccountCharList()	{}

	const MUID& GetUID()			{ return m_uid; }
	MCommand* GetResultCommand()	{ return m_pResultCommand; }
	int GetCharMaxLevel()			{ return m_nCharMaxLevel; }

	virtual void Run(void* pContext);
};

class MAsyncDBJob_GetCharInfo : public MAsyncJob {
protected:
	MUID				m_uid;

protected:	// Input Argument
	int					m_nAID;
	int					m_nCharIndex;

protected:	// Output Result
	MMatchCharInfo*			m_pCharInfo;
	MMatchClanDeleteState	m_DeleteState;

private :
	void SetDeleteState( const MMatchClanDeleteState DeleteState ) { m_DeleteState = DeleteState; }

public:
	MAsyncDBJob_GetCharInfo(const MUID& uid, int nAID, int nCharIndex)
		: MAsyncJob(MASYNCJOB_GETCHARINFO)
	{
		m_uid			= uid;
		m_nAID			= nAID;
		m_nCharIndex	= nCharIndex;
		m_DeleteState	= MMCDS_NORMAL;
	}
	virtual ~MAsyncDBJob_GetCharInfo()			{}

	const MUID&				GetUID()			{ return m_uid; }
	MMatchCharInfo*			GetCharInfo()		{ return m_pCharInfo; }
	MMatchClanDeleteState	GetDeleteState()	{ return m_DeleteState; }

	void SetCharInfo(MMatchCharInfo* pCharInfo)	{ m_pCharInfo = pCharInfo; }

	virtual void Run(void* pContext);
};

class MAsyncDBJob_UpdateCharClanContPoint : public MAsyncJob {
protected:
	int					m_nCID;
	int					m_nCLID;
	int					m_nAddedContPoint;
public:
	MAsyncDBJob_UpdateCharClanContPoint(int nCID, int nCLID, int nAddedContPoint)
		: MAsyncJob(MASYNCJOB_UPDATCHARCLANCONTPOINT)
	{
		m_nCID = nCID;
		m_nCLID = nCLID;
		m_nAddedContPoint = nAddedContPoint;
	}
	virtual ~MAsyncDBJob_UpdateCharClanContPoint()	{}

	virtual void Run(void* pContext);

};


class MAsyncDBJob_GetAccountCharInfo : public MAsyncJob {
protected:
	MUID			m_uid;

protected:	// Input Argument
	int				m_nAID;
	int				m_nCharNum;

protected:	// Output Result
	MCommand*		m_pResultCommand;

public:
	MAsyncDBJob_GetAccountCharInfo(const MUID& uid, int nAID, int nCharNum)
		: MAsyncJob(MASYNCJOB_GETACCOUNTCHARINFO)
	{
		m_uid = uid;
		m_nAID = nAID;
		m_nCharNum = nCharNum;
		m_pResultCommand = NULL;
	}
	virtual ~MAsyncDBJob_GetAccountCharInfo()	{}

	const MUID& GetUID()			{ return m_uid; }
	MCommand* GetResultCommand()	{ return m_pResultCommand; }

	virtual void Run(void* pContext);
};

class MAsyncDBJob_CreateChar : public MAsyncJob {
protected:
	MUID		m_uid;

protected: // Input Argument
	int			m_nAID;
	char		m_szCharName[32];
	int			m_nCharNum;
	int			m_nSex;
	int			m_nHair;
	int			m_nFace;
	int			m_nCostume;

protected:	// Output Result
	int			m_nResult;
	MCommand*	m_pResultCommand;

public:
	MAsyncDBJob_CreateChar(const MUID& uid, int nAID, const char* szCharName, int nCharNum, int nSex, int nHair, int nFace, int nCostume)
	: MAsyncJob(MASYNCJOB_CREATECHAR)
	{
		m_uid = uid;
		m_nAID = nAID;
		strcpy_safe(m_szCharName, szCharName);
		m_nCharNum = nCharNum;
		m_nSex = nSex;
		m_nHair = nHair;
		m_nFace = nFace;
		m_nCostume = nCostume;
		m_pResultCommand = NULL;
		m_nResult = MERR_UNKNOWN;
	}
	virtual ~MAsyncDBJob_CreateChar()	{}

	const MUID& GetUID()		{ return m_uid; }
	MCommand* GetResultCommand()	{ return m_pResultCommand; }

	virtual void Run(void* pContext);
};

class MAsyncDBJob_DeleteChar : public MAsyncJob {
protected:
	MUID		m_uid;

protected: // Input Argument
	int			m_nAID;
	int			m_nCharNum;
	char		m_szCharName[32];

protected:	// Output Result
	int			m_nDeleteResult;

public:
	MAsyncDBJob_DeleteChar(const MUID& uid, int nAID, int nCharNum, const char* szCharName)
	: MAsyncJob(MASYNCJOB_DELETECHAR)
	{
		m_uid = uid;
		m_nAID = nAID;
		m_nCharNum = nCharNum;
		strcpy_safe(m_szCharName, szCharName);
		m_nDeleteResult = MERR_UNKNOWN;
	}
	virtual ~MAsyncDBJob_DeleteChar()	{}

	const MUID& GetUID()		{ return m_uid; }
	int GetDeleteResult()		{ return m_nDeleteResult; }

	virtual void Run(void* pContext);

};


////////////////////////////////////////////////////////////////////////////////////////////////////
class MAsyncDBJob_InsertGameLog : public MAsyncJob {
protected: // Input Argument
	char			m_szGameName[256];
	char			m_szMap[256];
	char			m_szGameType[256];
	int				m_nRound;
	unsigned int	m_nMasterCID;
	int				m_nPlayerCount;
	char			m_szPlayers[512];
protected:	// Output Result

public:
	MAsyncDBJob_InsertGameLog()	: MAsyncJob(MASYNCJOB_INSERTGAMELOG) {}
	virtual ~MAsyncDBJob_InsertGameLog()	{}
	bool Input(const char* szGameName, 
			   const char* szMap, 
			   const char* szGameType,
               const int nRound, 
			   const unsigned int nMasterCID,
               const int nPlayerCount, 
			   const char* szPlayers);
	virtual void Run(void* pContext);
};

////////////////////////////////////////////////////////////////////////////////////////////////////
class MAsyncDBJob_CreateClan : public MAsyncJob {
protected:
	MUID		m_uidMaster;
	MUID		m_uidMember1;
	MUID		m_uidMember2;
	MUID		m_uidMember3;
	MUID		m_uidMember4;
protected: // Input Argument
	char		m_szClanName[256];
	int			m_nMasterCID;
	int			m_nMember1CID;
	int			m_nMember2CID;
	int			m_nMember3CID;
	int			m_nMember4CID;
protected:	// Output Result
	bool		m_bDBResult;
	int			m_nNewCLID;
public:
	MAsyncDBJob_CreateClan()	: MAsyncJob(MASYNCJOB_CREATECLAN) ,
											  m_bDBResult(false), 
											  m_nNewCLID(0)
	{
	
	}
	virtual ~MAsyncDBJob_CreateClan()	{}
	bool Input(const char* szClanName, 
			   const int nMasterCID, 
			   const int nMember1CID, 
			   const int nMember2CID,
               const int nMember3CID, 
			   const int nMember4CID,
			   const MUID& uidMaster,
			   const MUID& uidMember1,
			   const MUID& uidMember2,
			   const MUID& uidMember3,
			   const MUID& uidMember4);
	virtual void Run(void* pContext);
	bool GetDBResult() { return m_bDBResult; }
	int GetNewCLID() { return m_nNewCLID; }
	const MUID& GetMasterUID() { return m_uidMaster; }
	const MUID& GetMember1UID() { return m_uidMember1; }
	const MUID& GetMember2UID() { return m_uidMember2; }
	const MUID& GetMember3UID() { return m_uidMember3; }
	const MUID& GetMember4UID() { return m_uidMember4; }
	const char* GetClanName() { return m_szClanName; }
};
////////////////////////////////////////////////////////////////////////////////////////////////////
class MAsyncDBJob_ExpelClanMember : public MAsyncJob {
protected:
	MUID		m_uidAdmin;
protected: // Input Argument
	int			m_nCLID;
	int			m_nClanGrade;
	char		m_szTarMember[256]; 

protected:	// Output Result
	ExpelResult	DBResult;
public:
	MAsyncDBJob_ExpelClanMember()	: MAsyncJob(MASYNCJOB_EXPELCLANMEMBER), DBResult(ExpelResult::OK)
	{
	
	}
	virtual ~MAsyncDBJob_ExpelClanMember()	{}
	bool Input(const MUID& uidAdmin, int nCLID, int nClanGrade, const char* szTarMember);
	virtual void Run(void* pContext);

	auto GetDBResult() { return DBResult; }
	const MUID& GetAdminUID() { return m_uidAdmin; }
	const char* GetTarMember() { return m_szTarMember; }
};

////////////////////////////////////////////////////////////////////////////////////////////////////

class MMatchQuestGameLogInfoManager;
class MQuestPlayerLogInfo;

class MAsyncDBJob_InsertQuestGameLog : public MAsyncJob 
{
public :
	MAsyncDBJob_InsertQuestGameLog() : MAsyncJob(MASYNCJOB_INSERTQUESTGAMELOG), m_nMasterCID( 0 ), m_nScenarioID( 0 )
	{
	}

	~MAsyncDBJob_InsertQuestGameLog();
	

	bool Input( const char* pszStageName, 
				const int nScenarioID,
				const int nMasterCID, 
				MMatchQuestGameLogInfoManager* pQGameLogInfoMgr,
				const int nTotalRewardQItemCount,
				const int nElapsedPlayerTime );

	virtual void Run( void* pContext );

private :
	char							m_szStageName[ 64 ];
	int								m_nMasterCID;
	char							m_szMapName[ 32 ];
	int								m_nElapsedPlayTime;
	int								m_nScenarioID;
	vector< MQuestPlayerLogInfo* >	m_Player;
	int								m_PlayersCID[ 3 ];
	int								m_nTotalRewardQItemCount;
};

class MAsyncDBJob_UpdateQuestItemInfo : public MAsyncJob
{
public :
	MAsyncDBJob_UpdateQuestItemInfo () : MAsyncJob( MASYNCJOB_UPDATEQUESTITEMINFO )
	{
	}

	virtual ~MAsyncDBJob_UpdateQuestItemInfo();
	
	bool Input( const int nCID, 
				MQuestItemMap& QuestItemList, 
				MQuestMonsterBible& QuestMonster );

	virtual void Run( void* pContext );

private :
	int					m_nCID;
	MQuestItemMap		m_QuestItemList;
	MQuestMonsterBible	m_QuestMonster;
};



class MAsyncDBJob_SetBlockAccount : public MAsyncJob
{
public :
	MAsyncDBJob_SetBlockAccount() : MAsyncJob( MASYNCJOB_INSERTBLOCKLOG )
	{
	}

	virtual ~MAsyncDBJob_SetBlockAccount()
	{
	}

	bool Input( const u32 dwAID, 
				const u32 dwCID, 
				const u8 btBlockType, 
				const u8 btBlockLevel,
				const string& strComment, 
				const string& strIP,
				const string& strEndDate );

	virtual void Run( void* pContext );

private :
	u32	m_dwAID;
	u32	m_dwCID;
	u8	m_btBlockType;
	u8	m_btBlockLevel;
	string	m_strComment;
	string	m_strIP;
	string	m_strEndDate;
};


class MAsyncDBJob_ResetAccountBlock : public MAsyncJob
{
public :
	MAsyncDBJob_ResetAccountBlock() : MAsyncJob( MASYNCJOB_RESETACCOUNTBLOCK )
	{
	}

	~MAsyncDBJob_ResetAccountBlock() 
	{
	}

	bool Input( const u32 dwAID, const u8 btBlockType = 0 );

	virtual void Run( void* pContext );

private :
	u32	m_dwAID;
	u8	m_btBlockType;
};
	