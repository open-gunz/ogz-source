#include "StdAfx.h"
#include ".\ZGameTypeList.h"


ZGameTypeConfig::ZGameTypeConfig( void)
{
	m_nDefaultRound			= 0;
	m_nDefaultLimitTime		= 0;
	m_nDefaultMaxPlayers	= 16;
}


ZGameTypeConfig::~ZGameTypeConfig( void)
{
	// Delete round list
	while ( !m_Round.empty())
	{
		delete *m_Round.begin();
		m_Round.pop_front();
	}


	// Delete limit time list
	while ( !m_LimitTime.empty())
	{
		delete *m_LimitTime.begin();
		m_LimitTime.pop_front();
	}


	// Delete players list
	while ( !m_MaxPlayers.empty())
	{
		delete *m_MaxPlayers.begin();
		m_MaxPlayers.pop_front();
	}
}




ZGameTypeList::ZGameTypeList( void)
{
}


ZGameTypeList::~ZGameTypeList( void)
{
	Clear();
}


void ZGameTypeList::Clear( void)
{
	while( !m_GameTypeCfg.empty())
	{
		delete m_GameTypeCfg.begin()->second;
		m_GameTypeCfg.erase( m_GameTypeCfg.begin());
	}
}


bool ZGameTypeList::ParseGameTypeList( int nGameTypeNum, MXmlElement& element)
{
	int iCount = element.GetChildNodeCount();
	MXmlElement chrElement;
	char szTagName[256];

	ZGameTypeConfig* pConfig = new ZGameTypeConfig;

	int nRoundCount			= 0,
		nTimeLimitCount		= 0,
		nMaxPlayersCount	= 0;

	for (int i = 0; i < iCount; i++)
	{
		chrElement = element.GetChildNode(i);
		chrElement.GetTagName( szTagName);
		if ( szTagName[0] == '#') continue;

		if ( !_stricmp( szTagName, "ROUNDS"))
		{
			if ( ParseRound( pConfig, chrElement))
				pConfig->m_nDefaultRound = nRoundCount;

			nRoundCount++;
		}

		else if ( !_stricmp( szTagName, "LIMITTIME"))
		{
			if ( ParseLimitTime( pConfig, chrElement))
				pConfig->m_nDefaultLimitTime = nTimeLimitCount;

			nTimeLimitCount++;
		}

		else if ( !_stricmp( szTagName, "MAXPLAYERS"))
		{
			if ( ParseMaxPlayers( pConfig, chrElement))
				pConfig->m_nDefaultMaxPlayers = nMaxPlayersCount;

			nMaxPlayersCount++;
		}
	}

	m_GameTypeCfg.insert( m_GameTypeCfg.end(), MGAMETYPECFG::value_type( nGameTypeNum, pConfig));


	return true;
}


bool ZGameTypeList::ParseRound( ZGameTypeConfig* pConfig, MXmlElement& element)
{
	int nValue;
	char szStr[ 16];
	bool selected = false;
	element.GetAttribute( &nValue,   "round");
	element.GetAttribute( szStr,     "str");
	element.GetAttribute( &selected, "default");

	ZGameTypeConfigData* pCfgData = new ZGameTypeConfigData;
	pCfgData->m_nValue = nValue;
	strcpy_safe( pCfgData->m_szString, szStr);
	pConfig->m_Round.push_back( pCfgData);

	return selected;		// Is default?
}


bool ZGameTypeList::ParseLimitTime( ZGameTypeConfig* pConfig, MXmlElement& element)
{
	int nValue;
	char szStr[ 16];
	bool selected = false;
	element.GetAttribute( &nValue,   "sec");
	element.GetAttribute( szStr,     "str");
	element.GetAttribute( &selected, "default");

	ZGameTypeConfigData* pCfgData = new ZGameTypeConfigData;
	pCfgData->m_nValue = nValue;
	strcpy_safe( pCfgData->m_szString, szStr);
	pConfig->m_LimitTime.push_back( pCfgData);

	return selected;		// Is default?
}


bool ZGameTypeList::ParseMaxPlayers( ZGameTypeConfig* pConfig, MXmlElement& element)
{
	int nValue;
	char szStr[ 16];
	bool selected = false;
	element.GetAttribute( &nValue,   "player");
	element.GetAttribute( szStr,     "str");
	element.GetAttribute( &selected, "default");

	ZGameTypeConfigData* pCfgData = new ZGameTypeConfigData;
	pCfgData->m_nValue = nValue;
	strcpy_safe( pCfgData->m_szString, szStr);
	pConfig->m_MaxPlayers.push_back( pCfgData);

	return selected;		// Is default?
}


ZGameTypeConfig* ZGameTypeList::GetGameTypeCfg( int nGameTypeNum)
{
	ZGameTypeConfig* pGameTypeCfg = NULL;

	MGAMETYPECFG::iterator itr = m_GameTypeCfg.find( nGameTypeNum);
	if ( itr != m_GameTypeCfg.end())
	{
		pGameTypeCfg = (*itr).second;
	}


	return pGameTypeCfg;
}
