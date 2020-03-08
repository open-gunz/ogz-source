#ifndef _MQUEST_ITEM_H
#define _MQUEST_ITEM_H

#include "MUID.h"
#include <map>
#include "MQuestConst.h"
#include "MDebug.h"
#include "MZFileSystem.h"
#include "MXml.h"
#include "MSync.h"
#include "MMatchGlobal.h"
#include "MTime.h"

#define QUEST_ITEM_FILE_NAME	"zquestitem.xml"

#define MQICTOK_ITEM		"ITEM"
#define MQICTOK_ID			"id"
#define MQICTOK_NAME		"name"
#define MQICTOK_TYPE		"type"
#define MQICTOK_DESC		"desc"
#define MQICTOK_UNIQUE		"unique"
#define MQICTOK_PRICE		"price"
#define MQICTOK_SECRIFICE	"secrifice"
#define MQICTOK_PARAM		"param"


class MQuestMonsterBible
{
public :
	MQuestMonsterBible() {}
	~MQuestMonsterBible() {}

	void WriteMonsterInfo( int nMonsterBibleIndex );
	bool IsKnownMonster( const int nMonsterBibleIndex );
	
	inline const char MakeBit( const int nMonsterBibleIndex )
	{
		return 0x01 << (nMonsterBibleIndex % 8);
	}

	inline int operator [] (int nIndex ) const
	{
		return szData[ nIndex ];
	}

	void Clear() { memset( szData, 0, MAX_DB_MONSTERBIBLE_SIZE ); }

	char szData[ MAX_DB_MONSTERBIBLE_SIZE ];
};

#define MONSTER_BIBLE_SIZE sizeof(MQuestMonsterBible)


struct MQuestItemDesc
{
	u32	m_nItemID;
	char				m_szQuestItemName[ 32 ];
	int					m_nLevel;
	MQuestItemType		m_nType;
	int					m_nPrice;
	bool				m_bUnique;
	bool				m_bSecrifice;
	char				m_szDesc[ 8192 ];
	int					m_nLifeTime;
	int					m_nParam;

	int GetBountyValue() { return m_nPrice / 4; }
};

class MQuestItemDescManager : public std::map< int, MQuestItemDesc* >
{
public :
	MQuestItemDescManager();
	~MQuestItemDescManager();

	static MQuestItemDescManager& GetInst()
	{
		static MQuestItemDescManager QuestItemDescManager;
		return QuestItemDescManager;
	}

	bool ReadXml( const char* szFileName );
	bool ReadXml( MZFileSystem* pFileSystem, const char* szFileName );
	void ParseQuestItem( MXmlElement& element );
	void Clear();

	MQuestItemDesc* FindQItemDesc( const int nItemID );
	MQuestItemDesc* FindMonserBibleDesc( const int nMonsterBibleIndex );

	inline bool IsQItemID( const int nQItemID )
	{
		if( (MINID_QITEM_LIMIT > nQItemID) || (MAXID_QITEM_LIMIT < nQItemID) )
			return false;
		return true;
	}

	inline bool IsMonsterBibleID( const int nQItemID )
	{
		if( (MINID_MONBIBLE_LIMIT > nQItemID) || (MAXID_MONBIBLE_LIMIT < nQItemID) )
			return false;
		return true;
	}
private :
	std::map<int, MQuestItemDesc*> m_MonsterBibleMgr;
};

#define GetQuestItemDescMgr() MQuestItemDescManager::GetInst()

struct SimpleQuestItem
{
	u32	m_nItemID;
	unsigned int		m_nCount;
};

class MQuestItem
{
public:
	MQuestItem() : m_nCount( 0 ), m_pDesc( 0 ), m_bKnown( false )
	{
	}

	virtual ~MQuestItem() 
	{
	}

	bool Create( const u32 nItemID, const int nCount, MQuestItemDesc* pDesc, bool bKnown=true );
	int Increase( const int nCount = 1 );
	int Decrease( const int nCount = 1 );

	u32	GetItemID()	{ return m_nItemID; }
	int GetCount()	{ return m_nCount; }
	bool IsKnown()	{ return m_bKnown; }
	MQuestItemDesc* GetDesc();
	void SetDesc( MQuestItemDesc* pDesc ) { m_pDesc = pDesc; }
	void SetItemID( u32 nItemID )	{ m_nItemID = nItemID; }
	
	bool SetCount( int nCount, bool bKnown = true );

private:
	u32	m_nItemID;
	MQuestItemDesc*		m_pDesc;
	int					m_nCount;
	bool				m_bKnown;
};

class MQuestItemMap : public std::map< u32, MQuestItem* >
{
public :
	MQuestItemMap() : m_bDoneDbAccess( false )
	{
	}

	~MQuestItemMap()
	{
		Clear();
	}


	void SetDBAccess( const bool bState )	{ m_bDoneDbAccess = bState; }
	bool IsDoneDbAccess()					{ return m_bDoneDbAccess; }

	virtual bool	CreateQuestItem( const u32 nItemID, const int nCount, bool bKnown=true );
	void			Clear();
	void			Remove( const u32 nItemID );
	MQuestItem*		Find( const u32 nItemID );
	void			Insert( u32 nItemID, MQuestItem* pQuestItem );
	
private :
	static MUID				m_uidGenerate;
	static MCriticalSection	m_csUIDGenerateLock;
	bool					m_bDoneDbAccess;
};

inline bool IsQuestItemID(unsigned int nItemID)
{
	if ((MIN_QUESTITEM_ID <= nItemID) && (nItemID <= MAX_QUESTITEM_ID)) return true;
	return false;
}

#endif