#ifndef _MBMATCHSERVERCONFIGRELOADER
#define _MBMATCHSERVERCONFIGRELOADER


/*
 ReloadList.xml에 들어가는 파일 이름은 읽어들인후 모두 소문자로 변화해서 처리함. - by SungE

 현제 리로드 가능한 목록. 
	server.ini
	zitem.xml
	hashmap.txt
	event.xml
	eventlist.xml
 */


#define RELOAD_LIST "ReloadList.xml"


#include <vector>
#include <string>
#include <map>
#include "MUtil.h"
using namespace std;


class MBMatchServerReloadObj
{
public :
	MBMatchServerReloadObj() 
		: m_bIsReloadOK( false )
	{
		memset( &m_LastReloadTime, 0, sizeof(tm) );
	}

	virtual ~MBMatchServerReloadObj() 
	{
	}

	bool Reload()
	{
		if( m_strFileName.empty() ) return false;
		SetReloadStatus( false );

		const bool bIsReloadOK = OnReload();

		SetReloadStatus( bIsReloadOK );
		if( bIsReloadOK )
			m_LastReloadTime = *localtime( &unmove(time(0)) );
		
		return bIsReloadOK;
	}

	const string&		GetFileName() const			{ return m_strFileName; }
	const bool			IsReloadOK() const			{ return m_bIsReloadOK; }
	const tm&	GetLastReloadTime() const	{ return m_LastReloadTime; }

	void SetFileName( const string& strFileName )	{ m_strFileName = strFileName; }
	
private :
	void SetReloadStatus( const bool bIsReloadOK )	{ m_bIsReloadOK = bIsReloadOK; }

	virtual bool OnReload() = 0;

private :
	string		m_strFileName;
	bool		m_bIsReloadOK;
	tm	m_LastReloadTime;
};


class MBMatchServerServerIniReloadObj : public MBMatchServerReloadObj
{
private :
	bool OnReload();
};


class MBMatchServerZItemXmlReloadObj : public MBMatchServerReloadObj
{
private :
	bool OnReload();
};


class MBMatchServerHashmapTxtReloadObj : public MBMatchServerReloadObj
{
private :
	bool OnReload();
};

class MBMatchServerFileListCrcReloadObj : public MBMatchServerReloadObj
{
private :
	bool OnReload();
};


class MBMatchServerEventXmlReloadObj : public MBMatchServerReloadObj
{
private :
	bool OnReload();
};


class MBMatchServerEventListXmlReloadObj : public MBMatchServerReloadObj
{
private :
	bool OnReload();
};

typedef vector< string >						ReloadFileListVec;
typedef map< string, MBMatchServerReloadObj* >	ReloadObjMap;


class MBMatchServerConfigReloader
{
public :
	MBMatchServerConfigReloader();
	~MBMatchServerConfigReloader();

	bool Reload( const string& strReloadFileList );
	bool Reload( ReloadFileListVec& FileList );
	bool OnReload( const string& strReloadFile );
	bool Create();

	void ClearReloadObjMap();

private :
	bool InitReloadObjMap();
	bool InsertReloadMap( const string& strFileName, MBMatchServerReloadObj* pReloadObj );

private :
	ReloadFileListVec	m_FileList;
	ReloadObjMap		m_ReloadObjMap;
};


#endif