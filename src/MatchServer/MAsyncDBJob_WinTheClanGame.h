#ifndef _MASYNCDBJOB_WINTHECLANGAME_H
#define _MASYNCDBJOB_WINTHECLANGAME_H


#include "MAsyncDBJob.h"

class MAsyncDBJob_WinTheClanGame : public MAsyncJob {
protected:	// Input Argument
	int			m_nWinnerCLID;
	int			m_nLoserCLID;
	bool		m_bIsDrawGame;
    int			m_nWinnerPoint;
	int			m_nLoserPoint;
	char		m_szWinnerClanName[CLAN_NAME_LENGTH];
    char		m_szLoserClanName[CLAN_NAME_LENGTH];
	int			m_nRoundWins;
	int			m_nRoundLosses;
    int			m_nMapID;
	int			m_nGameType;
    char		m_szWinnerMembers[512];
	char		m_szLoserMembers[512];
protected:	// Output Result

public:
	MAsyncDBJob_WinTheClanGame()
		: MAsyncJob(MASYNCJOB_WINTHECLANGAME)
	{

	}
	virtual ~MAsyncDBJob_WinTheClanGame()	{}

	bool Input(const int nWinnerCLID, const int nLoserCLID, const bool bIsDrawGame,
					const int nWinnerPoint, const int nLoserPoint, const char* szWinnerClanName,
					const char* szLoserClanName, const int nRoundWins, const int nRoundLosses,
					const int nMapID, const int nGameType,
					const char* szWinnerMembers, const char* szLoserMembers);


	virtual void Run(void* pContext);
};




#endif