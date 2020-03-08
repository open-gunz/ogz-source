#pragma once

#include <string>

class MMatchDBFilter
{
public :
	MMatchDBFilter();
	~MMatchDBFilter();

	std::string Filtering(const std::string& str);
};