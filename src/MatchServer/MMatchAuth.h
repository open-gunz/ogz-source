#pragma once


#define MAUTHINFO_BUFLEN	4096


class MMatchAuthInfo {
public:
	MMatchAuthInfo()			{}
	virtual ~MMatchAuthInfo()	{}

	virtual const char* GetUserID() = 0;
	virtual const char* GetUniqueID() = 0;
	virtual const char* GetCertificate() = 0;
	virtual const char* GetName() = 0;
	virtual int GetAge() = 0;
	virtual int GetSex() = 0;
};

class MMatchAuthBuilder {
public:
	MMatchAuthBuilder()				{}
	virtual ~MMatchAuthBuilder()	{}

	virtual bool ParseAuthInfo(const char* pszData, MMatchAuthInfo** ppoutAutoInfo) = 0;
};
