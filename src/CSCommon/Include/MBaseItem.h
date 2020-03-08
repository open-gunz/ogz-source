#ifndef _MBASEITEM_H
#define _MBASEITEM_H


#define MAX_ITEM_COUNT					100			// 한사람이 최대로 갖을 수 있는 아이템 개수
//#define RENT_PERIOD_UNLIMITED			(8760)		// 기간제 아이템 hour period가 8760이면 무제한(1년)
#define RENT_MINUTE_PERIOD_UNLIMITED	(525600)	// 클라이언트한테는 기간제 아이템 기간을 minute단위로 보낸다. 525600이면 무제한(1년)


// 아이템. 서버, 클라이언트 공통의 부모 클래스
class MBaseItem
{
protected:
	bool				m_bIsRentItem;						///< 기간제 아이템인지 여부
	int					m_nRentMinutePeriodRemainder;		///< 기간제 남은기간(분단위)
	bool				m_bIsSpendingItem;					///< 소비아이템인지 여부
	int					m_nCount;							///< 수량
public:
	MBaseItem(): m_bIsRentItem(false), m_nRentMinutePeriodRemainder(RENT_MINUTE_PERIOD_UNLIMITED), m_bIsSpendingItem(false),
					m_nCount(0) { }
	virtual ~MBaseItem() = default;
	bool IsRentItem()		{ return m_bIsRentItem; }
	bool IsSpendingItem()	{ return m_bIsSpendingItem; }
	int GetRentMinutePeriodRemainder() { return ((IsRentItem()) ? m_nRentMinutePeriodRemainder : RENT_MINUTE_PERIOD_UNLIMITED); }
	void SetRentItem(int nRentMinutePeriodRemainder)	
	{ 
		m_bIsRentItem=true; 
		m_nRentMinutePeriodRemainder=nRentMinutePeriodRemainder; 
	}
};


#endif