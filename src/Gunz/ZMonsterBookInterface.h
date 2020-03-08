/***********************************************************************
  ZMonsterBookInterface.h
  
  용  도 : 몬스터 도감 인터페이스
  작성일 : 29, MAR, 2004
  작성자 : 임동환
************************************************************************/


#ifndef _ZMONSTERBOOKINTERFACE_H
#define _ZMONSTERBOOKINTERFACE_H



// 퀘스트 희생 아이템 간단 정보
struct MQuestItemSimpleDesc
{
	char			m_szName[ 32];								// 퀘스트 희생 아이템 이름
	// 원래 머 더 넣을려다가 걍 놔둠...

	MQuestItemSimpleDesc()
	{
		m_szName[ 0] = 0;
	}
};


// Class : ZMonsterBookInterface
class ZMonsterBookInterface
{
// protected varialbes
protected:
	MBitmapR2*		m_pBookBgImg;								// 배경 책 이미지
	MBitmapR2*		m_pIllustImg;								// 일러스트 비트맵 이미지

	map< int, MQuestItemSimpleDesc>	 m_QuestItemDesc;			// 퀘스트 아이템 정보 리스트

//	ZSkillTable		m_SkillTable;

	int				m_nCurrentPage;								// 현재 보고있는 페이지 번호를 기록

	bool			m_bIsFirstPage;								// 첫페이지인지 아닌지...


// protected functions
protected:
	void DrawPage( void);										// 페이지를 그린다
	void DrawFirstPage( void);									// 첫페이지를 그린다
	void DrawComplete( void);									// 달성률을 표시한다

	bool ReadQuestItemXML( void);								// 퀘스트 희생 아이템 XML을 읽는다
	bool ReadSimpleQuestItemDesc( int nItemID, map< int, MQuestItemSimpleDesc>::iterator* pIterator);		// 희생 아이템 리스트로부터 아이템 정보를 얻는다



// public variables
public:


// public functions
public:
	// Initialize
	ZMonsterBookInterface( void);								// Constructor
	virtual ~ZMonsterBookInterface( void);						// Destructor

	void OnCreate( void);										// On Create
	void OnDestroy( void);										// On destroy

	void OnPrevPage( void);										// 이전 페이지 넘기기 버튼을 눌렀을 때
	void OnNextPage( void);										// 다음 페이지 넘기기 버튼을 눌렀을 때

	bool SetIllustImage( const char* szFileName);				// 현재 페이지에 해당하는 몬스터 일러스트 이미지를 표시한다
	bool DeleteIllustImage( void);								// 읽어왔던 몬스터 일러스트 이미지를 메모리에서 삭제한다
};


#endif
