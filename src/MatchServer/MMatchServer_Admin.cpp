#include "stdafx.h"
#include "MMatchServer.h"
#include "MSharedCommandTable.h"
#include "MErrorTable.h"
#include "MBlobArray.h"
#include "MObject.h"
#include "MMatchObject.h"
#include "MMatchItem.h"
#include "MAgentObject.h"
#include "MMatchNotify.h"
#include "MMatchObjCache.h"
#include "MMatchStage.h"
#include "MMatchTransDataType.h"
#include "MMatchFormula.h"
#include "MMatchConfig.h"
#include "MCommandCommunicator.h"
#include "MMatchShop.h"
#include "MMatchTransDataType.h"
#include "MDebug.h"
#include "MMatchAuth.h"
#include "MMatchStatus.h"
#include "MAsyncDBJob.h"
#include "MMatchTransDataType.h"
#include "MMatchAntiHack.h"


void MMatchServer::OnAdminTerminal(const MUID& uidAdmin, const char* szText)
{
	MMatchObject* pObj = GetObject(uidAdmin);
	if (pObj == NULL) return;

	if (!IsAdminGrade(pObj))
	{
		return;
	}

	char szOut[32768]; szOut[0] = 0;

	if (m_Admin.Execute(uidAdmin, szText))
	{
		MCommand* pNew = CreateCommand(MC_ADMIN_TERMINAL, MUID(0,0));
		pNew->AddParameter(new MCmdParamUID(MUID(0,0)));
		pNew->AddParameter(new MCmdParamStr(szOut));
		RouteToListener(pObj, pNew);
	}
}

void MMatchServer::OnAdminAnnounce(const MUID& uidAdmin, const char* szChat, u32 nType)
{
	MMatchObject* pObj = GetObject(uidAdmin);
	if (pObj == NULL) return;

	if (!IsAdminGrade(pObj))
	{
		return;
	}

	char szMsg[256];
	strcpy_safe(szMsg, szChat);
	MCommand* pCmd = CreateCommand(MC_ADMIN_ANNOUNCE, MUID(0,0));
	pCmd->AddParameter(new MCmdParamUID(uidAdmin));
	pCmd->AddParameter(new MCmdParamStr(szMsg));
	pCmd->AddParameter(new MCmdParamUInt(nType));

	RouteToAllClient(pCmd);
}



void MMatchServer::OnAdminRequestServerInfo(const MUID& uidAdmin)
{
	MMatchObject* pObj = GetObject(uidAdmin);
	if (pObj == NULL) return;

	if (!IsAdminGrade(pObj))
	{
		return;
	}

}
void MMatchServer::OnAdminServerHalt(const MUID& uidAdmin)
{
	MMatchObject* pObj = GetObject(uidAdmin);
	if (pObj == NULL) return;

	MMatchUserGradeID nGrade = pObj->GetAccountInfo()->m_nUGrade;

	if ((nGrade != MMUG_ADMIN) && (nGrade != MMUG_DEVELOPER)) return;

	m_MatchShutdown.Start(GetGlobalClockCount());
}

void MMatchServer::OnAdminServerHalt()
{
	m_MatchShutdown.Start(GetGlobalClockCount());
}

void MMatchServer::OnAdminRequestBanPlayer(const MUID& uidAdmin, const char* szPlayer)
{
	MMatchObject* pObj = GetObject(uidAdmin);
	if (pObj == NULL) return;

	if (!IsAdminGrade(pObj))
	{
		return;
	}

	int nRet = MOK;

	if ((strlen(szPlayer)) < 2) return;
	MMatchObject* pTargetObj = GetPlayerByName(szPlayer);
	if (pTargetObj != NULL) 
	{
		DisconnectObject(pTargetObj->GetUID());
		GetDBMgr()->BanPlayer(pTargetObj->GetAccountInfo()->m_nAID, "", 0);
	}
	else
	{
		nRet = MERR_NO_TARGET;
	}

	MCommand* pNew = CreateCommand(MC_ADMIN_RESPONSE_BAN_PLAYER, MUID(0,0));
	pNew->AddParameter(new MCmdParamInt(nRet));
	RouteToListener(pObj, pNew);
}

void MMatchServer::OnAdminRequestUpdateAccountUGrade(const MUID& uidAdmin, const char* szPlayer)
{
	MMatchObject* pObj = GetObject(uidAdmin);
	if (pObj == NULL) return;

	if (!IsAdminGrade(pObj))
	{
		return;
	}

	int nRet = MOK;

	if ((strlen(szPlayer)) < 2) return;
	MMatchObject* pTargetObj = GetPlayerByName(szPlayer);
	if (pTargetObj == NULL) return;
}

void MMatchServer::OnAdminPingToAll(const MUID& uidAdmin)
{
	MMatchObject* pObj = GetObject(uidAdmin);
	if (pObj == NULL) return;

	if (!IsAdminGrade(pObj))
	{
		return;
	}

	MCommand* pNew = CreateCommand(MC_NET_PING, MUID(0,0));
	pNew->AddParameter(new MCmdParamUInt(static_cast<u32>(GetGlobalClockCount())));
	RouteToAllConnection(pNew);
}


void MMatchServer::OnAdminRequestSwitchLadderGame(const MUID& uidAdmin, const bool bEnabled)
{
	MMatchObject* pObj = GetObject(uidAdmin);
	if (!IsEnabledObject(pObj)) return;

	if (!IsAdminGrade(pObj))
	{
		return;
	}

	
	MGetServerConfig()->SetEnabledCreateLadderGame(bEnabled);


	char szMsg[256] = "설정되었습니다.";
	Announce(pObj, szMsg);
}

void MMatchServer::OnAdminHide(const MUID& uidAdmin)
{
	MMatchObject* pObj = GetObject(uidAdmin);
	if (!IsEnabledObject(pObj)) return;

	if (!IsAdminGrade(pObj))
	{
		return;
	}

	if (pObj->CheckPlayerFlags(MTD_PlayerFlags_AdminHide)) {
		pObj->SetPlayerFlag(MTD_PlayerFlags_AdminHide, false);
		Announce(pObj, "Now Revealing...");
	} else {
		pObj->SetPlayerFlag(MTD_PlayerFlags_AdminHide, true);
		Announce(pObj, "Now Hiding...");
	}
}

void MMatchServer::OnAdminResetAllHackingBlock( const MUID& uidAdmin )
{
	MMatchObject* pObj = GetObject( uidAdmin );
	if( (0 != pObj) && IsAdminGrade(pObj) )
	{
		GetDBMgr()->AdminResetAllHackingBlock();
	}
}