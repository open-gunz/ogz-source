#include "stdafx.h"
#include "MMatchObject.h"
#include "MMath.h"
#include "MAsyncDBJob_Event.h"
#include "MMatchEvent.h"
#include "MUtil.h"
#include <tuple>


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// Use tuple's operator <= to implement a lexicographical comparison.
auto tie_tm(const tm& t)
{
	return std::tie(t.tm_year, t.tm_mon, t.tm_yday, t.tm_hour);
};

auto get_cur_time()
{
	return *localtime(&unmove(time(0)));
}

bool CheckUsableEventTimeByStartTime( const tm& StartTime )
{
	return tie_tm(StartTime) <= tie_tm(get_cur_time());
}

bool CheckUsableEventTimeByEndTime( const tm& EndTime )
{
	return tie_tm(EndTime) >= tie_tm(get_cur_time());
}


bool CheckUsableEventPartTime( const EventPartTime& ept )
{
	auto cur_time = get_cur_time();
	return (ept.btStartHour <= cur_time.tm_hour) && (ept.btEndHour >= cur_time.tm_hour);
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

MMatchEvent::~MMatchEvent()
{
	m_dwEventListID = 0;
}


bool MMatchEvent::DoItNow( const u32 dwCurTime )
{
	return CheckEventTime() && CheckElapsedTimeIsOverflow( dwCurTime );
}


void MMatchEvent::CheckEventObj( MMatchObject* pObj, u64 dwCurTime )
{
	if( CheckEventTime() )
		OnCheckEventObj( pObj, dwCurTime );
}


void MMatchEvent::Run()
{
	if( CheckEventTime() )
		OnRun();
}


bool MMatchEvent::CheckEventTime()
{
	return CheckUsableEventTimeByStartTime(m_Start) && 
		   CheckUsableEventTimeByEndTime(m_End);
}


bool MMatchEvent::CheckElapsedTimeIsOverflow( u64 dwCurTime )
{
#ifdef _DEBUG
	if( m_dwCheckElapsedTime < (dwCurTime - m_dwLastCheckTime) )
		mlog( "UpdateEvent : %u(%u, %u)\nRealTime : (%s)\n", dwCurTime - m_dwLastCheckTime, m_dwLastCheckTime, dwCurTime, MGetStrLocalTime().c_str() );
#endif
	return (m_dwCheckElapsedTime < (dwCurTime - m_dwLastCheckTime));
}


void MMatchEvent::SetLastCheckTime( u64 dwCurTime ) 
{ 
	if( CheckElapsedTimeIsOverflow(dwCurTime) )
		m_dwLastCheckTime = dwCurTime; 
}


void MMatchEvent::Set( const u32 dwEventListID, const u32 dwEventType, const u32 dwGameType, const u32 dwCheckElapsedTime, const u32 dwPercent, 
		const u32 dwRate, const tm& Start, const tm& End, const string& strName, const string& strAnnounce,
		const float fXPBonusRatio, const float fBPBonusRatio, const vector<EventPartTime>& EventPartTimeVec )
{
	m_dwEventListID			= dwEventListID;
	m_dwEventType			= dwEventType;
	m_dwGameType			= dwGameType;
	m_dwCheckElapsedTime	= dwCheckElapsedTime;
	m_dwPercent				= dwPercent;
	m_dwRate				= dwRate;
	m_Start					= Start;
	m_End					= End;
	m_strName				= strName;
	m_strAnnounce			= strAnnounce;
	m_fXPBonusRatio			= fXPBonusRatio;
	m_fBPBonusRatio			= fBPBonusRatio;	

	vector< EventPartTime >::const_iterator it, end;
	end = EventPartTimeVec.end();
	for( it = EventPartTimeVec.begin(); it != end; ++it )
	{
		m_EventPartTimeVec.push_back( (*it) );
	}
}


void MMatchEvent::Reset()
{
	m_dwEventListID			= 0;
	m_dwEventID				= 0;
	m_dwEventType			= 0;
	m_dwGameType			= 0;
	m_dwCheckElapsedTime	= 0;
	m_dwLastCheckTime		= 0;
	m_dwPercent				= 0;
	m_dwRate				= 0;
	m_fXPBonusRatio			= 0.0f;
	m_fBPBonusRatio			= 0.0f;
}


bool MMatchEvent::CheckEventPartTime()
{
	if( m_EventPartTimeVec.empty() )
		return true;

	vector< EventPartTime >::iterator it, end;
	end = m_EventPartTimeVec.end();
	for( it = m_EventPartTimeVec.begin(); it != end; ++it )
	{
		if( CheckUsableEventPartTime((*it)) )
			return true;
	}

	return false;
}


///////////////////////////////////////////////////////////////////////////////////////////////////


void MMatchEventDesc::Set( const u32 dwEventID, const string& strDesc )
{
	m_dwEventID = dwEventID;
	m_strDesc = strDesc;
}


bool MMatchEventDescManager::Insert( const u32 dwEventID, MMatchEventDesc* pEventDesc )
{
	if( 0 == pEventDesc ) 
		return false;

	pair< MMatchEventDescManager::iterator, bool > prRes = 
		insert( MMatchEventDescManager::value_type(dwEventID, pEventDesc) );

	return prRes.second;
}


const MMatchEventDesc* MMatchEventDescManager::Find( const u32 dwEventID )
{
	auto itFind = find( dwEventID );

	if (itFind == end())
		return nullptr;

	return itFind->second;
}


bool MMatchEventDescManager::LoadEventXML( MZFileSystem* pFileSystem, const string& strFileName )
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

		if (!_stricmp(EV_EVENT, szTagName))
		{
			ParseEvent( chrElement );
		}
	}

	xmlIniData.Destroy();

	return true;
}


bool MMatchEventDescManager::LoadEventXML( const string& strFileName )
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

	for (int i = 0; i < iCount; i++)
	{
		chrElement = rootElement.GetChildNode(i);
		chrElement.GetTagName(szTagName);
		if (szTagName[0] == '#') continue;

		if (!_stricmp(EV_EVENT, szTagName))
		{
			ParseEvent( chrElement );
		}
	}

	xmlIniData.Destroy();
	
	return true;
}


void MMatchEventDescManager::ParseEvent( MXmlElement& chrElement )
{
	u32 dwEventID;
	string strDesc;
	MMatchEventDesc* pEventDesc;
	char szAttrName[ 128 ];
	char szAttrValue[ 256 ];
	const int nAttrCnt = chrElement.GetAttributeCount();
	for( int i = 0; i < nAttrCnt; ++i )
	{
		chrElement.GetAttribute( i, szAttrName, szAttrValue );

		if( 0 == _stricmp(EV_EVENTID, szAttrName) )
		{
			dwEventID = static_cast< u32 >( atol(szAttrValue) );
			continue;
		}

		if( 0 == _stricmp(EV_DESC, szAttrName) )
		{
			strDesc = MGetStringResManager()->GetString( string(szAttrValue) );
			continue;
		}
	}

	pEventDesc = new MMatchEventDesc;
	if( 0 != pEventDesc )
	{
		pEventDesc->Set( dwEventID, strDesc );

		Insert( dwEventID, pEventDesc );
	}
}


///////////////////////////////////////////////////////////////////////////////////////////////////


MMatchProbabiltyEventPerTime::~MMatchProbabiltyEventPerTime()
{
}


void MMatchProbabiltyEventPerTime::OnCheckEventObj( MMatchObject* pObj, const u32 dwCurTime )
{
	if( 0 == pObj ) 
		return;

	if( CheckElapsedTimeIsOverflow(dwCurTime) )
	{
#ifdef _DEBUG
		mlog( "Event user : UserID(%s), CharName(%s).\n", 
			pObj->GetAccountInfo()->m_szUserID, 
			pObj->GetCharInfo()->m_szName );
#endif

		const u32 nRndNum = RandomNumber( 1, m_dwPercent );
		if( nRndNum <= m_dwRate )
		{
			m_vEventObj.push_back( pObj->GetUID() );		
		}
        
		// 여기서 업데이트하면 가장 처음 유저만 처리되고 그 다음부턴 비정상 처리됨.
		// SetLastCheckTime( dwCurTime ); 
	}
}


void MMatchProbabiltyEventPerTime::OnRun()
{
	if( m_vEventObj.empty() ) 
		return;

	MMatchServer* pServer = MMatchServer::GetInstance();
	if( 0 == pServer )
		return;

	AsyncEventObj		EventObj;
	MMatchObject*		pObj;
	AsyncEventObjVec	EventObjList;

	MAsyncDBJob_ProbabiltyEventPerTime* pEvnJob = new MAsyncDBJob_ProbabiltyEventPerTime;
	if( 0 != pEvnJob )
	{
		vector< MUID >::iterator it, end;
		end = m_vEventObj.end();
		for( it = m_vEventObj.begin(); it != end; ++it )
		{
			pObj = pServer->GetObject( (*it) );
			if( 0 != pObj )
			{
				EventObj.uidUser = pObj->GetUID();
				EventObj.dwAID = pObj->GetAccountInfo()->m_nAID;
				EventObj.dwCID = pObj->GetCharInfo()->m_nCID;

				EventObjList.push_back( EventObj );
			}
		}
	}

	if( pEvnJob->Input(EventObjList, m_strName, m_strAnnounce) )
	{
		if( 0 != MMatchServer::GetInstance() )
			MMatchServer::GetInstance()->PostAsyncJob( pEvnJob );
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////

MMatchXPBonusEvent::~MMatchXPBonusEvent()
{
}


bool MMatchXPBonusEvent::CustomCheckEventObj( MMatchObject* pObj, void* pContext )
{
	if( !CheckEventTime() )
		return false;

	if( !CheckEventPartTime() )
		return false;

	if( (0 == pObj) || (0 == pContext) )
	{
		ASSERT( 0 );
		return false;
	}

	MMatchServer* pServer = MMatchServer::GetInstance();
	if( 0 == pServer )
		return false;

	if( 0 == pServer->GetObject(pObj->GetUID()) )
		return false;

	*((float*)pContext) += m_fXPBonusRatio;
	return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////

MMatchBPBonusEvent::~MMatchBPBonusEvent()
{
}


bool MMatchBPBonusEvent::CustomCheckEventObj( MMatchObject* pObj, void* pContext )
{
	if( !CheckEventTime() )
		return false;

	if( !CheckEventPartTime() )
		return false;

	if( (0 == pObj) || (0 == pContext) )
	{
		ASSERT( 0 );
		return false;
	}

	MMatchServer* pServer = MMatchServer::GetInstance();
	if( 0 == pServer )
		return false;

	if( 0 == pServer->GetObject(pObj->GetUID()) )
		return false;

	*((float*)pContext) += m_fBPBonusRatio;
	return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////

