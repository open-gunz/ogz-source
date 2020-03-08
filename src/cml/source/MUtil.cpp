#include "stdafx.h"
#include "MUtil.h"
#include "MDebug.h"
#include <ctime>

#ifdef WIN32
#include <Windows.h>
#include <Unknwn.h>

void D3DDeleter::operator()(IUnknown* ptr) const {
	ptr->Release();
}
#endif

std::string MGetStrLocalTime(MDateType DateType)
{
	const char* FormatString = [&]() -> const char* {
		switch (DateType)
		{
		case MDT_Y:
			return "%Y";
		case MDT_YM:
			return "%Y-%m";
		case MDT_YMD:
			return "%Y-%m-%d";
		case MDT_YMDH:
			return "%Y-%m-%d %H";
		case MDT_YMDHM:
			return "%Y-%m-%d %H-%M";
		default:
			return nullptr;
		}
	}();
	if (!FormatString)
		return{};

	auto t = time(nullptr);
	tm TM;
#ifdef _MSC_VER
	auto ret = localtime_s(&TM, &t);
#else
	auto ret = localtime_r(&t, &TM);
#endif
	if (ret != 0)
	{
		return{};
	}

	char buf[128];
	auto size = strftime(buf, sizeof(buf), FormatString, &TM);

	return{ buf, size };
}
