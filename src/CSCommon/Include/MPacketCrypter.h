#pragma once

#define PACKET_CRYPTER_KEY_LEN		32

struct MPacketCrypterKey
{
	char szKey[PACKET_CRYPTER_KEY_LEN];
};

class MPacketCrypter
{
private:
	MPacketCrypterKey	m_Key;
	static int				m_nSHL;
	static unsigned char	m_ShlMask;

	static char _Enc(char s, char key);
	static char _Dec(char s, char key);
public:
	MPacketCrypter();
	virtual ~MPacketCrypter() {}
	virtual bool InitKey(MPacketCrypterKey* pKey);
	virtual bool Encrypt(const char* pSource, int nSrcLen, char* pTarget, int nTarLen);
	virtual bool Decrypt(const char* pSource, int nSrcLen, char* pTarget, int nTarLen);
	virtual bool Encrypt(char* pSource, int nSrcLen);
	virtual bool Decrypt(char* pSource, int nSrcLen);
	const MPacketCrypterKey* GetKey() { return &m_Key; }

	static void InitConst();
	static bool Encrypt(const char* pSource, int nSrcLen,
		char* pTarget, int nTarLen,
		MPacketCrypterKey* pKey);
	static bool Decrypt(const char* pSource, int nSrcLen,
		char* pTarget, int nTarLen,
		MPacketCrypterKey* pKey);
	static bool Encrypt(char* pSource, int nSrcLen, MPacketCrypterKey* pKey);
	static bool Decrypt(char* pSource, int nSrcLen, MPacketCrypterKey* pKey);
};