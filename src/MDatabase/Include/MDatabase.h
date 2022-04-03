#pragma once

#include <cassert>
#include <string>
#include <vector>
#include <exception>
#include "variant.h"
#include "SafeString.h"
#include "StringView.h"
#include "MWindowsFwd.h"
#include <sqltypes.h>

#ifdef _WIN32
#pragma comment(lib, "odbc32.lib")
#endif

struct CStringSQL : std::string
{
	using std::string::string;
	using std::string::operator=;
	CStringSQL() = default;
	CStringSQL(const CStringSQL&) = default;
	CStringSQL(CStringSQL&&) = default;
	CStringSQL& operator=(const CStringSQL&) = default;
	CStringSQL& operator=(CStringSQL&&) = default;
	CStringSQL(std::string str) : std::string{ std::move(str) } {}
	explicit operator const char* () const { return c_str(); }
	char* GetBuffer() { return &(*this)[0]; }
	size_t GetLength() const { return length(); }
	void Format(const char* fmt, ...)
	{
		va_list va;
		va_start(va, fmt);
		*this = vstrprintf(fmt, va);
		va_end(va);
	}
};

struct CException
{
	void GetErrorMessage(...) const {}
};

struct CDBException
{
	const char* m_strError;
};

namespace SQLHandleTypes
{
	enum Type : SQLSMALLINT
	{
		Env = 1,
		DBC,
		Stmt,
	};
}

template <SQLSMALLINT HandleTypeParam>
struct SQLHandle
{
	static constexpr SQLSMALLINT HandleType = HandleTypeParam;
	operator SQLHANDLE() const { return Handle.get(); }
	struct Deleter { void operator()(SQLHANDLE Handle) const; };
	std::unique_ptr<void, Deleter> Handle;
};

template <SQLSMALLINT HandleTypeParam>
SQLSMALLINT const SQLHandle<HandleTypeParam>::HandleType;

struct MDatabase
{
	enum class DBDriver { ODBC, SQLServer, Max };
	enum class DBAuth { SQLServer, Windows, Max };

	struct ConnectionDetails
	{
		DBDriver Driver;
		DBAuth Auth;
		// These are only used if Driver is SQLServer.
		StringView Server;
		StringView Database;
		// This is only used is Driver is ODBC.
		StringView DSN;
		// These are only used if Auth is SQLServer.
		StringView Username;
		StringView Password;
	};

	typedef void(LOGCALLBACK)(const std::string& strLog);

	MDatabase() = default;
	MDatabase(const MDatabase&) = delete;
	MDatabase& operator=(const MDatabase&) = delete;

	bool CheckOpen();
	bool Connect(const ConnectionDetails& cd);
	void Disconnect();
	bool IsOpen() const;
	void ExecuteSQL(StringView SQL);
	void SetLogCallback(LOGCALLBACK* fnLogCallback) { m_fnLogCallback = fnLogCallback; }
	SQLHANDLE GetConn() const { return Conn; }

private:
	void WriteLog(const std::string& strLog) const { if (m_fnLogCallback) m_fnLogCallback(strLog); }

	SQLHandle<SQLHandleTypes::Env> Env;
	SQLHandle<SQLHandleTypes::DBC> Conn;
	CStringSQL ConnectString;
	LOGCALLBACK* m_fnLogCallback = nullptr;
};

inline const char* ToString(MDatabase::DBDriver Driver)
{
	switch (Driver)
	{
	case MDatabase::DBDriver::ODBC: return "ODBC";
	case MDatabase::DBDriver::SQLServer: return "SQLServer";
	}
	assert(false);
	return nullptr;
}

inline const char* ToString(MDatabase::DBAuth Auth)
{
	switch (Auth)
	{
	case MDatabase::DBAuth::SQLServer: return "SQLServer";
	case MDatabase::DBAuth::Windows: return "Windows";
	}
	assert(false);
	return nullptr;
}