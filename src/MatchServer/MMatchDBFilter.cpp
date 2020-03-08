#include "stdafx.h"
#include "MMatchDBFilter.h"

#include <utility>


MMatchDBFilter::MMatchDBFilter()
{
}


MMatchDBFilter::~MMatchDBFilter()
{
}


string MMatchDBFilter::Filtering( const string& str )
{
	static string strRemoveTok = "'";

	string strTmp = str;

	string::size_type pos;

	while( (pos = strTmp.find_first_of(strRemoveTok)) != string::npos )
		strTmp.erase(pos, 1);

	return strTmp;
}