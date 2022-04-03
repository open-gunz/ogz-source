#pragma once
#include "MDatabase.h"

template <typename T, typename U> inline T Cvt(const U& x) { return static_cast<T>(x); }
template <> inline CStringSQL Cvt(const long& x) { return strprintf("%l", x); };
template <> inline CStringSQL Cvt(const double& x) { return strprintf("%f", x); }
template <> inline long    Cvt(const CStringSQL& x) { return atol(x.c_str()); }
template <> inline double  Cvt(const CStringSQL& x) { return atof(x.c_str()); }

struct CDBField
{
	auto AsBool()   const { return GetI<bool>(); }
	auto AsChar()   const { return GetI<unsigned char>(); }
	auto AsShort()  const { return GetI<short>(); }
	auto AsInt()    const { return GetI<int>(); }
	auto AsLong()   const { return Get<long>(); }
	auto AsFloat()  const { return float(Get<double>()); }
	auto AsDouble() const { return Get<double>(); }
	auto AsString() const { return Get<CStringSQL>(); }

	bool IsNull()   const;

	variant<long, double, CStringSQL> var{ 0l };
	CStringSQL Name;
	SQLLEN LenOrInd;

private:
	template <typename T>
	T Get() const
	{
		if (IsNull())
			return T{};
		if (holds_alternative<T>(var))
			return get<T>(var);
		return visit([](auto& x) { return Cvt<T>(x); }, var);
	}

	template <typename T>
	T GetI() const
	{
		return static_cast<T>(Get<long>());
	}
};

#define BINARY_FIELD_MAX_SIZE	7000
#define BINARY_CHUNK_SIZE		7000
#define RESERVE_SIZE	100

class CSimpleDBBinary
{
public:
	CSimpleDBBinary() : m_UsedSize(0) {}
	~CSimpleDBBinary() {}

	int GetUsedSize() { return m_UsedSize; }

	void SetUsedSize(const int iSize)
	{
		m_UsedSize = iSize;
	}

	int GetData(unsigned char* pOutBuf, const int nOutBufSize)
	{
		if ((0 == pOutBuf) ||
			(m_UsedSize > nOutBufSize) ||
			(0 >= nOutBufSize))
			return -1;

		memcpy(pOutBuf, m_Data, m_UsedSize);

		return m_UsedSize;
	}

	int SetData(const unsigned char* pData, const int nInDataSize)
	{
		if ((0 == pData) ||
			(BINARY_FIELD_MAX_SIZE < nInDataSize) ||
			(0 > nInDataSize))
			return -1;

		m_UsedSize = nInDataSize;

		memcpy(m_Data, pData, nInDataSize);

		return m_UsedSize;
	}

private:
	char	m_Data[BINARY_FIELD_MAX_SIZE];
	int		m_UsedSize;

public:
	int test;
};

class CDBBinary
{
public:
	CDBBinary(const int iFieldNum = RESERVE_SIZE, const int iReserveSize = RESERVE_SIZE) :
		m_iIndex(0), m_iCurrentUsedSize(0), m_ReserveSize(iReserveSize)
	{
		Reserve(iFieldNum);
	}

	~CDBBinary()
	{
		m_vBinary.clear();
	}

	typedef int							BinaryLength;
	typedef std::vector< CSimpleDBBinary >	BinaryDataVec;
	typedef BinaryDataVec::iterator		BinaryDataIter;

	void Begin()
	{
		m_iIndex = 0;
	}

	void Clear() { m_vBinary.clear(); }

	int	GetCurUsedSize() { return m_iCurrentUsedSize; }
	int	GetCurIndex() { return m_iIndex; }
	int	GetReserverSize() { return m_ReserveSize; }

	void SetCurUsedSize(const int iCurUsedSize) { m_iCurrentUsedSize = iCurUsedSize; }

	void SetReserveSize(const int nReserveSize)
	{
		if (0 > nReserveSize)
		{
			assert(0);
			return;
		}

		m_ReserveSize = nReserveSize;
	}

	int GetNextData(unsigned char* pOutputBuf, const int nDestBufSize)
	{
		if (m_iIndex > m_iCurrentUsedSize)
			return -1;

		if (-1 == m_vBinary[m_iIndex].GetData(pOutputBuf, nDestBufSize))
			return -1;

		return m_vBinary[m_iIndex++].GetUsedSize();
	}

	int InsertData(const unsigned char* pData, const int nSrcDataSize)
	{
		if (m_iIndex >= static_cast<int>(m_vBinary.capacity()))
		{
			if (!Reserve(m_ReserveSize))
				return -1;
		}

		++m_iIndex;
		m_iCurrentUsedSize = m_iIndex;

		CSimpleDBBinary sbn;

		// test
		sbn.test = m_iIndex;

		if (nSrcDataSize != sbn.SetData(pData, nSrcDataSize))
		{
			--m_iIndex;
			--m_iCurrentUsedSize;

			return -1;
		}

		m_vBinary.push_back(sbn);

		return sbn.GetUsedSize();
	}

private:
	bool Reserve(const int iExtSize)
	{
		if (0 > iExtSize)
		{
			assert(0);
			return false;
		}

		try
		{
			m_vBinary.reserve(iExtSize + m_vBinary.size());
		}
		catch (...)
		{
			m_iCurrentUsedSize = -1;
			m_iIndex = -1;

			return false;
		}

		return true;
	}

private:
	int				m_iIndex;
	int				m_iCurrentUsedSize;
	int				m_ReserveSize;
	BinaryDataVec	m_vBinary;
};

struct CRecordset
{
	enum OpenType { forwardOnly };
	enum OpenOptions { readOnly };
};

struct CODBCRecordset
{
	CODBCRecordset(MDatabase* pDatabase) : m_pDatabase(pDatabase) {}
	bool Open(const CStringSQL& lpszSQL, u32 nOpenType = 0, u32 dwOptions = 0);
	void Close();
	void MoveNext();
	CDBField& Field(const char* szName);

	CDBBinary SelectBinary(const CStringSQL& strQuery);
	bool InsertBinary(const CStringSQL& strQuery, const u8* pData, int nSize);

	bool IsOpen() const { return Opened; }
	int GetRecordCount() const { return RecordCount; }
	bool IsBOF() const { return End; }
	bool IsEOF() const { return End; }

private:
	void Bind();
	SQLSMALLINT NextSet();

	std::vector<CDBField> Fields;
	long ReturnValue = 0;
	SQLLEN ReturnValueLenOrInd = -1;
	MDatabase* m_pDatabase = nullptr;
	SQLHandle<SQLHandleTypes::Stmt> Statement;
	SQLINTEGER RecordCount = 0;
	bool Opened = false;
	bool End = true;
};