#ifndef _COUNTRYCODE_FILTER_
#define _COUNTRYCODE_FILTER_

#ifdef _FILTER_TEST
#include "MDebug.h"
#endif

#include "GlobalTypes.h"

#include <map>
#include <string>
#include <vector>
#include <utility>

using std::string;
using std::vector;
using std::pair;
using std::map;

struct BlockCountryCodeInfo
{
	string	strCountryCode;
	string	strRoutingURL;
	bool	bIsBlock;
};


struct IPtoCountry
{
	u32	nIPFrom;
	u32	nIPTo;
	string	strCountryCode3;
};

struct CustomIP
{
	u32	nIPFrom;
	u32	nIPTo;
	bool	bIsBlock;
	string	strCountryCode3;
	string	strComment;
};


typedef vector< BlockCountryCodeInfo >	BlockCountryCodeInfoList;
typedef vector< IPtoCountry >			IPtoCountryList;
typedef vector< CustomIP >				CustomIPList;


class StrICmp
{
public :
	StrICmp( const string& strSrc ) : m_strSrc( strSrc ) {}

	bool operator () ( const BlockCountryCodeInfo& rfBlockCountryCodeInfo )
	{
		return 0 == _stricmp( m_strSrc.c_str(), rfBlockCountryCodeInfo.strCountryCode.c_str() );
	}

private :
	StrICmp() {}

	string m_strSrc;
};

class SortCmp
{
public :
	bool operator () ( const IPtoCountry& ipc1, const IPtoCountry& ipc2 )
	{
		return ipc1.nIPFrom > ipc2.nIPFrom;
	}
};

#ifdef _FILTER_TEST
		struct MdlTrace
		{
			size_t head;
			size_t middle;
			size_t tail;
		};
#endif

template <typename T >
class IPRangeBinarySearch
{
public :
	IPRangeBinarySearch() : m_dwIP( 0 ) 
	{
#ifdef _FILTER_TEST
		m_dwTraceCount = 0;
		m_dwMaxTraceCount = 10;
#endif
	}

	void SetIP( const u32 dwIP ) { m_dwIP = dwIP; }

	const int BinarySearch( const T& tVector )
	{
		size_t nHead = 0;
		size_t nTail = tVector.size() - 2;
		size_t nMiddle = nTail / 2;

		u32 dwIPFrom;
		u32 dwIPTo;

#ifdef _FILTER_TEST
		MdlTrace mt;
		vector< MdlTrace > vMdlTrace;
#endif

		while( true ){
			dwIPFrom = tVector[ nMiddle ].nIPFrom;
			dwIPTo   = tVector[ nMiddle ].nIPTo;

#ifdef _FILTER_TEST
			mt.head = nHead;
			mt.middle = nMiddle;
			mt.tail = nTail;
			vMdlTrace.push_back( mt );
#endif

			if( nTail > nHead ){
				if( (dwIPFrom <= m_dwIP) && (tVector[nMiddle + 1].nIPFrom > m_dwIP) ){
					if( dwIPTo >= m_dwIP ) 
						return static_cast<int>(nMiddle);
				}

				if( dwIPFrom < m_dwIP ){
					nHead = nMiddle + 1;
				}
				else if( dwIPFrom > m_dwIP ){
					nTail = nMiddle - 1;
				}

				nMiddle = (nHead + nTail) / 2;
			}
			else{
				if( (dwIPFrom <= m_dwIP) && (dwIPTo >= m_dwIP) )
					return static_cast<int>(nMiddle);
				++nMiddle;
				if( (tVector[nMiddle].nIPFrom <= m_dwIP) && (tVector[nMiddle].nIPTo >= m_dwIP) )
					return static_cast<int>(nMiddle);

#ifdef _FILTER_TEST
				if( m_dwMaxTraceCount > m_dwTraceCount ){
					u32 i = 0;
					T::const_iterator it, end;
					end = tVector.end();
					for( it = tVector.begin(); it != end; ++it, ++i ){
						if( it->nIPTo >= m_dwIP ){
							if( it->nIPFrom <= m_dwIP ){
								mlog( "================== CountryCode miss hit trace ====================" );
								mlog( "Index:%u, IP:%u\n", i, m_dwIP );
								vector< MdlTrace >::iterator it2, end2;
								it2 = vMdlTrace.begin();
								end2 = vMdlTrace.end();
								for( ; it2 != end2; ++it2 )
									mlog( "H:%u, M:%u, T:%u\n", it2->head, it2->middle, it2->tail );
								mlog("\n==================================================================\n" );
								++m_dwTraceCount;
							}
						}
					}
					mlog( "Can't find : %u\n", m_dwIP );
				}
#endif
				return -1;
			}
		}

#ifdef _FILTER_TEST
#endif
		return -1;
	}

private :
	u32 m_dwIP;
#ifdef _FILTER_TEST
	u32 m_dwTraceCount;
	u32 m_dwMaxTraceCount;
#endif
};


class MCountryCodeFilter
{
public :
	MCountryCodeFilter();
	~MCountryCodeFilter();

	bool Create( const BlockCountryCodeInfoList& rfBlockCountryCodeInfoList, 
				 const IPtoCountryList& rfIPtoCountryList );

	bool AddIPtoCountry( const u32 dwIPFrom, const u32 dwIPTo, const string& strCode );
	const int GetCustomIP( const string& strIP, string& strOutCountryCode, bool& bIsBlock, string& strComment );
	const int GetIPCountryCode( const string& strIP, string& strOutCountryCode );
	bool IsValidContryCode( const string& strCountryCode, string& strOutRoutingURL );
	bool Update( const BlockCountryCodeInfoList& rfBlockCountryCodeInfoList, 
				 const IPtoCountryList& rfIPtoCountryList );

	u64 GetLastUpdatedTime() const				{ return m_dwLastUpdatedTime; }
	void SetLastUpdatedTime( const u64 dwTime )	{ m_dwLastUpdatedTime = dwTime; }

private :
	bool InitContryCodeTableList( const BlockCountryCodeInfoList& rfBlockCountryCodeInfoList );
	bool InitIPtoCountryList( const IPtoCountryList& rfIPtoCountryList );

	bool CheckIPtoCountryRange( const u32 dwIPFrom, const u32 dwIPTo, const IPtoCountryList& icl );
	bool CheckIsLast( const u32 dwIPFrom, const u32 dwIPTo, const IPtoCountryList& icl );
	bool CheckIsDuplicatedRange( const u32 dwIPFrom, const u32 dwIPTo, const IPtoCountryList& icl );
	bool CheckIsInverseRange( const u32 dwIPFrom, const u32 dwIPTo );
	bool IsValidContryCode( const string& strCountryCode, string& strOutRoutingURL,
		BlockCountryCodeInfoList& bcil );

	bool UpdateCountryCodeTableList( const BlockCountryCodeInfoList& rfBlockCountryCodeInfoList );
	bool UpdateIPtoCountryList( const IPtoCountryList& rfIPtoCountryList );

	const u32 inet_aton( const string& strIP );

#ifdef _LOCATOR_TEST
public :
	void DoTest();
	void TestAddIPtoCountry( const IPtoCountryList& rfIPtoCountryList );
	bool FindEqual( const u32 dwIPFrom, const u32 dwIPTo, const string& strCode );
	void TestIP();
#endif

private :
    BlockCountryCodeInfoList	m_BlockCountryCodeInfoList;
	IPtoCountryList				m_IPtoCountryList;
	CustomIPList				m_CustomIPList;

	u64 m_dwLastUpdatedTime;

	IPRangeBinarySearch< IPtoCountryList >	m_IPCountryCodeSearch;
	IPRangeBinarySearch< CustomIPList >			m_CustomIPSearch;
};

#endif