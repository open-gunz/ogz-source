#include "stdafx.h"
#include "MXml.h"
#include "MMatchLocale.h"
#include "MMatchConfig.h"
#include "MMatchEventFactory.h"
#include "MZFileSystem.h"
#include "MBaseStringResManager.h"


u16 MMatchEventFactoryManager::m_LoadEventSize = 0;


MMatchEventFactory::MMatchEventFactory()
{
}


MMatchEventFactory::~MMatchEventFactory()
{
}


bool MMatchEventFactory::InsertEventData( const EventData& EvnData )
{
	if( ( MMATCH_GAMETYPE_ALL != EvnData.GameType) &&
		((0 > EvnData.GameType) && (8 < EvnData.GameType)) )
		return false;

	if( MMATCH_GAMETYPE_ALL == EvnData.GameType )
		m_vAllGameTypeEventData.push_back( EvnData );
	else
	{
		GameTypeEventMap::iterator itGameType = 
			m_mGameTypeEvent.find( EvnData.GameType );
		if( m_mGameTypeEvent.end() == itGameType )
		{
			EventDataVec edv;
			edv.push_back( EvnData );
			m_mGameTypeEvent.insert( GameTypeEventMap::value_type(EvnData.GameType, edv) );
		}
		else
			itGameType->second.push_back( EvnData );

		return true;
	}
	
	return false;
}


bool MMatchEventFactory::GetEventList( const MMATCH_GAMETYPE GameType, EventPtrVec& EvnPtrVec )
{
	if( (MMATCH_GAMETYPE_ALL != GameType) &&
		((0 > GameType) && (MMATCH_GAMETYPE_MAX < GameType)) )
		return false;

	if( !m_vAllGameTypeEventData.empty() )
	{
		if( !MakeEventList(m_vAllGameTypeEventData, EvnPtrVec) )
			return false;
	}

	if( !m_mGameTypeEvent.empty() && MMATCH_GAMETYPE_ALL != GameType )
	{
		GameTypeEventMap::iterator itGameType = 
			m_mGameTypeEvent.find( GameType );
		if( m_mGameTypeEvent.end() != itGameType )
			return MakeEventList( itGameType->second, EvnPtrVec );
	}

	return true;
}


bool MMatchEventFactory::MakeEventList( const EventDataVec& EvnDataVec, EventPtrVec& EvnPtrVec )
{
	bool bIsComplete = true;
	MMatchEvent* pEvent;
	EventDataVec::const_iterator it, end;
	it = EvnDataVec.begin();
	for( end = EvnDataVec.end(); it != end; ++it )
	{
		if( !CheckUsableEventTimeByEndTime(it->End) )
			continue;

		pEvent = CreateEvent( it->dwEventID );
		if( 0 != pEvent )
		{
			if( pEvent->InitEvent() )
			{
				pEvent->Set( it->dwEventListID, it->EventType, it->GameType, 
					it->dwElapsedTime, it->dwPercent, it->dwRate, 
					it->Start, it->End, it->strName,
					it->strAnnounce,
					it->fXPBonusRatio,
					it->fBPBonusRatio,
					it->EventPartTimeVec );
				EvnPtrVec.push_back( pEvent );
				pEvent = 0;
			}
			else
			{
				mlog( "MMatchEventFactory::MakeEventList - Init실패. ID:%u Name:%s.\n",
					pEvent->GetEventID(), pEvent->GetName().c_str() );
				delete pEvent;
				pEvent = 0;
				bIsComplete = false;
			}
		}
		else
			bIsComplete = false;
	}

	return bIsComplete;
}


MMatchEvent* MMatchEventFactory::CreateEvent( const u32 dwEventID )
{
	switch( dwEventID )
	{
	case EID_PROBABILITY_PER_TIME :
		{
			MMatchProbabiltyEventPerTime* pEvent = new MMatchProbabiltyEventPerTime;
			if( 0 != pEvent )
			{
				if( dwEventID == pEvent->GetEventID() )
					return pEvent;
				else
				{
					CreateFailMLog( pEvent, dwEventID );
					delete pEvent;
					pEvent = 0;
					return 0;
				}
			}
			else 
				return 0;
		}
		break;

	case EID_XP_BONUS :
		{
			MMatchXPBonusEvent* pEvent = new MMatchXPBonusEvent;
			if( 0 != pEvent )
			{
				if( dwEventID == pEvent->GetEventID() )
					return pEvent;
				else
				{
					CreateFailMLog( pEvent, dwEventID );
					delete pEvent;
					pEvent = 0;
					return 0;
				}
			}
			else
				return 0;
		}
		break;

	case EID_BP_BONUS :
		{
			MMatchBPBonusEvent* pEvent = new MMatchBPBonusEvent;
			if( 0 != pEvent )
			{
				if( dwEventID == pEvent->GetEventID() )
					return pEvent;
				else
				{
					CreateFailMLog( pEvent, dwEventID );
					delete pEvent;
					pEvent = 0;
					return 0;
				}
			}
			else
				return 0;
		}
		break;

	default :
		{
			ASSERT( 0 );
		}
		break;
	}

	return 0;
}


void MMatchEventFactory::CreateFailMLog( MMatchEvent* pEvent, const u32 dwEventID )
{
	mlog( "Fail to create event : created event:%u, event:%u\n",
		pEvent->GetEventID(), dwEventID );
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

MMatchEventFactoryManager::MMatchEventFactoryManager()
{
	m_bIsUsable = false;
}


MMatchEventFactoryManager::~MMatchEventFactoryManager()
{
}


bool MMatchEventFactoryManager::LoadEventListXML( MZFileSystem* pFileSystem, const string& strFileName )
{
	MXmlDocument	xmlIniData;
	xmlIniData.Create();

	//	<-----------------
	char *buffer;
	MZFile mzf;

	if(pFileSystem) 
	{
		if(!mzf.Open(strFileName.c_str(),pFileSystem)) 
		{
			if(!mzf.Open(strFileName.c_str())) 
			{
				xmlIniData.Destroy();
				return false;
			}
		}
	} 
	else 
	{
		if(!mzf.Open(strFileName.c_str()))
		{
			xmlIniData.Destroy();
			return false;
		}
	}

	buffer = new char[mzf.GetLength()+1];
	buffer[mzf.GetLength()] = 0;
	mzf.Read(buffer,mzf.GetLength());

	if(!xmlIniData.LoadFromMemory(buffer))
	{
		xmlIniData.Destroy();
		return false;
	}
	delete[] buffer;
	mzf.Close();
	//	<------------------

	MXmlElement rootElement, chrElement, attrElement;
	char szTagName[256];

	rootElement = xmlIniData.GetDocumentElement();
	int iCount = rootElement.GetChildNodeCount();

	for (int i = 0; i < iCount; i++)
	{
		chrElement = rootElement.GetChildNode(i);
		chrElement.GetTagName(szTagName);
		if (szTagName[0] == '#') continue;

		if (!_stricmp(EL_LOCALE, szTagName))
		{
			ParseLocale( chrElement );
		}
	}

	xmlIniData.Destroy();
	return true;
}


bool MMatchEventFactoryManager::LoadEventListXML( const string& strFileName )
{
	MXmlDocument	xmlIniData;

	xmlIniData.Create();

	if (!xmlIniData.LoadFromFile(strFileName.c_str()))
	{
		xmlIniData.Destroy();
		return false;
	}

	MXmlElement rootElement, chrElement, attrElement;
	char szTagName[256];

	rootElement = xmlIniData.GetDocumentElement();

	int iCount = rootElement.GetChildNodeCount();

	m_LoadEventSize = 0;

	for (int i = 0; i < iCount; i++)
	{
		chrElement = rootElement.GetChildNode(i);
		chrElement.GetTagName(szTagName);
		if (szTagName[0] == '#') continue;

		if (!_stricmp(EL_LOCALE, szTagName))
		{
			ParseLocale( chrElement );
		}
	}

	xmlIniData.Destroy();
	return true;
}


void MMatchEventFactoryManager::ParseLocale( MXmlElement& chrElement )
{
	EventData ed;
	char szAttrName[ 128 ];
	char szAttrValue[ 256 ];
	const int nAttrCnt = chrElement.GetAttributeCount();
	for( int i = 0; i < nAttrCnt; ++i )
	{
		chrElement.GetAttribute( i, szAttrName, szAttrValue );
		
		if( 0 == _stricmp(EL_COUNTRY, szAttrName) )
		{
			/*  // MBaseLocale.h // 
			enum MCountry
			{
				MC_KOREA			= 82,		// 한국
				MC_US				= 1,		// 미국(인터네셔날)
				MC_JAPAN			= 81,		// 일본
			};
			*/

			string strCountry;
			switch( MGetLocale()->GetCountry() )
			{
			case MC_KOREA :
				{
					strCountry = "kor";
				}
				break;

			case MC_US :
				{
					strCountry = "us";
				}
				break;

			case MC_JAPAN :
				{
					strCountry = "jpn";
				}
				break;

			case MC_BRAZIL :
				{
					strCountry = "brz";
				}
				break;

			case MC_INDIA :
				{
					strCountry = "ind";
				}
				break;

			default :
				{
					ASSERT( 0 );
				}
				break;
			}

			// 현제 서버랑 같은 국가 타입만 파싱함. 
#ifdef _DEBUG
			if( true )
#else
            if( 0 == _stricmp(strCountry.c_str(), szAttrValue) )
#endif
			{
				MXmlElement chrNode;
				char szTag[ 128 ];
				const int nChrCnt = chrElement.GetChildNodeCount();
				for( int j = 0; j < nChrCnt; ++j )
				{
					chrNode = chrElement.GetChildNode( j );
					chrNode.GetTagName( szTag );

					if( 0 == _stricmp(EL_EVENT, szTag) )
					{
						ParseEvent( chrNode );
					}
				}
			}
		}
	}
}


void MMatchEventFactoryManager::ParseEvent( MXmlElement& chrElement )
{
	char szAttrName[ 128 ];
	char szAttrValue[ 256 ];
	
	u32						dwEventListID = 0;
	u32						dwEventID = 0;
	string						strEventName;
	string						strAnnounce;
	EVENT_TYPE					EventType;
	u32						dwElapsedTime = 0;
	u32						dwPercent = 0;
	u32						dwRate = 0;
	vector< EventServerType >	vServerType;
	vector< EventGameType >		vGameType;
	tm					Start, End;
	float						fXPBonusRatio = 0.0f;
	float						fBPBonusRatio = 0.0f;
	vector< EventPartTime >		EventPartTimeVec;

	memset( &Start, 0, sizeof(tm) );
	memset( &End, 0, sizeof(tm) );

	const int nAttrCnt = chrElement.GetAttributeCount();
	for( int i = 0; i < nAttrCnt; ++i )
	{
		chrElement.GetAttribute( i, szAttrName, szAttrValue );

		if( 0 == _stricmp(EL_EVENT_LIST_ID, szAttrName) )
		{
			dwEventListID = static_cast< u32 >( atoi(szAttrValue) );
			ASSERT( 0 < dwEventListID );
			continue;
		}

		if( 0 == _stricmp(EL_EVENTID, szAttrName) )
		{
			dwEventID = static_cast< u32 >( atol(szAttrValue) );
			if( NULL == MMatchEventDescManager::GetInstance().Find(dwEventID) )
			{
				// TODO: Fix
				//ASSERT( 0 && "Event.xml에 없는 Event ID입니다." );
				mlog( "MMatchEventFactoryManager::ParseEvent - Event.xml에 없는 Event ID(%u)입니다.\n",
					dwEventID );
				return;
			}
			continue;
		}

		if( 0 == _stricmp(EL_NAME, szAttrName) )
		{
			strEventName = MGetStringResManager()->GetString( string(szAttrValue) );
			continue;
		}

		if( 0 == _stricmp(EL_EVENTTYPE, szAttrName) )
		{
			EventType = static_cast< EVENT_TYPE >( atoi(szAttrValue) );
			continue;
		}

		if( 0 == _stricmp(EL_ELAPSEDTIME, szAttrName) )
		{
			dwElapsedTime = static_cast< u32 >( atoi(szAttrValue) );
			continue;
		}

		if( 0 == _stricmp(EL_PERCENT, szAttrName) )
		{
			dwPercent = static_cast< u32 >( atol(szAttrValue) );
			continue;
		}

		if( 0 == _stricmp(EL_RATE, szAttrName) )
		{
			dwRate = static_cast< u32 >( atol(szAttrValue) );
			continue;
		}

		if( 0 == _stricmp(EL_ANNOUNCE, szAttrName) )
		{
			strAnnounce = MGetStringResManager()->GetString( string(szAttrValue) );
			continue;
		}

		if( 0 == _stricmp(EL_XPBONUS_RATIO, szAttrName) )
		{
			fXPBonusRatio = static_cast<float>( atoi(szAttrValue) ) / 100.0f;
			continue;
		}

		if( 0 == _stricmp(EL_BPBONUS_RATIO, szAttrName) )
		{
			fBPBonusRatio = static_cast<float>( atoi(szAttrValue) ) / 100.0f;
			continue;
		}
	}

	MXmlElement chrNode;
	char szTag[ 128 ];
	const int nChrNodeCnt = chrElement.GetChildNodeCount();

	EventPartTimeVec.clear();
	
	for( int j = 0; j < nChrNodeCnt; ++j )
	{
		chrNode = chrElement.GetChildNode( j );
		chrNode.GetTagName( szTag );

		if (szTag[0] == '#') continue;

		if( 0 == _stricmp(EL_SERVERTYPE, szTag) )
		{
			ParseServerType( chrNode, vServerType );
			continue;
		}

		if( 0 == _stricmp(EL_GAMETYPE, szTag) )
		{
			ParseGameType( chrNode, vGameType );
			continue;
		}

		if( 0 == _stricmp(EL_STARTTIME, szTag) )
		{
			ParseStartEndTime( chrNode, Start );
			continue;
		}

		if( 0 == _stricmp(EL_ENDTIME, szTag) )
		{
			ParseStartEndTime( chrNode, End );
			continue;
		}

		if( 0 == _stricmp(EL_PART_TIME, szTag) )
		{
			ParseEventPartTime(chrNode, EventPartTimeVec );
			continue;
		}
	}

	// check start end time.
	if( !CheckUsableEventTimeByEndTime(End) )
	{
#ifdef _DEBUG
		mlog( "Time out Event(%u:%u.%u.%u.%u~%u.%u.%u.%u)\n", 
			dwEventID,
			Start.tm_year, Start.tm_mon, Start.tm_yday, Start.tm_hour,
			End.tm_year, End.tm_mon, End.tm_yday, End.tm_hour);
#endif
		return;
	}
	
	// check server type.
	vector< EventServerType >::iterator itSvrTyp, endSvrTyp;
	bool bUseEvent = false;
	endSvrTyp = vServerType.end();
	for( itSvrTyp = vServerType.begin(); itSvrTyp != endSvrTyp; ++itSvrTyp )
	{
		// 모든 서버에 적용.
		if( MSM_ALL == itSvrTyp->ServerType )
		{
			bUseEvent = true;
			continue;
		}

		// 현제 서버 타입에만 적용.
		if( MGetServerConfig()->GetServerMode() == itSvrTyp->ServerType )
		{
			bUseEvent = true;
			continue;
		}
	}

	ASSERT( (0 < Start.tm_year) && (0 < Start.tm_mon) && (0 < Start.tm_yday) && (0 <= Start.tm_hour) &&
			(0 < End.tm_year) && (0 < End.tm_mon) && (0 < End.tm_yday) && (0 <= End.tm_hour) );

	// check game type.
	if( bUseEvent )
	{
		EventData ed;
		vector< EventGameType >::iterator itGmTyp, endGmTyp;
		endGmTyp = vGameType.end();
		for( itGmTyp = vGameType.begin(); itGmTyp != endGmTyp; ++itGmTyp )
		{
			// insert event.

			ed.dwEventListID	= dwEventListID;
			ed.dwEventID		= dwEventID;
			ed.EventType		= EventType;
			ed.GameType			= itGmTyp->GameType;
			ed.ServerType		= MGetServerConfig()->GetServerMode();
			ed.dwElapsedTime	= dwElapsedTime;
			ed.dwPercent		= dwPercent;
			ed.dwRate			= dwRate;
			ed.Start			= Start;
			ed.End				= End;
			ed.strName			= strEventName;
			ed.strAnnounce		= strAnnounce;
			ed.fXPBonusRatio	= fXPBonusRatio;
			ed.fBPBonusRatio	= fBPBonusRatio;

			ed.EventPartTimeVec.swap( EventPartTimeVec );

			InsertEvent( ed );

			++m_LoadEventSize;
		}		
	}
}


void MMatchEventFactoryManager::ParseServerType( MXmlElement& chrElement, vector< EventServerType >& vServerType )
{
	EventServerType ServerType;
	char szAttrName[ 128 ];
	char szAttrValue[ 256 ];
	const int nAttrCnt = chrElement.GetAttributeCount();
	for( int i = 0; i < nAttrCnt; ++i )
	{
		chrElement.GetAttribute( i, szAttrName, szAttrValue );

		if( 0 == _stricmp(EL_ORDER, szAttrName) )
		{
			ServerType.dwOrder = static_cast< u32 >( atoi(szAttrValue) );
			continue;
		}

		if( 0 == _stricmp(EL_TYPE, szAttrName) )
		{
			ServerType.ServerType = static_cast< MMatchServerMode >( atoi(szAttrValue) );
			continue;
		}
	}

	vServerType.push_back( ServerType );
}


void MMatchEventFactoryManager::ParseGameType( MXmlElement& chrElement, vector< EventGameType >& vGameType )
{
	EventGameType GameType;
	char szAttrName[ 128 ];
	char szAttrValue[ 256 ];
	const int nAttrCnt = chrElement.GetAttributeCount();
	for( int i = 0; i < nAttrCnt; ++i )
	{
		chrElement.GetAttribute( i, szAttrName, szAttrValue );

		if( 0 == _stricmp(EL_ORDER, szAttrName) )
		{
			GameType.dwOrder = static_cast< u32 >( atol(szAttrValue) );
			continue;
		}

		if( 0 == _stricmp(EL_TYPE, szAttrName) )
		{
			GameType.GameType = static_cast< MMATCH_GAMETYPE >( atoi(szAttrValue) );
			continue;
		}
	}

	vGameType.push_back( GameType );
}


void MMatchEventFactoryManager::ParseStartEndTime( MXmlElement& chrElement, tm& stTime )
{
	memset( &stTime, 0, sizeof(stTime) );

	char szAttrName[ 128 ];
	char szAttrValue[ 256 ];
	const int nAttrCnt = chrElement.GetAttributeCount();
	for( int i = 0; i < nAttrCnt; ++i )
	{
		chrElement.GetAttribute( i, szAttrName, szAttrValue );

		if( 0 == _stricmp(EL_YEAR, szAttrName) )
		{
			stTime.tm_year =  static_cast< u16 >( atoi(szAttrValue) );
			continue;
		}

		if( 0 == _stricmp(EL_MONTH, szAttrName) )
		{
			stTime.tm_mon = static_cast< u16 >( atoi(szAttrValue) );
			continue;
		}

		if( 0 == _stricmp(EL_DAY, szAttrName) )
		{
			stTime.tm_yday = static_cast< u16 >( atoi(szAttrValue) );
			continue;
		}

		if( 0 == _stricmp(EL_HOUR, szAttrName) )
		{
			stTime.tm_hour = static_cast< u16 >( atoi(szAttrValue) );
			continue;
		}
	}
}


void MMatchEventFactoryManager::ParseEventPartTime( MXmlElement& chrElement, vector<EventPartTime>& EventPartTimeVec )
{
	EventPartTime ept;

	char szAttrName[ 128 ];
	char szAttrValue[ 256 ];
	const int nAttrCnt = chrElement.GetAttributeCount();
	for( int i = 0; i < nAttrCnt; ++i )
	{
		chrElement.GetAttribute( i, szAttrName, szAttrValue );

		/*
		if( 0 == _stricmp(EL_ORDER, szAttrName) )
		{
			continue;
		}
		*/

		if( 0 == _stricmp(EL_START_HOUR, szAttrName) )
		{
			ept.btStartHour = static_cast< u8 >( atoi(szAttrValue) );
			continue;
		}

		if( 0 == _stricmp(EL_END_HOUR, szAttrName) )
		{
			ept.btEndHour = static_cast< u8 >( atoi(szAttrValue) );
			continue;
		}
	}

#ifdef _DEBUG
	// 중복되는 범위가 있는지 검사.
	vector< EventPartTime >::iterator it, end;
	end = EventPartTimeVec.end();
	for( it = EventPartTimeVec.begin(); it != end; ++it )
	{
		if( (it->btStartHour <= ept.btStartHour) && (ept.btStartHour <= it->btEndHour) )
		{
			ASSERT( 0 );
		}
		
		if( (it->btStartHour <= ept.btEndHour) && (ept.btEndHour <= it->btEndHour) )
		{
			ASSERT( 0 );
		}
	}
#endif

	EventPartTimeVec.push_back( ept );
}


bool MMatchEventFactoryManager::GetEventList( const MMATCH_GAMETYPE GameType, const EVENT_TYPE EventType, EventPtrVec& EvnPtrVec )
{
	if( !m_bIsUsable )
		return true;

	switch( EventType )
	{
	case ET_BEGIN :
		{
			return GetOnBeginEventFactory().GetEventList( GameType, EvnPtrVec );
		}
		break;

	case ET_ONGAME :
		{
			return GetOnGameEventFactory().GetEventList( GameType, EvnPtrVec );
		}
		break;

	case ET_END :
		{
			return GetOnEndEventFactory().GetEventList( GameType, EvnPtrVec );
		}
		break;

	case ET_CUSTOM_EVENT :
		{
			return GetCustomEventFactory().GetEventList( GameType, EvnPtrVec );
		}
		break;

	default :
		{
			ASSERT( 0 && "정의되지 않은 이벤트 타입." );
			return false;
		}
		break;
	}
	return false;
}


bool MMatchEventFactoryManager::InsertEvent( const EventData& EvnData )
{
	switch( EvnData.EventType )
	{
	case ET_BEGIN :
		{
			return GetOnBeginEventFactory().InsertEventData( EvnData );
		}
		break;

	case ET_ONGAME :
		{
			return GetOnGameEventFactory().InsertEventData( EvnData );
		}
		break;

	case ET_END :
		{
			return GetOnEndEventFactory().InsertEventData( EvnData );
		}
		break;

	case ET_CUSTOM_EVENT :
		{
			return GetCustomEventFactory().InsertEventData( EvnData );
		}
		break;

	default :
		{
			ASSERT( 0 && "정의되지 않은 이벤트 타입." );
			return false;
		}
		break;
	}
	return false;
}

