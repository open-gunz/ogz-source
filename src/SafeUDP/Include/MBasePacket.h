#pragma once

/////////////////////////////////////////////////////////////
//	BasePacket.h
//								 Programmed by Kim Young-Ho 
//								    LastUpdate : 2000/07/20
/////////////////////////////////////////////////////////////

#include "GlobalTypes.h"

#define SAFEUDP_FLAG_SAFE_PACKET		1
#define SAFEUDP_FLAG_CONTROL_PACKET	1 << 1
#define SAFEUDP_FLAG_ACK_PACKET		1 << 2
#define SAFEUDP_FLAG_LIGHT_PACKET	1 << 3

#pragma pack(push)
#pragma pack(1)

struct MBasePacket {
	u8 nFlags;

	MBasePacket()					{ nFlags = 0; }
	~MBasePacket()					{ }
	bool GetFlag(u8 nTFlag)		{ return (nFlags & nTFlag) != 0; }
	void SetFlag(u8 nTFlag)		{ nFlags |= nTFlag; }
	void ResetFlags(u8 nTFlag)	{ nFlags &= (0xffffffff ^ nTFlag); }
};

struct MACKPacket : MBasePacket {
	u8 nSafeIndex;

	MACKPacket()		{ SetFlag(SAFEUDP_FLAG_ACK_PACKET); }
	~MACKPacket()		{ }
};

struct MNormalPacket : MBasePacket {
	u16 wMsg;
};

struct MSafePacket : MBasePacket {
	u16 wMsg;
	u8 nSafeIndex;		// Using for SafePacket, Ignore on NormalPacket

	MSafePacket()		{ SetFlag(SAFEUDP_FLAG_SAFE_PACKET); }
	~MSafePacket()		{ }
};

struct MLightPacket : MBasePacket {
	u16 wMsg;

	MLightPacket()		{ SetFlag(SAFEUDP_FLAG_LIGHT_PACKET); }
	~MLightPacket()		{ }
};

struct MControlPacket : MSafePacket {
public:
	enum CONTROL {
		CONTROL_SYN,
		CONTROL_SYN_RCVD,
		CONTROL_FIN,
		CONTROL_FIN_RCVD,
		CONTROL_ACK
	};

	CONTROL	nControl;

	MControlPacket()	{ SetFlag(SAFEUDP_FLAG_CONTROL_PACKET); wMsg = 0; }
	~MControlPacket()	{ }
};

#pragma pack(pop)