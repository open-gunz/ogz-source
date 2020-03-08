#pragma once

#include "GlobalTypes.h"

using HANDLE = void*;
using SOCKET = uintptr_t;
struct in_addr;
class MSignalEvent;

namespace MSocket
{

bool Startup();
void Cleanup();

// Creates an event suitable for socket operations.
MSignalEvent CreateEvent();
int EventSelect(SOCKET, MSignalEvent&, int);

struct NetworkEvents
{
	int NetworkEventsSet;
	int ErrorCode[10];
};

int EnumNetworkEvents(SOCKET, MSignalEvent&, NetworkEvents*);

bool IsNetworkEventSet(const NetworkEvents&, int Flag);
int GetNetworkEventValue(const NetworkEvents&, int Flag);

namespace Error {
enum Type {
	Interrupted = 10004,
	BadFile = 10009,
	AccessDenied = 10013,
	Fault = 10014,
	Invalid = 10022,
	TooManyOpenFiles = 10024,

	WouldBlock = 10035,
	InProgess,
	Already,
	NotSock,
	DestAddrReq,
	MsgSize,
	Prototype,
	NoProtoOpt,
	ProtoNoSupport,
	SocketNoSupport,
	OperationNotSupported,
	PFNoSupport,
	AFNoSupport,
	AddressInUse,
	AddressNotAvailable,
	NetDown,
	NetUnreachable,
	NetReset,
	ConnectionAborted,
	ConnectionReset,
	NoBufs,
	IsConn,
	NotConn,
	Shutdown,
	TooManyRefs,
	TimedOut,
	ConnectionRefused,
	Loop,
	NameTooLong,
	HostDown,
	HostUnreachable,
	NotEmpty,
	Proclim,
	Users,
	DiskQuotaExceeded,
	Stale,
	Remote,

	SystemNotReady = 10091,
	VersionNotSupported,
	NotInitialized,

	Disconnected = 10101,
	NoMore,
	Cancelled,
	InvalidProcTable,
	InvalidProvider,
	ProviderFailedInit,
	SysCallFailure,
	ServiceNotFound,
	TypeNotFound,
	NoMore2,
	Cancelled2,
	Refused,
}; }

u32 GetLastError();
bool GetErrorString(u32 ErrorCode, char* Output, size_t OutputSize);
bool GetErrorString(u32 ErrorCode, std::string& Output);

template <size_t Size> bool GetErrorString(u32 ErrorCode, char(&Output)[Size]) {
	return GetErrorString(ErrorCode, Output, Size); }

#ifdef _WIN32
constexpr SOCKET InvalidSocket = -1;
constexpr SOCKET SocketError = -1;
#else
constexpr int InvalidSocket = -1;
constexpr int SocketError = -1;
#endif

void LogError(const char* CallerName, const char* FunctionName, int ErrorCode);

#define LOG_SOCKET_ERROR(FunctionName, ErrorCode) do {\
MSocket::LogError(__FUNCTION__, FunctionName, ErrorCode);\
assert(false); } while (false)

// POSIX socket API

struct in_addr
{
	static constexpr u32 None = 0xFFFFFFFF;
	static constexpr u32 Any = 0;

	in_addr() = default;
	in_addr(const ::in_addr&);

	union {
		union {
			struct { u8 s_b1, s_b2, s_b3, s_b4; };
			struct { u16 s_w1, s_w2; };
#ifndef s_imp
			struct { u16 id1, s_imp; };
			u32 s_addr;
			struct { u8 s_net; u8 s_host; u8 s_lh; u8 s_impno; };
#endif
		};
		union {
			struct { u8 s_b1, s_b2, s_b3, s_b4; } S_un_b;
			struct { u16 s_w1, s_w2; } S_un_w;
			u32 S_addr;
		} S_un;
	};

	operator const ::in_addr&() const;
};

struct in6_addr {
	union {
		u8  Byte[16];
		u16 Word[8];
		u32 Dword[4];
	} u;
};

struct sockaddr
{
	u16      sa_family;    /* address family */
	char     sa_data[14]; /* up to 14 bytes of direct address */
};

struct sockaddr_in
{
	i16      sin_family;
	u16      sin_port;
	in_addr  sin_addr;
	char     sin_zero[8];
};

struct sockaddr_in6 {
	i16      sin6_family;   /* AF_INET6 */
	u16      sin6_port;     /* port number */
	u32      sin6_flowinfo; /* IPv6 flow information */
	in6_addr sin6_addr;     /* IPv6 address */
	u32      sin6_scope_id; /* Scope ID (new in 2.4) */
};

struct sockaddr_storage {
	i16      ss_family;
	i8       ss_pad1[6];
	i64      ss_align;
	i8       ss_pad2[112];
};

static_assert(sizeof(sockaddr_storage) > sizeof(sockaddr_in6), "");

struct  hostent {
	char    * h_name;           /* official name of host */
	char    ** h_aliases;  /* alias list */
	short   h_addrtype;             /* host address type */
	short   h_length;               /* length of address */
	char    ** h_addr_list; /* list of addresses */
#define h_addr  h_addr_list[0]          /* address, for backward compat */
};


namespace FD
{
enum Type
{
	READ = 1 << 0,
	WRITE = 1 << 1,
	OOB = 1 << 2,
	ACCEPT = 1 << 3,
	CONNECT = 1 << 4,
	CLOSE = 1 << 5,
};
}

/*
* Address families.
*/
namespace AF
{
enum Type
{
	UNSPEC = 0,            /* unspecified */
	UNIX = 1,              /* local to host (pipes, portals) */
	INET = 2,              /* internetwork: UDP, TCP, etc. */
	IMPLINK = 3,           /* arpanet imp addresses */
	PUP = 4,               /* pup protocols: e.g. BSP */
	CHAOS = 5,             /* mit CHAOS protocols */
	IPX = 6,               /* IPX and SPX */
	NS = 6,                /* XEROX NS protocols */
	ISO = 7,               /* ISO protocols */
	OSI = ISO,             /* OSI is ISO */
	ECMA = 8,              /* european computer manufacturers */
	DATAKIT = 9,           /* datakit protocols */
	CCITT = 10,            /* CCITT protocols, X.25 etc */
	SNA = 11,              /* IBM SNA */
	DECnet = 12,           /* DECnet */
	DLI = 13,              /* Direct data link interface */
	LAT = 14,              /* LAT */
	HYLINK = 15,           /* NSC Hyperchannel */
	APPLETALK = 16,        /* AppleTalk */
	NETBIOS = 17,          /* NetBios-style addresses */
	VOICEVIEW = 18,        /* VoiceView */
	FIREFOX = 19,          /* FireFox */
	UNKNOWN1 = 20,         /* Somebody is using this! */
	BAN = 21,              /* Banyan */
	ATM = 22,
	INET6 = 23,
	CLUSTER = 24,
	IEEE12844 = 25,
	IRDA = 26,
	NETDES = 28,

	MAX,
};
}



/*
* Types
*/
namespace SOCK
{
enum Type
{
	STREAM = 1,               /* stream socket */
	DGRAM = 2,               /* datagram socket */
	RAW = 3,               /* raw-protocol interface */
	RDM = 4,               /* reliably-delivered message */
	SEQPACKET = 5,               /* sequenced packet stream */
};
}

/*
* Commands for ioctlsocket(),  taken from the BSD file fcntl.h.
*
*
* Ioctl's have the command encoded in the lower word,
* and the size of any in or out parameters in the upper
* word.  The high 2 bits of the upper word are used
* to encode the in/out status of the parameter; for now
* we restrict parameters to at most 128 bytes.
*/
namespace IOC
{
enum Type
{
	ParmMask = 0x7f,            /* parameters must be < 128 bytes */
	Void = 0x20000000,      /* no parameters */
	Out = 0x40000000,      /* copy out parameters */
	In = 0x80000000,      /* copy in parameters */
	InOut = (In | Out),
};
}
/* 0x20000000 distinguishes new &
old ioctl's */

constexpr auto io(u32 x, u32 y) {
	return IOC::Void | (x << 8) | y;
}

template <typename t>
constexpr auto ior(u32 x, u32 y) {
	return IOC::Out | ((sizeof(t) & IOC::ParmMask) << 16) | (x << 8) | y;
}

template <typename t>
constexpr auto iow(u32 x, u32 y) {
	return IOC::In | ((sizeof(t) & IOC::ParmMask) << 16) | (x << 8) | y;
}

namespace FIO
{
enum Type
{
	NREAD = ior<u32>('f', 127), /* get # bytes to read */
	NBIO  = iow<u32>('f', 126), /* set/clear non-blocking i/o */
	ASYNC = iow<u32>('f', 125), /* set/clear async i/o */
};
}

/*
* Protocols
*/
namespace IPPROTO
{
enum Type
{
	IP            = 0,               /* dummy for IP */
	ICMP          = 1,               /* control message protocol */
	IGMP          = 2,               /* group management protocol */
	GGP           = 3,               /* gateway^2 (deprecated) */
	TCP           = 6,               /* tcp */
	PUP           = 12,              /* pup */
	UDP           = 17,              /* user datagram protocol */
	IDP           = 22,              /* xns idp */
	ND            = 77,              /* UNOFFICIAL net disk proto */

	RAW           = 255,             /* raw IP packet */
	MAX           = 256,
};
}

/*
* TCP options.
*/
namespace TCP
{
enum Type
{
	NODELAY = 0x0001,
	BSDURGENT = 0x7000,
};
}

/*
* Level number for (get/set)sockopt() to apply to socket itself.
*/
namespace SOL { enum Type {
	SOCKET    = 0xffff,          /* options for socket level */
}; }

namespace SO
{
enum Type
{
	/*
	* Option flags per-socket.
	*/
	Debug = 0x0001,          /* turn on debugging info recording */
	ACCEPTCONN = 0x0002,          /* socket has had listen() */
	REUSEADDR = 0x0004,          /* allow local address reuse */
	KEEPALIVE = 0x0008,          /* keep connections alive */
	DONTROUTE = 0x0010,          /* just use interface addresses */
	BROADCAST = 0x0020,          /* permit sending of broadcast msgs */
	USELOOPBACK = 0x0040,          /* bypass hardware when possible */
	LINGER = 0x0080,          /* linger on close if data present */
	OOBINLINE = 0x0100,          /* leave received OOB data in line */

	DONTLINGER = (int)(~LINGER),
	EXCLUSIVEADDRUSE = (int)(~REUSEADDR), /* disallow local address reuse */

		/*
		* Additional options.
		*/
	SNDBUF = 0x1001,          /* send buffer size */
	RCVBUF = 0x1002,          /* receive buffer size */
	SNDLOWAT = 0x1003,          /* send low-water mark */
	RCVLOWAT = 0x1004,          /* receive low-water mark */
	SNDTIMEO = 0x1005,          /* send timeout */
	RCVTIMEO = 0x1006,          /* receive timeout */
	Error = 0x1007,          /* get error status and clear */
	TYPE = 0x1008,          /* get socket type */

		/*
		* WinSock 2 extension -- new options
		*/
	GROUP_ID = 0x2001,      /* ID of a socket group */
	GROUP_PRIORITY = 0x2002,      /* the relative priority within a group*/
	MAX_MSG_SIZE = 0x2003,      /* maximum message size */
	PROTOCOL_INFOA = 0x2004,      /* WSAPROTOCOL_INFOA structure */
	PROTOCOL_INFOW = 0x2005,      /* WSAPROTOCOL_INFOW structure */
#ifdef UNICODE
	PROTOCOL_INFO = PROTOCOL_INFOW,
#else
	PROTOCOL_INFO = PROTOCOL_INFOA,
#endif /* UNICODE */
	PROVIDER_CONFIG = 0x3001,       /* configuration info for service provider */
	CONDITIONAL_ACCEPT = 0x3002,   /* enable true conditional accept: */
		/*  connection is not ack-ed to the */
		/*  other side until conditional */
		/*  function returns CF_ACCEPT */
};
}

namespace SD
{
enum Type
{
	/*
	* WinSock 2 extension -- manifest constants for shutdown()
	*/
	RECEIVE = 0x00,
	SEND = 0x01,
	BOTH = 0x02,
};
}

struct  servent {
	char    * s_name;           /* official service name */
	char    ** s_aliases;  /* alias list */
#ifdef _WIN64
	char    * s_proto;          /* protocol to use */
	short   s_port;                 /* port # */
#else
	short   s_port;                 /* port # */
	char    * s_proto;          /* protocol to use */
#endif
};

struct  protoent {
	char    * p_name;           /* official protocol name */
	char    ** p_aliases;  /* alias list */
	short   p_proto;                /* protocol # */
};

#ifdef _MSC_VER
#define MSOCKET_CALL __stdcall
#else
#define MSOCKET_CALL
#endif

SOCKET MSOCKET_CALL accept(
	SOCKET s,
	struct sockaddr *addr,
	int *addrlen);

int MSOCKET_CALL bind(
	SOCKET s,
	const struct sockaddr *addr,
	int namelen);

int MSOCKET_CALL closesocket(SOCKET s);

int MSOCKET_CALL connect(
	SOCKET s,
	const struct sockaddr *name,
	int namelen);

int MSOCKET_CALL ioctlsocket(
	SOCKET s,
	long cmd,
	u32 *argp);

int MSOCKET_CALL getpeername(
	SOCKET s,
	struct sockaddr *name,
	int * namelen);

int MSOCKET_CALL getsockname(
	SOCKET s,
	struct sockaddr *name,
	int * namelen);

int MSOCKET_CALL getsockopt(
	SOCKET s,
	int level,
	int optname,
	char * optval,
	int *optlen);

u32 MSOCKET_CALL htonl(u32 hostlong);

u16 MSOCKET_CALL htons(u16 hostshort);

/*unsigned long MSOCKET_CALL inet_addr(const char * cp);

char * MSOCKET_CALL inet_ntoa(struct in_addr in);*/

int MSOCKET_CALL inet_pton(int af,
	const char* src,
	void* dst);

const char* MSOCKET_CALL inet_ntop(int af, const void *src,
	char *dst, size_t size);

int MSOCKET_CALL listen(
	SOCKET s,
	int backlog);

u32 MSOCKET_CALL ntohl(u32 netlong);

u16 MSOCKET_CALL ntohs(u16 netshort);

int MSOCKET_CALL recv(
	SOCKET s,
	char * buf,
	int len,
	int flags);

int MSOCKET_CALL recvfrom(
	SOCKET s,
	char * buf,
	int len,
	int flags,
	struct sockaddr * from,
	int * fromlen);

/*int MSOCKET_CALL select(
	int nfds,
	fd_set *readfds,
	fd_set *writefds,
	fd_set *exceptfds,
	const struct timeval *timeout);*/

int MSOCKET_CALL send(
	SOCKET s,
	const char * buf,
	int len,
	int flags);

int MSOCKET_CALL sendto(
	SOCKET s,
	const char * buf,
	int len,
	int flags,
	const struct sockaddr *to,
	int tolen);

int MSOCKET_CALL setsockopt(
	SOCKET s,
	int level,
	int optname,
	const char * optval,
	int optlen);

int MSOCKET_CALL shutdown(
	SOCKET s,
	int how);

SOCKET MSOCKET_CALL socket(
	int af,
	int type,
	int protocol);

/* Database function prototypes */

MSocket::hostent * MSOCKET_CALL gethostbyaddr(
	const char * addr,
	int len,
	int type);

MSocket::hostent * MSOCKET_CALL gethostbyname(const char * name);

int MSOCKET_CALL gethostname(
	char * name,
	int namelen);

MSocket::servent * MSOCKET_CALL getservbyport(
	int port,
	const char * proto);

MSocket::servent * MSOCKET_CALL getservbyname(
	const char * name,
	const char * proto);

MSocket::protoent * MSOCKET_CALL getprotobynumber(int proto);

MSocket::protoent * MSOCKET_CALL getprotobyname(const char * name);

}