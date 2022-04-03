#include "MDatabaseInternal.h"
#include "MUtil.h"

bool CDBField::IsNull() const { return LenOrInd == SQL_NULL_DATA; }

bool CODBCRecordset::Open(const CStringSQL& lpszSQL, u32, u32)
{
	RecordCount = 0;
	Opened = false;
	End = false;
	if (!m_pDatabase->IsOpen())
		return false;
	CALL(SQLAlloc, Statement, m_pDatabase->GetConn());
	CALL(SQLExecDirect, Statement, SQLData(lpszSQL), SQLSize(lpszSQL));
	Bind();
	if (!End)
		MoveNext();
	Opened = true;
	return true;
}

void CODBCRecordset::Close()
{
	Opened = false;
	Statement.Handle.reset();
}

void CODBCRecordset::Bind()
{
	SQLSMALLINT NumCols = 0;
	while (NumCols == 0)
	{
		CALL(SQLNumResultCols, Statement, &NumCols);
		if (NumCols == 0 && CALL(SQLMoreResults, Statement) == SQL_NO_DATA)
		{
			End = true;
			return;
		}
	}
	Fields.resize(NumCols);
	for (SQLSMALLINT i = 1; i <= NumCols; ++i)
	{
		auto&& Field = Fields[i - 1];
		constexpr SQLSMALLINT Size = 1024;
		SQLCHAR NameBuf[Size];
		SQLSMALLINT NameLength, DataType, DecimalDigits, Nullable;
		SQLULEN ColumnSize;
		CALL(SQLDescribeCol, Statement, i, NameBuf, Size, &NameLength,
			&DataType, &ColumnSize, &DecimalDigits, &Nullable);
		Field.Name.assign(reinterpret_cast<char*>(NameBuf), NameLength);
		switch (DataType)
		{
		case SQL_CHAR:
		case SQL_VARCHAR:
		case SQL_DATETIME:
		case SQL_NUMERIC:
		case SQL_DECIMAL:
		case SQL_TIME:
		case SQL_TIMESTAMP:
		case SQL_LONGVARCHAR:
		case SQL_BINARY:
		case SQL_VARBINARY:
		case SQL_LONGVARBINARY:
		case SQL_BIGINT:
		case SQL_GUID:
		{
			auto& Value = Field.var.emplace<CStringSQL>({});
			Value.resize(ColumnSize);
			CALL(SQLBindCol, Statement, i, SQL_C_CHAR, &Value[0], Value.size() + 1,
				&Field.LenOrInd);
			break;
		}
		case SQL_INTEGER:
		case SQL_SMALLINT:
		case SQL_TINYINT:
		case SQL_BIT:
		{
			CALL(SQLBindCol, Statement, i, SQL_C_LONG, &Field.var.emplace<long>({}), 0,
				&Field.LenOrInd);
			break;
		}
		case SQL_FLOAT:
		case SQL_REAL:
		case SQL_DOUBLE:
		{
			CALL(SQLBindCol, Statement, i, SQL_C_DOUBLE, &Field.var.emplace<double>({}), 0,
				&Field.LenOrInd);
			break;
		}
		case SQL_UNKNOWN_TYPE:
		default:
			assert(false);
			ThrowCDBException("Unknown type");
		}
	}
}

void CODBCRecordset::MoveNext()
{
	if (End)
	{
		assert(false);
		return;
	}

	auto ret = CALL(SQLFetch, Statement);
	if (ret == SQL_NO_DATA)
	{
		if (CALL(SQLMoreResults, Statement) == SQL_NO_DATA)
			End = true;
		else
			Bind();
	}
	else
	{
		++RecordCount;
	}
}

CDBField& CODBCRecordset::Field(const char* szName) {
	auto it = std::find_if(Fields.begin(), Fields.end(), [&](auto&& x) {
		return x.Name == szName;
		});
	if (it == Fields.end())
	{
		static CDBField NullField{ {0l}, "", SQL_NULL_DATA };
		return NullField;
	}
	return *it;
}

CDBBinary CODBCRecordset::SelectBinary(const CStringSQL& strQuery)
{
	CDBBinary DBBinary(5, 5);

	DBBinary.Clear();

	DBBinary.SetCurUsedSize(-1);

	if (0 == strQuery.GetLength())
	{
		return DBBinary;
	}

	HSTMT			hStmt;
	unsigned char	Data[BINARY_FIELD_MAX_SIZE] = { 0 };
	SQLLEN		ind = SQL_DATA_AT_EXEC;

	if (SQL_SUCCESS != SQLAllocHandle(SQL_HANDLE_STMT, m_pDatabase->GetConn(), &hStmt))
	{
		return DBBinary;
	}

	SQLPrepare(hStmt, SQLData(strQuery), SQL_NTS);

	if (SQL_ERROR == SQLBindParameter(hStmt, 1, SQL_PARAM_OUTPUT, SQL_C_BINARY, SQL_BINARY, BINARY_FIELD_MAX_SIZE,
		0, (SQLPOINTER)Data, BINARY_FIELD_MAX_SIZE, &ind))
	{
		return DBBinary;
	}

	if (SQL_ERROR == SQLExecute(hStmt))
	{
		return DBBinary;
	}

	SQLLEN sqlLen;

	DBBinary.Begin();

	while (SQL_SUCCESS == SQLFetch(hStmt))
	{
		if (!m_pDatabase->IsOpen())
		{
			DBBinary.SetCurUsedSize(-1);
			return DBBinary;
		}

		if (SQL_ERROR == SQLGetData(hStmt, 1, SQL_BINARY, (SQLPOINTER)Data, BINARY_FIELD_MAX_SIZE, &sqlLen))
		{
			DBBinary.SetCurUsedSize(-1);
			return DBBinary;
		}

		if (-1 == sqlLen)
		{
			if (0 != DBBinary.InsertData(Data, 0))
			{
				DBBinary.SetCurUsedSize(-1);
				return DBBinary;
			}
			continue;
		}

		if (SQL_ERROR == SQLGetData(hStmt, 1, SQL_C_DEFAULT, (SQLPOINTER)Data, sqlLen, &ind))
		{
			DBBinary.SetCurUsedSize(-1);
			return DBBinary;
		}

		if (sqlLen != DBBinary.InsertData(Data, sqlLen))
		{
			DBBinary.SetCurUsedSize(-1);
			return DBBinary;
		}
	}

	SQLFreeHandle(SQL_HANDLE_STMT, hStmt);

	return DBBinary;
}

bool CODBCRecordset::InsertBinary(const CStringSQL& strQuery, const u8* pData, int nSize)
{
	if (!m_pDatabase->IsOpen())
	{
		assert(0);
		return false;
	}

	if ((0 == strQuery.GetLength()) || 0 == pData)
		return false;

	if (8000 < nSize)
		return false;

	HSTMT		hStmt;
	SQLTCHAR* pWriteBuff;
	SQLLEN  ind = SQL_DATA_AT_EXEC;

	if (SQL_ERROR == SQLAllocHandle(SQL_HANDLE_STMT, m_pDatabase->GetConn(), &hStmt))
	{
		return false;
	}

	if (SQL_ERROR == SQLPrepare(hStmt, SQLData(strQuery), SQL_NTS))
	{
		return false;
	}

	if (SQL_ERROR == SQLBindParameter(hStmt, 1, SQL_PARAM_INPUT,
		SQL_C_BINARY, SQL_BINARY,
		nSize, 0, (SQLPOINTER)pData, nSize, &ind))
	{
		return false;
	}

	if (SQL_ERROR == SQLExecute(hStmt))
	{
		return false;
	}

	if (SQL_ERROR == SQLParamData(hStmt, (SQLPOINTER*)&pWriteBuff))
	{
		return false;
	}

	int			nWrSize;
	int			len;
	SQLRETURN	sqlRet;

	for (nWrSize = 0; nWrSize < nSize; nWrSize += BINARY_CHUNK_SIZE, pWriteBuff += BINARY_CHUNK_SIZE)
	{
		if (nWrSize + BINARY_CHUNK_SIZE < nSize)
			len = BINARY_CHUNK_SIZE;
		else
			len = nSize - nWrSize;

		sqlRet = SQLPutData(hStmt, (SQLPOINTER)pWriteBuff, len);
	}

	if (SQL_ERROR == sqlRet)
	{
		return false;
	}

	if (SQL_ERROR == SQLParamData(hStmt, (SQLPOINTER*)&pWriteBuff))
	{
		return false;
	}

	SQLFreeHandle(SQL_HANDLE_STMT, hStmt);

	return true;
}

#undef V