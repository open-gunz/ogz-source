#pragma once
#include "GlobalTypes.h"
#include "optional.h"
#include "function_view.h"
#ifndef _WIN32
#define USE_ASIO 1
#endif
#ifndef USE_ASIO
#include "RealCPNet.h"
#else
#include <vector>
#include <thread>
#include <array>
#define ASIO_STANDALONE
#include "asio.hpp"
#ifdef R_OK
#undef R_OK
#endif
namespace asio {
using io_context = io_service;
}
#endif

struct NetIO
{
	struct Connection
#ifdef USE_ASIO
		: std::enable_shared_from_this<Connection>
#endif
	{
#ifndef USE_ASIO
		SOCKET Socket;
#else	
		Connection(asio::io_context& IOContext, asio::ip::tcp::socket Socket, void* Context = nullptr)
			: Socket(std::move(Socket)), Strand(IOContext), Context(Context) {}
		asio::ip::tcp::socket Socket;
		asio::io_context::strand Strand;
		void* Context;
		std::array<u8, 8192> ReadBuffer;
#endif
	};

#ifndef USE_ASIO
	using ConnectionHandle = SOCKET;
#else
	using ConnectionHandle = uintptr_t;
#endif

	enum class IOOperation
	{
		None,
		Accept,
		Connect,
		Disconnect,
		Read,
		Write,
	};

	struct AcceptData
	{
		u32 Address;
		u16 Port;
	};

	struct ReadData
	{
		ArrayView<u8> Data;
	};

	using CallbackType = function_view<void(IOOperation, ConnectionHandle, const void*)>;
	using LogCallbackType = void(const char*, ...);

	bool Create(int Port, CallbackType Callback, bool Reuse = false);
	void Destroy();

	ConnectionHandle Connect(u32 Address, int Port, void* Context);
	void Disconnect(ConnectionHandle Handle);

	bool Send(ConnectionHandle Handle, void* Packet, int Size);

	void* GetContext(ConnectionHandle Handle);
	void SetContext(ConnectionHandle Handle, void* Context);

	void SetLogCallback(LogCallbackType Callback);
	void SetLogLevel(int Level);

private:
	CallbackType Callback;
#ifndef USE_ASIO
	MRealCPNet RealCPNet;
#else
	void Accept();
	void Read(std::shared_ptr<Connection> Conn);
	
	asio::io_context IOContext;
	asio::ip::tcp::socket ListenSocket{IOContext};
	asio::ip::tcp::acceptor Acceptor{IOContext};
	asio::ip::tcp::socket AcceptSocket{IOContext};
	std::vector<std::shared_ptr<Connection>> Connections;
	std::mutex ConnectionsMutex;
	std::atomic<bool> Stopped{false};

	template <typename... Args>
	void Log(Args... args)
	{
		if (LogLevel)
			LogCallback(args...);
	}

	LogCallbackType LogCallback;
	int LogLevel = 0;
#endif
};
