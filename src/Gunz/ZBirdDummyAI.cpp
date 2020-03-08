#include "stdafx.h"

#include "ZBirdDummyAI.h"
#include "ZGameInterface.h"
#include "ZApplication.h"
#include "ZPost.h"
#include "ZConsole.h"
#include "MCommandLogFrame.h"
#include "ZConfiguration.h"
#include "FileInfo.h"
#include "ZInterfaceItem.h"
#include "ZInterfaceListener.h"
#include "MDebug.h"
#include "MMatchTransDataType.h"
#include "MBlobArray.h"
#include "ZGameClient.h"
#include "time.h"
#include "ZBirdDummyClient.h"

#define BIRDDUMMY_AI_KILL_DELAY		300

ZBirdDummyAI::ZBirdDummyAI()
{
	m_nID = 0;
	m_bInCombat = false;
	m_nKillCount = 0;
	m_pClient = NULL;
	m_nLastCommandTime = 0;
	m_bCreated = false;
	if ((rand() % 100) < 20) m_nLobbyType = ZBDAI_MASTER; else m_nLobbyType = ZBDAI_GUEST;
	if ((rand() % 100) < 20) m_nMasterType = ZBDAI_STARTALONE; else m_nMasterType = ZBDAI_WAIT;
	if ((rand() % 100) < 30) m_nGuestType = ZBDAI_READY; else m_nGuestType = ZBDAI_FORCEDENTRY;
}	

ZBirdDummyAI::~ZBirdDummyAI()
{

}

void ZBirdDummyAI::OnCommand(MCommand* pCmd)
{
	Sleep(1);
	if (m_pClient == NULL) return;

	m_nLastCommandTime = GetGlobalTimeMS();


}

void ZBirdDummyAI::Run()
{
	if (!m_bCreated) return;

	OnRun();


}

void ZBirdDummyAI::Create(ZBirdDummyClient* pClient)
{
	m_pClient = pClient;
	m_nID = m_pClient->GetDummyID();
	m_bCreated = true;
}

////////////////////////////////////////////////////////////////////////////////////////////////
void ZBirdDummyAIMakeRoomFlood::OnCommand(MCommand* pCmd)
{
	Sleep(1);
	if (m_pClient == NULL) return;
	m_nLastCommandTime = GetGlobalTimeMS();

	switch (pCmd->GetID())
	{
	case MC_MATCH_CHANNEL_RESPONSE_RULE:
		{


		}
		break;
	case MC_MATCH_CHANNEL_RESPONSE_JOIN:
		{
			MUID uidChannel;
			char szChannelName[256];

			pCmd->GetParameter(&uidChannel,		0, MPT_UID);
			pCmd->GetParameter(szChannelName,	2, MPT_STR, sizeof(szChannelName) );

			static int nChannelID = 0;
			char szStageName[256];
			sprintf_safe(szStageName, "%s_stage%d", m_szUserID, nChannelID);
			nChannelID++;


			ZBIRDPOSTCMD4(m_pClient, MC_MATCH_STAGE_CREATE, 
				MCommandParameterUID(m_pClient->GetPlayerUID()), 
				MCmdParamStr(szStageName),
				MCmdParamBool(false), 
				MCmdParamStr(""));

		}
		break;
	case MC_MATCH_STAGE_LIST:
		{


		}
		break;
	case MC_MATCH_RESPONSE_STAGE_JOIN:
		{
			int nResult;
			pCmd->GetParameter(&nResult, 0, MPT_INT);

			// 방생성 실패면 다시 만든다
			if (nResult != MOK)
			{
				int nRandNum = rand() % 100000;
				char szStageName[256];
				sprintf_safe(szStageName, "%s_stage%d", "꼬붕즐", nRandNum);


				ZBIRDPOSTCMD4(m_pClient, MC_MATCH_STAGE_CREATE, 
					MCommandParameterUID(m_pClient->GetPlayerUID()), 
					MCmdParamStr(szStageName),
					MCmdParamBool(false), 
					MCmdParamStr(""));

			}
		}
		break;
	case MC_MATCH_STAGE_JOIN:
		{
			MUID uidChar, uidStage;
			char szStageName[256];

			pCmd->GetParameter(&uidChar, 0, MPT_UID);
			pCmd->GetParameter(&uidStage, 1, MPT_UID);
			pCmd->GetParameter(szStageName, 2, MPT_STR, sizeof(szStageName) );

			if (uidChar == m_pClient->GetPlayerUID())
			{
				m_nKillCount = 0;
				ZBIRDPOSTCMD3(m_pClient, MC_MATCH_STAGE_START, 
					MCommandParameterUID(m_pClient->GetPlayerUID()), 
					MCommandParameterUID(m_pClient->GetStageUID()), 
					MCommandParameterInt(3));


				static u32 stJoinCount = 0;
				stJoinCount++;
				char szTemp[512];
				if ((stJoinCount % 100) == 0)
				{
					sprintf_safe(szTemp, "Join Flood(%u)", stJoinCount++);
					AddToLogFrame(m_nID, szTemp);
				}
			}
		}
		break;
	case MC_MATCH_STAGE_LAUNCH:
		{

			ZBIRDPOSTCMD2(m_pClient, MC_MATCH_LOADING_COMPLETE, 
				MCommandParameterUID(m_pClient->GetPlayerUID()), 
				MCmdParamInt(100));

			// 게임에 들어갔다고 알림
			ZBIRDPOSTCMD2(m_pClient, MC_MATCH_STAGE_REQUEST_ENTERBATTLE, 
				MCommandParameterUID(m_pClient->GetPlayerUID()), 
				MCommandParameterUID(m_pClient->GetStageUID()));

		}
		break;
	case MC_MATCH_STAGE_ENTERBATTLE:
		{
			MUID uidChar, uidStage;
			int nParam;

			pCmd->GetParameter(&uidChar, 0, MPT_UID);
			pCmd->GetParameter(&uidStage, 1, MPT_UID);
			pCmd->GetParameter(&nParam, 2, MPT_INT);

			MCommandParameter* pParam = pCmd->GetParameter(3);
			if(pParam->GetType()!=MPT_BLOB) break;
			void* pBlob = pParam->GetPointer();

			MTD_PeerListNode* pPeerNode = (MTD_PeerListNode*)MGetBlobArrayElement(pBlob, 0);

			//OnStageEnterBattle(uidChar, uidStage, MCmdEnterBattleParam(nParam), pPeerNode);

			if (uidChar == m_pClient->GetPlayerUID())
			{
				m_bInCombat = true;
			}

			ZBIRDPOSTCMD2(m_pClient, MC_MATCH_STAGE_LEAVEBATTLE, 
				MCommandParameterUID(m_pClient->GetPlayerUID()), 
				MCommandParameterUID(m_pClient->GetStageUID()));

		}
		break;
	case MC_MATCH_STAGE_LEAVEBATTLE:
		{
			MUID uidChar, uidStage;

			pCmd->GetParameter(&uidChar, 0, MPT_UID);
			pCmd->GetParameter(&uidStage, 1, MPT_UID);

			if (uidChar == m_pClient->GetPlayerUID())
			{
				ZBIRDPOSTCMD2(m_pClient, MC_MATCH_STAGE_LEAVE, 
					MCommandParameterUID(m_pClient->GetPlayerUID()), 
					MCommandParameterUID(m_pClient->GetStageUID()));

				m_bInCombat = false;
			}
		}
		break;
	case MC_MATCH_STAGE_FINISH_GAME:
		{
			MUID uidStage;
			pCmd->GetParameter(&uidStage, 0, MPT_UID);
			
			ZBIRDPOSTCMD2(m_pClient, MC_MATCH_STAGE_LEAVEBATTLE, 
				MCommandParameterUID(m_pClient->GetPlayerUID()), 
				MCommandParameterUID(m_pClient->GetStageUID()));
		}
		break;

	}
}

void ZBirdDummyAIMakeRoomFlood::OnRun()
{
	u32 nNowTime = GetGlobalTimeMS();

	if (m_bInCombat)
	{
		if ((nNowTime - m_nKillLastTime) > BIRDDUMMY_AI_KILL_DELAY)
		{
			m_nKillCount++;
			ZBIRDPOSTCMD1(m_pClient, MC_MATCH_REQUEST_SUICIDE, 
				MCommandParameterUID(m_pClient->GetPlayerUID()));

			m_nKillLastTime = nNowTime;

			if (m_nKillCount > 50)
			{
				m_nKillCount = 0;
				ZBIRDPOSTCMD2(m_pClient, MC_MATCH_STAGE_LEAVEBATTLE, 
					MCommandParameterUID(m_pClient->GetPlayerUID()), 
					MCommandParameterUID(m_pClient->GetStageUID()));
			}
		}
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////////////
void ZBirdDummyAIJoinFlood::OnCommand(MCommand* pCmd)
{
	Sleep(1);
	if (m_pClient == NULL) return;
	m_nLastCommandTime = GetGlobalTimeMS();

	switch (pCmd->GetID())
	{
	case MC_MATCH_CHANNEL_RESPONSE_RULE:
		{


		}
		break;
	case MC_MATCH_CHANNEL_RESPONSE_JOIN:
		{
			MUID uidChannel;
			char szChannelName[256];

			pCmd->GetParameter(&uidChannel,		0, MPT_UID);
			pCmd->GetParameter(szChannelName,	2, MPT_STR, sizeof(szChannelName) );

			static int nChannelID = 0;
			char szStageName[256];
			sprintf_safe(szStageName, "%s_stage%d", m_szUserID, nChannelID);
			nChannelID++;
		}
		break;
	case MC_MATCH_STAGE_LIST:
		{
			int nPrevStageCount, nNextStageCount;
			pCmd->GetParameter(&nPrevStageCount, 0, MPT_INT);
			pCmd->GetParameter(&nNextStageCount, 1, MPT_INT);

			MCommandParameter* pParam = pCmd->GetParameter(2);
			if(pParam->GetType()!=MPT_BLOB) break;
			void* pBlob = pParam->GetPointer();
			int nCount = MGetBlobArrayCount(pBlob);

			for(int i=0; i<nCount; i++) {
				MTD_StageListNode* pNode = (MTD_StageListNode*)MGetBlobArrayElement(pBlob, i);

				// log debug
				if( pNode ) 
				{
					if (_stricmp(pNode->szStageName, m_szLastStage))
					{
						strcpy_safe(m_szLastStage, pNode->szStageName);

						m_uidWantedRoom = pNode->uidStage;
						ZBIRDPOSTCMD2(m_pClient, MC_MATCH_REQUEST_STAGE_JOIN,
							MCommandParameterUID(m_pClient->GetPlayerUID()), 
							MCommandParameterUID(pNode->uidStage));
						break;
					}
				}
			}
		}
		break;
	case MC_MATCH_RESPONSE_STAGE_JOIN:
		{
			int nResult;
			pCmd->GetParameter(&nResult, 0, MPT_INT);

			if (nResult != MOK)
			{
				ZBIRDPOSTCMD2(m_pClient, MC_MATCH_REQUEST_STAGE_JOIN,
					MCommandParameterUID(m_pClient->GetPlayerUID()), 
					MCommandParameterUID(m_uidWantedRoom));

			}
		}
		break;
	case MC_MATCH_STAGE_JOIN:
		{
			m_nReservedTime = GetGlobalTimeMS();
			m_bReserved = true;

			MUID uidChar, uidStage;
			char szStageName[256];

			pCmd->GetParameter(&uidChar, 0, MPT_UID);
			pCmd->GetParameter(&uidStage, 1, MPT_UID);
			pCmd->GetParameter(szStageName, 2, MPT_STR, sizeof(szStageName) );

//			ZBIRDPOSTCMD2(m_pClient, MC_MATCH_STAGE_LEAVE,
//				MCommandParameterUID(m_pClient->GetPlayerUID()), 
//				MCommandParameterUID(m_uidWantedRoom));

				static u32 stJoinCount = 0;
				stJoinCount++;
				char szTemp[512];
				if ((stJoinCount % 100) == 0)
				{
					sprintf_safe(szTemp, "Join Flood(%u)", stJoinCount++);
					AddToLogFrame(m_nID, szTemp);
				}
		}
		break;
	}
}

void ZBirdDummyAIJoinFlood::OnRun()
{
	u32 nNowTime = GetGlobalTimeMS();

	if (m_bReserved)
	{
		if ((nNowTime - m_nReservedTime) > 10000)
		{
			ZBIRDPOSTCMD2(m_pClient, MC_MATCH_STAGE_LEAVE,
				MCommandParameterUID(m_pClient->GetPlayerUID()), 
				MCommandParameterUID(m_uidWantedRoom));


			m_bReserved = false;
		}
	}
}

ZBirdDummyAIJoinFlood::ZBirdDummyAIJoinFlood() : ZBirdDummyAI()
{
	m_szLastStage[0] = 0;
	m_bReserved = false;
	m_nReservedTime = 0;	
}