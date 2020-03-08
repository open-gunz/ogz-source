#pragma once

#ifdef WIN32

class MRegistry {
public:
	static char*	szRegistryBasePath;
	static char*	szApplicationName;
public:
	// hRegKey = HKEY_LOCAL_MACHINE or HKEY_CURRENT_USER
	template<size_t size>
	static bool Read(HKEY hRegKey, const char* szRegString, char(&pOutBuffer)[size]) {
		return Read(hRegKey, szRegString, pOutBuffer, size);
	}
	static bool Read(HKEY hRegKey, const char* szRegString, char* pOutBuffer, int maxlen);
	static void Write(HKEY hRegKey, const char* szRegString, const char* pInBuffer);
	static bool ReadBinary(HKEY hRegKey, const char* szRegString, char* pOutBuffer, DWORD* pdwBufferLen);
	static void WriteBinary(HKEY hRegKey, const char* szRegString, const char* pInBuffer, DWORD dwBufferLen);

	template<size_t size>
	static bool Read(HKEY hRegKey, const char* szPath, const char* szValueName, char (&pOutBuffer)[size]) {
		return Read(hRegKey, szPath, szValueName, pOutBuffer, size);
	}
	static bool Read(HKEY hRegKey, const char* szPath, const char* szValueName, char* pOutBuffer, int maxlen);
	static void Write(HKEY hRegKey, const char* szPath, const char* szValueName, const char* szValue);
};

#endif