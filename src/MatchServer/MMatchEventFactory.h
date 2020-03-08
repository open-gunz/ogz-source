#ifndef _MMATCH_EVENT_FACTORY
#define _MMATCH_EVENT_FACTORY


#include "MBaseGameType.h"
#include "MMatchEventManager.h"


#include <vector>
#include <map>
#include <string>

using std::vector;
using std::map;
using std::string;


#define EVENT_LIST_XML_FILE_NAME "EventList.xml"


struct EventServerType
{
	u32				dwOrder;
	MMatchServerMode	ServerType;
};


struct EventGameType
{
	u32			dwOrder;
	MMATCH_GAMETYPE GameType;
};


struct EventData
{
	EventData()
	{
		dwEventListID	= 0;
		dwEventID		= 0;
		dwElapsedTime	= 0;
		dwPercent		= 0;
		dwRate			= 0;
		fXPBonusRatio	= 0.0f;
		fBPBonusRatio	= 0.0f;
	}

	u32					dwEventListID;
	u32					dwEventID;
	string					strName;
	string					strAnnounce;
	EVENT_TYPE				EventType;
	MMATCH_GAMETYPE			GameType;
	u32					dwElapsedTime;
	u32					dwPercent;
	u32					dwRate;
	MMatchServerMode		ServerType;
	tm				Start;
	tm				End;
	float					fXPBonusRatio;
	float					fBPBonusRatio;
	vector< EventPartTime > EventPartTimeVec;
};


typedef vector< EventData >			EventDataVec;
typedef map< u32, EventDataVec >	GameTypeEventMap;


class MMatchEventFactory
{
public :
	MMatchEventFactory();
	~MMatchEventFactory();

	bool InsertEventData( const EventData& EvnData );

	bool GetEventList( const MMATCH_GAMETYPE GameType, EventPtrVec& EvnPtrVec );

private :
	bool MakeEventList( const EventDataVec& EvnDataVec, EventPtrVec& epc );
	MMatchEvent* CreateEvent( const u32 dwEventID );

	void CreateFailMLog( MMatchEvent* pEvent, const u32 dwEventID );

private :
	EventDataVec		m_vAllGameTypeEventData;
	GameTypeEventMap	m_mGameTypeEvent;
};


class MMatchEventFactoryManager
{
public :
	MMatchEventFactoryManager();
	~MMatchEventFactoryManager();

	bool LoadEventListXML( MZFileSystem* pFileSystem, const string& strFileName );
	bool LoadEventListXML( const string& strFileName );

	bool		GetEventList( const MMATCH_GAMETYPE GameType, const EVENT_TYPE EventType, EventPtrVec& EvnPtrVec );
	const u16	GetLoadEventSize() const { return m_LoadEventSize; }

	void SetUsableState( const bool bIsUsable ) { m_bIsUsable = bIsUsable; }

	static MMatchEventFactoryManager& GetInstance()
	{
		static MMatchEventFactoryManager EventFactoryManager;
		return EventFactoryManager;
	}

private :
	MMatchEventFactory& GetOnBeginEventFactory()	{ return m_OnBeginEventFactory; }
	MMatchEventFactory& GetOnGameEventFactory()		{ return m_OnGameEventFactory; }
	MMatchEventFactory& GetOnEndEventFactory()		{ return m_OnEndEventFactory; }
	MMatchEventFactory& GetCustomEventFactory()		{ return m_CustomEventFactory; }

	bool InsertEvent( const EventData& EvnData );

	void ParseLocale( MXmlElement& chrElement );
	void ParseEvent( MXmlElement& chrElement );
	void ParseServerType( MXmlElement& chrElement, vector< EventServerType >& vServerType );
	void ParseGameType( MXmlElement& chrElement, vector< EventGameType >& vGameType );
	void ParseStartEndTime( MXmlElement& chrElement, tm& stTime );
	void ParseEventPartTime( MXmlElement& chrElement, vector<EventPartTime>& EventPartTimeVec );
	
private :
	MMatchEventFactory m_OnBeginEventFactory;	// 방을 만들어 게임을 시작하는 시점의 이벤트.
	MMatchEventFactory m_OnGameEventFactory;	// 방을 만들어 게임을 시작한 후의 게임진행중 이벤트.
	MMatchEventFactory m_OnEndEventFactory;		// 게임이 끝나는 시점의 이벤트.
	MMatchEventFactory m_CustomEventFactory;	// 직접 호출하는 이벤트.

	bool m_bIsUsable;

	static u16 m_LoadEventSize;
};

#endif