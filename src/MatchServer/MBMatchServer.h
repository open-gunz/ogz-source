#ifndef MBMATCHSERVER_H
#define MBMATCHSERVER_H

#include "MMatchServer.h"
#include "MBMatchServerConfigReloader.h"
#include <memory>
#include "MLocator.h"
#include "MFile.h"

class COutputView;
class CCommandLogView;

class MBMatchServer : public MMatchServer
{
private :
	MBMatchServerConfigReloader m_ConfigReloader;

public:
#ifdef MFC
	COutputView*		m_pView;
	CCommandLogView*	m_pCmdLogView;
#endif

	MUID m_uidKeeper;
protected:
	virtual bool OnCreate();
	virtual void OnDestroy();
	virtual void OnPrepareCommand(MCommand* pCommand);
	virtual bool OnCommand(MCommand* pCommand);

	virtual void OnRun() override;

public:
	MBMatchServer(COutputView* pView=NULL);
	virtual void Shutdown();
	virtual void Log(unsigned int nLogLevel, const char* szLog);
	void OnViewServerStatus();

private :
	bool InitSubTaskSchedule();
	bool AddClanServerAnnounceSchedule();
	bool AddClanServerSwitchDownSchedule();
	bool AddClanServerSwitchUpSchedule();
	
	void OnScheduleAnnounce( const char* pszAnnounce );
	void OnScheduleClanServerSwitchDown();
	void OnScheduleClanServerSwitchUp();

	const MUID GetKeeperUID() const { return m_uidKeeper; }
	void SetKeeperUID( const MUID& uidKeeper ) { m_uidKeeper = uidKeeper; }

	void WriteServerInfoLog();

protected :
	bool IsKeeper( const MUID& uidKeeper );

	void OnResponseServerStatus( const MUID& uidSender );
	void OnRequestServerHearbeat( const MUID& uidSender );
	void OnResponseServerHeartbeat( const MUID& uidSender );
	void OnRequestConnectMatchServer( const MUID& uidSender );
	void OnResponseConnectMatchServer( const MUID& uidSender );
	void OnRequestKeeperAnnounce( const MUID& uidSender, const char* pszAnnounce );
	void OnRequestStopServerWithAnnounce( const MUID& uidSender );
	void OnResponseStopServerWithAnnounce( const MUID& uidSender );
	void OnRequestSchedule( const MUID& uidSender, 
							const int nType, 
							const int nYear, 
							const int nMonth, 
							const int nDay, 
							const int nHour, 
							const int nMin,
							const int nCount,
							const int nCommand,
							const char* pszAnnounce );
	void OnResponseSchedule( const MUID& uidSender, 
							 const int nType, 
							 const int nYear, 
							 const int nMonth, 
							 const int nDay, 
							 const int nHour, 
							 const int nMin,
							 const int nCount,
							 const int nCommand,
							 const char* pszAnnounce );
	void OnRequestKeeperStopServerSchedule( const MUID& uidSender, const char* pszAnnounce );
	void OnResponseKeeperStopServerSchedule( const MUID& uidSender, const char* pszAnnounce );
	void OnRequestDisconnectServerFromKeeper( const MUID& uidSender );
	void OnRequestReloadServerConfig( const MUID& uidSender, const string& strFileList );
	void OnResponseReloadServerConfig( const MUID& uidSender, const string& strFileList );
	void OnRequestAddHashMapnResponseAddHashMap( const MUID& uidSender, const string& strNewHashValue );

public:
	char	m_strFileCrcDataPath[MFile::MaxPath];

	void InitLocator();

	void OnInput(const std::string& Input);

private:
	void InitConsoleCommands();

	std::unique_ptr<MLocator> Locator;

	struct ConsoleCommand
	{
		std::function<void()> Callback;
		int MinArgs = -1;
		int MaxArgs = -1;
		std::string Description;
		std::string Usage;
		std::string Help;
	};
	std::unordered_map<std::string, ConsoleCommand> ConsoleCommandMap;
};


#endif