#include "stdafx.h"
#include "MDebug.h"
#include "MMatchConfig.h"
#include "MMatchItem.h"
#include "MMatchAntiHack.h"
#include "MMatchEventFactory.h"
#include "MBMatchServerConfigReloader.h"


bool MBMatchServerServerIniReloadObj::OnReload()
{
	MGetServerConfig()->Clear();
	if( MGetServerConfig()->Create() )
		mlog( "MBMatchServerServerIniReloadObj::OnReload - success reload server.ini reload\n" );
	else
	{
		ASSERT( 0 && "Server.ini reload½ÇÆÐ." );
		mlog( "MBMatchServerServerIniReloadObj::OnReload - fail to server.ini reload\n" );
	}

	return true;
}


bool MBMatchServerZItemXmlReloadObj::OnReload()
{
	MGetMatchItemDescMgr()->Clear();
	MGetMatchItemDescMgr()->ReadXml( GetFileName().c_str() );

	mlog( "MBMatchServerZItemXmlReloadObj::OnReload - success reload %s\n",
		GetFileName().c_str() );

	return true;
}


bool MBMatchServerHashmapTxtReloadObj::OnReload()
{
	mlog( "MBMatchServerHashmapTxtReloadObj::OnReload - success reload %s\n",
		GetFileName().c_str() );

	return true;
}

bool MBMatchServerFileListCrcReloadObj::OnReload()
{
	MMatchAntiHack::ClearClientFileList();
	MMatchAntiHack::InitClientFileList();

	mlog( "MBMatchServerFileListCrcReloadObj::OnReload - success reload %s\n",
		GetFileName().c_str() );

	return true;
}


bool MBMatchServerEventXmlReloadObj::OnReload()
{
	if( MMatchEventDescManager::GetInstance().LoadEventXML(GetFileName().c_str()) )
		mlog( "MBMatchServerEventXmlReloadObj::OnReload - success reload %s\n", GetFileName().c_str() );
	else
	{
		ASSERT( 0 && "fail to reload Event.xml" );
		mlog( "MBMatchServerEventXmlReloadObj::OnReload - fail to reload %s\n", 
			GetFileName().c_str() );
		return false;
	}

	return true;
}


bool MBMatchServerEventListXmlReloadObj::OnReload()
{
	if( MMatchEventFactoryManager::GetInstance().LoadEventListXML(GetFileName().c_str()) )
		mlog( "MBMatchServerEventListXmlReloadObj::OnReload - success to reload %s\n",	GetFileName().c_str() );
	else
	{
		ASSERT( 0 && "fail to reload EventList.xml" );
		mlog( "MBMatchServerEventListXmlReloadObj::OnReload - fail to reload %s\n",	
			EVENT_LIST_XML_FILE_NAME );
		return false;
	}

	return true;
}


///////////////////////////////////////////////////////////////////////////////////////////////////////


MBMatchServerConfigReloader::MBMatchServerConfigReloader()
{
}


MBMatchServerConfigReloader::~MBMatchServerConfigReloader()
{
	ClearReloadObjMap();
}


bool MBMatchServerConfigReloader::Reload( const string& strReloadFileList )
{
	ReloadFileListVec	FileList;
	
	return Reload( FileList );
}


bool MBMatchServerConfigReloader::Reload( ReloadFileListVec& FileList )
{
	if( FileList.empty() )
		return false;

	ReloadFileListVec::iterator it, end;
	end = FileList.end();
	for( it = FileList.begin(); it != end; ++it )
		OnReload( (*it) );

	m_FileList.clear();
	m_FileList.swap( FileList );

	return true;
}


bool MBMatchServerConfigReloader::Create()
{
	if( !InitReloadObjMap() )
		return false;

	return true;
}


bool MBMatchServerConfigReloader::InitReloadObjMap()
{
	if( !InsertReloadMap(string("server.ini"), new MBMatchServerServerIniReloadObj) )
		return false;

	if( !InsertReloadMap(string("zitem.xml"), new MBMatchServerZItemXmlReloadObj) )
		return false;

	if( !InsertReloadMap(string("hashmap.txt"), new MBMatchServerHashmapTxtReloadObj) )
		return false;

	if( !InsertReloadMap(string("filelistcrc.txt"), new MBMatchServerFileListCrcReloadObj) )
		return false;

	if( !InsertReloadMap(string("event.xml"), new MBMatchServerEventXmlReloadObj) )
		return false;

	if( !InsertReloadMap(string("eventlist.xml"), new MBMatchServerEventListXmlReloadObj) )
		return false;

	return true;
}

bool MBMatchServerConfigReloader::InsertReloadMap( const string& strFileName, MBMatchServerReloadObj* pReloadObj )
{
	if( strFileName.empty() || (0 == pReloadObj) )
	{
		ASSERT( 0 );
		return false;
	}

	pReloadObj->SetFileName( strFileName );

	m_ReloadObjMap.insert( ReloadObjMap::value_type(strFileName, pReloadObj) );

	return true;
}


void MBMatchServerConfigReloader::ClearReloadObjMap()
{
	if( !m_ReloadObjMap.empty() )
		return;

	ReloadObjMap::iterator it, end;
	end = m_ReloadObjMap.end();
	for( it = m_ReloadObjMap.begin(); it != end; ++end )
		delete it->second;
	m_ReloadObjMap.clear();
}


bool MBMatchServerConfigReloader::OnReload( const string& strReloadFile )
{
	if( strReloadFile.empty() )
		return false;

	ReloadObjMap::iterator itFind = m_ReloadObjMap.find( strReloadFile );
	if( m_ReloadObjMap.end() == itFind )
		return false;

	return itFind->second->Reload();
}