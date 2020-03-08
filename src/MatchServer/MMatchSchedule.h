#pragma once

#include <vector>
#include <list>

using std::vector;
using std::list;

class MCommand;
class MServer;
class MMatchScheduleImpl;

/*
 * 모든 스케쥴은 bIsNeedDelete이 true가 되면 제거됨.
 *
 * once : 한번 실행하면 무조건 bIsNeedDelete를 true로 설정함.
 *
 * count : 실행시 m_nCount변수를 1씩 감소시키고 0이 되는 시점에서 bIsNeedDelete를 true로 설정함.
 *			cMonth, cDay, cHour, cMin이 실행 시간과 같아야 실행함.
 *			cNextMonth, cNextDay, cNextHour, cNextMin는 다음에 실행될 시간의 증가치.
 *
 * repeat : 계속해서 실행함. bIsNeedDelete를 true로 만들지 않음.
 *			cMonth, cDay, cHour, cMin이 실행 시간과 같아야 실행함.
 */

class MMatchDayOfMonth
{
public :
	MMatchDayOfMonth();
	~MMatchDayOfMonth();

	static MMatchDayOfMonth& GetInst()
	{
		static MMatchDayOfMonth inst;
		return inst;
	}

	inline char GetMaxDay( const int iMonth ) 
	{
		if( 1 > iMonth ) 
			return -1;

		// 배열값으로 수정후 처리.
		return m_cDayOfMonth[ iMonth - 1 ];
	}
private :
	void Init();

private :
	enum MONTH_NUM
	{
		MonthNum = 12,
	};

	char m_cDayOfMonth[ MONTH_NUM::MonthNum ];
};

class MMatchScheduleData
{
public:
	// 스케쥴의 종류는 1:repeat, 2:Count, 3:Once로 되어있음.
	enum MMatchScheduleState
	{
		REPEAT = 0,
		COUNT,
		ONCE,
	};

	MMatchScheduleData(void);
	~MMatchScheduleData(void);

	void CorrectTime();

	inline int					GetType()		{ return m_nType; }
	inline MCommand*			GetCommand()	{ return m_pCmd; }
	inline bool					IsNeedDelete()	{ return m_bIsNeedDelete; }
	inline MMatchScheduleImpl*	GetImpl()		{ return m_pImpl; }
	inline int					GetErrorTime()	{ return m_nErrorTime; }

	inline int GetCount() { return m_nCount; }

	inline void IncreaseCount() { ++m_nCount; }
	inline void DecreaseCount() { --m_nCount; }

	inline unsigned char GetYear()	{ return m_cYear; }
	inline unsigned char GetMonth()	{ return m_cMonth; }
	inline unsigned char GetDay()	{ return m_cDay; }
	inline unsigned char GetHour()	{ return m_cHour; }
	inline unsigned char GetMin()	{ return m_cMin; }

	inline unsigned char GetNextYear()	{ return m_cNextYear; }
	inline unsigned char GetNextMonth()	{ return m_cNextMonth; }
	inline unsigned char GetNextDay()	{ return m_cNextDay; }
	inline unsigned char GetNextHour()	{ return m_cNextHour; }
	inline unsigned char GetNextMin()	{ return m_cNextMin; }

	inline void SetType( const int nType )				{ m_nType = nType; }
	inline void SetCommand( MCommand* pCmd )			{ m_pCmd = pCmd; }
	inline void SetImpl( MMatchScheduleImpl* pImpl )	{ m_pImpl = pImpl; }
	inline void SetDeleteState( const bool bState )		{ m_bIsNeedDelete = bState; }
	inline void SetErrorTime( const int nErrorTime )	{ m_nErrorTime = nErrorTime; }

	inline void SetCount( const int nStartCount ) { m_nCount = nStartCount; }

	inline void SetYear( const unsigned char cYear )	{ m_cYear = cYear; }
	inline void	SetMonth( const unsigned char cMonth )	{ m_cMonth = cMonth; }
	inline void	SetDay( const unsigned char cDay )		{ m_cDay = cDay; }
	inline void SetHour( const unsigned char cHour )	{ m_cHour = cHour; }
	inline void	SetMin( const unsigned char cMin )		{ m_cMin = cMin; }

	inline void SetNextYear( const unsigned char cYear )	{ m_cNextYear = cYear; }
	inline void	SetNextMonth( const unsigned char cMonth )	{ m_cNextMonth = cMonth; }
	inline void	SetNextDay( const unsigned char cDay )		{ m_cNextDay = cDay; }
	inline void SetNextHour( const unsigned char cHour )	{ m_cNextHour = cHour; }
	inline void	SetNextMin( const unsigned char cMin )		{ m_cNextMin = cMin; }

	// return if same, then 0, less -1, greater 1.
	int	CompareCurrentTime();
	bool SetTimes( const unsigned char cYear, 
				   const unsigned char cMonth, 
				   const unsigned char cDay, 
				   const unsigned char cHour, 
				   const unsigned char cMin );
	bool SetNextTimes( const unsigned char cNextYear, 
					   const unsigned char cNextMonth, 
					   const unsigned char cNextDay, 
					   const unsigned char cNextHour, 
					   const unsigned char cNextMin );

	void ResetTime();

	void Release();

private :
	// int				nID;
	int					m_nType;			// 스케쥴 타입( REPAT, COUNT, ONCE ).
	MCommand*			m_pCmd;				// 스케쥴이 활성화 되었을때 실행할 명령.
	bool				m_bIsNeedDelete;	// true이면 스케쥴을 제거 또는 비활성화 함.
	MMatchScheduleImpl*	m_pImpl;			// 스케쥴이 수행되기전에 해야 할 수행자.

	int	m_nCount;		// 특정 횟수를 실행할 경우 사용.
	int m_nErrorTime;	// 보정에 적용될수 있는 한계치( 분 ).

	unsigned char	m_cYear;	// 실행 되어야 하는 해.
	unsigned char	m_cMonth;	// 실행 되어야 하는 달.
	unsigned char	m_cDay;		// 실행 되어야 하는 날.
	unsigned char	m_cHour;	// 실행 되어야 하는 시간.
	unsigned char	m_cMin;		// 실행 되어야 하는 분.
	
	unsigned char	m_cNextYear;	// 다음에 실해되어야 하는 해의 증가치.
	unsigned char	m_cNextMonth;	// 다음에 실행되어야 하는 달의 증가치.
	unsigned char	m_cNextDay;		// 다음에 실행되어야 하는 일의 증가치.
	unsigned char	m_cNextHour;	// 다음에 실행되어야 하는 시간의 증가치.
	unsigned char	m_cNextMin;		// 다음에 실행되어야 하는 분의 증가치.
};

class MMatchScheduleReleaser
{
public :
	void operator() ( MMatchScheduleData*& rfSchedulerData )
	{
		if( 0 != rfSchedulerData ){
			rfSchedulerData->Release();
			delete rfSchedulerData;
			rfSchedulerData = 0;
		}
	}
};

/// 각 스케쥴 타입에 맞는 데이터 처리를 해줄 수행자.
// 순수 가상 인터페이스 클래스.
class MMatchScheduleImpl
{
public :
	virtual ~MMatchScheduleImpl() {}

	// 시간 오차 보정.
	virtual void CorrectTime( MMatchScheduleData* pScheduleData ) = 0;
	// 시간을 다시 설정함.
	virtual void Reset( MMatchScheduleData* pScheduleData ) = 0;
};
// 제거되지 않고 계속 반복되는 스케쥴.
class MMatchRepeatScheduleImpl : public MMatchScheduleImpl
{
public :
	// 시간 오차 보정.
	void CorrectTime( MMatchScheduleData* pScheduleData );
	// 시간을 다시 설정함.
	void Reset( MMatchScheduleData* pScheduleData );
};
// 일정 횟수를 실행하고 제거되는 스케쥴.
class MMatchCountScheduleImpl : public MMatchScheduleImpl
{
public :
	// 시간 오차 보정.
	void CorrectTime( MMatchScheduleData* pScheduleData );
	// 시간을 다시 설정함.
	void Reset( MMatchScheduleData* pScheduleData );
};
// 한번만 실행하고 제거되는 스케쥴.
class MMatchOnceScheduleImpl : public MMatchScheduleImpl
{
public :
	// 시간 오차 보정.
	void CorrectTime( MMatchScheduleData* pScheduleData );
	// 시간을 다시 설정함.
	void Reset( MMatchScheduleData* pScheduleData );
};


/// 스케쥴을 저장하고 관리하는 스케쥴 관리자.
typedef vector< MMatchScheduleData* > ScheduleVec;
typedef ScheduleVec::iterator	ScheduleVecIter;

typedef list< MMatchScheduleData* >	ScheduleLst;
typedef ScheduleLst::iterator	ScheduleLstIter;


class MMatchScheduleMgr
{
public :
	MMatchScheduleMgr( MServer* pServer );
	~MMatchScheduleMgr();

	// 동적인 스케쥴 등록. 조건에 의해서 지워질수 있음.
	bool AddDynamicSchedule( MMatchScheduleData* pNewSchedule );
	// 시스템이 시작하면서 종료될때까지 유지되어야 하는 스케쥴.
	bool AddStaticSchedule( MMatchScheduleData* pNewSchedule );

	// Schedule data maker.
	MMatchScheduleData* MakeRepeatScheduleData( const unsigned char	cNextYear,	// 다음에 실행될 해의 상대값.	ex) 1년 후.
												const unsigned char cNextMonth, // 다음에 실행될 달의 상대값.	ex) 1달 후.
												const unsigned char cNextDay,	// 다음에 실행될 일의 상대값.	ex) 1일 후.
												const unsigned char cNextHour,	// 다음에 실행될 시간의 상대값.	ex) 1시간 후.
												const unsigned char cNextMin,	// 다음에 실행될 분의 상대값.	ex) 1분 후.
												MCommand* pCmd					// 수행할 명령.
												);
	MMatchScheduleData* MakeCountScheduleData( const unsigned char cNextYear,	// 다음에 실행될 해의 상대값.	ex) 1년 후.
											   const unsigned char cNextMonth,	// 다음에 실행될 달의 상대값.	ex) 1달 후.
											   const unsigned char cNextDay,	// 다음에 실행될 일의 상대값.	ex) 1일 후.
											   const unsigned char cNextHour,	// 다음에 실행될 시간의 상대값.	ex) 1시간 후.
											   const unsigned char cNextMin,	// 다음에 실행될 분의 상대값.	ex) 1분 후.
											   const unsigned int nCount,		// 실행될 횟수.
											   MCommand* pCmd					// 수행할 명령.
											   );
	MMatchScheduleData* MakeOnceScheduleData( const unsigned char cYear,	// 실행될 해.
											  const unsigned char cMonth,	// 실행될 달.
											  const unsigned char cDay,		// 실행될 날.
                                              const unsigned char cHour,	// 실행될 시간.
                                              const unsigned char cMin,		// 실행될 분.
                                              MCommand* pCmd				// 수행할 명령.	
											  );

	// 초기화.
	bool Init();

	inline void SetUpdateTerm( time_t tmTime ) { m_tmUpdateTerm = tmTime; }

	// 등록되있는 모든 스케쥴을 탐색하여 업데이트함.
	void Update();

	// 종료시까지 남아있는 스케쥬을 제거함.
	void Release();

private :
	// 마지막 업데이트 시점에서부터 경과시간 계산.
	const time_t CalculateElapseUpdateTime();
	// 정상적적인 데이터인지 검사함.
	bool CheckData( MMatchScheduleData* pScheduleData ) const;
	// 업데이트 되어야 하는 시간인지 검사함.
	bool CompareTime( MMatchScheduleData* pScheduleData );
	// 업데이트가 가능한지 검사.
	bool IsUpdate();

	void UpdateStaticSchedule();
	void UpdateDynamicSchedule();

	void SetLastUpdateTime();

	void ReleaseStaticSchedule();
	void ReleaseDynamicSchedule();

	enum MONTH_NUM { MonthNum = 12, };

	// 수행자를 가지고 있는 저장 클래스.
	class MMatchScheduleImplPrototype
	{
	private :
		friend MMatchScheduleMgr;

		typedef vector< MMatchScheduleImpl* >	ScheduleImplVec;

		MMatchScheduleImplPrototype() {}
		~MMatchScheduleImplPrototype() {}

		bool Init();
		// 해당 타입ID에 맞는 수행자의 포인터를 넘겨줌. 절대 delete하면 않됨.
		MMatchScheduleImpl* GetImpl( const unsigned int nTypeID );
		void Release();

		// 수행자를 저장하고 있을 벡터.
		ScheduleImplVec	m_ScheduleImplVec;
	};

	class MMatchScheduleImplRelease
	{
	public :
		void operator() ( MMatchScheduleImpl*& rfImpl )
		{
			if( 0 != rfImpl ){
				delete rfImpl;
				rfImpl = 0;
			}
		}
	};

private :

	MServer*	m_pServer;

	int	m_nIndex;

	time_t m_tmUpdateTerm;		// 업데이트되는 간격.
	time_t m_tmLastUpdateTime;	// 마지막 업데이트된 시간.

	MMatchScheduleImplPrototype	m_ScheduleImplPrototype;

	vector< MMatchScheduleData* >	m_vecStaticSchedule;	// 시스템 가동시 등록되는 스케쥴. 시스템수명과 같음.
	list< MMatchScheduleData* >		m_lstDynamicSchedule;	// 공지사항.
};

int MMatchGetLocalTime(tm *ptm);
char GetMaxDay( const int iMonth );
char GetMaxDay();
bool AddDynamicSchedule( MMatchScheduleMgr* pScheduleMgr, const int nType, MCommand* pCmd, const int nYear, const int nMonth, const int nDay, const int nHour, const int nMin, const int nCount );