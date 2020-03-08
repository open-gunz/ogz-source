#include "stdafx.h"

#ifdef WIN32
#include <WinSock2.h>
#include <WS2tcpip.h>
#undef CreateEvent
#else
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <arpa/inet.h>
#include <netdb.h>
#endif

#include "MSocket.h"
#include "MSync.h"
#include "MDebug.h"
#include "MInetUtil.h"
#include "StringView.h"
#include "MUtil.h"

#ifndef _MSC_VER
void _set_errno(int errnum)
{
	errno = errnum;
}
#endif

namespace MSocket
{

#ifdef WIN32

static bool Initialized;

bool Startup()
{
	if (Initialized)
		return true;

	const u16 ExpectedVersion = 0x0202;

	WSADATA	wsaData;
	auto StartupRet = WSAStartup(ExpectedVersion, &wsaData);
	if (StartupRet != 0)
	{
		MLog("MSocket::Startup -- WSAStartup failed with error code %d\n",
			StartupRet);

		return false;
	}

	const u16 ActualVersion = wsaData.wVersion;

	if (ExpectedVersion != ActualVersion)
	{
		MLog("MSocket::Startup -- WinSock version invalid\n"
			"Expected %04X; got %04X\n",
			ExpectedVersion, ActualVersion);

		return false;
	}

	Initialized = true;

	return true;
}

void Cleanup()
{
	if (!Initialized)
		return;

	WSACleanup();

	Initialized = false;
}

MSignalEvent CreateEvent()
{
	return MSignalEvent{ true };
}

int EventSelect(SOCKET Socket, MSignalEvent& EventHandle, int NetworkEvents)
{
	return WSAEventSelect(Socket, EventHandle.GetEventHandle(), NetworkEvents);
}

int EnumNetworkEvents(SOCKET Socket, MSignalEvent& EventHandle, NetworkEvents* Out)
{
	WSANETWORKEVENTS wsane{};
	auto ret = WSAEnumNetworkEvents(Socket, EventHandle.GetEventHandle(), &wsane);
	if (ret != 0)
		return ret;

	Out->NetworkEventsSet = wsane.lNetworkEvents;
	for (size_t i = 0; i < std::size(Out->ErrorCode); ++i)
	{
		Out->ErrorCode[i] = wsane.iErrorCode[i];
	}

	return ret;
}

bool IsNetworkEventSet(const NetworkEvents & Events, int Flag)
{
	return (Events.NetworkEventsSet & Flag) == Flag;
}

int GetNetworkEventValue(const NetworkEvents & Events, int Flag)
{
	return Events.ErrorCode[bsr(Flag)];
}

u32 GetLastError()
{
	return ::GetLastError();
}

bool GetErrorString(u32 ErrorCode, char* Output, size_t OutputSize)
{
	auto ret = FormatMessageA(FORMAT_MESSAGE_FROM_SYSTEM, nullptr, ErrorCode,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), Output, OutputSize, nullptr);
	return ret != 0;
}

bool GetErrorString(u32 ErrorCode, std::string& Output)
{
	char Buffer[512]; Buffer[0] = 0;
	auto ret = GetErrorString(ErrorCode, Buffer);
	if (!ret)
		return false;
	Output = Buffer;
	return true;
}

void LogError(const char* CallerName, const char* FunctionName, int ErrorCode)
{
	const auto LastErrorCode = MSocket::GetLastError();

	char LastErrorString[512]; LastErrorString[0] = 0;
	MSocket::GetErrorString(LastErrorCode, LastErrorString);

	MLog("%s -- %s failed with error code %d\n"
		"GetLastError() = %d, LastErrorString = %s\n",
		CallerName, FunctionName, ErrorCode,
		LastErrorCode, LastErrorString);
}

#else

static bool Initialized;

bool Startup()
{
	return true;
}

void Cleanup()
{
}

MSignalEvent CreateEvent()
{
	return MSignalEvent{ true };
}

int EventSelect(SOCKET Socket, MSignalEvent& EventHandle, int NetworkEvents)
{
	close(EventHandle.EventHandle);
	EventHandle.EventHandle = Socket;
	int flags = fcntl(Socket, F_GETFL, 0);
	fcntl(Socket, F_SETFL, flags | O_NONBLOCK);
	auto& e = EventHandle.Events;
	e = 0;
	if (NetworkEvents & FD::CONNECT)
		e |= MSignalEvent::Connect;
	if (NetworkEvents & FD::READ)
		e |= MSignalEvent::Read;
	if (NetworkEvents & FD::WRITE)
		e |= MSignalEvent::Write;
	if (NetworkEvents & FD::CLOSE)
		e |= MSignalEvent::Close;
	return 0;
}

int EnumNetworkEvents(SOCKET Socket, MSignalEvent& EventHandle, NetworkEvents* Out)
{
	Out->NetworkEventsSet = 0;
	if (EventHandle.SetEvents & MSignalEvent::Read)
	{
		auto IsClosed = [&] {
			int n = 0;
			ioctl(Socket, FIONREAD, &n);
			return n == 0;
		};
		if (EventHandle.Events & MSignalEvent::Close && IsClosed())
		{
			Out->NetworkEventsSet |= FD::CLOSE;
		}
		else
		{
			Out->NetworkEventsSet |= FD::READ;
			if (EventHandle.Events & MSignalEvent::Connect)
			{
				Out->NetworkEventsSet |= FD::CONNECT;
				EventHandle.Events &= ~MSignalEvent::Connect;
			}
		}
	}
	if (EventHandle.SetEvents & MSignalEvent::Write)
	{
		Out->NetworkEventsSet |= FD::WRITE;
		if (EventHandle.Events & MSignalEvent::Connect)
		{
			Out->NetworkEventsSet |= FD::CONNECT;
			EventHandle.Events &= ~MSignalEvent::Connect;
		}
	}
	for (auto& e : Out->ErrorCode)
		e = 0;
	return 0;
}

bool IsNetworkEventSet(const NetworkEvents & Events, int Flag)
{
	return (Events.NetworkEventsSet & Flag) == Flag;
}

int GetNetworkEventValue(const NetworkEvents & Events, int Flag)
{
	return Events.ErrorCode[bsr(Flag)];
}

u32 GetLastError()
{
	return u32(errno);
}

bool GetErrorString(u32 ErrorCode, char* Output, size_t OutputSize)
{
	return strerror_safe(int(ErrorCode), Output, OutputSize) != 0;
}

bool GetErrorString(u32 ErrorCode, std::string& Output)
{
	char Buffer[512]; Buffer[0] = 0;
	auto ret = GetErrorString(ErrorCode, Buffer);
	if (!ret)
		return false;
	Output = Buffer;
	return true;
}

void LogError(const char* CallerName, const char* FunctionName, int ErrorCode)
{
	const auto LastErrorCode = MSocket::GetLastError();

	char LastErrorString[512]; LastErrorString[0] = 0;
	MSocket::GetErrorString(LastErrorCode, LastErrorString);

	MLog("%s -- %s failed with error code %d\n"
		"GetLastError() = %d, LastErrorString = %s\n",
		CallerName, FunctionName, ErrorCode,
		LastErrorCode, LastErrorString);
}

#endif

in_addr::in_addr(const ::in_addr& src)
{
	memcpy(this, &src, sizeof(in_addr));
}

in_addr::operator const ::in_addr&() const
{
	return *reinterpret_cast<const ::in_addr*>(this);
}

// POSIX socket API

namespace detail {
// Stuff to convert between WinSock parameter types and POSIX ones to avoid repeating a bunch of casts.
template <typename To, typename From>
static To convert1(From x) { return x; }
template <>
::sockaddr* convert1(MSocket::sockaddr* x) { return reinterpret_cast<::sockaddr*>(x); }
static_assert(sizeof(::sockaddr) == sizeof(MSocket::sockaddr), "Wrong sockaddr_t size");
#ifndef _MSC_VER
template <>
socklen_t* convert1(int* x) { return reinterpret_cast<socklen_t*>(x); }
static_assert(sizeof(socklen_t) == sizeof(int), "Wrong socklen_t size");
#define CALL_API
#else
#define CALL_API __stdcall
#endif

template <typename Ret, typename... FuncArgs, typename... Args>
static Ret call(Ret (CALL_API *func)(FuncArgs...), Args... args) {
	return func(convert1<FuncArgs, Args>(args)...);
}
}

SOCKET MSOCKET_CALL accept(
	SOCKET s,
	struct sockaddr *addr,
	int *addrlen) {
	return detail::call(::accept, s, addr, addrlen);
}

int MSOCKET_CALL bind(
	SOCKET s,
	const struct sockaddr *addr,
	int namelen) {
	return ::bind(s, reinterpret_cast<const ::sockaddr*>(addr), namelen);
}

int MSOCKET_CALL closesocket(SOCKET s) {
#ifdef _MSC_VER
	return ::closesocket(s);
#else
	return ::close(s);
#endif
}

int MSOCKET_CALL connect(
	SOCKET s,
	const struct sockaddr *name,
	int namelen) {
	return ::connect(s, reinterpret_cast<const ::sockaddr*>(name), namelen);
}

int MSOCKET_CALL ioctlsocket(
	SOCKET s,
	long cmd,
	u32 *argp) {
#ifdef _MSC_VER
	return ::ioctlsocket(s, cmd, (unsigned long*)argp);
#else
	return ::ioctl(s, cmd, argp);
#endif
}

int MSOCKET_CALL getpeername(
	SOCKET s,
	struct sockaddr *name,
	int * namelen) {
	return detail::call(::getpeername, s, name, namelen);
}

int MSOCKET_CALL getsockname(
	SOCKET s,
	struct sockaddr *name,
	int * namelen) {
	return detail::call(::getsockname, s, name, namelen);
}

int MSOCKET_CALL getsockopt(
	SOCKET s,
	int level,
	int optname,
	char * optval,
	int *optlen) {
	return detail::call(::getsockopt, s, level, optname, optval, optlen);
}

u32 MSOCKET_CALL htonl(u32 hostlong) {
	return ::htonl(hostlong);
}

u16 MSOCKET_CALL htons(u16 hostshort) {
	return ::htons(hostshort);
}

/*unsigned long MSOCKET_CALL inet_addr(const char * cp) {
	return ::inet_addr(cp);
}

char * MSOCKET_CALL inet_ntoa(struct in_addr in) {
	return ::inet_ntoa(in);
}*/

static int inet_pton4(int af, const char* src, in_addr& dst)
{
	auto ip = GetIPv4Number(src);
	if (ip == MSocket::in_addr::None)
		return 0;

	dst.s_addr = ip;
	return 1;
}

static int inet_pton6(int af, const char* src, in6_addr& dst)
{
	static const StringView Separators = ".:";

	int BytesFilled = 0;

	bool FoundDoubleColon = false;
	int DoubleColonByteIndex = 0;

	bool FoundDot = false;
	int NumDots = 0;

	auto SrcView = StringView{ src };
	while (true)
	{
		auto CurSeparatorPos = SrcView.find_first_of(Separators);

		bool IsHex;
		if (CurSeparatorPos == SrcView.npos) {
			IsHex = !(BytesFilled == 15 && NumDots == 3);
		} else if (SrcView[CurSeparatorPos] == ':') {
			IsHex = true;
		} else if (SrcView[CurSeparatorPos] == '.') {
			IsHex = false;
		} else {
			return 0;
		}

		// Grab the substring from the current digit to before the separator.
		// In this example string: 2001:0db8:0000:0042:0000:8a2e:0370:7334,
		// substring would be "2001".
		auto Substring = SrcView.substr(0, CurSeparatorPos);

		if (Substring[0] == ':')
		{
			// The current separator is a double colon, which means
			// that the beginning bytes are implicitly zeroes.

			// A double colon can only occur once in a IPv6 string.
			if (FoundDoubleColon)
				return 0;

			// Set FoundDoubleColon to true and increment the pos
			// so the shrinking code at the end of the loop
			// skips the second colon too.
			FoundDoubleColon = true;
			DoubleColonByteIndex = BytesFilled;
			++CurSeparatorPos;
		}
		else if (IsHex)
		{
			// It's illegal for a dot separator to come before a colon separator.
			if (FoundDot)
				return 0;

			auto MaybeValue = StringToInt<u16, 16>(Substring);
			if (!MaybeValue.has_value())
				return 0;
			dst.u.Word[BytesFilled / 2] = MaybeValue.value();
		}
		else
		{
			if ((!FoundDoubleColon && BytesFilled != 12) ||
				(FoundDoubleColon && BytesFilled < 10))
				return 0;

			auto MaybeValue = StringToInt<u8, 10>(Substring);
			if (!MaybeValue.has_value())
				return 0;
			dst.u.Byte[BytesFilled / 2] = MaybeValue.value();

			FoundDot = true;
			++NumDots;
		}

		if (CurSeparatorPos == SrcView.npos)
			break;

		SrcView = SrcView.substr(CurSeparatorPos + 1);
	}

	if (FoundDot && NumDots != 4)
		return 0;

	if (FoundDoubleColon)
	{
		// Double colon must implicitly zero at least one field.
		if (BytesFilled >= 15)
			return 0;

		const auto NumImplicitlyZeroedBytes = 16 - BytesFilled;
		const auto Src = dst.u.Byte + DoubleColonByteIndex;
		const auto Dest = Src + NumImplicitlyZeroedBytes;

		// Move memory forward to make place for implicitly zeroed bytes.
		memmove(Dest, Src, BytesFilled);

		// Zero them.
		memset(Src, 0, NumImplicitlyZeroedBytes);
	}

	return 1;
}

int MSOCKET_CALL inet_pton(int af,
	const char* src,
	void* dst)
{
	if (src == nullptr || dst == nullptr)
		return 0;

	if (af == AF::INET) {
		return inet_pton4(af, src, *static_cast<in_addr*>(dst));
	} else if (af == AF::INET6) {
		return inet_pton6(af, src, *static_cast<in6_addr*>(dst));
	} else {
		_set_errno(EAFNOSUPPORT);
		return -1;
	}
}

const char* inet_ntop4(char* dst, size_t size, const in_addr& src)
{
	sprintf_safe(dst, size, "%d.%d.%d.%d",
		src.S_un.S_un_b.s_b1,
		src.S_un.S_un_b.s_b2,
		src.S_un.S_un_b.s_b3,
		src.S_un.S_un_b.s_b4);

	return dst;
}

const char* inet_ntop6(char* dst, size_t size, const in6_addr& src)
{
	sprintf_safe(dst, size, "%X:%X:%X:%X:%X:%X:%X:%X",
		src.u.Word[0],
		src.u.Word[1],
		src.u.Word[2],
		src.u.Word[3],
		src.u.Word[4],
		src.u.Word[5],
		src.u.Word[6],
		src.u.Word[7]);

	return dst;
}

const char* MSOCKET_CALL inet_ntop(int af, const void *src,
	char *dst, size_t size)
{
	if (af == AF::INET) {
		return inet_ntop4(dst, size, *static_cast<const in_addr*>(src));
	} else if (af == AF::INET6) {
		return inet_ntop6(dst, size, *static_cast<const in6_addr*>(src));
	} else {
		_set_errno(EAFNOSUPPORT);
		return nullptr;
	}
}

int MSOCKET_CALL listen(
	SOCKET s,
	int backlog) {
	return ::listen(s, backlog);
}

u32 MSOCKET_CALL ntohl(u32 netlong) {
	return ::ntohl(netlong);
}

u16 MSOCKET_CALL ntohs(u16 netshort) {
	return ::ntohs(netshort);
}

int MSOCKET_CALL recv(
	SOCKET s,
	char * buf,
	int len,
	int flags) {
	return ::recv(s, buf, len, flags);
}

int MSOCKET_CALL recvfrom(
	SOCKET s,
	char * buf,
	int len,
	int flags,
	struct sockaddr * from,
	int * fromlen) {
	return detail::call(::recvfrom, s, buf, len, flags, from, fromlen);
}

/*int MSOCKET_CALL select(
	int nfds,
	fd_set *readfds,
	fd_set *writefds,
	fd_set *exceptfds,
	const struct timeval *timeout) {
	return ::select(nfds, readfds, writefds, exceptfds, timeout);
}*/

int MSOCKET_CALL send(
	SOCKET s,
	const char * buf,
	int len,
	int flags) {
	return ::send(s, buf, len, flags);
}

int MSOCKET_CALL sendto(
	SOCKET s,
	const char * buf,
	int len,
	int flags,
	const struct sockaddr *to,
	int tolen) {
	return ::sendto(s, buf, len, flags, reinterpret_cast<const ::sockaddr*>(to), tolen);
}

int MSOCKET_CALL setsockopt(
	SOCKET s,
	int level,
	int optname,
	const char * optval,
	int optlen) {
	return ::setsockopt(s, level, optname, optval, optlen);
}

int MSOCKET_CALL shutdown(
	SOCKET s,
	int how) {
	return ::shutdown(s, how);
}

SOCKET MSOCKET_CALL socket(
	int af,
	int type,
	int protocol) {
	return ::socket(af, type, protocol);
}

/* Database function prototypes */

#pragma warning(push)
#pragma warning(disable: 4996)
MSocket::hostent * MSOCKET_CALL gethostbyaddr(
	const char * addr,
	int len,
	int type) {
	return reinterpret_cast<MSocket::hostent*>(::gethostbyaddr(addr, len, type));
}

MSocket::hostent * MSOCKET_CALL gethostbyname(const char * name) {
	return reinterpret_cast<MSocket::hostent*>(::gethostbyname(name));
}
#pragma warning(pop)

int MSOCKET_CALL gethostname(
	char * name,
	int namelen) {
	return ::gethostname(name, namelen);
}

MSocket::servent * MSOCKET_CALL getservbyport(
	int port,
	const char * proto) {
	return reinterpret_cast<MSocket::servent*>(::getservbyport(port, proto));
}

MSocket::servent * MSOCKET_CALL getservbyname(
	const char * name,
	const char * proto) {
	return reinterpret_cast<MSocket::servent*>(::getservbyname(name, proto));
}

MSocket::protoent * MSOCKET_CALL getprotobynumber(int proto) {
	return reinterpret_cast<MSocket::protoent*>(::getprotobynumber(proto));
}

MSocket::protoent * MSOCKET_CALL getprotobyname(const char * name) {
	return reinterpret_cast<MSocket::protoent*>(::getprotobyname(name));
}

}
