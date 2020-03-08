#include "stdafx.h"
#include "MZFileSystem.h"
#include "MMatchRuleQuest.h"
#include "MSacrificeQItemTable.h"



bool MSacrificeQItemTable::ReadXML( const char* pszFileName )
{
	if( 0 == pszFileName )
		return false;

	MXmlDocument xmlIniData;

	xmlIniData.Create();

	if( !xmlIniData.LoadFromFile(pszFileName) )
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

		if (!_stricmp(szTagName, MSQITC_ITEM))
		{
			ParseTable(chrElement);
		}
	}

	xmlIniData.Destroy();

	return true;
}


bool MSacrificeQItemTable::ReadXml( MZFileSystem* pFileSystem, const char* szFileName )
{
	if( (0 == pFileSystem) || (0 == szFileName) )
		return false;

	MXmlDocument	xmlIniData;
	xmlIniData.Create();

	//	<-----------------
	char *buffer;
	MZFile mzf;

	if(pFileSystem) 
	{
		if(!mzf.Open(szFileName,pFileSystem)) 
		{
			if(!mzf.Open(szFileName)) 
			{
				xmlIniData.Destroy();
				return false;
			}
		}
	} 
	else 
	{
		if(!mzf.Open(szFileName))
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

		if (!_stricmp(szTagName, MSQITC_ITEM))
		{
			ParseTable(chrElement);
		}
	}

	xmlIniData.Destroy();

	return true;
}


void MSacrificeQItemTable::ParseTable( MXmlElement& element )
{
	char szAttrName[ 256 ];
	char szAttrValue[ 1024 ];

	int					nQL;
	MSacrificeQItemInfo SacriQItemInfo;

	memset( &SacriQItemInfo, 0, sizeof(MSacrificeQItemInfo) );

	int nCount = element.GetAttributeCount();
	for( int i = 0; i < nCount; ++i )
	{
		element.GetAttribute( i, szAttrName, szAttrValue );

		if( 0 == strcmp(MSQITC_MAP, szAttrName) )
		{
			strcpy_safe( SacriQItemInfo.m_szMap, szAttrName );
		}
		else if( 0 == strcmp(MSQITC_QL, szAttrName) )
		{
			nQL = atoi( szAttrValue );
			SacriQItemInfo.m_nQL = nQL;
		}
		else if( 0 == strcmp(MSQITC_DIID, szAttrName) )
		{
			SacriQItemInfo.m_nDefaultQItemID = atoi( szAttrValue );
		}
		else if( 0 == strcmp(MSQITC_SIID1, szAttrName) )
		{
			SacriQItemInfo.m_nSpecialQItemID1 = atoi( szAttrValue );
		}
		else if( 0 == strcmp(MSQITC_SIID2, szAttrName) )
		{
			SacriQItemInfo.m_nSpecialQItemID2 = atoi( szAttrValue );
		}
		else if( 0 == strcmp(MSQITC_SIGNPC, szAttrName) )
		{
		}
		else if( 0 == strcmp(MSQITC_SDC, szAttrName) )
		{
		}
		else if( 0 == strcmp(MSQITC_SID, szAttrName) )
		{
			SacriQItemInfo.m_nScenarioID = atoi( szAttrValue );	
		}
	}

	/*
#ifdef _DEBUG
	mlog( "MSacrificeQItemTable::ParseTable - QL : %d\n", SacriQItemInfo.m_nQL );
	mlog( "MSacrificeQItemTable::ParseTable - Default Item id : %d\n", SacriQItemInfo.m_nDefaultQItemID );
	mlog( "MSacrificeQItemTable::ParseTable - Special Item1 id : %d\n", SacriQItemInfo.m_nSpecialQItemID1 );
	mlog( "MSacrificeQItemTable::ParseTable - Special Item2 id : %d\n", SacriQItemInfo.m_nSpecialQItemID2 );
#endif
	*/

	insert( value_type(nQL, SacriQItemInfo) );
}


int MSacrificeQItemTable::FindSacriQItemInfo( const int nQL, MQuestSacrificeSlot* pSacrificeSlot, int& outResultQL )
{

	if( 0 == nQL )
	{
		outResultQL = nQL;
		if( (0 == pSacrificeSlot[0].GetItemID()) && (0 == pSacrificeSlot[1].GetItemID()) )
			return MSQITRES_NOR;
		else
			return MSQITRES_INV;
	}

	MSacrificeQItemTable::iterator iter, itUpper;

	for( int i = 1; i <= nQL; ++i )
	{
		iter = lower_bound( i );
		if( end() == iter )
			return MSQITRES_ERR;

		itUpper = upper_bound( i );

		outResultQL = i;

		for( ; iter != itUpper; ++iter )
		{
#ifdef _DEBUG
			mlog( "CurQL:%d\n", i );
			mlog( "FindSacriQItemInfo - SacriSlot1:%d\n", pSacrificeSlot[0].GetItemID() );
			mlog( "FindSacriQItemInfo - SacriSlot2:%d\n", pSacrificeSlot[1].GetItemID() );
			mlog( "FindSacriQItemInfo - DefItemID:%d\n", iter->second.GetDefQItemID() );
			mlog( "FindSacriQItemInfo - SpecItemID1:%d\n", iter->second.GetSpeQItemID1() );
			mlog( "FindSacriQItemInfo - SpecItemID2:%d\n\n", iter->second.GetSpeQItemID2() );
#endif
			if( ((iter->second.GetDefQItemID() == pSacrificeSlot[0].GetItemID()) && (0 == pSacrificeSlot[1].GetItemID()))  ||
				((iter->second.GetDefQItemID() == pSacrificeSlot[1].GetItemID()) && (0 == pSacrificeSlot[0].GetItemID())) )
			{
				// 일반 시나리오.
				return MSQITRES_NOR;
			}
			else if( ((iter->second.GetSpeQItemID1() == pSacrificeSlot[0].GetItemID()) && (iter->second.GetSpeQItemID2() == pSacrificeSlot[1].GetItemID())) ||
				((iter->second.GetSpeQItemID1() == pSacrificeSlot[1].GetItemID()) && (iter->second.GetSpeQItemID2() == pSacrificeSlot[0].GetItemID())) )
			{
				// 특별 시나리오.
				return MSQITRES_SPC;
			}
			else if( (0 == pSacrificeSlot[0].GetItemID()) && (0 == pSacrificeSlot[1].GetItemID()) )
			{
				// 양쪽이 다 빈 슬롯. 
				// 양쪽이 다 비어있을경우 QL : 1에서 검색이 끝나기 때문에 outResultQL은 1로 설정이 됨.
				return MSQITRES_EMP;
			}
			else if( pSacrificeSlot[0].GetItemID() == pSacrificeSlot[1].GetItemID() )
			{
				// 같은 아이템 중복.
				outResultQL = 1;
				return MSQITRES_DUP;
			}
		}
	}

	// 여기까지 내려오면 테이블에 등록되지 않은 아이템.
	outResultQL = 1;
	return MSQITRES_INV;
}

bool MSacrificeQItemTable::TestInitTable()
{
	return true;
}