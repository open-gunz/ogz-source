#pragma once

#include <string>
#include <list>
#include <vector>
#include <map>
#include <deque>
#include <algorithm>
#include "MUID.h"

#define	CONSOLE_TEXTSIZE	4096
#define	CONSOLE_NAMESIZE	256
#define	ARGVNoMin (-1)
#define ARGVNoMax (-1)

#define ADMIN_COMMAND_PREFIX		'@'

struct MAdminArgvInfo
{
	char *cargv[255];
	int cargc;
	char argbuf[2048];
};

class MMatchServer;

class MMatchAdmin
{
private:
protected:
	MMatchServer*		m_pMatchServer;
	bool MakeArgv(char* szStr, MAdminArgvInfo* pAi);
public:
	MMatchAdmin();
	virtual ~MMatchAdmin();
	bool Create(MMatchServer* pServer);
	void Destroy();
	bool Execute(const MUID& uidAdmin, const char* szStr);
};