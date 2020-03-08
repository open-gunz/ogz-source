#include "stdafx.h"
#include "MMatchUtil.h"
#include "MZFileSystem.h"
#include "MSharedCommandTable.h"
#include "RMath.h"
#include "MMath.h"
#include "MDebug.h"

unsigned long int MGetTimeDistance(unsigned long int nTimeA, unsigned long int nTimeB)
{
	if (nTimeB >= nTimeA)
	{
		return nTimeB - nTimeA;
	}
	else
	{
		return nTimeB + (UINT_MAX - nTimeA + 1);
	}

	return 0;
}

u32 MGetMemoryChecksum(char *pBuffer, int nLen)
{
	u32 nChecksum = 0;
	for (int i=0; i<nLen; i++)
		nChecksum += pBuffer[i];

	return nChecksum;
}

u32 MGetMZFileChecksum(const char* pszFileName) 
{
	MZFile mzf;
	if(!mzf.Open(pszFileName)) 
		return 0;

	char* pBuffer = NULL;
	int nLen = mzf.GetLength();
	pBuffer = new char[mzf.GetLength()+1];
	pBuffer[nLen] = 0;
	mzf.Read(pBuffer, nLen);
	mzf.Close();

	u32 nChecksum = MGetMemoryChecksum(pBuffer,nLen);
	delete pBuffer;
	return nChecksum;
}

void MMakeSeedKey(MPacketCrypterKey* pKey, const MUID& uidServer, const MUID& uidClient, unsigned int nTimeStamp)
{
	unsigned char* p = (unsigned char*)pKey->szKey;
	// key
	memset(p, 0, sizeof(MPacketCrypterKey));
	int nUIDSize = sizeof(MUID);

	memcpy(p, &nTimeStamp, sizeof(unsigned int));
	memcpy(p+sizeof(unsigned int), &uidServer.Low, sizeof(unsigned int));
	memcpy(p+nUIDSize, &uidClient, nUIDSize);

	const u8 XOR[16] = {87, 2, 91, 4, 52, 6, 1, 8, 55, 10, 18, 105, 65, 56, 15, 120};

	for (int i = 0; i < 16; i++)
	{
		p[i] ^= XOR[i];
	}

	p = p + 16;

	// iv
	p[0] = 55;
	p[7] = 83;
	p[1] = 4;
	p[2] = 93;
	p[3] = 46;
	p[4] = 67;
	p[5] = MCOMMAND_VERSION;
	p[11] = 201;
	p[6] = 73;
	p[8] = 80;
	p[9] = 5;
	p[10] = 19;
	p[12] = 40;
	p[13] = 164;
	p[14] = 77;
	p[15] = 5;
}

PackedDirection PackDirection(const v3& src)
{
	return{ static_cast<int8_t>(atan2(src.y, src.x) * (INT8_MAX / PI)),
		static_cast<int8_t>(src.z * INT8_MAX) };
}

v3 UnpackDirection(const PackedDirection& src)
{
	v3 ret;
	auto Yaw = src.Yaw * (PI / INT8_MAX);
	ret.x = static_cast<float>(cos(Yaw));
	ret.y = static_cast<float>(sin(Yaw));
	ret.z = static_cast<float>(src.Pitch) / INT8_MAX;
	ret.x *= sqrt(1 - ret.z * ret.z);
	ret.y *= sqrt(1 - ret.z * ret.z);
	RealSpace2::Normalize(ret);
	return ret;
}