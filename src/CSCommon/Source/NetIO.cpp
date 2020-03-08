#include "NetIO.h"

#ifdef _WIN32
bool NetIO::Create(int Port, CallbackType Callback, bool Reuse)
{
	this->Callback = Callback;
	bool Ret = RealCPNet.Create(Port, Reuse);
	RealCPNet.SetCallback([](void* Context, RCP_IO_OPERATION Op, u32 Key,
		MPacketHeader* Packet, u32 Size) {
		auto& Me = *static_cast<NetIO*>(Context);
		auto CvtOp = [&] {
			switch (Op)
			{
			case RCP_IO_NONE:
				return IOOperation::None;
			case RCP_IO_ACCEPT:
				return IOOperation::Accept;
			case RCP_IO_CONNECT:
				return IOOperation::Connect;
			case RCP_IO_DISCONNECT:
				return IOOperation::Disconnect;
			case RCP_IO_READ:
				return IOOperation::Read;
			case RCP_IO_WRITE:
				return IOOperation::Write;
			}
			assert(false);
			return IOOperation::None;
		}();
		auto Handle = static_cast<SOCKET>(Key);
		void* Data;
		AcceptData AData;
		ReadData RData;
		if (CvtOp == IOOperation::Accept)
		{
			char AddressString[64];
			int Port;
			Me.RealCPNet.GetAddress(Handle, AddressString, &Port);
			auto Address = GetIPv4Number(AddressString);
			AData = {Address, u16(Port)};
			Data = &AData;
		}
		else if (CvtOp == IOOperation::Read)
		{
			RData = {{reinterpret_cast<u8*>(Packet), Size}};
			Data = &RData;
		}
		else
		{
			Data = nullptr;
		}
		Me.Callback(CvtOp, Handle, Data);
	}, this);
	return Ret;
}

void NetIO::Destroy()
{
	RealCPNet.Destroy();
}

NetIO::ConnectionHandle NetIO::Connect(u32 Address, int Port, void* Context)
{
	ConnectionHandle Ret;
	char AddressString[64];
	GetIPv4String(Address, AddressString);
	if (RealCPNet.Connect(&Ret, AddressString, Port))
	{
		RealCPNet.SetUserContext(Ret, Context);
	}
	return Ret;
}

void NetIO::Disconnect(ConnectionHandle Handle)
{
	RealCPNet.Disconnect(Handle);
}

bool NetIO::Send(ConnectionHandle Handle, void* Packet, int Size)
{
	return RealCPNet.Send(Handle, static_cast<MPacketHeader*>(Packet), Size);
}

void* NetIO::GetContext(ConnectionHandle Handle)
{
	return RealCPNet.GetUserContext(Handle);
}

void NetIO::SetContext(ConnectionHandle Handle, void* Context)
{
	RealCPNet.SetUserContext(Handle, Context);
}

void NetIO::SetLogCallback(LogCallbackType Callback)
{
	SetupRCPLog(Callback);
}

void NetIO::SetLogLevel(int i)
{
	RealCPNet.SetLogLevel(i);
}
#else
using asio::ip::tcp;

static auto& GetConn(NetIO::ConnectionHandle Handle)
{
	return *reinterpret_cast<NetIO::Connection*>(Handle);
}

static auto GetHandle(const std::shared_ptr<NetIO::Connection>& Ptr)
{
	return reinterpret_cast<NetIO::ConnectionHandle>(Ptr.get());
}

void NetIO::Read(std::shared_ptr<Connection> Conn)
{
	Conn->Strand.dispatch([this, Conn] {
		Conn->Socket.async_receive(asio::buffer(Conn->ReadBuffer), Conn->Strand.wrap(
		[this, Conn](std::error_code ec, size_t size) {
			if (ec)
			{
				Disconnect(GetHandle(Conn));
				return;
			}
			ReadData Data{{Conn->ReadBuffer.data(), size}};
			Callback(IOOperation::Read, GetHandle(Conn), &Data);
			Read(Conn);
		}));
	});
}

void NetIO::Accept()
{
	Acceptor.async_accept(AcceptSocket, [this](std::error_code ec) {
		if (!ec)
		{
			auto Endpoint = AcceptSocket.remote_endpoint();
			AcceptData Data{u32(Endpoint.address().to_v4().to_ulong()), Endpoint.port()};
			{
				std::lock_guard<std::mutex> lock(ConnectionsMutex);
				Connections.push_back(std::make_shared<Connection>(IOContext, std::move(AcceptSocket)));;
				auto& Conn = Connections.back();
				Callback(IOOperation::Accept, GetHandle(Conn), &Data);
				Read(Conn);
			}
		}
		Accept();
	});
}

bool NetIO::Create(int Port, CallbackType Callback, bool Reuse)
{
	Stopped = false;
	this->Callback = Callback;

	auto ThreadProc = [this] {
		while (!Stopped.load(std::memory_order_relaxed))
			IOContext.run();
	};
	for (size_t i = 0; i < 1; ++i)
		std::thread{ThreadProc}.detach();

	tcp::endpoint LocalEndpoint{tcp::v4(), u16(Port)};
	Acceptor = tcp::acceptor(IOContext, LocalEndpoint, Reuse);
	Accept();
	return true;
}

void NetIO::Destroy()
{
	Stopped = true;
	IOContext.stop();
}

NetIO::ConnectionHandle NetIO::Connect(u32 Address, int Port, void* Context)
{
	assert(false);
	abort();
}

void NetIO::Disconnect(ConnectionHandle Handle)
{
	std::lock_guard<std::mutex> lock(ConnectionsMutex);
	auto it = std::find_if(Connections.begin(), Connections.end(), [Conn = &GetConn(Handle)](auto& x) {
		return x.get() == Conn;
	});
	if (it == Connections.end())
		return;
	auto Conn = *it;
	if (Conn->Socket.is_open())
		Conn->Socket.shutdown(tcp::socket::shutdown_both);
	Conn->Socket.close();
	Callback(IOOperation::Disconnect, GetHandle(Conn), nullptr);
	Connections.erase(it);
}

bool NetIO::Send(ConnectionHandle Handle, void* Packet, int Size)
{
	auto Conn = GetConn(Handle).shared_from_this();
	Conn->Strand.dispatch([this, Conn, Packet, Size] {
		asio::async_write(Conn->Socket, asio::buffer(Packet, Size), Conn->Strand.wrap(
		[this, Conn, Packet](std::error_code ec, size_t) {
			if (!ec)
			{
				Callback(IOOperation::Write, GetHandle(Conn), nullptr);
			}
			free(Packet);
		}));
		return true;
	});
}

void* NetIO::GetContext(ConnectionHandle Handle)
{
	return GetConn(Handle).Context;
}

void NetIO::SetContext(ConnectionHandle Handle, void* Context)
{
	GetConn(Handle).Context = Context;
}

void NetIO::SetLogCallback(LogCallbackType){}
void NetIO::SetLogLevel(int){}

#endif
