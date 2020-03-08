#include "stdafx.h"

#include "ZPost.h"
#include "MBlobArray.h"
#include "MMatchTransDataType.h"
#include "MMatchGlobal.h"
#include "ZGame.h"
#include "ZMyCharacter.h"
#include "ZGameClient.h"
#include "ZApplication.h"
#include "ZConfiguration.h"
#include "RGVersion.h"

void ZPostUserOption()
{
	u32 nOptionFlags = 0;

	if (Z_ETC_REJECT_WHISPER)
		nOptionFlags |= MBITFLAG_USEROPTION_REJECT_WHISPER;
	if (Z_ETC_REJECT_INVITE)
		nOptionFlags |= MBITFLAG_USEROPTION_REJECT_INVITE;

	ZPOSTCMD1(MC_MATCH_USER_OPTION, MCmdParamUInt(nOptionFlags));
}

void ZPostLogin(const char* szUserID, const unsigned char *HashedPassword,
	int HashLength, unsigned int ChecksumPack)
{
	ZPostCmd(MC_MATCH_LOGIN, MCmdParamStr(szUserID), MCmdParamBlob(HashedPassword, HashLength),
		MCmdParamInt(MCOMMAND_VERSION), MCmdParamUInt(ChecksumPack),
		MCmdParamUInt(RGUNZ_VERSION_MAJOR), MCmdParamUInt(RGUNZ_VERSION_MINOR),
		MCmdParamUInt(RGUNZ_VERSION_PATCH), MCmdParamUInt(RGUNZ_VERSION_REVISION));
}
