#pragma once

#include <vector>

class ZWorld;
class ZWorldManager : public std::vector<ZWorld*>
{
	int m_nCurrent;
	std::set<ZWorld*>	m_Worlds;

public:
	ZWorldManager();
	virtual ~ZWorldManager();

	void Destroy();

	void AddWorld(const char* szMapName);

	void Clear();
	bool LoadAll(ZLoadingProgress *pLoading);

	int GetCount() { return (int)size(); }

	ZWorld	*GetWorld(int i);

	ZWorld	*SetCurrent(int i);
	ZWorld	*GetCurrent() { return GetWorld(m_nCurrent); }

	void OnInvalidate();
	void OnRestore();
};
