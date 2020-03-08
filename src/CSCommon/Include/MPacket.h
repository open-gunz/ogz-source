#pragma once

#include "MCommand.h"
#include "MCommandManager.h"
#include <algorithm>

#define MAX_PACKET_SIZE			16384

#define MSG_COMMAND	1000

#define MSGID_REPLYCONNECT	10
#define MSGID_RAWCOMMAND	100
#define MSGID_COMMAND		101

class MPacketCrypter;

#pragma pack(push)
#pragma pack(1)

struct MPacketHeader
{
	unsigned short nMsg;
	unsigned short nSize;
	unsigned short nCheckSum;

	MPacketHeader() { nMsg=MSG_COMMAND; nSize=0; nCheckSum=0; }
	int CalcPacketSize(MPacketCrypter* pCrypter);
};


struct MReplyConnectMsg : public MPacketHeader
{
	unsigned int	nHostHigh;
	unsigned int	nHostLow;
	unsigned int	nAllocHigh;
	unsigned int	nAllocLow;
	unsigned int	nTimeStamp;
};

struct MCommandMsg : public MPacketHeader
{
	char	Buffer[1];
};

#pragma pack(pop)

// Tiny CheckSum for MCommandMsg
inline unsigned short MBuildCheckSum(MPacketHeader* pPacket, int nPacketSize)
{
	int nStartOffset = sizeof(MPacketHeader);
	u8* pBulk = reinterpret_cast<u8*>(pPacket);
	nPacketSize = (std::min)(65535, nPacketSize);

	u32 nCheckSum = 0;
	for (int i=nStartOffset; i<nPacketSize; i++) {
		nCheckSum += pBulk[i];
	}
	nCheckSum -= (pBulk[0]+pBulk[1]+pBulk[2]+pBulk[3]);
	unsigned short nShortCheckSum = (nCheckSum & 0xFFFF) + (nCheckSum >> 16);
	return nShortCheckSum;
}