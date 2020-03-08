#include "stdafx.h"
#include "MAsyncDBJob_WinTheClanGame.h"

void MAsyncDBJob_WinTheClanGame::Run(void* pContext)
{
	auto* pDBMgr = static_cast<IDatabase*>(pContext);


	if (!pDBMgr->WinTheClanGame(m_nWinnerCLID, 
								m_nLoserCLID, 
								m_bIsDrawGame,
								m_nWinnerPoint, 
								m_nLoserPoint,
								m_szWinnerClanName, 
								m_szLoserClanName,
								m_nRoundWins, 
								m_nRoundLosses, 
								m_nMapID, 
								m_nGameType, 
								m_szWinnerMembers, 
								m_szLoserMembers ))
	{
		SetResult(MASYNC_RESULT_FAILED);
		return;
	}


	SetResult(MASYNC_RESULT_SUCCEED);
}


bool MAsyncDBJob_WinTheClanGame::Input(const int nWinnerCLID, const int nLoserCLID, const bool bIsDrawGame,
				const int nWinnerPoint, const int nLoserPoint, const char* szWinnerClanName,
				const char* szLoserClanName, const int nRoundWins, const int nRoundLosses,
				const int nMapID, const int nGameType,
				const char* szWinnerMembers, const char* szLoserMembers)
{
	m_nWinnerCLID = nWinnerCLID;
	m_nLoserCLID = nLoserCLID;
	m_bIsDrawGame = bIsDrawGame;
	m_nWinnerPoint = nWinnerPoint;
	m_nLoserPoint = nLoserPoint;
	m_nRoundWins = nRoundWins;
	m_nRoundLosses = nRoundLosses;
	m_nMapID = nMapID;
	m_nGameType = nGameType;
	strcpy_safe(m_szWinnerClanName, szWinnerClanName);
	strcpy_safe(m_szLoserClanName, szLoserClanName);
	strcpy_safe(m_szWinnerMembers, szWinnerMembers);
	strcpy_safe(m_szLoserMembers, szLoserMembers);

	return true;
}
