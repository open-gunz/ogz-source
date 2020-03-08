#include "stdafx.h"
#include "MQuestItem.h"
#include "MZFileSystem.h"
#include "MBaseStringResManager.h"

MUID MQuestItemMap::m_uidGenerate = MUID(0,0);
MCriticalSection MQuestItemMap::m_csUIDGenerateLock;

/////////////////////////////////////////////////////////////////////////////////////////////
bool MQuestItem::Create( const u32 nItemID, const int nCount, MQuestItemDesc* pDesc, bool bKnown )
{
	m_nItemID	= nItemID;
	m_pDesc		= pDesc;

	return SetCount( nCount, bKnown );
}

int MQuestItem::Increase( const int nCount)
{
	m_nCount += nCount;
	m_bKnown = true;

	if( MAX_QUEST_ITEM_COUNT < m_nCount )
	{
		int over = m_nCount - MAX_QUEST_ITEM_COUNT;
		m_nCount = MAX_QUEST_ITEM_COUNT;

		return over;
	}

	return 0;
}

int MQuestItem::Decrease( const int nCount)
{
	m_nCount -= nCount;
	if( 0 > m_nCount )
	{
		int lower = m_nCount;
		m_nCount = 0;

		return lower;
	}

	return 0;
}

MQuestItemDesc* MQuestItem::GetDesc()
{
	if( 0 != m_pDesc )
	{
		return m_pDesc; 
	}
	
	// 자신이 Description을 가지고 있을 않을경우.
	return GetQuestItemDescMgr().FindQItemDesc( m_nItemID );
}

bool MQuestItem::SetCount( int nCount, bool bKnown )
{ 
	if( (0 <= nCount) && (MAX_QUEST_ITEM_COUNT >= nCount) )
	{
		m_nCount = nCount;
	}
	else if( MAX_QUEST_ITEM_COUNT < nCount )
	{
		m_nCount = MAX_QUEST_ITEM_COUNT;
	}
	else
		return false;

	if (m_nCount == 0) m_bKnown = bKnown;
	else m_bKnown = true;
		
	return true;
}

/////////////////////////////////////////////////////////////////////////////////////////////

void MQuestMonsterBible::WriteMonsterInfo( int nMonsterBibleIndex )
{
	if( (0 > nMonsterBibleIndex) || (255 < nMonsterBibleIndex) )
		return;
	
	const int nIndex = nMonsterBibleIndex / 8;

	szData[ nIndex ] = ( szData[nIndex] | MakeBit(nMonsterBibleIndex) );
}


bool MQuestMonsterBible::IsKnownMonster( const int nMonsterBibleIndex )
{
	if( (0 > nMonsterBibleIndex) || (255 < nMonsterBibleIndex) )
		return false;
	
	const int  nIndex = nMonsterBibleIndex / 8;
	const char cBit   = MakeBit( nMonsterBibleIndex );

	if( cBit == (szData[nIndex] & cBit) )
		return true;

	return false;
}

/////////////////////////////////////////////////////////////////////////////////////////////


MQuestItemDescManager::MQuestItemDescManager()
{
	m_MonsterBibleMgr.clear();
}


MQuestItemDescManager::~MQuestItemDescManager()
{
	Clear();
}


bool MQuestItemDescManager ::ReadXml( const char* szFileName )
{
	if( 0 == szFileName )
		return false;


	MXmlDocument	xmlIniData;

	xmlIniData.Create();

	if (!xmlIniData.LoadFromFile(szFileName))
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

		if (!_stricmp(szTagName, MQICTOK_ITEM))
		{
			ParseQuestItem(chrElement);
		}
	}

	xmlIniData.Destroy();
	
	return true;
}


bool MQuestItemDescManager ::ReadXml( MZFileSystem* pFileSystem, const char* szFileName )
{
	if( (0== pFileSystem) || (0 == szFileName) )
		return false;

	MXmlDocument xmlIniData;
	if(!xmlIniData.LoadFromFile(szFileName, pFileSystem))
	{
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

		if (!_stricmp(szTagName, MQICTOK_ITEM))
		{
			ParseQuestItem(chrElement);
		}
	}

	xmlIniData.Destroy();

	return true;
}

void MQuestItemDescManager::ParseQuestItem( MXmlElement& element )
{
	MQuestItemDesc* pNewQuestItemDesc = new MQuestItemDesc;
	if( 0 == pNewQuestItemDesc )
		return;
	memset( pNewQuestItemDesc, 0, sizeof(MQuestItemDesc) );

	char szAttrName[ 256 ];
	char szAttrValue[ 1024 ];

	int nCount = element.GetAttributeCount();
	for( int i = 0; i < nCount; ++i )
	{
		element.GetAttribute( i, szAttrName, szAttrValue );
		if( 0 == strcmp(MQICTOK_ID, szAttrName) )
		{
			pNewQuestItemDesc->m_nItemID = atoi( szAttrValue );
		}
		else if( 0 == strcmp(MQICTOK_NAME, szAttrName) )
		{
			strcpy_safe(pNewQuestItemDesc->m_szQuestItemName, MGetStringResManager()->GetStringFromXml(szAttrValue));
		}
		else if( 0 == strcmp(MQICTOK_TYPE, szAttrName) )
		{
			if( 0 == strcmp(szAttrValue, "page") )				pNewQuestItemDesc->m_nType = MMQIT_PAGE;
			else if( 0 == strcmp(szAttrValue, "skull") )		pNewQuestItemDesc->m_nType = MMQIT_SKULL;
			else if( 0 == strcmp(szAttrValue, "fresh") )		pNewQuestItemDesc->m_nType = MMQIT_FRESH;
			else if( 0 == strcmp(szAttrValue, "ring") )			pNewQuestItemDesc->m_nType = MMQIT_RING;
			else if( 0 == strcmp(szAttrValue, "necklace") )		pNewQuestItemDesc->m_nType = MMQIT_NECKLACE;
			else if( 0 == strcmp(szAttrValue, "doll") )			pNewQuestItemDesc->m_nType = MMQIT_DOLL;
			else if( 0 == strcmp(szAttrValue, "book") )			pNewQuestItemDesc->m_nType = MMQIT_BOOK;
			else if( 0 == strcmp(szAttrValue, "object") )		pNewQuestItemDesc->m_nType = MMQIT_OBJECT;
			else if( 0 == strcmp(szAttrValue, "sword") )		pNewQuestItemDesc->m_nType = MMQIT_SWORD;
			else if( 0 == _stricmp(szAttrValue, "monbible") )	pNewQuestItemDesc->m_nType = MMQIT_MONBIBLE;
		}
		else if( 0 == strcmp(MQICTOK_DESC, szAttrName) )
		{
			strcpy_safe( pNewQuestItemDesc->m_szDesc, MGetStringResManager()->GetStringFromXml(szAttrValue) );
		}
		else if( 0 == strcmp(MQICTOK_UNIQUE, szAttrName) )
		{
			pNewQuestItemDesc->m_bUnique = (atoi( szAttrValue ) == 0) ? false : true;
		}
		else if( 0 == strcmp(MQICTOK_PRICE, szAttrName) )
		{
			pNewQuestItemDesc->m_nPrice = atoi( szAttrValue );
		}
		else if( 0 == strcmp(MQICTOK_SECRIFICE, szAttrName) )
		{
			pNewQuestItemDesc->m_bSecrifice = (atoi( szAttrValue ) == 0) ? false : true;
		}
		else if( 0 == strcmp(MQICTOK_PARAM, szAttrName) )
		{
			pNewQuestItemDesc->m_nParam = atoi( szAttrValue );
		}

	}

	_ASSERT( find(pNewQuestItemDesc->m_nItemID) == end() );
	insert( value_type(pNewQuestItemDesc->m_nItemID, pNewQuestItemDesc) );

	// Monster bible타입의 퀘스트 아이템은 Param값을 page로 사용함.
	if( MMQIT_MONBIBLE == pNewQuestItemDesc->m_nType )
		m_MonsterBibleMgr.insert( map<int, MQuestItemDesc*>::value_type(pNewQuestItemDesc->m_nParam, pNewQuestItemDesc) );
}


void MQuestItemDescManager::Clear()
{
	if( empty() )
		return;

	iterator It, End;
	End = end();
	for( It = begin(); It != End; ++It )
	{
		delete It->second;
	}
	clear();
}

MQuestItemDesc* MQuestItemDescManager::FindQItemDesc( const int nItemID )
{
	iterator it = find( nItemID );
	if (it != end())
	{
		return (*it).second;
	}
	return 0;
}


MQuestItemDesc* MQuestItemDescManager::FindMonserBibleDesc( const int nMonsterBibleIndex )
{
	map< int, MQuestItemDesc* >::iterator itMonsterBible = m_MonsterBibleMgr.find( nMonsterBibleIndex );
	if( m_MonsterBibleMgr.end() == itMonsterBible )
		return 0;

	if( IsMonsterBibleID(itMonsterBible->second->m_nItemID) )
	{
		return itMonsterBible->second;
	}
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////////


void MQuestItemMap::Clear()
{
	if( empty() ) 
		return;

	iterator It, End;
	End = end();
	for( It = begin(); It != End; ++It )
	{
		delete It->second;
	}
	clear();
}

void MQuestItemMap::Remove( const u32 nItemID )
{
	iterator It = find( nItemID );
	if( end() == It )
		return;

	MQuestItem* pDelQuestItem = It->second;
	delete pDelQuestItem;
	erase( It );
}

MQuestItem* MQuestItemMap::Find( const u32 nItemID )
{
	iterator It = find( nItemID );
	if( end() == It )
		return 0;

	return It->second;
}

bool MQuestItemMap::CreateQuestItem( const u32 nItemID, const int nCount, bool bKnown)
{
	MQuestItemDesc* pDesc = GetQuestItemDescMgr().FindQItemDesc( nItemID );
	if( 0 == pDesc )
	{
		// 해당 아이템만 추가를 하지 않고 정상적으로 진행하게 함.
		ASSERT( 0 );
		return true;
	}

	iterator itMyQItem = find( nItemID );
	if( end() != itMyQItem )
	{
		_ASSERT(0); // 같은 아이템을 이미 가지고 있으므로 생성안함. 이리로 오면 버그. - bird
		return false;
	}

	// 이전에 추가가 되어있지 않으므로 새롭게 추가됨.
	MQuestItem* pNewQuestItem = new MQuestItem();
	if( 0 == pNewQuestItem )
		return false;

	pNewQuestItem->Create( nItemID, nCount, pDesc, bKnown );

	insert( MQuestItemMap::value_type(nItemID, pNewQuestItem) );

	return true;
}

void MQuestItemMap::Insert( u32 nItemID, MQuestItem* pQuestItem )
{
	if( 0 == pQuestItem )
		return;

	insert( value_type(nItemID, pQuestItem) );
}