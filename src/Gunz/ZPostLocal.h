#ifndef _ZPOSTLOCAL_H
#define _ZPOSTLOCAL_H

#include "ZPrerequisites.h"
#include "ZGameClient.h"
#include "ZCommandTable.h"

// Local /////////////////////////////////////////////////////////////////////////////////////////////
inline void ZPostLocalReport119()
{
	ZPOSTCMD0(ZC_REPORT_119);
}

inline void ZPostLocalMessage(int nMessageID)
{
	ZPOSTCMD1(ZC_MESSAGE,MCommandParameterInt(nMessageID));
}

inline void ZPostLocalEventOptainSpecialWorldItem(const int nWorldItemID)
{
	ZPOSTCMD1(ZC_EVENT_OPTAIN_SPECIAL_WORLDITEM, MCmdParamInt(nWorldItemID));
}







#endif