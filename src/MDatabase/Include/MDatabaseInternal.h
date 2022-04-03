#pragma once
#include <string>
#include "GlobalTypes.h"
#include "SafeString.h"
#include "MDatabase.h"
#include "ODBCRecordset.h"
#include "MDebug.h"
#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#endif
#include <sql.h>
#include <sqlext.h>

static_assert(SQLHandleTypes::Env == SQL_HANDLE_ENV, "Wrong SQL handle type");
static_assert(SQLHandleTypes::DBC == SQL_HANDLE_DBC, "Wrong SQL handle type");
static_assert(SQLHandleTypes::Stmt == SQL_HANDLE_STMT, "Wrong SQL handle type");

template <typename T>
auto SQLData(const T& str) {
	return const_cast<SQLCHAR*>(reinterpret_cast<const SQLCHAR*>(str.data()));
}

template <typename T>
auto SQLSize(const T& str) {
	assert(str.size() <= size_t((std::numeric_limits<SQLSMALLINT>::max)()));
	return static_cast<SQLSMALLINT>(str.size());
}

template <typename T, size_t N>
auto SQLData(const T(&str)[N]) {
	return const_cast<SQLCHAR*>(reinterpret_cast<const SQLCHAR*>(str));
}

template <typename T, size_t N>
auto SQLSize(const T(&str)[N]) {
	assert(N <= size_t((std::numeric_limits<SQLSMALLINT>::max)()));
	return static_cast<SQLSMALLINT>(N);
}

const struct {
	template <SQLSMALLINT HandleType>
	SQLRETURN operator()(SQLHandle<HandleType>& Handle, SQLHANDLE Input) const
	{
		return SQLAllocHandle(HandleType, Input, MakeWriteProxy(Handle.Handle));
	}
} SQLAlloc{};

inline void ThrowCDBException(std::string Error)
{
	thread_local std::string ErrorMemory;
	thread_local CDBException Exception;
	ErrorMemory = std::move(Error);
	Exception.m_strError = ErrorMemory.c_str();
	throw& Exception;
}

inline std::string GetErrorMessage(const char* EnclosingFuncName, const char* File, int Line,
	const char* SQLFuncName, SQLHANDLE Handle, SQLSMALLINT HandleType)
{
	SQLINTEGER Error;
	char Message[1024];
	char State[SQL_SQLSTATE_SIZE + 1];

	auto GetDiagRec = [&](SQLSMALLINT i) {
		return SQLGetDiagRec(HandleType, Handle, i, SQLData(State), &Error,
			SQLData(Message), SQLSize(Message), nullptr);
	};

	auto ErrorMessage = strprintf("SQL function %s in function %s in file %s:%d failed.",
		SQLFuncName, EnclosingFuncName, File, Line);
	for (SQLSMALLINT i = 1; GetDiagRec(i) == SQL_SUCCESS; ++i)
	{
		auto Prefix = i == 1 ? "\nDiagnostic records:\n" : "\n";
		strprintf_append(ErrorMessage, "%s%d: [%5.5s] %s (%d)",
			Prefix, i, State, Message, Error);
	}
	return ErrorMessage;
}

inline void HandleError(const char* EnclosingFuncName, const char* File, int Line,
	const char* SQLFuncName, SQLHANDLE Handle, SQLSMALLINT HandleType)
{
	ThrowCDBException(GetErrorMessage(EnclosingFuncName, File, Line, SQLFuncName, Handle, HandleType));
}

template <typename E, typename F, typename H, typename... Args>
SQLRETURN SQLCall(const char* EnclosingFuncName, const char* File, int Line,
	const char* SQLFuncName, E&& OnError, F Func, H& Handle, Args&&... args)
{
	auto ret = Func(Handle, args...);
	if (ret == SQL_ERROR)
		OnError(EnclosingFuncName, File, Line, SQLFuncName, Handle, Handle.HandleType);
	return ret;
}

#ifdef _MSC_VER
#define PRETTY_FUNCTION __FUNCSIG__
#else
#define PRETTY_FUNCTION __PRETTY_FUNCTION__
#endif

#define CALL(func, ...) SQLCall(PRETTY_FUNCTION, __FILE__, __LINE__, #func, HandleError, func, __VA_ARGS__)
