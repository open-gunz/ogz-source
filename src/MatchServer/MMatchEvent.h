#ifndef _MMATCH_EVENT
#define _MMATCH_EVENT


#include <ctime>
#include <vector>
#include <string>
using std::vector;
using std::string;

#include "MUID.h"


class MMatchObject;


#define EVENT_XML_FILE_NAME "Event.xml"


#define EV_EVENT	"Event"
#define EV_EVENTID	"id"
#define EV_DESC		"Description"


#define EL_LOCALE			"Locale"
#define EL_COUNTRY			"country"
#define EL_EVENT			"Event"
#define EL_EVENT_LIST_ID	"id"
#define EL_EVENTID			"EventID"
#define EL_NAME				"name"
#define EL_EVENTTYPE		"event_type"
#define EL_ELAPSEDTIME		"elapsed_time"
#define EL_PERCENT			"percent"
#define EL_RATE				"rate"
#define EL_ANNOUNCE			"announce"
#define EL_SERVERTYPE		"ServerType"
#define EL_GAMETYPE			"GameType"
#define EL_STARTTIME		"StartTime"
#define EL_ENDTIME			"EndTime"
#define EL_XPBONUS_RATIO	"xp_bonus_ratio"
#define EL_BPBONUS_RATIO	"bp_bonus_ratio"
#define EL_PART_TIME		"PartTime"
#define		EL_ORDER			"order"
#define		EL_TYPE				"type"
#define		EL_YEAR				"year"
#define		EL_MONTH			"month"
#define		EL_DAY				"day"
#define		EL_HOUR				"hour"
#define		EL_START_HOUR		"start_hour"
#define		EL_END_HOUR			"end_hour"




enum EVENT_TYPE
{
	ET_BEGIN	= 0,		// : 게임을 시작할때.
	ET_ONGAME	= 1,		//  : 방을 만들어 게임을 시작을 해야 활성화되는 이벤트.
	ET_END		= 2,		// : 게임이 끝나후에.

	ET_CUSTOM_EVENT	= 100,	// : 직접 호출.

	ET_MAX,
};


enum EVENT_ID
{
	EID_PROBABILITY_PER_TIME = 1,
	EID_XP_BONUS,
	EID_BP_BONUS,

	EID_END,
};


struct EventPartTime
{
	u8 btStartHour;
	u8 btEndHour;
};


class MMatchEvent
{
public :
	virtual ~MMatchEvent();

	const u32 GetEventID() const		{ return m_dwEventID; }
	const u32 GetEventListID() const	{ return m_dwEventListID; }
	const string& GetName()				{ return m_strName; }
	// 정해진 이벤트 처리루틴외 등록을 위해서. 
	
	bool DoItNow( const u32 dwCurTime );

	void Set( const u32 dwEventListID, const u32 dwEventType, const u32 dwGameType, const u32 dwCheckElapsedTime, const u32 dwPercent, 
		const u32 dwRate, const tm& Start, const tm& End, const string& strName, const string& strAnnounce,
		const float fXPBonusRatio, const float fBPBonusRatio, const vector<EventPartTime>& EventPartTimeVec );

	void StartNewEvent() { m_vEventObj.clear(); }
	void Run();
	void CheckEventObj( MMatchObject* pObj, u64 dwCurTime );

	void Reset();

	void SetLastCheckTime( u64 dwCurTime );

	// 유저가 따로 초기화를 필요할때는 이것을 재정의 하면 된다.
	// MMatchEvent클래스를 생성시에 이 함수가 호출되어 같이 검사된다.
	virtual bool InitEvent() { return true; }
	// 정해진 루틴이 아닌 따로 처리해 줘야하는 이벤트는 이 함수를 제정의해서 사용함.
	// 처리시 필요한 유저타입데이터는 pContext로 넘겨서 처리.
	virtual bool CustomCheckEventObj( MMatchObject* pObj, void* pContext ) { return true; }

protected :
	MMatchEvent() { Reset(); }
	MMatchEvent( const u32 dwEventID ) : m_dwEventID ( dwEventID ), m_dwLastCheckTime( 0 ) {}
	
	bool CheckEventTime();
	bool CheckElapsedTimeIsOverflow(u64 dwCurTime );
	bool CheckEventPartTime();

protected : 
	virtual void OnCheckEventObj( MMatchObject* pObj, u64 dwCurTime ) {}
	virtual void OnRun() {}

protected :
	u32					m_dwEventListID;
	u32					m_dwEventID;
	u32					m_dwEventType;
	u32					m_dwGameType;
	u64						m_dwCheckElapsedTime;
	u64						m_dwLastCheckTime;
	u32					m_dwPercent;
	u32					m_dwRate;
	float					m_fXPBonusRatio;
	float					m_fBPBonusRatio;
	tm				m_Start;
	tm				m_End;
	std::string				m_strName;
	std::string				m_strAnnounce;
	std::vector< MUID >		m_vEventObj;
	std::vector< EventPartTime > m_EventPartTimeVec;
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// 

class MMatchEventDesc
{
public :
	MMatchEventDesc() {}
	~MMatchEventDesc() {}

	void Set( const u32 dwEventID, const string& strDesc );

	const u32 GetEventID() const			{ return m_dwEventID; }
	const string& GetDescription() const	{ return m_strDesc; }

private :
	u32	m_dwEventID;
	string	m_strDesc;
};


class MMatchEventDescManager : private std::map< u32, const MMatchEventDesc* >
{
public :
	MMatchEventDescManager() {}
	~MMatchEventDescManager()
	{
	}
	
	bool LoadEventXML( class MZFileSystem* pFileSystem, const string& strFileName );
	bool LoadEventXML( const std::string& strFileName );

	const MMatchEventDesc* Find( const u32 dwEventID );

	static MMatchEventDescManager& GetInstance()
	{
		static MMatchEventDescManager EventDescManager;
		return EventDescManager;
	}

private :
	void ParseEvent( MXmlElement& chrElement );
	bool Insert( const u32 dwEventID, MMatchEventDesc* pEventDesc );
};


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// 이 아래쪽으로 이벤트를 추가 한다. 추후에 이벤트가 많아지면 별도의 파일로 분리되어야 함.

class MMatchProbabiltyEventPerTime : public MMatchEvent
{
public :
	MMatchProbabiltyEventPerTime() : 
	   MMatchEvent( EID_PROBABILITY_PER_TIME ) {}
	virtual ~MMatchProbabiltyEventPerTime();
	
protected :
	void OnCheckEventObj( MMatchObject* pObj, const u32 dwCurTime );
	void OnRun();

private :
};


class MMatchXPBonusEvent : public MMatchEvent
{
public :
	MMatchXPBonusEvent() : 
	   MMatchEvent( EID_XP_BONUS ) {}
   virtual ~MMatchXPBonusEvent();

   bool CustomCheckEventObj( MMatchObject* pObj, void* pContext );
};


class MMatchBPBonusEvent : public MMatchEvent
{
public :
	MMatchBPBonusEvent() :
	   MMatchEvent( EID_BP_BONUS ) {}
   virtual ~MMatchBPBonusEvent();

   bool CustomCheckEventObj( MMatchObject* pObj, void* pContext );
};


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


bool CheckUsableEventTimeByStartTime( const tm& StartTime );
bool CheckUsableEventTimeByEndTime( const tm& EndTime );
bool CheckUsableEventPartTime( const EventPartTime& ept );

#endif