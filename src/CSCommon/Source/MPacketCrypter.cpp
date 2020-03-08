#include "stdafx.h"
#include "MPacketCrypter.h"
#include "MPacket.h"
#include "MSharedCommandTable.h"

int MPacketCrypter::m_nSHL = (MCOMMAND_VERSION % 6) + 1;
unsigned char MPacketCrypter::m_ShlMask = 0;

bool MPacketCrypter::InitKey(MPacketCrypterKey* pKey)
{
	memcpy(&m_Key, pKey, sizeof(MPacketCrypterKey));
	return true;
}

bool MPacketCrypter::Encrypt(const char* pSource, int nSrcLen, char* pTarget, int nTarLen)
{
	MPacketCrypter::Encrypt(pSource, nSrcLen, pTarget, nTarLen, &m_Key);
	return true;
}

bool MPacketCrypter::Decrypt(const char* pSource, int nSrcLen, char* pTarget, int nTarLen)
{
	MPacketCrypter::Decrypt(pSource, nSrcLen, pTarget, nTarLen, &m_Key);
	return true;
}

bool MPacketCrypter::Encrypt(char* pSource, int nSrcLen)
{
	MPacketCrypter::Encrypt(pSource, nSrcLen, &m_Key);
	return true;
}

bool MPacketCrypter::Decrypt(char* pSource, int nSrcLen)
{
	MPacketCrypter::Decrypt(pSource, nSrcLen, &m_Key);
	return true;
}

bool MPacketCrypter::Encrypt(const char* pSource, int nSrcLen, char* pTarget, int nTarLen, MPacketCrypterKey* pKey)
{
	int nKeyIndex = 0;
	for (int i = 0; i < nSrcLen; i++)
	{
		*pTarget = _Enc(*pSource, pKey->szKey[nKeyIndex]);

		nKeyIndex++;
		if (nKeyIndex >= PACKET_CRYPTER_KEY_LEN) nKeyIndex = 0;
		pTarget++;
		pSource++;
	}

	return true;
}

bool MPacketCrypter::Decrypt(const char* pSource, int nSrcLen, char* pTarget, int nTarLen, MPacketCrypterKey* pKey)
{
	int nKeyIndex = 0;
	for (int i = 0; i < nSrcLen; i++)
	{
		*pTarget = _Dec(*pSource, pKey->szKey[nKeyIndex]);

		nKeyIndex++;
		if (nKeyIndex >= PACKET_CRYPTER_KEY_LEN) nKeyIndex = 0;
		pTarget++;
		pSource++;
	}

	return true;
}


char MPacketCrypter::_Enc(char s, char key)
{
	u16 w;
	u8 b, bh;
	b = s ^ key;
	w = b << m_nSHL;
	bh = (w&0xFF00)>>8;
	b = w&0xFF;
	return u8(b | bh) ^ 0xF0;
}

char MPacketCrypter::_Dec(char s, char key)
{
	u8 b, bh, d;

	b = s^0xF0;
	bh = b&m_ShlMask;
	d = (bh<<(8-m_nSHL))|(b>>m_nSHL);

	return ( d ^ key );
}


bool MPacketCrypter::Encrypt(char* pSource, int nSrcLen, MPacketCrypterKey* pKey)
{
	int nKeyIndex = 0;
	for (int i = 0; i < nSrcLen; i++)
	{
		*pSource = _Enc(*pSource, pKey->szKey[nKeyIndex]);

		nKeyIndex++;
		if (nKeyIndex >= PACKET_CRYPTER_KEY_LEN) nKeyIndex = 0;
		pSource++;
	}
	return true;
}

bool MPacketCrypter::Decrypt(char* pSource, int nSrcLen, MPacketCrypterKey* pKey)
{
	int nKeyIndex = 0;
	for (int i = 0; i < nSrcLen; i++)
	{
		*pSource = _Dec(*pSource, pKey->szKey[nKeyIndex]);

		nKeyIndex++;
		if (nKeyIndex >= PACKET_CRYPTER_KEY_LEN) nKeyIndex = 0;
		pSource++;
	}

	return true;
}

MPacketCrypter::MPacketCrypter()
{
	InitConst();
	memset(&m_Key, 0, sizeof(MPacketCrypterKey));
}

void MPacketCrypter::InitConst()
{
	m_nSHL = (MCOMMAND_VERSION % 6) + 1;

	m_ShlMask = 0;
	for (int i = 0; i < m_nSHL; i++)
	{
		m_ShlMask += (1 << i);
	}
}