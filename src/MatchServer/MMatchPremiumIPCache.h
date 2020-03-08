#ifndef _MMATCHPREMIUMIPCACHE_H
#define _MMATCHPREMIUMIPCACHE_H


#include <map>

class MMatchPremiumIPNode
{
private:
	u32	IP;
	u64		Time;	// Last updated time
public:
	MMatchPremiumIPNode(u32 IP, u64 tmTime) : IP(IP), Time(Time) {}
	auto GetIP() const			{ return IP; }
	void SetIP(u32 dwIP)		{ IP = dwIP; }
	auto GetTime() const		{ return Time; }
	void SetTime(u64 tmTime)	{ Time = tmTime; }
};

class MMatchPremiumIPMap : public std::map<u32, MMatchPremiumIPNode> {};

class MMatchPremiumIPCache
{
private:
	std::mutex					m_csLock;
	MMatchPremiumIPMap			m_PremiumIPMap;
	MMatchPremiumIPMap			m_NotPremiumIPMap;
	int							m_nDBFailedCount;
	int							m_nFailedCheckCount;

	void Lock()		{ m_csLock.lock(); }
	void Unlock()	{ m_csLock.unlock(); }
public:
	MMatchPremiumIPCache();
	~MMatchPremiumIPCache();
	static MMatchPremiumIPCache* GetInstance();

	bool CheckPremiumIP(u32 dwIP, bool& outIsPremiumIP);
	void AddIP(u32 dwIP, bool bPremiumIP);
	void OnDBFailed();
	void Update();
};

inline MMatchPremiumIPCache* MPremiumIPCache()
{
	return MMatchPremiumIPCache::GetInstance();
}




#endif