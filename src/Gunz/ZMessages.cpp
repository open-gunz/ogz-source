#include "stdafx.h"

#include "ZMessages.h"
#include "MXml.h"
#include "ZFilePath.h"

#include "ZApplication.h"


const char* ZGetSexStr(MMatchSex nSex, bool bShort)
{
	if (bShort)
	{
		switch (nSex)
		{
		case MMS_MALE:		
			return ZMsg(MSG_WORD_MALE);

		case MMS_FEMALE:	
			return ZMsg(MSG_WORD_FEMALE);
		}
	}
	else
	{
		switch (nSex)
		{
		case MMS_MALE:		
			return ZMsg(MSG_WORD_MALE_SHORT);

		case MMS_FEMALE:	
			return ZMsg(MSG_WORD_FEMALE_SHORT);
		}
	}

	_ASSERT(0);
	return "";
}

void ZGetTimeStrFromSec(char* poutStr, size_t maxlen, u32 nSec)
{
	int d, h, m, s;

	d = (nSec / (60*60*24));
	nSec = nSec % (60*60*24);

	h = (nSec / (60*60));
	nSec = nSec % (60*60);

	m = (nSec / (60));
	nSec = nSec % (60);
	
	s = nSec;

	char sztemp[128];

	poutStr[0] = 0;

	auto append = [&](auto val, auto msgid) {
		sprintf_safe(sztemp, "%d%s ", val, ZMsg(msgid));
		strcat_safe(poutStr, maxlen, sztemp);
	};

	if (d != 0)
		append(d, MSG_CHARINFO_DAY);
	if (h != 0)
		append(h, MSG_CHARINFO_HOUR);
	if (m != 0)
		append(m, MSG_CHARINFO_MINUTE);

	append(s, MSG_CHARINFO_SECOND);
}
