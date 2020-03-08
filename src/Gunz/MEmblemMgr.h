#ifndef _MEMBLEMMGR_H
#define _MEMBLEMMGR_H

//#pragma once


#include "MHttpSpooler.h"
#include <string>
#include <map>
using namespace std;


class MEmblemNode {
protected:
	unsigned int	m_nCLID;
	u32	m_nChecksum;
	char			m_szURL[256];
	time_t			m_tmLastUsed;
public:
	MEmblemNode()			{ m_nCLID=0; m_nChecksum=0; m_szURL[0]=NULL; m_tmLastUsed=0; }
	virtual ~MEmblemNode()	{}

	unsigned int GetCLID()			{ return m_nCLID; }
	void SetCLID(unsigned int nCLID){ m_nCLID = nCLID; }
	const char* GetURL()			{ return m_szURL; }
	void SetURL(const char* pszURL)	{ strcpy_safe(m_szURL, pszURL); }
	u32 GetChecksum()		{ return m_nChecksum; }
	void SetChecksum(u32 nChecksum)	{ m_nChecksum = nChecksum; }

	time_t GetTimeLastUsed()		{ return m_tmLastUsed; }
	void SetTimeLastUsed(time_t tm)	{ m_tmLastUsed = tm; }
	void UpdateTimeLastUsed()		{ time(&m_tmLastUsed); }
};
class MEmblemMap : public map<unsigned int, MEmblemNode*> {};


class MEmblemMgr {
protected:
	MHttpSpooler	m_HttpSpooler;

	MEmblemMap		m_EmblemMap;

	std::string		m_szEmblemBaseDir;
	std::string		m_szEmblemDataFile;

	bool			m_bSave;
	u32	m_tmLastSavedTick;

	int				m_nTotalRequest;
	int				m_nCachedRequest;

protected:
	bool InitDefaut();
	string GetEmblemBaseDir()				{ return m_szEmblemBaseDir; }
	string GetEmblemDataFile()				{ return m_szEmblemDataFile; }

	bool CheckSaveFlag()						{ return m_bSave; }
	void SetSaveFlag(bool bSave)				{ m_bSave = bSave; }
	u32 GetLastSavedTick()			{ return m_tmLastSavedTick; }
	void SetLastSavedTick(u32 nTick)	{ m_tmLastSavedTick = nTick; }

	bool CreateCache();
	bool LoadCache();
	bool SaveCache();
	void ClearCache();

	void PostDownload(unsigned int nCLID, unsigned int nChecksum, const char* pszURL);
	bool RegisterEmblem(unsigned int nCLID, const char* pszURL, u32 nChecksum, time_t tmLastUsed=0);
	void NotifyDownloadDone(unsigned int nCLID, const char* pszURL);

public:
	void Create();
	void Destroy();

	bool GetEmblemPath(char* pszFilePath, size_t maxlen, const char* pszURL);
	template <size_t size>
	auto GetEmblemPath(char(&pszFilePath)[size], const char* pszURL) {
		return GetEmblemPath(pszFilePath, size, pszURL);
	}
	bool GetEmblemPathByCLID(unsigned int nCLID, char* poutFilePath, size_t maxlen);
	template <size_t size>
	auto GetEmblemPathByCLID(unsigned int nCLID, char(&poutFilePath)[size]) {
		return GetEmblemPathByCLID(nCLID, poutFilePath, size);
	}

	int GetTotalRequest()	{ return m_nTotalRequest; }
	int GetCachedRequest()	{ return m_nCachedRequest; }

	bool PrepareCache();

	bool CheckEmblem(unsigned int nCLID, u32 nChecksum);
	bool ProcessEmblem(unsigned int nCLID, const char* pszURL, u32 nChecksum);

	void Tick(u32 nTick);
};


#endif
