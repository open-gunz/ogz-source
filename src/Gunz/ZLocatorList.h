#pragma once

#include <vector>
#include <string>

#define LOCATOR_PORT 8900

class ZLocatorList : public std::map<int, std::string>
{
public:
	void Clear() { m_LocatorIPList.clear(); }
	int GetSize() const { return static_cast<int>( m_LocatorIPList.size() ); }
	const std::string& GetIPByPos(int nPos) const { return m_LocatorIPList[ nPos ]; };

	bool ParseLocatorList(MXmlElement& element);

private:
	bool ParseLocator(MXmlElement& element);

	std::vector<std::string> m_LocatorIPList;
};