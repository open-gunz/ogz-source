#include "stdafx.h"
#include "MPacket.h"
#include "MPacketCrypter.h"

int MPacketHeader::CalcPacketSize(MPacketCrypter* pCrypter)
{
	unsigned short nPacketSize = 0;

	if (nMsg == MSGID_COMMAND)
	{
		if (pCrypter)
		{
			pCrypter->Decrypt((char*)(&nSize), sizeof(unsigned short), (char*)&nPacketSize, sizeof(unsigned short));
		}
	}
	else
	{
		nPacketSize = nSize;
	}


	return (int)nPacketSize;
}
