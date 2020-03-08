#pragma once

#include "MQuestConst.h"
#include <set>
#include <map>
#include "SafeString.h"

/// 드랍 아이템 타입
enum MQuestDropItemType
{
	QDIT_NA			= 0,	///< 존재하지 않음
	QDIT_WORLDITEM	= 1,	///< HP, AP등의 일반적인 월드아이템
	QDIT_QUESTITEM	= 2,	///< 퀘스트 아이템
	QDIT_ZITEM		= 3,	///< 일반 아이템
};

/// 드롭 아이템 정보
struct MQuestDropItem
{
	MQuestDropItemType	nDropItemType;
	int					nID;
	int					nRentPeriodHour;
	// int					nMonsetBibleIndex;	// 어떤 종류의 몬스터가 아이템을 떨어뜨렸는지 이 아이템을 가지고 있던 몬스터의 종류를 저장.
											// 몬스터 도감을 위해서 사용.

	MQuestDropItem() : nDropItemType(QDIT_NA), nID(0), nRentPeriodHour(0) {}
	void Assign(MQuestDropItem* pSrc)		// 복사
	{
		nDropItemType	= pSrc->nDropItemType;
		nID				= pSrc->nID;
		nRentPeriodHour = pSrc->nRentPeriodHour;
	}
};

#define MAX_DROPSET_RATE		1000		///< 드롭 비율 0.001까지 설정 가능

/// 드롭 아이템 셋
class MQuestDropSet
{
private:
	int						m_nID;
	char					m_szName[16];
	MQuestDropItem			m_DropItemSet[MAX_QL+1][MAX_DROPSET_RATE];
	int						m_nTop[MAX_QL+1];
	std::set<int>				m_QuestItems;			// 이 세트가 가지고 있는 퀘스트 아이템 세트 - 클라이언트가 사용하려고 만듦
public:
	/// 생성자
	MQuestDropSet()
	{
		m_nID = 0;
		m_szName[0] = 0;
		memset(m_DropItemSet, 0, sizeof(m_DropItemSet));
		for (int i = 0; i <= MAX_QL; i++)
		{
			m_nTop[i] = 0;
		}
	}
	int GetID() { return m_nID; }									///< ID 반환
	const char* GetName() { return m_szName; }						///< 이름 반환
	void SetID(int nID) { m_nID = nID; }							///< ID 설정
	void SetName(const char* szName) { strcpy_safe(m_szName, szName); }	///< 이름 설정
	/// 드롭되는 아이템 추가
	/// @param pItem		드롭될 아이템 정보
	/// @param nQL			퀘스트 레벨
	/// @param fRate		나올 비율
	void AddItem(MQuestDropItem* pItem, int nQL, float fRate);
	/// 드롭될 아이템을 결정한다.
	/// @param outDropItem		드롭될 아이템 반환값
	/// @param nQL				퀘스트 레벨
	bool Roll(MQuestDropItem& outDropItem, int nQL);

	std::set<int>& GetQuestItems() { return m_QuestItems; }				///< 드롭될 아이템 종류
};


/// 드롭 테이블 관리자 클래스
class MQuestDropTable : public std::map<int, MQuestDropSet*>
{
private:
	void ParseDropSet(class MXmlElement& element);
	void ParseDropItemID(MQuestDropItem* pItem, const char* szAttrValue);
public:
	MQuestDropTable();													///< 생성자
	~MQuestDropTable();													///< 소멸자

	void Clear();
	
	bool ReadXml(const char* szFileName);								///< xml에서 정보를 읽는다. 
	bool ReadXml(class MZFileSystem* pFileSystem,const char* szFileName);		///< xml에서 정보를 읽는다. 
	/// 드롭 테이블 ID와 QL을 바탕으로 드롭될 아이템을 결정한다.
	/// @param outDropItem		드롭될 아이템 반환값
	/// @param nDropTableID		드롭 테이블 ID
	/// @param nQL				퀘스트 레벨
	bool Roll(MQuestDropItem& outDropItem, int nDropTableID, int nQL);
	MQuestDropSet* Find(int nDropTableID);								///< 드롭 아이템 셋 정보 반환
};