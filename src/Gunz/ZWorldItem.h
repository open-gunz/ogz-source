#pragma once

#include "RTypes.h"
#include "RMesh.h"
#include "RVisualMeshMgr.h"
#include "list"
#include "map"
#include "MMatchWorldItemDesc.h"

#define _WORLD_ITEM_

using namespace std;
using namespace RealSpace2;

//enum ZWORLD_ITEM_SPAWN_TYPE
enum ZWORLD_ITEM_SPAWN_FLAG
{
	WORLD_ITEM_TIME_ONCE			= 0x1,		// 한번 생성되고 먹히면 끝 ( DEFAULT)
	WORLD_ITEM_TIME_REGULAR			= 0x2,		// 먹힌 후 일정 시간이 지나면 생성됨 
	WORLD_ITEM_STAND_ALINE			= 0x4,		// 서버와 통신하지 않고 클라이언트만 처리
};

enum ZWORLD_ITEM_STATE
{
	WORLD_ITEM_INVALIDATE = 0,
	WORLD_ITEM_VALIDATE,
	WORLD_ITEM_WAITING,
	WORLD_ITEM_CANDIDATE,	// 클라이언트에서는 판정이 남, 서버가 판정해 주길 기다리고 있는 상태	
	NUM_WORLD_ITEM_STATE,
};

enum ZWORLD_ITEM_EFFECT
{
	WORLD_ITEM_EFFECT_CREATE = 0,
	WORLD_ITEM_EFFECT_IDLE,
	WORLD_ITEM_EFFECT_NUM,
	WORLD_ITEM_EFFECT_REMOVE,
};

#define MAX_NAME_LENGTH 256
class ZCharacter;

//////////////////////////////////////////////////////////////////////////
class ZWorldItem
{
protected:
	short					m_nID;							// 인스턴스 ID
	short					m_nItemID;						// 아이템 ID
	char					m_Name[MAX_NAME_LENGTH];		// 아이템 이름
	char					m_modelName[MAX_NAME_LENGTH];	// 모델 이름
	MMATCH_WORLD_ITEM_TYPE	m_Type;							// 아이템의 종류
	ZWORLD_ITEM_STATE		m_State;						// 아이템의 상태
	MTD_WorldItemSubType	m_SubType;
	rvector					m_Position;						// 아이템의 월드 위치
	rvector					m_Dir;
	rvector					m_Up;
	unsigned int			m_nSpawnTypeFlags;
	float					m_fAmount;											
public:
	RVisualMesh*			m_pVMesh;
	unsigned int			m_dwStartTime;
	unsigned int			m_dwToggleBackupTime;
	bool					m_bToggle;
	bool					m_bisDraw;
public:
	// 아이템 생성
	void Initialize( int nID, short nItemID, MTD_WorldItemSubType SubType, ZWORLD_ITEM_STATE state, unsigned int nSpawnTypeFlags,	// 스폰 형식
		const rvector& position, float fAmount		);

	virtual bool ApplyWorldItem( ZCharacter* pCharacter );					// 먹히기

	void CreateVisualMesh();

public:
	void SetPostion( const rvector& p )						{ m_Position = p; };
	void SetDir( const rvector& p )						{ m_Dir = p; };
	void SetUp( const rvector& p )						{ m_Up = p; };
	void SetState( ZWORLD_ITEM_STATE state )		{ m_State	= state; };
	void SetType( MMATCH_WORLD_ITEM_TYPE type )	{ m_Type = type; };
	void SetName( char* szName )							{ strcpy_safe(m_Name, szName );	};
	void SetModelName( char* szName )						{ strcpy_safe(m_modelName, szName );	};
	
	MTD_WorldItemSubType GetSubType()						{ return m_SubType; }

	rvector GetPosition() const									{ return m_Position; };
	rvector GetDir() const									{ return m_Dir; };
	rvector GetUp() const									{ return m_Up; };

	MMATCH_WORLD_ITEM_TYPE GetType() const	{ return m_Type;	};
	ZWORLD_ITEM_STATE GetState() const				{ return m_State; };
	int GetID() const										{ return m_nID;	};	
	short GetItemID() const											{ return m_nItemID; }
	const char* GetName() const								{ return m_Name; };
	const char* GetModelName() const						{ return m_modelName; };
	unsigned int GetSpawnTypeFlags() const					{ return m_nSpawnTypeFlags; };
	
public:
	ZWorldItem();
	~ZWorldItem();
};

//////////////////////////////////////////////////////////////////////////
typedef list<ZWorldItem* > WaitingList;

typedef map< int, ZWorldItem* >		WorldItemList;
typedef WorldItemList::iterator		WIL_Iterator;

typedef map<string, RealSpace2::RVisualMesh* >  WorldItemVMeshMap;
typedef WorldItemVMeshMap::iterator	WIVMM_iterator;

//////////////////////////////////////////////////////////////////////////
class ZWorldItemDrawer
{
protected:
	WorldItemVMeshMap mVMeshList;

protected:
	RVisualMesh* AddMesh( const char* pName );

public:
	void Clear();

	void DrawWorldItem( ZWorldItem* pWorldItem, bool Rotate = false );
	void DrawEffect( ZWORLD_ITEM_EFFECT effect, const rvector& pos );

public:
	~ZWorldItemDrawer();	
};

//////////////////////////////////////////////////////////////////////////
class ZWorldItemManager
{
protected:
	WorldItemList			mItemList;
	ZWorldItemDrawer	mDrawer;
	static ZWorldItemManager msInstance;

	int						m_nStandAloneIDGen;
	int GenStandAlondID();
protected:
	bool ApplyWorldItem( WIL_Iterator& iter, ZCharacter* pCharacter );	// pCharacter에게 pWorldItem을 적용시키기
	void DeleteWorldItem( WIL_Iterator& iter, bool bDrawRemoveEffect );
	bool SpawnWorldItem( WIL_Iterator& iter );
	void OnOptainWorldItem(ZWorldItem* pItem);
public:
	void update();																			// 캐릭터와의 충돌 체크, 아이템 생성시간 체크
	ZWorldItem *AddWorldItem( int nID, short nItemID,MTD_WorldItemSubType nItemSubType, const rvector& pos );	// 맵 로딩시 아이템 추가할때 호출하는 함수
			
	bool DeleteWorldItem( int nID, bool bDrawRemoveEffect=false );

	void Clear();											// 아이템 리스트, 웨이팅 리스트... // 게임에서 나올때 호출
	void Reset(bool bDrawRemoveEffect=false);				// 팀전에서 라운드 시작할때 호출
	int GetLinkedWorldItemID(MMatchItemDesc* pItemDesc);		// 아이템과 연결된 월드아이템 ID 반환

	bool ApplyWorldItem( int nID, ZCharacter* pCharacter );			// pCharacter에게 pWorldItem을 적용시키기
	
	
	void AddQuestPortal(rvector& pos);		// 퀘스트에서 포탈 열릴때 호출


	static ZWorldItemManager*	GetInstance()	{ return &msInstance; }
	

public:
	void Draw();
	void Draw(int mode,float height,bool bWaterMap);

public:
	ZWorldItemManager();
	~ZWorldItemManager() {};
};

ZWorldItemManager* ZGetWorldItemManager();


// worlditem.xml에 있는 특별한 id
#define WORLDITEM_PORTAL_ID			201				// 퀘스트에서 사용하는 포탈
