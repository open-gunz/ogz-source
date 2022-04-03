#include "MDatabaseInternal.h"
#include "MUtil.h"

#define CALL_NOTHROW(func, ...) do {\
		if (SQLCall(PRETTY_FUNCTION, __FILE__, __LINE__, #func,\
			[this](auto&... a) { this->WriteLog(GetErrorMessage(a...) + "\n"); },\
			func, __VA_ARGS__) == SQL_ERROR)\
		{\
			return false;\
		}\
	} while (false);

template <SQLSMALLINT HandleType>
void SQLHandle<HandleType>::Deleter::operator()(SQLHANDLE Handle) const
{
	if (!Handle)
		return;
	auto a = [](SQLRETURN r) { assert(r != SQL_ERROR); };
	if (HandleType == SQL_HANDLE_DBC)
		a(SQLDisconnect(Handle));
	else if (HandleType == SQL_HANDLE_STMT)
		a(SQLFreeStmt(Handle, SQL_CLOSE));
	a(SQLFreeHandle(HandleType, Handle));
}

static HWND GetHwnd()
{
#ifdef _WIN32
	return GetConsoleWindow();
#else
	return nullptr;
#endif
}

bool MDatabase::CheckOpen()
{
	if (!IsOpen())
	{
		SQLSMALLINT Len;
		CALL_NOTHROW(SQLDriverConnect, Conn, GetHwnd(), SQLData(ConnectString), SQLSize(ConnectString),
			nullptr, 0, &Len, SQL_DRIVER_COMPLETE);
	}

	return true;
}

static void append(ArrayView<char>& Dest, const char* fmt, ...)
{
	va_list va;
	va_start(va, fmt);
	auto ret = vsprintf_safe(Dest, fmt, va);
	Dest.remove_prefix(ret);
	va_end(va);
};

bool MDatabase::Connect(const ConnectionDetails& cd)
{
	char InitialConnectString[1024];
	ArrayView<char> buf = InitialConnectString;
	if (cd.Driver == DBDriver::SQLServer)
	{
		append(buf, "DRIVER={SQL Server};SERVER=%.*s;DATABASE=%.*s;",
			cd.Server.size(), cd.Server.data(),
			cd.Database.size(), cd.Database.data());
	}
	else if (cd.Driver == DBDriver::ODBC)
	{
		append(buf, "DSN=%.*s;", cd.DSN.size(), cd.DSN.data());
	}

	if (cd.Auth == DBAuth::SQLServer)
	{
		append(buf, "UID=%.*s;PWD=%.*s",
			cd.Username.size(), cd.Username.data(),
			cd.Password.size(), cd.Password.data());
	}
	else if (cd.Auth == DBAuth::Windows)
	{
		append(buf, "Trusted_Connection=yes");
	}

	CALL_NOTHROW(SQLAlloc, Env, static_cast<SQLHANDLE>(SQL_NULL_HANDLE));
	CALL_NOTHROW(SQLSetEnvAttr, Env, SQL_ATTR_ODBC_VERSION, reinterpret_cast<SQLPOINTER*>(SQL_OV_ODBC3), 0);
	CALL_NOTHROW(SQLAlloc, Conn, Env);
	ConnectString.resize(1024);
	SQLSMALLINT ConnStrLen;

	auto TryConnect = [&](SQLHandle<SQLHandleTypes::DBC>& Handle) {
		return SQLDriverConnect(Handle, GetHwnd(),
			SQLData(InitialConnectString), SQLSize(InitialConnectString) - buf.size(),
			SQLData(ConnectString), SQLSize(ConnectString),
			&ConnStrLen, SQL_DRIVER_COMPLETE);
	};

	CALL_NOTHROW(TryConnect, Conn);
	if (ConnStrLen > SQLSize(ConnectString))
	{
		ConnectString.resize(ConnStrLen);
		CALL_NOTHROW(TryConnect, Conn);
		assert(ConnStrLen == SQLSize(ConnectString));
	}
	else
	{
		ConnectString.resize(ConnStrLen);
		ConnectString.shrink_to_fit();
	}
	return true;
}

void MDatabase::Disconnect() { Conn.Handle.reset(); }

bool MDatabase::IsOpen() const
{
	SQLULEN Value = 0;
	CALL_NOTHROW(SQLGetConnectAttr, Conn, SQL_ATTR_CONNECTION_DEAD, &Value, 0, nullptr);
	return Value == 0;
}

void MDatabase::ExecuteSQL(StringView SQL)
{
	SQLHandle<SQL_HANDLE_STMT> Statement;
	CALL(SQLAlloc, Statement, Conn);
	CALL(SQLExecDirect, Statement, SQLData(SQL), SQLSize(SQL));
}

#undef CALL_NOTHROW